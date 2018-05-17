// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
#include "SImageBtnEx.h"
#include "SouiRealWndHandler.h"
#include "SRotateWindow.h"
#include "SImgCanvas.h"
#include "CmdLine.h"
#include <helper/AppDir.h>

//扩展控件
#include "stabctrl2.h"
#include "SButtonEx.h"
#include "SSkinMutiFrameImg.h"
#include "SChromeTabCtrl.h"
#include "simagemaskwnd.h"
#include "SFreeMoveWindow.h"
#include "SVscrollbar.h"
#include "SSkinNewScrollBar.h"
#include "gif/SSkinGif.h"
#include "gif/SSkinAPNG.h"
#include "gif/SGifPlayer.h"
#include "SScrollText.h"
#include "ExtendSkins.h"
#include "ExtendCtrls.h"
#include "SRatingBar.h"
#include "SListCtrlEx.h"
#include "SIPAddressCtrl.h"
#include "STurn3DView.h"
#include "SRadioBox2.h"
#include "SMcListViewEx/SHeaderCtrlEx.h"
#include "SDemoSkin.h"


//从PE文件加载，注意从文件加载路径位置
#ifdef _DEBUG
	#define RES_TYPE 0   //从文件中加载资源
#else
	#define RES_TYPE 1  //从PE资源中加载UI资源
#endif

#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif
	
//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
ROBJ_IN_CPP

SStringT g_CurDir;
void RegisterExtendControl(SApplication *theApp);
BOOL g_bHookCreateWnd = FALSE;

class SMyApp : public SApplication 
{
public:

	SMyApp::SMyApp(IRenderFactory *pRendFactory, HINSTANCE hInst, LPCTSTR pszHostClassName = _T("SOUIHOST"))
		: SApplication(pRendFactory, hInst, pszHostClassName)
	{
	}

	virtual SWindow * CreateWindowByName(LPCWSTR pszWndClass) const
	{
		if (!g_bHookCreateWnd)
			return (SWindow*)CreateObject(SObjectInfo(pszWndClass, Window));
		else
		{
			SStringT wndClassname = pszWndClass;
			if (wndClassname.CompareNoCase(_T("realwnd")) == 0)
				wndClassname = _T("ui_window");

			SWindow *pChild = (SWindow*)CreateObject(SObjectInfo(wndClassname, Window));
			if (!pChild)
			{
				if (wndClassname.CompareNoCase(L"template") != 0)
					pChild = (SWindow*)CreateObject(SObjectInfo(_T("ui_window"), Window));
			}

			if (pChild)
			{
				pChild->SetUserData((ULONG_PTR)(GetUIElmIndex()));
			}
			return pChild;
		}
	}
};


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    
    SComMgr *pComMgr = new SComMgr;

    //将程序的运行路径修改到项目所在目录所在的目录
    TCHAR szCurrentDir[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));
    LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
	lpInsertPos[1] = 0;
	g_CurDir = szCurrentDir;
    _tcscpy(lpInsertPos + 1, _T("..\\SouiEditor"));

    SetCurrentDirectory(szCurrentDir);
    {
/*
#ifdef WIN64
		HMODULE hSci = LoadLibrary(g_CurDir + _T("SciLexer64.dll"));
		if (!hSci) {
			MessageBox(GetActiveWindow(), _T("Load SciLexer64.dll failed! \nCopying third-part/SciLexer/bin/SciLexer64.dll to the running folder should resolve the problem!!"), _T("error"), MB_OK | MB_ICONSTOP);
			return -1;
		}
#else
		HMODULE hSci = LoadLibrary(g_CurDir + _T("SciLexer.dll"));
		if (!hSci) {
			MessageBox(GetActiveWindow(), _T("Load SciLexer.dll failed! \nCopying third-part/SciLexer/bin/SciLexer.dll to the running folder should resolve the problem!!"), _T("error"), MB_OK | MB_ICONSTOP);
			return -1;
		}
#endif // W64
*/
		Scintilla_RegisterClasses(hInstance);

        BOOL bLoaded=FALSE;
        CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;

		bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
        //bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [render] failed!"));
        bLoaded=pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		SMyApp *theApp = new SMyApp(pRenderFactory, hInstance);

		theApp->RegisterWindowClass<SDesignerRoot>();
		theApp->RegisterWindowClass<SUIWindow>();

		theApp->RegisterWindowClass<SMoveWnd>();
		theApp->RegisterWindowClass<SImageBtnEx>();

		theApp->RegisterWindowClass<SListBoxDrop>();
		theApp->RegisterWindowClass<CDropWnd>();

		theApp->RegisterWindowClass<SPropertyGrid>();//注册属性表控件
		theApp->RegisterWindowClass<SScrollText>();
		theApp->RegisterWindowClass<SRotateWindow>();
		theApp->RegisterWindowClass<SImgCanvas>();

		// 注册扩展控件
		RegisterExtendControl(theApp);

        //从DLL加载系统资源
        HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
        if (hModSysResource)
        {
            CAutoRefPtr<IResProvider> sysResProvider;
            CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
            sysResProvider->Init((WPARAM)hModSysResource, 0);
            theApp->LoadSystemNamedResource(sysResProvider);
            FreeLibrary(hModSysResource);
        }else
        {
            SASSERT(0);
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

		theApp->InitXmlNamedID(namedXmlID, ARRAYSIZE(namedXmlID),TRUE);
        theApp->AddResProvider(pResProvider);

		//读取自定义消息框布局
		int ret = 0;
		pugi::xml_document xmlDoc;
		if (!theApp->LoadXmlDocment(xmlDoc, _T("xml_messagebox"), _T("LAYOUT")) || !SetMsgTemplate(xmlDoc.child(L"SOUI")))
			ret = -1;
		if (ret == -1)
			SMessageBox(NULL, _T("【消息框皮肤】读取失败"), _T("提示"), 0);

		//设置真窗口处理接口
		CSouiRealWndHandler * pRealWndHandler = new CSouiRealWndHandler();
		theApp->SetRealWndHandler(pRealWndHandler);
		pRealWndHandler->Release();


        //加载多语言翻译模块。
        CAutoRefPtr<ITranslatorMgr> trans;
        bLoaded=pComMgr->CreateTranslator((IObjRef**)&trans);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("translator"));
        if(trans)
        {	//加载语言翻译包
            theApp->SetTranslator(trans);
            pugi::xml_document xmlLang;
            if(theApp->LoadXmlDocment(xmlLang,_T("lang_cn"),_T("translator")))
            {
                CAutoRefPtr<ITranslator> langCN;
                trans->CreateTranslator(&langCN);
                langCN->Load(&xmlLang.child(L"language"),1);//1=LD_XML
                trans->InstallTranslator(langCN);
            }
        }
        
        // BLOCK: Run application
        {
			CCmdLine cmdLine(GetCommandLine());
            CMainDlg dlgMain;
			if (cmdLine.GetParamCount() > 1)
				dlgMain.m_cmdWorkspaceFile = cmdLine.GetParam(1);
            dlgMain.Create(GetActiveWindow(), WS_CLIPCHILDREN | WS_TABSTOP | WS_POPUP, (DWORD)0);
            dlgMain.SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet = theApp->Run(dlgMain.m_hWnd);
        }

        delete theApp;
    }
    
    delete pComMgr;
    
	Scintilla_ReleaseResources();

    OleUninitialize();
    return nRet;
}

void RegisterExtendControl(SApplication *theApp)
{
	theApp->RegisterWindowClass<STabPage2>();//注册STabPage2
	theApp->RegisterWindowClass<STabCtrl2>();//注册STabCtrl2
	theApp->RegisterWindowClass<SHeaderCtrlEx>();//注册STabCtrl2
	theApp->RegisterWindowClass<SChromeTabCtrl>();//注册ChromeTabCtrl
	theApp->RegisterWindowClass<SImageMaskWnd>();//注册SImageMaskWnd
	theApp->RegisterWindowClass<SButtonEx>();
	theApp->RegisterWindowClass<SWindowEx>();
	theApp->RegisterWindowClass<SGifPlayer>();//注册gif
	theApp->RegisterWindowClass<SRatingBar>();
	theApp->RegisterWindowClass<SListCtrlEx>();
	theApp->RegisterWindowClass<SIPAddressCtrl>();
	theApp->RegisterWindowClass<STurn3dView>();
	theApp->RegisterWindowClass<SRadioBox2>();

	
	//extened skins
	theApp->RegisterSkinClass<SColorMask>();
	theApp->RegisterSkinClass<SSkinMutiFrameImg>();
	theApp->RegisterSkinClass<SSkinVScrollbar>();
	theApp->RegisterSkinClass<SSkinNewScrollbar>();
	theApp->RegisterSkinClass<SSkinGif>();
	theApp->RegisterSkinClass<SSkinAPNG>();
	theApp->RegisterSkinClass<SDemoSkin>();
}