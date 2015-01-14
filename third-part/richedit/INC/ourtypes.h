//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*
 *	OURTYPES.H
 *
 */

#ifndef _OURTYPES_H_
#define _OURTYPES_H_


#ifndef CONST
#define CONST               const
#endif

// WM_SYSKEYDOWN masks for lKeyData
#define SYS_ALTERNATE		0x20000000
#define SYS_PREVKEYSTATE	0x40000000


#if	!defined(MACPORT) && !defined(CHICAGO)
#define	GreyThisDialog(_hwnd)	Ctl3dSubclassDlgEx(_hwnd, CTL3D_ALL)
#else	/* CHICAGO || MACPORT */
#define	GreyThisDialog(_hwnd)
#endif	/* CHICAGO || MACPORT */

#ifndef WM_PAINTICON
#define WM_PAINTICON	    0x0026
#endif

#define WINDOWS
#define NOSHELLDEBUG
#ifndef MAX_PATH
  // This constant is defined in \mbudev\inc32\windef.h and is needed
  // by fcext.h.
# define MAX_PATH	260
#endif
#ifdef WIN16

  // These typedef's are necessary because the Chicago Shell is 32-bit,
  // and doesn't care that the data types in question are not present
  // in Win3.x

typedef struct _netixx { unsigned long uFiller; } NETRESOURCE, FAR * LPNETRESOURCE;
typedef BOOL FAR *	LPBOOL;
typedef struct { short x, y; } POINTS;

  // The following message crackers were not defined for Win16.

/* void Cls_OnDropFiles(HWND hwnd, HDROP hdrop) */
#define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HDROP)(wParam)), 0L)
#define FORWARD_WM_DROPFILES(hwnd, hdrop, fn) \
    (void)(fn)((hwnd), WM_DROPFILES, (WPARAM)(HDROP)(hdrop), 0L)
#endif

#ifndef CHICAGO
	// The following data structure is only defined in Chicago's WinUser.h,
	// but is used with abandon in Chicago's shlobj.h. This forces us to
	// define the bogoid data structure to shut up the compiler. RATS! -jkl

	// typedef void * LPDROPSTRUCT;

#endif

typedef LONG		PFN;

typedef UINT		BIT;

// Chicago (Windows 4.0) API's and #defines
#if	!defined(CHICAGO)
// #define MB_HELP             0x4000 // Help Button
#endif

// This flag should be set in a function that we have decided
// may not use the flag MAPI_DEFERRED_ERRORS
#define	NO_DEFER	0

/*
 * CopyDis Resource Type & Name
 *	If you change these, make sure you rebuild MlSetup
 *		as it tries to stamp MapiDlg with the CD Data.
 *  Also change them in the lang\usa\setup2\msmail.stf file.
 */
#define RT_CDDATATYPE	106
#define RES_CDDATANAME	96

// Capone-style definitions for buttons (9 matches Chicago's IDHELP)
#define PSB_OK 		IDOK
#define PSB_Cancel	IDCANCEL
#define PSB_Help	9

// Windows does not provide defines for WM_NCMOUSEFIRST and WM_NCMOUSELAST
// as is done for MOUSE and KEY events.

//#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
//#define WM_NCMOUSELAST WM_NCMBUTTONDBLCLK


#ifdef WIN32

#define	WINCAPI		__cdecl
#define	__loadds
#define _export

#else	/* !WIN32 */

#include <stdarg.h>					// Included in win32 windows.h, winuser.h

// We wouldn't need the below if we used the Win4.0 header file.
typedef const RECT FAR *        LPCRECT;


// Chicago removed these from shellapi.h
#define REG_SZ			1	    /* string type */
#define ERROR_SUCCESS           0L

#ifndef CHICAGO
#define EM_SCROLL               (WM_USER+5)     // from Chicago windows.h
#endif

// These are here since windows.h is broken

#undef	WINAPI
#define	WINAPI			_export _far _pascal
#undef	WINCAPI
#define	WINCAPI			_export _far _cdecl
#undef	CALLBACK
#define	CALLBACK		_export _far _pascal


typedef	int				INT;
typedef	unsigned long	ULONG;
typedef	short			SHORT;
typedef	unsigned short	USHORT;
typedef	char			TCHAR;
typedef char			CHAR;
typedef	unsigned short	WCHAR;
typedef WCHAR *			LPWSTR;
typedef const WCHAR *	LPCWSTR;
typedef TCHAR *			LPTSTR;
typedef TCHAR *			PTCHAR;
typedef double		LONGLONG;

#define	TEXT(a)			a
#define _T 				TEXT

#define	GetModuleFileNameA			GetModuleFileName
#define	LoadStringA					LoadString
#define	RegisterWindowMessageA		RegisterWindowMessage
#define	MessageBoxA					MessageBox
#define DialogBoxA					DialogBox
#define	SendMessageA				SendMessage
#define	SetDlgItemTextA				SetDlgItemText
#define	SetPropA					SetProp
#define	GetPropA					GetProp
#define	RemovePropA					RemoveProp
#define	TextOutA					TextOut
#define	OutputDebugStringA			OutputDebugString
#define	lstrlenA					lstrlen
#define	lstrcpyA					lstrcpy
#define	wsprintfA					wsprintf
#define	wvsprintfA					wvsprintf
#define GlobalGetAtomNameA			GlobalGetAtomName
#define GlobalAddAtomA				GlobalAddAtom
#define RegisterClipboardFormatA	RegisterClipboardFormat
#define	CharUpper					AnsiUpper
#define	CharNext					AnsiNext
#define	CharPrev					AnsiPrev

#define	GWL_USERDATA		DWL_USER

// Win16 does not provide a declaration for the DialogBoxHeader structure
// defined in the Windows 3.1 Programmer's Reference, Volume 4: Resource,
// Page 90. I fake this and make it source compatible with Win32 by
// declaring the following type.
// Surrounded with _MSC_VER to prevent it from being used in RC.

#ifdef _MSC_VER
#pragma pack(1)

typedef struct {
    DWORD	style;
	BYTE	cdit;
	WORD	x;
	WORD	y;
	WORD	cx;
	WORD	cy;
} DLGTEMPLATE;

typedef struct {
	WORD	x;
	WORD	y;
	WORD	cx;
	WORD	cy;
	WORD	id;
	DWORD	style;
} DLGITEMTEMPLATE;

#pragma pack()
#endif

#ifdef WIN31
// These Registry API functions are not declared in the version of SHELLAPI.H
// in \capone\extern\inc\16, but are in \mbudev\inc16

#define	HKEY_CLASSES_ROOT	1

typedef DWORD HKEY;
typedef HKEY FAR* PHKEY;

LONG WINAPI RegOpenKey(HKEY, LPCSTR, HKEY FAR*);
LONG WINAPI RegCreateKey(HKEY, LPCSTR, HKEY FAR*);
LONG WINAPI RegCloseKey(HKEY);
LONG WINAPI RegDeleteKey(HKEY, LPCSTR);
LONG WINAPI RegSetValue(HKEY, LPCSTR, DWORD, LPCSTR, DWORD);
LONG WINAPI RegQueryValue(HKEY, LPCSTR, LPSTR, LONG FAR*);
LONG WINAPI RegEnumKey(HKEY, DWORD, LPSTR, DWORD);

#endif

#ifndef __MAPIWIN_H__
typedef void * WIN32_FIND_DATA;
typedef const TCHAR FAR *   LPCTSTR;
#define	CopyMemory					hmemcpy
#define ZeroMemory(_pv, _cb)	memset( (_pv), 0, (_cb) )
#endif


#endif	/* !WIN32 */


/*
 *	C a p o n e   G l o b a l   C o n s t a n t s
 */


// Maximum size of folder names and message subjects, including terminator
#define cchMaxTextName 256
#define cchLimitTextName (cchMaxTextName - 1)


/*
 * Common offsets used by Insert.Message and Insert.File
 */
#define insertasNormal	0
#define insertasText	1
#define insertasLink	2

/*
 *	B o o l e a n   S t u f f
 */


#define FIff(_a, _b) (!(_a) == !(_b))
#define FImplies(_a, _b) (!(_a) || (_b))


/*
 *	C a p o n e   D i a l o g   S t a n d a r d s
 */



#ifdef MACPORT
// Font sizes in dialogs
#define CDS_SIZE 10
#define CDS_SIZE_HEX 0x000a
#define CDS_SIZE_FORM 10

#define CDS_FONT "MS Sans Serif"
#else
// Font sizes in dialogs
#define CDS_SIZE 8
#define CDS_SIZE_HEX 0x0008
#define CDS_SIZE_FORM 8

#define CDS_FONT "MS Sans Serif"
#endif

#ifdef	CHICAGO
#define CDS_DS DS_3DLOOK
#else
#define CDS_DS 0
#endif	

// In stuff below, CX/CY is size of ctl, whereas DX/DY denotes positioning

// Margins around dialog
#define CDS_DX_DLG_MARGIN	7
#define CDS_DY_DLG_MARGIN	7

// Margins inside Group boxes
#define CDS_DX_GRP_MARGIN	5
#define CDS_DY_GRP_MARGIN	5
#define CDS_DY_GRP_TOP		10
#define CDS_DY_GRP_TOPBOX	8

// Margins between controls
#define CDS_DX_CTL_MARGIN	3
#define CDS_DY_CTL_MARGIN	3

// Height of controls					
#define CDS_CY_CTL			14

// Edit controls are set in a bit
#define CDS_DY_EDT			1
#define CDS_CY_EDT			12

// Check boxes are set in a bit
#define CDS_DY_CHK			1
#define CDS_CY_CHK			12

// Combo boxes are set in a bit
#define CDS_DY_CMB			1
#define CDS_CY_CMB			12
#define CDS_CY_CMB_ROW		10
#define CDS_CY_CMB_3ROWS	30
#define CDS_CY_CMB_5ROWS	42

// Labels are set in even more
#define CDS_DY_TXT			3
#define CDS_CY_TXT			11

// Push buttons are just right
#define CDS_DY_PSB			0
#define CDS_CY_PSB			14
#define CDS_CX_PSB			50

// Button row offsets
#define CDS_DY_PSB1			0
#define CDS_DY_PSB2			17
#define CDS_DY_PSB3			34
#define CDS_DY_PSB4			51
#define CDS_DY_PSB5			68

// Backward compatibility
#define CAPONE_FONT 			CDS_FONT
#define CAPONE_FONT_SIZE		CDS_SIZE
#define CAPONE_FONT_SIZE_HEX	CDS_SIZE_HEX
#define DS_CAPONE				CDS_DS



/*
 *	S e g m e n t s
 */


#ifdef WIN16
# define CODESEG  __based(__segname("_CODE"))
#else
# define CODESEG
#endif

#ifdef _MSC_VER
//#	if defined (WIN32) && !defined (MACPORT)
#	if defined (WIN32)
#		ifndef _OLEERROR_H_
#			include <objerror.h>
#		endif
#		ifndef _OBJBASE_H_
#			include <objbase.h>
#		endif
#	else
#		ifndef _COMPOBJ_H_
#			include <compobj.h>
#		endif
#	endif
#endif

/*
 *	Stolen from 32-bit NTOLE "Beta3" compobj.h
 *
 *	Need _MSC_VER to prevert RC from barfing
 */
//$ FUTURE: Remove
#if defined(_MSC_VER)
//#	if defined (WIN32) && !defined (MACPORT)
#	if defined (WIN32)
#		define NTOLE_BETA3
#	else	// !WIN32
#		ifndef OLESTR
#			pragma message("Defining OLECHAR")
#			define LPOLESTR    	LPSTR
#			define LPCOLESTR   	LPCSTR
#			define OLECHAR     	char
#			define OLESTR(str) 	str
#		endif	//	!OLECHAR
#	endif	// !WIN32
#endif	// !_MSC_VER

#if defined(NTOLE_BETA3) && !defined(MACPORT)

#ifndef MakeOLESTR
#define CchSzAToSzW(_szA, _szW, _cbSzW)	\
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (_szA), -1, (_szW),	\
						(_cbSzW) / sizeof(WCHAR))

#define CchSzWToSzA(_szW, _szA, _cbSzA)	\
	WideCharToMultiByte(CP_ACP, 0, (_szW), -1, (_szA), (_cbSzA), NULL, NULL)

#define UsesMakeOLESTRX(_cchMax)	WCHAR szWT[_cchMax]
#define UsesMakeOLESTR				UsesMakeOLESTRX(MAX_PATH)
#define MakeOLESTR(_szA)	\
	(CchSzAToSzW((_szA), szWT, sizeof(szWT)) ? szWT : NULL)

#define UsesMakeANSIX(_cchMax)		CHAR szAT[_cchMax * 2]
#define UsesMakeANSI				UsesMakeANSIX(MAX_PATH)
#define MakeANSI(_szW)		\
	(CchSzWToSzA((_szW), szAT, sizeof(szAT)) ? szAT : NULL)
#endif	// !MakeOLESTR

HRESULT HrSzAFromSzW(LPWSTR szW, LPSTR * psz);

#else	// !NTOLE_BETA3

#ifndef MakeOLESTR
#define UsesMakeOLESTRX(_cchMax)	/##/
#define UsesMakeOLESTR				/##/
#define MakeOLESTR(_szA)		(_szA)

#define UsesMakeANSIX(_cchMax)		/##/
#define UsesMakeANSI				/##/
#define MakeANSI(_szW)			(_szW)
#endif	// !MakeOLESTR

#endif	// !WIN32

//#ifndef RC_INVOKED
//#include <gapidef.h>
//#include <gapi.h>
//#endif  /* RC_INVOKED */

#if defined (MACPORT) && defined (WS_SYSMENU)
#undef WS_SYSMENU
#define WS_SYSMENU 0x00
#endif //MACPORT && ws_sysmenu

// Help message for context-sensitive help
#ifndef CHICAGO
#define WM_HELP					0x0053
#endif // CHICAGO

#endif //_OURTYPES_H_



