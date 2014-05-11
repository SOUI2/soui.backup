#pragma once
#include "duistd.h"
#include "duirealwnd.h"

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


CDuiRealWnd::CDuiRealWnd()
    :m_bInit(FALSE)
    ,m_lpData(0)
    ,m_hRealWnd(0)
{
}

CDuiRealWnd::~CDuiRealWnd()
{
}

BOOL CDuiRealWnd::NeedRedrawWhenStateChange()
{
    return FALSE;
}

void CDuiRealWnd::ShowRealWindow()
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

LRESULT CDuiRealWnd::OnWindowPosChanged(LPRECT lpWndPos)
{
    CRect rcOldWnd = m_rcWindow;

    LRESULT lRet=__super::OnWindowPosChanged(lpWndPos);

    if (lRet==0 && rcOldWnd != m_rcWindow)
    {
        DUINMREALWNDCMN nms;
        nms.hdr.code = DUINM_REALWND_SIZE;
        nms.hdr.hDuiWnd=m_hDuiWnd;
        nms.hdr.idFrom = GetCmdID();
		nms.hdr.pszNameFrom=GetName();
        nms.pRealWnd=this;
        DuiNotify((LPDUINMHDR)&nms);
    }
	return lRet;
}

void CDuiRealWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __super::OnShowWindow(bShow, nStatus);
    ShowRealWindow();
}

void CDuiRealWnd::OnDestroy()
{
    if (IsWindow(m_hRealWnd))
    {
        DUINMREALWNDCMN nms;
        nms.hdr.code = DUINM_REALWND_DESTROY;
        nms.hdr.hDuiWnd = m_hDuiWnd;
        nms.hdr.idFrom = GetCmdID();
		nms.hdr.pszNameFrom=GetName();
        nms.pRealWnd=this;
        DuiNotify((LPDUINMHDR)&nms);
    }
}

BOOL CDuiRealWnd::Load(pugi::xml_node xmlNode)
{
    BOOL bRet=__super::Load(xmlNode);
    if(bRet)
    {
        m_realwndParam.m_xmlParams.append_copy(xmlNode.child("params"));
        if(m_bInit) InitRealWnd();
    }
    return bRet;
}

const HWND CDuiRealWnd::GetRealHwnd(BOOL bAutoCreate/*=TRUE*/)
{
    if(!bAutoCreate) return m_hRealWnd;

    if(!m_bInit && !IsWindow(m_hRealWnd))
    {
        InitRealWnd();
    }

    return m_hRealWnd;
}

BOOL CDuiRealWnd::InitRealWnd()
{
    m_realwndParam.m_dwStyle|= WS_CHILD;

    DUINMREALWNDCMN nms;
    nms.hdr.code = DUINM_REALWND_CREATE;
    nms.hdr.hDuiWnd = m_hDuiWnd;
    nms.hdr.idFrom = GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.pRealWnd=this;
    HWND hWnd =(HWND) DuiNotify((LPDUINMHDR)&nms);

    if(::IsWindow(hWnd))
    {
        m_hRealWnd=hWnd;
        if(!m_bInit)
        {
            //如果不是在加载的时候创建窗口，则需要自动调整窗口位置
            CRect rcClient;
            GetClient(&rcClient);
            SetWindowPos(m_hRealWnd,0,rcClient.left,rcClient.top,rcClient.Width(),rcClient.Height(),SWP_NOZORDER);
        }

        DUINMREALWNDCMN nms;
        nms.hdr.code = DUINM_REALWND_INIT;
        nms.hdr.hDuiWnd = m_hDuiWnd;
        nms.hdr.idFrom = GetCmdID();
        nms.hdr.pszNameFrom=GetName();
        nms.pRealWnd=this;

        BOOL bFocus=(BOOL)DuiNotify((LPDUINMHDR)&nms);
        if(bFocus)
        {
            SetFocus(m_hRealWnd);
        }
        return TRUE;
    }
    return FALSE;
}


}//namespace SOUI