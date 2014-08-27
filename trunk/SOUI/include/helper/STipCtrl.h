#pragma once
#include "core/SimpleWnd.h"
#include "core/smsgloop.h"

namespace SOUI
{
    struct TIPID
    {
        DWORD dwHi;
        DWORD dwLow;
        
        bool operator == (const TIPID & src)
        {
            return dwHi==src.dwHi && dwLow == src.dwLow;
        }
        
        bool operator != (const TIPID & src)
        {
            return dwHi!=src.dwHi || dwLow != src.dwLow;
        }
    };
    
    class STipCtrl : public CSimpleWnd 
        , public IMessageFilter
    {
    public:
        STipCtrl(void);
        virtual ~STipCtrl(void);

        BOOL Create(HWND hOwner);

        void RelayEvent(const MSG *pMsg);
        void SetDelayTime(DWORD dwType,UINT iTime);

        void UpdateTip(const TIPID &id, CRect rc,LPCTSTR pszTip);
        
        void ClearTip();

        void ShowTip(BOOL bShow);
        
        const TIPID & GetTipID(){return m_id;}
    protected:
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        void OnTimer(UINT_PTR idEvent);
        void OnPaint(HDC dc);

        BEGIN_MSG_MAP_EX(STipCtrl)
            MSG_WM_PAINT(OnPaint)
            MSG_WM_TIMER(OnTimer)
            REFLECT_NOTIFICATIONS_EX()
        END_MSG_MAP()

    protected:
        int            m_nDelay;
        int            m_nShowSpan;
        SStringT    m_strTip;
        CRect        m_rcTarget;
        HFONT        m_font;
        
        TIPID       m_id;
    };

}//namespace SOUI