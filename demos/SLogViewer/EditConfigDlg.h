#pragma once
#include "whwindow.h"

class CEditConfigDlg : public SHostDialog	, public CWHRoundRectFrameHelper<CEditConfigDlg>
{
public:
	CEditConfigDlg(void);
	~CEditConfigDlg(void);

	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CEditConfigDlg>)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()


	void OnSave();

	EVENT_MAP_BEGIN()
		EVENT_ID_COMMAND(R.id.btn_save,OnSave)
	EVENT_MAP_END()
private:
	CScintillaWnd *			 m_pSciter;

};

