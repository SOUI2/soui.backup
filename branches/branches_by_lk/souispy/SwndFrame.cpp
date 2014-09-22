#include "StdAfx.h"
#include "SwndFrame.h"

namespace SOUI
{
    SwndFrame::SwndFrame(void)
    {
    }

    SwndFrame::~SwndFrame(void)
    {
    }

    void SwndFrame::OnPaint( HDC hdc )
    {
        PAINTSTRUCT ps;
        hdc=::BeginPaint(m_hWnd, &ps);

        RECT rc;
        GetClientRect(&rc);
        HBRUSH br = ::CreateSolidBrush(RGB(255,0,0));
        ::FillRect(hdc,&rc,br);
        ::DeleteObject(br);

        ::EndPaint(m_hWnd,&ps);
    }

    BOOL SwndFrame::Create(HWND hOwner)
    {
        HWND hWnd=CSimpleWnd::Create(_T("SwndFrame"),WS_POPUP,WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_NOACTIVATE|WS_EX_TRANSPARENT,0,0,0,0,0,0);
        return hWnd!=0;
    }

    void SwndFrame::Show( RECT rc )
    {
        RECT rcWndPrev;
        GetWindowRect(&rcWndPrev);
        RECT rcWnd = rc;
        ::InflateRect(&rcWnd,2,2);
        if(memcmp(&rcWndPrev,&rcWnd,sizeof(RECT))==0)
            return;
        ShowWindow(SW_HIDE);
        MoveWindow(&rcWnd,FALSE);
        ::OffsetRect(&rcWnd,-rcWnd.left,-rcWnd.top);
        HRGN hRgn = ::CreateRectRgnIndirect(&rcWnd);
        ::InflateRect(&rcWnd,-2,-2);
        HRGN hRgn2 = ::CreateRectRgnIndirect(&rcWnd);
        ::CombineRgn(hRgn,hRgn,hRgn2,RGN_DIFF);
        SetWindowRgn(hRgn,FALSE);
        ::DeleteObject(hRgn);
        ::DeleteObject(hRgn2);
        SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOSENDCHANGING|SWP_SHOWWINDOW|SWP_NOACTIVATE);
        SetTimer(100,5000,NULL);
    }

    void SwndFrame::OnTimer( UINT_PTR nIDEvent )
    {
        ShowWindow(SW_HIDE);
        KillTimer(nIDEvent);
        MoveWindow(0,0,0,0);
    }
}
