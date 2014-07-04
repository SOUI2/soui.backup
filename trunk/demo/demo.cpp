// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "SApp.h" 

#ifdef _DEBUG
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#define RENDER_GDI

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	ASSERT(SUCCEEDED(hRes));

    CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
    CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
    
#ifdef _DEBUG
    HMODULE hImgDecoder = LoadLibrary(_T("imgdecoder-wic_d.dll"));
    #ifdef RENDER_GDI
    HMODULE hRender = LoadLibrary(_T("render-gdi_d.dll"));
    #else
    HMODULE hRender = LoadLibrary(_T("render-skia_d.dll"));
    #endif
#else
    HMODULE hImgDecoder = LoadLibrary(_T("imgdecoder-wic.dll"));
    #ifdef RENDER_GDI
    HMODULE hRender = LoadLibrary(_T("render-gdi.dll"));
    #else
    HMODULE hRender = LoadLibrary(_T("render-skia.dll"));
    #endif
#endif
    typedef BOOL (*fnCreateImgDecoderFactory)(SOUI::IImgDecoderFactory**,BOOL);
    fnCreateImgDecoderFactory funImg = (fnCreateImgDecoderFactory)GetProcAddress(hImgDecoder,"CreateImgDecoderFactory_WIC");
    funImg(&pImgDecoderFactory,TRUE);
    
    typedef BOOL (*fnCreateRenderFactory)(SOUI::IRenderFactory **,SOUI::IImgDecoderFactory *);
    #ifdef RENDER_GDI
    fnCreateRenderFactory funRender = (fnCreateRenderFactory)GetProcAddress(hRender,"CreateRenderFactory_GDI");
    #else
    fnCreateRenderFactory funRender = (fnCreateRenderFactory)GetProcAddress(hRender,"CreateRenderFactory_Skia");
    #endif
    
    funRender(&pRenderFactory,pImgDecoderFactory);
    
	SApplication *theApp=new SApplication(pRenderFactory,hInstance);

#if 1
    TCHAR szCurrentDir[MAX_PATH]={0};
    GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
    LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
    *lpInsertPos = _T('\0');   
    _tcscat( szCurrentDir, _T("\\..\\demo\\skin") );

    SResProviderFiles *pResProvider=new SResProviderFiles;
    if(!pResProvider->Init(szCurrentDir))
    {
        ASSERT(0);
        return 1;
    }
#else
    SResProviderPE *pResProvider = new SResProviderPE(hInstance);
#endif
    
    theApp->AddResProvider(pResProvider);

	BOOL bOK=theApp->Init(_T("IDR_DUI_INIT")); //初始化DUI系统,原来的系统初始化方式依然可以使用。
	theApp->SetMsgBoxTemplate(_T("IDR_DUI_MSGBOX"));

	int nRet = 0; 
	// BLOCK: Run application
	{
		CMainDlg dlgMain;  
		nRet = dlgMain.DoModal();  
	}



	delete theApp;
    
    delete pResProvider;
    
    pRenderFactory=NULL;
    pImgDecoderFactory=NULL;
    
	OleUninitialize();
	return nRet;
}
