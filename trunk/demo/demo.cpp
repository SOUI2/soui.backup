// dui-demo.cpp : main source file
//

#include "stdafx.h"

#include <unknown/com-loader.hpp>

#if defined(_DEBUG) && !defined(_WIN64)
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#define RENDER_GDI      //打开RENDER_GDI时使用render-gdi模块来渲染，否则采用render-skia渲染

#define SUPPORT_LANG    //打开SUPPORT_LANG时，演示多语言支持

#define RES_TYPE 0   //从文件中加载资源
// #define RES_TYPE 1   //从PE资源中加载UI资源
// #define RES_TYPE 2   //从zip包中加载资源

#if RES_TYPE==2
    #include "../components/resprovider-zip/SResProviderZip.h"
    #ifdef _DEBUG
    #pragma comment(lib,"resprovider-zip_d.lib")
    #else
    #pragma comment(lib,"resprovider-zip.lib")
    #endif
#endif


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    ASSERT(SUCCEEDED(hRes));
    
    int nRet = 0; 

    SComLoader imgDecLoader;
    SComLoader renderLoader;
    SComLoader transLoader;
    SComLoader scriptLoader;
        
    //将程序的运行路径修改到demo所在的目录
    TCHAR szCurrentDir[MAX_PATH]={0};
    GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
    LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
    _tcscpy(lpInsertPos,_T("\\..\\..\\demo"));
    SetCurrentDirectory(szCurrentDir);
    
    {

        CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
#ifdef _DEBUG
        imgDecLoader.CreateInstance(_T("imgdecoder-wicd.dll"),(IObjRef**)&pImgDecoderFactory);
#ifdef RENDER_GDI
        renderLoader.CreateInstance(_T("render-gdid.dll"),(IObjRef**)&pRenderFactory);
#else
        renderLoader.CreateInstance(_T("render-skiad.dll"),(IObjRef**)&pRenderFactory);
#endif
#else
        imgDecLoader.CreateInstance(_T("imgdecoder-wic.dll"),(IObjRef**)&pImgDecoderFactory);
#ifdef RENDER_GDI
        renderLoader.CreateInstance(_T("render-gdi.dll"),(IObjRef**)&pRenderFactory);
#else
        renderLoader.CreateInstance(_T("render-skia.dll"),(IObjRef**)&pRenderFactory);
#endif
#endif

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

        SApplication *theApp=new SApplication(pRenderFactory,hInstance);

#ifdef SUPPORT_LANG
        CAutoRefPtr<ITranslator> trans;
#ifdef _DEBUG
        transLoader.CreateInstance(_T("translatord.dll"),(IObjRef**)&trans);
#else
        transLoader.CreateInstance(_T("translator.dll"),(IObjRef**)&trans);
#endif//_DEBUG
        if(trans)
        {
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
        CAutoRefPtr<IScriptModule> pScriptLua;
#ifdef _DEBUG
        scriptLoader.CreateInstance(_T("scriptmodule-luad.dll"),(IObjRef**)&pScriptLua);
#else
        scriptLoader.CreateInstance(_T("scriptmodule-lua.dll"),(IObjRef**)&pScriptLua);
#endif//_DEBUG
        if(pScriptLua)
        {
            pScriptLua->executeScriptFile("lua/test.lua");
            theApp->SetScriptModule(pScriptLua);
        }
#endif//SUPPORT_LUA

#if (RES_TYPE == 0)
        SResProviderFiles *pResProvider=new SResProviderFiles;
        if(!pResProvider->Init(_T("skin")))
        {
            ASSERT(0);
            return 1;
        }
#elif (RES_TYPE==1)
        SResProviderPE *pResProvider = new SResProviderPE(hInstance);
#elif (RES_TYPE==2)
        SResProviderZip *pResProvider = new SResProviderZip(pRenderFactory);
        pResProvider->Init(_T("skin.zip"));
#endif

#ifdef SUPPORT_WKE
        SWkeWebkit::WkeWebkit_Init();
        theApp->RegisterWndFactory(TplSWindowFactory<SWkeWebkit>());//注册WKE浏览器
#endif//SUPPORT_WKE
        theApp->RegisterWndFactory(TplSWindowFactory<SGifPlayer>());//注册GIFPlayer
        theApp->RegisterSkinFactory(TplSkinFactory<SSkinGif>());//注册SkinGif
        SSkinGif::Gdiplus_Startup();
        
        theApp->AddResProvider(pResProvider);

        theApp->Init(_T("IDR_DUI_INIT")); 
        theApp->SetMsgBoxTemplate(_T("IDR_DUI_MSGBOX"));

        // BLOCK: Run application
        {
            CMainDlg dlgMain;  
            dlgMain.Create(GetActiveWindow(),0,0,800,600);
            dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            nRet=theApp->Run(dlgMain.m_hWnd);
            //  		nRet = dlgMain.DoModal();  
        }

        delete theApp;
        delete pResProvider;
        SSkinGif::Gdiplus_Shutdown();
#ifdef SUPPORT_WKE
        SWkeWebkit::WkeWebkit_Shutdown();
#endif//SUPPORT_WKE

    }

    OleUninitialize();
    return nRet;
}
