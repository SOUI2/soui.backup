// duiScrollBar.h : implementation file
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "duiwnd.h"

namespace SOUI
{

/////////////////////////////////////////////////////////////////////////////
// CDuiScrollBar
class SOUI_EXP CDuiScrollBar: public CDuiWindow
{
// Construction
public:
    DUIOBJ_DECLARE_CLASS_NAME(CDuiScrollBar, "scrollbar")
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
    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_SKIN_ATTRIBUTE("skin", m_pSkin, FALSE)
    DUIWIN_UINT_ATTRIBUTE("arrowsize", m_uAllowSize, FALSE)
    DUIWIN_INT_ATTRIBUTE("min", m_si.nMin, FALSE)
    DUIWIN_INT_ATTRIBUTE("max", m_si.nMax, FALSE)
    DUIWIN_INT_ATTRIBUTE("value", m_si.nPos, FALSE)
    DUIWIN_INT_ATTRIBUTE("page", m_si.nPage, FALSE)
    DUIWIN_INT_ATTRIBUTE("vertical", m_bVertical, FALSE)
    DUIWIN_DECLARE_ATTRIBUTES_END()

    DUIWIN_BEGIN_MSG_MAP()
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_MOUSELEAVE(OnMouseLeave)
    MSG_WM_DUITIMER(OnTimer)
    MSG_WM_PAINT(OnPaint)
    MESSAGE_HANDLER_EX(SBM_SETSCROLLINFO,OnSetScrollInfo)
    MESSAGE_HANDLER_EX(SBM_GETSCROLLINFO,OnGetScrollInfo)
    DUIWIN_END_MSG_MAP()

protected:
    CDuiSkinBase * m_pSkin;
    UINT		  m_uAllowSize;

    SCROLLINFO	m_si;
    BOOL		m_bDrag;
    CPoint		m_ptDrag;
    int			m_nDragPos;
    UINT		m_uClicked;
    BOOL		m_bNotify;
    UINT		m_uHtPrev;

    BOOL		m_bVertical;
};

}//namespace SOUI