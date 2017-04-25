#include "StdAfx.h"
#include "RealWndDlg.h"




CRealWndDlg::CRealWndDlg():SHostDialog(_T("LAYOUT:XML_RealwndDialog"))								
{
	m_menu_open.LoadMenu(_T("LAYOUT:menu_open"));
}

CRealWndDlg::~CRealWndDlg(void)
{
}
void CRealWndDlg::On_Menu_Open()
{
	
	
	CRect rc_menu;
	SWindow * pBtn = FindChildByID(401);
	if (pBtn)
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);
		m_menu_open.TrackPopupMenu(0, rc_menu.left, rc_menu.bottom, m_hWnd);
	}
}
void CRealWndDlg::On_Open_Files()
{
	
	::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)1);
}
void CRealWndDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{

	if (uNotifyCode == 0)
	{
		if (nID == 2000)
		{
			::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)2);
		}
		
		
		
	}
}
// 键盘消息
void CRealWndDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	
	case  VK_ESCAPE://退出全屏
	{
		::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)10);
	}break;
	case VK_SPACE://空格暂停
	{
		::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)11);
	}
	break;
	case VK_LEFT:  //快退
		{
			::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)15);
		}
		break;
	case VK_RIGHT://快进
		{
			::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)16);
		}
		break;
	}
}
void CRealWndDlg::OnLbuttonDBLCLK(UINT nFlags, CPoint pt)
{
		
	if (GetWindowRect().PtInRect(pt))//双击全屏/退出
		::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)12);
	SetMsgHandled(false);

}
void CRealWndDlg::OnLbuttonup(UINT nFlags, CPoint pt)
{

	if (GetWindowRect().PtInRect(pt))//单击播放/暂停
		::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND, 0, (LPARAM)(int)14);
	SetMsgHandled(false);

}

