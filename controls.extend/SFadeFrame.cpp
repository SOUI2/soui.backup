#include "stdafx.h"
#include "SFadeFrame.h"

#define TIMER_SHOW  1
#define TIMER_HIDE  2

namespace SOUI{
    SFadeFrame::SFadeFrame(void):m_nAniTime(200),m_nDelayTime(200)
    {
        m_style.m_bTrackMouseEvent = true;
        m_byAlphaCurrent = 0xFF;
    }

    SFadeFrame::~SFadeFrame(void)
    {
    }

    void SFadeFrame::OnMouseLeave()
    {
        SWindow::OnMouseLeave();
        KillTimer(TIMER_SHOW);
        SetTimer(TIMER_HIDE,m_nDelayTime);
    }

    void SFadeFrame::OnMouseHover(WPARAM wParam, CPoint ptPos)
    {
        SWindow::OnMouseHover(wParam,ptPos);
        KillTimer(TIMER_HIDE);
        SetTimer(TIMER_SHOW,m_nDelayTime);
    }

    void SFadeFrame::OnTimer(char cTimerID)
    {
        KillTimer(cTimerID);
        if(cTimerID == TIMER_SHOW)
        {
            if(m_byAlphaCurrent == m_style.m_byAlpha) return;
            m_nAlphaBegin = m_byAlphaCurrent;
            m_nAlphaEnd = m_style.m_byAlpha;
        }else if(cTimerID == TIMER_HIDE)
        {
            if(m_byAlphaCurrent == 0) return;
            m_nAlphaBegin = m_byAlphaCurrent;
            m_nAlphaEnd = 0;
        }
        SAnimator::Start(m_nAniTime);
        GetContainer()->RegisterTimelineHandler(this);
    }

    void SFadeFrame::OnAnimatorState(int percent)
    {
        if(percent == SAnimator::PER_END)
        {
            SetChildrenAlpha((BYTE)m_nAlphaEnd);
            GetContainer()->UnregisterTimelineHandler(this);
        }else
        {
            BYTE byAlpha = (BYTE)(m_nAlphaBegin + (m_nAlphaEnd - m_nAlphaBegin)* percent/100);
            SetChildrenAlpha(byAlpha);
        }
    }

    void SFadeFrame::OnNextFrame()
    {
        SAnimator::Update();
    }

    void SFadeFrame::SetChildrenAlpha(BYTE byAlpha)
    {
        SStringW strAlpha;
        strAlpha.Format(L"%d",byAlpha);
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(pChild->IsVisible(FALSE) ^ (byAlpha>0))
                pChild->SetVisible(byAlpha>0,TRUE);
            pChild->SetAttribute(L"alpha",strAlpha);
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
        m_byAlphaCurrent = byAlpha;
    }

}