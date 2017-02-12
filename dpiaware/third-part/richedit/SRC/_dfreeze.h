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
 *	@module _DFREEZE.H  Classes handle freezing the display |
 *	
 *	This module declares class used by logic to handle freezing the display
 *
 *	History: <nl>
 *		2/8/96	ricksa	Created
 */

#ifndef _DFREEZE_H
#define _DFREEZE_H

/*
 *	CAccumDisplayChanges
 *	
 * 	@class	This class is used to accumulate of all update to the display so
 *			so that at a later time the display can ask to be updated to 
 *			reflect all the previous updates.
 *
 *
 */
class CAccumDisplayChanges
{
//@access Public Methods
public:
						CAccumDisplayChanges();		//@cmember Constructor

						~CAccumDisplayChanges();	//@cmember Destructor

	void				AddRef();					//@cmember Add a reference

	LONG				Release();					//@cmember Release a reference

	void				UpdateRecalcRegion(			//@cmember Update the region 
							DWORD cp,				// for recalc
							DWORD cchDel,
							DWORD cchNew);

	void				GetUpdateRegion(			//@cmember Get the update 
							DWORD *pcpStart,		// region
							DWORD *pcchNew,
							DWORD *pcchDel,
							BOOL *pfUpdateCaret,
							BOOL *pfScrollIntoView);

	void				SaveUpdateCaret(			//@cmember Save update
							BOOL fScrollIntoView);	// caret state

//@access Private Data
private:

	LONG				_cRefs;						//@cmember Reference count

	DWORD				_cpMin;						//@cmember the min cp of the 
													// change w.r.t. the orignal 
													// text array

	DWORD				_cpMax;						//@cmember the max cp of the 
													// change w.r.t. the orignal 
													// text array

	LONG				_delta;						//@cmember the net # of chars 
													// changed

	BOOL				_fUpdateCaret;				//@cmember Whether update
													// caret required.

	BOOL				_fScrollIntoView;			//@cmember first parm to 
};

/*
 *	CAccumDisplayChanges::CAccumDisplayChanges()
 *
 *	@mfunc
 *		Initialize object for accumulating display changes
 *
 *	@rdesc
 *		void
 *
 */
inline CAccumDisplayChanges::CAccumDisplayChanges() 
	: _cRefs(1), _cpMin(INFINITE), _fUpdateCaret(FALSE)
{
	// Header does all the work
}

/*
 *	CAccumDisplayChanges::~CAccumDisplayChanges()
 *
 *	@mfunc
 *		Free object
 *
 *	@rdesc
 *		void
 *
 *	@devnote:
 *		This only serves a purpose in debug mode
 *
 */
inline CAccumDisplayChanges::~CAccumDisplayChanges()
{
	// Nothing to clean up
}

/*
 *	CAccumDisplayChanges::~CAccumDisplayChanges()
 *
 *	@mfunc
 *		Add another reference to this object
 *
 *	@rdesc
 *		void
 *
 */
inline void CAccumDisplayChanges::AddRef()
{
	++_cRefs;
}

/*
 *	CAccumDisplayChanges::Release()
 *
 *	@mfunc
 *		Release a reference to this object
 *
 *	@rdesc
 *		0 - no more references
 *		~0 - there are still outstanding references
 *
 *	@devnote:
 *		If 0 is returned the information should be retrieved from
 *		this object and passed on to the display so that it can
 *		update itself.
 *
 */
inline LONG CAccumDisplayChanges::Release()
{
	// When the reference count is 0, it is time to update the display.
	return --_cRefs;	
}

/*
 *	CAccumDisplayChanges::SaveUpdateCaret()
 *
 *	@mfunc
 *		Save parameters for update caret
 *
 *	@rdesc
 *		None.
 *
 */
inline void CAccumDisplayChanges::SaveUpdateCaret(
	BOOL fScrollIntoView)		//@parm First parm for UpdateCaret
{
	_fUpdateCaret = TRUE;

	if (!_fScrollIntoView)
	{
		_fScrollIntoView = fScrollIntoView;
	}
}

#endif // _DFREEZE_H