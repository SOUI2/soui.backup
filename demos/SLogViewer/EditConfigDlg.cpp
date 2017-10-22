#include "stdafx.h"
#include "EditConfigDlg.h"
#include <helper/mybuffer.h>

CEditConfigDlg::CEditConfigDlg(void):SHostDialog(UIRES.LAYOUT.dlg_edit_config),m_pSciter(NULL)
{
}

CEditConfigDlg::~CEditConfigDlg(void)
{
}

static char bom_utf8[3]={0xef,0xbb,0xbf};

BOOL CEditConfigDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
	SStringW strConfig=SApplication::getSingleton().GetAppDir();
	strConfig += L"\\config.xml";

	SRealWnd * pRealWnd = FindChildByID2<SRealWnd>(R.id.real_scilexer);
	SASSERT(pRealWnd);
	m_pSciter = (CScintillaWnd *)pRealWnd->GetUserData();
	m_pSciter->SendMessage(SCI_USEPOPUP,0,0);

	FILE *f = _wfopen(strConfig,L"rb");
	if(f)
	{
		fseek(f,0,SEEK_END);
		int nLen = ftell(f);
		fseek(f,0,SEEK_SET);

		char *buf = (char*)malloc(nLen+1);
		fread(buf,1,nLen,f);
		fclose(f);
		buf[nLen] = 0;
		if(memcmp(buf,bom_utf8,3)==0)
		{
			m_pSciter->SendMessage(SCI_SETTEXT,0,(LPARAM)(LPCSTR)buf+3);
		}else
		{
			m_pSciter->SendMessage(SCI_SETTEXT,0,(LPARAM)(LPCSTR)buf);
		}
		free(buf);
	}

	return TRUE;
}

void CEditConfigDlg::OnSave()
{
	unsigned int len = m_pSciter->SendMessage(SCI_GETTEXT, 0, 0);
	char *chText = new char[len];
	m_pSciter->SendMessage(SCI_GETTEXT, len, (LPARAM)chText);

	SStringW strConfig=SApplication::getSingleton().GetAppDir();
	strConfig += L"\\config.xml";

	FILE *f =_wfopen(strConfig,L"w+b");
	if(f)
	{
		fwrite(chText,1,len-1,f);
		fclose(f);
	}
	delete []chText;
	EndDialog(IDOK);
}
