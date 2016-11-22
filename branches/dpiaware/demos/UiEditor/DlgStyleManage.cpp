#include "stdafx.h"
#include "DlgStyleManage.h"
#include "CDebug.h"

namespace SOUI
{

	SDlgStyleManage::SDlgStyleManage():SHostDialog(_T("layout:样式管理"))
	{

	}


	//TODO:消息映射
	void SDlgStyleManage::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgStyleManage::OnMaximize()
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
	}
	void SDlgStyleManage::OnRestore()
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE);
	}
	void SDlgStyleManage::OnMinimize()
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	}

	void SDlgStyleManage::OnOK()
	{
		//int n = m_lbMain->GetCurSel();
		//if (n >= 0)
		//{
		//	m_lbMain->GetText(n, m_strSkinName);
		//}
		//else
		//{
		//	CDebug::Debug(_T("请选择其中一项"));
		//	return;
		//}


		SHostDialog::OnOK();
	}

	BOOL SDlgStyleManage::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		//m_lbMain = FindChildByName2<SListBox>(L"NAME_UIDESIGNER_NEW_SKIN_LB");


		return TRUE;
	}

}


