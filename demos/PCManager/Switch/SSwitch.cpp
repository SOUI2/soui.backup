#include "stdafx.h"
#include "SSwitch.h"

namespace SOUI
{

SSwitch::SSwitch(): m_BOpen(FALSE), m_BChangeing(FALSE), m_Iinterval(20), m_FrameCount(5), m_FrameNow(5),m_pSkin(NULL),m_pSkinForce(NULL)
{

}

SSwitch::~SSwitch()
{

}

void SSwitch::OnPaint( IRenderTarget *pRT )
{
    CRect rcWnd = GetWindowRect();
	if (m_pSkin)
		m_pSkin->Draw(pRT, rcWnd, 0);
	if (!m_pSkinForce) 
		return;
	UINT uState = _GetDrawState();
	SIZE skSize = m_pSkinForce->GetSkinSize();
	int dwSpace = rcWnd.Width() - skSize.cx ;
	CRect dwRet;
	dwRet.left =(m_FrameNow * dwSpace / m_FrameCount );
	dwRet.top = 0;
	dwRet.right = dwRet.left + skSize.cx ;
	dwRet.bottom = skSize.cy;
	POINT dwOff = rcWnd.TopLeft();
	dwRet.OffsetRect(dwOff);
	m_pSkinForce->Draw(pRT,dwRet,(DWORD)uState);
}

void SSwitch::OnTimer( char cTimerID )
{
	if (m_BChangeing)
	{
		(m_BOpenTarget) ? m_FrameNow-- : m_FrameNow++;
		if (m_FrameNow <= 0)
		{
			m_FrameNow = 0;
			m_BChangeing = FALSE;
			m_BOpen = m_BOpenTarget;
			KillTimer(1);
		}
		else if (m_FrameNow >= m_FrameCount)
		{
			m_FrameNow = m_FrameCount;
			m_BChangeing = FALSE;
			m_BOpen = m_BOpenTarget;
			KillTimer(1);
		}
	} 
	else
		KillTimer(1);
	InvalidateRect(NULL);
}

CSize SSwitch::GetDesiredSize( LPCRECT pRcContainer )
{
	CSize szRet;
	if(m_pSkin) szRet=m_pSkin->GetSkinSize();
	return szRet;
}

HRESULT SSwitch::OnAttrOpen( const SStringW& strValue, BOOL bLoading )
{
	m_BOpen = (strValue != L"0");
	m_FrameNow = m_BOpen ? 0 : m_FrameCount;
	return S_FALSE;
}

void SSwitch::OnLButtonUp( UINT nFlags, CPoint point )
{
	SetOpen(!m_BOpen);
	__super::OnLButtonUp(nFlags,point);
}

void SSwitch::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if(nChar==VK_SPACE)
	{
		SetOpen(!m_BOpen);
	}
}

void SSwitch::SetOpen( BOOL bOpen )
{
	m_BOpenTarget = bOpen;
	m_BChangeing = TRUE;

	SetTimer(1,m_Iinterval);
}

UINT SSwitch::_GetDrawState()
{
	DWORD dwState = GetState();
	UINT uState = 0;
	
	if (dwState & WndState_Disable)
		uState +=4;
	else
	{
		if (dwState & WndState_Hover)
		{
			uState +=1;
			if (dwState & WndState_PushDown)
				uState +=1;
		}
	}
	if (m_BOpen)
		uState +=4;
	
	return uState;
}

void SSwitch::OnStateChanged( DWORD dwOldState,DWORD dwNewState )
{
	InvalidateRect(NULL);
	__super::OnStateChanged(dwOldState, dwNewState);
}


}