#pragma once
#include "duistd.h"
#include "control/duirealwnd.h"

namespace SOUI
{


CDuiRealWndParam::CDuiRealWndParam()
    :m_dwStyle(WS_CHILD)
    ,m_dwExStyle(0)
{

}

CDuiRealWndParam::~CDuiRealWndParam()
{
}


SRealWnd::SRealWnd()
    :m_bInit(FALSE)
    ,m_lpData(0)
    ,m_hRealWnd(0)
{
}

SRealWnd::~SRealWnd()
{
}

BOOL SRealWnd::NeedRedrawWhenStateChange()
{
    return FALSE;
}

void SRealWnd::ShowRealWindow()
{
    if(IsVisible(TRUE) && !IsWindow(m_hRealWnd))
    {
        InitRealWnd();
    }
    if(IsWindow(m_hRealWnd))
    {
        ShowWindow(m_hRealWnd,IsVisible(TRUE) ? SW_SHOW : SW_HIDE);
    }
}

LRESULT SRealWnd::OnWindowPosChanged(LPRECT lpWndPos)
{
    CRect rcOldWnd = m_rcWindow;

    LRESULT lRet=__super::OnWindowPosChanged(lpWndPos);

    if (lRet==0 && rcOldWnd != m_rcWindow)
    {
        EventCmnArgs evt(this,EVT_REALWND_SIZE);
        FireEvent(evt);
    }
    return lRet;
}

void SRealWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __super::OnShowWindow(bShow, nStatus);
    ShowRealWindow();
}

void SRealWnd::OnDestroy()
{
    if (IsWindow(m_hRealWnd))
    {
        EventCmnArgs evt(this,EVT_REALWND_DESTROY);
        FireEvent(evt);
    }
}

BOOL SRealWnd::InitFromXml(pugi::xml_node xmlNode)
{
    BOOL bRet=__super::InitFromXml(xmlNode);
    if(bRet)
    {
        m_realwndParam.m_xmlParams.append_copy(xmlNode.child(L"params"));
        if(m_bInit) InitRealWnd();
    }
    return bRet;
}

const HWND SRealWnd::GetRealHwnd(BOOL bAutoCreate/*=TRUE*/)
{
    if(!bAutoCreate) return m_hRealWnd;

    if(!m_bInit && !IsWindow(m_hRealWnd))
    {
        InitRealWnd();
    }

    return m_hRealWnd;
}

BOOL SRealWnd::InitRealWnd()
{
    m_realwndParam.m_dwStyle|= WS_CHILD;

    EventRealWndCreate evt(this);
    FireEvent(evt);

    if(::IsWindow(evt.hWndCreated))
    {
        m_hRealWnd=evt.hWndCreated;
        if(!m_bInit)
        {
            //如果不是在加载的时候创建窗口，则需要自动调整窗口位置
            CRect rcClient;
            GetClient(&rcClient);
            SetWindowPos(m_hRealWnd,0,rcClient.left,rcClient.top,rcClient.Width(),rcClient.Height(),SWP_NOZORDER);
        }
        
        EventRealWndInit evt(this);
        FireEvent(evt);        
        if(evt.bSetFocus)
        {
            ::SetFocus(m_hRealWnd);
        }
        return TRUE;
    }
    return FALSE;
}


}//namespace SOUI