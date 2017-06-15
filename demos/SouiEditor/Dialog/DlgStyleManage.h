#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"

class ResManger;

namespace SOUI
{
	class SDlgStyleManage: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgStyleManage,L"dlgstylemanage")
	public:
		SDlgStyleManage(SStringT strClassName, SStringT strPath, BOOL bGetClass);

		~SDlgStyleManage(void)
		{

		}

		void OnClose();
		void OnBtnAdd();
		void OnBtnDel();
		void OnBtnSave();


		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	protected:

		void OnOK();

		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"btnClose", OnClose)
			EVENT_NAME_COMMAND(L"btnOK", OnOK)
			EVENT_NAME_COMMAND(L"btnAdd", OnBtnAdd)
			EVENT_NAME_COMMAND(L"btnDel", OnBtnDel)
			EVENT_NAME_COMMAND(L"btnSave", OnBtnSave)

			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SDlgStyleManage)
			MSG_WM_INITDIALOG(OnInitDialog)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:

	public:
		void InitStyleLB();
		SStringT GetLBCurSelText(SListBox * lb);

	public:
		ResManger* m_pResFileManger;	//所有资源文件的管理

		SListBox*  m_lbStyle;
		SEdit*     m_edtSearch;
		SRealWnd*  m_RealWnd;
		SWindow*   m_wndView;

		SStringT m_strProPath;
		SStringT m_strUIResFile;
		SStringT m_strStyleName;
		SStringT m_strStyleFile;
		BOOL m_bGetStyle;

	};

}