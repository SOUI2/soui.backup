#pragma once

#include "duiwnd.h"

//////////////////////////////////////////////////////////////////////////
// Real Window Control
// Binding a real window
//
// Usage: <realwnd id=xx wndclass="edit" wndname="name" style="00000001" exstyle="00000000"/>
//

namespace SOUI
{

class SOUI_EXP CDuiRealWndParam
{
public:
    CDuiRealWndParam();
    ~CDuiRealWndParam();

    CDuiStringT m_strClassName,m_strWindowName;
    DWORD     m_dwStyle,m_dwExStyle;
    pugi::xml_document m_xmlParams;
};


class SOUI_EXP CDuiRealWnd : public CDuiWindow
{
    SOUI_CLASS_NAME(CDuiRealWnd, "realwnd")
public:
    CDuiRealWnd();

    virtual ~CDuiRealWnd();


    const HWND GetRealHwnd(BOOL bAutoCreate=TRUE);

    const CDuiRealWndParam & GetRealWndParam()
    {
        return m_realwndParam;
    }

    void SetData(LPVOID lpData)
    {
        m_lpData=lpData;
    }
    const LPVOID GetData()
    {
        return m_lpData;
    }

    SOUO_ATTRIBUTES_BEGIN()
    DUIWIN_TSTRING_ATTRIBUTE("wndclass", m_realwndParam.m_strClassName, FALSE)
    DUIWIN_TSTRING_ATTRIBUTE("wndname", m_realwndParam.m_strWindowName, FALSE)
    DUIWIN_HEX_ATTRIBUTE("style", m_realwndParam.m_dwStyle, FALSE)
    DUIWIN_HEX_ATTRIBUTE("exstyle", m_realwndParam.m_dwExStyle, FALSE)
    DUIWIN_INT_ATTRIBUTE("init",m_bInit,FALSE)
    SOUI_ATTRIBUTES_END()
protected:
    virtual BOOL NeedRedrawWhenStateChange();
    virtual BOOL Load(pugi::xml_node xmlNode);


    LRESULT OnWindowPosChanged(LPRECT lpWndPos);

    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnDestroy();

    // Do nothing
    void OnPaint(CDCHandle dc) {}

    void ShowRealWindow();

    BOOL InitRealWnd();


    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_DUIWINPOSCHANGED(OnWindowPosChanged)
    MSG_WM_SHOWWINDOW(OnShowWindow)
    WND_MSG_MAP_END()

    CDuiRealWndParam    m_realwndParam;
    BOOL    m_bInit;

    HWND     m_hRealWnd;
    LPVOID    m_lpData;
};

interface SOUI_EXP IDuiRealWndHandler
{
    virtual HWND OnRealWndCreate(CDuiRealWnd *pRealWnd)=NULL;
    virtual BOOL OnRealWndInit(CDuiRealWnd *pRealWnd)=NULL;
    virtual void OnRealWndDestroy(CDuiRealWnd *pRealWnd)=NULL;
    virtual void OnRealWndSize(CDuiRealWnd *pRealWnd)=NULL;
};

}//namespace SOUI