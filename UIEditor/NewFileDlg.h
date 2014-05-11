#pragma once
class CMainDlg;

class CNewFileDlg : public CDuiHostWnd
{
public:
	CNewFileDlg(CMainDlg *pMainDlg);
	~CNewFileDlg(void);

	CDuiStringT m_strSrcFile;
	CDuiStringT m_strResType;
	CDuiStringT m_strResName;
	CDuiStringT m_strResPath;
	BOOL		m_bLayout;
protected:
	void OnBtn_SelFile();
	void OnBtn_OK();

	LRESULT OnCbx_ResType_SelChanged(LPDUINMHDR pnmhdr);

	virtual void EndDialog(UINT uRetCode)
	{
		AnimateHostWindow(200,AW_CENTER|AW_HIDE);
		__super::EndDialog(uRetCode);
	}

	DUI_NOTIFY_MAP_BEGIN()
		DUI_NOTIFY_ID_COMMAND(IDOK, OnBtn_OK)
		DUI_NOTIFY_NAME_COMMAND("btn_sel_file", OnBtn_SelFile)
		DUI_NOTIFY_NAME_HANDLER("cbx_res_type", DUINM_LBSELCHANGED,OnCbx_ResType_SelChanged)
	DUI_NOTIFY_MAP_END()	

	BEGIN_MSG_MAP_EX(CNewFileDlg)
		MSG_DUI_NOTIFY()
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CMainDlg *m_pMainDlg;
};
