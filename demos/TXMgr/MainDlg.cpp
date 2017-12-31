// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "GSSkin.h"
#include "GSTabCtrl.h"
	
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_nCloseAnime = 0;
	m_nMoveNumber = 0;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;

	m_pTrayDlg = new CTrayDlg;
	m_pTrayDlg->Create(m_hWnd,WS_POPUP);
	m_pTrayDlg->ShowWindow(SW_SHOW);
	m_pTrayDlg->SendMessage(WM_INITDIALOG);
	return 0;
}


//TODO:消息映射
void CMainDlg::OnClose()
{
	//CSimpleWnd::DestroyWindow();
	SetTimer(close_animation_timer, 30);
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}


void CMainDlg::OnTimer(char nIDEvent)
{
	SetMsgHandled(FALSE);
	if (nIDEvent == close_animation_timer)
	{
		SImageWnd* pWnd = FindChildByName2<SImageWnd>("close_animation");
		if (pWnd)
		{
			pWnd->SetAttribute(L"show", L"1");
			GSSkinImgList* pSkin = (GSSkinImgList*)pWnd->GetSkin();
			pSkin->OnSetSkinState(m_nCloseAnime++);
			pWnd->Invalidate();

			if (m_nCloseAnime == 12)
			{
				SWindow* pWndMain = FindChildByName("main_wnd");
				pWndMain->AnimateWindow(100, AW_HIDE | AW_SLIDE | AW_HOR_POSITIVE);
			}

			if (m_nCloseAnime == 28)
			{
				KillTimer(nIDEvent);
				CSimpleWnd::DestroyWindow();
			}
			
		}
	}
	else if (switch_mode_pioneer == nIDEvent)
	{
		CRect rect = GetWindowRect();
		if (rect.Width() > 920)
		{
			SWindow* pWnd = FindChildByName("switchmode_pioneer");
			if (pWnd)
			{
				pWnd->SetAttribute(L"skin", L"skin_switchmode_classic");
			}

			SWindow* pWndTab = FindChildByName("tab_main"); 
			if (pWndTab)
			{
				pWndTab->SetAttribute(L"tabWidth", L"150");
				pWndTab->SetAttribute(L"tab_show_name", L"1");
				pWndTab->SetAttribute(L"anihover", L"skin_table_item_c_hover");
				pWndTab->Invalidate();
			}

			m_nMoveNumber = 0;
			KillTimer(nIDEvent);
		}
		else
		{
			if (m_nMoveNumber == 0)
			{
				SWindow* pWnd = FindChildByName("main_wnd_show");
				if (pWnd)
				{
					ISkinObj* pSkin = GETSKIN(L"skin_wnd_bg",100);
					pSkin->SetAttribute(L"offset", L"0,0", FALSE);
					pWnd->Invalidate();

				}
			}
			if (m_nMoveNumber < 11)
			{
				
				SetWindowPos(HWND_NOTOPMOST, rect.left - 25, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
			}
			else
			{
				SetWindowPos(HWND_NOTOPMOST, rect.left, rect.top, rect.Width() + 30, rect.Height(), SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
			}

			m_nMoveNumber++;
		}		
	}
	else if (switch_mode_classic == nIDEvent)
	{
		CRect rect = GetWindowRect();
		if (rect.Width() <= 394)
		{
			SWindow* pWnd = FindChildByName("switchmode_pioneer");
			if (pWnd)
			{
				pWnd->SetAttribute(L"skin", L"skin_switchmode_pioneer");
			}

			if (m_nMoveNumber == 0)
			{
				SWindow* pWnd = FindChildByName("main_wnd_show");
				if (pWnd)
				{
					ISkinObj* pSkin = GETSKIN(L"skin_wnd_bg",100);
					pSkin->SetAttribute(L"offset", L"0.32,0", FALSE);
					pWnd->Invalidate();

				}
			}

			m_nMoveNumber++;

			SetWindowPos(HWND_NOTOPMOST, rect.left + 25, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW | SWP_NOSENDCHANGING);

			if (m_nMoveNumber >= 11)
			{
				m_nMoveNumber = 0;
				KillTimer(nIDEvent);
			}			
		}
		else
		{
			SWindow* pWndTab = FindChildByName("tab_main");
			if (pWndTab)
			{
				pWndTab->SetAttribute(L"tabWidth", L"58");
				pWndTab->SetAttribute(L"tab_show_name", L"0");
				pWndTab->SetAttribute(L"anihover", L"skin_table_item_hover");

				pWndTab->Invalidate();
			}

			SetWindowPos(HWND_NOTOPMOST, rect.left, rect.top, rect.Width() - 30, rect.Height(), SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
	}
}

void CMainDlg::OnMainTabSelChange(EventArgs *pEvt)
{
	EventTabSelChanged *pEvtTab = (EventTabSelChanged*)pEvt;
	GSTabCtrl* pTabCtrl = (GSTabCtrl*)(pEvt->sender);

	SWindow* pWndTitle1 = FindChildByName("logo_title_1");
	SWindow* pWndTitle2 = FindChildByName("logo_title_2");

	if (pWndTitle1 && pWndTitle2)
	{
		pWndTitle1->SetAttribute(L"show", (pEvtTab->uNewSel != 0) ? L"0" : L"1");

		pWndTitle2->SetAttribute(L"show", (pEvtTab->uNewSel != 0) ? L"1" : L"0");
	}
}

void CMainDlg::OnSwitchModePioneer()
{
	CRect rect = GetWindowRect();

	if (rect.Width() <= 394)
	{
		SetTimer(switch_mode_pioneer, 10);
	}
	else
	{
		SetTimer(switch_mode_classic, 10);
	}

}





