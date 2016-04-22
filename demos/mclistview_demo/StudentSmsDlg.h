#pragma once

struct IStudentCheckChanged
{
    virtual void OnStudentCheckChanged(int nSelCurrent, int nSelExpired) PURE;
};

class CStudentSmsDlg
	: public SOUI::SHostDialog
	, public IStudentCheckChanged
{
public:
    CStudentSmsDlg(void);
    ~CStudentSmsDlg(void);
    
protected:
    void OnBtnSmsRecord();
    
    void OnSearchFillList(EventArgs *e);
    void OnSearchValue(EventArgs *e);
    void OnSmsInputNotify(EventArgs *e);
    EVENT_MAP_BEGIN()
        EVENT_ID_HANDLER(R.id.edit_sms_input,EventRENotify::EventID,OnSmsInputNotify)
        EVENT_ID_COMMAND(R.id.btn_sms_record,OnBtnSmsRecord);
        EVENT_ID_HANDLER(R.id.edit_search,EventFillSearchDropdownList::EventID,OnSearchFillList)
        EVENT_ID_HANDLER(R.id.edit_search,EventDropdownListSelected::EventID,OnSearchValue)
    EVENT_MAP_END()
protected:
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnLButtonDown(UINT nFlags, CPoint point);

    BEGIN_MSG_MAP_EX(CStudentSmsDlg)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_INITDIALOG(OnInitDialog)
        CHAIN_MSG_MAP(SOUI::SHostDialog)
    END_MSG_MAP()

protected:
    virtual void OnStudentCheckChanged(int nSelCurrent, int nSelExpired);

protected:
	
    SWindow     * m_wndSmsRecord;
	SMCListView * m_tvStudent;
	SListView   * m_lvSmsRecord;
};

