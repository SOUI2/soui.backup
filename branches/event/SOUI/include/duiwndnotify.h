#pragma once

#ifndef HSTREEITEM
typedef ULONG_PTR HSTREEITEM;
#endif

//////////////////////////////////////////////////////////////////////////
// Notify Messages For SOUI Window User
//////////////////////////////////////////////////////////////////////////

#include "DuiDef.h"

#define IDC_RICHVIEW_WIN            2000
#define UM_SWND_NOTIFY                (WM_USER+100)

namespace SOUI
{
class SWindow;

#define NOTIFY_HANDLER_SOUI(id, func) \
    if (uMsg == UM_SWND_NOTIFY && wParam == IDC_RICHVIEW_WIN) \
    { \
    SetMsgHandled(TRUE); \
    lResult = func((LPSNMHDR)lParam); \
    if(IsMsgHandled()) \
    return TRUE; \
    }

#define MSG_SOUI_NOTIFY() NOTIFY_HANDLER_SOUI(IDC_RICHVIEW_WIN,_OnDuiNotify)

#define SOUI_NOTIFY_MAP_BEGIN()                   \
    LRESULT _OnDuiNotify(LPSNMHDR pnmh)        \
    {                                           \
        UINT_PTR  uCode = pnmh->code;           \
 

#define SOUI_NOTIFY_MAP_END()                     \
        SetMsgHandled(FALSE);                   \
        return FALSE;                           \
    }                                           \
 
#define DUI_CHAIN_NOTIFY_MAP(ChainClass)         \
        if(ChainClass::_OnDuiNotify(pnmh))       \
            return TRUE;                         \
 

// LRESULT OnDuiHandler(LPSNMHDR pnmh)
#define SOUI_NOTIFY_HANDLER(cd, func) \
    if(cd == pnmh->code) \
{ \
    return func(pnmh); \
} 


// LRESULT OnDuiIDHandler(LPSNMHDR pnmh)
#define SOUI_NOTIFY_ID_HANDLER(id, cd, func) \
    if(cd == pnmh->code && id == pnmh->idFrom) \
    { \
        return func(pnmh); \
    }

// LRESULT OnDuiNameHandler(LPSNMHDR pnmh)
#define SOUI_NOTIFY_NAME_HANDLER(name, cd, func) \
    if(cd == pnmh->code && pnmh->pszNameFrom!= NULL && wcscmp(pnmh->pszNameFrom,name)==0) \
{ \
    return func(pnmh); \
}

// void OnDuiIDCommand()
#define SOUI_NOTIFY_ID_COMMAND(itemid, func)                                  \
    if (NM_COMMAND == uCode && itemid == pnmh->idFrom)  \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \
 
// void OnDuiCommand()
#define SOUI_NOTIFY_ID_COMMAND_RANGE(itemidbegin, itemidend, func)                    \
    if (NM_COMMAND == uCode && itemidbegin <= pnmh->idFrom    \
        && itemidend >= pnmh->idFrom )                        \
    {                                                                            \
        func(pnmh->idFrom);                                   \
        return TRUE;                                                            \
    }                                                                            \

// void OnDuiNameCommand()
#define SOUI_NOTIFY_NAME_COMMAND(name, func)                                  \
    if (NM_COMMAND == uCode && pnmh->pszNameFrom!= NULL && wcscmp(pnmh->pszNameFrom,name)==0)  \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \


// void OnDuiCommand(UINT uItemID)
#define SOUI_NOTIFY_COMMAND(func)                                                     \
    if (NM_COMMAND == uCode)                                                      \
    {                                                                               \
        func(pnmh->idFrom);   \
        return TRUE;                                                                \
    }                                                                               \
 


// LRESULT OnDuiContextMenu(CPoint pt)
#define SOUI_NOTIFY_ID_CONTEXTMENU(id,func)                                      \
    if (NM_CONTEXTMENU == uCode && pnmh->idFrom==id)                          \
{                                                                               \
    func(((LPDUINMCONTEXTMENU)pnmh)->pt);                                        \
    return TRUE;                                                                \
}                                                                               \


// LRESULT OnDuiContextMenu(CPoint pt)
#define SOUI_NOTIFY_NAME_CONTEXTMENU(name,func)                                      \
    if (NM_CONTEXTMENU == uCode && pnmh->pszNameFrom!= NULL && wcscmp(pnmh->pszNameFrom,name)==0) \
{                                                                               \
    return func(((LPDUINMCONTEXTMENU)pnmh)->pt);                                        \
}                                                                               \

/*
// Command Notify
#define NM_COMMAND  1
typedef struct _DUINMCOMMAND
{
    DUINMHDR       hdr;
    ULONG_PTR   uItemData;
} DUINMCOMMAND, *LPDUINMCOMMAND;

// Command Notify
#define NM_CONTEXTMENU  2
typedef struct _DUINMCONTEXTMENU
{
    DUINMHDR       hdr;
    POINT           pt;
    ULONG_PTR   uItemData;
} DUINMCONTEXTMENU, *LPDUINMCONTEXTMENU;


// Tab Sel Change Notify
#define NM_TAB_SELCHANGING  9

typedef struct _DUINMTABSELCHANGE
{
    DUINMHDR       hdr;
    UINT        uTabItemIDNew;
    UINT        uTabItemIDOld;
    BOOL        bCancel;
} DUINMTABSELCHANGE, *LPDUINMTABSELCHANGE;

// BOOL OnDuiTabSelChange(int nTabItemIDOld, int nTabItemIDNew)
#define SOUI_NOTIFY_TAB_SELCHANGE(tabid, func)                                        \
    if (NM_TAB_SELCHANGING == uCode && tabid == ((SOUI::LPDUINMTABSELCHANGE)pnmh)->uTabID) \
    {                                                                               \
        BOOL bRet = func(                                                           \
            ((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDOld,                              \
            ((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDNew);                             \
        if (!bRet)                                                                  \
            ((LPDUINMTABSELCHANGE)pnmh)->bCancel = TRUE;                             \
        return TRUE;                                                                \
    }                                                                               \
 
// Tab Sel Change Notify
#define NM_TAB_SELCHANGED  10
typedef struct _DUINMTABSELCHANGED
{
    DUINMHDR       hdr;
    UINT        uTabID;
    UINT        uTabItemIDNew;
    UINT        uTabItemIDOld;
} DUINMTABSELCHANGED, *LPDUINMTABSELCHANGED;

// void OnDuiTabSelChanged(int nTabItemIDOld, int nTabItemIDNew)
#define SOUI_NOTIFY_TAB_SELCHANGED(tabid, func)                                        \
    if (NM_TAB_SELCHANGED == uCode && tabid == ((SOUI::LPDUINMTABSELCHANGE)pnmh)->uTabID) \
{                                                                               \
    func(                                                           \
    ((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDOld,                              \
    ((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDNew);                             \
    return TRUE;                                                                \
}                                                                               \
 
#define NM_TAB_ITEMHOVER    11
typedef struct _DUINMTABITEMHOVER
{
    DUINMHDR       hdr;
    UINT        iItem;
    CRect        rcItem;
} DUINMTABITEMHOVER, *LPDUINMTABITEMHOVER;

#define NM_TAB_ITEMLEAVE 12
typedef DUINMTABITEMHOVER DUINMTABITEMLEAVE, *LPDUINMTABITEMLEAVE;

class SScrollBar;
#define NM_SCROLL    14
typedef struct tagDUINMSCROLL
{
    DUINMHDR       hdr;
    UINT        uSbCode;
    int            nPos;
    BOOL        bVertical;
    SScrollBar *pScrollBar;
} DUINMSCROLL,*PDUINMSCROLL;

// void OnDuiScroll(UINT uSbCode,int nPos,SOUI::CDuiScrollBar *pBar)
#define SOUI_NOTIFY_SCROLL(id, func)                                  \
    if (NM_SCROLL == uCode && id == pnmh->idFrom)  \
{                                                                       \
    func(((PDUINMSCROLL)pnmh)->uSbCode,((SOUI::PDUINMSCROLL)pnmh)->nPos,((SOUI::PDUINMSCROLL)pnmh)->pScrollBar);   \
    return TRUE;                                                        \
}

class SItemPanel;

// Item Click Notify
#define NM_LBITEMNOTIFY     15

typedef struct tagDUINMITEMNOTIFY
{
    DUINMHDR       hdr;
    LPSNMHDR        pOriginHdr;    //原始消息
    SItemPanel *pItem;
    SWindow *    pHostDuiWin;
} DUINMITEMNOTIFY, *LPDUINMITEMNOTIFY;


#define NM_ITEMMOUSEEVENT    16
typedef struct tagDUINMITEMMOUSEEVENT
{
    DUINMHDR       hdr;
    SItemPanel *    pItem;
    UINT        uMsg;
    WPARAM        wParam;
    LPARAM        lParam;
} DUINMITEMMOUSEEVENT, *LPDUINMITEMMOUSEEVENT;



// Get Display Info Notify
#define NM_GETLBDISPINFO  17
typedef struct tagDUINMGETLBDISPINFO
{
    DUINMHDR       hdr;
    int         nListItemID;
    BOOL        bHover;
    BOOL        bSelect;
    SItemPanel *    pItem;
    SWindow *    pHostDuiWin;
} DUINMGETLBDISPINFO, *LPDUINMGETLBDISPINFO;

#define NM_LBSELCHANGING 18
#define NM_LBSELCHANGED    19
typedef struct tagDUINMLBSELCHANGE
{
    DUINMHDR       hdr;
    int nNewSel;
    int nOldSel;
    UINT uHoverID;
} DUINMLBSELCHANGE, *LPDUINMLBSELCHANGE;


// Get Display Info Notify
#define NM_GETTBDISPINFO  20
typedef struct tagDUINMGETTBDISPINFO
{
    DUINMHDR       hdr;
    HSTREEITEM  hItem;
    BOOL        bHover;
    BOOL        bSelect;
    SItemPanel *    pItem;
    SWindow *    pHostDuiWin;
} DUINMGETTBDISPINFO, *LPDUINMGETTBDISPINFO;

#define NM_TBSELCHANGING    21
typedef struct tagDUINMTBSELCHANGING
{
    DUINMHDR       hdr;
    HSTREEITEM hNewSel;
    HSTREEITEM hOldSel;
    BOOL        bCancel;
} DUINMTBSELCHANGING, *LPDUINMTBSELCHANGING;


#define NM_TBSELCHANGED    22
typedef struct tagDUINMTBSELCHANGED
{
    DUINMHDR       hdr;
    HSTREEITEM hNewSel;
    HSTREEITEM hOldSel;
} DUINMTBSELCHANGED, *LPDUINMTBSELCHANGED;


#define NM_RICHEDIT_NOTIFY    25
typedef struct tagDUIRICHEDITNOTIFY
{
    DUINMHDR hdr;
    DWORD iNotify;
    LPVOID pv;
} DUIRICHEDITNOTIFY,*LPDUIRICHEDITNOTIFY;

class SSliderBar;
#define NM_SLIDER    30
typedef struct tagDUINMSLIDER
{
    DUINMHDR hdr;
    UINT uSbCode;
    SSliderBar *pSliderBar;
    int     nPos;
    BOOL bVertical;
} DUINMSLIDER,*LPDUINMSLIDER;

//headerctrl
#define NM_HDCLICK    31    //点击表头
typedef struct tagDUINMHDCLICK
{
    DUINMHDR hdr;
    int   iItem;
} DUINMHDCLICK,*LPDUINMHDCLICK;

#define NM_HDSIZECHANGING    32    //调整表头宽度中
typedef struct tagDUINMHDSIZECHANGING
{
    DUINMHDR hdr;
    int   iItem;
    int   nWidth;
} DUINMHDSIZECHANGING,*LPDUINMHDSIZECHANGING;

#define NM_HDSIZECHANGED    33    //调整表头宽度完成
typedef struct tagDUINMHDSIZECHANGED
{
    DUINMHDR hdr;
    int   iItem;
    int   nWidth;
} DUINMHDSIZECHANGED,*LPDUINMHDSIZECHANGED;

#define NM_HDSWAP    34    //拖动表项调整位置
typedef struct tagDUINMHDSWAP
{
    DUINMHDR hdr;
    int   iOldIndex;
    int      iNewIndex;
} DUINMHDSWAP,*LPDUINMHDSWAP;

//calendar
#define NM_CALENDAR_SELECTDAY    40
typedef struct tagDUINMCALENDARSELECTDAY
{
    DUINMHDR hdr;
    WORD   wOldDay;
    WORD   wNewDay;
} DUINMCALENDARSELECTDAY,*LPDUINMCALENDARSELECTDAY;


#define NM_CBSELCHANGE 50
*/
//////////////////////////////////////////////////////////////////////////
//  internal notify message

#define NM_INTERNAL_FIRST    1000
#define NM_INTERNAL_LAST        2000

class SRealWnd;

typedef struct _DUINMREALWNDCMN
{
    DUINMHDR       hdr;
    SRealWnd    * pRealWnd;
} DUINMREALWNDCMN, *LPDUINMREALWNDCMN;

typedef struct _DUINMREALWNDMSGPROC
{
    DUINMHDR       hdr;
    HWND        hWnd;
    UINT        uMsg;
    WPARAM        wParam;
    LPARAM        lParam;
    BOOL        bMsgHandled;
} DUINMREALWNDMSGPROC;

#define NM_REALWND_CREATE    NM_INTERNAL_FIRST
#define NM_REALWND_INIT        (NM_INTERNAL_FIRST+1)
#define NM_REALWND_DESTROY    (NM_INTERNAL_FIRST+2)
#define NM_REALWND_SIZE    (NM_INTERNAL_FIRST+3)

}//namespace SOUI
