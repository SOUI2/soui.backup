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
/*	@doc INTERNAL
 *
 *	@module _OSDC.H  Off Screen DC class |
 *	
 *	Define class for off screen DC
 *
 *	Original Author: <nl>
 *		Ricksa
 *
 *	History: <nl>
 *		1/11/96	ricksa	Created
 */
#ifndef __OSDC_H__
#define __OSDC_H__

/*
 *	COffScreenDC
 *	
 * 	@class	The COffScreenDC is a helper that creates, fills and destroys
 *			an off screen DC and its bitmaps.
 *
 */
class COffScreenDC
{
//@access Public Methods
public:
									//@cmember Constructor - create null object.
						COffScreenDC();

									//@cmember Destructor - clean up allocated 
									// resources if any.
						~COffScreenDC();	

									//@cmember Initialize data based on input DC
	HDC					Init(				
							HDC hdc,
							LONG xWidth,
							LONG yHeight,
							COLORREF crBackground);

									//@cmember get the DC for offscreen rendering
	HDC					GetDC();
									//@cmember fill bitmap associated with off
									// screen rendering with background color.
	void				FillBitmap(LONG xWidth, LONG yHeight);

									//@cmember render off screen bit map to
									// another DC.
	void				RenderBitMap(
							HDC hdc,
							LONG xLeft,
							LONG yTop,
							LONG xWidth,
							LONG yHeight);

									//@cmember Get a copy of a bitmap from 
									// another DC
	BOOL				Get(
							HDC hdc,
							LONG xLeft,
							LONG yTop,
							LONG xWidth,
							LONG yHeight);

									//@cmember Reallocate bitmap for render
	BOOL				Realloc(
							LONG xWidth,
							LONG yHeight);

									//@cmember Select in a palette
	void				SelectPalette(
							HPALETTE hpa);

//@access Private Methods
private:
									//@cmember free all data associated with
									// the object.
	void				FreeData();

//@access Private Data 
private:

	HDC					_hdc;		//@cmember HDC for off screen DC

	HBITMAP				_hbmpOld;	//@cmember bitmap when DC created

	HBITMAP				_hbmp;		//@cmember compatible bit map for render

	HPALETTE			_hpalOld;	//@cmember palette used by DC
};


/*
 *	COffScreenDC::COffScreenDC
 *
 *	@mfunc	
 *		Set HDC to NULL to signal this object is not initialized
 *
 *	@rdesc
 *		None.
 *
 */
inline COffScreenDC::COffScreenDC() : _hdc(NULL)
{
	// Header does all the work
}

/*
 *	COffScreenDC::~COffScreenDC
 *
 *	@mfunc	
 *		Free resources associated with object
 *
 *	@rdesc
 *		None.
 *
 */
inline COffScreenDC::~COffScreenDC()
{
	FreeData();
}

/*
 *	COffScreenDC::GetDC
 *
 *	@mfunc	
 *		Get the DC associated with the object
 *
 *	@rdesc
 *		DC associated with object
 *
 */
inline HDC COffScreenDC::GetDC()
{
	return _hdc;
}

/*
 *	COffScreenDC::FillBitmap
 *
 *	@mfunc	
 *		Get the DC associated with the object
 *
 *	@rdesc
 *		DC associated with object
 *
 */
inline void COffScreenDC::FillBitmap(
	LONG xWidth,		//@parm Width to fill with background color
	LONG yHeight)		//@parm height to fill with background color
{
	// Erase the background
	RECT rcClient;
	rcClient.top = rcClient.left = 0;
	rcClient.right = xWidth;
	rcClient.bottom = yHeight;
	ExtTextOut(_hdc, 0, 0, ETO_OPAQUE, &rcClient, NULL, 0, NULL);
}

/*
 *	COffScreenDC::RenderBitMap
 *
 *	@mfunc	
 *		Render bit map to input DC
 *
 *	@rdesc
 *		None.
 *
 */
inline void COffScreenDC::RenderBitMap(
	HDC hdc,			//@parm HDC to render to
	LONG xLeft,			//@parm left position to start render
	LONG yTop,			//@parm top top position to start render
	LONG xWidth,		//@parm width to render
	LONG yHeight)		//@parm height to render
{
	BitBlt(hdc, xLeft, yTop, xWidth, yHeight, _hdc, 0, 0, SRCCOPY);
}
	

/*
 *	COffScreenDC::Get
 *
 *	@mfunc	
 *		Get a copy of a bitmap from another DC
 *
 *	@rdesc
 *		TRUE - succeeded
 *		FALSE - Failed
 *
 */
inline BOOL COffScreenDC::Get(
	HDC hdc,
	LONG xLeft,
	LONG yTop,
	LONG xWidth,
	LONG yHeight)
{
	return BitBlt(_hdc, 0, 0, xWidth, yHeight, hdc, xLeft, yTop, SRCCOPY);
}

#endif __OSDC_H__