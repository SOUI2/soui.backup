
// mfc.demo.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "mfc.demo.h"
#include "mfc.demoDlg.h"
#include "SouiRealWndHandler.h"
using namespace SOUI;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define COM_IMGDECODER  _T("imgdecoder-wicd.dll")
#define COM_RENDER_GDI  _T("render-gdid.dll")
#else
#define COM_IMGDECODER  _T("imgdecoder-wic.dll")
#define COM_RENDER_GDI  _T("render-gdi.dll")
#endif


#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif


// CmfcdemoApp 构造

CmfcdemoApp::CmfcdemoApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CmfcdemoApp 对象

CmfcdemoApp theApp;


// CmfcdemoApp 初始化

BOOL CmfcdemoApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	HRESULT hRes = CoInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

    {//这一个括号很重要，保证里面声明的局部对象在CoUninitialize()之前释放

        SComLoader imgDecLoader;
        SComLoader renderLoader;
        CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<IRenderFactory> pRenderFactory;
        imgDecLoader.CreateInstance(COM_IMGDECODER,(IObjRef**)&pImgDecoderFactory);
        renderLoader.CreateInstance(COM_RENDER_GDI,(IObjRef**)&pRenderFactory);

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

        SApplication *pSouiApp=new SApplication(pRenderFactory,theApp.m_hInstance);

        //加载系统资源
        HMODULE hSysResource=LoadLibrary(SYS_NAMED_RESOURCE);
        if(hSysResource)
        {
            CAutoRefPtr<IResProvider> sysSesProvider;
            CreateResProvider(RES_PE,(IObjRef**)&sysSesProvider);
            sysSesProvider->Init((WPARAM)hSysResource,0);
            pSouiApp->LoadSystemNamedResource(sysSesProvider);
        }

        CAutoRefPtr<IResProvider>   pResProvider;
        CreateResProvider(RES_PE,(IObjRef**)&pResProvider);
        pResProvider->Init((WPARAM)theApp.m_hInstance,0);

        pSouiApp->AddResProvider(pResProvider);    //从资源加载皮肤

        //设置真窗口处理接口
        CSouiRealWndHandler * pRealWndHandler = new CSouiRealWndHandler();
        pSouiApp->SetRealWndHandler(pRealWndHandler);
        pRealWndHandler->Release();

	    CmfcdemoDlg dlg;
	    m_pMainWnd = &dlg;
	    dlg.DoModal();
    	
        delete pSouiApp;

    }

	CoUninitialize();

	return FALSE;
}
