#pragma once

class CMainDlg;

struct SkinInfo
{
	CDuiStringT strType;
	CDuiStringT strName;
	CDuiStringT strSrc;
	int			nState;
	BOOL		bTile;
	CRect		rcMargin;
};

class CUIHander
{
public:
	CUIHander(CMainDlg *pMainDlg);
	~CUIHander(void);

protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnDestroy();

	void OnBtnClick_ProjectOpen();
	LRESULT OnListCtrl_File_SelChanged(LPDUINMHDR pnmh);
	LRESULT OnListCtrl_Skin_SelChanged(LPDUINMHDR pnmh);


	BEGIN_MSG_MAP_EX(CUIHander)
		MSG_DUI_NOTIFY()
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	DUI_NOTIFY_MAP_BEGIN()
		DUI_NOTIFY_NAME_HANDLER("prj_list_file",DUINM_LBSELCHANGED,OnListCtrl_File_SelChanged);
		DUI_NOTIFY_NAME_HANDLER("prj_list_skin",DUINM_LBSELCHANGED,OnListCtrl_Skin_SelChanged);
		DUI_NOTIFY_NAME_COMMAND("prg_btn_open", OnBtnClick_ProjectOpen)
	DUI_NOTIFY_MAP_END()	
private:
	void ClearSkinList();
	CDuiStringT GetImageSrcFile(const CDuiStringT & strSrcName);

	CDuiStringT	m_strPrjPath;	//皮肤路径
	CDuiStringT m_strInitFile;	//皮肤的初始化文件名

	CMainDlg * m_pMainDlg; 
};
