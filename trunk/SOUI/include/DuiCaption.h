#pragma once
#include "duiwnd.h"

namespace SOUI
{
class CDuiCaption :
    public CDuiWindow
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiCaption, "caption")
public:
    CDuiCaption(void);
    virtual ~CDuiCaption(void);

protected:
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    DUIWIN_BEGIN_MSG_MAP()
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
    DUIWIN_END_MSG_MAP()
};
}
