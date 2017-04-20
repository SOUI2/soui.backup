
#pragma once
#include "core/SWnd.h"

namespace SOUI
{

class SToggleEx : public SToggle
{
    SOUI_CLASS_NAME(SToggleEx, L"toggle_ex")
public: 
    void OnLButtonUp(UINT nFlags,CPoint pt);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONUP(OnLButtonUp)
    SOUI_MSG_MAP_END()
};

};