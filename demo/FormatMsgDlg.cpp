#include "stdafx.h"
#include "FormatMsgDlg.h"

CFormatMsgDlg::CFormatMsgDlg(void):SHostDialog(_T("layout:dlg_formatmsg"))
{
}

CFormatMsgDlg::~CFormatMsgDlg(void)
{
}

void CFormatMsgDlg::OnOK()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_xmlinput");
    m_strMsg = pEdit->GetWindowText();
    EndDialog(IDOK);
}
