#pragma once

#define SOUI_MSG_MAP_BEGIN()                                       \
protected:                                                          \
    virtual BOOL ProcessDuiWndMessage(                              \
    UINT uMsg, WPARAM wParam,                        \
    LPARAM lParam, LRESULT& lResult)                            \
    {

#define SOUI_MSG_MAP_END()                                        \
    if (!IsMsgHandled())                                        \
    return __super::ProcessDuiWndMessage(                   \
    uMsg, wParam, lParam, lResult);                     \
    return TRUE;                                                \
    }

#define WND_MSG_MAP_END_BASE()                                    \
    return SwndProc(uMsg,wParam,lParam,lResult);               \
    }


#define MSG_WM_WINPOSCHANGED_EX(func) \
    if (uMsg == WM_WINDOWPOSCHANGED) \
{ \
    SetMsgHandled(TRUE); \
    lResult=func((LPRECT)lParam); \
    if(IsMsgHandled()) \
    return TRUE; \
}

// BOOL OnEraseBkgnd(IRenderTarget * pRT)
#define MSG_WM_ERASEBKGND_EX(func) \
    if (uMsg == WM_ERASEBKGND) \
    { \
    SetMsgHandled(TRUE); \
    lResult = (LRESULT)func((IRenderTarget *)wParam); \
    if(IsMsgHandled()) \
    return TRUE; \
    }

// void OnPaint(IRenderTarget * pRT)
#define MSG_WM_PAINT_EX(func) \
    if (uMsg == WM_PAINT) \
    { \
    SetMsgHandled(TRUE); \
    func((IRenderTarget *)wParam); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
    }

// void OnNcPaint(IRenderTarget * pRT)
#define MSG_WM_NCPAINT_EX(func) \
    if (uMsg == WM_NCPAINT) \
{ \
    SetMsgHandled(TRUE); \
    func((IRenderTarget *)wParam); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}

// void OnSetFont(IFont *pFont, BOOL bRedraw)
#define MSG_WM_SETFONT_EX(func) \
    if (uMsg == WM_SETFONT) \
    { \
    SetMsgHandled(TRUE); \
    func((IFont*)wParam, (BOOL)LOWORD(lParam)); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
    }

// void OnSetDuiFocus()
#define MSG_WM_SETFOCUS_EX(func) \
    if (uMsg == WM_SETFOCUS) \
{ \
    SetMsgHandled(TRUE); \
    func(); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}

// void OnKillFocus()
#define MSG_WM_KILLFOCUS_EX(func) \
    if (uMsg == WM_KILLFOCUS) \
{ \
    SetMsgHandled(TRUE); \
    func(); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}

// void OnNcMouseHover(int nFlag,CPoint pt)
#define MSG_WM_NCMOUSEHOVER(func) \
    if(uMsg==WM_NCMOUSEHOVER)\
{\
    SetMsgHandled(TRUE); \
    func(wParam,CPoint(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam))); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}

// void OnNcMouseLeave()
#define MSG_WM_NCMOUSELEAVE(func) \
    if(uMsg==WM_NCMOUSELEAVE)\
{\
    SetMsgHandled(TRUE); \
    func(); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}


// void OnDuiTimer(char cTimerID)
#define MSG_WM_DUITIMER(func) \
    if (uMsg == WM_TIMER) \
{ \
    SetMsgHandled(TRUE); \
    func((char)wParam); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}

#define UM_DUI_TIMEREX    (WM_USER+5432)    //定义一个与HWND定时器兼容的DUI定时器

#define MSG_UM_TIMEREX(func) \
    if (uMsg == UM_DUI_TIMEREX) \
{ \
    SetMsgHandled(TRUE); \
    func(wParam); \
    lResult = 0; \
    if(IsMsgHandled()) \
    return TRUE; \
}
