#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"

namespace SOUI
{
	class SDlgStyleManage: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgStyleManage,L"dlgstylemanage")
	public:
		SDlgStyleManage();

		~SDlgStyleManage(void)
		{

		}

		void OnClose();
		void OnMaximize();
		void OnRestore();
		void OnMinimize();



		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);


	protected:

		void OnOK();


		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_close", OnClose)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_restore", OnRestore)



			EVENT_ID_COMMAND(IDOK,OnOK)

			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SDlgStyleManage)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:


	public:

	};

}