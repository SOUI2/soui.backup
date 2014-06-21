// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "DuiSystem.h" 

#ifdef _DEBUG
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	DUIASSERT(SUCCEEDED(hRes));

#ifdef _DEBUG
    HMODULE hRenderSkia = LoadLibrary(_T("render-skia_d.dll"));
#else
    HMODULE hRenderSkia = LoadLibrary(_T("render-skia.dll"));
#endif
    typedef BOOL (*fnCreateRenderFactory)(SOUI::IRenderFactory **);
    fnCreateRenderFactory fun = (fnCreateRenderFactory)GetProcAddress(hRenderSkia,"CreateRenderFactory");

    CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
    fun(&pRenderFactory);

	DuiSystem *pDuiSystem=new DuiSystem(pRenderFactory,hInstance);


	TCHAR szCurrentDir[MAX_PATH]; memset( szCurrentDir, 0, sizeof(szCurrentDir) );
	GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
	LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
	*lpInsertPos = _T('\0');   

    _tcscat( szCurrentDir, _T("\\..\\demo\\skin") );
    DuiResProviderFiles *pResFiles=new DuiResProviderFiles;
    if(!pResFiles->Init(szCurrentDir))
    {
        DUIASSERT(0);
        return 1;
    }
    
    pDuiSystem->AddResProvider(pResFiles);

	BOOL bOK=pDuiSystem->Init(_T("IDR_DUI_INIT")); //初始化DUI系统,原来的系统初始化方式依然可以使用。
	pDuiSystem->SetMsgBoxTemplate(_T("IDR_DUI_MSGBOX"));

	int nRet = 0; 
	// BLOCK: Run application
	{
		CMainDlg dlgMain;  
		nRet = dlgMain.DoModal();  
	}



	delete pDuiSystem;
    
    delete pResFiles;
    
    pRenderFactory=NULL;
    
	OleUninitialize();
	return nRet;
}
