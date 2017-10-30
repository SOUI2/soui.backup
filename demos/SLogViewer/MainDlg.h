// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "SLogAdapter.h"

#include "FilterDlg.h"
#include "FindDlg.h"
#include "magnet/MagnetFrame.h"
#include "droptarget.h"
#include "whwindow.h"

class CMainDlg : public SHostWnd
	, public CMagnetFrame
	, public IFileDropHandler 
	, public IFindListener
	, public CWHRoundRectFrameHelper<CMainDlg>
{
public:
	CMainDlg();
	~CMainDlg();


	void UpdateFilterTags(const SArray<SStringW>& lstTags);
protected:
	void OnSize(UINT nType, CSize size);

	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

    void OnContextMenu(HWND hwnd, CPoint point);
protected:
	virtual void OnFileDropdown(HDROP hDrop);
	virtual bool OnFind(const SStringT & strText, bool bFindNext, bool bMatchCase, bool bMatchWholeWord);
	
	void OpenFile(LPCTSTR pszFileName);

	void OnClose();
	void OnMaximize();
	void OnRestore();
	void OnMinimize();
	void OnMenu();

	void OnAbout();
	void OnHelp();

	void OnLanguage(const SStringT & strLang);
	void OnLanguageBtnCN();
	void OnLanguageBtnEN();
	void OnOpenFile();
	void OnClear();	

	void OnFilterInputChange(EventArgs *e);
	void OnLevelsChange(EventArgs *e);
	bool OnLvContextMenu(CPoint pt);
	void OnOpenFindDlg();
	void OnEditConfig();
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_lang_cn", OnLanguageBtnCN)
		EVENT_NAME_COMMAND(L"btn_lang_en", OnLanguageBtnEN)
		EVENT_ID_COMMAND(R.id.btn_open_file,OnOpenFile)
		EVENT_ID_COMMAND(R.id.btn_find,OnOpenFindDlg)
		EVENT_ID_COMMAND(R.id.btn_clear,OnClear)
		EVENT_ID_COMMAND(R.id.btn_edit_config,OnEditConfig)
		EVENT_ID_COMMAND(R.id.btn_menu,OnMenu)
		EVENT_ID_HANDLER(R.id.edit_filter,EventRENotify::EventID,OnFilterInputChange)
		EVENT_ID_HANDLER(R.id.cbx_levels,EventCBSelChange::EventID,OnLevelsChange)
		EVENT_ID_CONTEXTMENU(R.id.lv_log,OnLvContextMenu)
	EVENT_MAP_END()
		//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMainDlg>)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	void UpdateFilterPids(const SArray<UINT> & lstPid);
	void UpdateFilterTids(const SArray<UINT> & lstTid);

	void UpdateLogParser();
protected:
	CAutoRefPtr<SLogAdapter> m_logAdapter;
	SComboBox	*			 m_cbxLevels;
	SMCListView	*			 m_lvLogs;	
	CFilterDlg *			 m_pFilterDlg;
	CScintillaWnd *			 m_pSciter;
	CFindDlg   *			 m_pFindDlg;

	SList<ILogParse *>		 m_logParserPool;
};
