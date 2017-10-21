// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "FileHelper.h"
#include "LogParser.h"
#include <helper/SMenu.h>
#include <helper/mybuffer.h>
#include "EditConfigDlg.h"

CMainDlg::CMainDlg() 
: SHostWnd(_T("LAYOUT:XML_MAINWND"))
,m_lvLogs(NULL)
,m_cbxLevels(NULL)
,m_pFindDlg(NULL)
,m_pFilterDlg(NULL)
,m_pSciter(NULL)
{
	m_logAdapter.Attach(new SLogAdapter);
	m_logAdapter->SetLogParserPool(&m_logParserPool);
}

CMainDlg::~CMainDlg()
{
	SPOSITION pos = m_logParserPool.GetHeadPosition();
	while(pos)
	{
		ILogParse *pLogParser = m_logParserPool.GetNext(pos);
		pLogParser->Release();
	}
	m_logParserPool.RemoveAll();
}


BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	//设置为磁吸主窗口
	SetMainWnd(m_hWnd);

	UpdateLogParser();



	m_pFilterDlg = new CFilterDlg(this);
	m_pFilterDlg->Create(m_hWnd);
	AddSubWnd(m_pFilterDlg->m_hWnd,AM_LEFT,AA_TOP);

	m_pFilterDlg->ShowWindow(SW_SHOW);

	m_lvLogs = FindChildByID2<SMCListView>(R.id.lv_log);
	if(m_lvLogs)
	{
		m_lvLogs->SetAdapter(m_logAdapter);
	}

	m_cbxLevels = FindChildByID2<SComboBox>(R.id.cbx_levels);
	
	::RegisterDragDrop(m_hWnd,GetDropTarget());

	IDropTarget *pDT = new CDropTarget(this);
	GetContainer()->RegisterDragDrop(m_lvLogs->GetSwnd(),pDT);
	pDT->Release();

	SRealWnd * pRealWnd = FindChildByID2<SRealWnd>(R.id.real_scilexer);
	SASSERT(pRealWnd);
	m_pSciter = (CScintillaWnd *)pRealWnd->GetUserData();
	m_pSciter->SendMessage(SCI_USEPOPUP,0,0);
	m_logAdapter->SetScintillaWnd(m_pSciter);
	return 0;
}

void CMainDlg::OnLanguageBtnCN()
{
	OnLanguage(_T("lang_cn"));
}
void CMainDlg::OnLanguageBtnEN()
{
	OnLanguage(_T("lang_en"));
}

void CMainDlg::OnLanguage(const SStringT & strLang)
{
	ITranslatorMgr *pTransMgr = SApplication::getSingletonPtr()->GetTranslator();
	SASSERT(pTransMgr);

	pugi::xml_document xmlLang;
	if (SApplication::getSingletonPtr()->LoadXmlDocment(xmlLang, strLang,_T("languages")))
	{
		CAutoRefPtr<ITranslator> lang;
		pTransMgr->CreateTranslator(&lang);
		lang->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
		pTransMgr->SetLanguage(lang->name());
		pTransMgr->InstallTranslator(lang);
		SDispatchMessage(UM_SETLANGUAGE,0,0);
		m_pFilterDlg->SDispatchMessage(UM_SETLANGUAGE,0,0);
		if(m_pFindDlg) m_pFindDlg->SDispatchMessage(UM_SETLANGUAGE,0,0);
	}
}

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

void CMainDlg::OnOpenFile()
{
	CFileDialogEx openDlg(TRUE,_T("log"),0,6,_T("log files(*.log)\0*.log\0txt files(*.txt)\0*.txt\0All files (*.*)\0*.*\0\0"));
	if(openDlg.DoModal()==IDOK)
	{
		OpenFile(openDlg.m_szFileName);
	}
}

void CMainDlg::OnClear()
{
	m_logAdapter->Clear();
	m_pSciter->SendMessage(SCI_CLEARALL);
	SArray<SStringW> lstTags;
	m_pFilterDlg->UpdateTags(lstTags);


	SArray<UINT> lstPid;
	m_pFilterDlg->UpdatePids(lstPid);

	SArray<UINT> lstTid;
	m_pFilterDlg->UpdateTids(lstTid);
}

void CMainDlg::OnFilterInputChange(EventArgs *e)
{
	EventRENotify *e2 = sobj_cast<EventRENotify>(e);
	SASSERT(e2);
	if(e2->iNotify == EN_CHANGE)
	{
		SEdit * pEdit = sobj_cast<SEdit>(e2->sender);
		SStringT str = pEdit->GetWindowText();
		m_logAdapter->SetFilter(str);
	}
}

void CMainDlg::OnLevelsChange(EventArgs *e)
{
	EventCBSelChange * e2 = sobj_cast<EventCBSelChange>(e);
	m_logAdapter->SetLevel(e2->nCurSel);
}


void CMainDlg::OnFileDropdown(HDROP hDrop)
{
	bool success = false;
	TCHAR filename[MAX_PATH];
	success=!!DragQueryFile(hDrop, 0, filename, MAX_PATH);
	if(success) 
	{
		if(!(GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY))
		{
			OpenFile(filename);
		}
	}

}

void CMainDlg::OpenFile(LPCTSTR pszFileName)
{
	if(!m_logAdapter->Load(pszFileName))
	{
		SMessageBox(m_hWnd,GETSTRING(R.string.msg_open_failed),GETSTRING(R.string.title_no_name),MB_OK|MB_ICONSTOP);
		return;
	}
	
	TCHAR szName[MAX_PATH];
	_tsplitpath(pszFileName,NULL,NULL,szName,NULL);
	SStringT strFmt = GETSTRING(R.string.title);
	SStringT strTitle = SStringT().Format(S_CW2T(strFmt),szName);
	FindChildByID(R.id.txt_title)->SetWindowText(strTitle);
	CSimpleWnd::SetWindowText(strTitle);

	ILogParse *pLogParser = m_logAdapter->GetLogParse();
	if(pLogParser)
	{
		m_cbxLevels->ResetContent();
		int nLevels = pLogParser->GetLevels();
		wchar_t (*szLevels)[MAX_LEVEL_LENGTH] = new wchar_t[nLevels][MAX_LEVEL_LENGTH];
		pLogParser->GetLevelText(szLevels);
		for(int i=0;i<nLevels;i++)
		{
			m_cbxLevels->InsertItem(i,S_CW2T(szLevels[i]),0,i);
		}
		delete []szLevels;

	}

	SArray<SStringW> lstTags;
	m_logAdapter->GetTags(lstTags);
	m_pFilterDlg->UpdateTags(lstTags);


	SArray<UINT> lstPid;
	m_logAdapter->GetPids(lstPid);
	m_pFilterDlg->UpdatePids(lstPid);

	SArray<UINT> lstTid;
	m_logAdapter->GetTids(lstTid);
	m_pFilterDlg->UpdateTids(lstTid);

}

bool CMainDlg::OnLvContextMenu(CPoint pt)
{
	CPoint pt2 = pt;
	ClientToScreen(&pt2);

	SItemPanel *pItem = m_lvLogs->HitTest(pt);
	if(!pItem) return false;

	int iItem = pItem->GetItemIndex();
	SLogInfo *pLogInfo = m_logAdapter->GetLogInfo(iItem);

	SMenu menu;
	menu.LoadMenu(UIRES.smenu.menu_lv);
	SStringW str = TR(GETSTRING(R.string.exclude_tag),L"");
	str += pLogInfo->strTag;
	menu.ModifyMenuString(101,MF_BYCOMMAND,S_CW2T(str));
	str = TR(GETSTRING(R.string.only_tag),L"");
	str += pLogInfo->strTag;
	menu.ModifyMenuString(102,MF_BYCOMMAND,S_CW2T(str));

	int cmd = menu.TrackPopupMenu(TPM_RETURNCMD,pt2.x,pt2.y,NULL);
	if(cmd==100)
	{
		HGLOBAL hMen;   

		// 分配全局内存    
		hMen = GlobalAlloc(GMEM_MOVEABLE, pLogInfo->strContent.GetLength()*sizeof(WCHAR));    

		if (!hMen)   
		{   
			return false;         
		}   

		LPWSTR lpStr = (LPWSTR)GlobalLock(hMen);    

		// 内存复制   
		memcpy(lpStr, (LPCWSTR)pLogInfo->strContent, pLogInfo->strContent.GetLength()*sizeof(WCHAR));    
		// 释放锁    
		GlobalUnlock(hMen);   

		::OpenClipboard(m_hWnd);
		::SetClipboardData(CF_UNICODETEXT,hMen);
		::CloseClipboard();
	}else if(cmd == 101)
	{
		m_pFilterDlg->ExcludeTag(pLogInfo->strTag);
	}else if(cmd == 102)
	{
		m_pFilterDlg->OnlyTag(pLogInfo->strTag);
	}

	return true;
}

void CMainDlg::UpdateFilterTags(const SArray<SStringW>& lstTags)
{
	m_logAdapter->SetFilterTags(lstTags);
}

void CMainDlg::UpdateFilterPids(const SArray<UINT> & lstPid)
{
	m_logAdapter->SetFilterPids(lstPid);
}

void CMainDlg::UpdateFilterTids(const SArray<UINT> & lstTid)
{
	m_logAdapter->SetFilterTids(lstTid);
}

void CMainDlg::OnOpenFindDlg()
{
	if(m_pFindDlg==NULL)
	{
		m_pFindDlg = new CFindDlg(this);
		m_pFindDlg->Create(m_hWnd);
		m_pFindDlg->CenterWindow(m_pSciter->m_hWnd);
	}		
	m_pFindDlg->ShowWindow(SW_SHOW);
}

bool CMainDlg::OnFind(const SStringT & strText, bool bFindNext, bool bMatchCase, bool bMatchWholeWord)
{
	int flags = (bMatchCase?SCFIND_MATCHCASE:0) | (bMatchWholeWord?SCFIND_WHOLEWORD:0);
	TextToFind ttf;
	ttf.chrg.cpMin = m_pSciter->SendMessage(SCI_GETCURRENTPOS);
	if(bFindNext)
		ttf.chrg.cpMax = m_pSciter->SendMessage(SCI_GETLENGTH, 0, 0);
	else
		ttf.chrg.cpMax = 0;

	SStringA strUtf8 = S_CT2A(strText,CP_UTF8);
	ttf.lpstrText = (char *)(LPCSTR) strUtf8;
	int nPos = m_pSciter->SendMessage(SCI_FINDTEXT,flags,(LPARAM)&ttf);
	if(nPos==-1) return false;
	
	if(bFindNext)
		m_pSciter->SendMessage(SCI_SETSEL,nPos,nPos + strUtf8.GetLength());
	else
		m_pSciter->SendMessage(SCI_SETSEL,nPos+ strUtf8.GetLength(),nPos);

	m_pSciter->SetFocus();

	return true;
}

void CMainDlg::OnContextMenu(HWND hwnd, CPoint point)
{
	if(hwnd == m_pSciter->m_hWnd)
	{
		pugi::xml_node xmlMenu = SRicheditMenuDef::getSingleton().GetMenuXml();
		if(xmlMenu)
		{
			SMenu menu;
			if(menu.LoadMenu(xmlMenu))
			{
				BOOL canPaste=m_pSciter->SendMessage(SCI_CANPASTE);
				BOOL hasSel=m_pSciter->SendMessage(SCI_GETSELECTIONEMPTY)==0;
				UINT uLen=m_pSciter->SendMessage(SCI_GETTEXTLENGTH ,0,0);
				EnableMenuItem(menu.m_hMenu,MENU_CUT,MF_BYCOMMAND|((hasSel)?0:MF_GRAYED));
				EnableMenuItem(menu.m_hMenu,MENU_COPY,MF_BYCOMMAND|(hasSel?0:MF_GRAYED));
				EnableMenuItem(menu.m_hMenu,MENU_PASTE,MF_BYCOMMAND|((canPaste)?0:MF_GRAYED));
				EnableMenuItem(menu.m_hMenu,MENU_DEL,MF_BYCOMMAND|((hasSel)?0:MF_GRAYED));
				EnableMenuItem(menu.m_hMenu,MENU_SELALL,MF_BYCOMMAND|((uLen>0)?0:MF_GRAYED));

				UINT uCmd=menu.TrackPopupMenu(TPM_RETURNCMD|TPM_LEFTALIGN,point.x,point.y,m_hWnd);
				switch(uCmd)
				{
				case MENU_CUT:
					m_pSciter->SendMessage(SCI_CUT);
					break;
				case MENU_COPY:
					m_pSciter->SendMessage(SCI_COPY);
					break;
				case MENU_PASTE:
					m_pSciter->SendMessage(SCI_PASTE);
					break;
				case MENU_DEL:
					m_pSciter->SendMessage(SCI_REPLACESEL,0,(LPARAM)_T(""));
					break;
				case MENU_SELALL:
					m_pSciter->SendMessage(SCI_SETSEL,0,-1);
					break;
				default:
					break;
				}
			}
		}
	}
}

void CMainDlg::OnEditConfig()
{
	CEditConfigDlg editConfig;
	if(IDOK==editConfig.DoModal())
	{
		UpdateLogParser();
	}
}

void CMainDlg::UpdateLogParser()
{
	SStringW strAppDir = SApplication::getSingleton().GetAppDir();
	strAppDir += L"\\config.xml";
	pugi::xml_document doc;
	if(!doc.load_file(strAppDir))
	{
		DWORD dwSize = SApplication::getSingleton().GetRawBufferSize(_T("xml"),_T("config"));
		if(dwSize)
		{
			CMyBuffer<char> buf(dwSize);
			SApplication::getSingleton().GetRawBuffer(_T("xml"),_T("config"),buf,dwSize);
			FILE *f = _wfopen(strAppDir,L"w+b");
			if(f)
			{
				fwrite(buf,1,dwSize,f);
				fclose(f);
			}
			doc.load_buffer(buf,dwSize);
		}
	}

	SPOSITION pos = m_logParserPool.GetHeadPosition();
	while(pos)
	{
		ILogParse *pLogParser = m_logParserPool.GetNext(pos);
		pLogParser->Release();
	}
	m_logParserPool.RemoveAll();

	pugi::xml_node xmlLogParser = doc.child(L"logs").child(L"log");
	while(xmlLogParser)
	{
		SStringW strName = xmlLogParser.attribute(L"name").as_string();
		int nCodePage = xmlLogParser.attribute(L"codepage").as_int(CP_UTF8);
		SStringW strLevels = xmlLogParser.child(L"levels").text().as_string();
		strLevels.TrimBlank();
		SStringW strFmt = xmlLogParser.child(L"format").text().as_string();
		strFmt.TrimBlank();
		CLogParse *pLogParser = new CLogParse(strName,strFmt,strLevels,nCodePage);
		m_logParserPool.AddTail(pLogParser);
		xmlLogParser = xmlLogParser.next_sibling(L"log");
	}
}

void CMainDlg::OnAbout()
{
	SHostDialog dlgAbout(UIRES.LAYOUT.dlg_about);
	dlgAbout.DoModal();
}

