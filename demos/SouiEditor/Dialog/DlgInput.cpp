#include "stdafx.h"
#include "DlgInput.h"
#include "CDebug.h"

namespace SOUI
{

	SDlgInput::SDlgInput():SHostDialog(_T("LAYOUT:UIDESIGNER_XML_INPUT"))
	{

	}

	//TODO:消息映射
	void SDlgInput::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgInput::OnOK()
	{
	
		m_strValue = m_edt->GetWindowText();
		if (m_strValue.IsEmpty())
		{
			CDebug::Debug(_T("请输入内容"));
			return;
		}
		SHostDialog::OnOK();
	}

	BOOL SDlgInput::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_edt = FindChildByName2<SEdit>(L"edtInput");

		return TRUE;
	}

}


