// EtimesHelper.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MainWnd.h"
#include "com-cfg.h"

#ifdef _DEBUG
#define RES_TYPE 0
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define RES_TYPE 1
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif


// 此代码模块中包含的函数的前向声明:


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	
	
	SComMgr* pComMgr = new SComMgr(_T("imgdecoder-gdip"));
	{
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
		if (!pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory))
		{
			delete pComMgr;
			::MessageBox(NULL, _T("Load CreateRender_Skia Error!"), _T("提示"), MB_ICONINFORMATION);
			return 0;
		}
		if (!pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory))
		{
			delete pComMgr;
			::MessageBox(NULL, _T("Load CreateImgDecoder Error!"), _T("提示"), MB_ICONINFORMATION);
			return 0;
		}

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

		//定义一个唯一的SApplication对象，SApplication管理整个应用程序的资源
		SApplication *theApp=new SApplication(pRenderFactory, hInstance);
		//向app注册自定义类
		theApp->RegisterWindowClass<SPathBar>();
		theApp->RegisterWindowClass<SFileList>();
		theApp->RegisterSkinClass<SSkinSystemIconList>();
		
		//从 dll 加载 soui 自带系统资源
		HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
		if(NULL != hModSysResource)
		{
			CAutoRefPtr<IResProvider> sysResProvider;
			CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
			sysResProvider->Init((WPARAM)hModSysResource, 0);
			theApp->LoadSystemNamedResource(sysResProvider);
			FreeLibrary(hModSysResource);
			hModSysResource = NULL;
		}
		else
		{
			MessageBox(NULL, _T("Load SYS_NAMED_RESOURCE Error!"), _T("提示"), MB_ICONINFORMATION);
			SASSERT(0);
		}

		CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
		CreateResProvider(RES_FILE,(IObjRef**)&pResProvider);
		TCHAR lpResPath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, lpResPath, MAX_PATH);
		PathRemoveFileSpec(lpResPath);
		PathRemoveFileSpec(lpResPath);
		PathAddBackslash(lpResPath);
		// 这里用自己工程 名
		_tcscat_s(lpResPath, MAX_PATH, _T("demos\\DropDemo\\uires"));
		if (!pResProvider->Init((LPARAM)lpResPath, 0))
		{
			SASSERT(0);
			return 1;
		}
#else 
		CreateResProvider(RES_PE,(IObjRef**)&pResProvider);
		pResProvider->Init((WPARAM)hInstance,0);
#endif

		//将创建的IResProvider交给SApplication对象
		theApp->AddResProvider(pResProvider);

		SNotifyCenter* pNotifyCenter = new SNotifyCenter;
		{
		
			CMainWnd dlg;
			//dlg.DoModal();
			
			dlg.Create(GetActiveWindow(),0,0,0,0);

			dlg.GetNative()->SendMessage(WM_INITDIALOG);
			
			dlg.ShowWindow(SW_SHOWNORMAL);
					
			theApp->Run(dlg.m_hWnd);
		
		}
		delete pNotifyCenter;
		delete theApp;
	}
	
	delete pComMgr;

	OleUninitialize();
	return 0;
}


