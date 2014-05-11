#include "duistd.h"
#include "DuiCaption.h"

namespace SOUI
{

CDuiCaption::CDuiCaption(void)
{
}

CDuiCaption::~CDuiCaption(void)
{
}

void CDuiCaption::OnLButtonDown( UINT nFlags, CPoint point )
{
    HWND hHost=GetContainer()->GetHostHwnd();
    if (WS_MAXIMIZE == (GetWindowLong(hHost,GWL_STYLE) & WS_MAXIMIZE)) return;
    ::SendMessage(hHost,WM_SYSCOMMAND, SC_MOVE | HTCAPTION,0);
}

void CDuiCaption::OnLButtonDblClk( UINT nFlags, CPoint point )
{
    HWND hHost=GetContainer()->GetHostHwnd();

    if (GetWindowLong(hHost,GWL_STYLE) & WS_THICKFRAME)
    {
        if (WS_MAXIMIZE == (GetWindowLong(hHost,GWL_STYLE) & WS_MAXIMIZE))
            SendMessage(hHost,WM_SYSCOMMAND, SC_RESTORE | HTCAPTION,0);
        else
            SendMessage(hHost,WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION,0);
    }
}


}//namespace SOUI

