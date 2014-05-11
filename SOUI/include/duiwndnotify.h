#pragma once

#ifndef HSTREEITEM
typedef ULONG_PTR HSTREEITEM;
#endif

//////////////////////////////////////////////////////////////////////////
// Notify Messages For DuiWin User
//////////////////////////////////////////////////////////////////////////

#include "DuiDef.h"

#define IDC_RICHVIEW_WIN            2000
#define UM_DUI_NOTIFY				(WM_USER+100)
namespace SOUI
{
class CDuiWindow;

#define NOTIFY_HANDLER_DUI(id, func) \
	if (uMsg == UM_DUI_NOTIFY && wParam == IDC_RICHVIEW_WIN) \
	{ \
	SetMsgHandled(TRUE); \
	lResult = func((LPDUINMHDR)lParam); \
	if(IsMsgHandled()) \
	return TRUE; \
	}

#define MSG_DUI_NOTIFY() NOTIFY_HANDLER_DUI(IDC_RICHVIEW_WIN,_OnDuiNotify)

#define DUI_NOTIFY_MAP_BEGIN()                   \
    LRESULT _OnDuiNotify(LPDUINMHDR pnmh)		\
    {                                           \
        UINT_PTR  uCode = pnmh->code;           \
 

#define DUI_NOTIFY_MAP_END()                     \
        SetMsgHandled(FALSE);                   \
        return FALSE;                           \
    }                                           \
 
#define DUI_CHAIN_NOTIFY_MAP(ChainClass)         \
        if(ChainClass::_OnDuiNotify(pnmh))       \
            return TRUE;                         \
 

// LRESULT OnDuiIDHandler(LPDUINMHDR pnmh)
#define DUI_NOTIFY_ID_HANDLER(id, cd, func) \
	if(cd == pnmh->code && id == pnmh->idFrom) \
	{ \
		return func(pnmh); \
	}

// LRESULT OnDuiNameHandler(LPDUINMHDR pnmh)
#define DUI_NOTIFY_NAME_HANDLER(name, cd, func) \
	if(cd == pnmh->code && pnmh->pszNameFrom!= NULL && strcmp(pnmh->pszNameFrom,name)==0) \
{ \
	return func(pnmh); \
}

// void OnDuiIDCommand()
#define DUI_NOTIFY_ID_COMMAND(itemid, func)                                  \
    if (DUINM_COMMAND == uCode && itemid == pnmh->idFrom)  \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \
 
// void OnDuiCommand()
#define DUI_NOTIFY_ID_COMMAND_RANGE(itemidbegin, itemidend, func)					\
	if (DUINM_COMMAND == uCode && itemidbegin <= pnmh->idFrom	\
		&& itemidend >= pnmh->idFrom )						\
	{																			\
		func(pnmh->idFrom);                                   \
		return TRUE;															\
	}																			\

// void OnDuiNameCommand()
#define DUI_NOTIFY_NAME_COMMAND(name, func)                                  \
	if (DUINM_COMMAND == uCode && pnmh->pszNameFrom!= NULL && strcmp(pnmh->pszNameFrom,name)==0)  \
	{                                                                       \
		func();                                                             \
		return TRUE;                                                        \
	}                                                                       \


// void OnDuiCommand(UINT uItemID)
#define DUI_NOTIFY_COMMAND(func)                                                     \
    if (DUINM_COMMAND == uCode)                                                      \
    {                                                                               \
        func(pnmh->idFrom);   \
        return TRUE;                                                                \
    }                                                                               \
 


// LRESULT OnDuiContextMenu(CPoint pt)
#define DUI_NOTIFY_ID_CONTEXTMENU(id,func)                                      \
	if (DUINM_CONTEXTMENU == uCode && pnmh->idFrom==id)                          \
{                                                                               \
	func(((LPDUINMCONTEXTMENU)pnmh)->pt);										\
	return TRUE;                                                                \
}                                                                               \


// LRESULT OnDuiContextMenu(CPoint pt)
#define DUI_NOTIFY_NAME_CONTEXTMENU(name,func)                                      \
	if (DUINM_CONTEXTMENU == uCode && pnmh->pszNameFrom!= NULL && strcmp(pnmh->pszNameFrom,name)==0) \
{                                                                               \
	return func(((LPDUINMCONTEXTMENU)pnmh)->pt);										\
}                                                                               \


// Command Notify
#define DUINM_COMMAND  1
typedef struct _DUINMCOMMAND
{
    DUINMHDR       hdr;
    ULONG_PTR   uItemData;
} DUINMCOMMAND, *LPDUINMCOMMAND;

// Command Notify
#define DUINM_CONTEXTMENU  2
typedef struct _DUINMCONTEXTMENU
{
	DUINMHDR       hdr;
	POINT		   pt;
	ULONG_PTR   uItemData;
} DUINMCONTEXTMENU, *LPDUINMCONTEXTMENU;


// Tab Sel Change Notify
#define DUINM_TAB_SELCHANGING  9

typedef struct _DUINMTABSELCHANGE
{
    DUINMHDR       hdr;
    UINT        uTabItemIDNew;
    UINT        uTabItemIDOld;
    BOOL        bCancel;
} DUINMTABSELCHANGE, *LPDUINMTABSELCHANGE;

// BOOL OnDuiTabSelChange(int nTabItemIDOld, int nTabItemIDNew)
#define DUI_NOTIFY_TAB_SELCHANGE(tabid, func)                                        \
    if (DUINM_TAB_SELCHANGING == uCode && tabid == ((SOUI::LPDUINMTABSELCHANGE)pnmh)->uTabID) \
    {                                                                               \
        BOOL bRet = func(                                                           \
            ((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDOld,                              \
            ((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDNew);                             \
        if (!bRet)                                                                  \
            ((LPDUINMTABSELCHANGE)pnmh)->bCancel = TRUE;                             \
        return TRUE;                                                                \
    }                                                                               \
 
// Tab Sel Change Notify
#define DUINM_TAB_SELCHANGED  10
typedef struct _DUINMTABSELCHANGED
{
    DUINMHDR       hdr;
    UINT        uTabID;
    UINT        uTabItemIDNew;
    UINT        uTabItemIDOld;
} DUINMTABSELCHANGED, *LPDUINMTABSELCHANGED;

// void OnDuiTabSelChanged(int nTabItemIDOld, int nTabItemIDNew)
#define DUI_NOTIFY_TAB_SELCHANGED(tabid, func)                                        \
	if (DUINM_TAB_SELCHANGED == uCode && tabid == ((SOUI::LPDUINMTABSELCHANGE)pnmh)->uTabID) \
{                                                                               \
	func(                                                           \
	((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDOld,                              \
	((LPDUINMTABSELCHANGE)pnmh)->uTabItemIDNew);                             \
	return TRUE;                                                                \
}                                                                               \
 
#define DUINM_TAB_ITEMHOVER	11
typedef struct _DUINMTABITEMHOVER
{
    DUINMHDR       hdr;
    UINT        iItem;
    CRect		rcItem;
} DUINMTABITEMHOVER, *LPDUINMTABITEMHOVER;

#define DUINM_TAB_ITEMLEAVE 12
typedef DUINMTABITEMHOVER DUINMTABITEMLEAVE, *LPDUINMTABITEMLEAVE;

class CDuiScrollBar;
#define DUINM_SCROLL	14
typedef struct tagDUINMSCROLL
{
    DUINMHDR       hdr;
    UINT		uSbCode;
    int			nPos;
    BOOL		bVertical;
    CDuiScrollBar *pScrollBar;
} DUINMSCROLL,*PDUINMSCROLL;

// void OnDuiScroll(UINT uSbCode,int nPos,SOUI::CDuiScrollBar *pBar)
#define DUI_NOTIFY_SCROLL(id, func)                                  \
	if (DUINM_SCROLL == uCode && id == pnmh->idFrom)  \
{                                                                       \
	func(((PDUINMSCROLL)pnmh)->uSbCode,((SOUI::PDUINMSCROLL)pnmh)->nPos,((SOUI::PDUINMSCROLL)pnmh)->pScrollBar);   \
	return TRUE;                                                        \
}

class CDuiItemPanel;

// Item Click Notify
#define DUINM_LBITEMNOTIFY     15

typedef struct tagDUINMITEMNOTIFY
{
    DUINMHDR       hdr;
    LPDUINMHDR		pOriginHdr;	//原始消息
    CDuiItemPanel *pItem;
    CDuiWindow *	pHostDuiWin;
} DUINMITEMNOTIFY, *LPDUINMITEMNOTIFY;


#define DUINM_ITEMMOUSEEVENT	16
typedef struct tagDUINMITEMMOUSEEVENT
{
    DUINMHDR       hdr;
    CDuiItemPanel *	pItem;
    UINT		uMsg;
    WPARAM		wParam;
    LPARAM		lParam;
} DUINMITEMMOUSEEVENT, *LPDUINMITEMMOUSEEVENT;



// Get Display Info Notify
#define DUINM_GETLBDISPINFO  17
typedef struct tagDUINMGETLBDISPINFO
{
    DUINMHDR       hdr;
    int         nListItemID;
    BOOL        bHover;
    BOOL        bSelect;
    CDuiItemPanel *	pItem;
    CDuiWindow *	pHostDuiWin;
} DUINMGETLBDISPINFO, *LPDUINMGETLBDISPINFO;

#define DUINM_LBSELCHANGING 18
#define DUINM_LBSELCHANGED	19
typedef struct tagDUINMLBSELCHANGE
{
    DUINMHDR       hdr;
    int nNewSel;
    int nOldSel;
    UINT uHoverID;
    UINT uMsg;
} DUINMLBSELCHANGE, *LPDUINMLBSELCHANGE;


// Get Display Info Notify
#define DUINM_GETTBDISPINFO  20
typedef struct tagDUINMGETTBDISPINFO
{
    DUINMHDR       hdr;
    HSTREEITEM  hItem;
    BOOL        bHover;
    BOOL        bSelect;
    CDuiItemPanel *	pItem;
    CDuiWindow *	pHostDuiWin;
} DUINMGETTBDISPINFO, *LPDUINMGETTBDISPINFO;

#define DUINM_TBSELCHANGING	21
typedef struct tagDUINMTBSELCHANGING
{
	DUINMHDR       hdr;
	HSTREEITEM hNewSel;
	HSTREEITEM hOldSel;
	BOOL		bCancel;
} DUINMTBSELCHANGING, *LPDUINMTBSELCHANGING;


#define DUINM_TBSELCHANGED	22
typedef struct tagDUINMTBSELCHANGED
{
    DUINMHDR       hdr;
    HSTREEITEM hNewSel;
    HSTREEITEM hOldSel;
} DUINMTBSELCHANGED, *LPDUINMTBSELCHANGED;


#define DUINM_RICHEDIT_NOTIFY	25
typedef struct tagDUIRICHEDITNOTIFY
{
    DUINMHDR hdr;
    DWORD iNotify;
    LPVOID pv;
} DUIRICHEDITNOTIFY,*LPDUIRICHEDITNOTIFY;

class CDuiSliderBar;
#define DUINM_SLIDER	30
typedef struct tagDUINMSLIDER
{
	DUINMHDR hdr;
	UINT uSbCode;
	CDuiSliderBar *pSliderBar;
	int	 nPos;
	BOOL bVertical;
} DUINMSLIDER,*LPDUINMSLIDER;

//headerctrl
#define DUINM_HDCLICK	31	//点击表头
typedef struct tagDUINMHDCLICK
{
	DUINMHDR hdr;
	int   iItem;
} DUINMHDCLICK,*LPDUINMHDCLICK;

#define DUINM_HDSIZECHANGING	32	//调整表头宽度中
typedef struct tagDUINMHDSIZECHANGING
{
	DUINMHDR hdr;
	int   iItem;
	int   nWidth;
} DUINMHDSIZECHANGING,*LPDUINMHDSIZECHANGING;

#define DUINM_HDSIZECHANGED	33	//调整表头宽度完成
typedef struct tagDUINMHDSIZECHANGED
{
	DUINMHDR hdr;
	int   iItem;
	int   nWidth;
} DUINMHDSIZECHANGED,*LPDUINMHDSIZECHANGED;

#define DUINM_HDSWAP	34	//拖动表项调整位置
typedef struct tagDUINMHDSWAP
{
	DUINMHDR hdr;
	int   iOldIndex;
	int	  iNewIndex;
} DUINMHDSWAP,*LPDUINMHDSWAP;

//calendar
#define DUINM_CALENDAR_SELECTDAY	40
typedef struct tagDUINMCALENDARSELECTDAY
{
	DUINMHDR hdr;
	WORD   wOldDay;
	WORD   wNewDay;
} DUINMCALENDARSELECTDAY,*LPDUINMCALENDARSELECTDAY;


//////////////////////////////////////////////////////////////////////////
//  internal notify message

#define DUINM_INTERNAL_FIRST	1000
#define DUINM_INTERNAL_LAST		2000

class CDuiRealWnd;

typedef struct _DUINMREALWNDCMN
{
    DUINMHDR       hdr;
	CDuiRealWnd	* pRealWnd;
} DUINMREALWNDCMN, *LPDUINMREALWNDCMN;

typedef struct _DUINMREALWNDMSGPROC
{
    DUINMHDR       hdr;
    HWND		hWnd;
    UINT		uMsg;
    WPARAM		wParam;
    LPARAM		lParam;
    BOOL		bMsgHandled;
} DUINMREALWNDMSGPROC;

#define DUINM_REALWND_CREATE	DUINM_INTERNAL_FIRST
#define DUINM_REALWND_INIT		(DUINM_INTERNAL_FIRST+1)
#define DUINM_REALWND_DESTROY	(DUINM_INTERNAL_FIRST+2)
#define DUINM_REALWND_SIZE	(DUINM_INTERNAL_FIRST+3)

}//namespace SOUI
