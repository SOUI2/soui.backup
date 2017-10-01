#include "stdafx.h"
#include "GSAnimButton.h"


GSAnimButton::GSAnimButton()
{
	m_nHoveState = 0;
	m_nHoveAnim = 0;
	m_bAnimate = 1;
	m_bDrawFocusRect = FALSE;
	m_nElapseTime = 0;
}


GSAnimButton::~GSAnimButton()
{
}

void GSAnimButton::OnPaint(IRenderTarget *pRT)
{
	if (!m_pBgSkin) return;
	CRect rcClient;
	GetClientRect(&rcClient);

	if (m_bAnimate == 0)
	{//不在动画过程中
		m_pBgSkin->Draw(
			pRT, rcClient,
			IIF_STATE4(GetState(), 0, 1, 2, 3)
			);
	}
	else
	{//在动画过程中
		if (GetState()&WndState_Hover)
		{
			//get hover
			m_pBgSkin->Draw(pRT, rcClient, m_nHoveState, m_pBgSkin->GetAlpha());
		}
		else
		{
			if (m_nHoveAnim == 0)
			{
				m_pBgSkin->Draw(pRT, rcClient, IIF_STATE4(GetState(), 0, 1, m_pBgSkin->GetStates() - 1, m_pBgSkin->GetStates() - 1));
			}
			else
			{
				m_pBgSkin->Draw(pRT, rcClient, m_nHoveState, m_pBgSkin->GetAlpha());
			}
		}
	}

	SWindow::OnPaint(pRT);
}

void GSAnimButton::OnStateChanged(DWORD dwOldState, DWORD dwNewState)
{
	SWindow::OnStateChanged(dwOldState, dwNewState);
	

	if (GetCapture() == m_swnd)    //点击中
		return;

	if (m_bAnimate &&
		((dwOldState == WndState_Normal && dwNewState == WndState_Hover)))
	{
		StopCurAnimate();
		m_nHoveAnim = 1;
		GetContainer()->RegisterTimelineHandler(this);
	}
	else if (m_bAnimate && (dwOldState == WndState_Hover && dwNewState == WndState_Normal))
	{//启动动画
		m_nHoveAnim = -1;
		GetContainer()->RegisterTimelineHandler(this);
	}
}

//中止原来的动画
void GSAnimButton::StopCurAnimate()
{
	GetContainer()->UnregisterTimelineHandler(this);
	m_nHoveAnim = 0;
}

void GSAnimButton::OnNextFrame()
{
	m_nElapseTime++;
	if (m_nElapseTime < 3)
	{
		return;
	}

	m_nElapseTime = 0;
	m_nHoveState += m_nHoveAnim;

	Invalidate();

	if ((m_nHoveAnim == 1 && m_nHoveState >= m_pBgSkin->GetStates() - 2) || (m_nHoveAnim == -1 && m_nHoveState <= 0))
	{
		StopCurAnimate();
	}

	
}


