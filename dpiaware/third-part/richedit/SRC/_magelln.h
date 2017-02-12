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
 *	@module _MAGELLN.H -- Declaration of class to handle Magellan mouse. |
 *	
 *	Authors: <nl>
 *		Jon Matousek 
 *
 */

#if !defined(_MAGELLN_H) && !defined(NOMAGELLAN)	// Win NT/95 only class.
#define _MAGELLN_H

#include "_edit.h"


// All of the knobs for magellan mouse scrolling.

const LONG	DEAD_ZONE_TWIPS			= 60;	// 3 pixels normally
const DWORD FAST_ROLL_SCROLL_TRANSITION_TICKS = 900;  // in mili seconds.
const INT FASTER_ROLL1_COUNT		= 5;
const INT FASTER_ROLL2_COUNT		= 10;

const WORD SMOOTH_ROLL_CLINES		= 2;	// multiples of rolls for roll1, roll2.
const int SMOOTH_ROLL_NUM			= 1;
const int SMOOTH_ROLL_DENOM			= 3;

class CMagellan {

	friend class CMagellanBMPStateWrap;

private:
	VOID CheckInstallMagellanTrackTimer (CTxtEdit &ed);
	VOID CheckRemoveMagellanUpdaterTimer (CTxtEdit &ed);
	BOOL InvertMagellanDownBMP ( CDisplay *pdp, BOOL fTurnOn, HDC repaintDC );

	WORD		_fMagellanBitMapOn	:1;	// TRUE if the MDOWN bitmap is displayed.
	WORD		_fMButtonScroll		:1;	// Auto scrolling initiated via magellan-mouse.
	WORD		_fLastScrollWasRoll	:1;	// scroll will be wither roll or mdown.

 	SHORT		_ID_currMDownBMP;		// Resource ID of _MagellanMDownBMP.
	HBITMAP		_MagellanMDownBMP;		// Loaded BMP
	POINT		_zMouseScrollStartPt;	// Magellan mouse's start scroll pt.



public:

	BOOL MagellanStartMButtonScroll ( CTxtEdit &ed, POINT mDownPt );
	VOID MagellanEndMButtonScroll ( CTxtEdit &ed );
	VOID MagellanRollScroll ( CDisplay *pdp, int direction, WORD cLines, int speedNum, int speedDenom, BOOL fAdditive );
	VOID TrackUpdateMagellanMButtonDown ( CTxtEdit &ed, POINT mousePt);

	~CMagellan() { Assert( !_MagellanMDownBMP && !_fMButtonScroll /* client state problems? */); }

};

class CMagellanBMPStateWrap {
private:
	BOOL _fMagellanState;
	HDC _repaintDC;
	CTxtEdit &_ed;
public:
	CMagellanBMPStateWrap(CTxtEdit &ed, HDC repaintDC);
	~CMagellanBMPStateWrap();
};

#endif // _MAGELLN_H

