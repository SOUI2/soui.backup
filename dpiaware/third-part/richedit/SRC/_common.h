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
 *	_COMMON.H
 *	
 *	Purpose:
 *		RICHEDIT private common definitions
 *
 */

#ifndef _COMMON_H

#pragma message ("Compiling Common.H")

#define _COMMON_H

// disable 
//         4710 : function not expanded
//         4706 : assignment within conditional; FUTURE (alexgo) code around this.
//         4512 : assignment operator not generated
//         4505 : removed unreferenced local function
//         4305 : conversion warning; (FUTURE) turn this on!
//         4306 : conversion warning; (FUTURE) turn this on!
//         4244 : truncation warning; (FUTURE) turn this on!
//         4201 : nameless struct union
//         4127 : conditional expression is constant; (FUTURE) can this be worked out?
//         4100 : unreferenced formal; (FUTURE) there is a better way in C++
//         4097 : typedef name used as synonym for class name
//		   4514 : unreferenced inline function has been removed
#pragma warning (disable : 4710 4706 4512 4505 4305 4306 4244 4201 4127 4100 4097 4514 4311 4312)

#ifdef NT
	#ifndef WINNT
	#define WINNT
	#endif
#endif

#ifdef WIN32
	#define BEGIN_CODESPACE_DATA
	#define END_CODESPACE_DATA
#endif

#define LOCAL static

#ifdef UNICODE
	#ifndef _UNICODE
	#define _UNICODE
	#endif
#endif

#ifndef STRICT
#define STRICT
#endif

#define NOSHELLDEBUG			//disables asserts in shell.h

#include <limits.h>
#if defined(DEBUG) && !defined(PEGASUS)
#include <stdio.h>
#endif

#include <windows.h>
#include <windowsx.h>

#ifndef MACPORT
#include <imm.h>
#else
#include <tchar.h>
#include "wlmimm.h"
#endif	//MACPORT

/*
 *	Types
 */
#include <tchar.h>
#include <ourtypes.h>

/*
 *	Debug support
 */
#include <dbug32.h>
#include <atldef.h>
#ifndef ASSERT
#define ASSERT ATLASSERT
#endif

// for the benefit of the outside world, richedit.h uses cpMax instead
// of cpMost. I highly prefer cpMost
#ifdef cpMax
#error "cpMax hack won't work"
#endif

#define cpMax cpMost
#include <richedit.h>
#include <richole.h>
#undef cpMax

// Our Win32 wrapper class
#include "_w32sys.h"

#include "resource.h"

// Use explicit ASCII values for LF and CR, since MAC compilers
// interchange values of '\r' and '\n'
#define	LF			10
#define	CR			13
#define FF			TEXT('\f')
#define TAB			TEXT('\t')
#define CELL		7
#define VT			TEXT('\v')
#define	PS			0x2029
#define SOFTHYPHEN	0xAD

#define BOM			0xFEFF
#define BULLET		0x2022
#define EMDASH		0x2014
#define EMSPACE		0x2003
#define ENDASH		0x2013
#define	ENQUAD		0x2000
#define ENSPACE		0x2002
#define KASHIDA		0x0640
#define LDBLQUOTE	0x201c
#define LQUOTE		0x2018
#define LTRMARK		0x200E
#define RDBLQUOTE	0x201D
#define RQUOTE		0x2019
#define RTLMARK		0x200F
#define SOFTHYPHEN	0xAD
#define	UTF16		0xDC00
#define	UTF16_LEAD	0xD800
#define	UTF16_TRAIL	0xDC00
#define ZWJ			0x200D
#define ZWNJ		0x200C

#ifdef PWD_JUPITER
// GuyBark Jupiter: Used to preserve CRLF in table cells.
#define PWD_CRLFINCELL  0xFFFA
#endif // PWD_JUPITER

// Return TRUE if LF <= ch <= CR. NB: ch must be unsigned;
// TCHAR and unsigned short give wrong results!
#define IsASCIIEOP(ch)	((unsigned)((ch) - LF) <= CR - LF)
#define IN_RANGE(n1, b, n2)		((unsigned)((b) - n1) <= n2 - n1)

BOOL	IsEOP(unsigned ch);

#include "tom.h"

#include "zmouse.h"
#include "stddef.h"
#include "_util.h"

#ifdef DEBUG
#define EM_DBGPED (WM_USER + 150)
#endif

#ifdef abs
#undef abs
#endif
#define abs(_l) (((LONG) _l) < 0 ? -(_l) : (_l))
#define ABS(_x) ((_x) >= 0 ? (_x) : -(_x))

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

#include "_cfpf.h"

// Interesting OS versions
#define VERS4		4

// conversions between byte and character counts

// Unicode is alwys defined
#define CbOfCch(_x) ((_x) * 2)
#define CchOfCb(_x) ((_x) / 2)

#define chSysMost ((TCHAR) -1)

#define OLEstrcmp	wcscmp
#define OLEstrcpy wcscpy
#define OLEsprintf wsprintf
#define OLEstrlen wcslen

// index (window long) of the PED
#define ibPed 0

#define RETID_BGND_RECALC	0x01af
#define RETID_AUTOSCROLL	0x01b0
#define RETID_SMOOTHSCROLL	0x01b1
#define RETID_DRAGDROP		0x01b2
#define RETID_MAGELLANTRACK	0x01b3

// count of characters in CRLF marker
#define cchCRLF 2
#define cchCR	1

// RichEdit 1.0 uses a CRLF for an EOD marker
#define	CCH_EOD_10			2
// RichEdit 2.0 uses a simple CR for the EOD marker
#define CCH_EOD_20			1

extern TCHAR szCRLF[];
extern TCHAR szCR[];

extern const CHARFORMAT cfBullet;
extern const TCHAR chBullet;

extern HINSTANCE hinstRE;		// DLL instance

// ARULM: Globalize: This flag tracks if we have J support, i.e. locale 0411 is installed
extern DWORD g_fHasJapanSupport;

#ifdef PENWIN20
extern HINSTANCE hinstPenWin; // handle of penwin
extern BOOL (WINAPI *pfnCorrectWriting)(HWND, LPSTR, UINT, LPRC, DWORD, DWORD);
extern BOOL (WINAPI *pfnTPtoDP)(LPPOINT, INT);
extern int (WINAPI *pfnGetHotspotsHRCRESULT)(HRCRESULT, UINT, LPPOINT, UINT);
extern int (WINAPI *pfnDestroyHRCRESULT)(HRCRESULT);
extern int (WINAPI *pfnGetResultsHRC)(HRC, UINT, LPHRCRESULT, UINT);
extern int (WINAPI *pfnGetSymbolCountHRCRESULT)(HRCRESULT);
extern int (WINAPI *pfnGetSymbolsHRCRESULT)(HRCRESULT, UINT, LPSYV, UINT);
extern BOOL	(WINAPI *pfnSymbolToCharacter)(LPSYV, int, LPSTR, LPINT);
extern HRC (WINAPI *pfnCreateCompatibleHRC)(HRC, HREC);
extern int (WINAPI *pfnDestroyHRC)(HRC);
#endif // PENWIN20

#define	BF	UINT

#include <shellapi.h>

#ifndef MACPORT
  #ifdef DUAL_FORMATETC
  #undef DUAL_FORMATETC
  #endif
  #define DUAL_FORMATETC FORMATETC
#endif

#include "WIN2MAC.h"

extern "C"
{
	LRESULT CALLBACK RichEditWndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK RichEditANSIWndProc(HWND, UINT, WPARAM, LPARAM);
}

// Multi-Threading support
extern CRITICAL_SECTION g_CriticalSection;

// a class to simplify critical section management
class CLock 
{
public:
	CLock()		{EnterCriticalSection(&g_CriticalSection);}
	~CLock()	{LeaveCriticalSection(&g_CriticalSection);}
};

enum HITTEST
{
	HT_Undefined = 0,	// Hit hasn't been determined
	HT_Nothing,
	HT_SelectionBar,
	HT_OutlineSymbol,
	HT_LeftOfText,
	HT_BulletArea,
	HT_RightOfText,

	HT_Text,			// All hits are in text from HT_Text on so
	HT_Link,			//  if(hit >= HT_Text) identifies text of some kind
	HT_Italic,
	HT_Object
};

#ifdef DEBUG
//Debug api for dumping CTxtStory arrays.
extern "C" {
extern void DumpDoc(void *);
}
#endif

// Definition for switching between inline and non-inline based on whether
// the build is for DEBUG. This allows many inline routines to be debugged
// more easily. However, care must be used because inline is an easy way
// to create code bloat.
#ifdef DEBUG
#define INLINE
#else
#define INLINE inline
#endif // DEBUG

#endif


