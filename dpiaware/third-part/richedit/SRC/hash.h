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

#ifndef _HASH_H
#define _HASH_H

#include "tokens.h"

#define		HASHSIZE 	257	// DO NOT CHANGE!
						// this prime has been chosen because
						// there is a fast MOD257
						// if you use the % operator the thing
						// slows down to just about what a binary search is.

#define			MOD257(k) ((k) - ((k) & ~255) - ((k) >> 8) )	// MOD 257
#define			MOD257_1(k) ((k) & 255)	// MOD (257 - 1)

extern BOOL		_rtfHashInited;
VOID			HashKeyword_Init( );

VOID			HashKeyword_Insert ( const KEYWORD *token );
const KEYWORD	*HashKeyword_Fetch ( const CHAR *szKeyword );

#endif	// _HASH_H

