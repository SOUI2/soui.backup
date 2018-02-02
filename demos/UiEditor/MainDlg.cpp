// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "CNewGuid.h"
#include "DlgNewLayout.h"
#include "CDebug.h"
#include "ScintillaWnd.h"
#include "Scintilla.h"
#include "DlgAbout.h"
#include "DlgSkinSelect.h"
	
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif
#include "helpapi.h"
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:UIDESIGNER_XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
	shellNotifyIcon.Hide();
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


//初始化g
BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	this->ModifyState(0, WS_CAPTION);
	m_bLayoutInited = TRUE;
	m_bIsOpen = FALSE;

	//char *p = CNewGuid::Get();


	shellNotifyIcon.Create(m_hWnd,GETRESPROVIDER->LoadIcon(_T("ICON_LOGO"),16));
	shellNotifyIcon.Show();
    m_treePro = FindChildByName2<STreeCtrl>(L"NAME_UIDESIGNER_mytree");
	m_pLayoutContainer = FindChildByName2<SWindow>(L"NAME_UIDESIGNER_wnd_layout");
	m_pPropertyPage = FindChildByName2<STabCtrl>(L"NAME_UIDESIGNER_LEFT_MAIN_TAB");
	m_RealWnd =  FindChildByName2<SRealWnd>(L"SKIN_UIDESIGNER_scintilla");
	m_tabDesigner = FindChildByName2<STabCtrl>(L"NAME_UIDESIGNER_DESIGNER_MAIN_TAB");
	m_tabRight = FindChildByName2<STabCtrl>(L"NAME_UIDESIGNER_Right_MAIN_TAB");

	m_treeXmlStruct = FindChildByName2<STreeCtrl>(L"NAME_UIDESIGNER_xmltree");


	btn_save = FindChildByName2<SButton>(L"NAME_UIDESIGNER_btn_SaveLayout");
	


	m_lbControl = FindChildByName2<SListBoxDrop>(L"NAME_UIDESIGNER_LB_CONTROL");

	m_btnPreview = FindChildByName2<SToolbox>(L"NAME_UIDESIGNER_btn_YL");

	m_wndPropContainer = FindChildByName2<SWindow>(L"NAME_UIDESIGNER_propContainer");
	m_edtDesc =  FindChildByName2<SRichEdit>(L"NAME_UIDESIGNER_edtDesc");

	m_scrView = FindChildByName2<SScrollView>(L"NAME_UIDESIGNER_wnd_layout");


	m_tabDesigner->GetEventSet()->subscribeEvent(EVT_TAB_SELCHANGED,Subscriber(&CMainDlg::OnDesinerTabSelChanged,this));
    m_tabRight->GetEventSet()->subscribeEvent(EVT_TAB_SELCHANGED,Subscriber(&CMainDlg::OnRightTabSelChanged,this));

	m_pDesignerView = new SDesignerView((SHostDialog*)this, m_pLayoutContainer, m_treeXmlStruct);
	

	//pugi::xml_parse_result result = xmlDocCtrl.load_file(L"uires\\xml\\SkinConfig.xml", pugi::parse_default, pugi::encoding_utf8);
    pugi::xml_parse_result result = xmlDocCtrl.load_file(L"Config\\ctrl.xml");
	if(!result)
	{
		SMessageBox(m_hWnd, _T("加载SkinConfig.xml失败"), _T("SkinConfig.xml"), MB_OK);
	}else
	{
		//注册控件面板选择事件
		m_lbControl->init(&m_mapCtrlList, m_pDesignerView);
		m_lbControl->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED,Subscriber(&CMainDlg::OnLbControlSelChanged,this));
		m_lbControl->AddString(_T("指针"));

		pugi::xml_node xmlNode = xmlDocCtrl.child(L"root", false).child(L"控件列表").first_child();
		for (; xmlNode; xmlNode = xmlNode.next_sibling())
		{
			SStringT strNodeName = S_CW2T(xmlNode.name());
			pugi::xml_writer_buff writer;
			xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
			SStringW *strxml=new SStringW(writer.buffer(),writer.size());

			m_mapCtrlList[strNodeName] = xmlNode;
			m_lbControl->AddString(strNodeName);
		}
	}
	m_pDesignerView->InitProperty(m_wndPropContainer);
	return 0;
}


//TODO:消息映射
void CMainDlg::OnClose()
{
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
	
	SWindow *pBtnMax = FindChildByName(L"NAME_UIDESIGNER_btn_max");
	SWindow *pBtnRestore = FindChildByName(L"NAME_UIDESIGNER_btn_restore");
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




void CMainDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if(bShow)
	{
	}
}


void CMainDlg::OnBtnNewDialog() //新建Dialog
{	
	if (m_strProPath.IsEmpty())
	{
		CDebug::Debug(_T("请先打开工程"));
		return;
	}
	SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath);
	if (IDOK == DlgNewDialog.DoModal(m_hWnd))
	{
		CopyFile(_T("Config\\LayoutTmpl\\Dialog.xml"), DlgNewDialog.m_strPath, FALSE);
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

void CMainDlg::OnBtnNewInclude()  //新建Include
{
	if (m_strProPath.IsEmpty())
	{
		CDebug::Debug(_T("请先打开工程"));
		return;
	}
	SDlgNewLayout DlgNewDialog(_T("layout:UIDESIGNER_XML_NEW_LAYOUT"), m_strProPath);
	if (IDOK == DlgNewDialog.DoModal(m_hWnd))
	{
		CopyFile(_T("Config\\LayoutTmpl\\Include.xml"), DlgNewDialog.m_strPath, FALSE);
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

//打开工程
void CMainDlg::OnBtnOpen()
{
	if (!m_strProPath.IsEmpty())
	{
		CDebug::Debug(_T("请关闭当前工程后再打开新的"));
		return;
	}
	CFileDialogEx OpenDlg(TRUE, _T("idx"), _T("uires.idx"), 6,_T("soui skin index(*.idx)\0*.idx\0All files (*.*)\0*.*\0\0"));
	if (IDOK ==OpenDlg.DoModal())
	{
		//m_strProFileName = OpenDlg.m_szFileName;
	    SStringT m_strProFileName = OpenDlg.m_szFileName;
		OpenProject(m_strProFileName);
		m_bIsOpen = TRUE;
	}


}

//打开工程
void CMainDlg::OpenProject(SStringT strFileName)
{
	//注册事件
    m_treePro->RemoveAllItems();
	m_treePro->GetEventSet()->subscribeEvent(EVT_TC_DBCLICK,Subscriber(&CMainDlg::OnTreeItemDbClick,this));


	SStringT strFile = strFileName;

	int n = strFileName.ReverseFind(_T('\\'));

	m_strProPath = strFileName.Mid(0, n);

	m_pDesignerView->OpenProject(strFileName);


	pugi::xml_document xmlDoc;

	//if(!xmlDoc.load_file(strFile,pugi::parse_default,pugi::encoding_utf8)) 
	if(!xmlDoc.load_file(strFile))
		return;

	pugi::xml_node xmlLayout=xmlDoc.child(L"resource").child(L"LAYOUT", false);
	if(!xmlLayout) return ;
	pugi::xml_node xmlFile=xmlLayout.first_child();
	while(xmlFile)
	{

		SStringT strName = S_CW2T(xmlFile.attribute(L"name").value());

		SStringT *strPath = new SStringT(S_CW2T(xmlFile.attribute(L"path").value()));


		//将文件名插入工程列表
		HSTREEITEM item = m_treePro->InsertItem(strName);  //strName = "xml_mainwnd"
		m_treePro->SetItemText(item, strName);
		m_treePro->SetItemData(item, (LPARAM)strPath);  //strpath = "xml\dlg_maing.xml"


		//加载布局文件到xmlnode
		m_pDesignerView->InsertLayoutToMap(*strPath);

		xmlFile=xmlFile.next_sibling();
	}		


	//m_pDesignerView->RenameAllLayoutWnd();

	

}

void CMainDlg::OnBtnSaveAll()
{
	if (!m_bIsOpen)
	{
		return;
	}
	m_pDesignerView->SaveAll();
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

bool CMainDlg::OnLbControlSelChanged(EventArgs *pEvtBase)
{
	EventLBSelChanged *pEvt =(EventLBSelChanged*)pEvtBase;
	SListBox *listbox=(SListBox*)pEvt->sender;
	if (pEvt->nNewSel != 0)
	{
		SStringT strText = listbox->GetText(pEvt->nNewSel);

		//查找该类型的xml数据

		SMap<SStringT, pugi::xml_node>::CPair *p=m_mapCtrlList.Lookup(strText);  //查找
		if(p == NULL)
		{
			return false;
		}

		//m_pDesignerView->m_nState = 1;
		//m_pDesignerView->m_xmlNode = p->m_value;
		m_pDesignerView->SetSelCtrlNode(p->m_value);

	}else
	{
		m_pDesignerView->m_nState = 0;
	}


    return true;
}

bool CMainDlg::OnTreeItemDbClick(EventArgs *pEvtBase)
{
	//事件对象强制转换
	EventTCDbClick *pEvt =(EventTCDbClick*)pEvtBase;
	STreeCtrl *tree=(STreeCtrl*)pEvt->sender;

	SStringT *s = (SStringT*)tree->GetItemData(pEvt->hItem);


	m_pDesignerView->LoadLayout(*s);

	return true;
}



void CMainDlg::OnBtnWndLayout()
{
	if (m_pDesignerView->GetMoveWndRoot())
	{
		m_pDesignerView->m_nState = 0;
		m_pDesignerView->GetMoveWndRoot()->Click(0, CPoint(0,0));
	}


}

bool CMainDlg::OnDesinerTabSelChanged(EventArgs *pEvtBase)
{
	EventTabSelChanging *evt = (EventTabSelChanging *)pEvtBase;
	if (evt->uNewSel == 1)
	{
		CScintillaWnd *pWnd = (CScintillaWnd*)m_RealWnd->GetUserData();
		if (pWnd)
		{

			m_pDesignerView->AddCodeToEditor(pWnd);
			//pWnd->SendMessage(SCI_CLEARALL, 0, 0);

			//SStringA str;

			//pugi::xml_writer_buff writer;
			//m_pDesignerView->m_xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
			//SStringW *strDebug= new SStringW(writer.buffer(),writer.size());

			//str=S_CW2A(*strDebug,CP_UTF8);
			//pWnd->SendMessage(SCI_ADDTEXT, str.GetLength(),
			//	reinterpret_cast<LPARAM>((LPCSTR)str));

			//pWnd->SendMessage(SCI_SETWRAPMODE,(WPARAM)1); //自动换行

			//pWnd->SendMessage(SCI_SETWRAPMODE,(WPARAM)0); //取消自动换行
		}

	}else
	{
		CScintillaWnd *pWnd = (CScintillaWnd*)m_RealWnd->GetUserData();
		if (pWnd)
		{

			m_pDesignerView->GetCodeFromEditor(pWnd);
			//int n = pWnd->SendMessage(SCI_GETTEXT, 0,
			//	0);
			//char *chText = new char[n];
			//

			//pWnd->SendMessage(SCI_GETTEXT, n, (LPARAM)chText);

			//SStringA s(chText);
			//SStringT s1 = S_CA2T(s, CP_UTF8);
			//m_pDesignerView->Debug(s1);
						//pWnd->SendMessage(SCI_CLEARALL, 0, 0);
			
		}

	}
	return true;
}


bool CMainDlg::OnRightTabSelChanged(EventArgs *pEvtBase)
{
	EventTabSelChanging *evt = (EventTabSelChanging *)pEvtBase;
	if (evt->uNewSel == 1)
	{

		m_pDesignerView->m_treeXmlStruct->RemoveAllItems();
		m_pDesignerView->InitXMLStruct(m_pDesignerView->m_CurrentLayoutNode, STVI_ROOT);
	}else
	{

	}
	return true;
}

void CMainDlg::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
		m_bMsgHandled = FALSE;
}

void CMainDlg::OnbtnPreview()
{

	//调用类型对话框

	if (m_pDesignerView->GetMoveWndRoot())
	{
		if (m_pDesignerView->GetMoveWndRoot()->IsVisible())
		{
			m_pDesignerView->Preview();
		}else
		{
			m_pDesignerView->unPreview();
		}
	}

	



}

void CMainDlg::OnBtnYSGL()
{
  m_pDesignerView->ShowYSGLDlg();
}

void CMainDlg::OnBtnZYGL()
{
  m_pDesignerView->ShowZYGLDlg();
}

void CMainDlg::OnbtnAbout()
{
	SDlgAbout dlg;
	dlg.DoModal(m_hWnd);
}

void CMainDlg::test()
{
	static LOGFONT logfont ;

	CHOOSEFONT cf ;

	cf.lStructSize                = sizeof (CHOOSEFONT) ;

	cf.hwndOwner                  = m_hWnd ;

	cf.hDC                        = GetDC() ;

	cf.lpLogFont                  = &logfont ;

	cf.iPointSize                 = 0 ;

	cf.Flags                             = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS ;

	cf.rgbColors                         = 0 ;

	cf.lCustData                         = 0 ;

	cf.lpfnHook                          = NULL ;

	cf.lpTemplateName                = NULL ;

	cf.hInstance                         = NULL ;

	cf.lpszStyle                         = NULL ;

	cf.nFontType                         = 0 ;                         // Returned from ChooseFont

	cf.nSizeMin                                  = 0 ;

	cf.nSizeMax                                  = 0 ;



	ChooseFont (&cf) ;

	static CHOOSECOLOR    cc ;

	static COLORREF                      crCustColors[16] ;


	cc.lStructSize                       = sizeof (CHOOSECOLOR) ;

	cc.hwndOwner                         = NULL ;

	cc.hInstance                         = NULL ;

	cc.rgbResult                         = RGB (0x80, 0x80, 0x80) ;

	cc.lpCustColors                      = crCustColors ;

	cc.Flags                             = CC_RGBINIT | CC_FULLOPEN ;

	cc.lCustData                        = 0 ;

	cc.lpfnHook                          = NULL ;

	cc.lpTemplateName = NULL ;


 ChooseColor (&cc) ;


}


#include <helper/smenu.h>
LRESULT CMainDlg::OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/)
{
	switch (lParam)
	{
	case  WM_RBUTTONDOWN:
	{
            ////显示右键菜单
            //SMenu menu;
            //menu.LoadMenu(_T("menu_tray"),_T("smenu"));
            //POINT pt;
            //GetCursorPos(&pt);
            //menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
	}break;
	case WM_LBUTTONDOWN:
	{
		//显示/隐藏主界面
		if (IsIconic())
		   ShowWindow(SW_SHOWNORMAL);
		else
		   ShowWindow(SW_MINIMIZE);
	}break;
	default:
		break;
	}
	return S_OK;
}

//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		switch (nID)
		{
		case 6:
			PostMessage(WM_CLOSE);
			break;
		default:
			break;
		}
	}
}



