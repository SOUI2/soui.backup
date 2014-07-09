// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "SApp.h" 

#ifdef _DEBUG
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#define RENDER_GDI      //打开RENDER_GDI时使用render-gdi模块来渲染，否则采用render-skia渲染

#define SUPPORT_LANG    //打开SUPPORT_LANG时，演示多语言支持

#define RES_USINGFILE   //打开RES_USINGFILE从文件中加载资源，否则从PE资源中加载UI资源

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
    
#ifdef SUPPORT_LANG
    typedef BOOL (*funCreateTranslator)(ITranslator **);
    CAutoRefPtr<ITranslator> trans;
    #ifdef _DEBUG
    HMODULE hTrans=LoadLibrary(_T("translator_d.dll"));
    #else
    HMODULE hTrans=LoadLibrary(_T("translator.dll"));
    #endif//_DEBUG
    if(hTrans)
    {
        funCreateTranslator funCT = (funCreateTranslator)GetProcAddress(hTrans,"CreateTranslator");
        funCT(&trans);
        theApp->SetTranslator(trans);
        pugi::xml_document xmlLang;
        if(xmlLang.load_file(L"../demo/translation files/lang_cn.xml"))
        {
            CAutoRefPtr<ILang> langCN;
            trans->CreateLang(&langCN);
            langCN->Load(&xmlLang.child(L"language"),1);//1=LD_XML
            trans->InstallLang(langCN);
        }
    }
#endif//SUPPORT_LANG

#ifdef SUPPORT_LUA
    typedef BOOL (*funCreateScript)(IScriptModule **);
    CAutoRefPtr<IScriptModule> pScriptLua;
#ifdef _DEBUG
    HMODULE hScript=LoadLibrary(_T("scriptmodule-lua_d.dll"));
#else
    HMODULE hScript=LoadLibrary(_T("scriptmodule-lua.dll"));
#endif//_DEBUG
    if(hScript)
    {
        funCreateScript funCS = (funCreateScript)GetProcAddress(hScript,"CreateScriptModule_Lua");
        funCS(&pScriptLua);
        theApp->SetScriptModule(pScriptLua);
    }
#endif//SUPPORT_LUA

#ifdef RES_USINGFILE
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
//         dlgMain.Create(GetActiveWindow(),0,0,800,600);
//         dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
//         dlgMain.ShowWindow(SW_SHOWNORMAL);
//         nRet=theApp->Run(dlgMain.m_hWnd);
 		nRet = dlgMain.DoModal();  
	}



	delete theApp;
    
    delete pResProvider;
    
    pRenderFactory=NULL;
    pImgDecoderFactory=NULL;
    
	OleUninitialize();
	return nRet;
}
