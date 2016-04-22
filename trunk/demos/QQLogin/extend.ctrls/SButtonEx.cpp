#include "stdafx.h"
#include "SButtonEx.h"

SButtonEx::SButtonEx()
{
    m_bFocusable = FALSE;
    m_pbkgndSkin = NULL;
    m_sizeTextShift.cx = m_sizeTextShift.cy = 0;
}

SButtonEx::~SButtonEx(void)
{
}

void SButtonEx::OnPaint(IRenderTarget *pRT)
{
    DrawSkin(m_pbkgndSkin, pRT);
    DrawSkin(m_pBgSkin, pRT);
    SWindow::OnPaint(pRT);
}

void SButtonEx::DrawSkin(ISkinObj * pSkin, IRenderTarget *pRT)
{
    if (!pSkin) return;

    CRect rcClient;
    GetClientRect(&rcClient);

    if(m_byAlphaAni==0xFF)
    {//不在动画过程中
        pSkin->Draw(
            pRT, rcClient,
            IIF_STATE4(GetState(), 0, 1, 2, 3)
            );
    }
    else
    {//在动画过程中
        BYTE byNewAlpha=(BYTE)(((UINT)m_byAlphaAni*pSkin->GetAlpha())>>8);
        if(GetState()&WndState_Hover)
        {
            //get hover
            pSkin->Draw(pRT, rcClient, 0, pSkin->GetAlpha());
            pSkin->Draw(pRT, rcClient, 1, byNewAlpha);
        }
        else
        {
            //lose hover
            pSkin->Draw(pRT, rcClient,0, pSkin->GetAlpha());
            pSkin->Draw(pRT, rcClient, 1, pSkin->GetAlpha()-byNewAlpha);
        }
    }
}

void SButtonEx::DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    if ((m_sizeTextShift.cx != 0 || m_sizeTextShift.cy != 0) && 
        (GetState()&WndState_PushDown))
    {
        RECT rcNew = *pRect;
        rcNew.left += m_sizeTextShift.cx;
        rcNew.top  += m_sizeTextShift.cy;
        pRT->DrawText(pszBuf, cchText, &rcNew, uFormat);
    }
    else
    {
        SWindow::DrawText(pRT, pszBuf, cchText, pRect, uFormat);
    }
}
