#pragma once
class CMainDlg;

class CNewFileDlg : public SHostDialog
{
public:
	CNewFileDlg(CMainDlg *pMainDlg);
	~CNewFileDlg(void);

	SStringT m_strSrcFile;
	SStringT m_strResType;
	SStringT m_strResName;
	SStringT m_strResPath;
protected:
	void OnBtn_SelFile();
	void OnBtn_OK();

	void OnCbx_ResType_SelChanged(EventArgs *pEvt);

	virtual void EndDialog(UINT uRetCode)
	{
		AnimateHostWindow(200,AW_CENTER|AW_HIDE);
		__super::EndDialog(uRetCode);
	}

    EVENT_MAP_BEGIN()
		EVENT_ID_COMMAND(IDOK, OnBtn_OK)
		EVENT_NAME_COMMAND(L"btn_sel_file", OnBtn_SelFile)
        EVENT_NAME_HANDLER(L"cbx_res_type", EVT_CB_SELCHANGE, OnCbx_ResType_SelChanged)
    EVENT_MAP_END()	

	BEGIN_MSG_MAP_EX(CNewFileDlg)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CMainDlg *m_pMainDlg;
};
