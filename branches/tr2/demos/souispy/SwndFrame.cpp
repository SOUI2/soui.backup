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
        RECT rcWnd = rc;
        ::InflateRect(&rcWnd,2,2);
        ShowWindow(SW_HIDE);
        MoveWindow(&rcWnd);
        ::OffsetRect(&rcWnd,-rcWnd.left,-rcWnd.top);

        HDC hdc = GetDC();
        LOGFONT lf={0};
        lf.lfWeight = FW_BOLD;
        _tcscpy(lf.lfFaceName,_T("ËÎÌו"));
        lf.lfHeight=-20;
        HFONT hFont = CreateFontIndirect(&lf);
        HGDIOBJ hOldFont=SelectObject(hdc,hFont);
        BeginPath(hdc);
        SetBkMode(hdc,TRANSPARENT);
        SStringT strText = SStringT().Format(_T("(%d,%d)+[%d,%d]"),rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top);
        TextOut(hdc,2,2,strText,strText.GetLength());
        EndPath(hdc);
        SelectObject(hdc,hOldFont);
        DeleteObject(hFont);
        HRGN hRgnText = PathToRegion(hdc);
        ReleaseDC(hdc);

        HRGN hRgn = ::CreateRectRgnIndirect(&rcWnd);
        ::InflateRect(&rcWnd,-2,-2);
        HRGN hRgn2 = ::CreateRectRgnIndirect(&rcWnd);
        ::CombineRgn(hRgn,hRgn,hRgn2,RGN_DIFF);
        ::CombineRgn(hRgn,hRgn,hRgnText,RGN_OR);
        SetWindowRgn(hRgn,FALSE);
        ::DeleteObject(hRgnText);
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
