// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#define close_animation_timer		101
#define switch_mode_pioneer			102
#define switch_mode_classic			103

#include "TrayDlg.h"
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

	void OnTimer(char nIDEvent);	

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	enum { SENDER_ID = 30000 };
	virtual int GetID() const { return SENDER_ID; }

	void OnMainTabSelChange(EventArgs *pEvt);

	void OnSwitchModePioneer();
protected:
	//soui消息
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close", OnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
		EVENT_NAME_COMMAND(L"switchmode_pioneer", OnSwitchModePioneer)
		

		EVENT_ID_HANDLER(SENDER_ID, EventTabSelChanged::EventID, OnMainTabSelChange)

		EVENT_MAP_END()
	//HostWnd真实窗口消息处理
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)


		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL			m_bLayoutInited;
	int				m_nCloseAnime;

	int				m_nMoveNumber;
	CTrayDlg	*   m_pTrayDlg;
};
