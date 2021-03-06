// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TrayDlg.h"
	
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

	
CTrayDlg::CTrayDlg() : SHostWnd(_T("LAYOUT:XML_TRAYWND"))
{
	m_bLayoutInited = FALSE;
}

CTrayDlg::~CTrayDlg()
{
}

int CTrayDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

BOOL CTrayDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	//获取屏幕的分辨率   
	m_nFullWidth = GetSystemMetrics(SM_CXSCREEN);
	m_nFullHeight = GetSystemMetrics(SM_CYSCREEN);

	SetWindowPos(HWND_TOPMOST, m_nFullWidth - 400, 0, 200, m_nFullHeight - 100, SWP_SHOWWINDOW | SWP_NOSENDCHANGING);

	SetTimer(Update_Update_Speed, 500);
	SetTimer(Update_Down_Speed, 500);
	SetTimer(Update_Use_Speed, 1000);

	SWindow* pWndUseBg = FindChildByName("btn_use_bg");
	if (pWndUseBg)
	{
		pWndUseBg->GetEventSet()->subscribeEvent(EventSwndMouseHover::EventID, Subscriber(&CTrayDlg::OnUseBgMouseHover, this));
		pWndUseBg->GetEventSet()->subscribeEvent(EventSwndMouseLeave::EventID, Subscriber(&CTrayDlg::OnUseBgMouseLeave, this));
	}

	m_bLayoutInited = TRUE;
	return 0;
}

void CTrayDlg::OnLButtonDown(UINT nFlags, CPoint pt)
{
	SetMsgHandled(FALSE);
	//__super::OnLButtonDown(nFlags, pt);
}


void CTrayDlg::OnLButtonUp(UINT nFlags, CPoint pt)
{
	SetMsgHandled(FALSE);
	SWindow* pWndUseBg = FindChildByName("btn_use_bg");
	if (pWndUseBg)
	{
		CRect rectUse = pWndUseBg->GetWindowRect();
		if (rectUse.PtInRect(pt))
		{
			OnUseBg();
		}
	}
}


bool CTrayDlg::OnUseBgMouseHover(EventArgs *e)
{
	SWindow* pWndUse = FindChildByName("use");
	if (pWndUse)
	{
		pWndUse->SetAttribute(L"show", L"0");
	}
	return 1;
}

bool CTrayDlg::OnUseBgMouseLeave(EventArgs *e)
{
	SWindow* pWndUse = FindChildByName("use");
	if (pWndUse)
	{
		pWndUse->SetAttribute(L"show", L"1");
	}
	return 1;
}

void CTrayDlg::OnUseBg()
{
	SRocketAnimator * rocket = FindChildByName2<SRocketAnimator>(L"rocket");
	rocket->Fire();
}


void CTrayDlg::OnTimer(char nIDEvent)
{
	SetMsgHandled(FALSE);
	int nRand = rand() % 100;
	if (nIDEvent == Update_Update_Speed)
	{
		SStringW strSpeed;
		strSpeed.Format(L"%d.%d", nRand / 10, nRand % 10);
		SWindow* pWnd = FindChildByName("update_speed");
		if (pWnd)	pWnd->SetWindowTextW(strSpeed);

	}
	else if (nIDEvent == Update_Down_Speed)
	{
		SStringW strSpeed;
		strSpeed.Format(L"%d.%d", nRand / 10, nRand % 10);
		SWindow* pWnd = FindChildByName("down_speed");
		if (pWnd)	pWnd->SetWindowTextW(strSpeed);
	}
	else if (nIDEvent == Update_Use_Speed)
	{
		SStringW strSkinTen, strSkinUnit;
		strSkinTen.Format(L"skin.number_%d", nRand / 10);
		strSkinUnit.Format(L"skin.number_%d", nRand % 10);
		SWindow* pWndTen = FindChildByName("use_ten");
		SWindow* pWndUnit = FindChildByName("use_unit");
		if (pWndTen)	pWndTen->SetAttribute(L"skin", strSkinTen);
		if (pWndUnit)	pWndUnit->SetAttribute(L"skin", strSkinUnit);
	}

}

void CTrayDlg::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void CTrayDlg::OnRocketVisibleChange(EventArgs *e)
{
	SWindow *pSender = sobj_cast<SWindow>(e->sender);
	SWindow* pWndSpeed = FindChildByName("speed_wnd");
	if (pWndSpeed)
	{
		pWndSpeed->SetVisible(!pSender->IsVisible(FALSE));
	}

}


