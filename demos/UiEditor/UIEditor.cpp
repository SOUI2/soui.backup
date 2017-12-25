// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "SToolbox.h"
#include "SMoveWnd.h"

#include "../controls.extend/SRealWndHandler_Scintilla.h"

#include "../controls.extend/SChromeTabCtrl.h"
#include "../controls.extend/simagemaskwnd.h"
#include "../controls.extend/SFreeMoveWindow.h"
#include "../controls.extend/SVscrollbar.h"
#include "../controls.extend/SSkinNewScrollBar.h"
#include "../controls.extend/gif/SSkinGif.h"
#include "../controls.extend/gif/SSkinAPNG.h"
#include "../controls.extend/gif/SGifPlayer.h"
#include "../controls.extend/SScrollText.h"


#include "ExtendSkins.h"
#include "SButtonEx.h"
#include "ExtendCtrls.h"

//#include "SImagePlayer.h"
#include "SSkinMutiFrameImg.h"
#include "SText.h"
//#include "SImageEx.h"
//#include "SSplitBar.h"
#include "Stabctrl2.h"
//#include "SAnimImg.h"
#include "STurn3DView.h"
#include "SRotateWindow.h"

#include "SImageSwitcher.h"
#include "SListBoxDrop.h"
#include "helpapi.h"
#include "SDesignerRoot.h"
#include "SImgCanvas.h"

//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE 0
//#define RES_TYPE 0   //从文件中加载资源
// #define RES_TYPE 1  //从PE资源中加载UI资源

	
//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
ROBJ_IN_CPP


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    
    SComMgr *pComMgr = new SComMgr;
/*
#ifdef WIN64
	HMODULE hSci = LoadLibrary(_T("SciLexer64.dll"));
	if (!hSci) {
		MessageBox(GetActiveWindow(), _T("Load SciLexer64.dll failed! \nCopying third-part/SciLexer/bin/SciLexer64.dll to the running folder should resolve the problem!!"), _T("error"), MB_OK | MB_ICONSTOP);
		return -1;
}
#else
    HMODULE hSci = LoadLibrary(_T("SciLexer.dll"));
	if(!hSci){
		MessageBox(GetActiveWindow(),_T("Load SciLexer.dll failed! \nCopying third-part/SciLexer/bin/SciLexer.dll to the running folder should resolve the problem!!"),_T("error"),MB_OK|MB_ICONSTOP);
		return -1;
	}
#endif // W64
*/

#ifdef STATIC_BUILD_SCI  
	Scintilla_RegisterClasses(hInstance);
#else  
	HMODULE m_hSciLexerDll = NULL;
#ifdef _DEBUG
	m_hSciLexerDll = LoadLibrary(_T("Scintillad.dll"));
#else
	m_hSciLexerDll = LoadLibrary(_T("Scintilla.dll"));
#endif 
	if (NULL == m_hSciLexerDll)
	{
		MessageBox(GetActiveWindow(), _T("Load SciLexer.dll failed! \nCopying third-part/SciLexer/bin/SciLexer.dll to the running folder should resolve the problem!!"), _T("error"), MB_OK | MB_ICONSTOP);
		return FALSE;
	}
#endif 

    //将程序的运行路径修改到项目所在目录所在的目录
    TCHAR szCurrentDir[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));
    LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
    _tcscpy(lpInsertPos + 1, _T("..\\demos\\uieditor\\"));
    SetCurrentDirectory(szCurrentDir);
    {
        BOOL bLoaded=FALSE;
        CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;

		////if(nType == IDYES)
		//	bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
		////else
		////	bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);

        bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory); //SRenderFactory_GDI
        SASSERT_FMT(bLoaded,_T("load interface [render] failed!"));

        bLoaded=pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);//SImgDecoderFactory_GDIP
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);//SImgDecoderFactory_GDIP
        SApplication *theApp = new SApplication(pRenderFactory, hInstance);


		//向app注册自定义类
		theApp->RegisterWindowClass<SDesignerRoot>();

		theApp->RegisterWindowClass<SMoveWnd>();
		theApp->RegisterWindowClass<SFreeMoveWindow>();
		theApp->RegisterWindowClass<SToolbox>();



		theApp->RegisterWindowClass<STurn3dView>();

		// extened controls
		theApp->RegisterWindowClass<SChromeTabCtrl>();//注册ChromeTabCtrl
		theApp->RegisterWindowClass<SImageMaskWnd>();//注册SImageMaskWnd
		theApp->RegisterWindowClass<SButtonEx>();
		theApp->RegisterWindowClass<SWindowEx>();

		//theApp->RegisterWindowClass<SImagePlayer>();
		theApp->RegisterWindowClass<SText>();
		//theApp->RegisterWindowClass<SImageEx>();
		//theApp->RegisterWindowClass<SSplitBar>();
		theApp->RegisterWindowClass<SImageSwitcher>();

		theApp->RegisterWindowClass<STabPage2>();//注册STabPage2
		theApp->RegisterWindowClass<STabCtrl2>();//注册STabCtrl2
		//theApp->RegisterWindowClass<SAnimImg>();//注册SAnimImg
		theApp->RegisterWindowClass<SGifPlayer>();//注册gif

		theApp->RegisterWindowClass<SListBoxDrop>();
		theApp->RegisterWindowClass<CDropWnd>();

        theApp->RegisterWindowClass<SPropertyGrid>();//注册属性表控件
        theApp->RegisterWindowClass<SScrollText>();
        theApp->RegisterWindowClass<SRotateWindow>();
		theApp->RegisterWindowClass<SImgCanvas>();

		//extened skins
		theApp->RegisterSkinClass<SColorMask>();
		theApp->RegisterSkinClass<SSkinMutiFrameImg>();
		theApp->RegisterSkinClass<SSkinVScrollbar>();
		theApp->RegisterSkinClass<SSkinNewScrollbar>();
		theApp->RegisterSkinClass<SSkinGif>();
		theApp->RegisterSkinClass<SSkinAPNG>();
        {
			#ifdef _DEBUG
				HMODULE hSysRes = LoadLibrary(_T("soui-sys-resourced.dll"));		

			#else
				HMODULE hSysRes = LoadLibrary(_T("soui-sys-resource.dll"));		
			#endif

            CAutoRefPtr<IResProvider> sysResProvider;
            CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
            sysResProvider->Init((WPARAM)hSysRes, 0);
            theApp->LoadSystemNamedResource(sysResProvider);
			FreeLibrary(hSysRes);
        }

		

        CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
        CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
        if (!pResProvider->Init((LPARAM)_T("uires"), 0))
        {
            SASSERT(0);
            return 1;
        }
#else 
        CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
        pResProvider->Init((WPARAM)hInstance, 0);
#endif
		//如果需要在代码中使用R::id::namedid这种方式来使用控件必须要这一行代码：2016年2月2日，R::id::namedXmlID是由uiresbuilder 增加-h .\res\resource.h idtable 这3个参数后生成的。
		//theApp->InitXmlNamedID(namedXmlID,ARRAYSIZE(namedXmlID),TRUE);
		//theApp->Init(_T("XML_INIT"));  //这一句不在需要了 在AddResProvider时自动执行初始化
		

		theApp->AddResProvider(pResProvider, L"uidef:UIDESIGNER_XML_INIT");   // theApp->AddResProvider(pResProvider, L"uidef:xml_init");



		//设置真窗口处理接口
		SRealWndHandler_Scintilla * pRealWndHandler = new SRealWndHandler_Scintilla();
		theApp->SetRealWndHandler(pRealWndHandler);
		pRealWndHandler->Release();

        // BLOCK: Run application
        {			
            CMainDlg dlgMain;
            dlgMain.Create(GetActiveWindow());
            dlgMain.SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);	
			SStringT uideffile = lpstrCmdLine;		
			if (!uideffile.IsEmpty())
			{
				uideffile.Trim(_T('\"'));
				uideffile += _T("uires.idx");
#ifdef _DEBUG
				SMessageBox(NULL, uideffile, NULL, MB_OK);
#endif
				if(FileIsExist(uideffile))
					dlgMain.OpenProject(uideffile);
			}
            nRet = theApp->Run(dlgMain.m_hWnd);
        }

        delete theApp;
    }
//	FreeLibrary(hSci);
    
    delete pComMgr;
    
#ifdef STATIC_BUILD_SCI  
	Scintilla_ReleaseResources();
#else  
	if (m_hSciLexerDll != NULL)
	{
		::FreeLibrary(m_hSciLexerDll);
	}
#endif


    OleUninitialize();
    return nRet;
}
