#pragma once

class CRealWndDlg : public SOUI::SHostDialog
{
public:
    CRealWndDlg(void);
    ~CRealWndDlg(void);

    //响应MFC.button的按下消息, nID==100为在XML中指定的realwnd的id属性。
    void OnBtnClick( UINT uNotifyCode, int nID, HWND wndCtl )
    {
        if(uNotifyCode == BN_CLICKED && nID == 100)
        {
            SOUI::SMessageBox(m_hWnd,_T("the real mfc button is clicked!"),_T("mfc.demo"),MB_OK|MB_ICONEXCLAMATION);
        }
    }

    //消息映射表
    BEGIN_MSG_MAP_EX(CRealWndDlg)
        MSG_WM_COMMAND(OnBtnClick)
        CHAIN_MSG_MAP(SOUI::SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
};
