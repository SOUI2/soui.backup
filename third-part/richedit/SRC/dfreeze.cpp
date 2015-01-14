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
 *	@module _DFREEZE.CPP  Implementation for classes handle freezing the display |
 *	
 *	This module implements non-inline members used by logic to handle freezing the display
 *
 *	History: <nl>
 *		2/8/96	ricksa	Created
 */
#include	"_common.h"
#include	"_disp.h"
#include	"_dfreeze.h"

ASSERTDATA

/*
 *	CAccumDisplayChanges::GetUpdateRegion()
 *
 *	@mfunc
 *		Get region for display to update
 *
 *	@rdesc
 *		void
 *
 */
void CAccumDisplayChanges::GetUpdateRegion(
	DWORD *pcpStart,			//@parm where to put the cpStart
	DWORD *pcchDel,				//@parm where to put the del char count
	DWORD *pcchNew,				//@parm where to put the new char count
	BOOL *pfUpdateCaret,		//@parm whether caret update is needed
	BOOL *pfScrollIntoView)		//@parm whether to scroll caret into view
{
	DWORD cchDel;
	*pcpStart = _cpMin;

	if( pfUpdateCaret )
	{
		*pfUpdateCaret = _fUpdateCaret;
	}

	if( pfScrollIntoView )
	{
		*pfScrollIntoView = _fScrollIntoView;
	}

	if( _cpMin == INFINITE )
	{
		return;
	}

	cchDel = _cpMax - _cpMin;

	if( pcchDel )
	{
		*pcchDel =  cchDel;
	}

	*pcchNew = cchDel + _delta;

	_cpMin = INFINITE;
}


/*
 *	CAccumDisplayChanges::UpdateRecalcRegion()
 *
 *	@mfunc
 *		Merge new update with region to be recalculated
 *
 *	@rdesc
 *		void
 *
 */
void CAccumDisplayChanges::UpdateRecalcRegion(
	DWORD cpStartNew,	//@parm start of update
	DWORD cchDel,		//@parm number of chars to delete
	DWORD cchNew)		//@parm number of chars to add
{
	if (INFINITE == _cpMin)
	{
		// Object is empty so just assign values
		_cpMin = cpStartNew;
		_cpMax = cpStartNew + cchDel;
		_delta = cchNew - cchDel;
		return;
	}

	// The basic idea of this algorithm is to merge the updates so that
	// they appear to the display sub-system as if only one replace range
	// has occured. To do this we keep track of the start of the update 
	// (_cpMin) relative to the original text and the end of the update 
	// (_cpMax) relative to the original text and the change  in the count 
	// of text (_delta). We can recreate cchDel from _cpMost - _cpMin and 
	// cchNew from cchDel + _delta.

	// Do we need to update _cpMin? - we only need to update _cpMin if the
	// current update begins before the last update because the final update
	// need only know the very start of the range updated.
	if (cpStartNew < _cpMin)
	{
		_cpMin = cpStartNew;
	}

	// Do we need to udpate _cpMax? - we only need to update _cpMax if the
	// current update implies a _cpMax that is greater than the current one.
	// Note that because prior updates affect where the _cpMax is located
	// we need to compare againt the proposed _cpMax against the current
	// _cpMax adjusted by the change in the text since the beginning of the
	// updates.
	if (cpStartNew + cchDel > _cpMax + _delta)
	{
		_cpMax = cpStartNew + cchDel - _delta;
	}

	// Increment the total change by the change for this update.
	_delta += (cchNew - cchDel);
}



