#pragma once

#include <control/SCmnCtrl.h>

namespace SOUI
{
    class SRadioBox2 : public SRadioBox
    {
    SOUI_CLASS_NAME(SRadioBox2,L"radio2")
    public:
        SRadioBox2(void);
        ~SRadioBox2(void);
        
    protected:
        virtual CSize GetDesiredSize(LPCRECT pRcContainer)
        {
            return SWindow::GetDesiredSize(pRcContainer);
        }
        virtual void GetTextRect(LPRECT pRect)
        {
            SWindow::GetTextRect(pRect);
        }
        virtual void DrawFocus(IRenderTarget *pRT)
        {
        }

    protected:       
        void OnPaint(IRenderTarget *pRT);

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
        SOUI_MSG_MAP_END()
    };

}
