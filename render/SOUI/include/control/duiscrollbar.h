// duiScrollBar.h : implementation file
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "duiwnd.h"

namespace SOUI
{

/////////////////////////////////////////////////////////////////////////////
// CDuiScrollBar
class SOUI_EXP CDuiScrollBar: public SWindow
{
// Construction
public:
    SOUI_CLASS_NAME(CDuiScrollBar, "scrollbar")
    CDuiScrollBar();

    virtual ~CDuiScrollBar();

// Implementation
public:
    BOOL IsVertical();

    UINT HitTest(CPoint pt);

    int SetPos(int nPos);

    int GetPos();

    // Generated message map functions
protected:
    CRect GetPartRect(UINT uSBCode);

    virtual void OnAttributeFinish(pugi::xml_node xmlNode);

    void OnPaint(CDCHandle dc);

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
    ATTR_SKIN("skin", m_pSkin, FALSE)
    ATTR_UINT("arrowsize", m_uAllowSize, FALSE)
    ATTR_INT("min", m_si.nMin, FALSE)
    ATTR_INT("max", m_si.nMax, FALSE)
    ATTR_INT("value", m_si.nPos, FALSE)
    ATTR_INT("page", m_si.nPage, FALSE)
    ATTR_INT("vertical", m_bVertical, FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_MOUSELEAVE(OnMouseLeave)
    MSG_WM_DUITIMER(OnTimer)
    MSG_WM_PAINT(OnPaint)
    MESSAGE_HANDLER_EX(SBM_SETSCROLLINFO,OnSetScrollInfo)
    MESSAGE_HANDLER_EX(SBM_GETSCROLLINFO,OnGetScrollInfo)
    WND_MSG_MAP_END()

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