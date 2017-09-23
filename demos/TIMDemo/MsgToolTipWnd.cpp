#include "StdAfx.h"
#include "MsgToolTipWnd.h"

CMsgToolTipWnd::CMsgToolTipWnd(std::function<void(UINT)> fun)
	: SHostWnd(_T("layout:wnd_msgtooltip"))//这里定义主界面需要使用的布局文件 在uires.idx文件中定义的
	, m_funClick(fun)
	, m_uSenderId(0)
	, m_uTotalCount(0)
{
	
}


CMsgToolTipWnd::~CMsgToolTipWnd(void)
{
}



BOOL CMsgToolTipWnd::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{
	SStatic* pTitle;
	InitWnd<SStatic>(pTitle, L"text_title");
	pTitle->SetWindowText((LPCTSTR)lInitParam);

	InitWnd<SStatic>(m_pTextSender, L"text_sender");
	InitWnd<SStatic>(m_pTextSendMsg, L"text_content");
	InitWnd<SImageWnd>(m_pImgUser, L"img_user");
	InitWnd(m_pWinTotalCount, L"win_totalcount");
	InitWnd(m_pWinCount, L"win_count");

	m_pImgUser->SetAttribute(L"skin", L"default");

	return TRUE;
}

void CMsgToolTipWnd::MoveButtomRight(int nPadding/*=10*/)
{
	HMONITOR hMonitor = ::MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);

	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);
	::GetMonitorInfo(hMonitor, &minfo);
	
	CRect rc = GetClientRect();
	rc.OffsetRect(minfo.rcWork.right - rc.Width()-nPadding, minfo.rcWork.bottom - rc.Height()-nPadding);
	MoveWindow(&rc);
}

void CMsgToolTipWnd::OnBtnIgnore()
{
	m_uTotalCount = 0;
	m_uSenderId = 0;
	m_mapSenderCount.clear();

	if(nullptr != m_funClick)
		m_funClick(0);

	ShowWindow(FALSE);
}

void CMsgToolTipWnd::OnBtnViewAll()
{
	if(nullptr != m_funClick)
		m_funClick(m_uSenderId);

	OnBtnIgnore();
}

void CMsgToolTipWnd::ShowUnreadMsg()
{
	if(0 == m_uTotalCount)
		return ;

	if(!IsWindowVisible())
	{
		SetForegroundWindow(m_hWnd);
		ShowWindow(SW_SHOWNOACTIVATE);
	}
}

SStringT toFormatCount(UINT uCount)
{
	SStringT szCount;
	if(uCount > 99)
		szCount = _T("99+");
	else
		szCount.Format(_T("%d"), uCount);

	return szCount;
}

void CMsgToolTipWnd::AddUnreadMsg(UINT uUserId, LPCTSTR lpUserAlias, LPCTSTR lpContent, UINT uCount)
{
	m_uSenderId = uUserId;
	m_pTextSender->SetWindowText(lpUserAlias);
	m_pTextSendMsg->SetWindowText(lpContent);
	
	m_uTotalCount += uCount;							// 总数 +
	m_mapSenderCount[uUserId] += uCount;			// 该用户 +

	UINT uHadCount = m_mapSenderCount[uUserId];
	
	m_pWinTotalCount->SetWindowTextW(toFormatCount(m_uTotalCount));
	m_pWinCount->SetWindowTextW(toFormatCount(uHadCount));
}

void CMsgToolTipWnd::SetTotalCount(UINT uCount)
{
	m_uTotalCount = uCount;
}

