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
 *	@doc 	INTERNAL
 *
 *	@module _FRUNPTR.H  -- CFormatRunPtr class declaration |
 *	
 *	Original Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	History: <nl>
 *		06-25-95	alexgo	cleanup and commenting
 *
 */

#ifndef _FRUNPTR_H
#define _FRUNPTR_H

#include "_array.h"
#include "_text.h"
#include "_runptr.h"
#include "_format.h"

#define	CharFormat 0
#define	ParaFormat 1

/*
 *	CFormatRunPtr
 *
 *	@class	A Run pointer over an array of CFormatRun structures. 
 *	This pointer understands how to add remove character/paragraph
 *	formatting
 *
 *	@base	public | CRunPtr<lt>CFormatRun<gt>
 *
 *	@devnote	The format run pointer has one extra interesting state
 *				transistion beyond the normal CRunPtrBase transistions.
 *
 *				If this run pointer is in the NULL state, then InitRuns may
 *				be used to create or fetch the correct run array for the
 *				run pointer.  Note that if a run array is already allocated
 *				it will be simply be fetched and used.  This allows us to
 *				treat unitialized run pointers the same as run pointers to
 *				an uninitialized document.
 *				@xref See also <mf CFormatRunPtr::InitRuns>
 */
class CFormatRunPtr : public CRunPtr<CFormatRun>
{
	friend class CRchTxtPtr;
	friend class CTxtRange;
	friend class CReplaceFormattingAE;

//@access Public Methods
public:
#ifdef DEBUG
	BOOL	Invariant(void) const;			//@cmember	Invariant tests
#endif
								
	CFormatRunPtr(const CFormatRunPtr &rp)	//@cmember	Copy Constructor
		: CRunPtr<CFormatRun>((CRunPtrBase&)rp) {}
								
	CFormatRunPtr(CFormatRuns *pfr)			//@cmember	Constructor
		: CRunPtr<CFormatRun>((CRunArray *)pfr) {}
								
	LONG	GetFormat() const;			//@cmember	Get current format index

//@access Private Methods
private:
								//@cmember Format Run Array management
	BOOL	InitRuns(DWORD ich, DWORD cch, CFormatRuns **ppfrs);
								//@cmember Formatting replacement
	void	Delete(DWORD cch, IFormatCache *pf, LONG cchMove);
								//@cmember Formatting insertion
	DWORD	InsertFormat(DWORD cch, LONG iformat, IFormatCache *pf);
								//@cmember Merge two adjacent formatting runs
	void	MergeRuns(DWORD iRun, IFormatCache *pf);
								//@cmember Sets the format for the current run
	DWORD	SetFormat(LONG ifmt, DWORD cch, IFormatCache *pf);
								//@cmember Extends formatting from previous run
	void	AdjustFormatting(DWORD cch, IFormatCache *pf);
};



class CTxtEdit;

class CCFRunPtr : public CFormatRunPtr
{
	friend class CRchTxtPtr;
	friend class CTxtRange;

public:
	CTxtEdit *_ped;

	CCFRunPtr(const CRchTxtPtr &rtp);	//@cmember	Copy Constructor

	BOOL	IsHidden();			//@cmember True if charformat is hidden
	BOOL	IsInHidden();		//@cmember True if in hidden text
	LONG	FindUnhidden();		//@cmember Find nearest unhidden CF
	LONG	FindUnhiddenBackward();//@cmember Find previous unhidden CF
	LONG	FindUnhiddenForward();//@cmember Find previous unhidden CF
};

class CPFRunPtr : public CFormatRunPtr
{
	friend class CRchTxtPtr;
	friend class CTxtRange;

public:
	CTxtEdit *_ped;

	CPFRunPtr(const CRchTxtPtr &rtp);	//@cmember	Copy Constructor
								//@cmember Find Heading before cpMost
	LONG	FindHeading(LONG cch, LONG& lHeading);
	BOOL	IsCollapsed();		//@cmember True if paraformat is collapsed
#ifdef PWD_JUPITER // GuyBark Jupiter 18391
	LONG	FindExpanded(BOOL bDisallowBackwardSearch);		//@cmember Find nearest expanded PF
#else
	LONG	FindExpanded();		//@cmember Find nearest expanded PF
#endif // PWD_JUPITER
	LONG	FindExpandedBackward();//@cmember Find previous expanded PF
	LONG	FindExpandedForward();//@cmember Find next expanded PF
	LONG	GetOutlineLevel();	//@cmember Get outline level
	LONG	GetStyle();			//@cmember Get style
};

#endif

