#pragma once
#include "duiwnd.h"

namespace SOUI
{
class CDuiCaption :
    public CDuiWindow
{
    SOUI_CLASS_NAME(CDuiCaption, "caption")
public:
    CDuiCaption(void);
    virtual ~CDuiCaption(void);

protected:
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    WND_MSG_MAP_BEGIN()
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
    WND_MSG_MAP_END()
};
}
