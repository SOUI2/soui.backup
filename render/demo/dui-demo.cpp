// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "menuwndhook.h"
#include "DuiSystem.h" 
#include "DuiDefaultLogger.h"

#include "DuiSkinGif.h"	//应用层定义的皮肤对象

//从ZIP文件加载皮肤模块
#if !defined(_WIN64)
#include "zipskin/DuiResProviderZip.h"
#endif

#ifdef _DEBUG
#include <vld.h>//使用Vitural Leaker Detector来检测内存泄漏，可以从http://vld.codeplex.com/ 下载
#endif

#include "MainDlg.h"
#include "ResModeSelDlg.h"

//演示如何使用引擎外部实现的DUI控件
class  CDuiListBox2 :public CDuiListBoxEx
{
public:
	SOUI_CLASS_NAME(CDuiListBox2, "listboxex")

};
//派生一个只有纵向滚动条皮肤
class CDuiVScrollBarSkin : public SOUI::SSkinScrollbar
{
	SOUI_CLASS_NAME(CDuiVScrollBarSkin, "vscrollbar")

public:

	CDuiVScrollBarSkin():m_nStates(3)
	{

	}

	virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha=0xff)
	{
		if(!m_pImg) return;
		int nSbCode=LOWORD(dwState);
		int nState=LOBYTE(HIWORD(dwState));
		BOOL bVertical=HIBYTE(HIWORD(dwState));
		if(bVertical)
		{
			CRect rcMargin(0,0,0,0);
			rcMargin.top=m_nMargin,rcMargin.bottom=m_nMargin;
			CRect rcSour=GetPartRect(nSbCode,nState,bVertical);
			FrameDraw(dc, m_pImg , rcSour,rcDraw,rcMargin, CLR_INVALID, m_uDrawPart,m_bTile,byAlpha);
		}
	}

	//指示滚动条皮肤是否支持显示上下箭头
	virtual BOOL HasArrow(){return FALSE;}
	virtual int GetIdealSize(){
		if(!m_pImg) return 0;
		return m_pImg->GetWidth()/(1+m_nStates);//图片分成4 or 5 部分横向排列，第一块是轨道，2,3,4,5分别对应滑块的正常，浮动，下压, Disable状态
	}
	SOUI_ATTRS_BEGIN()
		ATTR_INT("states",m_nStates,FALSE)
		SOUI_ATTRS_END()
protected:
	//返回源指定部分在原位图上的位置。
	CRect GetPartRect(int nSbCode, int nState,BOOL bVertical)
	{
		CRect rc;
		if(!bVertical || nSbCode==SB_LINEDOWN || nSbCode==SB_LINEUP) return rc;

		rc.right=GetImage()->GetWidth()/(1+m_nStates);
		rc.bottom=GetImage()->GetHeight();

		if(nSbCode == SB_PAGEUP || nSbCode == SB_PAGEDOWN)
		{
			return rc;
		}
		rc.OffsetRect(rc.Width()*(1+nState),0);
		return rc;
	}

	int m_nStates;
};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpstrCmdLine*/, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	DUIASSERT(SUCCEEDED(hRes));

	pugi::xml_node node;
	node.set_name("abc");
#if !defined(_WIN64)
//#if 0
	//<--资源类型选择 
	DuiSystem * pDuiModeSel = new DuiSystem(hInstance);
	DuiSystem::getSingletonPtr();

	DuiResProviderZip *pResModeSel= new DuiResProviderZip;
	pResModeSel->Init(hInstance,MAKEINTRESOURCE(IDR_ZIP_MODESEL),_T("ZIP"));
	pDuiModeSel->SetDefResProvider(pResModeSel);

	pDuiModeSel->Init(_T("xml_init"));

	int nMode=-1;
 	{
 		CResModeSelDlg dlgModeSel;  
 		if(IDOK==dlgModeSel.DoModal())
 		{
 			nMode=dlgModeSel.m_nMode;
 		}
 	}
//	delete pResModeSel;
	delete pDuiModeSel->GetDefResProvider();
	delete pDuiModeSel;

	if(nMode==-1) return -1;
#else
	int nMode=0;	//64位时直接从文件加载资源
#endif
	//资源类型选择完成 -->
	
	DuiSystem *pDuiSystem=new DuiSystem(hInstance);

	//生成控件类厂并注册到系统
	pDuiSystem->RegisterWndFactory(TplDuiWindowFactory<CDuiListBox2>(),true);

	//生成皮肤类厂并注册到系统
	pDuiSystem->RegisterSkinFactory(TplSkinFactory<CDuiVScrollBarSkin>());
	pDuiSystem->RegisterSkinFactory(TplSkinFactory<CDuiSkinGif>());


	TCHAR szCurrentDir[MAX_PATH]; memset( szCurrentDir, 0, sizeof(szCurrentDir) );
	GetModuleFileName( NULL, szCurrentDir, sizeof(szCurrentDir) );
	LPTSTR lpInsertPos = _tcsrchr( szCurrentDir, _T('\\') );
	*lpInsertPos = _T('\0');   

	DefaultLogger loger;
	loger.setLogFilename(CDuiStringT(szCurrentDir)+_T("\\dui-demo.log")); 
	pDuiSystem->SetLogger(&loger);

	//根据选择的资源加载模式生成resprovider 
	switch(nMode)
	{
	case 0://load from files
		{
			_tcscat( szCurrentDir, _T("\\..\\demo") );
			DuiResProviderFiles *pResFiles=new DuiResProviderFiles;
			if(!pResFiles->Init(szCurrentDir))
			{
				DUIASSERT(0);
				return 1;
			}
			pDuiSystem->SetDefResProvider(pResFiles);
			pDuiSystem->logEvent(_T("load resource from files"));
		}
		break;
	case 1://load from PE
		{
			pDuiSystem->SetDefResProvider(new DuiResProviderPE(hInstance));
			pDuiSystem->logEvent(_T("load resource from exe"));
		}
		break;
	default://load form ZIP
		{
#if !defined(_WIN64)
			//从ZIP文件加载皮肤
			DuiResProviderZip *zipSkin=new DuiResProviderZip;
			CDuiStringT strZip=CDuiStringT(szCurrentDir)+_T("\\def_skin.zip");
			if(!zipSkin->Init(strZip))
			{ 
				DUIASSERT(0);
				return 1;
			}
			pDuiSystem->SetDefResProvider(zipSkin); 
			pDuiSystem->logEvent(_T("load resource from zip"));
#else
			return -1;
#endif;
		}
		break;
	}

	BOOL bOK=pDuiSystem->Init(_T("IDR_DUI_INIT")); //初始化DUI系统,原来的系统初始化方式依然可以使用。
	pDuiSystem->SetMsgBoxTemplate(_T("IDR_DUI_MSGBOX"));

#ifdef LUA_TEST
	CLuaScriptModule scriptLua;
	scriptLua.executeScriptFile("..\\dui_demo\\lua\\test.lua");
	pDuiSystem->SetScriptModule(&scriptLua);
#endif

	CMenuWndHook::InstallHook(hInstance,"skin_menuborder");
	int nRet = 0; 
	// BLOCK: Run application
	{
		pDuiSystem->logEvent(_T("demo started"));
		CMainDlg dlgMain;  
		nRet = dlgMain.DoModal();  
		pDuiSystem->logEvent(_T("demo end"));
	}


	delete pDuiSystem->GetDefResProvider();
	//释放资源 
	CMenuWndHook::UnInstallHook();

	delete pDuiSystem;

	OleUninitialize();
	return nRet;
}
