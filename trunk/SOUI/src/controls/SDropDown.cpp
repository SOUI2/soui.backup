/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 *
 * @file       SDropDown.cpp
 * @brief      SDropDownWnd类源文件
 * @version    v1.0
 * @author     soui
 * @date       2014-05-25
 *
 * Describe  此文件主要用于SDropDownWnd类相关实现
 */
#include "souistd.h"
#include "control/SDropDown.h"
#include <core/SMsgLoop.h>

namespace SOUI
{

    SDropDownWnd::SDropDownWnd(ISDropDownOwner* pOwner)
        :m_pOwner(pOwner)
        ,m_bClick(FALSE)
        ,m_uExitCode(IDCANCEL)
    {
        SMessageLoop::GetCurMsgLoop()->AddMessageFilter(this);
    }

    SDropDownWnd::~SDropDownWnd()
    {
        SMessageLoop::GetCurMsgLoop()->RemoveMessageFilter(this);
    }

    void SDropDownWnd::OnFinalMessage(HWND hWnd)
    {
        __super::OnFinalMessage(hWnd);
        delete this;
    }

    void SDropDownWnd::OnKillFocus( HWND wndFocus )
    {
        if(wndFocus != m_hWnd)
        {
            EndDropDown();
        }
    }

    BOOL SDropDownWnd::Create(LPCRECT lpRect ,LPVOID lParam,DWORD dwStyle,DWORD dwExStyle)
    {
        HWND hParent = m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd();
        HWND hWnd=CSimpleWnd::Create(NULL,dwStyle,dwExStyle,lpRect->left,lpRect->top,lpRect->right-lpRect->left,lpRect->bottom-lpRect->top,hParent,0);
        if(!hWnd) return FALSE;
        m_pOwner->OnDropDown(this);
        return TRUE;
    }

    void SDropDownWnd::OnLButtonDown( UINT nFlags, CPoint point )
    {
        CRect rcWnd;
        GetClientRect(&rcWnd);
        if(!rcWnd.PtInRect(point))
        {
            EndDropDown();
        }else
        {
            m_bClick=TRUE;
            SetMsgHandled(FALSE);
        }
    }

    void SDropDownWnd::OnLButtonUp( UINT nFlags, CPoint point )
    {
        if(m_bClick)
        {
            LRESULT lRes=0;
            HWND hWnd=m_hWnd;
            CRect rcWnd;
            GetClientRect(&rcWnd);
            SHostWnd::ProcessWindowMessage(m_hWnd,WM_LBUTTONUP,nFlags,MAKELPARAM(point.x,point.y),lRes);
            if(::IsWindow(hWnd) && !rcWnd.PtInRect(point))
                EndDropDown();//强制关闭弹出窗口
        } 
    }

    void SDropDownWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
    {
        if(nChar==VK_RETURN)
            EndDropDown(IDOK);
        else if(nChar==VK_ESCAPE)
            EndDropDown();
        else 
            SetMsgHandled(FALSE);
    }

    void SDropDownWnd::EndDropDown(UINT uCode)
    {
        m_uExitCode=uCode;
        HWND hWnd=m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd();
        DestroyWindow();
        SetActiveWindow(hWnd);
    }

    void SDropDownWnd::OnDestroy()
    {
        m_pOwner->OnCloseUp(this,m_uExitCode);
        SetMsgHandled(FALSE);
    }

    BOOL SDropDownWnd::PreTranslateMessage( MSG* pMsg )
    {
        if(pMsg->message==WM_ACTIVATEAPP)
        {
            CSimpleWnd::SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
            return FALSE;
        }
        if(!(pMsg->message>=WM_KEYFIRST && pMsg->message<=WM_KEYLAST) && pMsg->message!=WM_MOUSEWHEEL) return FALSE;
        CSimpleWnd::SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
        return TRUE;
    }

    void SDropDownWnd::OnActivateApp( BOOL bActive, DWORD dwThreadID )
    {
        if(!bActive)
        {
            EndDropDown();
        }
    }

    int SDropDownWnd::OnMouseActivate( HWND wndTopLevel, UINT nHitTest, UINT message )
    {
        return MA_NOACTIVATEANDEAT;
    }

}//end of namespace SOUI