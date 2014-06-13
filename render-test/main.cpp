#include "stdafx.h"

#include <vld.h>

#include <WINDOWS.H>

#include "SimpleWnd.h"
#include "duicrack.h"

#include "render-api.h"

#include <color.h>
#include <unknown/obj-ref-impl.hpp>

using namespace DuiEngine;


SOUI::IRenderFactory *g_render=NULL;

HINSTANCE g_hInst;

void RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LPBYTE GetResBuf(LPCTSTR pszName,LPCTSTR pszType,size_t & szBuf,HINSTANCE hInst)
{
	LPBYTE pRet=NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	
	// Locate the resource.
	imageResHandle = FindResource(hInst, pszName, pszType);

	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(hInst, imageResHandle);

		hr = imageResDataHandle ? S_OK : E_FAIL;
	}
	if (!SUCCEEDED(hr)) return NULL;

		// Lock it to get a system memory pointer.
	pRet = (LPBYTE)LockResource(imageResDataHandle);

	// Calculate the size.
	szBuf = SizeofResource(hInst, imageResHandle);

	return pRet;	
}

namespace SOUI
{
class CMainWnd : public CSimpleWnd
{
public:
	CMainWnd()
	{

	}

    ~CMainWnd()
    {
    }

	void OnPaint(HDC hdc)
	{
		PAINTSTRUCT ps;
		hdc=::BeginPaint(m_hWnd,&ps);
		RECT rcClient;
		GetClientRect(&rcClient);
		m_rt->BindDC(hdc,&rcClient);
  		m_rt->BeginDraw();
//   		m_rt->PushClipRegion(m_rgn);
        RECT rcClip=rcClient;
        InflateRect(&rcClip,-10,-10);
        m_rt->PushClipRect(&rcClip);

 		m_rt->DrawBitmap(&rcClient,m_bmp,NULL,128);
  		m_rt->FillRectangle(rcClient.left,rcClient.top,rcClient.right/2,rcClient.bottom);
  		TCHAR *psz=_T("文字输出测试,\nprefix &test\n文字输出测试,文字输出测试");
  		m_rt->DrawText(psz,-1,&rcClient,DT_VCENTER|DT_SINGLELINE|DT_CENTER,128);
        POINT pt[3]={{10,10},{10,100},{100,100}};
  		m_rt->DrawLines(pt,3);

        RECT rc={100,100,200,200};
        POINT pt2={5,5};

        CAutoRefPtr<IPen> pen,oldPen;
        m_rt->CreatePen(PS_DASHDOTDOT,CDuiColor(0xFF,0,0,0x55).toCOLORREF(),2,&pen);

        m_rt->SelectObject(pen,(IRenderObj**)&oldPen);
        m_rt->DrawRoundRect(&rc,pt2);
        m_rt->SelectObject(oldPen);

        InflateRect(&rc,-2,-2);

        m_rt->OffsetViewportOrg(50,50);
        
        RECT rc2={100,230,200,400};

        m_rt->GradientFill(&rc2,FALSE,RGB(255,0,0),RGB(0,0,255),128);

        CAutoRefPtr<IBrush> brColor,brOld;
        m_rt->CreateSolidColorBrush(0xFF0000FF,&brColor);
        m_rt->SelectObject(brColor,(IRenderObj**)&brOld);
        m_rt->FillRoundRect(&rc,pt2);
        m_rt->SelectObject(brOld);
        m_rt->OffsetViewportOrg(-50,-50);
        
        
        HDC hdc2=m_rt->GetDC(0);
        TextOut(hdc2,0,0,_T("text at (0,0)"),_tcslen(_T("text at (0,0)")));
        
        TextOut(hdc2,100,100,_T("text at (100,100)"),_tcslen(_T("text at (100,100)")));
        m_rt->ReleaseDC(hdc2);
        
        m_rt->PopClipRect();
//  		m_rt->PopClipRegion();
  		m_rt->EndDraw();
		::EndPaint(m_hWnd,&ps);
	}

	void OnDestroy()
	{
		m_rt.Release();

		PostMessage(WM_QUIT);
	}

	int OnCreate(void *)
	{
		g_render->CreateRenderTarget(&m_rt,0,0);
		m_rt->CreateRegion(&m_rgn);

		CAutoRefPtr<IFont> font;
		LOGFONT lf={0};
		_tcscpy(lf.lfFaceName,_T("隶书"));
		lf.lfHeight=30;
		lf.lfItalic=1;
		m_rt->CreateFont(lf,&font);
		m_rt->SelectObject(font);

		m_rt->SetTextColor(RGBA(255,0,0,255));
		RECT rc1={0,0,100,100};
		RECT rc2={200,200,300,300};
		RECT rc3={90,90,210,210};
		m_rgn->CombineRect(&rc1,RGN_OR);
		m_rgn->CombineRect(&rc2,RGN_OR);
		m_rgn->CombineRect(&rc3,RGN_OR);

		size_t szImg=0;
		LPBYTE pImgData=GetResBuf(MAKEINTRESOURCE(IDR_JPG1),_T("JPG"),szImg,g_hInst);
	
		m_rt->CreateBitmap(&m_bmp);
		m_bmp->LoadFromMemory(m_rt,pImgData,szImg,NULL);

		//创建位图画刷
		pImgData = GetResBuf(MAKEINTRESOURCE(IDR_ICON),_T("JPG"),szImg,g_hInst);
		CAutoRefPtr<IBitmap> icon;
		m_rt->CreateBitmap(&icon);
		icon->LoadFromMemory(m_rt,pImgData,szImg,NULL);

		m_rt->CreateBitmapBrush(icon,&m_brIcon);
		m_rt->SelectObject(m_brIcon);

		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam,LPARAM lParam,BOOL &bHandled)
	{
		SIZE size={LOWORD(lParam),HIWORD(lParam)};
		m_rt->Resize(size);
		bHandled=TRUE;
		return 0;
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		return FALSE;
	}
	BEGIN_MSG_MAP_EX(CMainWnd)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_PAINT(OnPaint);
		MESSAGE_HANDLER(WM_SIZE,OnSize)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
	END_MSG_MAP()

protected:
	CAutoRefPtr<IRenderTarget> m_rt;
	CAutoRefPtr<IBitmap> m_bmp;
	CAutoRefPtr<IRegion> m_rgn;
	CAutoRefPtr<IBrush>  m_brIcon;
};


}

//#define USING_LOADLIB
//
int WINAPI WinMain(
				   HINSTANCE hInstance,
				   HINSTANCE /*hPrevInstance*/,
				   LPSTR /*lpCmdLine*/,
				   int /*nCmdShow*/
				   )
{
	g_hInst=hInstance;
	CSimpleWndClass wndClass(hInstance,_T("DuiRenderHost"),HBRUSH(COLOR_WINDOW+1));
	CSimpleWndHelper::Init(wndClass);

	CoInitialize(NULL);



#ifdef USING_LOADLIB
#ifdef _DEBUG
    HMODULE hRenderSkia = LoadLibrary(_T("render-skia_d.dll"));
#else
    HMODULE hRenderSkia = LoadLibrary(_T("render-skia.dll"));
#endif
    typedef SOUI::IRenderFactory * (*fnCreateRenderFactory)();
    fnCreateRenderFactory fun = (fnCreateRenderFactory)GetProcAddress(hRenderSkia,"CreateRenderFactory");
    
    g_render = fun();
#else
    g_render = RENDER_SKIA::CreateRenderFactory();
#endif

	if(g_render)
	{
		{//保证wndMain对象先于render析构
			SOUI::CMainWnd wndMain;
			wndMain.Create(_T("render-skia Test"),0x17cf0000,0,0,0,300,300,NULL,NULL);

			RunMessageLoop();
		}
		g_render->Release();
	}
	
	CoUninitialize();

	CSimpleWndHelper::Uninit();
	return 0;
}