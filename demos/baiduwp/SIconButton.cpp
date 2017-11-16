#include "StdAfx.h"
#include "SIconButton.h"


SIconButton::SIconButton(void)
	:m_pSkinIcon(NULL)
	,m_ptText(-1,-1)
	,m_ptIcon(-1,-1)
{
}


SIconButton::~SIconButton(void)
{
}

void SIconButton::GetTextRect(LPRECT pRect)
{
	SButton::GetTextRect(pRect);
	if (m_pSkinIcon){
		if (m_ptText.x>0)
		{
			pRect->left += m_ptText.x;
		}else{
			pRect->left += m_ptIcon.x+m_pSkinIcon->GetSkinSize().cx;
		}
		if (m_ptText.y>0)
		{
			pRect->top += m_ptText.y;
			pRect->bottom += m_ptText.y;
		}
	}
}

void SIconButton::OnPaint(IRenderTarget *pRT)
{
	if (!m_pBgSkin) return;
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcIcon;
	if (m_pSkinIcon)
	{
		if (m_ptIcon.y < 0)
		{
			m_ptIcon.y = (rcClient.Height()-m_pSkinIcon->GetSkinSize().cy)/2;
		}
		if (m_ptIcon.x < 0)
		{
			m_ptIcon.x = m_ptIcon.y*2;
		}
		rcIcon = CRect(m_ptIcon+rcClient.TopLeft(),CSize(0,0));
		rcIcon.right=rcIcon.left+m_pSkinIcon->GetSkinSize().cx;
		rcIcon.bottom=rcIcon.top+m_pSkinIcon->GetSkinSize().cy;
	}

	if(m_byAlphaAni==0xFF)
	{//不在动画过程中
		m_pBgSkin->Draw(
			pRT, rcClient,
			IIF_STATE4(GetState(), 0, 1, 2, 3)
			);
		if (m_pSkinIcon)
		{
			m_pSkinIcon->Draw(
				pRT, rcIcon,
				IIF_STATE4(GetState(), 0, 1, 2, 3)
				);
		}
	}
	else
	{//在动画过程中
		BYTE byNewAlpha=(BYTE)(((UINT)m_byAlphaAni*m_pBgSkin->GetAlpha())>>8);
		if(GetState()&WndState_Hover)
		{
			//get hover
			m_pBgSkin->Draw(pRT, rcClient, 0, m_pBgSkin->GetAlpha());
			m_pBgSkin->Draw(pRT, rcClient, 1, byNewAlpha);
			if (m_pSkinIcon)
			{
				m_pSkinIcon->Draw(pRT, rcIcon, 0, m_pSkinIcon->GetAlpha());
				m_pSkinIcon->Draw(pRT, rcIcon, 1, byNewAlpha);
			}
		}
		else
		{
			//lose hover
			m_pBgSkin->Draw(pRT, rcClient,0, m_pBgSkin->GetAlpha());
			m_pBgSkin->Draw(pRT, rcClient, 1, m_pBgSkin->GetAlpha()-byNewAlpha);
			if (m_pSkinIcon)
			{
				m_pSkinIcon->Draw(pRT, rcIcon,0, m_pSkinIcon->GetAlpha());
				m_pSkinIcon->Draw(pRT, rcIcon, 1, m_pSkinIcon->GetAlpha()-byNewAlpha);
			}
		}
	}

	SWindow::OnPaint(pRT);
}
