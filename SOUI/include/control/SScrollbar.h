// duiScrollBar.h : implementation file
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "core/SWnd.h"

namespace SOUI
{

/////////////////////////////////////////////////////////////////////////////
// CDuiScrollBar
class SOUI_EXP SScrollBar: public SWindow
{
// Construction
public:
    SOUI_CLASS_NAME(SScrollBar, L"scrollbar")
    SScrollBar();

    virtual ~SScrollBar();

// Implementation
public:
    BOOL IsVertical();

    UINT HitTest(CPoint pt);

    int SetPos(int nPos);

    int GetPos();

    // Generated message map functions
protected:
    CRect GetPartRect(UINT uSBCode);

    virtual void OnInitFinished(pugi::xml_node xmlNode);

    void OnPaint(IRenderTarget * pRT);

    void OnLButtonUp(UINT nFlags, CPoint point) ;

    void OnLButtonDown(UINT nFlags, CPoint point) ;

    void OnMouseMove(UINT nFlags, CPoint point) ;

    void OnTimer(char nIDEvent) ;

    void OnMouseLeave();

    LRESULT OnSetScrollInfo(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnGetScrollInfo(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT NotifySbCode(UINT uCode,int nPos);

protected:
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"skin", m_pSkin, FALSE)
        ATTR_UINT(L"arrowsize", m_uAllowSize, FALSE)
        ATTR_INT(L"min", m_si.nMin, FALSE)
        ATTR_INT(L"max", m_si.nMax, FALSE)
        ATTR_INT(L"value", m_si.nPos, FALSE)
        ATTR_INT(L"page", m_si.nPage, FALSE)
        ATTR_INT(L"vertical", m_bVertical, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_DUITIMER(OnTimer)
        MSG_WM_PAINT_EX(OnPaint)
        MESSAGE_HANDLER_EX(SBM_SETSCROLLINFO,OnSetScrollInfo)
        MESSAGE_HANDLER_EX(SBM_GETSCROLLINFO,OnGetScrollInfo)
    SOUI_MSG_MAP_END()

protected:
    ISkinObj * m_pSkin;
    UINT          m_uAllowSize;

    SCROLLINFO    m_si;
    BOOL        m_bDrag;
    CPoint        m_ptDrag;
    int            m_nDragPos;
    UINT        m_uClicked;
    BOOL        m_bNotify;
    UINT        m_uHtPrev;

    BOOL        m_bVertical;
};

}//namespace SOUI