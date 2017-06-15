#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"

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


		void OnBtnDlgOpenFile();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);



	protected:
		void OnOK();
		void OnClose();

		EVENT_MAP_BEGIN()

			EVENT_NAME_COMMAND(L"btnOK", OnOK)
			EVENT_NAME_COMMAND(L"btnCancel", OnClose)
			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SDlgInput)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:

	public:
		SStringT m_strValue;	
		SEdit *m_edt;
	};

}