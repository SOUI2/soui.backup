#pragma once

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
	
	bool Evt_Test(SWindow * pSender, LPSNMHDR pNmhdr);

protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);

	void OnDestory();

	void OnAttrReposition();
	void OnRepEditSel();
	void OnIECtrl();
	void OnDuiMenu();
	void OnBtnAniList();

	void OnTimer(UINT_PTR uEventID);

	LRESULT OnEditNotify(LPSNMHDR pNHdr);

	LRESULT OnComboListSelChanging( LPSNMHDR pNHdr );

	LRESULT OnComboListSelChanged(LPSNMHDR pNHdr);

	LRESULT OnComboListItemNotify(LPSNMHDR pNHdr);

	LRESULT OnListPredraw(LPSNMHDR pNHdr);

	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

	LRESULT OnListBtnClick(LPSNMHDR pNHdr);

	void OnBtnInitListClick();

	bool OnListHeaderClick(SWindow * pSender, LPSNMHDR pNmhdr);
	
	void OnHideTestClick();

	BEGIN_MSG_MAP_EX(CUIHander)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestory)
		MSG_WM_COMMAND(OnCommand)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_hidetst",OnHideTestClick)
	EVENT_MAP_END()	
private:
	CMainDlg * m_pMainDlg; 
};
