#include "StdAfx.h"
#include "SCaptureButton.h"

namespace SOUI
{
    SCaptureButton::SCaptureButton(void)
    {
        m_evtSet.addEvent(EventCapture::EventID);
    }

    SCaptureButton::~SCaptureButton(void)
    {
    }

    void SCaptureButton::OnLButtonDown( UINT nFlags, CPoint point )
    {
        SWindow::OnLButtonDown(nFlags,point);
        HCURSOR hCursor=GETRESPROVIDER->LoadCursor(MAKEINTRESOURCE(IDC_HELP));
        ::SetCursor(hCursor);
    }

    void SCaptureButton::OnLButtonUp( UINT nFlags, CPoint point )
    {
        SWindow::OnLButtonUp(nFlags,point);
    }

    void SCaptureButton::OnMouseMove( UINT nFlags, CPoint point )
    {
        if(IsChecked())
        {
            EventCapture evt(this,point);
            FireEvent(evt);
        }
    }

    void SCaptureButton::OnPaint( IRenderTarget *pRT )
    {
        if(!m_pBgSkin) return;
        m_pBgSkin->Draw(pRT,m_rcWindow,IsChecked()?1:0);
    }

    CSize SCaptureButton::GetDesiredSize( LPRECT pRcContainer )
    {
        if(!m_pBgSkin) return CSize();
        return m_pBgSkin->GetSkinSize();
    }

    BOOL SCaptureButton::IsChecked()
    {
        return m_dwState & WndState_PushDown;
    }

    void SCaptureButton::OnMouseLeave()
    {
        
    }

}
