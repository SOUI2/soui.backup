// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "UIHander.h"



#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

CMainDlg::CMainDlg() : CDuiHostWnd(_T("IDR_DUI_MAIN_DIALOG"))
{
	m_pUiHandler = new CUIHander(this);
	m_bLayoutInited=FALSE;
} 

CMainDlg::~CMainDlg()
{
	delete m_pUiHandler; 
}

int CMainDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	// 		MARGINS mar = {5,5,30,5};
	// 		DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	SetMsgHandled(FALSE);
	return 0;
}

void CMainDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if(bShow)
	{
		AnimateHostWindow(200,AW_CENTER);
	}
}

