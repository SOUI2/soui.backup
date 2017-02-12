#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"
//#include "control/SRichEdit.h"
//#include "extend.ctrls/SImageEx.h"

namespace SOUI
{
	class SDlgInput: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgInput,L"dlginput")
	public:
		SDlgInput();

		~SDlgInput(void)
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

		EVENT_MAP_BEGIN()

			EVENT_ID_COMMAND(IDOK,OnOK)
			//EVENT_ID_COMMAND(IDCANCEL,OnCancel)
			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SDlgInput)
			MSG_WM_INITDIALOG(OnInitDialog)
			//MSG_WM_CLOSE(OnCancel)
			//MSG_WM_KEYDOWN(OnKeyDown)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:

	public:
		SStringT m_strValue;	
		SEdit *m_edt;
	};

}