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
 *	_DEVSTK.H
 *	
 *	Purpose:
 *		CDevState - handle access to device descriptor
 *	
 *	Authors:
 *		Rick Sailor
 */

#ifndef _DEVSTK_H_
#define _DEVSTK_H_


class CTxtEdit;
class CDrawInfo;

// device descriptor
class CDevState
{
public:
						CDevState(CTxtEdit * ped);

						~CDevState();

    					BOOL    IsValid() const;

						BOOL 	IsMetafile() const;

						BOOL	SetDrawInfo(
									DWORD dwDrawAspect,
									LONG lindex,
									const DVTARGETDEVICE *_ptd,
									HDC hdcDraw,
									HDC hicTargetDev);

						BOOL	SetDC(HDC hdc);

	void 				ResetDrawInfo();

	HDC					GetTargetDD();

	HDC	 				GetRenderDD();

	void				ReleaseDC();

	BOOL				SameDrawAndTargetDevice();

	LONG				ConvertXToTarget(LONG xPixels);

	LONG				ConvertXToDraw(LONG xPixels);

	LONG				ConvertYToDraw(LONG yPixels);

protected:

	CTxtEdit * 			_ped;        // used to GetDC and ReleaseDC

	CDrawInfo *			_pdd;

	HDC					_hicMainTarget;
	
};

#ifndef DEBUG
#include	<_devstki.h>
#endif // DEBUG


#endif // _DEVSTK_H_

