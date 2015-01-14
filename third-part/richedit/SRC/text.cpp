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
 *	@doc	INTERNAL
 *
 *	@module TEXT.C -- CTxtPtr implementation |
 *	
 *	Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	History: <nl>
 *		6/25/95		alexgo	cleanup and reorganization (use run pointers now)
 *
 */

#include "_common.h"
#include "_text.h"
#include "_edit.h"
#include "_antievt.h"
#include "_clasfyc.h"


ASSERTDATA

//-----------------------------Internal functions--------------------------------
// Text Block management
LOCAL void TxDivideInsertion(DWORD cch, DWORD ichBlock, DWORD cchAfter,
			DWORD *pcchFirst, DWORD *pcchLast);

#define IsSameNonFEClass(_c1, _c2)	(!(((_c1) ^ (_c2)) & WBF_CLASS))

#define IdeoKanaTypes (C3_HALFWIDTH | C3_FULLWIDTH | C3_KATAKANA | C3_HIRAGANA)
#define IdeoTypes	  (IdeoKanaTypes | C3_IDEOGRAPH)
#define IsIdeographic(_c1) ( 0 != (_c1 & (C3_KATAKANA | C3_HIRAGANA | C3_IDEOGRAPH)) )

/*
 *	IsSameClass(currType1, startType1, currType3, startType3 )
 *
 *	@func	Used to determine word breaks.
 *
 *	@comm	Ideographic chars are all considered to be unique, so that only
 *			one at a time is selected
 */
static BOOL IsSameClass(WORD currType1, WORD startType1,
						WORD currType3, WORD startType3 )
{
	BOOL	fIdeographic = IsIdeographic(currType3);

	// Do classifications for startType3 being ideographic
	if (IsIdeographic(startType3))
	{
		WORD	checkTypes  = (currType3 & IdeoTypes) ^ (startType3 & IdeoTypes);

		// We only get picky with non-ideographic Kana chars
		//  C3_HALFWIDTH | C3_FULLWIDTH | C3_KATAKANA | C3_HIRAGANA.
		return fIdeographic && (startType3 & IdeoKanaTypes) &&
			   (!checkTypes || checkTypes == C3_FULLWIDTH || checkTypes == C3_HIRAGANA || 
			   checkTypes == (C3_FULLWIDTH | C3_HIRAGANA));
	}	

	// Do classifications for nonideographic startType3
	return !fIdeographic && IsSameNonFEClass(currType1, startType1);
}

static int ClassifyChar(TCHAR ch)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "ClassifyChar");
	WORD wRes;

	if (IsKorean(ch))									// special Korean class
		return 0x0f;

	if (ch == WCH_EMBEDDING)							// Objects
		return 2 | WBF_BREAKAFTER;

	if (!W32->GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, &ch, 1, &wRes))
	{
	    return 0;  //GetStringTYpe failed - nothing more to do
	}    

	if(wRes & C1_SPACE)
	{
		if(wRes & C1_BLANK)								// Only TAB, BLANK, and
		{												//  nobreak BLANK are here
			if(ch == 0x20)
				return 2 | WBF_BREAKLINE | WBF_ISWHITE;
			if(ch == TAB)
				return 3 | WBF_ISWHITE;
			return 2 | WBF_ISWHITE;
		}
		if(ch == CELL)
			return 3 | WBF_ISWHITE;
		return 4 | WBF_ISWHITE;
	}
	if(wRes & C1_PUNCT)
		return ch == '-' ? (1 | WBF_BREAKAFTER) : 1;
	return 0;
}

static void ClassifyChars(const TCHAR *pch, LONG cch, WORD *pwRes)
{ 
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "ClassifyChars");
	WORD	wRes;
	TCHAR	ch;

	if (!W32->GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, pch, cch, pwRes))
	{
	    //since we don't return a value here, we need to make sure that pwRes is initialized
	    ZeroMemory(pwRes, cch);
	    return;
    }
    

	while(cch--)
	{
		wRes = *pwRes;
		ch = *pch++;
		
		if (IsKorean(ch))								
			wRes = 0x0f;								// special Korean class
		else if (ch == WCH_EMBEDDING)					// Objects
			wRes = 2 | WBF_BREAKAFTER;
		else if(wRes & C1_SPACE)
		{
			if (wRes & C1_BLANK)
			{
				wRes = 2 | WBF_ISWHITE;
				if(ch == 0x20)
					wRes = 2 | WBF_BREAKLINE | WBF_ISWHITE;
				if(ch == TAB)
					wRes = 3 | WBF_ISWHITE;
			}
			else
				wRes = 4 | WBF_ISWHITE;
		}
		else if(ch == CELL)
			wRes = 3 | WBF_ISWHITE;

		else if(wRes & C1_PUNCT)
			wRes = ch == '-' ? (1 | WBF_BREAKAFTER) : 1;

		else 
			wRes = 0;
		*pwRes++ = wRes;
	}
}

/*
 *	IsEOP(ch)
 *
 *	@func
 *		Used to determine if ch is an EOP char, i.e., CR, LF, VT, FF, PS, or
 *		LS (Unicode paragraph/line separator). This function (or its CR/LF
 *		subset) is often inlined, but is useful if speed isn't critical.
 *
 *	@rdesc
 *		TRUE if ch is an end-of-paragraph char
 */  
BOOL IsEOP(unsigned ch)
{
	return IN_RANGE(LF, ch, CR) || (ch | 1) == PS;
}

/*
 *	IsWhiteSpace(ch)
 *
 *	@func
 *		Used to determine if ch is an EOP char (see IsEOP() for definition),
 *		TAB or blank. This function is used in identifying sentence start
 *		and end.
 *
 *	@rdesc
 *		TRUE if ch is whitespace
 */  
BOOL IsWhiteSpace(unsigned ch)
{
	return ch == ' ' || IN_RANGE(CELL, ch, CR) || (ch | 1) == PS;
}

/*
 *	IsSentenceTerminator(ch)
 *
 *	@func
 *		Used to determine if ch is a standard sentence terminator character,
 *		namely, '?', '.', or '!'
 *
 *	@rdesc
 *		TRUE if ch is a question mark, period, or exclamation point.
 */  
BOOL IsSentenceTerminator(unsigned ch)
{
	return ch == '?' || ch == '.' || ch == '!';		// Std sentence delimiters
}


// ===========================  Invariant stuff  ==================================================

#define DEBUG_CLASSNAME CTxtPtr
#include "_invar.h"

// ===============================  CTxtPtr  ======================================================

#ifdef DEBUG

/*
 *	CTxtPtr::Invariant
 *
 *	@mfunc	invariant check
 */
BOOL CTxtPtr::Invariant( ) const
{
	static LONG	numTests = 0;
	numTests++;				// how many times we've been called.

	// make sure _cp is within range.
	Assert(_cp >= 0 );

	CRunPtrBase::Invariant();

	if( _prgRun )
	{
		// we use less than or equals here so that we can be an insertion
		// point at the *end* of the currently existing text (cpLim).
		Assert(_cp <= GetTextLength());

		// make sure all the blocks are consistent...
		((CTxtArray *)_prgRun)->Invariant();
	}

	Assert( _cp == CRunPtrBase::GetCp() );

	return TRUE;
}

/*
 *	CTxtPtr::MoveGapToEndOfBlock ()
 *	
 *	@mfunc
 *		Function to move buffer gap to current block end to aid in debugging
 */
void CTxtPtr::MoveGapToEndOfBlock () const
{
 	CTxtBlk *ptb = GetRun(0);
	ptb->MoveGap(ptb->_cch);				// Move gaps to end of cur block
}

#endif	// DEBUG


/*
 *	CTxtPtr::CTxtPtr(ped, cp)
 *
 *	@mfunc	constructor
 */
CTxtPtr::CTxtPtr (
	CTxtEdit *ped,		//@parm	Ptr to CTxtEdit instance
	DWORD cp)			//@parm cp to set the pointer to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::CTxtPtr");

	_ped = ped;
	_cp = 0;
	SetRunArray((CRunArray *) &ped->GetTxtStory()->_TxtArray);
	_cp = BindToCp(cp);
}

/*
 *	CTxtPtr::CTxtPtr(&tp)
 *
 *	@mfunc	Copy Constructor
 */
CTxtPtr::CTxtPtr (
	const CTxtPtr &tp)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::CTxtPtr");

	// copy all the values over
	*this = tp;
}	

/*
 *	CTxtPtr::GetTextLength()
 *	
 *	@mfunc
 *		Return count of characters in the story pointed to by this
 *		text ptr.  Includes the story's final CR in the count
 *
 *	@rdesc
 *		cch for the story pointed to by this text ptr
 *
 *	@devnote
 *		This method returns 0 if the text ptr is a zombie, a state
 *		identified by _ped = NULL.
 */
DWORD CTxtPtr::GetTextLength() const
{
	return _ped ? ((CTxtArray *)_prgRun)->_cchText : 0;
}

/*
 *	CTxtPtr::GetChar()
 *	
 *	@mfunc
 *		Return character at this text pointer, NULL if text pointer is at 
 *		end of text
 *
 *	@rdesc
 *		Character at this text ptr
 */
TCHAR CTxtPtr::GetChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetChar");

	LONG		 cchValid;
	const TCHAR *pch = GetPch(cchValid);

	return pch ? *pch : 0;
}  

/*
 *	CTxtPtr::GetPrevChar()
 *	
 *	@mfunc
 *		Return character just before this text pointer, NULL if text pointer 
 *		beginning of text
 *
 *	@rdesc
 *		Character just before this text ptr
 */
TCHAR CTxtPtr::GetPrevChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPrevChar");

	LONG		 cchValid;
	const TCHAR *pch = GetPchReverse(cchValid);
       if (pch && (pch-1))
            return *(pch - 1);
       else
            return 0;
}  

/*
 *	CTxtPtr::GetPch(&cchValid)
 *	
 *	@mfunc
 *		return a character pointer to the text at this text pointer
 *
 *	@rdesc
 *		a pointer to an array of characters.  May be NULL.  If non-null,
 *		then cchValid is guaranteed to be at least 1
 */
const TCHAR * CTxtPtr::GetPch(
	LONG & 	cchValid)		//@parm	Count of characters for which the
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPch");
							//		returned pointer is valid
	_TEST_INVARIANT_

	DWORD		ich = _ich;
	TCHAR *		pchBase;
	CTxtBlk *	ptb = _prgRun ? GetRun(0) : NULL;

	if( !ptb )
	{
		cchValid = 0;
		return NULL;
	}

	// if we're at the edge of a run, grab the next run or 
	// stay at the current run.

	if( _ich == ptb->_cch )
	{
		if( _iRun < Count() - 1 )
		{
			// set us to the next text block
			ptb = GetRun(1);
			ich = 0;
		}
		else
		{
			//we're at the very end of the text, just return NULL
			cchValid = 0;
			return NULL;
		}
	}

	AssertSz(CbOfCch(ich) <= ptb->_cbBlock,
		"CTxtPtr::GetPch(): _ich bigger than block");

	pchBase = ptb->_pch + ich;


	// Check to see if we need to skip over gap.  Recall that
	// the game may come anywhere in the middle of a block,
	// so if the current ich (note, no underscore, we want 
	// the active ich) is beyond the gap, then recompute pchBase
	// by adding in the size of the block.
	//
	// cchValid will then be the number of characters left in
	// the text block (or _cch - ich) 
  
	if(CbOfCch(ich) >= ptb->_ibGap)
	{
		pchBase += CchOfCb(ptb->_cbBlock) - ptb->_cch;
		cchValid = ptb->_cch - ich;
	}
	else
	{
		//we're valid until the buffer gap (or see below).
		cchValid = CchOfCb(ptb->_ibGap) - ich;
	}

	Assert(cchValid);
	return pchBase;
}

/*
 *	CTxtPtr::GetPchReverse(&cchValidReverse, pcchValid)
 *	
 *	@mfunc
 *		return a character pointer to the text at this text pointer
 *		adjusted so that there are some characters valid *behind* the
 *		pointer.
 *
 *	@rdesc
 *		a pointer to an array of characters.  May be NULL.  If non-null,
 *		then cchValidReverse is guaranteed to be at least 1
 */
const TCHAR * CTxtPtr::GetPchReverse(
	LONG & 	cchValidReverse,		//@parm	length for reverse 
	LONG *	pcchValid)				//@parm length forward
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPchReverse");

	_TEST_INVARIANT_

	LONG		cchTemp;
	DWORD		ich = _ich;
	TCHAR *		pchBase;
	CTxtBlk *	ptb = GetRun(0);

	if( !ptb )
	{
		cchValidReverse = 0;
		return NULL;
	}

	// if we're at the edge of a run, grab the previous run or 
	// stay at the current run.

	if( !_ich )
	{
		if( _iRun )
		{
			// set us to the next text block
			ptb = GetRun(-1);
			ich = ptb->_cch;
		}
		else
		{
			//we're at the very beginning of the text, just return NULL
			cchValidReverse = 0;
			return NULL;
		}
	}

	AssertSz(CbOfCch(ich) <= ptb->_cbBlock,
		"CTxtPtr::GetPchReverse(): _ich bigger than block");

	pchBase = ptb->_pch + ich;

	// Check to see if we need to skip over gap.  Recall that
	// the game may come anywhere in the middle of a block,
	// so if the current ich (note, no underscore, we want 
	// the active ich) is at least one char past the gap, then recompute
	// pchBase by adding the size of the gap (so that it's after
	// the gap).  This differs from GetPch(), which works forward and
	// wants pchBase to include the gap size if ich is at the gap, let
	// alone one or more chars past it.
	//
	// Also figure out the count of valid characters.  It's
	// either the count of characters from the beginning of the 
	// text block, i.e. ich, or the count of characters from the
	// end of the buffer gap.

	cchValidReverse = ich;					// Default for ich <= gap offset
	cchTemp = ich - CchOfCb(ptb->_ibGap);	// Calculate displacement
	if(cchTemp > 0)							// Positive: pchBase is after gap
	{
		cchValidReverse = cchTemp;
		pchBase += CchOfCb(ptb->_cbBlock) - ptb->_cch;	// Add in gap size
	}
	if ( pcchValid )						// if client needs forward length
	{
		if ( cchTemp > 0 )
			cchTemp = ich - ptb->_cch;
		else
			cchTemp = -cchTemp;

		*pcchValid = cchTemp;
	}

	Assert(cchValidReverse);

	return pchBase;
}

/*
 *	CTxtPtr::BindToCp(cp)
 *
 *	@mfunc
 *		set cached _cp = cp (or nearest valid value)
 *
 *	@rdesc
 *		_cp actually set
 *
 *	@comm
 *		This method overrides CRunPtrBase::BindToCp to keep _cp up to date
 *		correctly.
 *
 *	@devnote
 *		Do *not* call this method when high performance is needed; use
 *		AdvanceCp() instead, which advances from 0 or from the cached
 *		_cp, depending on which is closer.
 */
DWORD CTxtPtr::BindToCp(
	DWORD	cp)			//@parm	char position to bind to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::BindToCp");

	_cp = CRunPtrBase::BindToCp(cp);

	// We want to be able to use this routine to fix up things so we don't
	// check invariants on entry.
	_TEST_INVARIANT_
	return _cp;
}


/*
 *	CTxtPtr::SetCp(cp)
 *
 *	@mfunc
 *		'efficiently' sets cp by advancing from current position or from 0,
 *		depending on which is closer
 *
 *	@rdesc
 *		cp actually set to
 */
DWORD CTxtPtr::SetCp(
	DWORD	cp)		//@parm char position to set to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::SetCp");

	AdvanceCp(cp - _cp);
	return _cp;
}

/*
 *	CTxtPtr::AdvanceCp(cch)
 *
 *	@mfunc
 *		Advance cp by cch characters
 *
 *	@rdesc
 *		Actual number of characters advanced by
 *
 *	@comm
 *		We override CRunPtrBase::AdvanceCp so that the cached _cp value
 *		can be correctly updated and so that the advance can be made
 *		from the cached _cp or from 0, depending on which is closer.
 *
 *	@devnote
 *		It's also easy to bind at the end of the story. So an improved
 *		optimization would bind there if 2*(_cp + cch) > _cp + text length.
 */
LONG CTxtPtr::AdvanceCp(
	LONG cch)			// @parm count of chars to advance by
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::AdvanceCp");

	if(!IsValid())							// No runs yet, so don't go
		return 0;							//  anywhere

	const LONG	cpSave = _cp;				// Save entry _cp
	LONG		cp = cpSave + cch;			// Requested target cp (maybe < 0)

	if(cp < cpSave/2)						// Closer to 0 than cached cp
	{
		cp = max(cp, 0);					// Don't undershoot
		_cp = CRunPtrBase::BindToCp(cp);
	}
	else
		_cp += CRunPtrBase::AdvanceCp(cch);	//  exist

	// NB! the invariant check needs to come at the end; we may be
	// moving 'this' text pointer in order to make it valid again
	// (for the floating range mechanism).

	_TEST_INVARIANT_
	return _cp - cpSave;					// cch this CTxtPtr moved
}

/*
 *	CTxtPtr::GetText(cch, pch)
 *	
 *	@mfunc
 *		get a range of cch characters starting at this text ptr. A literal
 *		copy is made, i.e., with no CR -> CRLF and WCH_EMBEDDING -> ' '
 *		translations.  For these translations, see CTxtPtr::GetPlainText()
 *	
 *	@rdesc
 *		count of characters actually copied
 *
 *  @comm
 *		Doesn't change this text ptr
 */
LONG CTxtPtr::GetText(
	LONG	cch, 			//@parm Count of characters to get
	TCHAR *	pch)			//@parm Buffer to copy the text into
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetText");

	LONG cchSave = cch;
	LONG cchValid;
	const TCHAR *pchRead;
	CTxtPtr tp(*this);

	_TEST_INVARIANT_

	// Use tp to read valid blocks of text until all the requested
	// text is read or until the end of story is reached.
	while( cch )
	{
		pchRead = tp.GetPch(cchValid);
		if(!pchRead)					// No more text
			break;

		cchValid = min(cchValid, cch);
		CopyMemory(pch, pchRead, cchValid*sizeof(TCHAR));
		pch += cchValid;
		cch -= cchValid;
		tp.AdvanceCp(cchValid);
	}
	return cchSave - cch;
}

/*
 *	CTxtPtr::GetPlainText(cchBuff, pch, cpMost, fTextize)
 *	
 *	@mfunc
 *		Copy up to cchBuff characters or up to cpMost, whichever comes
 *		first, translating lone CRs into CRLFs.  Move this text ptr just
 *		past the last character processed.  If fTextize, copy up to but
 *		not including the first WCH_EMBEDDING char. If not fTextize, 
 *		replace	WCH_EMBEDDING by a blank since RichEdit 1.0 does.
 *	
 *	@rdesc
 *		Count of characters copied
 *
 *  @comm
 *		An important feature is that this text ptr is moved just past the
 *		last char copied.  In this way, the caller can conveniently read
 *		out plain text in bufferfuls of up to cch chars, which is useful for
 *		stream I/O.  This routine won't copy the final CR even if cpMost
 *		is beyond it.
 */
LONG CTxtPtr::GetPlainText(
	LONG	cchBuff,		//@parm Buffer cch
	TCHAR *	pch,			//@parm Buffer to copy text into
	LONG	cpMost,			//@parm Largest cp to get
	BOOL	fTextize)		//@parm True if break on WCH_EMBEDDING
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::GetPlainText");

	LONG		 cch = cchBuff;				// Countdown counter
	LONG		 cchValid;					// Valid ptr cch
	LONG		 cchT;						// Temporary cch
	unsigned	 ch;						// Current char
	const TCHAR *pchRead;					// Backing-store ptr

	_TEST_INVARIANT_

	AdjustCpCRLF();							// Be sure we start on an EOP bdy

	LONG cchText = _ped->GetAdjustedTextLength();
	cpMost = min(cpMost, cchText);			// Don't write final CR
	if((LONG)GetCp() >= cpMost)
		return 0;

	while(cch > 0)							// While room in buffer
	{
		if(!(pchRead = GetPch(cchValid)))	// No more chars available
			break;							//  so we're out of here
		
		cchT = GetCp() + cchValid - cpMost;
		if(cchT > 0)						// Don't overshoot
		{
			cchValid -= cchT;
			if(cchValid <= 0)
				break;						// Nothing left before cpMost
		}

		for(cchT = 0; cch > 0 && cchT < cchValid; cchT++, cch--)
		{
			ch = *pch++ = *pchRead++;		// Copy next char (but don't
			if(IsASCIIEOP(ch))				//  count it yet)
			{
				AdvanceCp(cchT);			// Move up to CR
				if(cch < 2)					// No room for LF, so don't				
					goto done;				//  count CR either
											// Bypass EOP w/o worrying about
				cchT = AdvanceCpCRLF();		//  buffer gaps and blocks
				if(cchT > 2)				// Translate CRCRLF to ' '
				{							// Usually copied count exceeds
					Assert(cchT == 3);		//  internal count, but CRCRLFs
					*(pch - 1) = ' ';		//  reduce the relative increase:
				}							//  NB: error for EM_GETTEXTLENGTHEX
				else						// CRLF or lone CR
				{							// Store LF in both cases for
					*(pch - 1) = CR;		// Be sure it's a CR not a VT,
#ifndef MACPORT								//  FF, or lone LF
					*pch++ = LF;			// Windows. No LF for Mac
					cch--;					// One less for target buffer
#endif
				}
				cch--;						// CR (or ' ') copied
				cchT = 0;					// Don't AdvanceCp() more below
				break;						// Go get new pchRead & cchValid
			}
			else if(ch == WCH_EMBEDDING)	// Object lives here
			{
				if(fTextize)				// Break on WCH_EMBEDDING
				{
					AdvanceCp(cchT);		// Move this text ptr up to
					goto done;				//  WCH_EMBEDDING and return
				}
				*(pch - 1) = ' ';			// Replace embedding char by ' '
			}								//  since RichEdit 1.0 does
#ifdef PWD_JUPITER
            else if(ch == CELL)
            {
                // GuyBark Jupiter: We're writing out plain text here, so DON'T write
                // out RichEdit's own value for cell delimiters in tables. Instead
                // just write out a tab. This allows rows to be pasted into PExcel.
                *(pch - 1) = TAB;
            }
            // GuyBark JupiterJ 49481: J users do not want this action...
            // ARULM: Globalize: This is just plain weird, but the Japanese dont want 
            // this conversion to happen on Japanese machines. Whatever...
            else if((ch == LDBLQUOTE || ch == RDBLQUOTE) && !g_fHasJapanSupport)
            {
                // Left and right dbl quotes have code points in the 1252 code page.
                // This means unless we take action here, we output these left and
                // right characters in the text file. This is fine when re-opened in
                // PWord or Word97. But if the text file is opened in Notepage, it 
                // simply displays the square blocks for these extended characters.
                // That's not what we want, so always output the straight dbl quotes
                // for these characters when saving as text. This mimics Word97.
                *(pch - 1) = '"';
            }
            else if((ch == LQUOTE || ch == RQUOTE) && !g_fHasJapanSupport)
            {
                // The above also applies to left and right single quotes.
                *(pch - 1) = '\'';
            }
			// #endif // !FAREAST
            else if(ch == PWD_CRLFINCELL)
            {
                // We've hit the special character used to represent CRLF in 
                // table cells. Just is always followed by another space. This
                // isn't what Word97 does, (it starts a new line here), but who cares?
                *(pch - 1) = ' ';
            }

#endif // PWD_JUPITER
		}
		AdvanceCp(cchT);
	}

done:
	return cchBuff - cch;
}

/*
 *	CTxtPtr::AdvanceCpCRLF()
 *	
 *	@mfunc
 *		Advance text pointer by one character, safely advancing
 *		over CRLF, CRCRLF, and UTF-16 combinations
 *	
 *	@rdesc
 *		Number of characters text pointer has been moved by
 *
 *	@future
 *		Advance over Unicode combining marks
 */
LONG CTxtPtr::AdvanceCpCRLF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::AdvanceCpCRLF");

	_TEST_INVARIANT_

	DWORD	cpSave	= _cp;
	TCHAR	ch		= GetChar();		// Char on entry
	TCHAR	ch1		= NextChar();		// Advance to and get next char
	BOOL	fTwoCRs = FALSE;

	if(ch == CR)
	{
		if(ch1 == CR && _cp < GetTextLength()) 
		{
			fTwoCRs = TRUE;				// Need at least 3 chars to
			ch1 = NextChar();			//  have CRCRLF at end
		}
		if(ch1 == LF)
			AdvanceCp(1);				// Bypass CRLF
		else if(fTwoCRs)
			AdvanceCp(-1);				// Only bypass one CR of two

		AssertSz(_ped->Get10Mode() || _cp == cpSave + 1,
			"CTxtPtr::AdvanceCpCRLF: EOP isn't a single char");
	}

//	To support UTF-16, include the following code
//	if ((ch & UTF16) == UTF16_TRAIL)	// Landed on UTF-16 trail word
//		AdvanceCp(1);					// Bypass UTF-16 trail word

	return _cp - cpSave;				// # chars bypassed
}

/*
 *	CTxtPtr::NextChar()
 *	
 *	@mfunc
 *		Increment this text ptr and return char it points at
 *	
 *	@rdesc
 *		Next char
 */
TCHAR CTxtPtr::NextChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::NextChar");

	_TEST_INVARIANT_

 	AdvanceCp(1);
	return GetChar();
}

/*
 *	CTxtPtr::PrevChar()
 *	
 *	@mfunc
 *		Decrement this text ptr and return char it points at
 *	
 *	@rdesc
 *		Previous char
 */
TCHAR CTxtPtr::PrevChar()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::PrevChar");

	_TEST_INVARIANT_

	return AdvanceCp(-1) ? GetChar() : 0;
}

/*
 *	CTxtPtr::BackupCpCRLF()
 *	
 *	@mfunc
 *		Backup text pointer by one character, safely backing up
 *		over CRLF, CRCRLF, and UTF-16 combinations
 *	
 *	@rdesc
 *		Number of characters text pointer has been moved by
 *
 *	@future
 *		Backup over Unicode combining marks
 */
LONG CTxtPtr::BackupCpCRLF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::BackupCpCRLF");

	_TEST_INVARIANT_

	DWORD cpSave = _cp;
	TCHAR ch	 = PrevChar();			// Advance to and get previous char

	if (ch == LF &&					 	// Try to back up 1 char in any case
		(_cp && PrevChar() != CR ||		// If LF, does prev char = CR?
		 _cp && PrevChar() != CR))		// If so, does its prev char = CR?
	{									//  (CRLF at BOD checks CR twice; OK)
		AdvanceCp(1);					// Backed up too far
	}
//	To support UTF-16, include the following code
//	if ((ch & UTF16) == UTF16_TRAIL)	// Landed on UTF-16 trail word
//		AdvanceCp(-1);					// Backup to UTF-16 lead word

	return _cp - cpSave;				// - # chars this CTxtPtr moved
}

/*
 *	CTxtPtr::AdjustCpCRLF()
 *	
 *	@mfunc
 *		Adjust the position of this text pointer to the beginning of a CRLF,
 *		CRCRLF, or UTF-16 combination if it is in the middle of such a
 *		combination
 *	
 *	@rdesc
 *		Number of characters text pointer has been moved by
 *
 *	@future
 *		Adjust to beginning of sequence containing Unicode combining marks
 */
LONG CTxtPtr::AdjustCpCRLF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::AdjustCpCRLF");

	_TEST_INVARIANT_

	LONG	 cpSave = _cp;
	unsigned ch		= GetChar();

//	To support UTF-16, include the following code
//	if((ch & UTF16) == UTF16_TRAIL)
//		AdvanceCp(-1);

	if(!IsASCIIEOP(ch))							// Early out
		return 0;

	if(ch == LF && cpSave && PrevChar() != CR)	// Landed on LF not preceded
	{											//  by CR, so go back to LF
		AdvanceCp(1);							// Else on CR of CRLF or 
	}											//  second CR of CRCRLF

	if(GetChar() == CR)							// Land on a CR of CRLF or
	{											//  second CR of CRCRLF?
		CTxtPtr tp(*this);

		if(tp.NextChar() == LF)
		{
			tp.AdvanceCp(-2);					// First CR of CRCRLF ?
			if(tp.GetChar() == CR)				// Yes or CRLF is at start of
				AdvanceCp(-1);					//  story. Try to back up over
		}										//  CR (If at BOS, no effect)
	}
	return _cp - cpSave;
}

/*
 *	CTxtPtr::IsAtEOP()
 *	
 *	@mfunc
 *		Return TRUE iff this text pointer is at an end-of-paragraph mark
 *	
 *	@rdesc
 *		TRUE if at EOP
 *
 *	@devnote
 *		End of paragraph marks for RichEdit 1.0 and the MLE can be CRLF
 *		and CRCRLF.  For RichEdit 2.0, EOPs can also be CR, VT (0xb - Shift-
 *		Enter), and FF (0xc - page break).
 */
BOOL CTxtPtr::IsAtEOP()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAtEOP");

	_TEST_INVARIANT_

	unsigned ch = GetChar();

	if(IsASCIIEOP(ch))							// See if LF <= ch <= CR
	{											// Clone tp in case
		CTxtPtr tp(*this);						//  AdjustCpCRLF moves
		return !tp.AdjustCpCRLF();				// Return TRUE unless in
	}											//  middle of CRLF or CRCRLF
	return (ch | 1) == PS;						// Allow Unicode 0x2028/9 also
}

/*
 *	CTxtPtr::IsAfterEOP()
 *	
 *	@mfunc
 *		Return TRUE iff this text pointer is just after an end-of-paragraph
 *		mark
 *	
 *	@rdesc
 *		TRUE iff text ptr follows an EOP mark
 */
BOOL CTxtPtr::IsAfterEOP()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAfterEOP");

	_TEST_INVARIANT_

	CTxtPtr tp(*this);							// Clone to look around with
	TCHAR ch = GetChar();

	if(IsASCIIEOP(GetChar()) &&					// If in middle of CRLF
		tp.AdjustCpCRLF())						//  or CRCRLF, return FALSE
	{
		return FALSE;
	}

	return IsEOP(tp.PrevChar());				// After EOP if after Unicode
}								   				//  PS or LF, VT, FF, or CR

// needed for the routine below
#if cchGapInitial < 1
#error "cchGapInitial must be at least one"
#endif

/*
 *	CTxtPtr::FindWordBreak(action, cpMost)
 *	
 *	@mfunc
 *		Find a word break and move this text pointer to it.
 *
 *	@rdesc
 *		Offset from cp of the word break
 */
LONG CTxtPtr::FindWordBreak(
	INT		action,		//@parm See TxWordBreakProc header
	LONG	cpMost)		//@parm Limiting character position
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindWordBreak");

	_TEST_INVARIANT_

	const INT			breakBufSize = 16;
	LONG				bufferSize;
	LONG				cch;
	LONG				cchBuffer;
	LONG				cchChunk;
	DWORD				cchText = GetTextLength();
	TCHAR				ch = GetChar();
	TCHAR				chBreakBuf[breakBufSize];
	LONG				cpSave = _cp;				// For calculating break pt
	LONG				ichBreak;
	TCHAR *				pBuf;
	TCHAR const *		pch;
	EDITWORDBREAKPROC	pfnWB = _ped->_pfnWB;
	LONG				t;							// Temp for abs() macro

	if(action == WB_CLASSIFY || action == WB_ISDELIMITER)
	{
		return ch ? pfnWB(&ch, 0, CbOfCch(1), action) : 0;
	}

	if(action & 1)									// Searching forward
	{												// Easiest to handle EOPs			
		if(action == WB_MOVEWORDRIGHT && IsEOP(ch))	//  explicitly (spanning
		{											//  a class can go too
			AdjustCpCRLF();							//  far). Go to end of
			AdvanceCpCRLF();						//  EOP "word"
			goto done;
		}
													// Calc. max search
		if((DWORD)cpMost > cchText)					// Bounds check: get < 0
			cpMost = cchText;						//  as well as too big
		cch = cpMost - _cp;

		while(cch > 0)
		{											// The independent buffer
			cchBuffer = min(cch, breakBufSize - 1);	//  avoids gaps in BS
			cch -= bufferSize = cchBuffer;
			pBuf = chBreakBuf;						// Fill buffer forward

			// Grab the first character in reverse for fnWB that require 2
			// chars. Note, we play with _ich to get single char fnWB
			// to ignore this character.			 							
			pch = GetPchReverse(cchChunk);
			if ( !cchChunk ) pch = L" ";			// Any break char
			*pBuf++ = *pch;

			while ( cchBuffer )						// Finish filling
			{
				pch = GetPch(cchChunk);
				if (!cchChunk) { Assert(0); break; }

				cchChunk = min(cchBuffer, cchChunk);
				AdvanceCp(cchChunk);
				wcsncpy(pBuf, pch, cchChunk);
				pBuf += cchChunk;
				cchBuffer -= cchChunk;
			}
			ichBreak = pfnWB(chBreakBuf, 1,			// Find the break
						CbOfCch(bufferSize+1), action) - 1;

			// Apparently, some fnWBs return ambiguous results
			if(ichBreak >= 0 && ichBreak <= bufferSize)
			{
				// Ambiguous break pt?
				// Due to the imprecise nature of the word break proc spec,
				// we've reached an ambiguous condition where we don't know
				// if this is really a break, or just the end of the data.
				// By backing up or going forward by 2, we'll know for sure.
				// NOTE: we'll always be able to advance or go back by 2
				// because we guarantee that when !cch that we have
				// at least breakBufSize (16) characters in the data stream.
				if (ichBreak < bufferSize || !cch)
				{
					AdvanceCp( ichBreak - bufferSize );
					break;
				}

				// Need to recalc break pt to disambiguate
				t = AdvanceCp(ichBreak - bufferSize - 2);	// abs() is a
				cch += abs(t);						//  macro
			}
		}
	}
	else	// REVERSE - code dup based on EliK "streams" concept.
	{
		if(!_cp)									// Can't go anywhere
			return 0;

		if(action == WB_MOVEWORDLEFT)				// Easiest to handle EOPs			
		{											//  here
			if(IsASCIIEOP(ch) && AdjustCpCRLF())	// In middle of a CRLF or
				goto done;							//  CRCRLF "word"
			ch = PrevChar();						// Check if previous char
			if(IsEOP(ch))							//  is an EOP char
			{
				if(ch == LF)						// Backspace to start of
					AdjustCpCRLF();					//  CRLF and CRCRLF
				goto done;
			}
			AdvanceCp(1);							// Move back to start char
		}
													// Calc. max search
		if((DWORD)cpMost > _cp)						// Bounds check (also
			cpMost = _cp;							//  handles cpMost < 0)
		cch = cpMost;

		while(cch > 0)
		{											// The independent buffer
			cchBuffer = min(cch, breakBufSize - 1);	//  avoids gaps in BS
			cch -= bufferSize = cchBuffer;
			pBuf = chBreakBuf + cchBuffer;			// Fill from the end.

			// Grab the first character forward for fnWB that require 2 chars.
			// Note: we play with _ich to get single char fnWB to ignore this
			// character.
			pch = GetPch(cchChunk);
			if ( !cchChunk ) pch = L" ";			// Any break char
			*pBuf = *pch;

			while ( cchBuffer > 0 )					// Fill rest of buffer 
			{										//  before going in reverse
				pch = GetPchReverse(cchChunk );
				if (!cchChunk) { Assert(0); break; }

				cchChunk = min(cchBuffer, cchChunk);
				AdvanceCp(-cchChunk);
				pch -= cchChunk;
				pBuf -= cchChunk;
				wcsncpy(pBuf, pch, cchChunk);
				cchBuffer -= cchChunk;
			}
													// Get break left.
			ichBreak = pfnWB(chBreakBuf, bufferSize,
							 CbOfCch(bufferSize+1), action);
			
			// Apparently, some fnWBs return ambiguous results
			if(ichBreak >= 0 && ichBreak <= bufferSize)
			{										// Ambiguous break pt?
				// NOTE: when going in reverse, we have >= bufsize - 1
				//  because there is a break-after char (hyphen).
				if ( ichBreak > 0 || !cch )
				{
					AdvanceCp(ichBreak);			// Move _cp to break point.
					break;
				}													
				cch += AdvanceCp(2 + ichBreak);		// Need to recalc break pt
			}										//  to disambiguate.
		}
	}

done:
	return _cp - cpSave;							// Offset of where to break
}

/*
 *	INT CALLBACK TxWordBreakProc (pch, ich, cb, action)
 *	
 *	@func
 *		Default word break proc used in conjunction with FindWordBreak. ich
 *		is character offset (start position) in the buffer pch, which is cb
 *		bytes in length.  Possible action values are:
 *
 *	WB_CLASSIFY
 *		Returns char class and word break flags of char at start position.
 *
 *	WB_ISDELIMITER
 *		Returns TRUE iff char at start position is a delimeter.
 *
 *	WB_LEFT
 *		Finds nearest word beginning before start position using word breaks.
 *
 *	WB_LEFTBREAK
 *		Finds nearest word end before start position using word breaks.
 *		Used by CMeasurer::Measure()
 *
 *	WB_MOVEWORDLEFT
 *		Finds nearest word beginning before start position using class
 *		differences. This value is used during CTRL+LEFT key processing.
 *
 *	WB_MOVEWORDRIGHT
 *		Finds nearest word beginning after start position using class
 *		differences. This value is used during CTRL+RIGHT key processing.
 *
 *	WB_RIGHT
 *		Finds nearest word beginning after start position using word breaks.
 *		Used by CMeasurer::Measure()
 *
 *	WB_RIGHTBREAK
 *		Finds nearest word end after start position using word breaks.
 *	
 *	@rdesc
 *		Character offset from start of buffer (pch) of the word break
 */
INT CALLBACK TxWordBreakProc(
	TCHAR const *pch,	//@parm Char buffer
	INT			ich,	//@parm Char offset of _cp in buffer
	INT			cb,		//@parm Count of bytes in buffer
	INT			action)	//@parm Type of breaking action
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "TxWordBreakProc");

	LONG	cchBuff = CchOfCb(cb);
	LONG	cch = cchBuff - ich;
	TCHAR	ch;
	WORD	cType3[MAX_CLASSIFY_CHARS];
	INT		kinsokuClassifications[MAX_CLASSIFY_CHARS];
	WORD *	pcType3;
	INT  *	pKinsoku1, *pKinsoku2;
	WORD *	pwRes;
	WORD	startType3 = 0;
	WORD	wb = 0;
	WORD	wClassifyData[MAX_CLASSIFY_CHARS];	// For batch classifying

       if ((!(cchBuff < MAX_CLASSIFY_CHARS)) || (!(ich >= 0 && ich < cchBuff)))
        {
            Assert(0);
            return ich;
        }
	
	// Single character actions
	if ( action == WB_CLASSIFY )
		return ClassifyChar(pch[ich]);

	if ( action == WB_ISDELIMITER )
		return !!(ClassifyChar(pch[ich]) & WBF_BREAKLINE);

        if (!(cchBuff <= sizeof(wClassifyData)))
        {
            ASSERT(0);
            return ich;
        }

	// Batch classify buffer for whitespace and kinsoku classes
	ClassifyChars		(pch, cchBuff, wClassifyData);
	BatchKinsokuClassify(pch, cchBuff, cType3, kinsokuClassifications );

	// Setup pointers
	pKinsoku2 = kinsokuClassifications + ich; 		// Ptr to current  kinsoku
	pKinsoku1 = pKinsoku2 - 1;						// Ptr to previous kinsoku

	if(!(action & 1))								// WB_(MOVE)LEFTxxx
	{
		ich--;
		Assert(ich >= 0);
	}
	pwRes	 = &wClassifyData[ich];
	pcType3	 = &cType3[ich];						// for ideographics

	switch(action)
	{
	case WB_LEFT:
		for(; ich >= 0 && *pwRes & WBF_BREAKLINE;	// Skip preceding line
			ich--, pwRes--)							//  break chars
				;									// Empty loop. Then fall
													//  thru to WB_LEFTBREAK
	case WB_LEFTBREAK:
		for(; ich >= 0 && !CanBreak(*pKinsoku1, *pKinsoku2);
			ich--, pwRes--, pKinsoku1--, pKinsoku2--)
				;									// Empty loop
		if(action == WB_LEFTBREAK)					// Skip preceding line
		{											//  break chars
			for(; ich >= 0 && *pwRes & WBF_BREAKLINE;
				ich--, pwRes--)
					;								// Empty loop
		}
		return ich + 1;

	case WB_MOVEWORDLEFT:
		for(; ich >= 0 && (*pwRes & WBF_CLASS) == 2;// Skip preceding blank
			ich--, pwRes--, pcType3--)				//  chars
				;
		if(ich >= 0)								// Save starting wRes and
		{											//  startType3
			wb = *pwRes--;							// Really type1
			startType3 = *pcType3--;				// type3
			ich--;
		}
		// Skip to beginning of current word
		while(ich >= 0 && (*pwRes & WBF_CLASS) != 3 &&
			(IsSameClass(*pwRes, wb, *pcType3, startType3) ||
			!wb && ich && ((ch = pch[ich]) == '\'' || ch == RQUOTE)))
		{
			ich--, pwRes--, pcType3--;
		}
		return ich + 1;


	case WB_RIGHTBREAK:
		for(; cch > 0 && *pwRes & WBF_BREAKLINE;	// Skip any leading line
			cch--, pwRes++)							//  break chars
				;									// Empty loop
													// Fall thru to WB_RIGHT
	case WB_RIGHT:
		// Skip to end of current word
		for(; cch > 0 && !CanBreak(*pKinsoku1, *pKinsoku2);
			cch--, pKinsoku1++, pKinsoku2++, pwRes++)
				;
		if(action != WB_RIGHTBREAK)					// Skip trailing line
		{											//  break chars
			for(; cch > 0 && *pwRes & WBF_BREAKLINE;
				cch--, pwRes++)
					;
		}
		return cchBuff - cch;

	case WB_MOVEWORDRIGHT:
		if(cch <= 0)								// Nothing to do
			return ich;

		wb = *pwRes;								// Save start wRes
		startType3 = *pcType3;						//  and startType3

		// Skip to end of word
		if (startType3 & C3_IDEOGRAPH ||			// If ideographic or
			(*pwRes & WBF_CLASS) == 3)				//  tab/cell, just
		{
			cch--, pwRes++;							//  skip one char
		}
		else while(cch > 0 &&
			(IsSameClass(*pwRes, wb, *pcType3, startType3) || !wb &&
			 ((ch = pch[cchBuff - cch]) == '\'' || ch == RQUOTE)))
		{
			cch--, pwRes++, pcType3++;
		}

		for(; cch > 0 && (*pwRes & WBF_CLASS) == 2;	// Skip trailing blank
			cch--, pwRes++)							//  chars
					;
		return cchBuff - cch;
	}

	TRACEERRSZSC("TxWordBreakProc: unknown action", action);
	return ich;
}

/*
 *	CTxtPtr::ReplaceRange(cchOld, cchNew, *pch, pcpFirstRecalc)
 *	
 *	@mfunc
 *		replace a range of text at this text pointer.
 *	
 *	@rdesc
 *		count of new characters added
 *	
 *	@comm	SideEffects: <nl>
 *		moves this text pointer to end of replaced text <nl>
 *		moves text block array <nl>
 *
 */
DWORD CTxtPtr::ReplaceRange(
	LONG cchOld, 				//@parm length of range to replace 
								// (<lt> 0 means to end of text)
	DWORD cchNew, 				//@parm length of replacement text
	TCHAR const *pch, 			//@parm replacement text
	IUndoBuilder *publdr,		//@parm if non-NULL, where to put an 
								// anti-event for this action
	IAntiEvent *paeCF,			//@parm char format AE
	IAntiEvent *paePF )			//@parm paragraph formatting AE
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::ReplaceRange");

	_TEST_INVARIANT_

	DWORD cchAdded = 0;
	DWORD cchInBlock;
	DWORD cchNewInBlock;

	CTxtBlk *ptb;

	if(cchOld < 0)
		cchOld = GetTextLength() - _cp;

	if( publdr )
	{
		HandleReplaceRangeUndo( cchOld, cchNew, publdr, paeCF, paePF);
	}

	// blocks involving replacement

	while(cchOld > 0 && cchNew > 0) 
	{	
		ptb = GetRun(0);

		// cchOld should never be nonzero if the text run is empty
		AssertSz(ptb,
			"CTxtPtr::Replace() - Pointer to text block is NULL !");

		ptb->MoveGap(_ich);
		cchInBlock = min((DWORD)cchOld, ptb->_cch - _ich);
		if(cchInBlock > 0)
		{
			cchOld			-= cchInBlock;
			ptb->_cch		-= cchInBlock;
			((CTxtArray *)_prgRun)->_cchText	-= cchInBlock;
		}
		cchNewInBlock = CchOfCb(ptb->_cbBlock) - ptb->_cch;

		// if there's room for a gap, leave one
		if(cchNewInBlock > cchGapInitial)
			cchNewInBlock -= cchGapInitial;

		if(cchNewInBlock > cchNew)
			cchNewInBlock = cchNew;

		if(cchNewInBlock > 0)
		{
			CopyMemory(ptb->_pch + _ich, pch, CbOfCch(cchNewInBlock));
			cchNew			-= cchNewInBlock;
			_cp				+= cchNewInBlock;
			_ich			+= cchNewInBlock;
			pch				+= cchNewInBlock;
			cchAdded		+= cchNewInBlock;
			ptb->_cch		+= cchNewInBlock;
			ptb->_ibGap		+= CbOfCch(cchNewInBlock);
			((CTxtArray *)_prgRun)->_cchText	+= cchNewInBlock;
		}
	   	if(_iRun >= Count() - 1 || !cchOld )
		   	break;

		// go to next block
		_iRun++;
   		_ich = 0;
	}

	if(cchNew > 0)
		cchAdded += InsertRange(cchNew, pch);
	else if(cchOld > 0)
		DeleteRange(cchOld);
	
	return cchAdded;
}

/*
 *	CTxtPtr::HandleReplaceRangeUndo (cchOld, cchNew, pch, publdr)
 *
 *	@mfunc
 *		worker function for ReplaceRange.  Figures out what will happen in
 *		the replace range call and creates the appropriate anti-events
 *
 *	@devnote
 *		We first check to see if our replace range data can be merged into
 *		an existing anti-event.  If it can, then we just return.
 *		Otherwise, we copy the deleted characters into an allocated buffer
 *		and then create a ReplaceRange anti-event.
 *
 *		In order to handle ordering problems between formatting and text
 *		anti-events (that is, text needs to exist before formatting can
 *		be applied), we have any formatting anti-events passed to us first.
 */
void CTxtPtr::HandleReplaceRangeUndo( 
	DWORD			cchOld, //@parm Count of characters to delete
	DWORD			cchNew, //@parm Count of new characters to add
	IUndoBuilder *	publdr,	//@parm Undo builder to receive anti-event
	IAntiEvent *	paeCF,	//@parm char formatting AE
	IAntiEvent *	paePF )	//@parm paragraph formatting AE
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::HandleReplaceRangeUndo");

	_TEST_INVARIANT_

	IAntiEvent *pae = publdr->GetTopAntiEvent();
	TCHAR *		pch = NULL;

	if( pae )
	{
		SimpleReplaceRange	sr;
		sr.cpMin = _cp;
		sr.cpMax = _cp + cchNew;
		sr.cchDel = cchOld;
	
#ifdef PWD_JUPITER // GuyBark Jupiter 18411: Don't attempt to merge events during an undo
		if( !(publdr->_bNoAttemptToMerge) && pae->MergeData(MD_SIMPLE_REPLACERANGE, &sr) == NOERROR )
#else
		if( pae->MergeData(MD_SIMPLE_REPLACERANGE, &sr) == NOERROR )
#endif // PWD_JUPITER
		{
			// if the data was merged successfully, then we do
			// not need these anti-events
			if( paeCF )
			{
				DestroyAEList(paeCF);
			}
			if( paePF )
			{
				DestroyAEList(paePF);
			}

			// we've done everything we need to.  
			return;
		}
	}

	// allocate a buffer and grab the soon-to-be deleted
	// text (if necessary)

	if( cchOld > 0 )
	{
		pch = new TCHAR[cchOld];
		if( pch )
		{
			GetText(cchOld, pch);
		}
		else
		{
			cchOld = 0;
		}
	}

	// the new range will exist from our current position plus
	// cchNew (because everything in cchOld gets deleted)

	pae = gAEDispenser.CreateReplaceRangeAE(_ped, _cp, _cp + cchNew, 
			cchOld, pch, paeCF, paePF);

	if( !pae )
	{
		delete [] pch;
	}

	if( pae )
	{
		publdr->AddAntiEvent(pae);
	}
}

/*
 *	CTxtPtr::InsertRange(cch, pch)
 *	
 *	@mfunc
 *		Insert a range of characters at this text pointer			
 *	
 *	@rdesc
 *		Count of characters successfully inserted
 *	
 *	@comm Side Effects: <nl>
 *		moves this text pointer to end of inserted text <nl>
 *		moves the text block array <nl>
 */
DWORD CTxtPtr::InsertRange (
	DWORD cch, 				//@parm length of text to insert
	TCHAR const *pch)		//@parm text to insert
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::InsertRange");

	_TEST_INVARIANT_

	DWORD cchSave = cch;
	DWORD cchInBlock;
	DWORD cchFirst;
	DWORD cchLast = 0;
	DWORD ctbNew;
	CTxtBlk *ptb;
	
	// Ensure text array is allocated
	if(!Count())
	{
		LONG	cbSize = -1;

		// If we don't have any blocks, allocate first block to be big enuf
		// for the inserted text *only* if it's smaller than the normal block
		// size. This allows us to be used efficiently as a display engine
		// for small amounts of text.

		if( cch < CchOfCb(cbBlockInitial) )
			cbSize = CbOfCch(cch);

		if( !((CTxtArray *)_prgRun)->AddBlock(0, cbSize) )
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto done;
		}
	}

	ptb = GetRun(0);
	cchInBlock = CchOfCb(ptb->_cbBlock) - ptb->_cch;
	AssertSz(ptb->_cbBlock <= cbBlockMost, "block too big");

	// try and resize without splitting...
	if(cch > cchInBlock &&
		cch <= cchInBlock + CchOfCb(cbBlockMost - ptb->_cbBlock))
	{
		if( !ptb->ResizeBlock(min(cbBlockMost,
				CbOfCch(ptb->_cch + cch + cchGapInitial))) )
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto done;
		}
		cchInBlock = CchOfCb(ptb->_cbBlock) - ptb->_cch;
	}
	if(cch <= cchInBlock)
	{
		// all fits into block without any hassle

		ptb->MoveGap(_ich);
		CopyMemory(ptb->_pch + _ich, pch, CbOfCch(cch));
		_cp				+= cch;					// *this points at end of
		_ich			+= cch;					//  insertion
		ptb->_cch		+= cch;
		((CTxtArray *)_prgRun)->_cchText	+= cch;
		ptb->_ibGap		+= CbOfCch(cch);

		return cch;
	}

	// won't all fit in this block

	// figure out best division into blocks
	TxDivideInsertion(cch, _ich, ptb->_cch - _ich,&cchFirst, &cchLast);

	// Subtract cchLast up front so return value isn't negative
	// if SplitBlock() fails
	cch -= cchLast;	// don't include last block in count for middle blocks

	// split block containing insertion point

	// ***** moves _prgtb ***** //
	if(!((CTxtArray *)_prgRun)->SplitBlock(_iRun, _ich, cchFirst, cchLast,
		_ped->IsStreaming()))
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		goto done;
	}
	ptb = GetRun(0);			// recompute ptb after (*_prgRun) moves

	// copy into first block (first half of split)
	if(cchFirst > 0)
	{
		AssertSz(ptb->_ibGap == CbOfCch(_ich), "split first gap in wrong place");
		AssertSz(cchFirst <= CchOfCb(ptb->_cbBlock) - ptb->_cch, "split first not big enough");

		CopyMemory(ptb->_pch + _ich, pch, CbOfCch(cchFirst));
		cch				-= cchFirst;
		pch				+= cchFirst;
		_ich			+= cchFirst;
		ptb->_cch		+= cchFirst;
		((CTxtArray *)_prgRun)->_cchText	+= cchFirst;
		ptb->_ibGap		+= CbOfCch(cchFirst);
	}

	// copy into middle blocks
	// FUTURE: (jonmat) I increased the size for how large a split block
	// could be and this seems to increase the performance, we should test
	// the block size difference on a retail build, however. 5/15/1995
	ctbNew = cch / cchBlkInsertmGapI /* cchBlkInitmGapI */;
	if(ctbNew <= 0 && cch > 0)
		ctbNew = 1;
	for(; ctbNew > 0; ctbNew--)
	{
		cchInBlock = cch / ctbNew;
		AssertSz(cchInBlock > 0, "nothing to put into block");

		// ***** moves _prgtb ***** //
		if(!((CTxtArray *)_prgRun)->AddBlock(++_iRun, 
			CbOfCch(cchInBlock + cchGapInitial)))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			BindToCp(_cp);	//force a rebind;
			goto done;
		}
		// NOTE: next line intentionally advances ptb to next CTxtBlk

		ptb = GetRun(0);
		AssertSz(ptb->_ibGap == 0, "New block not added correctly");

		CopyMemory(ptb->_pch, pch, CbOfCch(cchInBlock));
		cch				-= cchInBlock;
		pch				+= cchInBlock;
		_ich			= cchInBlock;
		ptb->_cch		= cchInBlock;
		((CTxtArray *)_prgRun)->_cchText	+= cchInBlock;
		ptb->_ibGap		= CbOfCch(cchInBlock);
	}
	AssertSz(cch == 0, "Didn't use up all text");

	// copy into last block (second half of split)
	if(cchLast > 0)
	{
		AssertSz(_iRun < Count()-1, "no last block");
		ptb = Elem(++_iRun);
		AssertSz(ptb->_ibGap == 0,	"split last gap in wrong place");
		AssertSz(cchLast <= CchOfCb(ptb->_cbBlock) - ptb->_cch,
									"split last not big enuf");

		CopyMemory(ptb->_pch, pch, CbOfCch(cchLast));
		// don't subtract cchLast from cch; it's already been done
		_ich			= cchLast;
		ptb->_cch		+= cchLast;
		((CTxtArray *)_prgRun)->_cchText	+= cchLast;
		ptb->_ibGap		= CbOfCch(cchLast);
		cchLast = 0;						// Inserted all requested chars
	}

done:
	AssertSz(cch + cchLast >= 0, "we should have inserted some characters");
	AssertSz(cch + cchLast <= cchSave, "don't insert more than was asked for");

	cch = cchSave - cch - cchLast;			// # chars successfully inserted
	_cp += cch;

	AssertSz (GetTextLength() == 
				((CTxtArray *)_prgRun)->GetCch(), 
				"CTxtPtr::InsertRange(): _prgRun->_cchText messed up !");
	return cch;
}


/*
 *	TxDivideInsertion(cch, ichBlock, cchAfter, pcchFirst, pcchLast)
 *	
 *	@func
 *		Find best way to distribute an insertion	
 *	
 *	@rdesc
 *		nothing
 */
LOCAL void TxDivideInsertion(
	DWORD cch, 				//@parm length of text to insert
	DWORD ichBlock, 		//@parm offset within block to insert text
	DWORD cchAfter,			//@parm length of text after insertion in block
	DWORD *pcchFirst, 		//@parm exit: length of text to put in first block
	DWORD *pcchLast)		//@parm exit: length of text to put in last block
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "TxDivideInsertion");

	DWORD cchFirst = max(0, (LONG)(cchBlkCombmGapI - ichBlock));
	DWORD cchLast  = max(0, (LONG)(cchBlkCombmGapI - cchAfter));
	DWORD cchPartial;
	DWORD cchT;

	// Fill first and last blocks to min block size if possible

	cchFirst = min(cch, cchFirst);
	cch		-= cchFirst;
	cchLast = min(cch, cchLast);
	cch		-= cchLast;

	// How much is left over when we divide up the rest?
	cchPartial = cch % cchBlkInsertmGapI;
	if(cchPartial > 0)
	{
		// Fit as much as the leftover as possible in the first and last
		// w/o growing the first and last over cbBlockInitial
		cchT		= max(0, (LONG)(cchBlkInsertmGapI - ichBlock - cchFirst));
		cchT		= min(cchT, cchPartial);
		cchFirst	+= cchT;
		cch			-= cchT;
		cchPartial	-= cchT;
		if(cchPartial > 0)
		{
			cchT	= max(0, (LONG)(cchBlkInsertmGapI - cchAfter - cchLast));
			cchT	= min(cchT, cchPartial);
			cchLast	+= cchT;
		}
	}
	*pcchFirst = cchFirst;
	*pcchLast = cchLast;
}


/*
 *	CTxtPtr::DeleteRange(cch)
 *	
 *	@mfunc
 *		Delete cch characters starting at this text pointer		
 *	
 *	@rdesc
 *		nothing
 *	
 *	@comm Side Effects:	<nl>
 *		moves text block array
 */
void CTxtPtr::DeleteRange(
	DWORD cch)		//@parm length of text to delete
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::DeleteRange");

	_TEST_INVARIANT_

	DWORD		cchInBlock;
	DWORD		ctbDel = 0;					// Default no blocks to delete
	DWORD		itb;
	CTxtBlk *	ptb = GetRun(0);

	AssertSz(ptb,
		"CTxtPtr::DeleteRange: want to delete, but no text blocks");

	if (cch > GetTextLength() - _cp)	// Don't delete beyond end of story
		cch = GetTextLength() - _cp;

	((CTxtArray *)_prgRun)->_cchText -= cch;

	// remove from first block
	ptb->MoveGap(_ich);
	cchInBlock = min(cch, ptb->_cch - _ich);
	cch -= cchInBlock;
	ptb->_cch -= cchInBlock;

#ifdef DEBUG
	((CTxtArray *)_prgRun)->Invariant();
#endif // DEBUG


	for(itb = ptb->_cch ? _iRun + 1 : _iRun;
			cch && cch >= Elem(itb)->_cch; ctbDel++, itb++)
	{
		// More to go: scan for complete blocks to remove
		cch -= Elem(itb)->_cch;
	}

	if(ctbDel)
	{
		// ***** moves (*_prgRun) ***** //
		itb -= ctbDel;
		((CTxtArray *)_prgRun)->RemoveBlocks(itb, ctbDel);
	}


	// remove from last block
	if(cch > 0)
	{
		ptb = Elem(itb);
		AssertSz(cch < ptb->_cch, "last block too small");
		ptb->MoveGap(0);
		ptb->_cch -= cch;
#ifdef DEBUG
		((CTxtArray *)_prgRun)->Invariant();
#endif // DEBUG

	}
	((CTxtArray *)_prgRun)->CombineBlocks(_iRun);

	if(_iRun >= Count() || !Elem(_iRun)->_cch)
		BindToCp(_cp);					// Empty block: force tp rebind

	AssertSz (GetTextLength() == 
				((CTxtArray *)_prgRun)->GetCch(), 
				"CTxtPtr::DeleteRange(): _prgRun->_cchText messed up !");
}

/*
 *	CTxtPtr::FindText (cpMost, dwFlags, pch, cchToFind)
 *
 *	@mfunc
 *		Find the text string <p pch> of length <p cchToFind> starting at this
 *		text pointer. If found, move this text pointer to the end of the
 *		matched string and return the cp of the first character of the matched
 *		string.  If not found, return -1 and don't change this text ptr.
 *	
 *	@rdesc
 *		character position of first match
 *		<lt> 0 if no match
 */
LONG CTxtPtr::FindText (
	LONG		cpLimit, 	//@parm limit of search or <lt> 0 for end of text
	DWORD		dwFlags, 	//@parm FR_MATCHCASE	case must match <nl>
							//		FR_WHOLEWORD	match must be a whole word
	TCHAR const *pch, 		//@parm text to search for
	const DWORD cchToFind)	//@parm length of text to search for
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindText");

	_TEST_INVARIANT_

	BOOL		bTemp;					// Used in find-first-char loops
	LONG		cch;
	const DWORD	cchAdjText	= _ped->GetAdjustedTextLength();
	LONG		cchChunk, cchChunkForward = 0;
	LONG		cchLeft;
	LONG		cchLimitFirst;			
	const DWORD	cchText		= GetTextLength();
	LONG		cchUnmatched;
	LONG		cchValid;
	TCHAR		chFirst;
	LONG		cpMatch;
	BOOL		fIgnoreCase		= !(FR_MATCHCASE & dwFlags);
	BOOL		fSearchForward	= FR_DOWN & dwFlags;
	BOOL		fWholeWord;
	const int	Direction = fSearchForward ? 1 : -1; // Which dir to step?
	const TCHAR *pchCurrent;
	const TCHAR *pchSave;
	const TCHAR *pchStart;				// ptr to start of new block search
	CTxtPtr		tp(*this);				// tp used to walk CTxtBlk chunks
	EDITWORDBREAKPROC pfnWB = _ped->_pfnWB;

	if( !cchToFind )
	{
		return -1;
	}

	if ( fSearchForward )
	{
		if((DWORD)cpLimit > cchText)			// NB: catches cpLimit < 0 too
			cpLimit = cchText;
		cch = cpLimit - _cp;
	}
	else
	{
		if((DWORD)cpLimit > _cp)				// NB: catches cpLimit < 0 too
			cpLimit = 0;
		cch = _cp - cpLimit;   
	}
	if((LONG)cchToFind > cch)
		return -1;

	chFirst = pch[0];
	if(fIgnoreCase)
	{
		// Read Win32 spec to understand the following strange casts
		chFirst = TCHAR (CharLower ((LPTSTR) chFirst));
	}

	cchLimitFirst = cch - (LONG)cchToFind + 1;				 
	if (! fSearchForward )	
	{
		 tp.AdvanceCp(-(LONG)cchToFind + 1);
	}

	while(cchLimitFirst > 0)
	{
		// Fetch how many characters are remaining for this pchCurrent. If
		// searching in reverse, fetch how many characters there are to the
		// start of pchCurrent.
		// NOTE: strings pointed to by pchCurrent are delineated by blocks
		//		 or block gaps.

		if ( fSearchForward )
		{
			pchCurrent = tp.GetPch(cchChunk);
			cchChunk = min(cchLimitFirst, cchChunk);
			tp.AdvanceCp(cchChunk);
			cchChunkForward = cchChunk;					// cch to tp.GetCp()
		}
		else
		{
			pchCurrent = tp.GetPchReverse(cchChunk, &cchChunkForward);
			pchCurrent--;
			cchChunkForward++;
			cchChunk = min(cchLimitFirst, cchChunk);
			tp.AdvanceCp(-cchChunk);
		}

		if( !pchCurrent )
		{
			return -1;
		}

		pchSave = pchStart = pchCurrent;				// Save starting ptr
		cchLimitFirst -= cchChunk;
		while(cchChunk > 0)
		{
			// Match first char
			if ( fIgnoreCase )
				do {
					bTemp = ( 2 == CompareString(LOCALE_USER_DEFAULT,
						NORM_IGNORECASE | NORM_IGNOREWIDTH, 
						pchCurrent, 1, &chFirst, 1) );
					pchCurrent += Direction;
				} while(!bTemp && --cchChunk > 0);
			else
				do {
					bTemp = TCHAR( *pchCurrent ) == chFirst;
					pchCurrent += Direction;
				} while(!bTemp && --cchChunk > 0);
			
			cchChunkForward += pchSave - pchCurrent;	// cch still valid
														//  for pchCurrent
			pchSave = pchCurrent;
			if(!fSearchForward)							// Setup is for
			{											//  forward match
				pchCurrent += 2;
				cchChunkForward -= 2;
			}						

			// Found first character; now find rest of string. Common routine
			// for forward and backward: in both cases, we match the rest of
			// the string going forward.

			if(cchChunk > 0)
			{
				cchChunk--;
				cchLeft = min((LONG)cchToFind - 1, cchChunkForward);

				if( fIgnoreCase )
				{
					if ( !cchLeft )
					{
						// nothing to compare, we find a match
						cchUnmatched = FALSE;
					}
					else
					{
						// Note: CompareString returns 2 if the two strings matched.
						cchUnmatched = !( 2 == CompareString( LOCALE_USER_DEFAULT, 
							NORM_IGNORECASE | NORM_IGNOREWIDTH, 
							&pch[1], cchLeft, pchCurrent, cchLeft) ) ;
					}
				}
				else
				{
					cchUnmatched = wcsncmp( &pch[1], pchCurrent, cchLeft);
				}

				if(!cchUnmatched)					
				{										
					// Matched whole (or partial) string. Calculate potential
					// match cpFirst
					cpMatch = tp.GetCp() + (fSearchForward ? 
								( -cchChunk - 1) : cchChunk );

					fWholeWord = (dwFlags & FR_WHOLEWORD) &&
								 ((DWORD)cpMatch + cchToFind <= cchAdjText);

					// Check for word delimiter before the match
					if((dwFlags & FR_WHOLEWORD) && cpMatch > 0)
					{
						CTxtPtr tpT(_ped, cpMatch - 1);

						if((pfnWB((LPTSTR)tpT.GetPch(cchValid), 0,
							sizeof(TCHAR), WB_CLASSIFY) & WBF_CLASS) == 0)
						{ goto no_match; }
					}

					// doesn't attempt to check next char if there isn't one

					if((LONG)cchToFind - 1 < cchChunkForward)
					{
						// pchCurrent is valid for one or more chars, so are
						// either done or can check for word delimeter

						// CTxtPtr tpT(_ped, cpMatch + cchToFind);
						// TCHAR* temptemp=tpT.GetPch();

						if (!fWholeWord || 
							(pfnWB((LPTSTR)(pchCurrent + cchLeft), 0,
								sizeof(TCHAR),WB_CLASSIFY) & WBF_CLASS) != 0)
						{ goto done; }
					}
					else	// Need to compare across chunks
						if(tp.MatchAcrossChunks(dwFlags, cchChunkForward,
								pch + 1, cchToFind - 1, fWholeWord, pfnWB))
					{ goto done; }
				}
no_match:
				pchCurrent = pchSave;
			}
		}
	}
	return -1;								// No match

done:
	if (cpMatch >= 0)
		SetCp(cpMatch + cchToFind);			// Match: set this tp just past
											//  match. When we have wildcard
	return cpMatch;							//  chars, we may need something
}											//  more general than cchToFind

/* 
 *	CTxtPtr::MatchAcrossChunks (dwFlags, cchChunkForward, pch, cchToMatch, 
 *								fWholeWord, pfnWB)
 *
 *	@mfunc		helper function for FindText
 *
 *	@rdesc		TRUE if match found
 *
 *	@comm		"*this" points to char to check against pch[cchMatched]
 */
#define __OPT_PLAT_FLAG (defined (ARMV4T))
#define __OPT_VER_OFF
#define	__OPT_BUGNUMSTRING	"26316"
//#include <optimizer.h>

INLINE BOOL CTxtPtr::MatchAcrossChunks (
	DWORD		 dwFlags,			//@parm Search flags
	DWORD		 cchMatched,		//@parm Amount already matched
	const TCHAR *pch, 				//@parm String to match against
	DWORD		 cchToMatch,		//@parm Total cch to match 
	BOOL		 fWholeWord,		//@parm Match against whole word
	EDITWORDBREAKPROC pfnWB) const	//@parm Word break proc
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::MatchAcrossChunks");

	_TEST_INVARIANT_

	LONG		 cchChunk;
	LONG		 cchValid;
	const TCHAR *pchCurrent;
	CTxtPtr		 tp(*this);			// tp already points to next 
									// chunk if searching forward.

	BOOL	fSearchForward	= (FR_DOWN & dwFlags) != 0;
	BOOL	fIgnoreCase		= (FR_MATCHCASE & dwFlags) == 0;

	if ( !fSearchForward )
	{
		// even though the overall search is in reverse, the compare is done
		// forward.
		tp.GetPch(cchValid);		// searching in reverse leaves _cp at previous chunk
		tp.AdvanceCp(cchValid);		// line it back up with searching forward.
	}

	pch += cchMatched;							// Bypass chars already matched
	cchToMatch -= cchMatched;
	if(!cchToMatch)
		pchCurrent = tp.GetPch(cchValid);

	while(cchToMatch > 0)
	{
		pchCurrent = tp.GetPch(cchChunk);
		if(cchChunk <= 0)
			return FALSE;
		cchChunk = min(cchChunk, (LONG)cchToMatch);
		tp.AdvanceCp(cchChunk);
		cchToMatch -= cchChunk;

		if ( fIgnoreCase )
		{
#ifdef PWD_JUPITER
			// GuyBark Jupiter 50123: fIgnoreCase means ignore width too. 
			// So take the same action as elsewhere when comparing strings.
			// Both pch and pchCurrent point to strings that are at least
			// cchChunk long.

			// Note: CompareString returns 2 if the two strings matched.
			if(CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_IGNOREWIDTH, 
			                 pchCurrent, cchChunk, pch, cchChunk) == 2)
			{
			    // Strings are close enough! Set things up as they would be set
			    // up using the ifdef'd out comparison test. Don't actually 
			    // need to set up pchCurrent here, but can't do any harm.

			    pch        += cchChunk;
			    pchCurrent += cchChunk;
			
			    cchChunk = 0;
			}

			// If the comparison failed, we don't need to mess with the pointers here,
			// as we're going to leave anyway below because cchChunk is still non-zero.
#else
			do {
				if ( TCHAR (CharLower ( (LPTSTR) *pchCurrent++ )) != TCHAR (CharLower ( (LPTSTR) *pch++ )) )
					break;
			} while( --cchChunk > 0);
#endif // PWD_JUPITER
		}
		else
		{
			do {
				if ( *pchCurrent++ != *pch++ )
					break;
			} while( --cchChunk > 0);
		}
				
		if(cchChunk > 0)
			return FALSE;
	}

	return !fWholeWord || (pfnWB((LPTSTR)tp.GetPch(cchValid), 0,
							sizeof(TCHAR), WB_CLASSIFY)	& WBF_CLASS);
}

#define __OPT_PLAT_FLAG (defined (ARMV4T))
#define __OPT_VER_RESTORE
#define	__OPT_BUGNUMSTRING	"26316"
//#include <optimizer.h>

/*
 *	CTxtPtr::FindEOP(cchMax)
 *	
 *	@mfunc
 *		Find EOP mark in a range within cchMax chars from this text pointer
 *		and position *this after it.  If no EOP is found and cchMax is not
 *		enough to reach the start or end of the story, leave this text ptr
 *		alone and return 0.  If no EOP is found and cchMax is sufficient to
 *		reach the start or end of the story, position this text ptr at the
 *		beginning/end of document (BOD/EOD) for cchMax <lt>/<gt> 0,
 *		respectively, that is, BOD and EOD are treated as a BOP and an EOP,
 *		respectively.	
 *
 *	@rdesc
 *		Return cch this text ptr is moved
 *
 *	@devnote
 *		This function assumes that this text ptr isn't in middle of a CRLF
 *		or CRCRLF (found only in RichEdit 1.0 compatibility mode).  Changing
 *		the for loop could speed up ITextRange MoveUntil/While substantially.
 */
LONG CTxtPtr::FindEOP (
	LONG  cchMax,		//@parm Max signed count of chars to search
	LONG *pResults)		//@parm Flags saying if EOP and CELL are found
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindEOP");

	LONG		cch = 0, cchStart;			// cch's for scans
	unsigned	ch;							// Current char
	DWORD		cpSave	= _cp;				// Save _cp for returning delta
	LONG		iDir	= 1;				// Default forward motion
	const TCHAR*pch;						// Used to walk text chunks
	LONG		Results = 0;				// Nothing found yet
	CTxtPtr		tp(*this);					// tp to search text with

	if(cchMax < 0)							// Backward search
	{
		iDir = -1;							// Backward motion
		cchMax = -cchMax;					// Make max count positive
		cch = tp.AdjustCpCRLF();			// If in middle of CRLF or
		if(!cch && IsAfterEOP())			//  CRCRLF, or follow any EOP,
			cch = tp.BackupCpCRLF();		//  backup before EOP
		cchMax += cch;
	}

	while(cchMax > 0)						// Scan until get out of search
	{										//  range or match an EOP
		pch = iDir > 0						// Point pch at contiguous text
			? tp.GetPch(cch)				//  chunk going forward or
			: tp.GetPchReverse(cch);		//  going backward

		if(!pch)							// No more text to search
			break;

		if(iDir < 0)						// Going backward, point at
			pch--;							//  previous char

		cch = min(cch, cchMax);				// Limit scan to cchMax chars
		for(cchStart = cch; cch; cch--)		// Scan chunk for EOP
		{
                    if (!pch)
                        break;
                     ch = *pch;
			if(ch == CELL)
				Results |= FEOP_CELL;		// Note that CELL was found

			if(IsEOP(ch))					// Note that EOP was found
			{								// Going forward, cch may = 0
				Results |= FEOP_EOP;		
				break;
			}
			pch += iDir;
		}
		cchStart -= cch;					// Get cch of chars passed by
		cchMax -= cchStart;					// Update cchMax
		tp.AdvanceCp(iDir*cchStart);		// Update tp
		if((Results & FEOP_EOP) || !(pch))				// Found an EOP
			break;
	}										// Continue with next chunk

	if(pResults)							// Report whether EOP and CELL
		*pResults = Results;				//  were found

	if(Results & FEOP_EOP || !tp.GetCp())	// Found EOP or cp is at beginning
	{										//  of story:
		SetCp(tp.GetCp());					//  set _cp = cp
		if(GetChar() == LF)					// In case there's a LF there,
			AdvanceCp(1);					//  bypass it
		else if(iDir > 0)					// Position this ptr just after
			AdvanceCpCRLF();				//  EOP
	}
	return _cp - cpSave;					// Return cch this tp moved
}

/*
 *	CTxtPtr::FindBOSentence(cch)
 *	
 *	@mfunc
 *		Find beginning of sentence in a range within cch chars from this text
 *		pointer and	position *this at it.  If no sentence beginning is found,
 *		position *this at beginning of document (BOD) for cch <lt> 0 and
 *		leave *this unchanged for cch >= 0.
 *
 *	@rdesc
 *		Count of chars moved *this moves
 *
 *	@comm 
 *		This routine defines a sentence as a character string that ends with
 *		period followed by at least one whitespace character or the EOD.  This
 *		should be replacable so that other kinds of sentence endings can be
 *		used.  This routine also matches initials like "M. " as sentences.
 *		We could eliminate those by requiring that sentences don't end with
 *		a word consisting of a single capital character.  Similarly, common
 *		abbreviations like "Mr." could be bypassed.  To allow a sentence to
 *		end with these "words", two blanks following a period could be used
 *		to mean an unconditional end of sentence.
 */
LONG CTxtPtr::FindBOSentence (
	LONG cch)			//@parm max signed count of chars to search
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindBOSentence");

	_TEST_INVARIANT_

	LONG	cchWhite = 0;						// No whitespace chars yet
	DWORD	cp;
	DWORD	cpSave	 = _cp;						// Save value for return
	LONG	cpLim	 = cpSave + cch;			// Limiting cp for search
	BOOL	fST;								// TRUE if sent terminator
	LONG	iDir	 = cch > 0 ? 1 : -1;		// AdvanceCp() increment
	CTxtPtr	tp(*this);							// tp to search with

	if(iDir > 0)								// If going forward in white
		while(IsWhiteSpace(tp.GetChar()) &&		//  space, backup to 1st non
				tp.AdvanceCp(-1)) ;				//  whitespace char (in case
												//  inside sentence ending)
	while(iDir > 0 || tp.AdvanceCp(-1))			// Need to back up if finding
	{											//  backward
		for(fST = FALSE; cch; cch -= iDir)		// Find sentence terminator
		{
			fST = IsSentenceTerminator(tp.GetChar());
			if(fST || !tp.AdvanceCp(iDir))
				break;
		}
		if(!fST)								// If FALSE, we	ran out of
			break;								//  chars

		while(IsWhiteSpace(tp.NextChar()) && cch)
		{										// Bypass a span of blank
			cchWhite++;							//  chars
			cch--;
		}

		if(cchWhite && (cch >= 0 || tp._cp < cpSave))// Matched new sentence
			break;								//  break

		if(cch < 0)								// Searching backward
		{
			tp.AdvanceCp(-cchWhite - 1);		// Back up to terminator
			cch += cchWhite + 1;				// Fewer chars to search
		}
		cchWhite = 0;							// No whitespace yet for next
	}											//  iteration

	cp = tp._cp;
	if(cchWhite || !cp || cp == GetTextLength())// If sentence found or got
		SetCp(cp);								//  start/end of story, set
												//  _cp to tp's
	return _cp - cpSave;						// Tell caller cch moved
}

/*
 *	CTxtPtr::IsAtBOSentence()
 *	
 *	@mfunc
 *		Return TRUE iff *this is at the beginning of a sentence (BOS) as
 *		defined in the description of the FindBOSentence(cch) routine
 *
 *	@rdesc
 *		TRUE iff this text ptr is at the beginning of a sentence
 */
BOOL CTxtPtr::IsAtBOSentence()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAtBOSentence");

	if(!_cp)									// Beginning of story is an
		return TRUE;							//  unconditional beginning
												//  of sentence
	unsigned ch = GetChar();

	if (IsWhiteSpace(ch) ||						// Proper sentences don't
		IsSentenceTerminator(ch))				//  start with whitespace or
	{											//  sentence terminators
		return FALSE;
	}
												
	LONG	cchWhite;
	CTxtPtr tp(*this);							// tp to walk preceding chars

	for(cchWhite = 0;							// Backspace over possible
		IsWhiteSpace(ch = tp.PrevChar());		//  span of whitespace chars
		cchWhite++) ;

	return cchWhite && IsSentenceTerminator(ch);
}

/*
 *	CTxtPtr::IsAtBOWord()
 *	
 *	@mfunc
 *		Return TRUE iff *this is at the beginning of a word, that is,
 *		_cp = 0 or the char at _cp is an EOP, or
 *		FindWordBreak(WB_MOVEWORDRIGHT) would break at _cp.
 *
 *	@rdesc
 *		TRUE iff this text ptr is at the beginning of a Word
 */
BOOL CTxtPtr::IsAtBOWord()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::IsAtBOWord");

	if(!_cp || IsAtEOP())					// Story beginning is also
		return TRUE;						//  a word beginning

	CTxtPtr tp(*this);
	tp.AdvanceCp(-1);
	tp.FindWordBreak(WB_MOVEWORDRIGHT);
	return _cp == tp._cp;
}

/*
 *	CTxtPtr::CheckMoveGap ( DWORD cchLine )
 *	
 *	@mfunc
 *		Helper function for rendering, so that the renderer wont bump
 *		up against a gap when it is trying to render text with overhangs.
 *	
 *	@rdesc
 *		We are always called from the renderer when we are at the start of
 *		a line, so _ich is already at the line's start. We check cchLine
 *		to see if it crosses the gap, and if so, we move the gap to the
 *		beginning of the line.
 *
 *	@comm
 *		This code does not handle the case when the line crosses a block
 *		boundry.
 */
void
CTxtPtr::CheckMoveGap (
	DWORD cchLine )	 //@parm cch of the line about to be rendered.
{
 	CTxtBlk *ptb;
	DWORD	ichGap;

	Assert (IsValid());

 	ptb = GetRun(0);

	ichGap = CchOfCb(ptb->_ibGap);		// if line crosses block gap.
	if ( _ich < ichGap && ichGap < (_ich + cchLine) )
	{
		ptb->MoveGap(_ich);				// move gap to line start.
	}
}

/*
 *	CTxtPtr::FindExact(cchMax, pch)
 *	
 *	@mfunc
 *		Find exact text match for null-terminated string pch in a range
 *		starting at this text pointer. Position this just after matched
 *		string and return cp at start of string, i.e., same as FindText().
 *	
 *	@rdesc
 *		Return cp of first char in matched string and *this pointing at cp
 *		just following matched string.  Return -1 if no match
 *
 *	@comm
 *		Much faster than FindText, but still a simple search, i.e., could
 *		be improved.
 *
 *		FindText can delegate to this search for search strings in which
 *		each char can only match itself.
 */
LONG CTxtPtr::FindExact (
	LONG	cchMax,		//@parm signed max # of chars to search 
	TCHAR *	pch)		//@parm ptr to null-terminated string to find exactly
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::FindExact");

	_TEST_INVARIANT_

	LONG	cch, cchStart;
	LONG	cchValid;
	DWORD	cchText = GetTextLength();
	LONG	cpMatch;
	LONG	iDir = 1;						// Default for forward search
	const TCHAR	*pc;
	CTxtPtr	tp(*this);						// tp to search text with

	if(!*pch)
		return -1;							// Signal null string not found

	if(cchMax < 0)							// Backward search
	{
		iDir = -1;
		cchMax = -cchMax;					// Make count positive
	}

	while(cchMax > 0)
	{
		if(iDir > 0)
		{
			if(tp.GetCp() >= cchText)		// Can't go further
				break;
			pc  = tp.GetPch(cchValid);		// Characters we can search w/o
			cch = cchValid; 				//  encountering block end/gap,
		}									//  i.e., stay within text chunk
		else
		{
			if(!tp.GetCp())					// Can't back up any more
				break;
			tp.AdvanceCp(-1);
			pc  = tp.GetPchReverse(cchValid);
			cch = cchValid + 1;
		}

		cch = min(cch, cchMax);
		if(!cch || !pc)
			break;							// No more text to search

		for(cchStart = cch;					// Find first char
			cch && *pch != *pc; cch--)		// Most execution time is spent
		{									//  in this loop going forward or
			pc += iDir;						//  backward. x86 rep scasb/scasw
		}									//  are faster

		cchStart -= cch;
		cchMax	 -= cchStart;				// Update cchMax
		tp.AdvanceCp( iDir*(cchStart));		// Update tp

		if(cch && *pch == *pc)				// Matched first char
		{									// See if matches up to null
			cpMatch = tp.GetCp();			// Save cp of matched first char
			cch = cchMax;
			for(pc = pch;					// Try to match rest of string
				cch && *++pc==tp.NextChar();// Note: this match goes forward
				cch--) ;					//  for both values of iDir
			if(!cch)
				break;						// Not enuf chars for string

			if(!*pc)						// Matched null-terminated string
			{								//  *pch. Set this tp just after
				SetCp(tp.GetCp());			//  matched string and return cp
				return cpMatch;				//  at start
			}
			tp.SetCp(cpMatch + iDir);		// Move to char just following or
		}									//  preceding matched first char
	}										// Up-to-date tp: continue search

	return -1;								// Signal string not found
}

/*
 *	CTxtPtr::NextCharCount(&cch)
 *
 *	@mfunc
 *		Helper function for getting next char and decrementing abs(*pcch)
 *
 *	@rdesc
 *		Next char
 */
TCHAR CTxtPtr::NextCharCount (
	LONG& cch)					//@parm count to use and decrement
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtPtr::NextCharCount");

	LONG	iDelta = (cch > 0) ? 1 : -1;

	if(!cch || !AdvanceCp(iDelta))
		return 0;

	cch -= iDelta;							// Count down or up
	return GetChar();						// Return char at _cp
}

/*
 *	CTxtPtr::Zombie ()
 *
 *	@mfunc
 *		Turn this object into a zombie by NULLing out its _ped member
 */
void CTxtPtr::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtPtr::Zombie");

	_ped = NULL;
	_cp = 0;
	SetToNull();
}

