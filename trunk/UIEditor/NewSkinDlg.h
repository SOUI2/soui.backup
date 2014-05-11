#pragma once

class CMainDlg;

class CNewSkinDlg : public CDuiHostWnd
{
public:
	CNewSkinDlg(CMainDlg *pMainDlg);
	~CNewSkinDlg(void);

protected:
	void UpdateImglstPreview();
	void UpdateImgframePreview();

	LRESULT OnInitDialog(HWND wnd, LPARAM lParam);

	LRESULT OnEditNotify_imglst(LPDUINMHDR pNHdr);
	LRESULT OnEditNotify_imgframe(LPDUINMHDR pNHdr);

	LRESULT OnCrChange_Button_Border(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Normal_Up(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Normal_Down(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Hover_Up(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Hover_Down(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Pushdown_Up(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Pushdown_Down(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Disable_Up(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Button_Disable_Down(LPDUINMHDR pNHdr);
	void SetButtonSkinColor(const CDuiStringA &strAttr,COLORREF cr);

	LRESULT OnCrChange_Gradataion_From(LPDUINMHDR pNHdr);
	LRESULT OnCrChange_Gradataion_To(LPDUINMHDR pNHdr);
	void OnGradationDir();

	void OnOK();

	DUI_NOTIFY_MAP_BEGIN()
		DUI_NOTIFY_NAME_HANDLER("edit_imglst_src_name",DUINM_RICHEDIT_NOTIFY,OnEditNotify_imglst)
		DUI_NOTIFY_NAME_HANDLER("edit_imglst_states",DUINM_RICHEDIT_NOTIFY,OnEditNotify_imglst)
		DUI_NOTIFY_NAME_COMMAND("chk_imglst_vert",UpdateImglstPreview)
		DUI_NOTIFY_NAME_COMMAND("chk_imglst_tile",UpdateImglstPreview)

		DUI_NOTIFY_NAME_HANDLER("edit_imgframe_src_name",DUINM_RICHEDIT_NOTIFY,OnEditNotify_imgframe)
		DUI_NOTIFY_NAME_HANDLER("edit_imgframe_states",DUINM_RICHEDIT_NOTIFY,OnEditNotify_imgframe)
		DUI_NOTIFY_NAME_HANDLER("edit_imgframe_frame",DUINM_RICHEDIT_NOTIFY,OnEditNotify_imgframe)
		DUI_NOTIFY_NAME_COMMAND("chk_imgframe_vert",UpdateImgframePreview)
		DUI_NOTIFY_NAME_COMMAND("chk_imgframe_tile",UpdateImgframePreview)

		DUI_NOTIFY_NAME_HANDLER("edit_imgframe_frame",DUINM_RICHEDIT_NOTIFY,OnEditNotify_imgframe)

		DUI_NOTIFY_NAME_HANDLER("crpk_bt1_up",DUINM_COLORCHANGE,OnCrChange_Button_Normal_Up)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt1_down",DUINM_COLORCHANGE,OnCrChange_Button_Normal_Down)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt2_up",DUINM_COLORCHANGE,OnCrChange_Button_Hover_Up)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt2_down",DUINM_COLORCHANGE,OnCrChange_Button_Hover_Down)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt3_up",DUINM_COLORCHANGE,OnCrChange_Button_Pushdown_Up)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt3_down",DUINM_COLORCHANGE,OnCrChange_Button_Pushdown_Down)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt4_up",DUINM_COLORCHANGE,OnCrChange_Button_Disable_Up)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt4_down",DUINM_COLORCHANGE,OnCrChange_Button_Disable_Down)
		DUI_NOTIFY_NAME_HANDLER("crpk_bt_border",DUINM_COLORCHANGE,OnCrChange_Button_Border)

		DUI_NOTIFY_NAME_HANDLER("crpk_gradation_from",DUINM_COLORCHANGE,OnCrChange_Gradataion_From)
		DUI_NOTIFY_NAME_HANDLER("crpk_gradation_to",DUINM_COLORCHANGE,OnCrChange_Gradataion_To)
		DUI_NOTIFY_NAME_COMMAND("chk_gradation_dir",OnGradationDir)

		DUI_NOTIFY_ID_COMMAND(IDOK,OnOK)
	DUI_NOTIFY_MAP_END()	

	BEGIN_MSG_MAP_EX(CNewSkinDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_DUI_NOTIFY()
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	CMainDlg *m_pMainDlg;
};
