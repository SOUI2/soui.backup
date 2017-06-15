#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"
//#include "control/SRichEdit.h"
//#include "extend.ctrls/SImageEx.h"

namespace SOUI
{
	class SDlgNewSkin: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgNewSkin,L"dlgnewskin")
	public:
		SDlgNewSkin(LPCTSTR pszXmlName);

		~SDlgNewSkin(void)
		{

		}

		void OnBtnDlgOpenFile();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
		//virtual INT_PTR DoModal(HWND hParent=NULL);

		//virtual void EndDialog(INT_PTR nResult);


	protected:
		//void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnOK();
		void OnCancel();
		//virtual SMessageLoop * GetMsgLoop(){return m_MsgLoop;}

		EVENT_MAP_BEGIN()
			
			EVENT_NAME_COMMAND(L"btnOK", OnOK)
			EVENT_NAME_COMMAND(L"btnCancel",OnCancel)
			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SDlgNewSkin)
			MSG_WM_INITDIALOG(OnInitDialog)
			//MSG_WM_CLOSE(OnCancel)
			//MSG_WM_KEYDOWN(OnKeyDown)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:

	public:
		SStringT m_strSkinName;	
		SListBox *m_lbMain;
	};

}