#pragma once

#include <core/Swnd.h>
#include <control/SCmnCtrl.h>

#include "SAnimator.h"

namespace SOUI
{

#define EVT_FLYSTATE    (EVT_EXTERNAL_BEGIN+200)

    class FlyStateEvent : public TplEventArgs<FlyStateEvent>
    {
        SOUI_CLASS_NAME(FlyStateEvent,L"on_flywnd_state")
    public:
        FlyStateEvent(SWindow *pSender,int _percent,BOOL _bEndPos)
            :TplEventArgs<FlyStateEvent>(pSender)
            ,nPercent(_percent)
            ,bEndPos(_bEndPos)
        {

        }
        
        enum{EventID = EVT_FLYSTATE};

        int nPercent;
        BOOL bEndPos;
    };

    class SFlyWnd : public SWindow ,public SAnimator , public ITimelineHandler
    {
        SOUI_CLASS_NAME(SFlyWnd,L"flywnd")
    public:
        SFlyWnd(void);
        ~SFlyWnd(void);
        
        BOOL SwitchState(BOOL bEndPos);
        BOOL IsAtEndPos() const;
    protected:
        virtual const SwndLayout * GetLayout() const;
        virtual BOOL CreateChildren(pugi::xml_node xmlNode);
        virtual void OnRelayout(const CRect &rcOld, const CRect & rcNew);

        virtual void OnAnimatorState(int percent);
        virtual void OnNextFrame();

        HRESULT OnAttrPosEnd(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrOffsetEnd(const SStringW& strValue, BOOL bLoading);
                
        bool OnSwitchClick(EventArgs *pEvt);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"posEnd",OnAttrPosEnd)
            ATTR_CUSTOM(L"offsetEnd",OnAttrOffsetEnd)
            ATTR_UINT(L"AniTime",m_nAniTime,FALSE)
        SOUI_ATTRS_END()
    protected:
        SwndLayout    m_endLayout;
        DWORD         m_nAniTime;
        BOOL          m_bEndPos;

        //动画开始和结束位置
        CRect         m_rcAniBegin,m_rcAniEnd;
        BOOL          m_bAniMove;
    };

}
