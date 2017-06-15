#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"
#include "control/SRichEdit.h"
#include "SImageEx.h"

namespace SOUI
{
	class SDlgNewLayout: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgNewLayout, L"dlgnewlayout")
	public:
		SDlgNewLayout(LPCTSTR pszXmlName, SStringT strProPath);

		~SDlgNewLayout(void)
		{
		}


		void OnClose();
		void OnBtnDlgOpenFile();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
		//virtual INT_PTR DoModal(HWND hParent=NULL);

		//virtual void EndDialog(INT_PTR nResult);


	protected:
		//void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnOK();
		//void OnCancel();
		//virtual SMessageLoop * GetMsgLoop(){return m_MsgLoop;}
		void OnResNameInputNotify(EventArgs *e);

		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_close", OnClose)

			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_dlg", OnBtnDlgOpenFile)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_OK", OnOK)
			//EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_ZY_NEW", OnZYNew)
			//EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_ZY_DEL", OnZYDel)
			//EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_SKIN_NEW", OnSkinNew)
			//EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_SKIN_DEL", OnSkinDel)

			//EVENT_ID_COMMAND(IDOK,OnOK)
			//EVENT_ID_COMMAND(IDCANCEL,OnCancel)
			EVENT_ID_HANDLER(R.id.new_layout_resname, EventRENotify::EventID, OnResNameInputNotify)
		EVENT_MAP_END()

		BEGIN_MSG_MAP_EX(SDlgSkinSelect)
			MSG_WM_INITDIALOG(OnInitDialog)
			//MSG_WM_CLOSE(OnCancel)
			//MSG_WM_KEYDOWN(OnKeyDown)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()

	protected:
		SStringT m_strProPath;

		SEdit *m_edtName;
		SEdit *m_edtPath;




	public:
		SStringT m_strPath;
		SStringT m_strName;		
	};

}