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
 *	@module _RUNPTR.H -- Text run and run pointer class defintion |
 *	
 *	Original Author:	<nl>
 *		Christian Fortini
 *
 *	History: <nl>
 *		6/25/95	alexgo	Commenting and Cleanup
 */

#ifndef _RUNPTR_H
#define _RUNPTR_H

#include "_array.h"
#include "_doc.h"

typedef CArray<CTxtRun> CRunArray;

/*
 *	CRunPtrBase
 *
 *	@class	Base run pointer functionality.  Keeps a position within an array
 *  	of text runs.
 *
 *	@devnote	Run pointers go through three different possible states :
 *
 *	NULL:	there is no data and no array (frequently a startup condition) <nl>
 *			<mf CRunPtrBsae::SetRunArray> will transition from this state to 
 *			to the Empty state.  It is typically the derived class'
 *			to define when that method should be called.
 *
 *			<md CRunPtrBase::_prgRun> == NULL <nl>
 *			<md CRunPtrBase::_iRun> == 0 <nl>
 *			<md CRunPtrBase::_ich> == 0 <nl>
 *
 *	Empty:	an array class exists, but there is no data (can happen if all 
 *			of the elements in the array are deleted). <nl>
 *	 		<md CRunPtrBase::_prgRun> != NULL <nl>
 *			<md CRunPtrBase::_iRun> == 0 <nl>
 *			<md CRunPtrBase::_ich> <gt>= 0 <nl>
 *			<md CRunPtrBase::_prgRun>-<gt>Elem[0] == NULL <nl>
 *
 *	Normal:	the array class exists and has data <nl>
 *			<md CRunPtrBase::_prgRun> != NULL <nl>
 *			<md CRunPtrBase::_iRun> >= 0 <nl>
 *			<md CRunPtrBase::_ich> >= 0 <nl>
 *			<md CRunPtrBase::_prgRun>-<gt>Elem[<md CRunPtrBase::_iRun>] 
 *					!= NULL <nl>		
 *	
 *	Note that in order to support the empty and normal states, the actual 
 *	array element at <md CRunPtrBase::_iRun> must be explicitly fetched in
 *	any method that may need it.
 *
 *	Currently, there is no way to transition to the NULL state from any of
 *  the other states.  If we needed to, we could support that by explicitly 
 *	fetching the array from the document on demand.
 *
 *	Note that only <md CRunPtrBase::_iRun> is kept.  We could also keep 
 * 	a pointer to the actual run (i.e. _pRun).  Earlier versions of this
 *	engine did in fact do this.  I've opted to not do this for several
 *	reasons: <nl>
 *		1. _pRun is *always* available by calling Elem(_iRun).
 * 		Therefore, there is nominally no need to keep both _iRun and _pRun.<nl>
 *		2. Run pointers are typically used to either just move around
 *		and then fetch data or move and fetch data every time (like during 
 *		a measuring loop).  In the former case, there is no need to always
 *		bind _pRun; you can just do it on demand.  In the latter case, the
 *		two models are equivalent.  
 *
 */

class CRunPtrBase
{
	// FUTURE (alexgo): remove these friends when we get the new display engine.
	
	friend class CDisplayML;
	friend class CDisplaySL;

//@access Public methods
public:

#ifdef DEBUG
	BOOL	Invariant(void) const;			//@cmember	Invariant tests
	void	ValidatePtr(void *pRun) const;	//@cmember	Validate <p pRun>
	DWORD 	GetCch() const;					//@cmember  Get total cch in runs
#define	VALIDATE_PTR(pRun)	ValidatePtr(pRun)

#else
#define	VALIDATE_PTR(pRun)
#endif // DEBUG

	CRunPtrBase(CRunArray *prgRun);			//@cmember	Constructor
	CRunPtrBase(CRunPtrBase& rp);			//@cmember	Constructor

	//
	//@comm 	Run Control
	//
									
	void	SetRunArray(CRunArray *prgRun)	//@cmember Set run array for this
	{										// run ptr
		_prgRun = prgRun;
	}
									
	BOOL 	SetRun(LONG iRun, DWORD ich);	//@cmember Set this runptr to run
											// <p iRun> & char offset <p ich>
	BOOL	NextRun();						//@cmember Advance to next run
	BOOL	PrevRun();						//@cmember Go back to prev run
	BOOL	ChgRun(LONG cRun)				//@cmember Move <p cRun> runs
	{										// returning TRUE if successful
		return SetRun(_iRun + cRun, 0);
	}	
											//@cmember Count <p cRun> runs 
	LONG	CountRuns(LONG &cRun,			// returning cch counted and
				LONG cchMax,				// updating <p cRun>
				LONG cchText) const;

											//@cmember Find run range limits
	void	FindRun (LONG *pcpMin,
				LONG *pcpMost, LONG cpMin, LONG cch, LONG cchText) const;

	CTxtRun * GetRun(LONG cRun) const;		//@cmember Retrieve run element at 
											// offset <p cRun> from this run
	DWORD	Count() const					//@cmember	Get count of runs
	{
		return _prgRun->Count();
	}
	BOOL	SameRuns(CRunPtrBase *prp)		//@cmember Return TRUE iff same runs
	{
		return _prgRun == prp->_prgRun;
	}

	//
	//@comm 	Character position control
	//

	DWORD 	BindToCp(DWORD cp);	//@cmember	Set cp for this run ptr = <p cp>
	DWORD 	GetCp() const;		//@cmember	Retrieve current cp
	LONG	AdvanceCp(LONG cch);//@cmember	Advance cp by <p cch> chars
	void 	AdjustBackward();	//@cmember	If on the edge of two runs, 
								// adjust to end of left (previous) run
	void	AdjustForward();	//@cmember	If at the edge of two runs,
								// adjust to start of right (next) run
	DWORD 	GetIch() const		//@cmember	Return <md CRunPtrBase::_ich>
				{return _ich;}
	DWORD	GetCchLeft() const;	//@cmember	Return GetRun(0)->_cch - GetIch()								
	BOOL	IsValid() const;	//@cmember	Return FALSE if run ptr is in
								// empty or NULL states.  TRUE otherwise
	void	SetToNull();		//@cmember	Clears data from run pointer

//@access Protected Data
protected:
	CRunArray *	_prgRun;    	//@cmember	Pointer to CTxtRun array
	DWORD 		_iRun;  	    //@cmember	Index of current run in array
	DWORD 		_ich;		    //@cmember	Char offset inside current run

};


/*
 *	CRunPtr	(template)
 *
 *	@class	a template over CRunPtrBase allowing for type-safe versions of
 *		run pointers
 * 
 *	@tcarg	class 	| CElem | run array class to be used
 *
 *	@base	public | CRunPtrBase
 */

template <class CElem>
class CRunPtr : public CRunPtrBase
{
public:
	CRunPtr (void)								//@cmember	Constructor
		: CRunPtrBase (0) {}
	CRunPtr (CRunArray *prgRun)					//@cmember	Constructor
		: CRunPtrBase (prgRun) {}
	CRunPtr (CRunPtrBase& rp)					//@cmember	Constructor
		: CRunPtrBase (rp) {}

	// Array management 
										
	CElem *	Add (DWORD cRun, DWORD *pielIns)	//@cmember Add <p cRun> 	
	{											// elements at end of array
		return (CElem *)_prgRun->Add(cRun, pielIns);
	}
										
	CElem *	Insert (DWORD cRun)					//@cmember Insert <p cRun>
	{											// elements at current pos
		return (CElem *)_prgRun->Insert(_iRun, cRun);
	}
										
	void 	Remove (LONG cRun, ArrayFlag flag)	//@cmember Remove <p cRun>
	{											// elements at current pos
		_prgRun->Remove (_iRun, cRun, flag);
	}
										//@cmember	Replace <p cRun> elements
										// at current position with those
										// from <p parRun>
	BOOL 	Replace (LONG cRun, CArrayBase *parRun)
	{
		return _prgRun->Replace(_iRun, cRun, parRun);
	}

	CElem *	Elem(LONG iRun) const		//@cmember	Get ptr to run <p iRun>
	{
		return (CElem *)_prgRun->Elem(iRun);
	}
										
	CElem *	GetRun(LONG cRun) const		//@cmember	Get ptr <p cRun> runs
	{									//  away from current run
		return Elem(_iRun + cRun);
	}

	void	IncPtr(CElem *&pRun) const	//@cmember	Increment ptr <p pRun>
	{
		VALIDATE_PTR(pRun);				// Allow invalid ptr after ++ for
		pRun++;							//  for loops
	}
										
	CElem *	GetPtr(CElem *pRun, LONG cRun) const//@cmember Get ptr <p cRun>
	{											// runs away from ptr <p pRun>
		VALIDATE_PTR(pRun + cRun);
		return pRun + cRun;
	}
};

#endif


