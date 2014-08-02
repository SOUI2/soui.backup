/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 * 
 * @file       DuiCaption.cpp
 * @brief      标签控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-05-28
 * 
 * Describe    此类完成标题栏控件
 */
#include "souistd.h"
#include "control/SCaption.h"

namespace SOUI
{

SCaption::SCaption(void)
:m_bIsMaxDown(FALSE)
{
}

SCaption::~SCaption(void)
{
}

void SCaption::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsMaxDown = FALSE;
}

void SCaption::OnMouseMove(UINT nFlags, CPoint point)
{
	HWND hHost=GetContainer()->GetHostHwnd();
    if (WS_MAXIMIZE == (GetWindowLong(hHost,GWL_STYLE) & WS_MAXIMIZE) && m_bIsMaxDown)
	{
		::SendMessage(hHost,WM_SYSCOMMAND, SC_RESTORE | HTCAPTION,0);
		
		POINT pt;
		::GetCursorPos(&pt);

		RECT rc;
		::GetWindowRect(hHost, &rc);
		::SetWindowPos(hHost, NULL, pt.x - (rc.right - rc.left)/2, pt.y - 5, rc.right - rc.left, rc.bottom - rc.top, 0);

		::SendMessage(hHost, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
		m_bIsMaxDown = FALSE;
		return;
	}
}
void SCaption::OnLButtonDown( UINT nFlags, CPoint point )
{
    HWND hHost=GetContainer()->GetHostHwnd();
    if (WS_MAXIMIZE == (GetWindowLong(hHost,GWL_STYLE) & WS_MAXIMIZE))
	{
		m_bIsMaxDown = TRUE;
		return;
	}
    ::SendMessage(hHost,WM_SYSCOMMAND, SC_MOVE | HTCAPTION,0);
}

void SCaption::OnLButtonDblClk( UINT nFlags, CPoint point )
{
    HWND hHost=GetContainer()->GetHostHwnd();

    if (GetWindowLong(hHost,GWL_STYLE) & WS_THICKFRAME)
    {
        if (WS_MAXIMIZE == (GetWindowLong(hHost,GWL_STYLE) & WS_MAXIMIZE))
            ::SendMessage(hHost,WM_SYSCOMMAND, SC_RESTORE | HTCAPTION,0);
        else
            ::SendMessage(hHost,WM_SYSCOMMAND, SC_MAXIMIZE | HTCAPTION,0);
    }
}


}//namespace SOUI

