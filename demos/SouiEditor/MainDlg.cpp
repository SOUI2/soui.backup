// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "helpapi.h"
#include "CDebug.h"
#include "Dialog/DlgNewLayout.h"
#include "Dialog/DlgAbout.h"
#include "SysdataMgr.h"
#include <vector>
#include <helper/SMenu.h>
#include <helper/SMenuEx.h>
#include "SImgCanvas.h"
#include "DragDownMgr.h"

#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif
	

#define TIMERID_RELOAD_LAYOUT  100

#define UIRES_FILE	L"uires.idx"
//////////////////////////////////////////////////////////////////////////
CMainDlg* g_pMainDlg = NULL;
CSysDataMgr g_SysDataMgr;
//////////////////////////////////////////////////////////////////////////
class CSkinMcAdapter : public SMcAdapterBase
{
#define NUMSCALE 100000
public:
	struct SKININFO
	{
		SStringT skin_name;
		SStringT skin_src;
	};

	SArray<SKININFO> m_itemInfo;

public:
	CSkinMcAdapter()
	{
	}

	void Add(SStringT skinname, SStringT src)
	{
		SKININFO info;
		info.skin_name = skinname;
		info.skin_src = src;

		m_itemInfo.Add(info);
	}

	virtual int getCount()
	{
		return m_itemInfo.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		SKININFO *psi = m_itemInfo.GetData() + position%m_itemInfo.GetCount();
		pItem->FindChildByName(L"txt_name")->SetWindowText(S_CW2T(psi->skin_name));
		SImageWnd* imgView = (SImageWnd *)(pItem->FindChildByName(L"img_icon"));
		IBitmap *pImg = SResLoadFromFile::LoadImage(S_CW2T(psi->skin_src));
		if (pImg)
			imgView->SetImage(pImg);
	}


	SStringW GetColumnName(int iCol) const {
		return SStringW().Format(L"col%d", iCol + 1);
	}

	struct SORTCTX
	{
		int iCol;
		SHDSORTFLAG stFlag;
	};

	bool OnSort(int iCol, SHDSORTFLAG * stFlags, int nCols)
	{
		if (iCol == 5) //最后一列“操作”不支持排序
			return false;

		SHDSORTFLAG stFlag = stFlags[iCol];
		switch (stFlag)
		{
		case ST_NULL:stFlag = ST_UP; break;
		case ST_DOWN:stFlag = ST_UP; break;
		case ST_UP:stFlag = ST_DOWN; break;
		}
		for (int i = 0; i < nCols; i++)
		{
			stFlags[i] = ST_NULL;
		}
		stFlags[iCol] = stFlag;

		SORTCTX ctx = { iCol,stFlag };
		qsort_s(m_itemInfo.GetData(), m_itemInfo.GetCount(), sizeof(SKININFO), SortCmp, &ctx);
		return true;
	}

	static int __cdecl SortCmp(void *context, const void * p1, const void * p2)
	{
		SORTCTX *pctx = (SORTCTX*)context;
		const SKININFO *pSI1 = (const SKININFO*)p1;
		const SKININFO *pSI2 = (const SKININFO*)p2;
		int nRet = 0;
		if (pctx->stFlag == ST_UP)
			nRet = -nRet;
		return nRet;
	}
};

class CColorMcAdapter : public SMcAdapterBase
{
#define NUMSCALE 100000
public:
	struct SKININFO
	{
		SStringT name;
		SStringT color;
	};

	SArray<SKININFO> m_softInfo;

public:
	CColorMcAdapter()
	{
	}

	void Add(SStringT skinname, SStringT src)
	{
		SKININFO info;
		info.name = skinname;
		info.color = src;

		m_softInfo.Add(info);
	}

	virtual int getCount()
	{
		return m_softInfo.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		SKININFO *psi = m_softInfo.GetData() + position%m_softInfo.GetCount();
		pItem->FindChildByName(L"txt_name")->SetWindowText(S_CW2T(psi->name + L": " + psi->color));
		pItem->FindChildByName(L"img_icon")->SetAttribute(L"colorBkgnd", psi->color);
	}

	SStringW GetColumnName(int iCol) const {
		return SStringW().Format(L"col%d", iCol + 1);
	}
};

class CStringRecordAdapter : public SAdapterBase
{
protected:
	struct RecordInfo
	{
		SStringT name;
		SStringT text;
	};

	SArray<RecordInfo> m_Record;

public:
	CStringRecordAdapter()
	{
		
	}

	void Add(SStringT name, SStringT txt)
	{
		RecordInfo info;
		info.name = name;
		info.text = txt;

		m_Record.Add(info);
	}

protected:
	virtual int getCount()
	{
		return m_Record.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}
		RecordInfo & rec = m_Record[position];
		pItem->FindChildByID(R.id.txt_string)->SetWindowText(rec.name+": " + rec.text);
	}

};

//////////////////////////////////////////////////////////////////////////
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	g_pMainDlg = this;
	m_strOrigTitle = GETSTRING(R.string.title);

	m_bIsOpen = FALSE;
	m_bLayoutInited = TRUE;

	m_treePro = FindChildByName2<STreeCtrl>(L"workspace_tree");
	m_lbWorkSpaceXml = FindChildByName2<SListBox>(L"workspace_xmlfile_lb");
	m_staticAppTitle = FindChildByName2<SStatic>(L"apptitle");

	m_pLayoutContainer = FindChildByName2<SWindow>(L"uidesigner_wnd_layout");
	m_treeXmlStruct = FindChildByName2<STreeCtrl>(L"uidesigner_wnd_xmltree");
	m_textNodenum = FindChildByName2<SStatic>(L"uidesigner_nodenum");
	m_textCurXmlFile = FindChildByName2<SStatic>(L"uidesigner_curfile");
	m_textCtrlTypename = FindChildByName2<SStatic>(L"uidesigner_CtrlTypename");
	
	m_RealWndLayoutEdit = FindChildByName2<SRealWnd>(L"uidesigner_scintilla");
	m_RealWndXmlFile = FindChildByName2<SRealWnd>(L"uidesigner_xml_scintilla");
	// 控件列表
	m_lbControl = FindChildByName2<SListBoxDrop>(L"uidesigner_control_list");

	m_wndPropContainer = FindChildByName2<SWindow>(L"uidesigner_propContainer");
	m_edtDesc = FindChildByName2<SRichEdit>(L"uidesigner_DescText");

	m_tabDesigner = FindChildByName2<STabCtrl>(L"uidesigner_maintab");
	m_tabWorkspace = FindChildByName2<STabCtrl>(L"workspace_tab");
	m_pageEditor = FindChildByName2<STabPage>(L"page_editor");

	m_mcAllColor = FindChildByName2<SMCListView>("mclv_color");
	m_mcAllSkin = FindChildByName2<SMCListView>("mclv_skin");
	m_lvAllString = FindChildByName2<SListView>("lv_allString");
	m_lbAllStyle = FindChildByName2<SListBox>("lb_allStyle");
	//======================================================================
	m_pDesignerView = new SDesignerView((SHostDialog*)this, m_pLayoutContainer, m_treeXmlStruct);
	m_RealWndXmlFile->GetRealHwnd();	//触发建立真窗口
	
	m_textCurXmlFile->SetWindowText(_T(" 在编辑窗口按Ctrl+S保存文件"));

	m_treePro->GetEventSet()->subscribeEvent(EVT_TC_DBCLICK, Subscriber(&CMainDlg::OnTreeItemDbClick, this));

	m_tabDesigner->GetEventSet()->subscribeEvent(EVT_TAB_SELCHANGED, Subscriber(&CMainDlg::OnDesinerTabSelChanged, this));
	m_tabWorkspace->GetEventSet()->subscribeEvent(EVT_TAB_SELCHANGED, Subscriber(&CMainDlg::OnWorkspaceTabSelChanged, this));
	m_lbWorkSpaceXml->GetEventSet()->subscribeEvent(EVT_LB_DBCLICK, Subscriber(&CMainDlg::OnWorkspaceXMLDbClick, this));

	if (m_pDesignerView->m_bXmlResLoadOK)
	{

		
		g_SysDataMgr.LoadSysData(g_CurDir + L"Config");
		BOOL result = SDesignerView::LoadConfig(xmlDocCtrl,_T("Config\\ctrl.xml"));
		if (!result)
		{
			SMessageBox(m_hWnd, _T("加载ctrl.xml失败"), _T("SouiEditor"), MB_OK);
		}
		else
		{
			//注册控件面板选择事件
			m_lbControl->init(&m_mapCtrlList, m_pDesignerView);
			m_lbControl->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED, Subscriber(&CMainDlg::OnLbControlSelChanged, this));
			m_lbControl->AddString(_T("指针"));

			pugi::xml_node xmlNode = xmlDocCtrl.child(L"root", false).child(L"控件列表").first_child();
			for (; xmlNode; xmlNode = xmlNode.next_sibling())
			{
				SStringT strNodeName = S_CW2T(xmlNode.name());
				pugi::xml_writer_buff writer;
				xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
				SStringW *strxml = new SStringW(writer.buffer(), writer.size());

				m_mapCtrlList[strNodeName] = xmlNode;
				m_lbControl->AddString(strNodeName);
			}
		}
	}
	
	m_pDesignerView->InitProperty(m_textCtrlTypename, m_wndPropContainer);
	m_pDesignerView->BindXmlcodeWnd(m_RealWndLayoutEdit);

	RefreshWorkSpaceAllList();

	HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
	RegisterDragDrop(m_wndPropContainer->GetSwnd(), new CFileDropTarget(m_wndPropContainer));
	RegisterDragDrop(m_treePro->GetSwnd(), new CFileDropTarget(m_treePro));
	RegisterDragDrop(m_pLayoutContainer->GetSwnd(), new CFileDropTarget(m_pLayoutContainer));
	SWindow* MainWnd = FindChildByName2<SWindow>("UI_main_caption");
	if (MainWnd)
		RegisterDragDrop(MainWnd->GetSwnd(), new CFileDropTarget(MainWnd));

	return 0;
}

void CMainDlg::OnLanguageBtnCN()
{
	OnLanguage(1);
}

void CMainDlg::OnLanguageBtnJP()
{
	OnLanguage(0);
}

bool CMainDlg::OnTreeproContextMenu(CPoint pt)
{
	CPoint pt2 = pt;
	ClientToScreen(&pt2);

	HSTREEITEM Item = m_treePro->HitTest(pt);
	if (!Item) return false;

	SMenuEx menu;
	menu.LoadMenu(UIRES.smenu.menu_layoutfile);
	int cmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt2.x, pt2.y, m_hWnd);
	//  SMenu menu;
	//  menu.LoadMenu(UIRES.smenu.menu_layoutfile);
	// 	int cmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt2.x, pt2.y, NULL);
	if (cmd == 100)
	{
		SStringT *s = (SStringT*)m_treePro->GetItemData(Item);

		SStringT filename = m_strProPath + _T("\\") + *s;
		CScintillaWnd *pScintillaWnd = (CScintillaWnd*)m_RealWndXmlFile->GetUserData();
		if (pScintillaWnd)
		{
			pScintillaWnd->SetSaveCallback((SCIWND_FN_CALLBACK)&CMainDlg::OnScintillaSave);
			if (pScintillaWnd->OpenFile(filename))
			{
				m_textCurXmlFile->SetWindowText(filename);
				m_tabDesigner->SetCurSel(1);
			}
		}
	}
	return false;
}

LRESULT CMainDlg::OnShowMsgBox(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	SMessageBox(m_hWnd, (LPCTSTR)wp, (LPCTSTR)lp, MB_OK);
	return LRESULT(0);
}

void CMainDlg::OnLanguage(int nID)
{
	ITranslatorMgr *pTransMgr = SApplication::getSingletonPtr()->GetTranslator();
	SASSERT(pTransMgr);
	bool bCnLang = nID == 1;

	pugi::xml_document xmlLang;
	if (SApplication::getSingletonPtr()->LoadXmlDocment(xmlLang, bCnLang ? _T("lang_cn") : _T("lang_jp"), _T("translator")))
	{
		CAutoRefPtr<ITranslator> lang;
		pTransMgr->CreateTranslator(&lang);
		lang->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
		pTransMgr->SetLanguage(lang->name());
		pTransMgr->InstallTranslator(lang);
		SDispatchMessage(UM_SETLANGUAGE,0,0);
	}
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	if (m_bIsOpen)
	{
		if (SMessageBox(NULL, _T("确定要关闭当前工程吗? 未保存的结果将丢失."), _T("提示"), MB_YESNO) != IDYES)
			return;

		CloseProject();
	}

	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);
	if (bShow && !m_cmdWorkspaceFile.IsEmpty())
	{
		OpenProject(m_cmdWorkspaceFile);
		m_bIsOpen = TRUE;
		m_cmdWorkspaceFile.Empty();
	}
}

void CMainDlg::OnTimer(UINT_PTR timeID)
{
	if (timeID == TIMERID_RELOAD_LAYOUT)
	{
		m_pDesignerView->GetCodeFromEditor(NULL);
		CSimpleWnd::KillTimer(TIMERID_RELOAD_LAYOUT);
	}
	else
	{
		SetMsgHandled(FALSE);
	}
}

bool CMainDlg::Desiner_TabSelChanged(EventTabSelChanged *evt_sel)
{
	CSimpleWnd::SetTimer(TIMERID_RELOAD_LAYOUT, 300);
	return false;
}

void CMainDlg::DelayReloadLayout(STabCtrl* pTabHost)
{
	pTabHost->SetAttribute(_T("animateSteps"), _T("0"));
	pTabHost->GetEventSet()->unsubscribeEvent(EVT_TAB_SELCHANGED, Subscriber(&CMainDlg::Desiner_TabSelChanged, this));
	pTabHost->GetEventSet()->subscribeEvent<CMainDlg, EventTabSelChanged>(&CMainDlg::Desiner_TabSelChanged, this);
}

void CMainDlg::OutOpenProject(SStringT filename)
{
	if (m_bIsOpen)
	{
		CDebug::Debug(_T("请关闭当前工程后再打开新的"));
		return;
	}

	OpenProject(filename);
}

void CMainDlg::OnBtnOpen()
{
	if (m_bIsOpen)
	{
		CDebug::Debug(_T("请关闭当前工程后再打开新的"));
		return;
	}
	CFileDialogEx OpenDlg(TRUE, _T("idx"), _T("uires.idx"), 6, _T("soui skin index(*.idx)\0*.idx\0All files (*.*)\0*.*\0\0"));
	if (IDOK == OpenDlg.DoModal())
	{
		OpenProject(OpenDlg.m_szFileName);
	}
}

void CMainDlg::OnBtnClose()
{
	if (!m_bIsOpen || m_strProPath.IsEmpty())
		return;

	if (SMessageBox(NULL, _T("确定要关闭当前工程吗? 未保存的结果将丢失."), _T("提示"), MB_YESNO) != IDYES)
		return;

	CloseProject();
}

void CMainDlg::OnBtnReload()
{
	if (!m_bIsOpen || m_strProPath.IsEmpty())
		return;

	if (SMessageBox(NULL, _T("确定要重新加载当前工程吗? 未保存的结果将丢失."), _T("提示"), MB_YESNO) != IDYES)
		return;

	CloseProject();
	OpenProject(m_strUiresPath);
}

void CMainDlg::CloseProject()
{
	m_pDesignerView->CloseProject();
	m_treePro->RemoveAllItems();
	m_lbWorkSpaceXml->DeleteAll();
	m_UIResFileMgr.ReleaseUIRes();
	RefreshSkinList();
	RefreshColorList();
	RefreshStringList();
	m_lbAllStyle->DeleteAll();

	CScintillaWnd *pScintillaWnd = (CScintillaWnd*)m_RealWndXmlFile->GetUserData();
	if (pScintillaWnd)
	{
		m_textCurXmlFile->SetWindowText(_T("在编辑窗口按Ctrl+S保存文件"));
		pScintillaWnd->SendEditor(SCI_CLEARALL);
		pScintillaWnd->SetDirty(false);
	}

	m_bIsOpen = FALSE;
	m_staticAppTitle->SetWindowText(m_strOrigTitle);
}

//打开工程
void CMainDlg::OpenProject(SStringT strFileName)
{
	//注册事件
	m_treePro->RemoveAllItems();

	SStringT strFile = strFileName;
	int n = strFileName.ReverseFind(_T('\\'));
	m_strUiresPath = strFileName;
	m_strProPath = strFileName.Mid(0, n);
	m_pDesignerView->OpenProject(strFileName);

	pugi::xml_document xmlDoc;

	//if(!xmlDoc.load_file(strFile,pugi::parse_default,pugi::encoding_utf8)) 
	if (!xmlDoc.load_file(strFile))
		return;

	pugi::xml_node xmlLayout = xmlDoc.child(L"resource").child(L"LAYOUT", false);
	if (!xmlLayout) return;
	pugi::xml_node xmlFile = xmlLayout.first_child();
	while (xmlFile)
	{
		SStringT strName = S_CW2T(xmlFile.attribute(L"name").value());
		SStringT *strPath = new SStringT(S_CW2T(xmlFile.attribute(L"path").value()));

		//将文件名插入工程列表
		HSTREEITEM item = m_treePro->InsertItem(strName);  //strName = "xml_mainwnd"
		m_treePro->SetItemText(item, strName);
		m_treePro->SetItemData(item, (LPARAM)strPath);  //strpath = "xml\dlg_maing.xml"

		//加载布局文件到xmlnode
		m_pDesignerView->InsertLayoutToMap(*strPath);

		xmlFile = xmlFile.next_sibling();
	}

	m_UIResFileMgr.LoadUIResFromFile(m_strUiresPath);
	//m_pDesignerView->RenameAllLayoutWnd();

	m_staticAppTitle->SetWindowText(m_strOrigTitle + _T("      ") + m_strUiresPath);

	RefreshWorkSpaceAllList();

	m_bIsOpen = TRUE;
}

void CMainDlg::ReloadWorkspaceUIRes()
{
	if (!m_bIsOpen)
		return;

	m_UIResFileMgr.LoadUIResFromFile(m_strUiresPath);
	RefreshWorkSpaceAllList();
}

void CMainDlg::OnBtnSaveAll()
{
	if (!m_bIsOpen)
	{
		return;
	}
	m_pDesignerView->SaveAll();
	CScintillaWnd *pScintillaWnd = (CScintillaWnd*)m_RealWndXmlFile->GetUserData();
	if (pScintillaWnd)
	{
		pScintillaWnd->DoSave();
	}
}

void CMainDlg::OnBtnSaveLayout()
{
	if (!m_bIsOpen)
	{
		return;
	}
	if (!m_pDesignerView->SaveLayoutFile())
	{
		PrintLassErrorMessage();
	}
	else
	{
		SMessageBox(m_hWnd, _T("保存成功！"), NULL, MB_OK);
	}
}

void CMainDlg::OnBtnNewDialog()
{
	if (m_strProPath.IsEmpty())
	{
		CDebug::Debug(_T("请先打开工程"));
		return;
	}
	SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath);
	if (IDOK == DlgNewDialog.DoModal(m_hWnd))
	{
		CopyFile(g_CurDir + ("Config\\LayoutTmpl\\Dialog.xml"), DlgNewDialog.m_strPath, FALSE);
		m_pDesignerView->NewLayout(DlgNewDialog.m_strName, DlgNewDialog.m_strPath);

		SStringT *strShortPath = new SStringT(DlgNewDialog.m_strPath.Mid(m_strProPath.GetLength() + 1));

		//将文件名插入工程列表
		HSTREEITEM item = m_treePro->InsertItem(DlgNewDialog.m_strName);  //strName = "xml_mainwnd"
		m_treePro->SetItemText(item, DlgNewDialog.m_strName);
		m_treePro->SetItemData(item, (LPARAM)strShortPath);  //strpath = "xml\dlg_maing.xml"
		m_treePro->Invalidate();

		//加载布局文件到xmlnode
		m_pDesignerView->InsertLayoutToMap(*strShortPath);
	}
}

void CMainDlg::OnBtnNewInclude()
{
	if (m_strProPath.IsEmpty())
	{
		CDebug::Debug(_T("请先打开工程"));
		return;
	}
	SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath);
	if (IDOK == DlgNewDialog.DoModal(m_hWnd))
	{
		CopyFile(g_CurDir + _T("Config\\LayoutTmpl\\Include.xml"), DlgNewDialog.m_strPath, FALSE);
		m_pDesignerView->NewLayout(DlgNewDialog.m_strName, DlgNewDialog.m_strPath);

		SStringT *strShortPath = new SStringT(DlgNewDialog.m_strPath.Mid(m_strProPath.GetLength() + 1));

		//将文件名插入工程列表
		HSTREEITEM item = m_treePro->InsertItem(DlgNewDialog.m_strName);  //strName = "xml_mainwnd"
		m_treePro->SetItemText(item, DlgNewDialog.m_strName);
		m_treePro->SetItemData(item, (LPARAM)strShortPath);  //strpath = "xml\dlg_maing.xml"
		m_treePro->Invalidate();

		//加载布局文件到xmlnode
		m_pDesignerView->InsertLayoutToMap(*strShortPath);
	}
}

void CMainDlg::OnBtnWndLayout()
{
	if (m_pDesignerView->GetMoveWndRoot())
	{
		m_pDesignerView->m_nState = 0;
		m_pDesignerView->GetMoveWndRoot()->Click(0, CPoint(0, 0));
	}
}

void CMainDlg::OnBtnZYGL()
{
	m_pDesignerView->ShowZYGLDlg();
}

void CMainDlg::OnBtnYSGL()
{
	m_pDesignerView->ShowYSGLDlg();
}

void CMainDlg::OnBtnAbout()
{
	SDlgAbout dlg;
	dlg.DoModal(m_hWnd);
}

void CMainDlg::OnbtnPreview()
{
	if (m_pDesignerView->GetMoveWndRoot())
	{
		if (m_pDesignerView->GetMoveWndRoot()->IsVisible())
		{
			m_pDesignerView->Preview();
		}
		else
		{
			m_pDesignerView->unPreview();
		}
	}
}

void CMainDlg::LoadWorkSpace()
{
	if (!m_bIsOpen)
		return;

	SStringT strOpenLayoutFile = m_pDesignerView->m_strCurLayoutXmlFile;
	m_pDesignerView->CloseProject();
	OpenProject(m_strUiresPath);
	m_bIsOpen = TRUE;

	if (!strOpenLayoutFile.IsEmpty())
		m_pDesignerView->LoadLayout(strOpenLayoutFile);
}

bool CMainDlg::OnTreeItemDbClick(EventArgs *pEvtBase)
{
	//事件对象强制转换
	EventTCDbClick *pEvt = (EventTCDbClick*)pEvtBase;
	STreeCtrl *tree = (STreeCtrl*)pEvt->sender;

	SStringT *s = (SStringT*)tree->GetItemData(pEvt->hItem);

	if (m_tabDesigner->GetCurSel() == 0)
	{
		m_pDesignerView->LoadLayout(*s);
	}
	else {
		SStringT filename = m_strProPath + _T("\\") + *s;
		CScintillaWnd *pScintillaWnd = (CScintillaWnd*)m_RealWndXmlFile->GetUserData();
		if (pScintillaWnd)
		{
			pScintillaWnd->SetSaveCallback((SCIWND_FN_CALLBACK)&CMainDlg::OnScintillaSave);
			if (pScintillaWnd->OpenFile(filename))
			{
				m_textCurXmlFile->SetWindowText(filename);
			}
		}
	}

	return true;
}

bool CMainDlg::OnLbControlSelChanged(EventArgs *pEvtBase)
{
	EventLBSelChanged *pEvt = (EventLBSelChanged*)pEvtBase;
	SListBox *listbox = (SListBox*)pEvt->sender;
	if (pEvt->nNewSel != 0)
	{
		SStringT strText;
		strText = listbox->GetText(pEvt->nNewSel);

		//查找该类型的xml数据
		SMap<SStringT, pugi::xml_node>::CPair *p = m_mapCtrlList.Lookup(strText);  //查找
		if (p == NULL)
		{
			return false;
		}

		//m_pDesignerView->m_nState = 1;
		//m_pDesignerView->m_xmlNode = p->m_value;
		m_pDesignerView->SetSelCtrlNode(p->m_value);
	}
	else
	{
		m_pDesignerView->m_nState = 0;
	}

	return true;
}

bool CMainDlg::OnDesinerTabSelChanged(EventArgs *pEvtBase)
{
	EventTabSelChanging *evt = (EventTabSelChanging *)pEvtBase;
	if (evt->uNewSel == 1)
	{
	}
	
	return true;
}

bool CMainDlg::OnWorkspaceTabSelChanged(EventArgs * pEvtBase)
{
	EventTabSelChanging *evt = (EventTabSelChanging *)pEvtBase;
	if (evt->uNewSel == 1)
	{
		if (m_strProPath.IsEmpty() || !m_bIsOpen)
		{
			m_tabWorkspace->SetCurSel(0);
			return true;
		}

		m_lbWorkSpaceXml->DeleteAll();
		m_UIResFileMgr.LoadUIResFromFile(m_strUiresPath);

		std::vector<SStringT> vecTemp;
		SPOSITION pos = m_UIResFileMgr.m_mapXmlFile.GetStartPosition();
		while (pos)
		{
			const SMap<SStringT, SStringT>::CPair* item = m_UIResFileMgr.m_mapXmlFile.GetAt(pos);
			SStringT strLayoutName = RT_LAYOUT;
			strLayoutName.MakeLower();
			if (item->m_key.Find(strLayoutName + L":") == -1)
			{
				vecTemp.push_back(item->m_key);
			}
			m_UIResFileMgr.m_mapXmlFile.GetNext(pos);
		}

		m_lbWorkSpaceXml->AddString(UIRES_FILE);
		std::sort(vecTemp.begin(), vecTemp.end(), SortSStringNoCase);
		std::vector<SStringT>::iterator it = vecTemp.begin();
		for (; it != vecTemp.end(); it++)
		{
			m_lbWorkSpaceXml->AddString(*it);
		}
	}

	return true;
}

// 双击打开文件
bool CMainDlg::OnWorkspaceXMLDbClick(EventArgs * pEvtBase)
{
	EventLBDbClick *pEvt = (EventLBDbClick*)pEvtBase;
	SListBox *listbox = (SListBox*)pEvt->sender;
	if (pEvt->nCurSel != -1)
	{
		SStringT strText;
		strText = listbox->GetText(pEvt->nCurSel);

		SStringT filename = m_strProPath + L"\\";
		//查找此XML对应的文件
		SMap<SStringT, SStringT>::CPair *p = m_UIResFileMgr.m_mapXmlFile.Lookup(strText);  //查找
		if (p == NULL)
		{
			if (strText.CompareNoCase(UIRES_FILE) == 0)
				filename = m_strUiresPath;
			else
				return false;
		}
		else
			filename += p->m_value;

		m_tabDesigner->SetCurSel(1);
		CScintillaWnd *pScintillaWnd = (CScintillaWnd*)m_RealWndXmlFile->GetUserData();
		if (pScintillaWnd)
		{
			pScintillaWnd->SetSaveCallback((SCIWND_FN_CALLBACK)&CMainDlg::OnScintillaSave);
			if (pScintillaWnd->OpenFile(filename))
			{
				m_textCurXmlFile->SetWindowText(filename);
			}
		}
	}

	return true;
}

void CMainDlg::OnScintillaSave(CScintillaWnd *pObj, int custom_msg, SStringT str)
{
	if (!g_pMainDlg->m_bIsOpen)
		return;

	CScintillaWnd *pScintillaWnd = pObj;
	if (pScintillaWnd)
	{
		if (0 == custom_msg)
		{
			if (str.IsEmpty())
			{	// 布局可视化编辑时按了Ctrl+S
				g_pMainDlg->m_pDesignerView->GetCodeFromEditor(NULL);
				pScintillaWnd->SetDirty(false);
			}
			else
			{	// 是在直接编辑文件
				pScintillaWnd->DoSave();
				g_pMainDlg->LoadWorkSpace();
			}
		}
		else if (1 == custom_msg)
		{	// Edit控件内容有修改
			const LPCWSTR strModify = L" ***";

			STabCtrl* pTab = g_pMainDlg->m_tabDesigner;
			SStringT oldTitle = g_pMainDlg->m_pageEditor->GetTitle();
			if (!str.IsEmpty())
			{
				if (oldTitle.Find(strModify) == -1)
				{
					pTab->SetItemTitle(1, oldTitle + strModify);
				}
			}
			else
			{
				oldTitle.Replace(strModify, _T(""));
				pTab->SetItemTitle(1, oldTitle);
			}
		}
	}
}

void CMainDlg::RefreshWorkSpaceAllList()
{
	RefreshSkinList();
	RefreshColorList();
	RefreshStringList();
	RefreshStyleList();
}

void CMainDlg::RefreshSkinList()
{
	//多列listview
	if (m_mcAllSkin)
	{
		IMcAdapter *pAdapter = new CSkinMcAdapter;

		((CSkinMcAdapter*)pAdapter)->m_itemInfo.RemoveAll();

		SPOSITION pos = m_UIResFileMgr.m_mapSkins.GetStartPosition();
		while (pos)
		{
			const SMap<SStringT, ResManger::SkinItem>::CPair* item = m_UIResFileMgr.m_mapSkins.GetAt(pos);
			
			SStringT path = m_UIResFileMgr.GetResPathByName(item->m_value.src);
			((CSkinMcAdapter*)pAdapter)->Add(item->m_value.name, path);
			m_UIResFileMgr.m_mapSkins.GetNext(pos);
		}

		m_mcAllSkin->SetAdapter(pAdapter);
		pAdapter->Release();
	}
}

void CMainDlg::RefreshColorList()
{
	//多列listview
	if (m_mcAllColor)
	{
		IMcAdapter *pAdapter = new CColorMcAdapter;

		((CColorMcAdapter*)pAdapter)->m_softInfo.RemoveAll();

		SPOSITION pos = m_UIResFileMgr.m_mapColors.GetStartPosition();
		while (pos)
		{
			const SMap<SStringT, ResManger::ValueItem>::CPair* item = m_UIResFileMgr.m_mapColors.GetAt(pos);

			((CColorMcAdapter*)pAdapter)->Add(item->m_value.class_name, item->m_value.value);
			m_UIResFileMgr.m_mapColors.GetNext(pos);
		}

		m_mcAllColor->SetAdapter(pAdapter);
		pAdapter->Release();
	}
}

void CMainDlg::RefreshStringList()
{
	//listview
	if (m_lvAllString)
	{
		ILvAdapter *pLvAdapter = new CStringRecordAdapter;

		SPOSITION pos = m_UIResFileMgr.m_mapStrings.GetStartPosition();
		while (pos)
		{
			const SMap<SStringT, ResManger::ValueItem>::CPair* item = m_UIResFileMgr.m_mapStrings.GetAt(pos);

			((CStringRecordAdapter*)pLvAdapter)->Add(item->m_value.class_name, item->m_value.value);
			m_UIResFileMgr.m_mapStrings.GetNext(pos);
		}

		m_lvAllString->SetAdapter(pLvAdapter);
		pLvAdapter->Release();
	}
}


void CMainDlg::RefreshStyleList()
{
	//listbox
	SListBox * pListbox = FindChildByName2<SListBox>("lb_allStyle");
	if (pListbox)
	{
		pListbox->DeleteAll();

		SPOSITION pos = m_UIResFileMgr.m_mapStyles.GetStartPosition();
		while (pos)
		{
			const SMap<SStringT, ResManger::StyleItem>::CPair* item = m_UIResFileMgr.m_mapStyles.GetAt(pos);

			pListbox->AddString(item->m_value.name);
			m_UIResFileMgr.m_mapStyles.GetNext(pos);
		}
	}
}

