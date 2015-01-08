#pragma once

#include <core/Swnd.h>
#include <control/SCmnCtrl.h>

#include "SAnimator.h"

namespace SOUI
{
#define NAME_SWITCH L"switch"


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
                
        bool OnSwitchClick(EventArgs *pEvt);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"posEnd",OnAttrPosEnd)
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
