#pragma once

class CResModeSelDlg :
	public CDuiHostWnd
{
public:
	CResModeSelDlg(void);
	~CResModeSelDlg(void);

	void OnOK()
	{
		m_nMode=((CDuiTabCtrl*)FindChildByCmdID(200))->GetCurSel();
		EndDialog(IDOK);
	}

	int m_nMode;

	DUI_NOTIFY_MAP_BEGIN()
		DUI_NOTIFY_ID_COMMAND(1, OnOK)
	DUI_NOTIFY_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_DUI_NOTIFY()
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};
