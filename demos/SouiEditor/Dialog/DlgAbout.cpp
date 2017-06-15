#include "stdafx.h"
#include "DlgAbout.h"
#include "CDebug.h"

namespace SOUI
{

	SDlgAbout::SDlgAbout():
		SHostDialog(_T("LAYOUT:UIDESIGNER_XML_ABOUT"))
	{
	}

	//TODO:œ˚œ¢”≥…‰
	void SDlgAbout::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgAbout::OnOK()
	{

		SHostDialog::OnOK();
	}

	BOOL SDlgAbout::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{

		return TRUE;
	}

}


