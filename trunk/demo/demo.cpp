// dui-demo.cpp : main source file
//

#include "stdafx.h"

#include <unknown/com-loader.hpp>
#include <helper/mybuffer.h>
#if defined(_DEBUG) && !defined(_WIN64)
// #include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"

#define SUPPORT_LANG    //打开SUPPORT_LANG时，演示多语言支持

#define RES_TYPE 0   //从文件中加载资源
// #define RES_TYPE 1   //从PE资源中加载UI资源
// #define RES_TYPE 2   //从zip包中加载资源

#include "../components/resprovider-zip/zipresprovider-param.h"

#ifdef _DEBUG
#define COM_IMGDECODER  _T("imgdecoder-wicd.dll")
#define COM_RENDER_GDI  _T("render-gdid.dll")
#define COM_RENDER_SKIA _T("render-skiad.dll")
#define COM_SCRIPT_LUA _T("scriptmodule-luad.dll")
#define COM_TRANSLATOR _T("translatord.dll")
#define COM_ZIPRESPROVIDER _T("resprovider-zipd.dll")
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define COM_IMGDECODER  _T("imgdecoder-wic.dll")
#define COM_RENDER_GDI  _T("render-gdi.dll")
#define COM_RENDER_SKIA _T("render-skia.dll")
#define COM_SCRIPT_LUA _T("scriptmodule-lua.dll")
#define COM_TRANSLATOR _T("translator.dll")
#define COM_ZIPRESPROVIDER _T("resprovider-zip.dll")
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
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
    SComLoader zipResLoader;
    
    //将程序的运行路径修改到demo所在的目录
    TCHAR szCurrentDir[MAX_PATH]={0};
    GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
    LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
    _tcscpy(lpInsertPos,_T("\\..\\demo"));
    SetCurrentDirectory(szCurrentDir);
    
    {

        CAutoRefPtr<IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<IRenderFactory> pRenderFactory;
        CAutoRefPtr<ITranslatorMgr> trans;
        CAutoRefPtr<IScriptModule> pScriptLua;
        
        BOOL bLoaded=FALSE;
        int nType=MessageBox(GetActiveWindow(),_T("选择渲染类型：\n[yes]: Skia\n[no]:GDI\n[cancel]:Quit"),_T("select a render"),MB_ICONQUESTION|MB_YESNOCANCEL);
        if(nType == IDCANCEL) return -1;
        bLoaded=renderLoader.CreateInstance(nType==IDYES?COM_RENDER_SKIA:COM_RENDER_GDI,(IObjRef**)&pRenderFactory);
        ASSERT(bLoaded);
        bLoaded=imgDecLoader.CreateInstance(COM_IMGDECODER,(IObjRef**)&pImgDecoderFactory);
        ASSERT(bLoaded);

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);

        bLoaded=transLoader.CreateInstance(COM_TRANSLATOR,(IObjRef**)&trans);
        ASSERT(bLoaded);
        
        SApplication *theApp=new SApplication(pRenderFactory,hInstance);

        CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
        CreateResProvider(RES_FILE,(IObjRef**)&pResProvider);
        if(!pResProvider->Init((LPARAM)_T("uires"),0))
        {
            ASSERT(0);
            return 1;
        }
#elif (RES_TYPE==1)
        CreateResProvider(RES_PE,(IObjRef**)&pResProvider);
        pResProvider->Init((WPARAM)hInstance,0);
#elif (RES_TYPE==2)
        bLoaded=zipResLoader.CreateInstance(COM_ZIPRESPROVIDER,(IObjRef**)&pResProvider);
        ASSERT(bLoaded);
        ZIPRES_PARAM param;
        param.type = ZIPRES_PARAM::ZIPFILE;
        param.pRenderFac = pRenderFactory;
        param.pszZipFile = _T("uires.zip");
        bLoaded = pResProvider->Init((WPARAM)&param,0);
        ASSERT(bLoaded);
#endif
        theApp->AddResProvider(pResProvider);

        if(trans)
        {
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
#ifdef DLL_SOUI
        bLoaded=scriptLoader.CreateInstance(COM_SCRIPT_LUA,(IObjRef**)&pScriptLua);
        ASSERT(bLoaded);
        if(pScriptLua)
        {
            theApp->SetScriptModule(pScriptLua);
            size_t sz=pResProvider->GetRawBufferSize(_T("script"),_T("lua_test"));
            if(sz)
            {
                CMyBuffer<char> lua;
                lua.Allocate(sz);
                pResProvider->GetRawBuffer(_T("script"),_T("lua_test"),lua,sz);
                pScriptLua->executeScriptBuffer(lua,sz);
            }
        }
#endif//DLL_SOUI

        SWkeLoader wkeLoader;
        if(wkeLoader.Init(_T("wke.dll")))        
        {
            theApp->RegisterWndFactory(TplSWindowFactory<SWkeWebkit>());//注册WKE浏览器
        }
        theApp->RegisterWndFactory(TplSWindowFactory<SGifPlayer>());//注册GIFPlayer
        theApp->RegisterSkinFactory(TplSkinFactory<SSkinGif>());//注册SkinGif
        SSkinGif::Gdiplus_Startup();
        
        
        HMODULE hSysResource=LoadLibrary(SYS_NAMED_RESOURCE);
        if(hSysResource)
        {
            CAutoRefPtr<IResProvider> sysSesProvider;
            CreateResProvider(RES_PE,(IObjRef**)&sysSesProvider);
            sysSesProvider->Init((WPARAM)hSysResource,0);
            theApp->LoadSystemNamedResource(sysSesProvider);
        }

        theApp->Init(_T("xml_init")); 

        // BLOCK: Run application
        {
            CMainDlg dlgMain;  
            dlgMain.Create(GetActiveWindow(),0,0,800,600);
            dlgMain.GetNative()->SendMessage(WM_INITDIALOG);
            dlgMain.ShowWindow(SW_SHOWNORMAL);
            dlgMain.SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
            nRet=theApp->Run(dlgMain.m_hWnd);
            //  		nRet = dlgMain.DoModal();  
        }
        
        delete theApp;
        SSkinGif::Gdiplus_Shutdown();

    }

    OleUninitialize();
    return nRet;
}
