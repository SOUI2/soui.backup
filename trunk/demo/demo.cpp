// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "DuiSystem.h" 

#ifdef _DEBUG
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#ifndef DLL_SOUI
#include "../render-skia/render-api.h"
#include "../render-gdi/render-api.h"
#include "../imgdecoder-wic/imgdecoder-wic.h"

#ifdef _DEBUG
#pragma comment(lib,"utilities_d.lib")
#pragma comment(lib,"render-skia_d.lib")
#pragma comment(lib,"render-gdi_d.lib")
#pragma comment(lib,"imgdecoder-wic_d.lib")
#pragma comment(lib,"skcore_d.lib")
#pragma comment(lib,"../myskia/third_party/freetype/lib/freetype253_D.lib")
#pragma comment(lib,"usp10.lib")
#else
#pragma comment(lib,"utilities.lib")
#pragma comment(lib,"render-skia.lib")
#pragma comment(lib,"render-gdi.lib")
#pragma comment(lib,"imgdecoder-wic.lib")
#pragma comment(lib,"skcore.lib")
#pragma comment(lib,"../myskia/third_party/freetype/lib/freetype253.lib")
#pragma comment(lib,"usp10.lib")
#endif
#endif

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	DUIASSERT(SUCCEEDED(hRes));

    CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
    CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
    
#ifndef _LIB
#ifdef _DEBUG
    HMODULE hImgDecoder = LoadLibrary(_T("imgdecoder-wic_d.dll"));
    HMODULE hRender = LoadLibrary(_T("render-skia_d.dll"));
#else
    HMODULE hImgDecoder = LoadLibrary(_T("imgdecoder-wic.dll"));
    HMODULE hRender = LoadLibrary(_T("render-skia.dll"));
#endif
    typedef BOOL (*fnCreateImgDecoderFactory)(SOUI::IImgDecoderFactory**,BOOL);
    fnCreateImgDecoderFactory funImg = (fnCreateImgDecoderFactory)GetProcAddress(hImgDecoder,"CreateImgDecoderFactory_WIC");
    funImg(&pImgDecoderFactory,TRUE);
    
    typedef BOOL (*fnCreateRenderFactory)(SOUI::IRenderFactory **,SOUI::IImgDecoderFactory *);
    fnCreateRenderFactory funRender = (fnCreateRenderFactory)GetProcAddress(hRender,"CreateRenderFactory_Skia");
    funRender(&pRenderFactory,pImgDecoderFactory);
#else
    SOUI::CreateImgDecoderFactory_WIC(&pImgDecoderFactory,TRUE);
    CreateRenderFactory_Skia(&pRenderFactory,pImgDecoderFactory);
#endif
    
	DuiSystem *pDuiSystem=new DuiSystem(pRenderFactory,hInstance);

#if 1
    TCHAR szCurrentDir[MAX_PATH]={0};
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
    pImgDecoderFactory=NULL;
    
	OleUninitialize();
	return nRet;
}
