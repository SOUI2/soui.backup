// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

class CMainDlg : public SHostDialog
{
public:
	CMainDlg();
	~CMainDlg();

protected:
    void OnBtnAnimate();

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_ani", OnBtnAnimate)
    EVENT_MAP_END()	
};
