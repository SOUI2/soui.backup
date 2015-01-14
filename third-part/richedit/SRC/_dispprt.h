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
 *  _DISPPRT.H
 *  
 *  Purpose:
 *      CDisplayPrinter class. Multi-line display for printing.
 *  
 *  Authors:
 *      Original RichEdit code: David R. Fulmer
 *      Christian Fortini
 *      Jon Matousek
 */

#ifndef _DISPPRT_H
#define _DISPPRT_H

#include "_dispml.h"


class CDisplayPrinter : public CDisplayML
{
public:
					CDisplayPrinter (
						CTxtEdit* ped, 
						HDC hdc, 
						LONG x, 
						LONG y, 
						SPrintControl prtcon);

    virtual BOOL    IsMain() const { return FALSE; }

	inline RECT 	GetPrintView( void ) { return rcPrintView; }
	inline void 	SetPrintView( const RECT & rc ) { rcPrintView = rc; }

	inline RECT		GetPrintPage(void) { return _rcPrintPage;}
	inline void		SetPrintPage(const RECT &rc) {_rcPrintPage = rc;}

    // Format range support
    LONG    		FormatRange(LONG cpFirst, LONG cpMost);

	// Natural size calculation
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight);

	virtual BOOL	IsPrinter() const;

	void			SetPrintDimensions(RECT *prc);


protected:

	RECT			rcPrintView;	// for supporting client driven printer banding.
	RECT			_rcPrintPage;	// the entire page size

	SPrintControl	_prtcon;		// Control print behavior
};
#endif

