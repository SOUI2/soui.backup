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
 *	@module	URLSUP.CPP	URL detection support |
 *
 *	Author:	alexgo 4/3/96
 */

#include "_common.h"
#include "_edit.h"
#include "_urlsup.h"
#include "_m_undo.h"
#include "_select.h"
#include "_clasfyc.h"

ASSERTDATA

// arrays for URL detection.  The first array is the protocols
// we support, followed by the "size" of the array.
// NB!! Do _not_ modify these arrays without first making sure
// that the code in ::IsURL is updated appropriately.

const LPCWSTR rgszURL[] = {
	L"http:",
	L"file:",
	L"mailto:",
	L"ftp:",
	L"https:",
	L"gopher:",
	L"nntp:",
	L"prospero:",
	L"telnet:",
	L"news:",
	L"wais:",
	L"outlook:",
};

const LONG rgcchURL[] = {
	5,
	5,
	7,
	4,
	6,
	7,
	5,
	9,
	7,
	5,
	5,
	8
};

const LPCWSTR rgszWWWURL[] = {
  L"www."
};

const LONG rgcchWWWURL[] = {
  4
};

#define MAXURLHDRSIZE	9

// return TRUE is white space or FE chars
inline BOOL IsURLWhiteSpace(WCHAR ch)
{
	return ( IsWhiteSpace(ch) || ch >= 0x03000 );
}

/*
 *	CDetectURL::CDetectURL
 *
 *	@mfunc	constructor; registers this class in the notification manager.
 *
 *	@rdesc	void
 */
CDetectURL::CDetectURL(
	CTxtEdit *ped)		//@parm the edit context to use
{
	CNotifyMgr *pnm = ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Add((ITxNotify *)this);
	}

	_ped = ped;
}

/*
 *	CDetectURL::~CDetectURL
 *
 *	@mfunc	destructor; removes ths class from the notification manager
 *
 *	@rdesc	void
 */
CDetectURL::~CDetectURL()
{
	CNotifyMgr *pnm = _ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Remove((ITxNotify *)this);
	}
}

//
//	ITxNotify	methods
//

/*
 *	CDetectURL::OnPreRelaceRange
 *
 *	@mfunc	called before a change is made
 *
 *	@rdesc	void
 */
void CDetectURL::OnPreReplaceRange(
	DWORD cp,			//@parm start of changes
	DWORD cchDel,		//@parm #of chars deleted
	DWORD cchNew,		//@parm #of chars added
	DWORD cpFormatMin,	//@parm min cp of formatting change
	DWORD cpFormatMax)	//@parm max cp of formatting change
{
	; // don't need to do anything here
}

/*
 *	CDetectURL::OnPostReplaceRange
 *
 *	@mfunc	called after a change has been made to the backing store.  We
 *			simply need to accumulate all such changes
 *
 *	@rdesc	void
 *
 */
void CDetectURL::OnPostReplaceRange(
	DWORD cp,			//@parm start of changes
	DWORD cchDel,		//@parm #of chars deleted
	DWORD cchNew,		//@parm #of chars added
	DWORD cpFormatMin,	//@parm min cp of formatting change
	DWORD cpFormatMax)	//@parm max cp of formatting change
{
	// we don't need to worry about format changes; just data changes
	// to the backing store

	if( cp != INFINITE )
	{
		Assert(cp != CONVERT_TO_PLAIN);
		_adc.UpdateRecalcRegion(cp, cchDel, cchNew);
	}
}

/*
 *	CDetectURL::Zombie ()
 *
 *	@mfunc
 *		Turn this object into a zombie
 */
void CDetectURL::Zombie ()
{

}

/*
 *	CDetectURL::ScanAndUpdate
 *
 *	@mfunc	scans the affect text, detecting new URL's and removing old ones.
 *
 *	@rdesc	void
 *
 *	@comm	The algorithm we use is straightforward: <nl>
 *
 *			1. find the update region and expand out to whitespace in either
 *			direction. <nl>
 *
 *			2. Scan through region word by word (where word is contiguous
 *			non-whitespace). 
 *			
 *			3. Strip these words off punctuation marks. This may be a bit 
 *			tricky as some of the punctuation may be part of the URL itself.
 *			We assume that generally it's not, and if it is, one has to enclose
 *			the URL in quotes, brackets or such. We stop stripping the 
 *			punctuation off the end as soon as we fing the matching bracket.
 *			
 *			4. If it's a URL, enable the effects, if it's
 *			incorrectly labelled as a URL, disabled the effects.
 *
 *			Note that this algorithm will only remove
 */
void CDetectURL::ScanAndUpdate(
	IUndoBuilder *publdr)	//@parm the undo context to use
{
	LONG		cpStart, cpEnd, cp; 
	CTxtRange	rg(*(CTxtRange *)_ped->GetSel());
	const CCharFormat *pcfDefault = _ped->GetCharFormat(-1);
	COLORREF	crDefault = 0;
	URLFLAGS	fUseUnderline = URL_NOUNDERLINE;
	BOOL		fCleanedThisURL;
	BOOL		fCleanedSomeURL = FALSE;

	// clear away some unecessary features of the range that will
	// just slow us down.
	rg.SetIgnoreFormatUpdate(TRUE);
	rg._rpPF.SetToNull();
		
	if( !GetScanRegion(cpStart, cpEnd) )
	{
		return;
	}

	if( pcfDefault )
	{
		crDefault = pcfDefault->crTextColor;
		fUseUnderline = (pcfDefault->dwEffects & CFE_UNDERLINE) ? 
				URL_USEUNDERLINE : URL_NOUNDERLINE;
	}


	rg.Set(cpStart, 0);

	while( (cp = rg.GetCp()) < cpEnd )
	{
		Assert(rg.GetCch() == 0 );
		
		LONG cchAdvance; 

		ExpandToURL(rg, cchAdvance);

		if( rg.GetCch() == 0 )
		{
			break;
		}

		if( IsURL(rg) )
		{
			SetURLEffects(rg, publdr);

			LONG cpNew = rg.GetCp() - rg.GetCch();

			// anything before the detected URL did not really belong to it
			if (rg.GetCp() > cp)
			{
				rg.Set(cp, cp - rg.GetCp());
				CheckAndCleanBogusURL(rg, crDefault, fUseUnderline, 
										fCleanedThisURL, publdr);
				fCleanedSomeURL |= fCleanedThisURL;
			}

			// collapse to the end of the URL range so that ExpandToURL will
			// find the next candidate.
			rg.Set(cpNew, 0);

			// skip to the end of word; this can't be another URL!
			cp = cpNew;
			cchAdvance = -MoveByDelimiters(rg._rpTX, 1, URL_STOPATWHITESPACE, 0);
		}

		if (cchAdvance)
		{	
			rg.Set(cp, cchAdvance);
			CheckAndCleanBogusURL(rg, crDefault, fUseUnderline, 
									fCleanedThisURL, publdr);
			fCleanedSomeURL |= fCleanedThisURL;

			// collapse to the end of scanned range so that ExpandToURL will
			// find the next candidate.
			rg.Set(cp - cchAdvance, 0);
		}
	}

	// if we cleaned some URL, we might need to reset the default format
	if (fCleanedSomeURL && !_ped->GetSel()->GetCch())
		_ped->GetSel()->Update_iFormat(-1);

	return;
}


//
//	PRIVATE methods
//

/*
 *	CDetectURL::GetScanRegion
 *
 *	@mfunc	Gets the region of text to scan for new URLs by expanding the
 *			changed region to be bounded by whitespace
 *
 *	@rdesc	void
 */
BOOL CDetectURL::GetScanRegion(
	LONG&	rcpStart,		//@parm where to put the start of the range
	LONG&	rcpEnd			//@parm where to the the end of the range
)
{
	DWORD cp, cch, adjlength;
	CRchTxtPtr rtp(_ped, 0);
	WCHAR chBracket;
	LONG cchExpand;

	_adc.GetUpdateRegion(&cp, NULL, &cch, NULL, NULL);

	if( cp == INFINITE )
	{
		return FALSE;
	}

	// first, find the start of the region
	rtp.SetCp(cp);
	rcpStart = cp;
	rcpEnd = cp + cch;
	
	// now let's see if we need to expand to the nearest quotation mark
	// we do if we have quotes in our region or we have the LINK bit set
	// on either side of the region that we might need or not need to clear
	BOOL fExpandToBrackets = (rcpEnd - rcpStart ? 
						      GetAngleBracket(rtp._rpTX, rcpEnd - rcpStart) : 0);

	BOOL fKeepGoing = TRUE;	
	while(fKeepGoing)
	{
		fKeepGoing = FALSE;

		// expand left to the entire word
		rtp.SetCp(rcpStart);
		rcpStart += MoveByDelimiters(rtp._rpTX, -1, URL_STOPATWHITESPACE, 0);

		// now the other end
		rtp.SetCp(rcpEnd);
		rcpEnd += MoveByDelimiters(rtp._rpTX, 1, URL_STOPATWHITESPACE, 0);

		// if we have LINK formatting around, we'll need to expand to nearest quotes
		rtp.SetCp(rcpStart);
		rtp._rpCF.AdjustBackward();
		fExpandToBrackets = fExpandToBrackets ||
						(_ped->GetCharFormat(rtp._rpCF.GetFormat())->dwEffects & CFE_LINK);

		rtp.SetCp(rcpEnd);
		rtp._rpCF.AdjustForward();
		fExpandToBrackets = fExpandToBrackets || 
						(_ped->GetCharFormat(rtp._rpCF.GetFormat())->dwEffects & CFE_LINK);

		if (fExpandToBrackets)
		// we have to expand to nearest angle brackets in both directions
		{
			rtp.SetCp(rcpStart);
			chBracket = LEFTANGLEBRACKET;
			cchExpand = MoveByDelimiters(rtp._rpTX, -1, URL_STOPATCHAR, &chBracket);
		
			// did we really hit a bracket?	
			if (chBracket == LEFTANGLEBRACKET)
			{
				rcpStart += cchExpand;
				fKeepGoing = TRUE;
			}

			// same thing, different direction
			rtp.SetCp(rcpEnd);
			chBracket = RIGHTANGLEBRACKET;
			cchExpand =  MoveByDelimiters(rtp._rpTX, 1, URL_STOPATCHAR, &chBracket);

			if (chBracket == RIGHTANGLEBRACKET)
			{
				rcpEnd += cchExpand;
				fKeepGoing = TRUE;
			}

			fExpandToBrackets = FALSE;
		}
	}
		
	if( rcpEnd > (LONG)(adjlength = _ped->GetAdjustedTextLength()) )
	{
		rcpEnd = adjlength;
	}

	return TRUE;
}

/*
 *	CDetectURL::ExpandToURL
 *
 *	@mfunc	skips white space and sets the range to the very next
 *			block of non-white space text. Strips this block off
 *			punctuation marks
 *
 *	@rdesc	void
 */
void CDetectURL::ExpandToURL(
	CTxtRange&	rg,	//@parm the range to move
	LONG &cchAdvance//@parm how much to advance to the next URL from the current cp		
							)	
{
	DWORD cp;
	LONG cch;

	Assert(rg.GetCch() == 0 );

	cp = rg.GetCp();

	// skip white space first, record the advance
	cp  -= (cchAdvance = -MoveByDelimiters(rg._rpTX, 1, 
							URL_EATWHITESPACE|URL_STOPATNONWHITESPACE, 0));
	rg.Set(cp, 0);

	// strip off punctuation marks
	WCHAR chStopChar = URL_INVALID_DELIMITER;

	// skip all punctuation from the beginning of the word
	LONG cchHead = MoveByDelimiters(rg._rpTX, 1, 
							URL_STOPATWHITESPACE|URL_STOPATNONPUNCT, 
							&chStopChar);

	// now skip up to white space (i.e. expand to the end of the word).
	cch = MoveByDelimiters(rg._rpTX, 1, 
						   URL_STOPATWHITESPACE|URL_EATNONWHITESPACE, 0);
	
	// this is how much we want to advance to start loking for the next URL
	// if this does not turn out to be one: one word
	// We increment/decrement  the advance so we can accumulate changes in there
	cchAdvance -= cch;
	WCHAR chLeftDelimiter = chStopChar;

	// check if anything left; if not, it's not interesting -- just return
	Assert(cchHead <= cch);
	if (cch == cchHead)
	{
		rg.Set(cp, -cch);
		return;
	}

	// set to the end of range
	rg.Set(cp + cch, 0);
		
// get the space after so we always clear white space between words
//	cchAdvance -= MoveByDelimiters(rg._rpTX, 1, 
//									URL_EATWHITESPACE|URL_STOPATNONWHITESPACE, 0);

	// and go back while skipping punctuation marks and not finding a match
	// to the left-side encloser

	chStopChar = BraceMatch(chStopChar);
	LONG cchTail = MoveByDelimiters(rg._rpTX, -1, 
							URL_STOPATWHITESPACE|URL_STOPATNONPUNCT|URL_STOPATCHAR, 
							&chStopChar);

	// something should be left of the word, assert that
	Assert(cch - cchHead + cchTail > 0);

	if (chLeftDelimiter == LEFTANGLEBRACKET)
	{ 
		// now do some weird stuff if we stopped at a quote: go forward looking
		// for the enclosing quote, even if there are spaces. As a weak protection
		// mechanism to this procedure that could turn all of the screen blue and underlined,
		// we never go beyond URL_MAX_SCOPE and count the quotes first to make sure there's
		// an odd number of them and we can get one to enclose the URL. A pretty weak 
		// heuristics, I have to admit...

		// move to the end of the string
		rg.Set(cp + cch + cchTail, 0);
		chStopChar = RIGHTANGLEBRACKET;
		if (GetAngleBracket(rg._rpTX) < 0) // closing bracket
		{
			LONG cchExtend = MoveByDelimiters(rg._rpTX, 1,	
											  URL_STOPATCHAR,	&chStopChar);

			Assert(cchExtend <= URL_MAX_SIZE);
			// did we really get the closing bracket?
			if (chStopChar == RIGHTANGLEBRACKET)
			{
				rg.Set(cp + cchHead, -(cch - cchHead + cchTail + cchExtend - 1));
				return;
			}
		}
		// otherwise the quotes did not work out; fall through to
		// the general case
	}

	rg.Set(cp + cchHead, -(cch - cchHead + cchTail));

	return;
}

/*
 *	CDetectURL::IsURL
 *
 *	@mfunc	if the range is over a URL, return TRUE.  We assume
 *			that the range has been preset to cover a block of non-white
 *			space text.
 *
 *	@rdesc	TRUE/FALSE
 */
BOOL CDetectURL::IsURL(
	CTxtRange&	rg)		//@parm the range of text to check
{
	LONG i, j;
	TCHAR szBuf[MAXURLHDRSIZE + 1];
	LONG cch;
	
	// make sure the active end is cpMin
	Assert(rg.GetCch() < 0 );
	
	cch = rg._rpTX.GetText(MAXURLHDRSIZE, szBuf);
	szBuf[cch] = L'\0';

	// first, scan the buffer to see if we have a colon, since
	// all URLs must contain that.  wcsnicmp is a fairly expensive
	// call to be making frequently.

	// note that we start at index 3; no URL protocol that we support
	// has the ':' before that.
	for( i = 3; i < cch; i++ )
	{
		if( szBuf[i] == L':')
		{
			for( j = 0; j < sizeof(rgszURL)/sizeof(LPCWSTR); j++ )
			{
				// the strings must match _and_ we must have at least
				// one more character
				if( _wcsnicmp(szBuf, rgszURL[j], rgcchURL[j]) == 0 )
				{
					if( -(rg.GetCch()) > rgcchURL[j] )
					{
						return TRUE;
					}
					return FALSE;
				}
			}

			return FALSE;
		}
		else if( szBuf[i] == L'.')
		{
			for( j = 0; j < sizeof(rgszWWWURL)/sizeof(LPCWSTR); j++ )
			{
				// the strings must match _and_ we must have at least
				// one more character
				if( _wcsnicmp(szBuf, rgszWWWURL[j], rgcchWWWURL[j]) == 0 )
				{
					if( -(rg.GetCch()) > rgcchWWWURL[j] )
					{
						return TRUE;
					}
					return FALSE;
				}
			}

			return FALSE;
		
		}
	}
	return FALSE;
}

/*
 *	CDetectURL::SetURLEffects
 *
 *	@mfunc	sets URL effects for the given range.
 *
 *	@rdesc	void
 *
 *	@comm	The URL effects currently are blue text, underline, with 
 *			CFE_LINK.
 */
void CDetectURL::SetURLEffects(
	CTxtRange&	rg,			//@parm the range on which to set the effects
	IUndoBuilder *publdr)	//@parm the undo context to use
{
	CCharFormat cf;

	cf.dwMask = CFM_LINK | CFM_COLOR | CFM_UNDERLINE;
	cf.dwEffects = CFE_UNDERLINE | CFE_LINK;
	cf.crTextColor = RGB(0, 0, 255);

	// NB!  The undo system should have already figured out what should
	// happen with the selection by now.  We just want to modify the
	// formatting and not worry where the selection should go on undo/redo.
	rg.SetCharFormat(&cf, CTxtRange::IGNORESELAE, publdr);
}

/*
 *	CDetectURL::CheckAndCleanBogusURL
 *
 *	@mfunc	checks the given range to see if it has CFE_LINK set,
 *			and if so, removes is.  We assume that the range is already
 *			_not_ a well-formed URL string.
 *
 *	@rdesc	void
 */
void CDetectURL::CheckAndCleanBogusURL(
	CTxtRange&	rg,			//@parm the range to use
	COLORREF	cr,			//@parm the color to use for restoration
	URLFLAGS	flags,		//@parm the underline style to use for restoration
	BOOL	   &fDidClean,	//@parm return TRUE if we actually did some cleaning
	IUndoBuilder *publdr)	//@parm the undo context to use
{
	LONG cch = -(rg.GetCch());
	Assert(cch > 0);

	CCharFormat cfApply;
	CFormatRunPtr rp(rg._rpCF);

	fDidClean = FALSE;

	// if there are no format runs, nothing to do.
	if( !rp.IsValid() )
	{
		return;
	}

	rp.AdjustForward();
	// run through the format runs in this range; if there is no
	// link bit set, then just return.
	while( cch > 0 )
	{
	    const CCharFormat *ccf = _ped->GetCharFormat(rp.GetFormat());
	    if (ccf)
	    {
    		if( ccf->dwEffects & CFE_LINK )
    		{
    			break;
    		}
        }
		cch -= rp.GetCchLeft();
		rp.NextRun();
	}

	// if there is no link bit set on any part of the range, just return.	
	if( cch <= 0 )
	{
		return;
	}

	// uh-oh, it's a bogus link.  Set the color back to the default color
	// and underline style.
	fDidClean = TRUE;

	// FUTURE: we could try to be smarter about this an only remove formatting
	// that we applied, but that's a bit more complicated (alexgo)

	cfApply.dwMask = CFM_LINK | CFM_COLOR | CFM_UNDERLINE;
	cfApply.dwEffects = 0;

	cfApply.crTextColor = cr;

	if( flags == URL_USEUNDERLINE )
	{
		cfApply.dwEffects |= CFE_UNDERLINE;
	}

	// NB!  The undo system should have already figured out what should
	// happen with the selection by now.  We just want to modify the
	// formatting and not worry where the selection should go on undo/redo.
	rg.SetCharFormat(&cfApply, CTxtRange::IGNORESELAE, publdr);
}

/*
 *	CDetectURL::MoveByDelimiters
 *
 *	@mfunc	returns the signed number of characters until the next delimiter 
 *			character in the given direction.
 *
 *	@rdesc	LONG
 */
LONG CDetectURL::MoveByDelimiters(
	const CTxtPtr&	tpRef,		//@parm the cp/tp to start looking from
	LONG iDir,					//@parm the direction to look, must be 1 or -1
	DWORD grfDelimiters,		//@parm eat or stop at different types of
								// characters. Use one of URL_EATWHITESPACE,
								// URL_EATNONWHITESPACE, URL_STOPATWHITESPACE
								// URL_STOPATNONWHITESPACE, URL_STOPATPUNCT,
								// URL_STOPATNONPUNCT ot URL_STOPATCHAR
	WCHAR *pchStopChar)			// @parm Out: delimiter we stopped at
								// In: the additional char that stops us
								// when URL_STOPATCHAR is specified
{
	CTxtPtr	tp(tpRef);
	LONG	cch = 0;
	LONG	cchvalid = 0;
	const WCHAR *pch;
	LONG	i;
	WCHAR chScanned = URL_INVALID_DELIMITER;

	// determine the scan mode: do we stop at white space, at punctuation, 
	// at a stop character? 
	BOOL fWhiteSpace = (0 != (grfDelimiters & URL_STOPATWHITESPACE));
	BOOL fNonWhiteSpace = (0 != (grfDelimiters & URL_STOPATNONWHITESPACE));
	BOOL fPunct = (0 != (grfDelimiters & URL_STOPATPUNCT));
	BOOL fNonPunct = (0 != (grfDelimiters & URL_STOPATNONPUNCT));
	BOOL fStopChar = (0 != (grfDelimiters & URL_STOPATCHAR));

	Assert( iDir == 1 || iDir == -1 );
	Assert( fWhiteSpace || fNonWhiteSpace || (!fPunct && !fNonPunct));
	Assert( !fStopChar || NULL != pchStopChar);

	// break anyway if we scanned more than URL_MAX_SIZE chars
	for (LONG cchScanned = 0; cchScanned < URL_MAX_SIZE; NULL)
	{
		// get the text
		if( iDir == 1 )
		{
			i = 0;
			pch = tp.GetPch(cchvalid);
		}
		else
		{
			i = -1;
			pch = tp.GetPchReverse(cchvalid);
			// this is a bit odd, but basically compensates for
			// the backwards loop running one-off from the forwards
			// loop.
			
			cchvalid++;
		}

		if( !pch )
		{
			goto exit;
		}

		// loop until we hit a character within criteria.  Note that for
		// our purposes, the embedding character counts as whitespace.

		while( abs(i) < cchvalid  && cchScanned < URL_MAX_SIZE
			&& (IsURLWhiteSpace(pch[i]) ? !fWhiteSpace : !fNonWhiteSpace)
			&& (IsURLDelimiter(pch[i]) ? !fPunct : !fNonPunct)
			&& !(fStopChar && (*pchStopChar == chScanned) && (chScanned != URL_INVALID_DELIMITER))
			&& ((chScanned != CR && chScanned != LF) || fNonWhiteSpace ))
		{	
			chScanned = pch[i];
			i += iDir;
			++cchScanned;
		}


		// if we're going backwards, i will be off by one; adjust
		if( iDir == -1 )
		{
			Assert(i < 0 && cchvalid > 0);
			i++;
			cchvalid--;
		}

		cch += i;

		if( abs(i) < cchvalid )
		{
			break;
		}

		tp.AdvanceCp(i);
	}

exit:	
	// stop char parameter is present, fill it in
	// with the last character scanned and accepted
	if (pchStopChar)
		{
		*pchStopChar = chScanned;
		}

	return cch; 
}

/*
 *	CDetectURL::BraceMatch
 *
 *	@mfunc	returns the matching bracket to the one passed in.
 *			if the symbol passed in is not a bracket it returns 
 *			URL_INVALID_DELIMITER
 *
 *	@rdesc	LONG
 */
WCHAR CDetectURL::BraceMatch(WCHAR chEnclosing)
{	
	// we're matching "standard" roman braces only. Thus only them may be used
	// to enclose URLs. This should be fine (after all, only Latin letters are allowed
	// inside URLs, right?).
	// I hope that the compiler converts this into some efficient code
	switch(chEnclosing)
		{
	case(TEXT('\"')): 
	case(TEXT('\'')): return chEnclosing;
	case(TEXT('(')): return TEXT(')');
	case(TEXT('<')): return TEXT('>');
	case(TEXT('[')): return TEXT(']');
	case(TEXT('{')): return TEXT('}');
	default: return URL_INVALID_DELIMITER;
		}

}

/*
 *	CDetectURL::GetAngleBracket
 *
 *	@mfunc	goes forward as long as the current paragraph 
 *			or URL_SCOPE_MAX not finding quotation marks and counts 
 *			those quotation marks
 *			returns their parity
 *
 *	@rdesc	LONG
 */
LONG CDetectURL::GetAngleBracket(CTxtPtr &tpRef, LONG cchMax)
{	
	CTxtPtr	tp(tpRef);
	LONG	cchvalid = 0;
	const WCHAR *pch;

	Assert (cchMax >= 0);

	if (!cchMax)
	{
		cchMax = URL_MAX_SCOPE;
	}

	// break anyway if we scanned more than cchLimit chars
	for (LONG cchScanned = 0; cchScanned < cchMax; NULL)
	{
		pch = tp.GetPch(cchvalid);

		if (!cchvalid)
		{
			return 0;
		}
		
        LONG i = 0;
		for (LONG i = 0; (i < cchvalid); ++i)
		{
			if (pch[i] == CR || pch[i] == LF || cchScanned >= cchMax)
			{
				return 0;
			}

			if (pch[i] == LEFTANGLEBRACKET)
			{
				return 1;
			}

			if (pch[i] == RIGHTANGLEBRACKET)
			{
				return -1;
			}

			++ cchScanned;

		}

		tp.AdvanceCp(i);

	}
	return 0;
}		

