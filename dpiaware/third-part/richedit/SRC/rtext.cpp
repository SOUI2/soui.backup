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
 *	@module RTEXT.C - Rich-text ptr class |
 *
 *		This text ptr consists of a plain text ptr (_rpTX), a char-format
 *		run ptr (_rpCF), and a para-format run ptr (_rpPF). This module
 *		contains the methods to manipulate this combination of run ptrs
 *		consistently.
 *	
 *	Authors:<nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Main implementation: Murray Sargent <nl>
 *		Undo and notification implementations: Alex Gounares <nl>
 *
 */

#include "_common.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_rtext.h"
#include "_disp.h"
#include "_select.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_objmgr.h"

ASSERTDATA

#define DEBUG_CLASSNAME CRchTxtPtr
#include "_invar.h"

#ifdef DEBUG
/*
 *	CRchTxtPtr::Invariant
 */
BOOL CRchTxtPtr::Invariant( void ) const
{
	unsigned ch;
	DWORD cch;
	DWORD cchLength = GetTextLength();
	LONG cp;

#ifdef EXTREME_CHECKING
	const TCHAR *pch;
	const TCHAR *pchReverse;
	LONG cchvalid;
#endif // EXTREME_CHECKING
	
	_rpTX.Invariant();
	_rpCF.Invariant();
	_rpPF.Invariant();

	if( _rpCF.IsValid() )
	{
		// BEGIN tone down of assert for IME Rich composition.
		CFormatRun *pRun = _rpCF.Elem(0);

		if ( !IsIMERich() || !pRun || pRun->_iFormat != -1 || _rpCF.Count() != 1 )
		// END tone down
		{
			Assert(GetCp()   == (cp  = _rpCF.GetCp()));
			Assert(cchLength == (cch = _rpCF.GetCch()));
		}
	}

	if( _rpPF.IsValid() )
	{
		Assert(GetCp()   == (cp  = _rpPF.GetCp()));
		Assert(cchLength == (cch = _rpPF.GetCch()));

		CTxtPtr	tp(_rpTX);

		tp.AdvanceCp(_rpPF.GetCchLeft() - 1);
		ch = tp.GetChar();
		if(!IsASCIIEOP(ch))
		{
			_rpTX.MoveGapToEndOfBlock();			// Make it easier to see
			AssertSz(FALSE,							//  what's going on
				"CRchTxtPtr::Invariant: PF run doesn't end with EOP");
		}

#ifdef EXTREME_CHECKING
		// we don't do this check normally as it is _extremely_ slow.
		// However, it's very useful for catching para-format run problems

		// now make sure that each para format run ends on a paragraph mark!
		CFormatRunPtr	rppf(_rpPF);

		rppf.BindToCp(0);
		tp.BindToCp(0);

		do
		{
			tp.SetCp(rppf.GetCp() + rppf.GetRun(0)->_cch);
			if( !tp.IsAfterEOP() )
			{
				cp = tp.GetCp();

				pch = tp.GetPch(cchvalid);
				pchReverse = tp.GetPchReverse(cchvalid);
				pchReverse -= cchvalid;
		
				AssertSz(0, "ParaFormat Run not aligned along paragraphs!");
			}
		} while( rppf.NextRun() );
#endif // EXTREME_CHECKING
	}

	return TRUE;
}

#endif  // DEBUG

//======================= CRchTxtPtr constructors ========================================

CRchTxtPtr::CRchTxtPtr(CTxtEdit *ped) :
	_rpTX(ped, 0), _rpCF(NULL),	_rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs(0);
}

CRchTxtPtr::CRchTxtPtr(CTxtEdit *ped, DWORD cp) :
	_rpTX(ped, cp), _rpCF(NULL),	_rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs(cp);
}

CRchTxtPtr::CRchTxtPtr (const CRchTxtPtr& rtp) :
	_rpTX(rtp._rpTX), _rpCF(NULL), _rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs(rtp.GetCp());
}

CRchTxtPtr::CRchTxtPtr (const CDisplay * pdp) :
	_rpTX(pdp->GetED(), 0), _rpCF(NULL), _rpPF(NULL)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::CRchTxtPtr");

	InitRunPtrs(0);
}

/*
 *	CRchTxtPtr::Advance(cch)
 *	
 *	@mfunc
 *		Move this rich-text ptr forward <p cch> characters.  If <p cch>
 *		<lt> 0, move backward by -<p cch> characters.
 *	
 *	@rdesc
 *		cch actually moved
 *
 */
LONG CRchTxtPtr::Advance(
	LONG cch)			// @parm count of characters to move - may be <lt> 0
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Advance");

	if( cch != 0 )
	{
		cch = _rpTX.AdvanceCp(cch);
		_rpCF.AdvanceCp(cch);
		_rpPF.AdvanceCp(cch);
	}

	_TEST_INVARIANT_

	return cch;
}

/*
 *  CRchTxtPtr::AdvanceCRLF()
 *
 *  @mfunc
 *      Advance this text ptr one char, treating CRLF as a single char.
 *
 *  @rdesc
 *      cch actually moved
 */
LONG CRchTxtPtr::AdvanceCRLF()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CRchTxtPtr::AdvanceCRLF");

    LONG cch = _rpTX.AdvanceCpCRLF();
    _rpPF.AdvanceCp(cch);
    _rpCF.AdvanceCp(cch);
    return cch;
}

/*
 *  CRchTxtPtr::BackupCRLF()
 *
 *  @mfunc
 *      Backup this text ptr one char, treating CRLF as a single char.
 *
 *  @rdesc
 *      cch actually moved
 */
LONG CRchTxtPtr::BackupCRLF()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CRchTxtPtr::BackupCRLF");

    LONG cch = _rpTX.BackupCpCRLF();
    _rpPF.AdvanceCp(cch);
    _rpCF.AdvanceCp(cch);
    return cch;
}

/*
 * CRchTxtPtr::ValidateCp(&cp)
 *
 *	@mfunc
 *		If <p cp> <lt> 0, set it to 0; if it's <gt> text length, set it to
 *		text length.
 */
void CRchTxtPtr::ValidateCp(
	LONG &cp) const			// @parm new cp for this text ptr
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ValidateCp");

	LONG cchT = GetTextLength();

	cp = min(cp, cchT);				// Be sure cp is valid
	cp = max(cp, 0);
}

/*
 * CRchTxtPtr::SetCp(cp)
 *
 *	@mfunc
 *		Set this rich text ptr's cp to cp
 */
DWORD CRchTxtPtr::SetCp(
	DWORD cp)			// @parm new cp for this text ptr
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::SetCp");

	CRchTxtPtr::Advance(cp - GetCp());
	return GetCp();
}

/*	CRchTxtPtr::GetIchRunXX() and CRchTxtPtr::GetCchRunXX()
 *
 *	@mfunc
 *		Text-run management to retrieve current text run cch and offset
 *
 *	@rdesc
 *		current run ich or cch
 *
 *	@devnote
 *		Use of queries like _rpCF.IsValid() instead of an inclusive fRich
 *		allows rich-text formatting to be applied per rich-text category,
 *		e.g., CHARFORMATs, but not necessarily PARAFORMATs.  If the rp isn't
 *		valid, _cp is used for ich and the document length is used for cch,
 *		i.e., the values for a document describable by a single plain-text run
 */
LONG CRchTxtPtr::GetIchRunCF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetIchRunCF");

	return _rpCF.IsValid() ? _rpCF.GetIch() : GetCp();
}

LONG CRchTxtPtr::GetCchRunCF() 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCchRunCF");

	return _rpCF.IsValid() ? _rpCF.GetRun(0)->_cch : GetTextLength();
}

/*	CRchTxtPtr::GetCchLeftRunCF() / GetCchLeftRunPF()
 *
 *	@mfunc
 *		Return cch left in run, i.e., cchRun - ich
 *
 *	@rdesc
 *		cch left in run
 */
LONG CRchTxtPtr::GetCchLeftRunCF() 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCchLeftRunCF");

	return _rpCF.IsValid()
		? _rpCF.GetCchLeft() : GetTextLength() - GetCp();
}

LONG CRchTxtPtr::GetCchLeftRunPF() 
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCchLeftRunPF");

	return _rpPF.IsValid()
		? _rpPF.GetCchLeft() : GetTextLength() - GetCp();
}

/*
 *	CRchTxtPtr::FindText(cpMost, dwFlags, pch, cchToFind)
 *	
 *	@mfunc
 *		Find text in a range starting at this text pointer;
 *		if found, moves this text pointer to that position.
 *	
 *	@rdesc
 *		character position of first match
 *		<lt> 0 if no match
 *
 *	@devnote
 *		Would be easy to match a single format (like Word 6) provided
 *		cchToFind is nonzero.  Else need to search runs (also pretty easy).
 *		For format-sensitive searches, might be easier to search for matching
 *		format run first and then within that run search for text.
 */
LONG CRchTxtPtr::FindText (
	LONG		cpMost,		// @parm Limit of search; <lt> 0 for end of text
	DWORD		dwFlags,	// @parm FR_MATCHCASE	case must match
							//		 FR_WHOLEWORD	match must be a whole word
	TCHAR const *pch,		// @parm Text to search for
	LONG		cchToFind)	// @parm Length of text to search for
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::FindText");

	_TEST_INVARIANT_

	LONG cpSave = GetCp();
	LONG cpMatch = _rpTX.FindText(cpMost, dwFlags, pch, cchToFind);

	if(cpMatch >= 0)					// cpMatch = -1 means "not found"
		SetRunPtrs(GetCp(), cpSave);	
	
			// possible code for format-dependent Finds

	return cpMatch;
}

/*
 *	CRchTxtPtr::GetCF()/GetPF()
 *	
 *	@mfunc
 *		Return ptr to CCharFormat/CParaFormat at this text ptr. If no CF/PF runs
 *		are allocated, then return ptr to default format
 *	
 *	@rdesc
 *		Ptr to CCharFormat/CParaFormat at this text ptr
 */
const CCharFormat* CRchTxtPtr::GetCF() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetCF");

	return GetPed()->GetCharFormat(_rpCF.GetFormat());
}

const CParaFormat* CRchTxtPtr::GetPF() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::GetPF");

	return GetPed()->GetParaFormat(_rpPF.GetFormat());
}

/*
 *	CRchTxtPtr::ReplaceRange(cchOld, cchNew, *pch, pcpFirstRecalc, publdr,
 *							 iFormat)
 *	@mfunc
 *		Replace a range of text at this text pointer using CCharFormat iFormat
 *		and updating other text runs as needed
 *	
 *	@rdesc
 *		Count of new characters added
 *	
 *	@devnote
 *		Moves this text pointer to end of replaced text.
 *		May move text block and formatting arrays.
 */
LONG CRchTxtPtr::ReplaceRange(
	LONG		cchOld,		// @parm length of range to replace
							//		(<lt> 0 means to end of text)
	LONG		cchNew,		// @parm length of replacement text
	TCHAR const *pch,		// @parm replacement text
	IUndoBuilder *publdr,	// @parm CCharFormat iFormat to use for cchNew
	LONG		iFormat)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ReplaceRange");

	LONG		  cch;
	LONG		  cchEndEOP = 0;				// Default 0 final EOP fixup
	LONG		  cchMove = 0;					// Default nothing to move
	LONG		  cchNextEOP = cchOld;			// cch to next EOP
	LONG		  cchPrevEOP = 0;				// cch back to previous EOP
	LONG		  cpFR;							//  between PF runs
	LONG 		  cpSave = GetCp();
	LONG		  cpFormatMin = cpSave;			// Used for notifications
	LONG		  cpFormat = cpSave;			// Will add cchOld, maybe cchMove
	IAntiEvent *  paeCF = NULL;
	IAntiEvent *  paePF = NULL;
	CNotifyMgr *  pnm;
	CObjectMgr *  pobjmgr;

 	_TEST_INVARIANT_

	LONG cchEnd = GetTextLength() - GetCp();
	if(cchOld < 0)
		cchOld = cchEnd;

  	if(IsRich() && cchOld == cchEnd)			// Attempting to delete up
	{											//  thru final EOP
		cchEndEOP = (GetPed()->Get10Mode())		// Calc cch of final EOP
				  ? CCH_EOD_10 : CCH_EOD_20;

		if(cchEndEOP <= cchOld)					// Don't delete it unless
			cchOld -= cchEndEOP;				//  converting from 2.0
	}
	else if(_rpPF.IsValid() && cchOld)			// If PARAFORMATs are enabled,
	{											//  get tp and rp at end of 
		CFormatRunPtr rp(_rpPF);				//  range. Need bounding para
		CTxtPtr 	  tp(_rpTX);				//  counts to save valid PF
		BOOL		  fIsAtBOP;					//  for undo

		tp.AdvanceCp(cchOld);
		rp.AdvanceCp(cchOld);

		cch = 0;
		if(tp.IsAfterEOP())						// Range ends with an EOP:
		{										//  get EOP length by
			cch = -tp.BackupCpCRLF();			//  backing up over it
			tp.AdvanceCp(cch);					// Advance past EOP
		}
		cchNextEOP = tp.FindEOP(tomForward);	// Get cch up to next EOP

		fIsAtBOP = !GetCp() || _rpTX.IsAfterEOP();
		if (!fIsAtBOP && cch == cchOld &&		// Deleting EOP alone before
			!rp.GetIch())						//  new PARAFORMAT run start
		{										//  in para with more than EOP
			cchMove = cchNextEOP;				// Need to move chars up to
			cpFormat += cchMove;				//  end of next para for
		}
		
		cchNextEOP += cchOld;					// Count from GetCp() to EOP
			
		tp.SetCp(GetCp());						// Back to this ptr's _cp
		if(!fIsAtBOP)
			cchPrevEOP = tp.FindEOP(tomBackward);// Get cch to start of para

		// If deleting from within one format run up to or into another, set
		// up to move last para in starting format run into the run following
		// the deleted text
		if(rp.GetFormat() != _rpPF.GetFormat()	// Change of format during  
			&& !fIsAtBOP && !cchMove)			//  deleted text not starting
		{										//  at BOP
			cchMove = cchPrevEOP;				// Get cch to start of para
			cpFormatMin += cchMove;				//  in this ptr's run for
		}										//  moving into rp's run
	}
	
	Assert(cchNew >= 0 && cchOld >= 0);
	if(!(cchNew + cchOld))						// Nothing to do (note: all
		return 0;								//  these cch's are >= 0)

	// Handle pre-replace range notifications.  This method is very
	// useful for delayed rendering of data copied to the clipboard.

	pnm = GetPed()->GetNotifyMgr();
	if( pnm )
	{
		pnm->NotifyPreReplaceRange((ITxNotify *)this, cpSave, cchOld,
			cchNew, cpFormatMin, cpFormat + cchOld);
	}

	if(iFormat >= 0)
		Check_rpCF();

	// Get rid of objects first.  This let's us guarantee that when we
	// insert the objects as part of an undo, the objects themselves are
	// restored _after_ their corresponding WCH_EMBEDDINGs have been
	// added to the backing store.

	if(GetObjectCount())
	{
		pobjmgr = GetPed()->GetObjectMgr();

		if (NULL == pobjmgr)
		{
		    TRACEWARNSZ("GetObjectMgr failed");
		    Assert(pobjmgr);         //if we Got an ObjectCount, how did we not get an ObjectMgr?
		    return 0;  //nothing we can do
	    }
	    

		pobjmgr->ReplaceRange(cpSave, cchOld, publdr);
	}

	if(IsRich() || IsIMERich())				// Rich text enabled
	{
		// The anti-events used below are a bit tricky (paeCF && paePF).
		// Essentially, this call, CRchTxtPtr::ReplaceRange generates one
		// 'combo' anti-event composed of up to two formatting AE's plus
		// the text anti-event.  These anti-events are combined together
		// to prevent ordering problems during undo/redo.
		cpFR = ReplaceRangeFormatting(cchOld + cchEndEOP, cchNew + cchEndEOP, 
			iFormat, publdr, &paeCF, &paePF, cchMove, cchPrevEOP, cchNextEOP);

		if (cchEndEOP)
		{
			// If we added in the EOP we need to back up by the EOP so
			// that the invariants don't get annoyed and the richtext object
			// doesn't get out of sync.
			_rpCF.AdvanceCp(-cchEndEOP);
			_rpPF.AdvanceCp(-cchEndEOP);
		}
				
		if(cpFR < 0)
		{
			Tracef(TRCSEVERR, "ReplaceRangeFormatting(%ld, %ld, %ld) failed", GetCp(), cchOld, cchNew);
			cch = 0;
			goto Exit;
		}
	}

	// As noted above in the call to ReplaceRangeFormatting, the anti-events
	// paeCF and paePF, if non-NULL, were generated by ReplaceRangeFormatting.
	// In order to solve ordering problems, the anti-event generated by this
	// method is actually a combo anti-event of text && formatting AE's.
	cch = _rpTX.ReplaceRange(cchOld, cchNew, pch, publdr, paeCF, paePF);
	if (cch != cchNew)
	{
		Tracef(TRCSEVERR, "_rpTX.ReplaceRange(%ld, %ld, ...) failed", cchOld, cchNew);

#ifndef NODUMPFORMATRUNS
		// Boy, out of memory or something bad.  Dump our formatting and hope
		// for the best.  
		//
		// FUTURE: (alexgo) degrade more gracefully than loosing formatting
		// info.

		// Notify every interested party that they should dump their formatting
		if( pnm )
		{
			pnm->NotifyPreReplaceRange(NULL, CONVERT_TO_PLAIN, 0, 0, 0, 0);
		}

		// Tell document to dump its format runs
		GetPed()->GetTxtStory()->DeleteFormatRuns();
#endif
		goto Exit;
	}

	AssertSz(!_rpPF.IsValid() || _rpPF.GetIch() || !GetCp() || _rpTX.IsAfterEOP(),
		"CRchTxtPtr::ReplaceRange: EOP not at end of PF run");
			
	// BUGBUG!! (alexgo) doesn't handle correctly the case where things fail
	// (due to out of memory or whatever).  See also notes in CTxtPtr::HandleReplaceRange
	// Undo.  The assert below is therefore somewhat bogus, but if it fires, 
	// then our floating ranges are going to be in trouble until we fix
	// up the logic here.

	Assert(cch == cchNew);

Exit:

#ifdef DEBUG
	// test the invariant again before calling out to replace range notification;
	// in this way, we can catch bugs earlier. The invariant has its own
	// scope for convenience.
	if( 1 )
	{
		_TEST_INVARIANT_
	}
#endif

	if( pnm )
	{
		pnm->NotifyPostReplaceRange((ITxNotify *)this, cpSave, cchOld, cch,
			cpFormatMin, cpFormat + cchOld);
	}

	GetPed()->GetCallMgr()->SetChangeEvent(CN_TEXTCHANGED);

	return cch;
}

/*
 *	CRchTxtPtr::InitRunPtrs(cp)
 *
 *	@mfunc
 *		Initialize Run Ptrs of this rich-text ptr to correspond to
 *		document given by ped and to cp given by cp.
 */
void CRchTxtPtr::InitRunPtrs(
	LONG cp)			// @parm character position to move RunPtrs to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::InitRunPtrs");

	CTxtStory *pStory;

	AssertSz(GetPed(), "RTP::InitRunPtrs: illegal GetPed()");

	if( IsRich() || IsIMERich() )						
	{
		pStory = GetPed()->GetTxtStory();
		if(pStory->_pCFRuns)						//  and there's RichData,
		{											//  initialize format-run ptrs
			_rpCF.SetRunArray((CRunArray *)pStory->_pCFRuns);
			_rpCF.BindToCp(cp);
		}
		if (pStory->_pPFRuns)
		{
			_rpPF.SetRunArray((CRunArray *)pStory->_pPFRuns);
			_rpPF.BindToCp(cp);
		}
	}
 
}

/*
 *	CRchTxtPtr::SetRunPtrs(cp, cpFrom)
 *
 *	@mfunc set Run Ptrs of this rich-text ptr to correspond to cp
 *
 *	@rdesc
 *			TRUE unless cp is outside of doc (in which case RunPtrs are
 *			set to nearest document end).
 */
BOOL CRchTxtPtr::SetRunPtrs(
	LONG cp,				// @parm character position to move RunPtrs to
	LONG cpFrom)			// @parm cp to start with
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::SetRunPtrs");

	BOOL fRet = TRUE;

	if(cpFrom && 2*cp >= cpFrom)
	{
		if (_rpCF.IsValid())
			fRet = _rpCF.AdvanceCp(cp - cpFrom);

		if(_rpPF.IsValid())
			fRet = _rpPF.AdvanceCp(cp - cpFrom);
		return fRet;
	}
		
#ifdef LATER
	if(_rpOB.IsValid())
		fRet = _rpOB.RpSetCp(cp);

	if(_rpRTF.IsValid())
		fRet = _rpRTF.RpSetCp(cp);
#endif

	if(_rpCF.IsValid())
		fRet = _rpCF.BindToCp(cp);

	if(_rpPF.IsValid())
		fRet = _rpPF.BindToCp(cp);

	return fRet;
}

/*
 *	CRchTxtPtr::ReplaceRangeFormatting(cchOld, cchNew, iFormat, publdr,
 *									   ppaeCF, ppaePF, cchMove)
 *	@mfunc
 *		replace character and paragraph formatting at this text pointer
 *		using CCharFormat with index iFormat
 *	
 *	@rdesc
 *		count of new characters added
 *	
 *	@devnote
 *		moves _rpCF and _rpPF to end of replaced text
 *		moves format run arrays
 *		CCharFormat for iFormat is fully configured, i.e., no NINCHes
 */
INLINE LONG CRchTxtPtr::ReplaceRangeFormatting(
	LONG		cchOld,		//@parm length of range to replace
	LONG		cchNew,		//@parm length of replacement text
	LONG		iFormat,	//@parm Char format to use
	IUndoBuilder *publdr,	//@parm UndoBuilder to receive antievents
	IAntiEvent **ppaeCF,	//@parm where to return 'extra' CF anti-events
	IAntiEvent **ppaePF,	//@parm where to return extra PF anti-events
	LONG		cchMove,	//@parm cch to move between PF runs
	LONG		cchPrevEOP,	//@parm cch from _cp back to prev EOP
	LONG		cchNextEOP)	//@parm cch from _cp up to next EOP
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ReplaceRangeFormatting");

	LONG				cp = GetCp();				
	ICharFormatCache *	pcfc;
	IParaFormatCache *	ppfc;
	DWORD iRunMerge	= 0;

	AssertSz(cchOld >= 0,
		"CRchTxtPtr::ReplaceRangeFormatting: Illegal cchOld");

	if(_rpCF.IsValid())
	{
		iRunMerge = _rpCF._iRun;
		if( iRunMerge > 0 )
		{
			iRunMerge--;
		}

		if(FAILED(GetCharFormatCache(&pcfc)))
			return -1;							// Access to CF caches failed

		if(cchOld > 0)
		{										// add the soon-to-be deleted
			if( publdr )						// formats to the undo list
			{
				*ppaeCF = gAEDispenser.CreateReplaceFormattingAE(
							GetPed(), _rpCF, cchOld, pcfc, CharFormat);
			}
												// Delete/modify CF runs <-->
			_rpCF.Delete(cchOld, pcfc, 0);		//  to cchOld chars
		}
		// If we deleted all of text in story, don't bother adding a new
		// run.	Else insert/modify CF runs corresponding to cchNew chars
		//
		// For IME composition in a plain text control, there is no
		// trailing carriage return and thus need the extra test.
		if(cchNew > 1 || cchNew && ( cchOld < (LONG)GetTextLength()
						|| ( IsIMERich() && cchOld == (LONG)GetTextLength())))
		{
			_rpCF.InsertFormat(cchNew, iFormat, pcfc);
		}
#ifdef DEBUG
 		else if( cchNew > 0 )
		{
			// Make sure that we don't have any format runs; we should
			// be in the new document state.
			Assert(!_rpCF.IsValid());
		}
#endif // DEBUG

		if((cchOld || cchNew) && _rpCF.IsValid())// Deleting all text
		{										//  invalidates _rpCF
			_rpCF.MergeRuns(iRunMerge, pcfc);
			_rpCF.BindToCp(cp + cchNew);
		}
	}

	if(_rpPF.IsValid())
	{
		_rpPF.AdjustForward();					// Be absolutely sure that
												//  PF runs end with EOPs
		iRunMerge = _rpPF._iRun;
		if( iRunMerge > 0 )
		{
			iRunMerge--;
		}

		if(FAILED(GetParaFormatCache(&ppfc)))
			return -1;							// Access to PF caches failed

		if(cchOld)								// Delete cchOld from PF runs
		{										// add the soon-to-be deleted
			if( publdr )						// formats to the undo list
			{
				CFormatRunPtr rp(_rpPF);

				rp.AdvanceCp(cchPrevEOP);
				*ppaePF = gAEDispenser.CreateReplaceFormattingAE(GetPed(),
								rp, cchNextEOP - cchPrevEOP, ppfc, ParaFormat);
			}
		    _rpPF.Delete(cchOld, ppfc, cchMove);
		}

		if(_rpPF.IsValid())						// Deleting all text
		{										//  invalidates _rpPF
			_rpPF.AdjustForward();
			_rpPF.GetRun(0)->_cch += cchNew;	// Insert cchNew into current
			_rpPF._ich	+= cchNew;				//  PF run
			if(cchOld || cchNew)
			{
				_rpPF.MergeRuns(iRunMerge, ppfc);
				_rpPF.BindToCp(cp + cchNew);
			}
		}
	}
	return cchNew;
}

/*
 *	CRchTxtPtr::ExtendFormattingCRLF()
 *	
 *	@mfunc
 *		Use the same CCharFormat and CParaFormat indices for the EOP at
 *		this text ptr as those immediately preceeding it.
 *
 *	@devnote
 *		Leaves this text ptr's format ptrs at run you get from AdjustBackward
 *		since this run ends up including the new text.
 */	
void CRchTxtPtr::ExtendFormattingCRLF()
{
	LONG		  cch = GetTextLength() - GetPed()->GetAdjustedTextLength();
	IFormatCache *pf;
	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

	GetCharFormatCache((ICharFormatCache **)&pf);
	_rpCF.AdjustFormatting(cch, pf);

	GetParaFormatCache((IParaFormatCache **)&pf);
	_rpPF.AdjustFormatting(cch, pf);

	if( pnm )
	{
		// we assume that Cch is positive (or zero) here
		Assert(cch >= 0);
		pnm->NotifyPostReplaceRange((ITxNotify *)this, INFINITE, 0, 0,
				GetCp(), GetCp() + cch);
	}
}

/*
 *	CRchTxtPtr::IsRich()
 *	
 *	@mfunc
 *		Determine whether rich-text operation is operable
 *	
 *	@rdesc
 *		TRUE if associated CTxtEdit::_fRich = 1, i.e., control is allowed
 *		to be rich.
 */
BOOL CRchTxtPtr::IsRich()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::IsRich");

	return GetPed()->IsRich();
}

/*
 *	CRchTxtPtr::IsIMERich() const
 *	
 *	@mfunc
 *		This allow CF runs during plain text IME composition.
 *	
 *	@rdesc
 *		TRUE if not IsRich() and IME composition is occuring.
 */
BOOL CRchTxtPtr::IsIMERich() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::IsIMERich");
	CTxtEdit *ped;

	ped = GetPed();

	Assert(ped);

	return !ped->_fRich && ped->IsIMEComposition();
}

/*
 *	CRchTxtPtr::Check_rpCF()
 *	
 *	@mfunc
 *		enable _rpCF if it's not already enabled
 *	
 *	@rdesc
 *		TRUE if _rpCF is enabled
 *
 *	FUTURE (murrays)
 *		Combine more of Check_rpCF and Check_rpPF. Maybe define common routine:
 *
 *			BOOL CRchTxtPtr::Check_rp(CFormatPtr& rp, DWORD cbOffset)
 *
 *		called by
 *
 *			Check_rp (_rpCF, offsetof(CTxtStory, _pCFRuns))
 *
 *		Note that cbOffset arg turns into a fast 2-byte constant push on x86
 */
BOOL CRchTxtPtr::Check_rpCF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Check_rpCF");

	if(_rpCF.IsValid())
		return TRUE;

	if( !IsRich() && !IsIMERich() )
		return FALSE;
	
	if( !_rpCF.InitRuns (GetCp(), GetTextLength(),
				&(GetPed()->GetTxtStory()->_pCFRuns)) )
	{
		return FALSE;
	}

	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();	// For notifying of changes
	if( pnm )
		pnm->NotifyPostReplaceRange(	 		// Notify interested parties
				(ITxNotify *)this, INFINITE,	//  that
				0, 0, INFINITE, INFINITE);

	return TRUE;
}

/*
 *	CRchTxtPtr::Check_rpPF()
 *	
 *	@mfunc
 *		enable _rpPF if it's not already enabled
 *	
 *	@rdesc
 *		TRUE if _rpPF is enabled
 */
BOOL CRchTxtPtr::Check_rpPF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Check_rpPF");

	if(_rpPF.IsValid())
		return TRUE;

	if( !IsRich() )
		return FALSE;

	if( !_rpPF.InitRuns (GetCp(), GetTextLength(),
				&(GetPed()->GetTxtStory()->_pPFRuns)) )
	{
		return FALSE;
	}

	CNotifyMgr *pnm = GetPed()->GetNotifyMgr();	// For notifying of changes
	if( pnm )
		pnm->NotifyPostReplaceRange(	 		// Notify interested parties
				(ITxNotify *)this, INFINITE,	// of the change.
				0, 0, INFINITE, INFINITE);

	return TRUE;
}

/*
 * CRchTxtPtr::FindWordBreak(action, cpMost)
 *
 *	@mfunc
 *		Same as CTxtPtr::FindWordBreak(), but moves the whole rich text ptr
 *
 *	@rdesc
 *		cch this rich text ptr is moved
 */
LONG CRchTxtPtr::FindWordBreak(
	INT		action,		//@parm Kind of word break to find
	LONG	cpMost)		//@parm Limiting character position

{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::FindWordBreak");

	LONG cch = _rpTX.FindWordBreak(action, cpMost);
	_rpCF.AdvanceCp(cch);
	_rpPF.AdvanceCp(cch);

	return cch;
}

/*
 *	CRchTxtPtr::BindToCp(dwNewCp)
 *
 *	@mfunc
 *		Set cp to new value and recalculate that new position.
 */
void CRchTxtPtr::BindToCp(
	DWORD cp)			// @parm new cp for rich text
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::BindToCp");

	cp = _rpTX.BindToCp(cp);		// Recalculate cp for plain text

	// use the InitRunPtrs routine so that the run pointers will get 
	// re-initialized and rebound with the correct run array.  The
	// run array formerly used (if any at all) is not necessary valid
	// when this function is called.

	InitRunPtrs(cp);

	// Do invariant testing at end because this fixes up the rich text
	// pointer in the face of backing store changes.
	_TEST_INVARIANT_
}

/*
 *	CRchTxtPtr::CheckFormatRuns ()
 *
 *	@mfunc
 *		Check the format runs against what's in CTxtStory.  If
 *		different, forces a rebind to <p cp>
 */
void CRchTxtPtr::CheckFormatRuns( )
{
	CTxtStory *pStory = GetPed()->GetTxtStory();

	if( (pStory->GetCFRuns() != (CFormatRuns *)_rpCF._prgRun) ||
		(pStory->GetPFRuns() != (CFormatRuns *)_rpPF._prgRun) )
	{
		InitRunPtrs(GetCp());
	}

	_TEST_INVARIANT_
}

/*
 *	CRchTxtPtr::ChangeCase(cch, Type, publdr)
 *	
 *	@mfunc
 *		Change case of cch chars starting at this text ptr according to Type,
 *		which has the possible values:
 *
 *		tomSentenceCase	= 0: capitalize first letter of each sentence
 *		tomLowerCase	= 1: change all letters to lower case
 *		tomUpperCase	= 2: change all letters to upper case
 *		tomTitleCase	= 3: capitalize the first letter of each word
 *		tomToggleCase	= 4: toggle the case of each letter
 *	
 *	@rdesc
 *		TRUE iff a change occurred
 *
 *	@devnote
 *		Since this routine only changes the case of characters, it has no
 *		effect on rich-text formatting.  However it is part of the CRchTxtPtr
 *		class in order to notify the display of changes.  CTxtRanges are also
 *		notified just in case the text blocks are modified.
 */
BOOL CRchTxtPtr::ChangeCase (
	LONG		  cch,			//@parm # chars to change case for
	LONG		  Type,			//@parm Type of change case command
	IUndoBuilder *publdr)		//@parm UndoBuilder to receive anti-event
								//  	for any replacements
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::ChangeCase");
	_TEST_INVARIANT_

#define	BUFFERLEN	256

	LONG	cchChunk, cchFirst, cchFormat, cchGet, cchLast;
	BOOL	fAlpha, fToUpper, fUpper;			// Flags controling case change
	BOOL	fChange = FALSE;					// No change yet
	BOOL	fStart = TRUE;						// Start of Word/Sentence
	TCHAR *	pch;								// Ptr to walk rgCh with
	WORD *	pType;								// Ptr to walk rgType with
	WCHAR	rgCh[BUFFERLEN];					// Char buffer to work in
	WORD	rgType[BUFFERLEN];					// C1_TYPE array for rgCh

	if( GetCp() )
	{
		if( Type == tomSentenceCase )
		{
			fStart = _rpTX.IsAtBOSentence();
		}
		else if( Type == tomTitleCase )
		{
			// check to see if we are at the beginning of
			// a word.  This is the case if the character preceeding
			// our current position is white space.
			fStart = IsWhiteSpace(_rpTX.GetPrevChar());
		}
	}
	while(cch > 0)								// Do 'em all (or as many as
	{											//  in story)
		cchChunk = min(BUFFERLEN, cch);			// Get next bufferful
		if(_rpCF.IsValid())						// Make sure it stays within
		{										//  current char/paraformat
			cchFormat = _rpCF.GetCchLeft();		//  runs to simplify changing
			cchChunk = min(cchChunk, cchFormat);//  text with undo
		}										
		if(_rpPF.IsValid())						
		{
			cchFormat = _rpPF.GetCchLeft(); 
			cchChunk = min(cchChunk, cchFormat); 
		}
		cch -= cchChunk;						// Decrement the count
		cchGet = _rpTX.GetText(cchChunk, rgCh);	// Manipulate chars in buffer
		if(cchGet < cchChunk)					//  (for undo, need to use
		{										//  ReplaceRange())
			cch = 0;							// No more chars in story,
			if(!cchGet)							//  so we'll be done
				break;							// We're done already
			cchChunk = cchGet;					// Something in this chunk
		}

		if (0 == W32->GetStringTypeEx(0, CT_CTYPE1, rgCh,		// Find out whether chars are
						 cchChunk, rgType))				//  UC, LC, or neither
        {
            TRACEWARNSZ("GetStringTypeEx failed");
            return FALSE;
        }    
		cchLast = 0;									// Default nothing to replace
		cchFirst = -1;
		for(pch = rgCh, pType = rgType;			// Process buffered chars
			cchChunk;
			cchChunk--, pch++, pType++)
		{
			fAlpha = *pType & (C1_UPPER | C1_LOWER); // Nonzero if UC or LC
			fUpper = (*pType & C1_UPPER) != 0;	// TRUE if UC
			fToUpper = fStart ? TRUE : fUpper;	// capitalize first letter of a
												// sentence
			switch(Type)
			{									// Decide whether to change
			case tomLowerCase:					//  case and determine start
				fToUpper = FALSE;				//  of word/sentence for title
				break;							//  and sentence cases

			case tomUpperCase:
				fToUpper = TRUE;
				break;

			case tomToggleCase:
				fToUpper = !fUpper;
				break;

			case tomSentenceCase:
				if(*pch == TEXT('.'))			// If sentence terminator,
					fStart = TRUE;				//  capitalize next alpha
				if(fAlpha)						// If this char is alpha, next
					fStart = FALSE;				//  char can't start a
				break;							//  sentence

			case tomTitleCase:					// If this char is alpha, next
				fStart = (fAlpha == 0);			//  char can't start a word
				break;
			default:
				return FALSE;
			}

			if(fAlpha && (fToUpper ^ fUpper))	// Only change case if it
			{									//  makes a difference (saves
				if(fToUpper)					//  on system calls and undos)
					CharUpperBuff(pch, 1);
				else
					CharLowerBuff(pch, 1);

				fChange = TRUE;					// Return value: change made
				if( cchFirst == -1 )			// Save cch of unchanged
					cchFirst = cchGet-cchChunk;	//  leading string
				cchLast = cchChunk - 1;			// Save cch of unchanged
			}									//  trailing string
		}
		if( cchFirst == -1 )
		{
			Assert(cchLast == 0);
			cchFirst = cchGet;
		}

		Advance(cchFirst);						// Skip unchanged leading
		cchGet -= cchFirst + cchLast;			//  string. cchGet = cch of
		_rpCF.AdjustForward();					//  changed span. Adjust in
												//  case cchFirst = 0
		ReplaceRange(cchGet, cchGet, rgCh		
			+ cchFirst, publdr, _rpCF.GetFormat());
		Advance(cchLast);						// Skip unchanged trailing 
	}											//  string
	return fChange;
}

// The following defines a mask for Units implemented by UnitCounter()
#define IMPL ((1 << tomCharacter)  + (1 << tomWord) + (1 << tomSentence) + \
			  (1 << tomParagraph)  + (1 << tomLine) + (1 << tomStory) +	\
			  (1 << tomCharFormat) + (1 << tomParaFormat) + (1 << tomObject))
 
/*
 *	CRchTxtPtr::UnitCounter (Unit, &cUnit, cchMax)
 *
 *	@mfunc
 *		Helper function to count chars in <p cUnit> Units defined by <p Unit>
 *		<p cUnit> is a signed count.  If it extends beyond either end of the
 *		story, count up to that end and update <p cUnit> accordingly.  If
 *		<p cchMax> is nonzero, stop counting when the count exceeds <p cchMax>
 *		in magnitude.
 *
 *	@rdesc
 *		If unit is implemented, return cch corresponding to the units counted
 *		(up to a maximum magnitude of <p cchMax>) and update cUnit;
 *		else return tomForward to signal unit not implemented and cUnit = 0.
 *		If unit is implemented but unavailable, e.g., tomObject with no
 *		embedded objects, return tomBackward.
 *
 *	@devnote
 *		This is the basic engine used by the TOM CTxtRange::Move() and Index()
 *		methods.
 */
LONG CRchTxtPtr::UnitCounter (
	LONG	Unit,				//@parm Type of unit to count
	LONG &	cUnit,				//@parm Count of units to count chars for
	LONG	cchMax)				//@parm Maximum character count
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CRchTxtPtr::UnitCounter");

	LONG	action;				// Gives direction and tomWord commands
	LONG	cch;				// Collects cch counted
	LONG	cchText = GetTextLength();
	LONG	cp = GetCp();
	LONG	iDir = cUnit > 0 ? 1 : -1;
	LONG	j;					// For-loop index
	CDisplay *pdp;				// Used for tomLine case

	if(!cUnit)									// Nothing to count
	{
		return ((DWORD)Unit > tomObject || !((IMPL >> Unit) & 1))
			? tomForward : 0;					// Indicate Unit not
	}											//  implemented
	if(cchMax <= 0)
		cchMax = tomForward;					// No cch limit

	switch(Unit)
	{
	case tomCharacter:							// Smallest Unit
		cp += cUnit;							// Requested new cp
		ValidateCp(cp);							// Make sure it's OK
		cch = cUnit = cp - GetCp();				// How many cch, cUnits
		break;									//  actually moved

	case tomStory:								// Largest Unit
		cch = (cUnit > 0) ? cchText - cp : -cp;	// cch to start of story
		cUnit = cch ? iDir : 0;					// If already at end/start,
		break;									//  of story, no count

	case tomCharFormat:							// Constant CHARFORMAT
		cch = _rpCF.CountRuns(cUnit, cchMax, cchText);
		break;

	case tomParaFormat:							// Constant PARAFORMAT
		cch = _rpPF.CountRuns(cUnit, cchMax, cchText);
		break;

	case tomObject:
		if(!GetObjectCount())					// No objects: can't move, so
		{
			cUnit = 0;							//  set cUnit = 0 and
			return tomBackward;					//  signal Unit unavailable
		}
		cch = GetPed()->_pobjmgr->CountObjects(cUnit, GetCp());
		break;

	case tomLine:
		pdp = GetPed()->_pdp;
		if(pdp)									// If this story has a display
		{										//  use a CLinePtr
			CLinePtr rp(pdp);
			pdp->WaitForRecalc(cp, -1);
			rp.RpSetCp(cp, FALSE);
			cch = rp.CountRuns(cUnit, cchMax, cchText);
			break;
		}										// Else fall thru to treat as
												//  tomPara
	default:									// tp dependent cases
	  {											// Block to contain tp() which
		CTxtPtr tp(_rpTX);						//  takes time to construct

		if (cUnit < 0)							// Counting backward
		{
			action = (Unit == tomWord)
				? WB_MOVEWORDLEFT : tomBackward;
		}
		else									// Counting forward
		{
			action = (Unit == tomWord)
				? WB_MOVEWORDRIGHT : tomForward;
		}
	
		for (cch = 0, j = cUnit; j && abs(cch) < cchMax; j -= iDir)
		{
			cp = tp.GetCp();					// Save starting cp for
			switch (Unit)						//  calculating cch for this
			{									//  Unit
			case tomWord:
				tp.FindWordBreak(action);
				break;
	
			case tomSentence:
				tp.FindBOSentence(action);
				break;
		
			case tomLine:						// Story has no line array:
			case tomParagraph:					//  treat as tomParagraph
				tp.FindEOP(action);
				break;
		
			default:
				cUnit = 0;
				return tomForward;				// Return error
			}
			if(tp.GetCp() - cp == 0)			// No count:
				break;							//  don't decrement cUnit
			cch += tp.GetCp() - cp;
		}
		cUnit -= j;								// Discount any runs not
	  }											//  counted if |cch| >= cchMax
	}

	if(abs(cch) > cchMax)						// Keep cch within requested
	{											//  limit
		cch = cch > 0 ? cchMax : -cchMax;
		if(Unit == tomCharacter)
			cUnit = cch;
	}		

	Advance(cch);								// Move to new position
	return cch;									// Total cch counted
}

/*
 *	CRchTxtPtr::Zombie ()
 *
 *	@mfunc
 *		Turn this object into a zombie by NULLing out its _ped member
 */
void CRchTxtPtr::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CRchTxtPtr::Zombie");

	_rpTX.Zombie();
	_rpCF.SetToNull();
	_rpPF.SetToNull();
}

