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
 *
 *	REINIT.C
 *	
 *	Purpose:
 *		RICHEDIT initialization routines
 *	
 */

#include "_common.h"
#include "_font.h"
#include "_format.h"
#include "_disp.h"
#include "_clasfyc.h"
#include "zmouse.h"
#include "_rtfconv.h"

ASSERTDATA

class CTxtEdit;

extern void ReleaseTypeInfoPtrs();

#pragma BEGIN_CODESPACE_DATA
static char szClassREA[sizeof(RICHEDIT_CLASSA)];
static WCHAR wszClassREW[sizeof(RICHEDIT_CLASSW)/sizeof(WCHAR)];

// a critical section for multi-threading support.
CRITICAL_SECTION g_CriticalSection;
#pragma END_CODESPACE_DATA

HINSTANCE hinstRE = 0;

LOCAL BOOL RichFRegisterClass(VOID);
void RegisterFETCs();

#ifdef DEBUG
BOOL fInDllMain = FALSE;  // used to ensure that GDI calls are not made during
						  // DLL_PROCESS_ATTACH
#endif

UINT gWM_MouseRoller;					// RegisterWindowMessage for Magellan mouse.

// These are defined in font.cpp
BOOL InitFontCache();
void FreeFontCache();

// ARULM: Globalize: This flag tracks if we have J support, i.e. locale 0411 is installed
DWORD		g_fHasJapanSupport;

extern "C"
{

// these are defiend in CRT0DAT.C (part of CORELIBC.LIB)
void __cdecl _cinit(void);
void __cdecl _cexit(void);


#ifdef PEGASUS
BOOL WINAPI NewDllMain(HANDLE hmod, DWORD dwReason, LPVOID lpvReserved)
#else
BOOL WINAPI NewDllMain(HMODULE hmod, DWORD dwReason, LPVOID lpvReserved)
#endif
{
       WCHAR szcp[5];
   
	if(dwReason == DLL_PROCESS_DETACH)		// We are unloading
	{
		CRTFConverter::FreeFontSubInfo();
		FreeFontCache();
		DestroyFormatCaches();
		ReleaseTypeInfoPtrs();
		UninitKinsokuClassify();
		if(hinstRE)
		{
			#ifndef PEGASUS
				UnregisterClassA(szClassREA, hinstRE);
				#ifdef RICHED32_BUILD
					UnregisterClassA(szClassRE10A, hinstRE);
				#endif
			#endif
			W32->UnregisterClass(wszClassREW, hinstRE);
		}
		delete W32;
		DeleteCriticalSection(&g_CriticalSection);
	}
	else if(dwReason == DLL_PROCESS_ATTACH) // We have just loaded
	{
		#ifdef DEBUG
			fInDllMain = TRUE;
		#endif
		InitializeCriticalSection(&g_CriticalSection);

		// ARULM: Globalize: This flag tracks if we have J support, i.e. locale 0411 is installed
		g_fHasJapanSupport = (0 != GetLocaleInfo(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, (LPWSTR)szcp, sizeof(szcp)/sizeof(WCHAR)));

		RegisterFETCs();					// Register new clipboard formats
		CreateFormatCaches();				// Create global format caches
		if ( !InitKinsokuClassify() )
		{
			// Init tables for classifying Unicode chars.
			return FALSE;
		}

		#ifdef PEGASUS
			hinstRE = (HINSTANCE) hmod;
		#else
			hinstRE = hmod;
		#endif

		W32 = new CW32System;

		// GuyBark Jupiter: Take same error action as above if necessary.
		if(!W32)
		{
			return FALSE;
		}

		WCHAR wszFileName[_MAX_PATH];
		CopyMemory(szClassREA, RICHEDIT_CLASSA, sizeof(CERICHEDIT_CLASSA));
		CopyMemory(wszClassREW, RICHEDIT_CLASSW, sizeof(CERICHEDIT_CLASSW));
		int iLen = W32->GetModuleFileName((HMODULE) hmod, wszFileName, _MAX_PATH);
		if (iLen)
		{
			iLen -= sizeof("riched20.dll") - 1;
			if (0 != lstrcmpi(&wszFileName[iLen] , TEXT("riched20.dll")))
			{
				// This code allows the dll to be renamed for Win CE.
				Assert(sizeof(RICHEDIT_CLASSA) == sizeof(CERICHEDIT_CLASSA));
				Assert(sizeof(RICHEDIT_CLASSW) == sizeof(CERICHEDIT_CLASSW));
				CopyMemory(szClassREA, CERICHEDIT_CLASSA, sizeof(CERICHEDIT_CLASSA));
				CopyMemory(wszClassREW, CERICHEDIT_CLASSW, sizeof(CERICHEDIT_CLASSW));
			}
		}

		if(!RichFRegisterClass())
		{
			// return FALSE;
		}

		// GuyBark Jupiter: Take same error action as above if necessary.
		if(!InitFontCache())
		{
			return FALSE;
		}

		#ifdef DEBUG
			fInDllMain = FALSE;
		#endif
	}

	return TRUE;
}

#ifdef PEGASUS
BOOL WINAPI DllMain(HANDLE hmod, DWORD dwReason, LPVOID lpvReserved)
#else
BOOL WINAPI DllMain(HMODULE hmod, DWORD dwReason, LPVOID lpvReserved)
#endif
{
     BOOL retcode;
     if (dwReason == DLL_PROCESS_ATTACH)
     {
          // ARULM: initialize static constructors (since we bypass DllMainCRTStartup)
         _cinit();
     }

     retcode = NewDllMain(hmod, dwReason, lpvReserved);

     if (dwReason == DLL_PROCESS_DETACH)
     {
          // ARULM: call static destructors (since we bypass DllMainCRTStartup)
         _cexit();
     }
     return retcode;

}
} 	// extern "C"

/*
 *	RichFRegisterClass
 *
 *	Purpose:	
 *		registers the window classes used by richedit
 *
 *	Algorithm:
 *		register two window classes, a Unicode one and an ANSI
 *		one.  This enables clients to optimize their use of 
 *		the edit control w.r.t to ANSI/Unicode data 
 */

LOCAL BOOL RichFRegisterClass(VOID)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichFRegisterClass");
	WNDCLASS wc;

	wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_PARENTDC;
	wc.lpfnWndProc = RichEditWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(CTxtEdit FAR *);
	wc.hInstance = hinstRE;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wszClassREW;

	if( W32->RegisterREClass(&wc, szClassREA, RichEditANSIWndProc) == NULL )
	{
		return FALSE;
	};

	return TRUE;
}


#ifdef PENWIN20

#pragma BEGIN_CODESPACE_DATA

static const char szPenWinDLL[] = "PENWIN32.DLL";
static const char szCorrectWriting[] = "CorrectWriting";
static const char szTPtoDP[] = "TPtoDP";
static const char szGetHotspotsHRCRESULT[] = "GetHotspotsHRCRESULT";
static const char szDestroyHRCRESULT[] = "DestroyHRCRESULT";
static const char szGetResultsHRC[] = "GetResultsHRC";
static const char szGetSymbolCountHRCRESULT[] = "GetSymbolCountHRCRESULT";
static const char szGetSymbolsHRCRESULT[] = "GetSymbolsHRCRESULT";
static const char szSymbolToCharacter[] = "SymbolToCharacter";
static const char szCreateCompatibleHRC[] = "CreateCompatibleHRC";
static const char szDestroyHRC[] = "DestroyHRC";

#pragma END_CODESPACE_DATA

LOCAL BOOL FCheckPenWin()
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "FCheckPenWin");

	// check if penwin is installed on this system
	// if not, no pen functionality.
	if (!GetSystemMetrics(SM_PENWINDOWS))
		return FALSE;

	// try to load penwin32.dll and get proc addresses. 
	if((hinstPenWin = W32->LoadLibrary(szPenWinDLL)) > (HINSTANCE)HINSTANCE_ERROR)
	{
		(FARPROC)pfnCorrectWriting = GetProcAddress(hinstPenWin, szCorrectWriting);
		(FARPROC)pfnTPtoDP = GetProcAddress(hinstPenWin, szTPtoDP);
		(FARPROC)pfnGetHotspotsHRCRESULT = GetProcAddress(hinstPenWin, szGetHotspotsHRCRESULT);
		(FARPROC)pfnDestroyHRCRESULT = GetProcAddress(hinstPenWin, szDestroyHRCRESULT);
		(FARPROC)pfnGetResultsHRC = GetProcAddress(hinstPenWin, szGetResultsHRC);
		(FARPROC)pfnGetSymbolCountHRCRESULT = GetProcAddress(hinstPenWin, szGetSymbolCountHRCRESULT);
		(FARPROC)pfnGetSymbolsHRCRESULT = GetProcAddress(hinstPenWin, szGetSymbolsHRCRESULT);
		(FARPROC)pfnSymbolToCharacter = GetProcAddress(hinstPenWin, szSymbolToCharacter);
		(FARPROC)pfnCreateCompatibleHRC = GetProcAddress(hinstPenWin, szCreateCompatibleHRC);
		(FARPROC)pfnDestroyHRC = GetProcAddress(hinstPenWin, szDestroyHRC);
		AssertSz(pfnGetHotspotsHRCRESULT &&
					pfnDestroyHRCRESULT &&
					pfnGetResultsHRC &&
					pfnGetSymbolCountHRCRESULT &&
					pfnGetSymbolsHRCRESULT &&
					pfnSymbolToCharacter &&
					pfnCreateCompatibleHRC &&
					pfnDestroyHRC,
					"Failed to load PENWIN32 functions");
		return TRUE;
	}

	hinstPenWin = NULL;
	return FALSE;
}
#endif // PENWIN20



