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
 *	@doc	INTERNAL
 *
 *	@module	_PENSUP.CPP	Pen Support Routines	|
 *
 *	This file contains support functions for Win95 Pen Services 2.0
 *
 *	Author:	3/17/96	alexgo
 */
#include "_common.h"
#include "_pensup.h"

static HMODULE	hPenDll;

// function pointers
static BOOL (*pfIsPenEvent)(UINT msg, LONG lExtraInfo);
static int (*pfDoDefaultPenInput)(HWND hwnd, UINT wEventRef);

/*
 *	@func	InitPenServices | initializes PenWindows 2.0
 *
 *	@devnote	This function may be called from multiple-threads.
 *			If we happen to have a race (very unlikely, since
 *			we watch mouse button downs), one app may not do pen
 *			processing for the first mouse down.  That's OK.
 */
void InitPenServices()
{
	static BOOL	fAlreadyTried = FALSE;

	if( fAlreadyTried == FALSE )
	{		
		fAlreadyTried = TRUE;

		hPenDll = (HMODULE)GetSystemMetrics(SM_PENWINDOWS);

		if( hPenDll )
		{
			pfIsPenEvent = (BOOL (*)(UINT, LONG))GetProcAddress(hPenDll, 
							"IsPenEvent");

			if(pfIsPenEvent)
			{
				pfDoDefaultPenInput = (int (*)(HWND, UINT))
							GetProcAddress(hPenDll, "DoDefaultPenInput");

				if( !pfDoDefaultPenInput )
				{
					pfIsPenEvent = NULL;
				}
			}
		}

	}
}
		
/*
 *	@func	REIsPenEvent	| indicates if the event is a
 *			a pen event
 */
BOOL REIsPenEvent( UINT msg, LONG lExtraInfo )
{
	InitPenServices();

	if( pfIsPenEvent )
	{
		return (*pfIsPenEvent)(msg, lExtraInfo);
	}
	return FALSE;
}

/*
 *	@func	REDoDefaultPenInput | starts up a pen edit session
 */
int REDoDefaultPenInput( HWND hwnd, UINT wEventRef )
{
	Assert(pfDoDefaultPenInput);

	return (*pfDoDefaultPenInput)(hwnd, wEventRef);
}


