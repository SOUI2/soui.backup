#pragma once

class CMainDlg;

class CUIHander
{
public:

	CUIHander(CMainDlg *pMainDlg);
	~CUIHander(void);
	
	bool Evt_Test(EventArgs *pEvt);

protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);

	void OnDestory();

	void OnAttrReposition();
	void OnRepEditSel();
	void OnIECtrl();
	void OnDuiMenu();
	void OnBtnAniList();

	void OnTimer(UINT_PTR uEventID);

	LRESULT OnEditNotify(EventArgs *pEvt);

	LRESULT OnComboListSelChanging(EventArgs *pEvt);

	LRESULT OnComboListSelChanged(EventArgs *pEvt);

	LRESULT OnComboListItemNotify(EventArgs *pEvt);

	LRESULT OnListPredraw(EventArgs *pEvt);

	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

	LRESULT OnListBtnClick(EventArgs *pEvt);

	void OnBtnInitListClick();

	bool OnListHeaderClick(EventArgs *pEvt);
	
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
