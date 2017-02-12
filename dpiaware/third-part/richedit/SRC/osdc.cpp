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
 *	@doc INTERNAL
 *
 *	@module	OSDC.CP -- Off Screen DC class |
 *
 *		This contains method used to implement the off screen
 *		DC class
 *	
 *	Owner:<nl>
 *		Rick Sailor
 */
#include	"_common.h"
#include	"_osdc.h"

ASSERTDATA

/*
 *	COffScreenDC::Init
 *
 *	@mfunc	
 *		Initialize off screen DC with compatible bit map
 *
 *	@rdesc
 *		DC created
 *
 */
HDC	COffScreenDC::Init(
	HDC hdc,				//@parm DC to be compatible with
	LONG xWidth,			//@parm width of compatible bit map
	LONG yHeight,			//@parm height of compatible bit map
	COLORREF crBackground)	//@parm default bacground for bit map
{
	// HDC to return to caller
	HDC hdcRet = NULL;

	// Assume failure
	_hbmpOld = NULL;
	_hbmp = NULL;
	_hpalOld = NULL;

	// Create a memory DC
	_hdc = CreateCompatibleDC(hdc);

	if (NULL == _hdc)
	{
		goto Exit;
	}

	// Create the bit map based on the size of the client rectangle
	_hbmp = CreateCompatibleBitmap(hdc, xWidth, yHeight);

	if (NULL == _hbmp)
	{
		goto Exit;
	}

	// Select the bitmap into the hdc
	_hbmpOld = (HBITMAP) SelectObject(_hdc, _hbmp);

	if (NULL == _hbmpOld)
	{
		goto Exit;
	}

	if (SetBkMode(_hdc, TRANSPARENT) == 0)
	{
		goto Exit;
	}

	if (SetBkColor(_hdc, crBackground) != CLR_INVALID)
	{
		hdcRet = _hdc;
	}



Exit:

	if (NULL == hdcRet)
	{
		FreeData();
	}

	return hdcRet;
}

/*
 *	COffScreenDC::SelectPalette
 *
 *	@mfunc	
 *		Set a new palette into the hdc
 *
 *	@rdesc
 *		None.
 *
 */
void COffScreenDC::SelectPalette(
		HPALETTE hpal)			//@parm Handle to the palette to set
{
#ifndef PEGASUS
	if (hpal)
	{
		_hpalOld = ::SelectPalette(_hdc, hpal, TRUE);
		RealizePalette(_hdc);
	}
#endif
}

/*
 *	COffScreenDC::FreeData
 *
 *	@mfunc	
 *		Free resources associated with the bit map
 *
 *	@rdesc
 *		None.
 *
 */
void COffScreenDC::FreeData()
{
	if (_hdc != NULL)
	{
#ifndef PEGASUS
		if (_hpalOld != NULL)
		{
			::SelectPalette(_hdc, _hpalOld, TRUE);
		}
#endif
		if (_hbmpOld != NULL)
		{
			SelectObject(_hdc, _hbmpOld);
		}

		if (_hbmp)
		{
			DeleteObject(_hbmp);
		}

		DeleteDC(_hdc);
	}
}

/*
 *	COffScreenDC::Realloc
 *
 *	@mfunc	
 *		Reallocate the bitmap
 *
 *	@rdesc
 *		TRUE - succeeded 
 *		FALSE - failed
 *
 */
BOOL COffScreenDC::Realloc(
	LONG xWidth,			//@parm Width of new bitmap
	LONG yHeight)			//@parm Height of new bitmap
{
	// Create the bit map based on the size of the client rectangle
	HBITMAP hbmpNew = CreateCompatibleBitmap(_hdc, xWidth, yHeight);

	if (NULL == hbmpNew)
	{
		AssertSz(FALSE, "COffScreenDC::Realloc CreateCompatibleBitmap failed"); 
		return FALSE;
	}

	// Select out the old bit map
#ifdef DEBUG
	HBITMAP hbmpDebug = (HBITMAP) 
#endif // DEBUG

	SelectObject(_hdc, hbmpNew);

	AssertSz(hbmpDebug == _hbmp, 
		"COffScreenDC::Realloc different bitmap"); 

	// Delete the old bitmap
#ifdef DEBUG
	BOOL fSuccess =
#endif // DEBUG

	DeleteObject(_hbmp);

	AssertSz(hbmpDebug == _hbmp, 
		"COffScreenDC::Realloc Delete old bitmap failed"); 

	// Put in the new bitmap
	_hbmp = hbmpNew;

	return TRUE;
}