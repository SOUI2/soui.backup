// VUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainWnd.h"
#include <com-cfg.h>
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	int nRet = 0; 

	//将程序的运行路径修改到项目所在目录所在的目录
	TCHAR szCurrentDir[MAX_PATH]={0};
	GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
	LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
	_tcscpy(lpInsertPos+1,_T("..\\demos\\VUI"));
	SetCurrentDirectory(szCurrentDir);

	SComMgr *pComMgr = new SComMgr();
	{

		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;

		pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

		SApplication *theApp=new SApplication(pRenderFactory,hInstance);

		HMODULE hSysResource=LoadLibrary(SYS_NAMED_RESOURCE);
		if(hSysResource)
		{
			CAutoRefPtr<IResProvider> sysSesProvider;
			CreateResProvider(RES_PE,(IObjRef**)&sysSesProvider);
			sysSesProvider->Init((WPARAM)hSysResource,0);
			theApp->LoadSystemNamedResource(sysSesProvider);
		}


		CAutoRefPtr<IResProvider>   pResProvider;
		CreateResProvider(RES_FILE,(IObjRef**)&pResProvider);
		if(!pResProvider->Init((LPARAM)_T("uires"),0))
		{
			SASSERT(0);
			return 1;
		}
		theApp->AddResProvider(pResProvider);


		{//在这里加入主窗口运行代码
			CMainWnd wndMain;  
			wndMain.Create(GetActiveWindow(),0,0,0,0);
			wndMain.SendMessage(WM_INITDIALOG);
			wndMain.CenterWindow(wndMain.m_hWnd);
			wndMain.ShowWindow(SW_SHOWNORMAL);
			nRet=theApp->Run(wndMain.m_hWnd);
			//程序结束

		}

		delete theApp;

	}
	delete pComMgr;

	OleUninitialize();
	return nRet;
}