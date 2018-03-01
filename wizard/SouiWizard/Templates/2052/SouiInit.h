//用户控件/皮肤/布局/插值算法注册类
class SUserObjectDefaultRegister : public TObjRefImpl<ISystemObjectRegister>
{
public:
	void RegisterWindows(SObjectFactoryMgr *objFactory);
	//void RegisterSkins(SObjectFactoryMgr *objFactory);
	//void RegisterLayouts(SObjectFactoryMgr *objFactory);
	//void RegisterInterpolator(SObjectFactoryMgr *objFactory);
};
//Soui 加载器
class CSouiLoader
{
	SApplication *theApp;
	SComMgr *pComMgr;
public:
	//通过过传入一个ISystemObjectRegister对像来注册用户控件，其余参数和SApplication的参数一致
	CSouiLoader(HINSTANCE hInst,ISystemObjectRegister *pUserObjRegister=new SUserObjectDefaultRegister(), LPCTSTR pszHostClassName = _T("SOUIHOST"), ISystemObjectRegister *pSysObjRegister = new SObjectDefaultRegister()) 
		:theApp(NULL),pComMgr(NULL)
	{		
		pComMgr = new SComMgr;
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
[!if RADIO_RANDER_GDI]
		BOOL bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
[!else]
		BOOL bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
[!endif]
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));
		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		theApp = new SApplication(pRenderFactory, hInst, pszHostClassName, pSysObjRegister);

		[!if CHECKBOX_USE_LUA]
		//加载LUA脚本模块。
#if (defined(DLL_CORE) || defined(LIB_ALL)) && !defined(_WIN64)
		//加载LUA脚本模块，注意，脚本模块只有在SOUI内核是以DLL方式编译时才能使用。
		CAutoRefPtr<SOUI::IScriptFactory> pScriptLuaFactory;
		bLoaded = pComMgr->CreateScrpit_Lua((IObjRef**)&pScriptLuaFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		theApp->SetScriptFactory(pScriptLuaFactory);
#endif//DLL_CORE
[!endif]

[!if CHECKBOX_TRANSLATOR_SUPPORT]
		//加载多语言翻译模块。
		CAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = pComMgr->CreateTranslator((IObjRef**)&trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//加载语言翻译包
			theApp->SetTranslator(trans);
			pugi::xml_document xmlLang;
			if (theApp->LoadXmlDocment(xmlLang, _T("lang_cn"), _T("translator")))
			{
				CAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				langCN->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
[!endif]

		//注册用户自定义的东西
		pUserObjRegister->RegisterLayouts(theApp);
		pUserObjRegister->RegisterSkins(theApp);
		pUserObjRegister->RegisterWindows(theApp);
		pUserObjRegister->RegisterInterpolator(theApp);
		pUserObjRegister->Release();
	}
	~CSouiLoader()
	{
		if (theApp)
			delete theApp;
		if (pComMgr)
			delete pComMgr;
	}
	SApplication *GetApp()
	{
		SASSERT(theApp);
		return theApp;
	}
	SComMgr *GetComMgr()
	{
		SASSERT(pComMgr);
		return pComMgr;
	}
};
//初使化资源加载路径
void InitDir(TCHAR *Path=NULL);

void InitSystemRes(SApplication *theApp, SComMgr *pComMgr);

void InitUserRes(SApplication * theApp, SComMgr *pComMgr);

template<class T>
int Run(SApplication *theApp)
{
	T dlgMain;
	dlgMain.Create(GetActiveWindow());
	dlgMain.SendMessage(WM_INITDIALOG);
	dlgMain.CenterWindow(dlgMain.m_hWnd);
[!if CHECKBOX_MAXIMIZED]
	dlgMain.ShowWindow(SW_SHOWMAXIMIZED);
[!else]
	dlgMain.ShowWindow(SW_SHOWNORMAL);
[!endif]
	return theApp->Run(dlgMain.m_hWnd);
}

