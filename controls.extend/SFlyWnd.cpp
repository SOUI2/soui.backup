#include "stdafx.h"
#include "SFlyWnd.h"
#include <core/SwndLayoutBuilder.h>

namespace SOUI
{
    static const wchar_t NAME_SWITCH[] =   L"switch";

    SFlyWnd::SFlyWnd(void):m_bEndPos(FALSE),m_nAniTime(200)
    {
       m_evtSet.addEvent(EVT_FLYSTATE);
    }

    SFlyWnd::~SFlyWnd(void)
    {
    }

    const SwndLayout * SFlyWnd::GetLayout() const
    {
        if(m_bEndPos) return &m_endLayout;
        else return SWindow::GetLayout();
    }

    HRESULT SFlyWnd::OnAttrPosEnd( const SStringW& strValue, BOOL bLoading )
    {
        if(!m_endLayout.InitPosFromString(strValue)) return E_FAIL;
        if(bLoading && m_bEndPos && GetParent())
        {
            GetParent()->UpdateChildrenPosition();
        }
        return S_FALSE;
    }

    HRESULT SFlyWnd::OnAttrOffsetEnd( const SStringW& strValue, BOOL bLoading )
    {
        if(!m_endLayout.InitOffsetFromString(strValue)) return E_FAIL;
        if(bLoading && m_bEndPos && GetParent())
        {
            GetParent()->UpdateChildrenPosition();
        }
        return S_FALSE;
    }

    bool SFlyWnd::OnSwitchClick( EventArgs *pEvt )
    {
        //让窗口可以更新hover状态
        SWindow *pBtn = (SWindow*)(pEvt->sender);
        pBtn->SSendMessage(WM_MOUSELEAVE);

        SwitchState(!m_bEndPos);
        
        return true;
    }

    BOOL SFlyWnd::CreateChildren( pugi::xml_node xmlNode )
    {
        BOOL bRet = SWindow::CreateChildren(xmlNode);
        if(!bRet) return FALSE;
        SToggle *pSwitch = FindChildByName2<SToggle>(NAME_SWITCH);
        if(pSwitch)
        {
            pSwitch->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SFlyWnd::OnSwitchClick,this));
        }
        return TRUE;
    }

    void SFlyWnd::OnAnimatorState( int percent )
    {
        if(percent == SAnimator::PER_END)
        {
            m_bAniMove = TRUE;
            Move(m_rcAniEnd);
            m_bAniMove = FALSE;
            m_bFloat = FALSE;

            GetContainer()->UnregisterTimelineHandler(this);
        }else
        {
            CRect rcWnd;
            rcWnd.left   = m_rcAniBegin.left   + (m_rcAniEnd.left  - m_rcAniBegin.left  ) * percent / 100; 
            rcWnd.top    = m_rcAniBegin.top    + (m_rcAniEnd.top   - m_rcAniBegin.top   ) * percent / 100;
            rcWnd.right  = m_rcAniBegin.right  + (m_rcAniEnd.right - m_rcAniBegin.right ) * percent / 100;
            rcWnd.bottom = m_rcAniBegin.bottom + (m_rcAniEnd.bottom- m_rcAniBegin.bottom) * percent / 100;

            m_bAniMove = TRUE;
            Move(rcWnd);
            m_bFloat = FALSE;
            m_bAniMove = FALSE;
        }
        FlyStateEvent evt(this,percent,m_bEndPos);
        FireEvent(evt);
    }

    void SFlyWnd::OnRelayout( const CRect &rcOld, const CRect & rcNew )
    {
        if(!m_bAniMove)
        {//非动画过程产生的relayout消息
            if(!SAnimator::IsBusy())
            {//不在动画过程中
                SWindow::OnRelayout(rcOld,rcNew);
            }else
            {//在动画过程中，只修改目标位置
                m_rcAniEnd = rcNew;
            }
        }else
        {
            SWindow::OnRelayout(rcOld,rcNew);
        }
    }

    void SFlyWnd::OnNextFrame()
    {
        SAnimator::Update();
    }

    BOOL SFlyWnd::SwitchState(BOOL bEndPos)
    {
        if(m_bEndPos == bEndPos) return FALSE;
        
        m_rcAniBegin = m_rcWindow;
        m_bEndPos = bEndPos;
        SwndLayoutBuilder::InitLayoutState(m_rcAniEnd);

        SASSERT(GetParent());
        CRect rcContainer = GetParent()->GetChildrenLayoutRect();
        int nRet =SwndLayoutBuilder::CalcPosition(this,rcContainer,m_rcAniEnd);
        SASSERT(nRet == 0);

        SAnimator::Start(m_nAniTime);
        GetContainer()->RegisterTimelineHandler(this);
        
        SToggle *pSwitch = FindChildByName2<SToggle>("switch");
        if(pSwitch) pSwitch->SetToggle(m_bEndPos);
        
        return TRUE;
    }

    BOOL SFlyWnd::IsAtEndPos() const
    {
        return m_bEndPos;
    }

}
