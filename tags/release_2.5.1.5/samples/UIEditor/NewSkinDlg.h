#pragma once

class CMainDlg;

class CNewSkinDlg : public SHostDialog
{
public:
	CNewSkinDlg(CMainDlg *pMainDlg);
	~CNewSkinDlg(void);

protected:
	void UpdateimglistPreview();
	void UpdateImgframePreview();

	LRESULT OnInitDialog(HWND wnd, LPARAM lParam);

	void OnEditNotify_imglist(EventArgs * pEvt);
	void OnEditNotify_imgframe(EventArgs * pEvt);

	void OnCrChange_Button_Border(EventArgs * pEvt);
	void OnCrChange_Button_Normal_Up(EventArgs * pEvt);
	void OnCrChange_Button_Normal_Down(EventArgs * pEvt);
	void OnCrChange_Button_Hover_Up(EventArgs * pEvt);
	void OnCrChange_Button_Hover_Down(EventArgs * pEvt);
	void OnCrChange_Button_Pushdown_Up(EventArgs * pEvt);
	void OnCrChange_Button_Pushdown_Down(EventArgs * pEvt);
	void OnCrChange_Button_Disable_Up(EventArgs * pEvt);
	void OnCrChange_Button_Disable_Down(EventArgs * pEvt);
	void SetButtonSkinColor(const SStringW &strAttr,COLORREF cr);

	void OnCrChange_Gradataion_From(EventArgs * pEvt);
	void OnCrChange_Gradataion_To(EventArgs * pEvt);
	void OnGradationDir();

	void OnOK();

    EVENT_MAP_BEGIN()
		EVENT_NAME_HANDLER(L"edit_imglist_src_name",EVT_RE_NOTIFY,OnEditNotify_imglist)
		EVENT_NAME_HANDLER(L"edit_imglist_states",EVT_RE_NOTIFY,OnEditNotify_imglist)
		EVENT_NAME_COMMAND(L"chk_imglist_vert",UpdateimglistPreview)
		EVENT_NAME_COMMAND(L"chk_imglist_tile",UpdateimglistPreview)

		EVENT_NAME_HANDLER(L"edit_imgframe_src_name",EVT_RE_NOTIFY,OnEditNotify_imgframe)
		EVENT_NAME_HANDLER(L"edit_imgframe_states",EVT_RE_NOTIFY,OnEditNotify_imgframe)
		EVENT_NAME_HANDLER(L"edit_imgframe_frame",EVT_RE_NOTIFY,OnEditNotify_imgframe)
		EVENT_NAME_COMMAND(L"chk_imgframe_vert",UpdateImgframePreview)
		EVENT_NAME_COMMAND(L"chk_imgframe_tile",UpdateImgframePreview)

		EVENT_NAME_HANDLER(L"edit_imgframe_frame",EVT_RE_NOTIFY,OnEditNotify_imgframe)

        EVENT_NAME_HANDLER(L"crpk_bt1_up",EventColorChange::EventID,OnCrChange_Button_Normal_Up)
		EVENT_NAME_HANDLER(L"crpk_bt1_down",EventColorChange::EventID,OnCrChange_Button_Normal_Down)
		EVENT_NAME_HANDLER(L"crpk_bt2_up",EventColorChange::EventID,OnCrChange_Button_Hover_Up)
		EVENT_NAME_HANDLER(L"crpk_bt2_down",EventColorChange::EventID,OnCrChange_Button_Hover_Down)
		EVENT_NAME_HANDLER(L"crpk_bt3_up",EventColorChange::EventID,OnCrChange_Button_Pushdown_Up)
		EVENT_NAME_HANDLER(L"crpk_bt3_down",EventColorChange::EventID,OnCrChange_Button_Pushdown_Down)
		EVENT_NAME_HANDLER(L"crpk_bt4_up",EventColorChange::EventID,OnCrChange_Button_Disable_Up)
		EVENT_NAME_HANDLER(L"crpk_bt4_down",EventColorChange::EventID,OnCrChange_Button_Disable_Down)
		EVENT_NAME_HANDLER(L"crpk_bt_border",EventColorChange::EventID,OnCrChange_Button_Border)

		EVENT_NAME_HANDLER(L"crpk_gradation_from",EventColorChange::EventID,OnCrChange_Gradataion_From)
		EVENT_NAME_HANDLER(L"crpk_gradation_to",EventColorChange::EventID,OnCrChange_Gradataion_To)
		EVENT_NAME_COMMAND(L"chk_gradation_dir",OnGradationDir)

		EVENT_ID_COMMAND(IDOK,OnOK)
    EVENT_MAP_END()

	BEGIN_MSG_MAP_EX(CNewSkinDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
	CMainDlg *m_pMainDlg;
};
