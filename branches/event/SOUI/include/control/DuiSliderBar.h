//////////////////////////////////////////////////////////////////////////
//  CDuiSliderBar
//    Author: Huang Jianxiong
//    date: 2013/2/17
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "DuiCmnCtrl.h"

namespace SOUI
{

class SOUI_EXP SSliderBar : public SProgress
{
    SOUI_CLASS_NAME(SSliderBar, L"sliderbar")

public:
    SSliderBar();
    ~SSliderBar();

    enum
    {
        SC_RAIL,
        SC_SELECT,
        SC_THUMB,
    };

public:
    int             HitTest(CPoint pt);

protected:
    BOOL            m_bDrag;
    CPoint          m_ptDrag;
    int                m_nDragValue;
    int             m_uHtPrev;

    ISkinObj *  m_pSkinThumb;

protected:
    LRESULT         NotifyPos(UINT uCode, int nPos);

    virtual CSize    GetDesiredSize(LPRECT pRcContainer);

    CRect           GetPartRect(UINT uSBCode);

    void            OnPaint(IRenderTarget * pRT);
    void            OnLButtonUp(UINT nFlags, CPoint point);
    void            OnLButtonDown(UINT nFlags, CPoint point);
    void            OnMouseMove(UINT nFlags, CPoint point);
    void            OnMouseLeave();


    SOUI_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_PAINT_EX(OnPaint)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"thumbskin", m_pSkinThumb, FALSE)
    SOUI_ATTRS_END()
};

template<typename T>
void DUI_SWAP(T &a,T &b)
{
    T t=a;
    a=b;
    b=t;
}

}
