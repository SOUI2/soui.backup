#pragma once

#include "DuiMenu.h" 

class CMainDlg;

class CUIHander
{
public:
	struct student{
		TCHAR szName[100];
		TCHAR szSex[10];
		int age;
		int score;
	};

	CUIHander(CMainDlg *pMainDlg);
	~CUIHander(void);
	
	bool Evt_Test(CDuiWindow * pSender, LPDUINMHDR pNmhdr);

protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);

	void OnDestory();

	void OnAttrReposition();
	void OnRepEditSel();
	void OnIECtrl();
	void OnDuiMenu();
	void OnBtnAniList();

	void OnTimer(UINT_PTR uEventID);

	LRESULT OnEditNotify(LPDUINMHDR pNHdr);

	LRESULT OnComboListSelChanging( LPDUINMHDR pNHdr );

	LRESULT OnComboListSelChanged(LPDUINMHDR pNHdr);

	LRESULT OnComboListItemNotify(LPDUINMHDR pNHdr);

	LRESULT OnListPredraw(LPDUINMHDR pNHdr);

	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

	LRESULT OnListBtnClick(LPDUINMHDR pNHdr);

	void OnBtnInitListClick();

	bool OnListHeaderClick(CDuiWindow * pSender, LPDUINMHDR pNmhdr);
	
	void OnHideTestClick();

	BEGIN_MSG_MAP_EX(CUIHander)
		MSG_DUI_NOTIFY()
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestory)
		MSG_WM_COMMAND(OnCommand)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

	DUI_NOTIFY_MAP_BEGIN()
		DUI_NOTIFY_ID_COMMAND(≤‚ ‘, OnAttrReposition)
		DUI_NOTIFY_ID_COMMAND(IDC_REPSEL, OnRepEditSel)
		DUI_NOTIFY_ID_COMMAND(1307, OnIECtrl)
		DUI_NOTIFY_ID_COMMAND(1360, OnDuiMenu)
		DUI_NOTIFY_ID_HANDLER(1140,DUINM_RICHEDIT_NOTIFY,OnEditNotify)
		DUI_NOTIFY_ID_HANDLER(1310,DUINM_LBSELCHANGED,OnComboListSelChanged)
		DUI_NOTIFY_ID_HANDLER(1310,DUINM_LBSELCHANGING,OnComboListSelChanging)
		DUI_NOTIFY_ID_HANDLER(1310,DUINM_LBITEMNOTIFY,OnComboListItemNotify)
		DUI_NOTIFY_ID_HANDLER(mylist,DUINM_LBITEMNOTIFY,OnListBtnClick)
		DUI_NOTIFY_ID_HANDLER(mylist2,DUINM_GETLBDISPINFO,OnListPredraw)

		DUI_NOTIFY_ID_COMMAND(btn_ani,OnBtnAniList)

		DUI_NOTIFY_NAME_COMMAND("btn_hidetst",OnHideTestClick)
	DUI_NOTIFY_MAP_END()	
private:
	CMainDlg * m_pMainDlg; 
};
