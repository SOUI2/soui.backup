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
 *	@module	range.cpp - Implement the CTxtRange Class |
 *	
 *		This module implements the internal CTxtRange methods.  See range2.c
 *		for the ITextRange methods
 *
 *	Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	Revisions: <nl>
 *		AlexGo: update to runptr text ptr; floating ranges, multilevel undo
 *
 */

#include "_common.h"
#include "_range.h"
#include "_edit.h"
#include "_text.h"
#include "_rtext.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_disp.h"

ASSERTDATA

TCHAR	szEmbedding[] = {WCH_EMBEDDING, 0};

// ===========================  Invariant stuff  ======================================================

#define DEBUG_CLASSNAME CTxtRange
#include "_invar.h"

#ifdef DEBUG
BOOL
CTxtRange::Invariant( void ) const
{
	LONG cpMin, cpMost;
	LONG diff = GetRange(cpMin, cpMost);

	Assert ( cpMin >= 0 );
	Assert ( cpMin <= cpMost );
	Assert ( cpMost <= GetTextLength() );
	Assert ( cpMin != cpMost || cpMost <= (LONG)GetAdjustedTextLength());

	static LONG	numTests = 0;
	numTests++;				// how many times we've been called.

	// make sure the selections are in range.

	return CRchTxtPtr::Invariant();
}
#endif


CTxtRange::CTxtRange(CTxtEdit *ped, LONG cp, LONG cch) :
	CRchTxtPtr(ped, cp),
	_cch(0),
	_iFormat(0),
	_cRefs(0)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

	LONG cchText = GetAdjustedTextLength();
	LONG cpOther = cp - cch;			// Calculate cpOther with entry cp

	_dwFlags = FALSE;					// This range isn't a selection
	_iFormat = -1;						// Set up the default format, which
										//  doesn't get AddRefFormat'd
	ValidateCp(cpOther);				// Validate requested other end
	cp = GetCp();						// Validated cp
	if(cp == cpOther && cp > cchText)	// IP cannot follow undeletable
		cp = cpOther = SetCp(cchText);	//  EOP at end of story

	_cch = cp - cpOther;				// Store valid length
	Update_iFormat(-1);					// Choose _iFormat

	CNotifyMgr *pnm = ped->GetNotifyMgr();

    if( pnm )
        pnm->Add( (ITxNotify *)this );
}

CTxtRange::CTxtRange(const CTxtRange &rg) :
	CRchTxtPtr((CRchTxtPtr)rg),
    _cch(0),
	_iFormat(0),
	_cRefs(0)
	
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

	_cch = rg._cch;
	_dwFlags = FALSE;				// This range isn't a selection
	_iFormat = -1;					// Set up the default format, which
									//  doesn't get AddRefFormat'd
	Set_iCF(rg._iFormat);

	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

    if( pnm )
        pnm->Add( (ITxNotify *)this );
}

CTxtRange::~CTxtRange()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::~CTxtRange");

	if(!IsZombie())
	{
		CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

		if( pnm )
			pnm->Remove( (ITxNotify *)this );
	}

	ReleaseFormats(_iFormat, -1);
}

CRchTxtPtr& CTxtRange::operator =(const CRchTxtPtr &rtp)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::operator =");

	_TEST_INVARIANT_ON(rtp)

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	CRchTxtPtr::operator =(rtp); 
	CheckChange(cpSave);
	return *this;
}

CTxtRange& CTxtRange::operator =(const CTxtRange &rg)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::operator =");

	_TEST_INVARIANT_ON( rg );

	LONG cchSave = _cch;			// Save entry _cp, _cch for change check
	LONG cpSave  = GetCp();

	CRchTxtPtr::operator =(rg);
	_cch = rg._cch;					// Can't use CheckChange(), since don't
									//  use _fExtend
	Update_iFormat(-1); 
	_TEST_INVARIANT_

	if( _fSel && (cpSave != GetCp() || cchSave != _cch) )
		GetPed()->GetCallMgr()->SetSelectionChanged();

	return *this;
}

/*
 *	CTxtRange::OnPreReplaceRange (cp, cchDel, cchNew, cpFormatMin,
 *									cpFormatMax)
 *
 *	@mfunc
 *		called when the backing store changes
 *
 *	@devnote
 *		1) if this range is before the changes, do nothing
 *
 *		2) if the changes are before this range, simply
 *		add the delta change to GetCp()
 *
 *		3) if the changes overlap one end of the range, collapse
 *		that end to the edge of the modifications
 *
 *		4) if the changes are completely internal to the range,
 *		adjust _cch and/or GetCp() to reflect the new size.  Note
 *		that two overlapping insertion points will be viewed as
 *		a 'completely internal' change.
 *
 *		5) if the changes overlap *both* ends of the range, collapse
 *		the range to cp
 *
 *		Note that there is an ambiguous cp case; namely the changes
 *		occur *exactly* at a boundary.  In this case, the type of
 *		range matters.  If a range is normal, then the changes
 *		are assumed to fall within the range.  If the range is
 *		is protected (either in reality or via DragDrop), then
 *		the changes are assumed to be *outside* of the range.
 */
void CTxtRange::OnPreReplaceRange (
	DWORD cp,					//@parm cp at start of change
	DWORD cchDel,				//@parm Count of chars deleted
	DWORD cchNew,				//@parm Count of chars inserted
	DWORD cpFormatMin,			//@parm the min cp of a format change
	DWORD cpFormatMax)			//@parm the max cp of a format change
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::OnPreReplaceRange");

	if (CONVERT_TO_PLAIN == cp)
	{
		// We need to dump our formatting because it is gone.
		_rpCF.SetToNull();
		_rpPF.SetToNull();

		if( _fSel )
		{
			GetPed()->_fUpdateSelection = TRUE;	
		}

		Update_iFormat(-1);
		return;
	}
}

/*
 *	CTxtRange::OnPostReplaceRange (cp, cchDel, cchNew, cpFormatMin,
 *									cpFormatMax)
 *
 *	@mfunc
 *		called when the backing store changes
 *
 *	@devnote
 *		1) if this range is before the changes, do nothing
 *
 *		2) if the changes are before this range, simply
 *		add the delta change to GetCp()
 *
 *		3) if the changes overlap one end of the range, collapse
 *		that end to the edge of the modifications
 *
 *		4) if the changes are completely internal to the range,
 *		adjust _cch and/or GetCp() to reflect the new size.  Note
 *		that two overlapping insertion points will be viewed as
 *		a 'completely internal' change.
 *
 *		5) if the changes overlap *both* ends of the range, collapse
 *		the range to cp
 *
 *		Note that there is an ambiguous cp case; namely the changes
 *		occur *exactly* at a boundary.  In this case, the type of
 *		range matters.  If a range is normal, then the changes
 *		are assumed to fall within the range.  If the range is
 *		is protected (either in reality or via DragDrop), then
 *		the changes are assumed to be *outside* of the range.
 */
void CTxtRange::OnPostReplaceRange (
	DWORD cp,					//@parm cp at start of change
	DWORD cchDel,				//@parm Count of chars deleted
	DWORD cchNew,				//@parm Count of chars inserted
	DWORD cpFormatMin,			//@parm the min cp of a format change
	DWORD cpFormatMax)			//@parm the max cp of a format change
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::OnPostReplaceRange");

	// NB!! We can't do invariant testing here, because we could
	// be severely out of date!

	DWORD cchtemp;
	DWORD cpMin, cpMost;
	LONG cchAdjTextLen;
	LONG delta = cchNew - cchDel;

	Assert (CONVERT_TO_PLAIN != cp);
	GetRange((LONG&)cpMin, (LONG&)cpMost);
	
	// This range is before the changes. Note: an insertion pt at cp
	// shouldn't be changed
	if( cp >= cpMost )
	{
		// double check to see if we need to fix up our format
		// run pointers.  If so, all we need to do is rebind
		// our inherited rich text pointer

		if( cpFormatMin <=  cpMost || cpFormatMin == INFINITE)
		{
			InitRunPtrs(GetCp());
		}
		else
		{
		 	// It's possible that the format runs changed anyway,
			// e.g., they became allocated, deallocated, or otherwise
			// changed.  Normally, BindToCp takes care of this
			// situation, but we don't want to pay that cost all
			// the time.
			//
			// Note that starting up the rich text subsystem will 
			// generate a notification with cpFormatMin == INFINITE
			//
			// So here, call CheckFormatRuns.  This makes sure that
			// the runs are in sync with what CTxtStory has
			// (doing an InitRunPtrs() _only_ if absolutely necessary).
			CheckFormatRuns();
		}
		return;
	}


	// Anywhere in the following that we want to increment the current cp by a
	// delta, we are counting on the following invariant.
	Assert(GetCp() >= 0);

	// changes are entirely before this range.  Specifically,
	// that's determined by looking at the incoming cp *plus* the number
	// of characters deleted
	if( (cp + cchDel) < cpMin  ||
		(_fDragProtection == TRUE && (cp + cchDel) <= cpMin ))
	{
		cchtemp = _cch;
		BindToCp(GetCp() + delta);
		_cch = cchtemp;
	}	
	// the changes are internal to the range or start within the
	// range and go beyond.
	else if( cp >= cpMin && cp <= cpMost )
	{
		// nobody should be modifying a drag-protected range.  Unfortunately,
		// Ren re-enters us with a SetText call during drag drop, so we need
		// to handle this case 'gracefully'.
		if( _fDragProtection )
		{
			TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
		}

		if( cp + cchDel <= cpMost )
		{
			// changes are purely internal, so
			// be sure to preserve the active end.  Basically, if
			// GetCp() *is* cpMin, then we only need to update _cch.
			// Otherwise, GetCp() needs to be moved as well
			if( _cch >= 0 )
			{
				Assert(GetCp() == (LONG)cpMost);
				cchtemp = _cch;
				BindToCp(GetCp() + delta);
				_cch = cchtemp + delta;
			}
			else
			{
				BindToCp(GetCp());
				_cch -= delta;
			}

			// Special case: the range is left with only the final EOP
			// selected. This means all the characters in the range were
			// deleted so we want to move the range back to an insertion
			// point at the end of the text.
			cchAdjTextLen = GetPed()->GetAdjustedTextLength();

			if (GetCpMin() >= cchAdjTextLen)
			{
				// Reduce the range to an insertion point
				_cch = 0;

				_fExtend = FALSE;

				// Set the cp to the end of the document.
				SetCp(cchAdjTextLen);
			}
		}
		else
		{
			// Changes extended beyond cpMost.  In this case,
			// we want to truncate cpMost to the *beginning* of 
			// the changes (i.e. cp)

			if( _cch > 0 )
			{
				BindToCp(cp);
				_cch = cp - cpMin;
			}
			else
			{
				BindToCp(cpMin);
				_cch = cpMin - cp;
			}
		}
	}
	else if( (cp + cchDel) >= cpMost )
	{
		// nobody should be modifying a drag-protected range.  Unfortunately,
		// Ren re-enters us with a SetText call during drag drop, so we need
		// to handle this case 'gracefully'.
		if( _fDragProtection )
		{
			TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
		}

		// entire range was deleted, so collapse to an insertion point at cp
		BindToCp(cp);
		_cch = 0;
	}
	else
	{
		// nobody should be modifying a drag-protected range.  Unfortunately,
		// Ren re-enters us with a SetText call during drag drop, so we need
		// to handle this case 'gracefully'.
		if( _fDragProtection )
		{
			TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
		}

		// the change crossed over just cpMin.  In this case move cpMin
		// forward to the unchanged part
		LONG cchdiff = (cp + cchDel) - cpMin;

		Assert( (cp + cchDel) < cpMost );
		Assert( (cp + cchDel) >= cpMin );
		Assert( cp < cpMin );

		cchtemp = _cch;
		if( _cch > 0 )
		{
			BindToCp(GetCp() + delta);
			_cch = cchtemp - cchdiff;
		}
		else
		{
			BindToCp(cp + cchNew);
			_cch = cchtemp + cchdiff;
		}
	}

	if( _fSel )
	{
		GetPed()->_fUpdateSelection = TRUE;		
		GetPed()->GetCallMgr()->SetSelectionChanged();
	}

	Update_iFormat(-1);					// Make sure format is up to date

	_TEST_INVARIANT_
}	

/*
 *	CTxtRange::Zombie ()
 *
 *	@mfunc
 *		Turn this range into a zombie (_cp = _cch = 0, NULL ped, ptrs to
 *		backing store arrays.  CTxtRange methods like GetRange(),
 *		GetCpMost(), GetCpMin(), and GetTextLength() all work in zombie mode,
 *		returning zero values.
 */
void CTxtRange::Zombie()
{
	CRchTxtPtr::Zombie();
	_cch = 0;
}

/*
 *	CTxtRange::CheckChange(cpSave, cchSave)
 *
 *	@mfunc
 *		Set _cch according to _fExtend and set selection-changed flag if
 *		this range is a CTxtSelection and the new _cp or _cch differ from
 *		cp and cch, respectively.
 *
 *	@devnote
 *		We can count on GetCp() and cpSave both being <= GetTextLength(),
 *		but we can't leave GetCp() equal to GetTextLength() unless _cch ends
 *		up > 0.
 */
LONG CTxtRange::CheckChange(
	LONG cpSave)		//@parm Original _cp for this range
{
	LONG cchAdj = GetAdjustedTextLength();
	LONG cchSave = _cch;

	if(_fExtend)								// Wants to be nondegenerate
	{											//  and maybe it is
		LONG cp = GetCp();

		_cch = cp - (cpSave - cchSave);
		CheckIfSelHasEOP(cpSave, cchSave);
	}
	else
	{
		_cch = 0;								// Insertion point
		_fSelHasEOP = FALSE;					// Selection doesn't contain
		_fSelHasCell = FALSE;					//  any char, let alone a CR
	}											//  or table cell

	if(!_cch && GetCp() > cchAdj)				// If still IP and active end
		CRchTxtPtr::SetCp(cchAdj);				//  follows nondeletable EOP,
												//  backspace over that EOP
	if(cpSave != GetCp() || cchSave != _cch)
	{
		Update_iFormat(-1);
		if(_fSel)
			GetPed()->GetCallMgr()->SetSelectionChanged();

		_TEST_INVARIANT_
	}

	LONG cch = GetCp() - cpSave;
	_fMoveBack = cch < 0;
	return cch;
}

/*
 *	CTxtRange::CheckIfSelHasEOP(cpSave, cchSave)
 *	
 *	@mfunc
 *		Maintains _fSelHasEOP = TRUE iff selection contains one or more EOPs.
 *		When cpSave = -1, calculates _fSelHasEOP unconditionally and cchSave
 *		is ignored (it's only used for conditional execution). Else _fSelHasEOP
 *		is only calculated for cases that may change it, i.e., it's assumed
 *		be up to date before the change.
 *
 *	@rdesc
 *		TRUE iff _fSel and _cch
 *
 *	@devnote
 *		Call after updating range _cch
 */
BOOL CTxtRange::CheckIfSelHasEOP(
	LONG cpSave,	//@parm Previous active end cp or -1
	LONG cchSave)	//@parm Previous signed length if cpSave != -1
{
	// _fSelHasEOP only maintained for the selection
	if(!_fSel)
		return FALSE;

	if(!_cch)
	{
		_fSelHasEOP  = FALSE;			// Selection doesn't contain
		_fSelHasCell = FALSE;			//  any char, let alone CR
		return FALSE;					
	}

	LONG cpMin, cpMost;					
	GetRange(cpMin, cpMost);

	if(cpSave != -1)					// Selection may have changed
	{									// Set up to bypass text scan if
		LONG cpMinPrev, cpMostPrev;		//  selection grew and _fSelHasEOP
										//  is already TRUE or got smaller
		cpMinPrev = cpMostPrev = cpSave;//  and _fSelHasEOP is FALSE.

		if(cchSave > 0)					// Calculate previous cpMin
			cpMinPrev  -= cchSave;		//  and cpMost
		else
			cpMostPrev -= cchSave;

		// Note: _fSelHasCell shouldn't change while in a table, since
		// Update() should always expand to a cell once _fSelHasCell has
		// been deteted.
		if (!_fSelHasEOP && cpMin >= cpMinPrev && cpMost <= cpMostPrev ||
			 _fSelHasEOP && cpMin <= cpMinPrev && cpMost >= cpMostPrev)
		{		
			return TRUE;				// _fSelHasEOP can't change
		}
	}
	
	LONG	FEOP_Results;
	CTxtPtr tp(_rpTX);					// Scan range for an EOP

	tp.SetCp(cpMin);
	tp.FindEOP(cpMost - cpMin, &FEOP_Results);
	_fSelHasCell = (FEOP_Results & FEOP_CELL) != 0;
	_fSelHasEOP  = (FEOP_Results & FEOP_EOP)  != 0;
	return TRUE;
}

/*
 *	CTxtRange::GetRange(&cpMin, &cpMost)
 *	
 *	@mfunc
 *		set cpMin  = this range cpMin
 *		set cpMost = this range cpMost
 *		return cpMost - cpMin, i.e. abs(_cch)
 *	
 *	@rdesc
 *		abs(_cch)
 */
LONG CTxtRange::GetRange (
	LONG& cpMin,				// @parm Pass-by-ref cpMin
	LONG& cpMost) const			// @parm Pass-by-ref cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetRange");

	LONG cch = _cch;

	if(cch >= 0)
	{
		cpMost	= GetCp();
		cpMin	= cpMost - cch;
	}
	else
	{
		cch		= -cch;
		cpMin	= GetCp();
		cpMost	= cpMin + cch;
	}
	return cch;
}

/*
 *	CTxtRange::GetCpMin()
 *	
 *	@mfunc
 *		return this range's cpMin
 *	
 *	@rdesc
 *		cpMin
 *
 *	@devnote
 *		If you need cpMost and/or cpMost - cpMin, GetRange() is faster
 */
LONG CTxtRange::GetCpMin() const
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCpMin");

	LONG cp = GetCp();
	return min(cp, cp - _cch);
}

/*
 *	CTxtRange::GetCpMost()
 *	
 *	@mfunc
 *		return this range's cpMost
 *	
 *	@rdesc
 *		cpMost
 *
 *	@devnote
 *		If you need cpMin and/or cpMost - cpMin, GetRange() is faster
 */
LONG CTxtRange::GetCpMost() const
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCpMost");

	LONG cp = GetCp();
	return max(cp, cp - _cch);
}

/*
 *	CTxtRange::Update(fScrollIntoView)
 *
 *	@mfunc
 *		Virtual stub routine overruled by CTxtSelection::Update() when this
 *		text range is a text selection.  The purpose is to update the screen
 *		display of the caret or	selection to correspond to changed cp's.
 *
 *	@rdesc
 *		TRUE
 */
BOOL CTxtRange::Update (
	BOOL fScrollIntoView)		//@parm TRUE if should scroll caret into view
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Update");

	return TRUE;				// Simple range has no selection colors or
}								//  caret, so just return TRUE

/*
 * CTxtRange::SetCp(cp)
 *
 *	@mfunc
 *		Set active end of this range to cp. Leave other end where it is or
 *		collapse range depending on _fExtend (see CheckChange()).
 *
 *	@rdesc
 *		cp at new active end (may differ from cp, since cp may be invalid).
 */
DWORD CTxtRange::SetCp(
	DWORD cp)			// @parm new cp for active end of this range
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtRange::SetCp");

	LONG cpSave = GetCp();

	CRchTxtPtr::SetCp(cp);
	CheckChange(cpSave);					// NB: this changes _cp if after 
	return GetCp();							//  final CR and _cch = 0
}

/*
 *	CTxtRange::Set (cp, cch)
 *	
 *	@mfunc
 *		Set this range's active-end cp and signed cch
 */
BOOL CTxtRange::Set (
	LONG cp,					// @parm Desired active end cp
	LONG cch)					// @parm Desired signed count of chars
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Set");

	BOOL bRet	 = FALSE;
	LONG cchSave = _cch;			// Save entry _cp, _cch for change check
	LONG cchText = GetAdjustedTextLength();
	LONG cpSave  = GetCp();
	LONG cpOther = cp - cch;		// Desired "other" end

	ValidateCp(cp);							// Be absolutely sure to validate
	ValidateCp(cpOther);					//  both ends

	if(cp == cpOther && cp > cchText)		// IP cannot follow undeletable
		cp = cpOther = cchText;				//  EOP at end of story

	CRchTxtPtr::Advance(cp - GetCp());
	AssertSz(cp == GetCp(),
		"CTxtRange::Set: inconsistent cp");

	_cch = cp - cpOther;					// Validated _cch value
	CheckIfSelHasEOP(cpSave, cchSave);		// Maintain _fSelHasEOP in
											//  outline mode
	if(cpSave != GetCp() || cchSave != _cch)
	{
		if(_fSel)
			GetPed()->GetCallMgr()->SetSelectionChanged();

		Update_iFormat(-1);
		bRet = TRUE;
	}
	 
	_fMoveBack = GetCp() < cpSave;
	_TEST_INVARIANT_
	return bRet;
}

/*
 *	CTxtRange::Advance(cch)
 *
 *	@mfunc
 *		Advance active end of range by cch.
 *		Other end stays put iff _fExtend
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::Advance (
	LONG cch)				// @parm Signed char count to move active end
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Advance");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()
		
	CRchTxtPtr::Advance(cch);
	return CheckChange(cpSave);
}	

/*
 *	CTxtRange::AdvanceCRLF()
 *
 *	@mfunc
 *		Advance active end of range one char, treating CRLF as a single char.
 *		Other end stays put iff _fExtend is nonzero.
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::AdvanceCRLF()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::AdvanceCRLF");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	CRchTxtPtr::AdvanceCRLF();
	return CheckChange(cpSave);
}

/*
 *	CTxtRange::BackupCRLF()
 *
 *	@mfunc
 *		Backup active end of range one char, treating CRLF as a single char.
 *		Other end stays put iff _fExtend
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::BackupCRLF()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::BackupCRLF");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	CRchTxtPtr::BackupCRLF();
	return CheckChange(cpSave);
}

/*
 *	CTxtRange::FindWordBreak(action)
 *
 *	@mfunc
 *		Move active end as determined by plain-text FindWordBreak().
 *		Other end stays put iff _fExtend
 *
 *	@rdesc
 *		cch actually moved
 */
LONG CTxtRange::FindWordBreak (
	INT action)			// @parm action defined by CTxtPtr::FindWordBreak()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtPtr::FindWordBreak");

	LONG cpSave = GetCp();			// Save entry _cp for CheckChange()

	CRchTxtPtr::FindWordBreak(action);
	return CheckChange(cpSave);
}

/*
 *	CTxtRange::FlipRange()
 *
 *	@mfunc
 *		Flip active and non active ends
 */
void CTxtRange::FlipRange()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FlipRange");

	_TEST_INVARIANT_

	CRchTxtPtr::Advance(-_cch);
	_cch = -_cch;
}
	
/*
 *	CTxtRange::CleanseAndReplaceRange(cch, *pch, fTestLimit, publdr)
 *	
 *	@mfunc
 *		Cleanse the string pch (replace CRLFs by CRs, etc.) and substitute
 *		the resulting string for the text in this range using the CCharFormat
 *		_iFormat and updating other text runs as needed. For single-line
 *		controls, truncate on the first EOP and substitute the truncated
 *		string.  Also truncate if string would overflow the max text length.
 *	
 *	@rdesc
 *		Count of new characters added
 *
 *	@devnote
 *		If we know somehow that pch comes from RichEdit, we shouldn't have to
 *		cleanse it.  This could be tied into delayed IDataObject rendering.
 *		This code is similar to that in CLightDTEngine::ReadPlainText(), but
 *		deals with a single string instead of a series of stream buffers.
 */
LONG CTxtRange::CleanseAndReplaceRange (
	LONG			cch,		// @parm Length of replacement text
	const TCHAR *	pch,		// @parm Replacement text
	BOOL			fTestLimit,	// @parm whether we need to do a limit test
	IUndoBuilder *	publdr)		// @parm UndoBuilder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CleanseAndReplaceRange");

	CTxtEdit *	ped = GetPed();
	LONG		cchLen = CalcTextLenNotInRange();
	DWORD		cchMax = ped->TxGetMaxLength();
	LONG		cchT;
	CTempWcharBuf twcb;
	TCHAR *		pchT;

	if(!pch)									// No text so no cleansing
		cch = 0;

	else if(!ped->TxGetMultiLine())				// Single-line control
	{
		if(cch < 0)								// Calculate string length
			cch = tomForward;					//  while looking for EOP
												// Truncate at 1st EOP to be
		for(cchT = 0; cchT < cch &&				//  compatible with RE 1.0
			*pch && !IsASCIIEOP(*pch);			//  and user's SLE and to
			cchT++, pch++)						//  give consistent display
			;									//  behavior
		cch = cchT;
		pch -= cchT;							// Restore pch
	}
	else										// Multiline control
	{
		if(cch < 0)								// Calculate length
			cch = (LONG)wcslen(pch);

		if(!GetPed()->Get10Mode() && cch)		// Cleanse if not RE 1.0
		{										//  and some new chars
			pchT = twcb.GetBuf(cch);

			if (NULL == pchT)
			{
				// Could not allocate buffer so give up with no update.
				return 0;
			}

			cch = Cleanse(pchT, pch, cch);
			pch = pchT;
		}
	}
	if(fTestLimit && cch && (DWORD)(cch + cchLen) > cchMax)	// New plus old	count exceeds
	{											//  max allowed, so truncate
		cch = cchMax - cchLen;					//  down to what fits
		cch = max(cch, 0);						// Keep it positive
		ped->GetCallMgr()->SetMaxText();		// Tell anyone who cares
	}

	LONG cchUpdate = ReplaceRange(cch, pch, publdr, SELRR_REMEMBERRANGE);

	return cchUpdate;
}

/*
 *	CTxtRange::ReplaceRange(cchNew, *pch, publdr. selaemode)
 *	
 *	@mfunc
 *		Replace the text in this range by pch using CCharFormat _iFormat
 *		and updating other text runs as needed.
 *	
 *	@rdesc
 *		Count of new characters added
 *	
 *	@devnote
 *		moves this text pointer to end of replaced text and
 *		may move text block and formatting arrays
 */
LONG CTxtRange::ReplaceRange (
	LONG			cchNew,		// @parm Length of replacement text
	TCHAR const *	pch,		// @parm Replacement text
	IUndoBuilder *	publdr,		// @parm UndoBuilder to receive antievents
	SELRR			selaemode)	// @parm Controls how selection antievents
								// are to be generated.
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::ReplaceRange");

	LONG lRet;
	LONG iFormat = _iFormat;
	BOOL fReleaseFormat = FALSE;
	ICharFormatCache * pcf;

	_TEST_INVARIANT_

	if(!(cchNew | _cch))					// Nothing to add or delete,
		return 0;							//  so we're done

	if( publdr && selaemode != SELRR_IGNORE )
	{
		Assert(selaemode == SELRR_REMEMBERRANGE);
		HandleSelectionAEInfo(GetPed(), publdr, GetCp(), _cch, 
				GetCpMin() + cchNew, 0, SELAE_MERGE);
	}
	
	if(_cch > 0)
		FlipRange();

	// If we are replacing a non-degenerate selection, then the Word95
	// UI specifies that we should use the rightmost formatting at cpMin.

	if( _cch < 0 && _rpCF.IsValid() && !_fDualFontMode && !_fUseiFormat )
	{
		_rpCF.AdjustForward();
		iFormat = _rpCF.GetFormat();

		// This is a bit icky, but the idea is to stabilize the
		// reference count on iFormat.  When we get it above, it's
		// not addref'ed, so if we happen to delete the text in the
		// range and the range is the only one with that format,
		// then the format will go away.

		if(FAILED(GetCharFormatCache(&pcf)))
		{
			AssertSz(0, "couldn't get format cache yet we have formatting");
			return 0;
		}
		pcf->AddRefFormat(iFormat);

		fReleaseFormat = TRUE;
	}
	_fUseiFormat = FALSE;
	
	LONG cchForReplace = -_cch;	
	_cch = 0;
	lRet = CRchTxtPtr::ReplaceRange(cchForReplace, cchNew, pch, publdr, 
				iFormat);
	Update_iFormat(fReleaseFormat ? iFormat : -1);

	if(fReleaseFormat)
	{
		Assert(pcf);
		pcf->ReleaseFormat(iFormat);
	}

	return lRet;
}

/*
 *	CTxtRange::GetCharFormat(pCF, flags)
 *	
 *	@mfunc
 *		Set *pCF = CCharFormat for this range. If cbSize = sizeof(CHARFORMAT)
 *		only transfer CHARFORMAT data.
 *	
 *	@devnote
 *		NINCH means No Input No CHange (a Microsoft Word term). Here used for
 *		properties that change during the range of cch characters.	NINCHed
 *		properties in a Word-Font dialog have grayed boxes. They are indicated
 *		by zero values in their respective dwMask bit positions. Note that
 *		a blank at the end of the range does not participate in the NINCH
 *		test, i.e., it can have a different CCharFormat without zeroing the
 *		corresponding dwMask bits.  This is done to be compatible with Word
 *		(see also CTxtSelection::SetCharFormat when _fWordSelMode is TRUE).
 */
void CTxtRange::GetCharFormat (
	CCharFormat *pCF, 		// @parm CCharFormat to fill with results
	DWORD flags) const		// @parm flags
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCharFormat");
	_TEST_INVARIANT_
	
	CTxtEdit * const ped = GetPed();

	if(!_cch || !_rpCF.IsValid())					// IP or invalid CF
	{												//	run ptr: use CF at
		ped->GetCharFormat(_iFormat)->Get(pCF);		//  this text ptr
		return;
	}

	LONG		  cpMin, cpMost;					// Nondegenerate range:
	LONG		  cch = GetRange(cpMin, cpMost);	//  need to scan
	LONG		  cchChunk;							// cch in CF run
	LONG		  iDirection;						// Direction of scan
	CFormatRunPtr rp(_rpCF);						// Nondegenerate range

	/*
	 * The code below reads character formatting the way Word does it,
	 * that is, by not including the formatting of the last character in the
	 * selection if that character is a blank. 
	 *
	 * See also the corresponding code in CTxtSelection::SetCharFormat().
	 */

	if(cch > 1 && _fSel && (flags & SCF_USEUIRULES))// If more than one char,
	{												//  don't include trailing
		CTxtPtr tp(ped, cpMost - 1);				//  blank in NINCH test
		if(tp.GetChar() == ' ')
		{											// Have trailing blank:
			cch--;									//  one less char to check
			if(_cch > 0)							// Will scan backward, so
				rp.AdvanceCp(-1);					//  backup before blank
		}
	}

	if(_cch < 0)									// Setup direction and
	{												//  initial cchChunk
		iDirection = 1;								// Scan forward
		rp.AdjustForward();
		cchChunk = rp.GetCchLeft();					// Chunk size for _rpCF
	}
	else
	{
		iDirection = -1;							// Scan backward
		rp.AdjustBackward();						// If at BOR, go to
		cchChunk = rp.GetIch();						//  previous EOR
	}

	ped->GetCharFormat(rp.GetFormat())->Get(pCF);	// Initialize *pCF to
													//  starting format
	AssertSz(pCF->dwMask == (pCF->cbSize == sizeof(CHARFORMAT)
			? CFM_ALL : CFM_ALL2),
		"CTxtRange::GetCharFormat: dwMask not initialized");
	while(cchChunk < cch)							// NINCH properties that
	{												//  change over the range
		cch -= cchChunk;							//	given by cch
		if(!rp.ChgRun(iDirection))					// No more runs
			return;									//	(cch too big)
		cchChunk = rp.GetRun(0)->_cch;

		const CCharFormat *pCFTemp = ped->GetCharFormat(rp.GetFormat());

		if (pCFTemp)
		{
    		pCFTemp->Delta(pCF);						// NINCH properties that
		}    
        else
        {
            AssertSz(pCFTemp, "ped->GetCharFormat failed");
        }
													//  changed, i.e., reset
	}												//  corresponding bits
}

/*
 *	CTxtRange::SetCharFormat(pCF, flags, publdr)
 *	
 *	@mfunc
 *		apply CCharFormat *pCF to this range.  If range is an insertion point,
 *		and fApplyToWord is TRUE, then apply CCharFormat to word surrounding
 *		this insertion point
 *	
 *	@rdesc
 *		HRESULT = (successfully set whole range) ? NOERROR : S_FALSE
 *
 *	@devnote
 *		SetParaFormat() is similar, but simpler, since it doesn't have to
 *		special case insertion-point ranges or worry about bullet character
 *		formatting, which is given by EOP formatting.
 */
HRESULT CTxtRange::SetCharFormat (
	const CCharFormat *pCF,	// @parm CCharFormat to fill with results
	DWORD flags,			// @parm APPLYTOWORD or IGNORESELAE
	IUndoBuilder *publdr)	// @parm the undo builder to use.
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SetCharFormat");

	LONG				cch = -_cch;			// Defaults for _cch <= 0
	LONG				cchBack = 0;			// cch to back up for formatting
	LONG				cchFormat;				// cch for formatting
	CCharFormat			CF;						// Temporary CF
	LONG				cp = 0;					// Used for DEBUG only
	LONG				cpMin, cpMost;
	LONG                cpStart = 0;
	LONG				cpWordMin, cpWordMost;
	LONG				delta;
	BOOL				fApplyToWord = (flags & APPLYTOWORD);
	BOOL				fApplyToEOP = FALSE;
	BOOL				fProtected = FALSE;
	HRESULT				hr = NOERROR;
	LONG				icf;
	CTxtEdit * const	ped = GetPed();			//  defined and not style
	ICharFormatCache *	pf;

 	_TEST_INVARIANT_

	AssertSz(IsValidCharFormat((CHARFORMAT *)pCF),
		"RTR::SetCharFormat(): invalid CCharFormat");

	if (!Check_rpCF() ||
		FAILED(GetCharFormatCache(&pf)))
	{
		return NOERROR;
	}

	if(_cch > 0)								// Active end at range end
	{
		cchBack = -_cch;						// Setup to back up _rpCF to
		cch = _cch;								//  start of format area
	}
	else if (_cch < 0)
    {
        _rpCF.AdjustForward();
    }
	else if(!cch && fApplyToWord )
	{
		FindWord(&cpWordMin, &cpWordMost, FW_EXACT);

		// If nearest word is within this range, calculate cchback and cch
		// so that we can apply the given format to the word
		if( cpWordMin < GetCp() && GetCp() < cpWordMost )
		{
			// RichEdit 1.0 made 1 final check: ensure word's format
			// is constant w.r.t. the format passed in
			CTxtRange rg(*this);

			rg.Set(cpWordMin, cpWordMin - cpWordMost);
			fProtected = rg.WriteAccessDenied();
			if(!fProtected)
			{
				rg.GetCharFormat(&CF);
				if( (CF.dwMask & pCF->dwMask) == pCF->dwMask )
				{
					cchBack = cpWordMin - GetCp();
					cch = cpWordMost - cpWordMin;
				}
			}
		}
		else if(_rpTX.IsAtEOP() && !GetPF()->wNumbering)
		{
			CTxtPtr tp(_rpTX);
			cch = tp.AdvanceCpCRLF();
			_rpCF.AdjustForward();				// Go onto format EOP
			fApplyToEOP = TRUE;
		}
	}
	cchFormat = cch;

	BOOL fFullyDefined = FALSE;					// Default input CF not fully
	BOOL fInOurHost = ped->fInOurHost();		//  defined	

	if (ped->HandleStyle(&CF, pCF) == NOERROR &&
		pf->Cache(&CF, &icf) == NOERROR)
	{
		fFullyDefined = TRUE;
	}

	if(!cch)									// Set degenerate-range	(IP)
	{											//  CF
		if(!fFullyDefined)						// If pCF doesn't specify
		{										//  full CF, fill in undefined
			ped->GetCharFormat(_iFormat)		// Copy current CF at IP to CF
				->Get(&CF);
			hr = CF.Apply(pCF, fInOurHost);		// Apply *pCF
			if(hr != NOERROR)					// Cache result if new
				return hr;
			hr = pf->Cache(&CF, &icf);			// In any case, get icf
			if(hr != NOERROR)
				return hr;
		}
		pf->ReleaseFormat(_iFormat);			//  AddRefFormat() it)
		_iFormat = icf;
		if(fProtected)							// Signal to beep if UI
			hr = S_FALSE;
	}
	else										// Set nondegenerate-range CF
	{											// Get start of affected area
		CNotifyMgr *pnm = ped->GetNotifyMgr();	// Get the notification mgr
		if(pnm)
		{
			cpStart = GetCp() + cchBack;		// Bulletting may move
												//  affected area back if
			if(GetPF()->wNumbering)				//  formatting hits EOP that
			{									//  affects bullet at BOP
				FindParagraph(&cpMin, &cpMost);

				if(cpMost <= GetCpMost())
					cpStart = cpMin;
			}
			pnm->NotifyPreReplaceRange(this,	// Notify interested parties of
				INFINITE, 0, 0, cpStart,		// the impending update
					cpStart + cchFormat);
		}

		_rpCF.AdvanceCp(cchBack);				// Back up to formatting start
		CFormatRunPtr rp(_rpCF);				// Clone _rpCF to walk range

		if( publdr )
		{
			IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE(
								ped, rp, cch, pf, CharFormat);
			if( pae )
				publdr->AddAntiEvent(pae);
		}
	
		while(cch > 0 && rp.IsValid())
		{
			if(!fFullyDefined)					// If input CF not fully def'd
			{									//  fill in remaining members
				ped->GetCharFormat(rp.GetFormat())	// Copy rp CF to temp CF
					->Get(&CF);
				hr = CF.Apply(pCF, fInOurHost);	// Apply *pCF
				if(hr != NOERROR)
					return hr;
				hr = pf->Cache(&CF, &icf);		// Cache result if new, In any
				if(hr != NOERROR)				//  cause, use format index icf
					break;							
			}
			delta = rp.SetFormat(icf, cch, pf);	// Set format for this run
			if( delta == -1 )
			{
				ped->GetCallMgr()->SetOutOfMemory();
				break;
			}
			cch -= delta;
		}
		_rpCF.AdjustBackward();					// Expand scope for merging
		rp.AdjustForward();						//  runs

		rp.MergeRuns(_rpCF._iRun, pf);			// Merge adjacent runs that
												//  have the same format
		if(cchBack)								// Move _rpCF back to where it
			_rpCF.AdvanceCp(-cchBack);			//  was
		else									// Active end at range start:
			_rpCF.AdjustForward();				//  don't leave at EOR

		if (pnm)
		{
			pnm->NotifyPostReplaceRange(this, 	// Notify interested parties
				INFINITE, 0, 0, cpStart,		// of the change.
					cpStart + cchFormat - cch);
		}

		if( publdr && !(flags & IGNORESELAE))
		{
			HandleSelectionAEInfo(ped, publdr, GetCp(), _cch, GetCp(), _cch, 
					SELAE_FORCEREPLACE);
		}

		if(!_cch)								// In case IP with ApplyToWord
		{
			if(fApplyToEOP)						// Formatting EOP only
			{
				_rpCF.AdjustForward();			// Get EOP format			
				Set_iCF(_rpCF.GetFormat());		// Use it for IP
			}
			else
				Update_iFormat(-1);				
		}
		ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);
	}
	if(_fSel)
		ped->GetCallMgr()->SetSelectionChanged();
 
	AssertSz(GetCp() == (cp = _rpCF.GetCp()),
		"RTR::SetCharFormat(): incorrect format-run ptr");

	return (hr == NOERROR && cch) ? S_FALSE : hr;
}

/*
 *	CTxtRange::GetParaFormat(pPF)
 *	
 *	@mfunc
 *		return CParaFormat for this text range. If no PF runs are allocated,
 *		then return default CParaFormat
 *	
 *	@devnote
 *		This is very similar to GetCharFormat, but it's not obvious how to
 *		reduce the code size further w/o introducing virtual methods on
 *		CCharFormat and CParaFormat, which inherit from structs.  We don't
 *		want a vtable, since it would increase the storage of format classes
 */
void CTxtRange::GetParaFormat (
	CParaFormat *pPF) const		// @parm ptr to user-supplied CParaFormat to
{								//  be filled with possibly NINCH'd values
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetParaFormat");

	CTxtEdit * const ped = GetPed();

   	_TEST_INVARIANT_

	pPF->dwMask = pPF->cbSize == sizeof(PARAFORMAT2)// Default presence of
			? PFM_ALL2 : PFM_ALL;					//  all properties	

	CFormatRunPtr rp(_rpPF);
	LONG		  cch = -_cch;

	if(cch < 0)										// At end of range:
	{												//  go to start of range
		rp.AdvanceCp(cch);
		cch = -cch;									// Count with cch > 0
	}

	ped->GetParaFormat(rp.GetFormat())->Get(pPF);	// Initialize *pPF to
													//  starting paraformat
	if(!cch || !rp.IsValid())						// No cch or invalid PF
		return;										//	run ptr: use PF at
													//  this text ptr
	LONG cchChunk = rp.GetCchLeft();				// Chunk size for rp
	while(cchChunk < cch)							// NINCH properties that
	{												//  change over the range
		cch -= cchChunk;							//	given by cch
		if(!rp.NextRun())		 					// Go to next run													// No more runs
			break;									//	(cch too big)
		cchChunk = rp.GetCchLeft();
		ped->GetParaFormat(rp.GetFormat())			// NINCH properties that
			->Delta(pPF);							//  changed, i.e., reset
	}												//  corresponding bits
}

/*
 *	CTxtRange::SetParaFormat(pPF, publdr)
 *
 *	@mfunc
 *		apply CParaFormat *pPF to this range.
 *
 *	@rdesc
 *		if successfully set whole range, return NOERROR, otherwise
 *		return error code or S_FALSE.
 */
HRESULT CTxtRange::SetParaFormat (
	const CParaFormat* pPF,		// @parm CParaFormat to apply to this range
	IUndoBuilder *publdr)		// @parm the undo context for this operation
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SetParaFormat");

	LONG				cch;				// length of text to format
	LONG				cchBack;			// cch to back up for formatting
	LONG				cp = 0;				// Used for DEBUG only
	LONG				cpMin, cpMost;		// limits of text to format
	LONG				delta;
	HRESULT				hr = NOERROR;
	LONG				ipf;				// index to a CParaFormat
	CTxtEdit * const	ped = GetPed();
	CParaFormat			PF;					// Temporary CParaFormat
	IParaFormatCache *	pf;					// ptr to format caches for Cache,
											//  AddRefFormat, ReleaseFormat
											
	AssertSz(IsValidParaFormat((PARAFORMAT *)pPF),
		"RTR::SetParaFormat(): invalid CParaFormat");

 	_TEST_INVARIANT_

	if(!Check_rpPF())
		return E_FAIL;

	if(FAILED(hr = GetParaFormatCache(&pf)))
		return hr;

	FindParagraph(&cpMin, &cpMost);				// Get limits of text to
	cch = cpMost - cpMin;						//  format, namely closest

	CNotifyMgr *pnm = ped->GetNotifyMgr();
	if (pnm)
	{
		pnm->NotifyPreReplaceRange(this,		// Notify interested parties of
			INFINITE, 0, 0, cpMin,	cpMost);	// the impending update
	}

	cchBack = cpMin - GetCp();

	_rpPF.AdvanceCp(cchBack);					// Back up to formatting start
	CFormatRunPtr rp(_rpPF);					// Clone _rpPF to walk range

	if( publdr )
	{
		IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE(ped,
							rp, cch, pf, ParaFormat);
		if( pae )
			publdr->AddAntiEvent(pae);
	}
	
	BOOL fFullyDefined = FALSE;					// Default input PF not fully
												//  defined
	if (ped->HandleStyle(&PF, pPF) == NOERROR &&
		pf->Cache(&PF, &ipf) == NOERROR)
	{
		fFullyDefined = TRUE;
	}

	do
	{
		if (!fFullyDefined)						// If pPF doesn't specify
		{										//  full PF, fill in undefined
			ped->GetParaFormat(rp.GetFormat())	// Copy rp PF to temp PF
				->Get(&PF);
			hr = PF.Apply(pPF);					// Apply *pPF
			if(hr != NOERROR)					//  (Probably E_INVALIDARG)
				return hr;						// Cache result if new; in any
			hr = pf->Cache(&PF, &ipf);			//  case, get format index ipf
			if(hr != NOERROR)					// Can't necessarily return
				break;							//  error, since may need
		}										//  fixups below
		delta = rp.SetFormat(ipf, cch, pf);		// Set format for this run
		if( delta == -1 )
		{
			ped->GetCallMgr()->SetOutOfMemory();
			break;
		}
		cch -= delta;
	} while (cch > 0) ;

	_rpPF.AdjustBackward();						// If at BOR, go to prev EOR
	rp.MergeRuns(_rpPF._iRun, pf);				// Merge any adjacent runs
												//  that have the same format
	if(cchBack)									// Move _rpPF back to where it
		_rpPF.AdvanceCp(-cchBack);				//  was
	else										// Active end at range start:
		_rpPF.AdjustForward();					//  don't leave at EOR

	if (pnm)
	{
		pnm->NotifyPostReplaceRange(this,		// Notify interested parties of
			INFINITE, 0, 0, cpMin,	cpMost);	//  the update
	}

	if( publdr )
	{
		// Paraformatting works a bit differently, it just remembers the
		// current selection. Cast selection to range to avoid including
		// _select.h; we only need range methods.
		CTxtRange *psel = (CTxtRange *)GetPed()->GetSel();

		if( psel )
		{
			HandleSelectionAEInfo(ped, publdr, psel->GetCp(), 
					psel->GetCch(), psel->GetCp(), psel->GetCch(), 
					SELAE_FORCEREPLACE);
		}
	}

	ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);

	AssertSz(GetCp() == (cp = _rpPF.GetCp()),
		"RTR::SetParaFormat(): incorrect format-run ptr");

	return (hr == NOERROR && cch) ? S_FALSE : hr;
}

/*
 *	CTxtRange::SetParaStyle(pPF, publdr)
 *
 *	@mfunc
 *		apply CParaFormat *pPF using the style pPF->sStyle to this range.
 *
 *	@rdesc
 *		if successfully set whole range, return NOERROR, otherwise
 *		return error code or S_FALSE.
 *
 *	@comm
 *		If pPF->dwMask & PFM_STYLE is nonzero, this range is expanded to
 *		complete paragraphs.  If it's zero, this call just passes control
 *		to CTxtRange::SetParaStyle().
 */
 HRESULT CTxtRange::SetParaStyle (
	const CParaFormat* pPF,		// @parm CParaFormat to apply to this range
	IUndoBuilder *publdr)		// @parm Undo context for this operation
{
	LONG	cchSave = _cch;			// Save range cp and cch in case
	LONG	cpSave  = GetCp();		//  para expand needed
	HRESULT hr;
	
	if(publdr)
		publdr->StopGroupTyping();

	if(pPF->fSetStyle())
	{
		CCharFormat	CF;				// Need to apply associated CF
		CF.dwMask = CFM_STYLE;

		LONG cpMin, cpMost;
		Expander(tomParagraph, TRUE, NULL, &cpMin, &cpMost);

		CF.sStyle = pPF->sStyle;
		hr = SetCharFormat(&CF, 0, publdr);
		if(hr != NOERROR)
			return hr;
	}
	hr = SetParaFormat(pPF, publdr);
	Set(cpSave, cchSave);			// Restore this range in case expanded
	return hr;
}

/*
 *	CTxtRange::Update_iFormat(iFmtDefault)
 *	
 *	@mfunc
 *		update _iFormat to CCharFormat at current active end
 *
 *	@devnote
 *		_iFormat is only used when the range is degenerate
 *
 *		The Word 95 UI specifies that the *previous* format should
 *		be used if we're in at an ambiguous cp (i.e. where a formatting
 *		change occurs) _unless_ the previous character is an EOP
 *		marker _or_ if the previous character is protected.
 */
void CTxtRange::Update_iFormat (
	LONG iFmtDefault)		//@parm Format index to use if _rpCF isn't valid
{
	DWORD	dwEffects;
	LONG	ifmt, ifmtForward;
	const CCharFormat *pCF, *pCFForward;

	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Update_iFormat");

	if(_cch || _fDontUpdateFmt)					// _iFormat is only used
		return;									//  for degenerate ranges

	if(_rpCF.IsValid() && iFmtDefault == -1)
	{
		// Get forward info before possibly adjusting backward
		_rpCF.AdjustForward();
		ifmt = ifmtForward = _rpCF.GetFormat();
		pCF  = pCFForward  = GetPed()->GetCharFormat(ifmtForward);
		if (NULL == pCF)
		{
		    Assert(pCF);
		    return;
		}    
		    
		dwEffects = pCF->dwEffects;
		
		if( !_rpTX.IsAfterEOP() )
		{
			_rpCF.AdjustBackward();					// Adjust backward
			ifmt = _rpCF.GetFormat();
			pCF = GetPed()->GetCharFormat(ifmt);
			dwEffects = pCF->dwEffects;
		}

		if (!_rpTX.GetCp() && (pCF->bInternalEffects & CFEI_RUNISDBCS))
		{
			// If at beginning of document, and text is protected, just use
			// default format.
			ifmt = iFmtDefault;
		}
		else if((dwEffects & (CFE_PROTECTED | CFE_LINK | CFE_HIDDEN)) || 
			(pCF->bInternalEffects & CFEI_RUNISDBCS))
		{
			// If range is protected or a hyperlink, pick forward format
			_rpCF.AdjustForward();
			ifmt = _rpCF.GetFormat();
		}
		else if (pCF->bCharSet != pCFForward->bCharSet)
		{
			// If charsets don't match, and currently in IME composition mode,
			// and forward format matches keyboard, prefer forward format.
			if (GetPed()->IsIMEComposition() && W32->FormatMatchesKeyboard(pCFForward))
			{
				_rpCF.AdjustForward();
				ifmt = ifmtForward;
			}
		}
		iFmtDefault = ifmt;
	}

	// Don't allow _iFormat to include CFE_LINK or CFE_HIDDEN attributes
	// unless they're the default

	if( iFmtDefault != -1 )
	{
		pCF = GetPed()->GetCharFormat(iFmtDefault);
		if(pCF->dwEffects & (CFE_LINK | CFE_HIDDEN))
		{
			CCharFormat CF = *pCF;
			CF.cbSize = sizeof(CHARFORMAT2);
			CF.dwEffects &= ~(CFE_LINK | CFE_HIDDEN);
			CF.dwMask = CFM_ALL2;

			if(pCF->dwEffects & CFE_LINK)
			{
				// Copy default's color and underline effect
				pCF = GetPed()->GetCharFormat(-1);
				CF.crTextColor = pCF->crTextColor;
				CF.dwEffects &= ~(CFE_UNDERLINE | CFE_AUTOCOLOR);
				CF.dwEffects |= pCF->dwEffects & (CFE_UNDERLINE | CFE_AUTOCOLOR);
			}
			// This range must be an insertion point!
			Assert(_cch == 0);
			SetCharFormat(&CF, FALSE, NULL);
			return;
		}
	}
	Set_iCF(iFmtDefault);
}

/*
 *	CTxtRange::Get_iCF()
 *	
 *	@mfunc
 *		Get this range's _iFormat (AddRef'ing, of course)
 *
 *	@devnote
 *		Get_iCF() is used by the RTF reader
 */
LONG CTxtRange::Get_iCF ()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Get_iCF");

	ICharFormatCache *	pcf;
	if(FAILED(GetCharFormatCache(&pcf)))
		return -1;
	pcf->AddRefFormat(_iFormat);
	return _iFormat;
}

/*
 *	CTxtRange::Set_iCF(iFormat)
 *	
 *	@mfunc
 *		Set range's _iFormat to iFormat
 *
 *	@devnote
 *		Set_iCF() is used by the RTF reader and by Update_iFormat()
 */
void CTxtRange::Set_iCF (
	LONG iFormat)				//@parm Index of char format to use
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Set_iCF");

	if(iFormat != _iFormat)
	{
		ICharFormatCache *	pCF;

		if(FAILED(GetCharFormatCache(&pCF)))
			return;
		pCF->AddRefFormat(iFormat);
		pCF->ReleaseFormat(_iFormat);		// Note: _iFormat = -1 doesn't
		_iFormat = iFormat;					//  get AddRef'd or Release'd
	}
	AssertSz(GetCF(), "CTxtRange::Set_iCF: illegal format");
}
 
/*
 *	CTxtRange::Get_iPF()
 *	
 *	@mfunc
 *		Get paragraph format at active end
 *
 *	@devnote
 *		Get_iPF() is used by the RTF reader on encountering a start group
 */
LONG CTxtRange::Get_iPF ()
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Get_iPF");

	LONG iPF = _rpPF.GetFormat();
	IParaFormatCache *	pPF;

	if(FAILED(GetParaFormatCache(&pPF)))
		return -1;
	pPF->AddRefFormat(iPF);
	return iPF;
}

 /*
 *	CTxtRange::IsProtected(iDirection)
 *	
 *	@mfunc
 *		Return TRUE if any part of this range is protected (HACK:  or 
 *		if any part of the range contains DBCS text stored in our Unicode 
 *		backing store).  If degenerate,
 *		use CCharFormat from run specified by iDirection, that is, use run
 *		valid up to, at, or starting at this GetCp() for iDirection less, =,
 *		or greater than 0, respectively.
 *	
 *	@rdesc
 *		TRUE iff any part of this range is protected (HACK:  or if any part 
 *		of the range contains DBCS text stored in our Unicode backing store).
 */
BOOL CTxtRange::IsProtected (
	LONG iDirection)	// @parm Controls which run to check if range is IP
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::IsProtected");

	CCharFormat	cf;
	LONG		iFormat = -1;					// Default default CF

	_TEST_INVARIANT_

	if(_rpCF.IsValid())							// Active rich-text runs
	{
		if(_cch)								// Range is nondegenerate
		{
			cf.dwMask = CFM_PROTECTED;			//  of range is protected
			GetCharFormat(&cf);

			if(cf.bInternalEffects & CFEI_RUNISDBCS)
				return PROTECTED_YES;

			if(!(cf.dwMask & CFM_PROTECTED) ||
				(cf.dwEffects & CFE_PROTECTED))
			{
				return PROTECTED_ASK;
			}
			return PROTECTED_NO;
		}
		iFormat = _iFormat;						// Degenerate range: default
		if(iDirection != 0)						//  this range's iFormat
		{										// Specific run direction
			CFormatRunPtr rpCF(_rpCF);

			if(iDirection < 0)					// If at run ambiguous pos,
				rpCF.AdjustBackward();			//  use previous run

			else
				rpCF.AdjustForward();

			iFormat = rpCF.GetFormat();			// Get run format
		}								
	}
	
	const CCharFormat *pCF = GetPed()->GetCharFormat(iFormat);
	if (NULL == pCF)
	{
	    Assert(pCF);
	    return FALSE;
    }

	if(pCF->bInternalEffects & CFEI_RUNISDBCS)
		return PROTECTED_YES;

	if(pCF->dwEffects & CFE_PROTECTED)
		return PROTECTED_ASK;

	return PROTECTED_NO;
}

/*
 *	CTxtRange::AdjustEndEOP (NewChars)
 *
 *	@mfunc
 *		If this range is a selection and ends with an EOP and consists of
 *		more than just this EOP and fAdd is TRUE, or this EOP is the final
 *		EOP (at the story end), or this selection doesn't begin at the start
 *		of a paragraph, then move cpMost just before the end EOP. This
 *		function is used by UI methods that delete the selected text, such
 *		as PutChar(), Delete(), cut/paste, drag/drop.
 *
 *	@rdesc
 *		TRUE iff range end has been adjusted
 *
 *	@devnote
 *		This method leaves the active end at the selection cpMin.  It is a
 *		CTxtRange method to handle the selection when it mascarades as a
 *		range for Cut/Paste.
 */
BOOL CTxtRange::AdjustEndEOP (
	EOPADJUST NewChars)			//@parm NEWCHARS if chars will be added
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtRange::AdjustEndEOP");

	LONG cpMin, cpMost;
	LONG cch = GetRange(cpMin, cpMost);
	LONG cchSave = _cch;
	BOOL fRet = FALSE;

	if(cch && cch < GetTextLength())
	{
		LONG	cchEOP = GetPed()->Get10Mode() ? 2 : 1;
		CTxtPtr tp(_rpTX);

		if(_cch > 0)							// Ensure active end is cpMin
			FlipRange();						// (ReplaceRange() needs to
		else									//  do this anyhow)
			tp.AdvanceCp(-_cch);				// Ensure tp is at cpMost

		if(tp.IsAfterEOP() &&					// Don't delete EOP at sel 
		   (NewChars == NEWCHARS	||			//  end if there're chars to
			(cpMin && !_rpTX.IsAfterEOP() &&	//  add, or cpMin isn't at
			 cch > cchEOP)))					//  BOP and more than EOP
		{										//  is selected
			_cch -= tp.BackupCpCRLF();			// Shorten range before EOP
												// Note: the -= _adds_ to a
			Update_iFormat(-1);					//  negative _cch to make
			fRet = TRUE;						//  it less negative
		}
		if((_cch ^ cchSave) < 0 && _fSel)		// Keep active end the same
			FlipRange();						//  selection undo
	}
	return fRet;
}

/*
 *	CTxtRange::CheckTextLength(cch)
 *
 *	@mfunc
 *		Check to see if can add cch characters. If not, notify parent
 *
 *	@rdesc
 *		TRUE if OK to add cch chars
 */
BOOL CTxtRange::CheckTextLength (
	LONG cch)
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CheckTextLength");

	_TEST_INVARIANT_

	if((DWORD)(CalcTextLenNotInRange() + cch)
		 > GetPed()->TxGetMaxLength())
	{
		GetPed()->GetCallMgr()->SetMaxText();
		return FALSE;
	}
	return TRUE;
}

/*
 *	CTxtRange::FindObject(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest embedded object cpMin <lt>= range cpMin
 *		Set *pcpMost = closest embedded object cpMost <gt>= range cpMost
 *
 *	@rdesc
 *		TRUE iff object found
 *
 *	@comm
 *		An embedded object cpMin points at the first character of an embedded
 *		object. For RichEdit, this is the WCH_EMBEDDING character.  An
 *		embedded object cpMost follows the last character of an embedded
 *		object.  For RichEdit, this immediately follows the WCH_EMBEDDING
 *		character.
 */
BOOL CTxtRange::FindObject(
	LONG *pcpMin,		//@parm Out parm to receive object's cpMin;  NULL OK
	LONG *pcpMost) const//@parm Out parm to receive object's cpMost; NULL OK
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindObject");

	if(!GetObjectCount())					// No objects: can't move, so
		return FALSE;						//  return FALSE

	BOOL	bRet = FALSE;					// Default no object
	LONG	cpMin, cpMost;
	CTxtPtr tp(_rpTX);

	GetRange(cpMin, cpMost);
	if(pcpMin)
	{
		tp.SetCp(cpMin);
		if(tp.GetChar() != WCH_EMBEDDING)
		{
			cpMin = tp.FindExact(tomBackward, szEmbedding);
			if(cpMin >= 0)
			{
				bRet = TRUE;
				*pcpMin = cpMin;
			}
		}
	}
	if(pcpMost)
	{
		tp.SetCp(cpMost);
		if (tp.PrevChar() != WCH_EMBEDDING &&
			tp.FindExact(tomForward, szEmbedding) >= 0)
		{
			bRet = TRUE;
			*pcpMost = tp.GetCp();
		}
	}
	return bRet;
}

/*
 *	CTxtRange::FindCell(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest cell cpMin  <lt>= range cpMin (see comment)
 *		Set *pcpMost = closest cell cpMost <gt>= range cpMost
 *	
 *	@comment
 *		This function does nothing if the range isn't completely in a table.
 */
void CTxtRange::FindCell (
	LONG *pcpMin,			// @parm Out parm for bounding-cell cpMin
	LONG *pcpMost) const	// @parm Out parm for bounding-cell cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindCell");

	WCHAR		ch;
	LONG		cpMin, cpMost;
	CRchTxtPtr	rtp(*this);

	_TEST_INVARIANT_

	GetRange(cpMin, cpMost);

	if(pcpMin)
	{
		if(_cch > 0)
			rtp.Advance(-_cch);

		rtp._rpPF.AdjustBackward();
		if(rtp.GetPF()->wEffects & PFE_TABLE)
		{
			while(rtp.GetCp())
			{
				rtp.BackupCRLF();
				ch = rtp.GetChar();
				if(ch == CR || ch == CELL)
				{
					rtp.AdvanceCRLF();
					break;
				}
				Assert(rtp.GetPF()->wEffects & PFE_TABLE);
			}
		}
		*pcpMin = rtp.GetCp();
	}

	if(pcpMost)
	{
		rtp.SetCp(cpMost);
		if(rtp.GetPF()->wEffects & PFE_TABLE)
		{
			rtp.BackupCRLF();
			do
			{
				ch = rtp.GetChar();
				rtp.AdvanceCRLF();
				Assert(rtp.GetPF()->wEffects & PFE_TABLE);
			} while(ch && ch != CR && ch != CELL);
		}
		*pcpMost = rtp.GetCp();
	}
}

/*
 *	CTxtRange::FindParagraph(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest paragraph cpMin  <lt>= range cpMin (see comment)
 *		Set *pcpMost = closest paragraph cpMost <gt>= range cpMost
 *	
 *	@devnote
 *		If this range's cpMost follows an EOP, use it for bounding-paragraph
 *		cpMost unless 1) the range is an insertion point, and 2) pcpMin and
 *		pcpMost are both nonzero, in which case use the next EOP.  Both out
 *		parameters are nonzero if FindParagraph() is used to expand to full
 *		paragraphs (else StartOf or EndOf is all that's requested).  This
 *		behavior is consistent with the selection/IP UI.  Note that FindEOP
 *		treats the beginning/end of document (BOD/EOD) as a BOP/EOP,
 *		respectively, but IsAfterEOP() does not.
 */
void CTxtRange::FindParagraph (
	LONG *pcpMin,			// @parm Out parm for bounding-paragraph cpMin
	LONG *pcpMost) const	// @parm Out parm for bounding-paragraph cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindParagraph");

	LONG	cpMin, cpMost;
	CTxtPtr	tp(_rpTX);

	_TEST_INVARIANT_

	GetRange(cpMin, cpMost);
	if(pcpMin)
	{
		tp.SetCp(cpMin);					// tp points at this range's cpMin
		if(!tp.IsAfterEOP())				// Unless tp directly follows an
			tp.FindEOP(tomBackward);		//  EOP, search backward for EOP
		*pcpMin = cpMin = tp.GetCp();
	}

	if(pcpMost)
	{
		tp.SetCp(cpMost);					// If range cpMost doesn't follow
		if (!tp.IsAfterEOP() ||				//  an EOP or else if expanding
			(!cpMost || pcpMin) &&
			 cpMin == cpMost)				//  IP at paragraph beginning,
		{
			tp.FindEOP(tomForward);			//  search for next EOP
		}
		*pcpMost = tp.GetCp();
	}
}

/*
 *	CTxtRange::FindSentence(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest sentence cpMin  <lt>= range cpMin
 *		Set *pcpMost = closest sentence cpMost <gt>= range cpMost
 *	
 *	@devnote
 *		If this range's cpMost follows a sentence end, use it for bounding-
 *		sentence cpMost unless the range is an insertion point, in which case
 *		use the	next sentence end.  The routine takes care of aligning on
 *		sentence beginnings in the case of range ends that fall on whitespace
 *		in between sentences.
 */
void CTxtRange::FindSentence (
	LONG *pcpMin,			// @parm Out parm for bounding-sentence cpMin
	LONG *pcpMost) const	// @parm Out parm for bounding-sentence cpMost
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindSentence");

	LONG	cpMin, cpMost;
	CTxtPtr	tp(_rpTX);

	_TEST_INVARIANT_

	GetRange(cpMin, cpMost);
	if(pcpMin)								// Find sentence beginning
	{
		tp.SetCp(cpMin);					// tp points at this range's cpMin
		if(!tp.IsAtBOSentence())			// If not at beginning of sentence
			tp.FindBOSentence(tomBackward);	//  search backward for one
		*pcpMin = cpMin = tp.GetCp();
	}

	if(pcpMost)								// Find sentence end
	{										// Point tp at this range's cpLim
		tp.SetCp(cpMost);					// If cpMost isn't at sentence
		if (!tp.IsAtBOSentence() ||			//  beginning or if at story
			(!cpMost || pcpMin) &&			//  beginning or expanding
			 cpMin == cpMost)				//  IP at sentence beginning,
		{									//  find next sentence beginning
			if(!tp.FindBOSentence(tomForward))
				tp.SetCp(GetTextLength());	// End of story counts as 
		}									//  sentence end too
		*pcpMost = tp.GetCp();
	}
}

/*
 *	CTxtRange::FindVisibleRange(pcpMin, pcpMost)
 *	
 *	@mfunc
 *		Set *pcpMin  = _pdp->_cpFirstVisible
 *		Set *pcpMost = _pdp->_cpLastVisible
 *	
 *	@rdesc
 *		TRUE iff calculated cp's differ from this range's cp's
 *	
 *	@devnote
 *		CDisplay::GetFirstVisible() and GetCliVisible() return the first cp
 *		on the first visible line and the last cp on the last visible line.
 *		These won't be visible if they are scrolled off the screen.
 *		FUTURE: A more general algorithm would CpFromPoint (0,0) and
 *		(right, bottom).
 */
BOOL CTxtRange::FindVisibleRange (
	LONG *pcpMin,			// @parm Out parm for cpFirstVisible
	LONG *pcpMost) const	// @parm Out parm for cpLastVisible
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindVisibleRange");

	_TEST_INVARIANT_

	CDisplay *	pdp = GetPed()->_pdp;

	if(!pdp)
		return FALSE;

	if(pcpMin)
		*pcpMin = pdp->GetFirstVisibleCp();

	pdp->GetCliVisible(pcpMost);

	return TRUE;
}

/*
 *	CTxtRange::FindWord(pcpMin, pcpMost, type)
 *	
 *	@mfunc
 *		Set *pcpMin  = closest word cpMin  <lt>= range cpMin
 *		Set *pcpMost = closest word cpMost <gt>= range cpMost
 *
 *	@comm
 *		There are two interesting cases for finding a word.  The first,
 *		(FW_EXACT) finds the exact word, with no extraneous characters.
 *		This is useful for situations like applying formatting to a
 *		word.  The second case, FW_INCLUDE_TRAILING_WHITESPACE does the
 *		obvious thing, namely includes the whitespace up to the next word.
 *		This is useful for the selection double-click semantics and TOM.
 */
void CTxtRange::FindWord(
	LONG *pcpMin,			//@parm Out parm to receive word's cpMin; NULL OK
	LONG *pcpMost,			//@parm Out parm to receive word's cpMost; NULL OK
	FINDWORD_TYPE type) const //@parm Type of word to find
{
	TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindWord");

	LONG	cch, cch1;
	LONG	cpMin, cpMost;
	CTxtPtr	tp(_rpTX);

	_TEST_INVARIANT_

	Assert(type == FW_EXACT || type == FW_INCLUDE_TRAILING_WHITESPACE );

	GetRange(cpMin, cpMost);
	if(pcpMin)
	{
		tp.SetCp(cpMin);
		if(!tp.IsAtBOWord())							// cpMin not at BOW:
			cpMin += tp.FindWordBreak(WB_MOVEWORDLEFT);	//  go there

		*pcpMin = cpMin;

		Assert(cpMin >= 0 && cpMin <= GetTextLength());
	}

	if(pcpMost)
	{
		tp.SetCp(cpMost);
		if (!tp.IsAtBOWord() ||							// If not at word strt
			(!cpMost || pcpMin) && cpMin == cpMost)		//  or there but need
		{												//  to expand IP,
			cch = tp.FindWordBreak(WB_MOVEWORDRIGHT);	//  move to next word

			if(cch && type == FW_EXACT)					// If moved and want
			{											//  word proper, move
				cch1 = tp.FindWordBreak(WB_LEFTBREAK);	//  back to end of
				if(cch + cch1 > 0)						//  preceding word
					cch += cch1;						// Only do so if were
			}											//  not already at end
			cpMost += cch;
		}
		*pcpMost = cpMost;

		Assert(cpMost >= 0 && cpMost <= GetTextLength());
		Assert(cpMin <= cpMost);
	}
}

/*
 *	CTxtRange::CalcTextLenNotInRange()
 *	
 *	@mfunc
 *		Helper function that calculates the total length of text
 *		excluding the current range. 
 *
 *	@comm
 *		Used for limit testing. The problem being solved is that 
 *		the range can contain the final EOP which is not included
 *		in the adjusted text length.
 */
LONG CTxtRange::CalcTextLenNotInRange()
{
	LONG	cchAdjLen = GetPed()->GetAdjustedTextLength();
	LONG	cchLen = cchAdjLen - abs(_cch);
	LONG	cpMost = GetCpMost();

	if (cpMost > cchAdjLen)
	{
		// Selection extends beyond adjusted length. Put amount back in the
		// selection as it has become too small by the difference.
		cchLen += cpMost - cchAdjLen;
	}

	return cchLen;
}

/*
 *	CTxtRange::ExpandToLink()
 *
 *	@mfunc	
 *		helper function that expands this range to the bounding set of runs
 *		with CFE_LINK set
 *
 *	@devnote	FUTURE (alexgo): this should be recoded in the future to make
 *				it smaller && less demented.  Potentially, we could use a 
 *				generic "search for partial charformat" routine.
 */
void CTxtRange::ExpandToLink(void)
{
	CTxtEdit *ped = GetPed();
	LONG	cpMin, cpMost;
	const CCharFormat *pcf;

	// do the easy check first

	Expander(tomCharFormat, TRUE, NULL, &cpMin, &cpMost);

	// make cpMin be the active end
	if( _cch > 0 )
	{
		FlipRange();
	}

	SetExtend(TRUE);		

	CFormatRunPtr rp(_rpCF);

	// go backwards until we don't see any more CFE_LINK bits
	while( 1 )
	{
		rp.AdjustBackward();

		pcf = ped->GetCharFormat(rp.GetFormat());

		if( !pcf || !(pcf->dwEffects & CFE_LINK) )
		{
			break;
		}

		if( !Advance(-(LONG)rp.GetIch()) )
		{
			break;
		}
		rp.AdvanceCp(-(LONG)rp.GetIch());
	}

	// now flip the range around and go forwards until we

	FlipRange();
	rp = _rpCF;

	while( 1 )
	{
		rp.AdjustForward();

		pcf = ped->GetCharFormat(rp.GetFormat());

		if( !pcf || !(pcf->dwEffects & CFE_LINK) )
		{
			break;
		}

		if( !Advance( rp.GetRun(0)->_cch ) )
		{
			break;
		}

		rp.AdvanceCp(rp.GetRun(0)->_cch);
	}

	SetExtend(FALSE);
}

////////////////////////// Outline Support //////////////////////////////////

/*
 *	CTxtRange::Promote(lparam, publdr)
 *
 *	@mfunc
 *		Promote selected text according to:
 *
 *		LOWORD(lparam) == 0 ==> promote to body-text
 *		LOWORD(lparam) != 0 ==> promote/demote current selection by
 *								LOWORD(lparam) levels
 *	@rdesc
 *		TRUE iff promotion occurred
 *
 *	@devnote
 *		Changes this range
 */
HRESULT CTxtRange::Promote (
	LPARAM		  lparam,	//@parm 0 to body, < 0 demote, > 0 promote
	IUndoBuilder *publdr)	//@parm undo builder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtRange::Promote");

	if(abs(lparam) >= NHSTYLES)
		return E_INVALIDARG;

	if(publdr)
		publdr->StopGroupTyping();

	if(_cch > 0)							// Point at cpMin
		FlipRange();

	LONG		cchText = GetTextLength();
	LONG		cpEnd = GetCpMost();
	LONG		cpMin, cpMost;
	BOOL		fHeading = TRUE;			// Default heading in range
	HRESULT		hr;
	LONG		Level = 0;
	LONG		nHeading = NHSTYLES;		// Setup to find any heading
	CParaFormat PF;
	const CParaFormat *pPF;
	CPFRunPtr	rp(*this);
	LONG		cch = rp.FindHeading(abs(_cch), nHeading);
	WORD		wEffects;

	if(!lparam)								// Demote to subtext
	{
		if(cch)								// Already in subtext so don't
			return S_FALSE;					//  need to demote

		CTxtPtr tp(_rpTX);

		if(!tp.IsAfterEOP())
			cch = tp.FindEOP(tomBackward);
		nHeading = 1;
		if(tp.GetCp())						// Get previous level and convert
		{									//  to heading to set up
			rp.AdvanceCp(cch);				//  following Level code
			rp.AdjustBackward();
			nHeading = rp.GetOutlineLevel()/2 + 1;
		}
	}
	else if(cch == tomBackward)				// No heading in range					
	{										// Set up to promote to
		nHeading = rp.GetOutlineLevel()/2	//  heading
				 + (lparam > 0 ? 2 : 1);
		fHeading = FALSE;					// Signal no heading in range
	}
	else if(cch)							// Range starts in subtext
	{
		SetExtend(TRUE);
		Advance(cch);						// Bypass initial nonheading
	}

	Level = 2*(nHeading - 1);				// Heading level
	PF.bOutlineLevel = Level | 1;			// Corresponding subtext level

	if (!Level && lparam > 0 ||				// Can't promote Heading 1
		nHeading == NHSTYLES && lparam < 0)	//  or demote Heading 9
	{										
		return S_FALSE;
	}
	do									
	{
		_cch = 0;
		Level -= 2*lparam;					// Promote Level
		pPF = GetPF();
		wEffects = pPF->wEffects;
		if(pPF->bOutlineLevel & 1)			// Handle contiguous text in
		{									//  one fell swoop
			cch = fHeading ? _rpPF.GetCchLeft() : cpEnd - GetCp(); 
			if(cch > 0)
			{
				SetExtend(TRUE);
				Advance(cch);
			}
		}
		Expander(tomParagraph, TRUE, NULL, &cpMin, &cpMost);

		if((unsigned)Level < 2*NHSTYLES)
		{									// Promoted Level is valid
			PF.dwMask = PFM_OUTLINELEVEL;	// Default setting subtext level
			if(!(Level & 1) && lparam)		// Promoting or demoting heading
			{								// Preserve collapse status
				PF.wEffects = Level ? wEffects : 0; // H1 is aways expanded		
				PF.sStyle = -Level/2 + STYLE_HEADING_1;
				PF.dwMask = PFM_STYLE + PFM_COLLAPSED;
				PF.bOutlineLevel = Level | 1;// Set up subtext
			}
			else if(!lparam)				// Changing heading to subtext
			{								//  or uncollapsing subtext
				PF.wEffects = 0;			// Turn off collapsed
				PF.sStyle = STYLE_NORMAL;
				PF.dwMask = PFM_STYLE + PFM_OUTLINELEVEL + PFM_COLLAPSED;
			}
			hr = SetParaStyle(&PF, publdr);
			if(hr != NOERROR)
				return hr;
		}
		if(GetCp() >= cchText)				// Have handled last PF run
			break;
		Assert(_cch > 0);					// Para/run should be selected
		pPF = GetPF();						// Points at next para
        Assert(pPF);
		if (pPF)
		{
		    Level = pPF->bOutlineLevel;
		}    
	}										// Iterate until past range &
	while((Level & 1) || fHeading &&		// any subtext that follows
		  (GetCp() < cpEnd || pPF->wEffects & PFE_COLLAPSED));

	return NOERROR;
}

/*
 *	CTxtRange::ExpandOutline(Level, fWholeDocument)
 *
 *	@mfunc
 *		Expand outline according to Level and fWholeDocument. Wraps
 *		OutlineExpander() helper function and updates selection/view
 *
 *	@rdesc
 *		NOERROR if success
 */
HRESULT CTxtRange::ExpandOutline(
	LONG Level,				//@parm If < 0, collapse; else expand, etc.
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
	BOOL fWholeDocument,	//@parm If TRUE, whole document
	IUndoBuilder *publdr)
#else
	BOOL fWholeDocument)	//@parm If TRUE, whole document
#endif // PWD_JUPITER
{
	if (!IsInOutlineView())
		return NOERROR;

#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
	HRESULT hres = OutlineExpander(Level, fWholeDocument, publdr);
#else
	HRESULT hres = OutlineExpander(Level, fWholeDocument);
#endif // PWD_JUPITER
	if(hres != NOERROR)
		return hres;

#ifdef PWD_JUPITER
	// GuyBark Jupiter 31960:
	// Notify PWord that the user has expanded or collapsed a paragraph.
	// But do not do this if PWord itself has told RichEdit to perform
	// the expansion/collapse. We can reliably tell this by whether
	// fWholeDocument is true, as PWord expansions normally apply to the
	// whole document. If not then it's a PWord localized expansion or
	// collapse, in which we want the level combo to be cleared.
	if(!fWholeDocument)
#endif // PWD_JUPITER
	GetPed()->TxNotify(EN_PARAGRAPHEXPANDED, NULL);
	return GetPed()->UpdateOutline();
}

/*
 *	CTxtRange::OutlineExpander(Level, fWholeDocument)
 *
 *	@mfunc	
 *		Expand/collapse outline for this range according to Level
 *		and fWholeDocument.  If fWholeDocument is TRUE, then
 *		1 <= Level <= NHSTYLES collapses all headings with numbers
 *		greater than Level and collapses all nonheadings. Level = -1
 *		expands all.
 *
 *		fWholeDocument = FALSE expands/collapses (Level > 0 or < 0)
 *		paragraphs depending on whether an EOP and heading are included
 *		in the range.  If Level = 0, toggle heading's collapsed status.
 *
 *	@rdesc
 *		(change made) ? NOERROR : S_FALSE
 */
HRESULT CTxtRange::OutlineExpander(
	LONG Level,				//@parm If < 0, collapse; else expand, etc.
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
	BOOL fWholeDocument,	//@parm If TRUE, whole document
	IUndoBuilder *publdr)
#else
	BOOL fWholeDocument)	//@parm If TRUE, whole document
#endif // PWD_JUPITER
{
	CParaFormat PF;
	PF.dwMask = PFM_STYLE;

#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
	if(publdr)
		publdr->StopGroupTyping();
#endif // PWD_JUPITER

    if(fWholeDocument)							// Apply to whole document
	{
        if (IN_RANGE(1, Level, NHSTYLES) ||		// Collapse to heading
	        Level == -1)						// -1 means all
		{
			Set(0, tomBackward);				// Select whole document
			PF.sStyle = STYLE_COMMAND + (BYTE)Level;
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
			SetParaFormat(&PF, publdr);			// No undo
#else
			SetParaFormat(&PF, NULL);			// No undo
#endif // PWD_JUPITER
			return NOERROR;
		}
		return S_FALSE;							// Nothing happened (illegal
	}											//  arg)

	// Expand/Collapse for Level positive/negative, respectively

	LONG cpMin, cpMost;							// Get range cp's
	LONG cchMax = GetRange(cpMin, cpMost);
	if(_cch > 0)								// Ensure cpMin is active
		FlipRange();							//  for upcoming rp and tp

	LONG	  nHeading = NHSTYLES;				// Setup to find any heading
	LONG	  nHeading1;
	CTxtEdit *ped = GetPed();
	CPFRunPtr rp(*this);
	LONG	  cch = rp.FindHeading(cchMax, nHeading);

	if(cch == tomBackward)						// No heading found within range
		return S_FALSE;							// Do nothing

	Assert(cch <= cchMax && (Level || !cch));	// cch is count up to heading
	CTxtPtr tp(_rpTX);
	cpMin += cch;								// Bypass any nonheading text
	tp.AdvanceCp(cch);							//  at start of range

	// If toggle collapse or if range contains an EOP,
	// collapse/expand all subordinates
	cch = tp.FindEOP(tomForward);				// Find next para
	if(!cch)
		return NOERROR;

    if(!Level || cch < -_cch)					// Level = 0 or EOP in range
	{
		if(!Level)								// Toggle collapse status
		{
			LONG cchLeft = rp.GetCchLeft();
			if (cch < cchLeft || !rp.NextRun() ||
				nHeading == STYLE_HEADING_1 - rp.GetStyle() + 1)
			{
				return NOERROR;					// Next para has same heading
			}
			Assert(cch == cchLeft);
			Level = rp.IsCollapsed();
			rp.AdvanceCp(-cchLeft);
		}
		PF.dwMask = PFM_COLLAPSED;
		PF.wEffects = Level > 0 ? 0 : PFE_COLLAPSED;
		while(cpMin < cpMost)
		{										// We're at a heading
			tp.SetCp(cpMin);
			cch = tp.FindEOP(-_cch);
			cpMin += cch;						// Bypass it		
			if(!rp.AdvanceCp(cch))				// Point at next para
				break;							// No more, we're done
			nHeading1 = nHeading;				// Setup to find heading <= nHeading
			cch = rp.FindHeading(tomForward, nHeading1);
			if(cch == tomBackward)				// No more higher headings
				cch = GetTextLength() - cpMin;	// Format to end of text
			Set(cpMin, -cch);					// Collapse/expand up to here
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
			SetParaFormat(&PF, publdr);
#else
			SetParaFormat(&PF, NULL);
#endif // PWD_JUPITER
			cpMin += cch;						// Move past formatted area
			nHeading = nHeading1;				// Update nHeading to possibly
		}										//  lower heading #
		return NOERROR;
	}

	// Range contains no EOP: expand/collapse deepest level.
	// If collapsing, collapse all nonheading text too. Expand
	// nonheading text only if all subordinate levels are expanded.
	BOOL	fCollapsed;
	LONG	nHeadStart, nHeadDeepNC, nHeadDeep;
	LONG	nNonHead = -1;						// No nonHeading found yet
	const CParaFormat *pPF;

	cpMin = tp.GetCp();							// Point at start of 
	cpMost = cpMin;								//  next para
	pPF = ped->GetParaFormat(_rpPF.GetFormat());
	nHeading = pPF->bOutlineLevel;

	Assert(!(nHeading & 1) &&					// Must start with a heading
		!(pPF->wEffects & PFE_COLLAPSED));		//  that isn't collapsed

	nHeadStart = nHeading/2 + 1;				// Convert outline level to
	nHeadDeep = nHeadDeepNC = nHeadStart;		//  heading number

	while(cch)									// Determine deepest heading
	{											//  and deepest collapsed
		rp.AdvanceCp(cch);						//  heading
		pPF = ped->GetParaFormat(rp.GetFormat());
		fCollapsed = pPF->wEffects & PFE_COLLAPSED;
		nHeading = pPF->bOutlineLevel;
		if(nHeading & 1)						// Text found
		{										// Set nNonHead > 0 if
			nNonHead = fCollapsed;				//  collapsed; else 0
			cch = rp.GetCchLeft();				// Zip to end of contiguous
			tp.AdvanceCp(cch);					//  text paras
		}										
		else									// It's a heading
		{
			nHeading = nHeading/2 + 1;			// Convert to heading number
			if(nHeading <= nHeadStart)			// If same or shallower as
				break;							//  start heading we're done

			// Update deepest and deepest nonCollapsed heading #'s
			nHeadDeep = max(nHeadDeep, nHeading);
			if(!fCollapsed)						 
				nHeadDeepNC = max(nHeadDeepNC, nHeading); 
			cch = tp.FindEOP(tomForward);		// Go to next paragraph
		}				
		cpMost = tp.GetCp();					// Include up to it
	}

	PF.sStyle = STYLE_COMMAND + nHeadDeepNC;
	if(Level > 0)								// Expand
	{
		if(nHeadDeepNC < nHeadDeep)				// At least one collapsed
			PF.sStyle++;						//  heading: expand shallowest
		else									// All heads expanded: do others
			PF.sStyle = (unsigned short) (STYLE_COMMAND + 0xFF);
	}											// In any case, expand nonheading
	else if(nNonHead)							// Collapse. If text collapsed
	{											//  or missing, do headings
		if(nHeadDeepNC == nHeadStart)
			return S_FALSE;						// Everything already collapsed
		PF.sStyle--;							// Collapse to next shallower
	}											//  heading

	Set(cpMin, cpMin - cpMost);					// Select range to change
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
	SetParaFormat(&PF, publdr);					// No undo
#else
	SetParaFormat(&PF, NULL);					// No undo
#endif // PWD_JUPITER
	return NOERROR;
}

/*
 *	CTxtRange::CheckOutlineLevel(publdr)
 *
 *	@mfunc	
 *		If the paragraph style at this range isn't a heading, make
 *		sure its outline level is compatible with the preceeding one
 */
void CTxtRange::CheckOutlineLevel(
	IUndoBuilder *publdr)		// @parm undo context for this operation
{
	LONG	  LevelBackward, LevelForward;
	CPFRunPtr rp(*this);

	Assert(!_cch);

	rp.AdjustBackward();
	LevelBackward = rp.GetOutlineLevel() | 1;	// Nonheading level corresponding
												//  to previous PF run
	rp.AdjustForward();
	LevelForward = rp.GetOutlineLevel();

	if (!(LevelForward & 1) || 					// Any heading can follow
		LevelForward == LevelBackward)			//  any style. Also if
	{											//  forward level is correct,
		return;									//  return
	}

	LONG		cch;							// One or more nonheadings
	LONG		lHeading = NHSTYLES;			//  with incorrect outline
	CParaFormat PF;								//  levels follow

	PF.dwMask = PFM_OUTLINELEVEL;				// Setup to adjust outline
	PF.bOutlineLevel = LevelBackward;			//  level

	cch = rp.FindHeading(tomForward, lHeading);	// Find next heading
	if(cch == tomBackward)
		cch = tomForward;

	Set(GetCp(), -cch);							// Select all nonheading text
	SetParaFormat(&PF, publdr);					// Change its outline level
	Set(GetCp(), 0);							// Restore range to IP
}

