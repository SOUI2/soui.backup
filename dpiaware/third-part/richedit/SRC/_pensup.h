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
 *	@module	_PENSUP.H	Pen Support Routines	|
 *
 *	This file contains support functions for Win95 Pen Services 2.0
 *
 *	Author:	3/17/96	alexgo
 */
#ifndef _PENSUP_H
#define	_PENSUP_H

// wrappers for PenWindows functions
BOOL REIsPenEvent( UINT msg, LONG lExtraInfo );
int REDoDefaultPenInput( HWND hwnd, UINT wEventRef );

#endif


