// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "DuiSystem.h" 

#ifdef _DEBUG
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#ifndef DLL_SOUI
#include "../render-gdi/render-api.h"
#include "../imgdecoder-wic/imgdecoder-wic.h"

#ifdef _DEBUG
#pragma comment(lib,"utilities_d.lib")
#pragma comment(lib,"render-gdi_d.lib")
#pragma comment(lib,"imgdecoder-wic_d.lib")
#else
#pragma comment(lib,"utilities.lib")
#pragma comment(lib,"render-gdi.lib")
#pragma comment(lib,"imgdecoder-wic.lib")
#endif
#endif

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
// 	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	HRESULT hRes = OleInitialize(NULL);
	DUIASSERT(SUCCEEDED(hRes));

    CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
    CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
    
#ifndef _LIB
#ifdef _DEBUG
    HMODULE hImgDecoder = LoadLibrary(_T("imgdecoder-wic_d.dll"));
    HMODULE hRender = LoadLibrary(_T("render-gdi_d.dll"));
#else
    HMODULE hImgDecoder = LoadLibrary(_T("imgdecoder-wic.dll"));
    HMODULE hRender = LoadLibrary(_T("render-gdi.dll"));
#endif
    typedef BOOL (*fnCreateImgDecoderFactory)(SOUI::IImgDecoderFactory**);
    fnCreateImgDecoderFactory funImg = (fnCreateImgDecoderFactory)GetProcAddress(hImgDecoder,"CreateImgDecoderFactory");
    funImg(&pImgDecoderFactory);
    
    typedef BOOL (*fnCreateRenderFactory)(SOUI::IRenderFactory **,SOUI::IImgDecoderFactory *);
    fnCreateRenderFactory funRender = (fnCreateRenderFactory)GetProcAddress(hRender,"CreateRenderFactory");
    funRender(&pRenderFactory,pImgDecoderFactory);
    pImgDecoderFactory=NULL;
#else
    SOUI::CreateImgDecoderFactory(&pImgDecoderFactory);
    RENDER_GDI::CreateRenderFactory(&pRenderFactory,pImgDecoderFactory);
#endif
    
	DuiSystem *pDuiSystem=new DuiSystem(pRenderFactory,hInstance);

#if 1
    TCHAR szCurrentDir[MAX_PATH]; memset( szCurrentDir, 0, sizeof(szCurrentDir) );
    GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
    LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
    *lpInsertPos = _T('\0');   
    _tcscat( szCurrentDir, _T("\\..\\demo\\skin") );

    DuiResProviderFiles *pResProvider=new DuiResProviderFiles;
    if(!pResProvider->Init(szCurrentDir))
    {
        DUIASSERT(0);
        return 1;
    }
#else
    DuiResProviderPE *pResProvider = new DuiResProviderPE(hInstance);
#endif
    
    pDuiSystem->AddResProvider(pResProvider);

	BOOL bOK=pDuiSystem->Init(_T("IDR_DUI_INIT")); //初始化DUI系统,原来的系统初始化方式依然可以使用。
	pDuiSystem->SetMsgBoxTemplate(_T("IDR_DUI_MSGBOX"));

	int nRet = 0; 
	// BLOCK: Run application
	{
		CMainDlg dlgMain;  
		nRet = dlgMain.DoModal();  
	}



	delete pDuiSystem;
    
    delete pResProvider;
    
    pRenderFactory=NULL;
    
	OleUninitialize();
	return nRet;
}
