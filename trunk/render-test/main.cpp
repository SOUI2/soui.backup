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

 		m_rt->DrawBitmap(&rcClient,m_bmp,0,0,128);
 		RECT rcBrush=rcClient;
 		rcBrush.right/=2;
  		m_rt->FillRectangle(&rcBrush);
  		TCHAR *psz=_T("文字输出测试,\nprefix &test\n文字输出测试,文字输出测试");
  		m_rt->DrawText(psz,-1,&rcClient,DT_VCENTER|DT_SINGLELINE|DT_CENTER,128);
        POINT pt[3]={{10,10},{10,100},{100,100}};
  		m_rt->DrawLines(pt,3);

        RECT rc={100,100,200,200};
        POINT pt2={5,5};

        CAutoRefPtr<IPen> pen,oldPen;
        m_rt->CreatePen(PS_DASHDOTDOT,SColor(0xFF,0,0,0x55).toCOLORREF(),2,&pen);

        m_rt->SelectObject(pen,(IRenderObj**)&oldPen);
        m_rt->DrawRoundRect(&rc,pt2);
        m_rt->SelectObject(oldPen);

        InflateRect(&rc,-2,-2);

        //viewport origin test
        {
            m_rt->OffsetViewportOrg(50,50);

            RECT rc2={100,230,200,400};

            m_rt->GradientFill(&rc2,FALSE,RGB(255,0,0),RGB(0,0,255),128);

            CAutoRefPtr<IBrush> brColor,brOld;
            m_rt->CreateSolidColorBrush(0xFF0000FF,&brColor);
            m_rt->SelectObject(brColor,(IRenderObj**)&brOld);
            m_rt->FillRoundRect(&rc,pt2);
            m_rt->SelectObject(brOld);
            m_rt->OffsetViewportOrg(-50,-50);
        }
        

        //gdi test        
        {
            HDC hdc2=m_rt->GetDC(0);
            TextOut(hdc2,0,0,_T("text at (0,0)"),_tcslen(_T("text at (0,0)")));
            TextOut(hdc2,100,100,_T("text at (100,100)"),_tcslen(_T("text at (100,100)")));
            m_rt->ReleaseDC(hdc2);
        }
        
        //draw9patch test
        {
            RECT rcSrc={0,0,13,85};
            RECT rcMargin={5,5,5,55};
            
            RECT rcDest={300,10,500,250};
            
            RECT rcMargin2={5,0,5,0};
            m_rt->DrawBitmap9Patch(&rcDest,m_bmp9Patch,&rcSrc,&rcMargin2,EM_TILE,0xcc);
            OffsetRect(&rcDest,0,rcDest.bottom-rcDest.top +5);
            rcDest.right=rcDest.left+100;
            rcDest.bottom=rcDest.top+100;
            m_rt->DrawBitmap9Patch(&rcDest,m_bmp9Patch,&rcSrc,&rcMargin,EM_STRETCH,0xcc);
            OffsetRect(&rcDest,0,rcDest.bottom-rcDest.top+5);
            rcDest.bottom=rcDest.top+50;
            m_rt->DrawBitmap9Patch(&rcDest,m_bmp9Patch,&rcSrc,&rcMargin,EM_STRETCH,0xcc);
        }
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
		g_render->CreateRegion(&m_rgn);

		CAutoRefPtr<IFont> font;
		LOGFONT lf={0};
		_tcscpy(lf.lfFaceName,_T("隶书"));
		lf.lfHeight=30;
		lf.lfItalic=1;
		g_render->CreateFont(&font,lf);
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
	
		g_render->CreateBitmap(&m_bmp);
		m_bmp->LoadFromMemory(pImgData,szImg,NULL);

		//创建位图画刷
		pImgData = GetResBuf(MAKEINTRESOURCE(IDR_ICON),_T("JPG"),szImg,g_hInst);
		CAutoRefPtr<IBitmap> icon;
		g_render->CreateBitmap(&icon);
		icon->LoadFromMemory(pImgData,szImg,NULL);

		m_rt->CreateBitmapBrush(icon,&m_brIcon);
		m_rt->SelectObject(m_brIcon);

        g_render->CreateBitmap(&m_bmp9Patch);
        pImgData = GetResBuf(MAKEINTRESOURCE(IDB_PNG1),_T("PNG"),szImg,g_hInst);
        m_bmp9Patch->LoadFromMemory(pImgData,szImg,NULL);
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
    CAutoRefPtr<IBitmap> m_bmp9Patch;
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
    typedef  BOOL (*fnCreateRenderFactory)(SOUI::IRenderFactory **ppRenderFactory);
    fnCreateRenderFactory fun = (fnCreateRenderFactory)GetProcAddress(hRenderSkia,"CreateRenderFactory");
    
    fun(&g_render);
#else
    RENDER_SKIA::CreateRenderFactory(&g_render);
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