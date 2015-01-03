#pragma once

#include <core/Swnd.h>
#include <control/SCmnCtrl.h>

namespace SOUI
{
#define NAME_SWITCH L"switch"

    class SAnimator
    {
    public:
        enum {
            PER_START = 0,
            PER_END = 100,
        };

        SAnimator():m_uDuration(0){}

        virtual ~SAnimator(){}

        void Start(UINT uDuration)
        {
            m_dwStart = GetTickCount();
            m_uDuration = uDuration;
            OnAnimatorState(0);
        }
        
        BOOL IsBusy(){return m_uDuration != 0;}

        BOOL Update()
        {
            if(m_uDuration == 0) return FALSE;

            DWORD dwElapse = GetTickCount() - m_dwStart;
            if(dwElapse >= m_uDuration)
            {
                OnAnimatorState(100);
                m_uDuration = 0;
            }else
            {
                OnAnimatorState(dwElapse*100/m_uDuration);
            }
            return TRUE;
        }

        //动画状态改变，percent in [0,100]
        virtual void OnAnimatorState(int percent)
        {

        }
    private:
        DWORD m_dwStart;
        DWORD m_uDuration;
    };

    class SFlyWnd : public SWindow ,public SAnimator , public ITimelineHandler
    {
        SOUI_CLASS_NAME(SFlyWnd,L"flywnd")
    public:
        SFlyWnd(void);
        ~SFlyWnd(void);
        
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
