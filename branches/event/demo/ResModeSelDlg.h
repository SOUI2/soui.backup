#pragma once

class CResModeSelDlg :
	public CDuiHostWnd
{
public:
	CResModeSelDlg(void);
	~CResModeSelDlg(void);

	void OnOK()
	{
		m_nMode=((STabCtrl*)FindChildByID(200))->GetCurSel();
		EndDialog(IDOK);
	}

	int m_nMode;

	SOUI_NOTIFY_MAP_BEGIN()
		SOUI_NOTIFY_ID_COMMAND(1, OnOK)
	SOUI_NOTIFY_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_SOUI_NOTIFY()
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
};
