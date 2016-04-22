#pragma once

#include <control/scmnctrl.h>

namespace SOUI
{
    class EventCapture : public TplEventArgs<EventCapture>
    {
        SOUI_CLASS_NAME(EventCapture,L"on_capture")
    public:
        EventCapture(SWindow *pWnd,CPoint pt):TplEventArgs<EventCapture>(pWnd),pt_(pt){}
        enum{EventID=EVT_EXTERNAL_BEGIN};

        CPoint pt_;
    };

    class EventCaptureFinish : public TplEventArgs<EventCaptureFinish>
    {
        SOUI_CLASS_NAME(EventCaptureFinish,L"on_capture_finish")
    public:
        EventCaptureFinish(SWindow *pWnd,CPoint pt):TplEventArgs<EventCaptureFinish>(pWnd),pt_(pt){}
        enum{EventID=EVT_EXTERNAL_BEGIN+1};

        CPoint pt_;
    };

    class SCaptureButton : public SWindow
    {
        SOUI_CLASS_NAME(SCaptureButton,L"captureButton")
    public:        
        SCaptureButton(void);
        ~SCaptureButton(void);

    protected:
        virtual CSize GetDesiredSize(LPCRECT pRcContainer);  
        
        BOOL IsChecked();      
    protected:
        void OnLButtonDown(UINT nFlags, CPoint point);
        void OnLButtonUp(UINT nFlags, CPoint point);
        void OnMouseMove(UINT nFlags, CPoint point);
        void OnMouseLeave();
        
        void OnPaint(IRenderTarget *pRT);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_PAINT_EX(OnPaint)
        SOUI_MSG_MAP_END()
    };
}
