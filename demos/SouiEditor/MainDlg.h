// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <core/Swnd.h>
#include <control/SCmnCtrl.h>
#include "Dialog/DlgCreatePro.h"
#include "DesignerView.h"
#include "SMoveWnd.h"
#include "propgrid/SPropertyGrid.h"
//#include "STabCtrl2.h"
#include "SImageBtnEx.h"
#include "SListBoxDrop.h"
#include "ResManger.h"


extern SStringT g_CurDir;

#define WM_MSG_SHOWBOX (WM_USER+100)

class CMainDlg : public SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnSize(UINT nType, CSize size);
	void OnShowWindow(BOOL bShow, UINT nStatus);

	// 外部调用打开工程
	void OutOpenProject(SStringT filename);

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	void OpenProject(SStringT strFileName);
	void ReloadWorkspaceUIRes();
	void CloseProject();

	void DelayReloadLayout(STabCtrl* pTabHost);

	void OnBtnOpen(); //打开工程
	void OnBtnClose();
	void OnBtnReload();
	void OnBtnSaveAll();
	void OnBtnSaveLayout(); //保存布局
	void OnBtnNewDialog(); //新建Dialog
	void OnBtnNewInclude(); //新建Include
	void OnBtnWndLayout();
	void OnbtnPreview();
	void OnBtnZYGL();
	void OnBtnYSGL();
	void OnBtnAbout();

	// 直接修改XML文件后加载工程
	void LoadWorkSpace();

	bool OnTreeItemDbClick(EventArgs *pEvtBase);
	bool OnLbControlSelChanged(EventArgs *pEvtBase);

	bool OnDesinerTabSelChanged(EventArgs *pEvtBase);
	bool OnWorkspaceTabSelChanged(EventArgs *pEvtBase);
	bool OnWorkspaceXMLDbClick(EventArgs *pEvtBase);

	void OnScintillaSave(CScintillaWnd * pObj, int custom_msg, SStringT str);

	void RefreshWorkSpaceAllList();

	void RefreshSkinList();

	void RefreshColorList();

	void RefreshStringList();

	void RefreshStyleList();

protected:
	void OnLanguage(int nID);
	void OnLanguageBtnCN();
	void OnLanguageBtnJP();

	bool OnTreeproContextMenu(CPoint pt);
	LRESULT OnShowMsgBox(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled);
	
	void OnTimer(UINT_PTR timeID);

	bool Desiner_TabSelChanged(EventTabSelChanged * evt_sel);

	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"zh_cn", OnLanguageBtnCN)
		EVENT_NAME_COMMAND(L"jp", OnLanguageBtnJP)

		EVENT_NAME_COMMAND(L"toolbar_btn_Open", OnBtnOpen)
		EVENT_NAME_COMMAND(L"toolbar_btn_Close", OnBtnClose)
		EVENT_NAME_COMMAND(L"toolbar_btn_reload", OnBtnReload)
		
		EVENT_NAME_COMMAND(L"toolbar_btn_NewDialog", OnBtnNewDialog)
		EVENT_NAME_COMMAND(L"toolbar_btn_NewInclude", OnBtnNewInclude)
		EVENT_NAME_COMMAND(L"toolbar_btn_SaveAll", OnBtnSaveAll)
		EVENT_NAME_COMMAND(L"toolbar_btn_SaveLayout", OnBtnSaveLayout)
		EVENT_NAME_COMMAND(L"toolbar_btn_ZYGL", OnBtnZYGL)
		EVENT_NAME_COMMAND(L"toolbar_btn_YSGL", OnBtnYSGL)
		EVENT_NAME_COMMAND(L"toolbar_btn_YL", OnbtnPreview)
		EVENT_NAME_COMMAND(L"toolbar_btn_about", OnBtnAbout)

		EVENT_NAME_COMMAND(L"uidesigner_wnd_layout", OnBtnWndLayout)
		
		EVENT_ID_CONTEXTMENU(R.id.workspace_tree, OnTreeproContextMenu)

	EVENT_MAP_END()

		//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_MSG_SHOWBOX, OnShowMsgBox)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()


private:
	BOOL			m_bLayoutInited;

public:
	SButton* btn_new;
	STreeCtrl * m_treePro;			//工程Layout列表
	SListBox* m_lbWorkSpaceXml;		//工程中的XML文件列表

	SStringT  m_strOrigTitle;		//编辑器原有的默认标题

	SStringT  m_cmdWorkspaceFile;	//命令行要打开的工程文件

	SButton* btn_save;

	ResManger m_UIResFileMgr;	// 管理编辑的UI文件资源

	//SList<SDesignerView*> m_ViewList;
	SDesignerView *m_pDesignerView;

	SWindow *m_pLayoutContainer;

	//加载控件列表
	pugi::xml_document xmlDocCtrl;
	SMap<SStringT, pugi::xml_node> m_mapCtrlList;

	SStatic* m_staticAppTitle;		//软件标题
	SListBoxDrop* m_lbControl;		//控件列表框
	STreeCtrl* m_treeXmlStruct;		//当前编辑窗口的控件结构
	SStatic* m_textNodenum;			//当前编辑窗口的控件数量
	SStatic* m_textCurXmlFile;		//当前XML编辑窗口的文件名显示
	SStatic* m_textCtrlTypename;	//显示选择的控件类型

	STabPage*	m_pageEditor;		//编辑器的Page页
//	SPropertyGrid *m_pPropgrid;

	//SMap<SStringT, pugi::xml_node> m_mapCommonProperty; //属性通用样式列表   <"skin", xmlnode> <"pos", xmlNode>

	//SMap<SStringT, pugi::xml_node> m_mapProperty;  //控件属性列表 <"button",xmlnode>  <"checkbox",xmlnode>

	SStringT m_strUiresPath;	//uires.idx 的全路径
	SStringT m_strProPath;
	STabCtrl *m_pPropertyPage;
	SWindow* m_wndPropContainer;  //属性控件的容器

	SRichEdit* m_edtDesc; //属性描述

	SRealWnd *m_RealWndLayoutEdit;
	SRealWnd *m_RealWndXmlFile;

	STabCtrl *m_tabDesigner;
	STabCtrl *m_tabWorkspace;
	STabCtrl *m_tabRight;

	SListBox *m_lbAllStyle;
	SListView *m_lvAllString;
	SMCListView *m_mcAllSkin;
	SMCListView *m_mcAllColor;

	SImageBtnEx *m_btnPreview;
	SScrollView *m_scrView;
	BOOL m_bIsOpen;  //工程是否打开
};
