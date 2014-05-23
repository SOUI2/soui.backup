#pragma once
#include "SimpleWnd.h"

namespace SOUI
{

class CDuiTipCtrl : public CSimpleWnd
{
public:
    CDuiTipCtrl(void);
    virtual ~CDuiTipCtrl(void);

    BOOL Create(HWND hOwner);

    void RelayEvent(const MSG *pMsg);
    void UpdateTip(CRect rc,LPCTSTR pszTip,BOOL bText=TRUE);
    void SetDelayTime(DWORD dwType,UINT iTime);

    void ShowTip(BOOL bShow);

    DWORD    m_dwHostID;
protected:
    void OnTimer(UINT_PTR idEvent);
    void OnPaint(CDCHandle dc);

    BEGIN_MSG_MAP_EX(CDuiTipCtrl)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_TIMER(OnTimer)
    REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

protected:
    int            m_nDelay;
    int            m_nShowSpan;
    CDuiStringT    m_strTip;
    BOOL        m_bTextTip;    
    CRect        m_rcTarget;
    CFont        m_font;
};

}//namespace SOUI