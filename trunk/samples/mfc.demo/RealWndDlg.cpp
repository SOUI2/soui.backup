#include "StdAfx.h"
#include "RealWndDlg.h"

CRealWndDlg::CRealWndDlg(void):SOUI::SHostDialog(_T("xml::maindlg"))
{
}

CRealWndDlg::~CRealWndDlg(void)
{
}

void CRealWndDlg::OnBtnClick( UINT uNotifyCode, int nID, HWND wndCtl )
{
    if(uNotifyCode == BN_CLICKED && nID == 100)
    {
        SOUI::SMessageBox(m_hWnd,_T("the real mfc button is clicked!"),_T("mfc.demo"),MB_OK|MB_ICONEXCLAMATION);
    }
}