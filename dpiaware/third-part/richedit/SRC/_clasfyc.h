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
 *	@doc
 *
 *	@module _clasfyc.H -- chracter classification |
 *	
 *	Authors: <nl>
 *		Jon Matousek 
 */

#ifndef _CLASFYC_H
#define _CLASFYC_H

BOOL InitKinsokuClassify();
VOID UninitKinsokuClassify();
VOID BatchKinsokuClassify ( const WCHAR *ch, INT cch, WORD *cType3, INT *kinsokuClassifications );
BOOL CanBreak( INT class1, INT class2 );
BOOL IsURLDelimiter(WCHAR ch);

#define MAX_CLASSIFY_CHARS (256L)

// Korean Unicode range
#define IsKorean(ch) ( (unsigned)((ch) - 0x0AC00) <= (0x0D7FF - 0x0AC00) )

#endif


