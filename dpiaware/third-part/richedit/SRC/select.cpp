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
 *	@module	select.cpp -- Implement the CTxtSelection class |
 *	
 *		This module implements the internal CTxtSelection methods.
 *		See select2.c and range2.c for the ITextSelection methods
 *
 *	Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	@devnote
 *		The selection UI is one of the more intricate parts of an editor.
 *		One common area of confusion is the "ambiguous cp", that is,
 *		a cp at the beginning of one line, which is also the cp at the
 *		end of the previous line.  We control which location to use by
 *		the _fCaretNotAtBOL flag.  Specifically, the caret is OK at the
 *		beginning of the line (BOL) (_fCaretNotAtBOL = FALSE) except in
 *		three cases:
 *
 *			1) the user clicked at or past the end of a wrapped line,
 *			2) the user typed End key on a wrapped line,
 *			3) the active end of a nondegenerate selection is at the EOL.
 *
 */

#include "_common.h"
#include "_select.h"
#include "_edit.h"
#include "_disp.h"
#include "_measure.h"
#include "_font.h"
#include "_rtfconv.h"
#include "_ime.h"
#include <Imm.h>

// GuyBark JupiterJ IME:
#ifndef TARGET_NT
#include "keybd.h"
#endif // !TARGET_NT

#ifndef MACPORT
// default FE Fonts for handling autoFont switching
const TCHAR lpJapanFontName[]		= { 0x0FF2D, 0x0FF33, L' ', 0x0660E, 0x0671D, 0x0 };
const TCHAR lpKoreanFontName[]	= { 0x0AD74, 0x0B9BC, 0x0CCB4, 0x0 };
const TCHAR lpSChineseFontName[]	= { 0x05B8B, 0x04F53, 0x0 };
const TCHAR lpTChineseFontName[]	= { 0x065B0, 0x07D30, 0x0660E, 0x09AD4, 0x0 };
#endif

#define MAX_RUNTOSEARCH (256L)

extern WCHAR	lfJapaneseFaceName[LF_FACESIZE];
extern WCHAR	lfHangulFaceName[LF_FACESIZE];
extern WCHAR	lfBig5FaceName[LF_FACESIZE];
extern WCHAR	lfGB2312FaceName[LF_FACESIZE];

ASSERTDATA

// ======================= Invariant stuff and Constructors ======================================================

#define DEBUG_CLASSNAME CTxtSelection
#include "_invar.h"

#ifdef DEBUG
BOOL
CTxtSelection::Invariant( void ) const
{
	// FUTURE: maybe add some thoughtful asserts...

	static LONG	numTests = 0;
	numTests++;				// how many times we've been called
	
	if(IsInOutlineView() && _cch)
	{
		LONG cpMin, cpMost;					
		GetRange(cpMin, cpMost);

		CTxtPtr tp(_rpTX);					// Scan range for an EOP
		tp.SetCp(cpMin);

		AssertSz((unsigned)(tp.FindEOP(cpMost - cpMin) != 0) == _fSelHasEOP,
			"Incorrect CTxtSelection::_fSelHasEOP");
	}

	return CTxtRange::Invariant();
}
#endif

CTxtSelection::CTxtSelection(CDisplay * const pdp) :
				CTxtRange(pdp->GetED())
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CTxtSelection");

	Assert(pdp);

	_fSel = TRUE;					// This range is a selection
	_pdp = pdp;

	// Set the show selection flag to the inverse of the hide selection flag in 
	// the PED.
	_fShowSelection = !pdp->GetED()->fHideSelection();

	// When we are initialized we don't have a selection therefore,
	// we do want to show the caret.
	_fShowCaret = TRUE;
}	

inline void SelectionNull(CTxtEdit *ped)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "SelectionNull");

	if(ped)
		ped->SetSelectionToNull();
}
										

CTxtSelection::~CTxtSelection()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::~CTxtSelection");

	// Notify edit object that we are gone (if there's a nonNULL ped, i.e.,
	// if the selection isn't a zombie).
	SelectionNull(GetPed());
}

////////////////////////////////  Assignments  /////////////////////////////////////////


CRchTxtPtr& CTxtSelection::operator =(const CRchTxtPtr& rtp)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::operator =");

    _TEST_INVARIANT_
    return CTxtRange::operator =(rtp);
}

CTxtRange& CTxtSelection::operator =(const CTxtRange &rg)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::operator =");

    _TEST_INVARIANT_
    return CTxtRange::operator =(rg);
}

//////////////////////  Update caret & selection mechanism  ///////////////////////////////

/*
 *	CTxtSelection::Update(fScrollIntoView)
 *
 *	@mfunc
 *		Update selection and/or caret on screen. As a side
 *		effect, this methods ends deferring updates.
 *
 *	@rdesc
 *		TRUE if success, FALSE otherwise
 */
BOOL CTxtSelection::Update (
	BOOL fScrollIntoView)		//@parm TRUE if should scroll caret into view
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Update");

	LONG cch;
	LONG cchSave = _cch;
	LONG cchText = GetTextLength();
	LONG cp, cpMin, cpMost;
	BOOL fMoveBack = _fMoveBack;

	if(!_cch)								// Update _cpAnchor, etc.
		UpdateForAutoWord();

	else if(GetPF()->InTable() && (_fSelHasEOP || _fSelHasCell))
	{
		Expander(_fSelHasEOP ? tomParagraph : tomCell,
				 TRUE, NULL, &cpMin, &cpMost);
	}
				
	if(IsInOutlineView() && !GetPed()->IsMouseDown())
	{
		CPFRunPtr rp(*this);

		cp = GetCp();
		GetRange(cpMin, cpMost);
		if(_cch && (cpMin || cpMost < cchText))
		{
			LONG *pcpMin = &cpMin;
			LONG *pcpMost = &cpMost;

			// If selection contains an EOP, expand to para boundaries
			if(_fSelHasEOP)
			{
				if(_fMoveBack ^ (_cch < 0))	// Decreasing selection
				{							//  size: move active end
					if(_fMoveBack)			
						pcpMost = NULL;		//  to StartOf para
					else
						pcpMin = NULL;		//  to EndOf para				
				}
				Expander(tomParagraph, TRUE, NULL, pcpMin, pcpMost);
			}

			LONG cpMinSave  = cpMin;		// Save initial cp's to see if		
			LONG cpMostSave = cpMost;		//  we need to Set() below

			// The following handles selection expansion correctly, but
			// not compression; need logic like that preceding Expander()
			rp.AdvanceCp(cpMin - cp);		// Start at cpMin
			if(rp.IsCollapsed())
				cpMin += rp.FindExpandedBackward();

			rp.AdjustForward();

			BOOL fCpMinCollapsed = rp.IsCollapsed();
			rp.AdvanceCp(cpMost - cpMin);	// Go to cpMost
			if(rp.IsCollapsed())
				cpMost += rp.FindExpandedForward();

            // GuyBark Jupiter 17766.
            //
            // If fCpMinCollapsed is set, then cpMin is in a collapsed range. Therefore 
            // remove the selection and move the caret to cpMost. That's a reasonable 
            // thing to do.  Also check to see if cpMost is in a collapsed region. If 
            // so, the selection is removed, and the caret is moved to cpMin, This is 
            // also a reasonable thing to do.
            //
            // However, the test of rp.IsCollapsed() is not complete. Say you tap the +
            // for a collapsed header. The header and the entire collapsed range is 
            // selected on the pen down. This includes the final eop for the collapsed
            // range. Then on the pen up, we end up here. The call to rp.IsCollapsed()
            // here checks to see if the cp after the end of selection is in a collapsed range.
            // But the end of the selection here is after the final eop in the range.
            // It's actually at the start of the next visible text, which by definition
            // cannot be collapsed. So the cpMost is found not to be collapsed, and we
            // don't mess with cpMost here. BUT say the collapsed text for the header 
            // selected runs to the end of the document. In this case, there's nothin
            // beyond the final eop in the collapsed range. So when we try to find if 
            // whatever's beyond that is collapsed, we can't. So we say that cpMost 
            // does lie in a collapsed range.
            //
            // So therefore, check if cpMost is at the end of the document. If so, we 
            // won't mess with cpMost here. This makes sense, as how can a non-existant
            // cp beyond the end of the document lie in a collapsed range.
            //
            // This leads to one other change to the RichEdit behavior. In Normal View,
            // select part way through the last header in the document up to the end of
            // the document, (including the final eop). Change to outline view, such that
            // as soon as you get there, the last header will be the last visible line.
            // Previously we found here that cpMin was not in collapsed text, but that 
            // cpMost was. This meant the selection was removed, and the caret is set
            // to the beginning of the header. With this change, we find that cpMost is
            // not in collapsed text, so we don't mess with it. This means by the time 
            // we get to Outline View, the entire header, (and all its collapsed text),
            // is selected. That's how other headers behave in the document too. So this
            // change in behavior is a good thing.

            // At least I think that's what's going on.
#ifdef PWD_JUPITER
			if(fCpMinCollapsed || (rp.IsCollapsed() && (cpMost < cchText)))
#else
			if(fCpMinCollapsed || rp.IsCollapsed())
#endif // PWD_JUPITER
			{
				if(rp.IsCollapsed())
				{
					rp.AdvanceCp(cpMin - cpMost);
					rp.AdjustForward();
					cpMost = cpMin;
				}
				else
					cpMin = cpMost;
			}							
			if(cpMin != cpMinSave || cpMost != cpMostSave)
				Set(cpMost, cpMost - cpMin);
		}
		if(!_cch && rp.IsCollapsed())		// Note: above may have collapsed
		{									//  selection...
			cch = fMoveBack ? rp.FindExpandedBackward() : 0;
			if(rp.IsCollapsed())
#ifdef PWD_JUPITER // GuyBark Jupiter 18391
				cch = rp.FindExpanded(FALSE);
#else
				cch = rp.FindExpanded();
#endif // PWD_JUPITER

			_fExtend = FALSE;
			Advance(cch);
			rp.AdjustForward();
			if(cch <= 0 && rp.IsCollapsed() && _rpTX.IsAfterEOP())
				BackupCRLF();
			_fCaretNotAtBOL = FALSE;
		}
	}

	// Don't let active end be in hidden text
	CCFRunPtr rp(*this);

	cp = GetCp();
	GetRange(cpMin, cpMost);
	if(_cch && (cpMin || cpMost < cchText))
	{
		rp.AdvanceCp(cpMin - cp);		// Start at cpMin
		BOOL fHidden = rp.IsInHidden();
		rp.AdvanceCp(cpMost - cpMin);	// Go to cpMost

		if(fHidden)						// It's hidden, so collapse
			Collapser(tomEnd);			//  selection at End for treatment

		else if(rp.IsInHidden() &&		// cpMin OK, how about cpMost?
			cpMost < cchText)
		{								// Check both sides of edge
			Collapser(tomEnd);			//  collapse selection at end
		}								
	}
	if(!_cch && rp.IsInHidden())		// Note: above may have collapsed
	{									//  selection...
		cch = fMoveBack ? rp.FindUnhiddenBackward() : 0;
		if(!fMoveBack || rp.IsHidden())
			cch = rp.FindUnhidden();

		_fExtend = FALSE;
		Advance(cch);
		_fCaretNotAtBOL = FALSE;
	}
	if((cchSave ^ _cch) < 0)			// Don't change active end
		FlipRange();

	if(!_cch && cchSave)
	{
		Update_iFormat(-1);
		_fCaretNotAtBOL = FALSE;
	}

	_TEST_INVARIANT_

	if( !GetPed()->fInplaceActive() || GetPed()->IsStreaming() )
	{
		// nothing to do while inactive or streaming in text or RTF data.
		return TRUE;
	}

	// Recalc up to active end (caret)
	if(!_pdp->WaitForRecalc(GetCp(), -1))
	{										// Line recalc failure
		Set(0, 0);							// Put caret at start of text 
	}

	ShowCaret(!_cch);
	UpdateCaret(fScrollIntoView);			// Update Caret position, possibly
											//  scrolling it into view
	GetPed()->TxShowCaret(FALSE);
	UpdateSelection();						// Show new selection
	GetPed()->TxShowCaret(TRUE);

	return TRUE;
}

/*
 *	CTxtSelection::CheckSynchCharSet()
 *
 *	@mfunc
 *		Check if the current keyboard matches the current font's charset;
 *		if not, call CheckChangeFont to find the right font
 */
void CTxtSelection::CheckSynchCharSet()
{	
	CTxtEdit*			ped = GetPed();
	const CCharFormat	*pCF;
	UINT				lcidKbd;
	UINT				uKbdCodePage;

	if (_cch)
	{
		// for selection, we need to get the character format at cpMin+1
		CTxtRange rg( ped, GetCpMin()+1, 0 );
		Set_iCF(rg.Get_iCF ());

		_fUseiFormat = TRUE;
	}
		
	pCF = ped->GetCharFormat(_iFormat);

 	// If current font is not set correctly,
	// change to a font preferred by current keyboard.
	lcidKbd = GetKeyboardLCID();
	uKbdCodePage = ConvertLanguageIDtoCodePage(lcidKbd);
	if 	(lcidKbd && pCF && !ped->_fSingleCodePage && 
		 ((UINT)GetCodePage(pCF->bCharSet) != uKbdCodePage) && 
		 pCF->bCharSet != SYMBOL_CHARSET && pCF->bCharSet != DEFAULT_CHARSET &&
		 pCF->bCharSet != OEM_CHARSET &&
		 (!IsFELCID(lcidKbd) || pCF->bCharSet != ANSI_CHARSET ||
		 uKbdCodePage == _JAPAN_CP && ped->_fKANAMode && pCF->bCharSet != SHIFTJIS_CHARSET ) 
		)
	{
		CheckChangeFont (ped, TRUE, lcidKbd, uKbdCodePage);
	}
}

/*
 *	CTxtSelection::UpdateCaret(fScrollIntoView)
 *
 *	@mfunc
 *		This routine updates caret/selection active end on screen. 
 *		It figures its position, size, clipping, etc. It can optionally 
 *		scroll the caret into view.
 *
 *	@rdesc
 *		TRUE if view was scrolled, FALSE otherwise
 *
 *	@devnote
 *		The caret is actually shown on screen only if _fShowCaret is TRUE.
 */
BOOL CTxtSelection::UpdateCaret (
	BOOL fScrollIntoView)	//@parm If TRUE, scroll caret into view if we have
							// focus or if not and selection isn't hidden 
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateCaret");
							// of focus
	_TEST_INVARIANT_

	// Is the display currently frozen
	if (_pdp->IsFrozen())
	{
		// Save this call for another time.
		_pdp->SaveUpdateCaret(fScrollIntoView);
		return FALSE;
	}

	BOOL		fAutoVScroll	= FALSE;
	BOOL		fAutoHScroll	= FALSE;
	CTxtEdit* 	ped				= GetPed();
	POINT 		pt;
	CLinePtr 	rp(_pdp);

	RECT		rcClient;
	RECT		rcView;
	LONG		xWidthView;
	LONG		yHeightView;

	LONG		xScroll			= _pdp->GetXScroll();
	LONG		yScroll			= _pdp->GetYScroll();

	INT 		yAbove			= 0;	// ascent of line above & beyond IP
	INT			yAscent         = 0;	// ascent of IP
	INT 		yAscentLine;
	LONG		yBase;					// base of IP & line
	INT 		yBelow			= 0;	// descent of line below & beyond IP
	INT 		yDescent        = 0;    // descent of IP
	INT 		yDescentLine;
	const INT 	yHeightSave		= _yHeightCaret;
	INT			ySum;
	LONG		yViewTop;
	LONG		yViewBottom;

	DWORD		dwScrollBars	= ped->TxGetScrollBars();

	if( ped->IsStreaming() )
	{
		// don't bother doing anything if we are loading in text or RTF
		// data.
		return FALSE;
	}
	_yCurrentDescent = -1;

	// We better be inplace active if we get here
	AssertSz(GetPed()->fInplaceActive(), 
		"CTxtSelection::UpdateCaret no inplace active");

	// Get the client rectangle once to save various
	// callers getting it.
	GetPed()->TxGetClientRect(&rcClient);

	_pdp->GetViewRect(rcView, &rcClient);

	// View can be bigger than client rect because insets can be negative.
	// We don't want the caret to be any bigger than the client view otherwise
	// the caret will leave pixel dust on other windows.
	yViewTop = max(rcView.top, rcClient.top);
	yViewBottom = min(rcView.bottom, rcClient.bottom);

	xWidthView = rcView.right - rcView.left;
	yHeightView = yViewBottom - yViewTop;

	if(fScrollIntoView)
	{
		fAutoVScroll = (dwScrollBars & ES_AUTOVSCROLL) != 0;
		fAutoHScroll = (dwScrollBars & ES_AUTOHSCROLL) != 0;

		// If we're not forcing a scroll, only scroll if window has focus
		// or selection isn't hidden
		fScrollIntoView = ped->_fFocus || !ped->fHideSelection();
	}

	if(!fScrollIntoView && (fAutoVScroll || fAutoHScroll))
	{											// Would scroll but don't have
		ped->_fScrollCaretOnFocus = TRUE;		//  focus. Signal to scroll
		fAutoVScroll = fAutoHScroll = FALSE;	//  when we do get focus
	}

	if(_pdp->PointFromTp(*this, &rcClient, _fCaretNotAtBOL, pt, &rp,
		TA_BASELINE) < 0 ||
		!_cch && IsInOutlineView() && (GetPF()->wEffects & PFE_COLLAPSED))
	{
		goto not_visible;
	}

	// HACK ALERT - Because plain-text multiline controls do not have the 
	// automatic EOP, we need to special case their processing here because 
	// if you are at the end of the document and last character is an EOP, 
	// you need to be on the next line in the display not the current line.

	if(CheckPlainTextFinalEOP())					//  terminated by an EOP
	{
		if (GetPF()->wAlignment == PFA_CENTER)
			pt.x = (rcView.left + rcView.right) >> 1;
		else
			// Set the x to the beginning of the line
			pt.x = rcView.left;

		pt.x -= xScroll;							// Absolute coordinate

		// Bump the y up a line. We get away with the calculation because 
		// the document is plain text so all lines have the same height. 
		// Also, note that the rp below is used only for height 
		// calculations, so it is perfectly valid for the same reason 
		// even though it is not actually pointing to the correct line. 
		// (I told you this is a hack.)
		pt.y += rp->_yHeight;
	}

	_xCaret = (LONG) pt.x;
	yBase   = (LONG) pt.y;
	
	// Compute caret height, ascent, and descent
	yAscent = GetCaretHeight(&yDescent);
	yAscent -= yDescent;

	// Default to line empty case. Use what came back from the default 
	// calculation above.
	yDescentLine = yDescent;
	yAscentLine = yAscent;

	if( rp.IsValid() )
	{
		if (rp->_yDescent != -1)
		{
			// Line has been measured so we can use the values in the
			// line.
			yDescentLine = rp->_yDescent;
			yAscentLine = rp->_yHeight - yDescentLine;
		}
	}

	if(yAscent + yDescent == 0)
	{
		yAscent = yAscentLine;
		yDescent = yDescentLine;
	}
	else
	{
		// this is a bit counter-intuitive at first.  Basically,
		// even if the caret should be large (i.e. due to a
		// large font at the insertion point), we can only make it
		// as big as the line.  If a character is inserted, then 
		// the line becomes bigger, and we can make the caret
		// the correct size.
		yAscent = min(yAscent, yAscentLine);
		yDescent = min(yDescent, yDescentLine);
	}

	if(fAutoVScroll)
	{
		Assert(yDescentLine >= yDescent);
		Assert(yAscentLine >= yAscent);

		yBelow = yDescentLine - yDescent;
		yAbove = yAscentLine - yAscent;

		ySum = yAscent;

		// Scroll as much as possible into view, giving priorities
		// primarily to IP and secondarily ascents
		if(ySum > yHeightView)
		{
			yAscent = yHeightView;
			yDescent = 0;
			yAbove = 0;
			yBelow = 0;
		}
		else if((ySum += yDescent) > yHeightView)
		{
			yDescent = yHeightView - yAscent;
			yAbove = 0;
			yBelow = 0;
		}
		else if((ySum += yAbove) > yHeightView)
		{
			yAbove = yHeightView - (ySum - yAbove);
			yBelow = 0;
		}
		else if((ySum += yBelow) > yHeightView)
			yBelow = yHeightView - (ySum - yBelow);
	}
#ifdef DEBUG
	else
	{
		AssertSz(yAbove == 0, "yAbove non-zero");
		AssertSz(yBelow == 0, "yBelow non-zero");
	}
#endif


// Update real caret x pos (constant during vertical moves)

	_xCaretReally = _xCaret - rcView.left + xScroll;
	Assert(_xCaretReally >= 0);
	
	if(_xCaret + GetCaretDelta() > rcView.right && 	// Caret off right edge,
		!((dwScrollBars & ES_AUTOHSCROLL) ||		//  not auto hscrolling
		_pdp->IsHScrollEnabled()))					//  and no scrollbar:
	{												// Back caret up to
		_xCaret = rcView.right - dxCaret;			//  exactly the right edge
	}

	// From this point on we need a new caret
	_fCaretCreated = FALSE;

	if( ped->_fFocus)
		ped->TxShowCaret(FALSE);					// Hide old caret before
													//  making a new one
	if(yBase + yDescent + yBelow > yViewTop &&
		yBase - yAscent - yAbove < yViewBottom)
	{
		if(yBase - yAscent - yAbove < yViewTop)		// Caret is partially
		{											//  visible
			if(fAutoVScroll)						// Top isn't visible
				goto scrollit;
			Assert(yAbove == 0);

			yAscent = yBase - yViewTop;				// Change ascent to amount
			if(yBase < yViewTop)					//  visible
			{										// Move base to top
				yDescent += yAscent;
				yAscent = 0;
				yBase = yViewTop;
			}
		}
		if(yBase + yDescent + yBelow > yViewBottom)
		{
			if(fAutoVScroll)						// Bottom isn't visible
				goto scrollit;
			Assert(yBelow == 0);

			yDescent = yViewBottom - yBase;			// Change descent to amount
			if(yBase > yViewBottom)					//  visible
			{										// Move base to bottom
				yAscent += yDescent;
				yDescent = 0;
				yBase = yViewBottom;
			}
		}

		// Anything still visible?
		if(yAscent <= 0 && yDescent <= 0)
			goto not_visible;

		// If left or right isn't visible, scroll or set non_visible
		if (_xCaret <= rcView.left + CDisplay::GetXWidthSys() && xScroll // Left isn't visible
			|| _xCaret + GetCaretDelta() > rcView.right)	// Right isn't visible
		{
			if(fAutoHScroll)
				goto scrollit;
			goto not_visible;
		}

		_yCaret = yBase - yAscent;
		_yHeightCaret = (INT) yAscent + yDescent;
		_yCurrentDescent = yDescent;
	}
	else if(fAutoHScroll || fAutoVScroll)			// Caret isn't visible
		goto scrollit;								//  scroll it into view
	else
	{

not_visible:
		// Caret isn't visible, don't show it
		_xCaret = -32000;
		_yCaret = -32000;
		_yHeightCaret = 1;
	}

// Now update caret for real on screen

	// We only want to show the caret if it is in the view
	// and there is no selection.
	if((ped->_fFocus) && _fShowCaret && (0 == _cch))
	{
		CreateCaret();
		ped->TxShowCaret(TRUE);
		CheckChangeKeyboardLayout( FALSE );
	}

#ifdef DBCS
	UpdateIMEWindow();
	FSetIMEFontH(ped->_hwnd, AttGetFontHandle(ped, ped->_cpCaret));
#endif
	return FALSE;


scrollit:

	if(fAutoVScroll)
	{
		// Scroll to top for cp = 0. This is important if the first line
		// contains object(s) taller than the client area is high.	The
		// resulting behavior agrees with the Word UI in all ways except in
		// Backspacing (deleting) the char at cp = 0 when it is followed by
		// other chars that preceed the large object.
		if(!GetCp())											
			yScroll = 0;

		else if(yBase - yAscent - yAbove < yViewTop)			// Top invisible
			yScroll -= yViewTop - (yBase - yAscent - yAbove);	// Make it so

		else if(yBase + yDescent + yBelow > yViewBottom)		// Bottom invisible
		{
			yScroll += yBase + yDescent + yBelow - yViewBottom;	// Make it so

			// Don't do following special adjust if the current line is bigger
			// than the client area
			if(rp->_yHeight < yViewBottom - yViewTop)
			{
				yScroll = _pdp->AdjustToDisplayLastLine(yBase + rp->_yHeight, 
					yScroll);
			}
		}
	}
	if(fAutoHScroll)
	{
		if(_xCaret <= (rcView.left + CDisplay::GetXWidthSys()))	// Left invisible
		{
			xScroll -= rcView.left - _xCaret;				// Make it visible
			if(xScroll > 0)									// Scroll left in
			{												//  chunks to make
				xScroll -= xWidthView / 3;					//  typing faster
				xScroll = max(0, xScroll);
			}
		}
		else if(_xCaret + GetCaretDelta() > rcView.right)	// right invisible
			xScroll += _xCaret + dxCaret - rcView.left		// Make it visible
					- xWidthView;							// We don't scroll
															// in chunks because
															// this more edit 
															// control like.
					//- xWidthView * 2 / 3;					//  scrolling in
	}														//  chunks

	if(yScroll != _pdp->GetYScroll() || xScroll != _pdp->GetXScroll())
		return _pdp->ScrollView(xScroll, yScroll, FALSE, FALSE);

#ifdef DBCS
	VwUpdateIMEWindow(ped);
	FSetIMEFontH(ped->_hwnd, AttGetFontHandle(ped, ped->_cpCaret));
#endif

	return FALSE;
}

/*
 *	CTxtSelection::GetCaretHeight(pyDescent)
 *
 *	@mfunc
 *		Add a given amount to _xCaret (to make special case of inserting 
 *		a character nice and fast)
 *
 *	@rdesc
 *		Caret height, <lt> 0 if failed
 */
INT CTxtSelection::GetCaretHeight (
	INT *pyDescent) const		//@parm Out parm to receive caret descent
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetCaretHeight");
								// (undefined if the return value is <lt> 0)
	_TEST_INVARIANT_

	const CCharFormat *pcf = GetPed()->GetCharFormat(_iFormat);
	const CDevDesc *pdd = _pdp->GetDdRender();
	INT			CaretHeight = -1;
	HDC			hdc;
	CCcs *		pccs;

    if ((NULL == pdd) || (NULL == pcf))
        return CaretHeight;


 	hdc = pdd->GetDC();

	if(!hdc)
		return -1;

	pccs = fc().GetCcs(hdc, pcf, _pdp->GetZoomNumerator(),
		_pdp->GetZoomDenominator(), GetDeviceCaps(hdc, LOGPIXELSY));

	if(!pccs)
		goto ret;

	if(pyDescent)
		*pyDescent = (INT) pccs->_yDescent;

	CaretHeight = pccs->_yHeight;
	
	pccs->Release();

ret:

	pdd->ReleaseDC(hdc);

	return CaretHeight;
}

/*
 *	CTxtSelection::ShowCaret(fShow)
 *
 *	@mfunc
 *		Hide or show caret
 *
 *	@rdesc
 *		TRUE if caret was previously shown, FALSE if it was hidden
 */
BOOL CTxtSelection::ShowCaret (
	BOOL fShow)		//@parm TRUE for showing, FALSE for hiding
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ShowCaret");

	_TEST_INVARIANT_

	const BOOL fRet = _fShowCaret;

	if(fRet != fShow)
	{
		_fShowCaret = fShow;

		if (GetPed()->_fFocus)
		{
			if (fShow && !_fCaretCreated)
			{
				CreateCaret();
			}

			GetPed()->TxShowCaret(fShow);
		}
	}

	return fRet;
}

/*
 *	CTxtSelection::IsCaretInView()
 *
 *	@mfunc
 *		Returns TRUE iff caret is inside visible view
 */
BOOL CTxtSelection::IsCaretInView() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::IsCaretInView");

	_TEST_INVARIANT_

	RECT rc;
	_pdp->GetViewRect(rc);
		
	return  (_xCaret + dxCaret		 > rc.left) &&
			(_xCaret				 < rc.right) &&
		   	(_yCaret + _yHeightCaret > rc.top) &&
			(_yCaret				 < rc.bottom);
}

/*
 *	CTxtSelection::CaretNotAtBOL()
 *
 *	@mfunc
 *		Returns TRUE iff caret is not allowed at BOL
 */
BOOL CTxtSelection::CaretNotAtBOL() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CaretNotAtBOL");

	_TEST_INVARIANT_

	return _cch ? (_cch > 0) : _fCaretNotAtBOL;
}

/*
 *	CTxtSelection::LineLength()
 *
 *	@mfunc
 *		get # unselected chars on lines touched by current selection
 *
 *	@rdesc
 *		said number of chars
 */
LONG CTxtSelection::LineLength() const
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::LineLength");

	_TEST_INVARIANT_

	LONG cch;
	CLinePtr rp(_pdp);

	if(!_cch)								// Insertion point
	{
		rp.RpSetCp(GetCp(), _fCaretNotAtBOL);
		cch = rp.GetAdjustedLineLength();
	}
	else
	{
		LONG cpMin, cpMost, cchLast;
		GetRange(cpMin, cpMost);
		rp.RpSetCp(cpMin, FALSE);		// Selections can't start at EOL
		cch = rp.RpGetIch();
		rp.RpSetCp(cpMost, TRUE);		// Selections can't end at BOL

		// now remove the trailing EOP (if it exists and isn't
		// already selected).
		cchLast = rp.GetAdjustedLineLength() - rp.RpGetIch();
		if( cchLast > 0 )
		{
			cch += cchLast;
		}
	}
	return cch;
}

/*
 *	CTxtSelection::ShowSelection(fShow)
 *
 *	@mfunc
 *		Update, hide or show selection on screen
 *
 *	@rdesc
 *		TRUE iff selection was previously shown
 */
BOOL CTxtSelection::ShowSelection (
	BOOL fShow)			//@parm TRUE for showing, FALSE for hiding
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ShowSelection");

	_TEST_INVARIANT_

	const BOOL fShowPrev = _fShowSelection;
	const BOOL fInplaceActive = GetPed()->fInplaceActive();
	LONG cpSelSave = _cpSel;
	LONG cchSelSave = _cchSel;

	// Sleep(1000);
	_fShowSelection = fShow;

	if(fShowPrev && !fShow)
	{
		if(cchSelSave)			// Hide old selection
		{
			// Set up selection before telling the display to update
			_cpSel = 0;
			_cchSel = 0;

			if (fInplaceActive)
			{
				_pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
			}
		}
	}
	else if(!fShowPrev && fShow)
	{
		if(_cch)								// Show new selection
		{
			// Set up selection before telling the display to update
			_cpSel = GetCp();
			_cchSel = _cch;

			if (fInplaceActive)
			{
				_pdp->InvertRange(GetCp(), _cch, selSetHiLite);
			}

		}
	}

	return fShowPrev;
}

/*
 *	CTxtSelection::UpdateSelection()
 *
 *	@mfunc
 *		Updates selection on screen 
 *
 *	Note:
 *		This method inverts the delta between old and new selections
 */
void CTxtSelection::UpdateSelection()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::UpdateSelection");

	_TEST_INVARIANT_
	
	LONG	cp = GetCp();
	LONG	cpNA	= cp - _cch;
	LONG	cpSelNA = _cpSel - _cchSel;
	LONG 	cpMin, cpMost, cpMinSel, cpMostSel;
	CObjectMgr* pobjmgr = NULL;
	LONG	NumObjInSel = 0, NumObjInOldSel = 0;
	LONG	cpSelSave = _cpSel;
	LONG	cchSelSave = _cchSel;

	GetRange(cpMin, cpMost);

	//We need to know if there were objects is the previous and current
	//selections to determine how they should be selected.
	if (GetPed()->HasObjects())
	{
		pobjmgr = GetPed()->GetObjectMgr();
 
		if (pobjmgr)
		{
			CTxtRange	tr(GetPed(), _cpSel, _cchSel);

			tr.GetRange(cpMinSel, cpMostSel);
			NumObjInSel = pobjmgr->CountObjectsInRange(cpMin, cpMost);
			NumObjInOldSel = pobjmgr->CountObjectsInRange(cpMinSel, cpMostSel);
		}
	}

	//If the old selection contained a single object and nothing else
	//we need to notify the object manager that this is no longer the
	//case if the selection is changing.
	if (NumObjInOldSel && (abs(_cchSel) == 1) &&
		!(cpMin == cpMinSel && cpMost == cpMostSel))
	{
		if (pobjmgr)
		{
			pobjmgr->HandleSingleSelect(GetPed(), cpMinSel, /* fHilite */ FALSE);
		}
	}

	// Update selection data before the invert so the selection can be
	// painted by the render
	_cpSel  = GetCp();
	_cchSel = _cch;

	if( _fShowSelection )
	{
		if( !_cch || !cchSelSave ||				// Old/new selection missing,
			cpMost < min(cpSelSave, cpSelNA) ||	//  or new preceeds old,
			cpMin  > max(cpSelSave, cpSelNA))	//  or new follows old, so
		{										//  they don't intersect
			if(_cch)
				_pdp->InvertRange(cp, _cch, selSetHiLite);
			if(cchSelSave)
				_pdp->InvertRange(cpSelSave, cchSelSave, selSetNormal);
		}
		else
		{
			if(cpNA != cpSelNA)					// Old & new dead ends differ
			{									// Invert text between them
				_pdp->InvertRange(cpNA, cpNA - cpSelNA, selUpdateNormal);
			}
			if(cp != cpSelSave)					// Old & new active ends differ
			{									// Invert text between them
				_pdp->InvertRange(cp, cp - cpSelSave, selUpdateHiLite);
			}
		}
	}

	//If the new selection contains a single object and nothing else
	//we need to notify the object manager as long as it's not the same
	//object.
	if (NumObjInSel && (abs(_cch) == 1) &&
		!(cpMin == cpMinSel && cpMost == cpMostSel))
	{
		if (pobjmgr)
		{
			pobjmgr->HandleSingleSelect(GetPed(), cpMin, /* fHiLite */ TRUE);
		}
	}
}

/*
 * 	CTxtSelection::SetSelection(cpFirst, cpMost)
 *
 *	@mfunc
 *		Set selection between two cp's
 *	
 *	@devnote
 *		<p cpFirst> and <p cpMost> must be greater than 0, but may extend
 *		past the current max cp.  In that case, the cp will be truncated to
 *		the max cp (at the end of the text).	
 */
void CTxtSelection::SetSelection (
	LONG cpMin,				//@parm Start of selection and dead end
	LONG cpMost)			//@parm End of selection and active end
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetSelection");

	_TEST_INVARIANT_

	StopGroupTyping();

	_fCaretNotAtBOL = FALSE;			// Put caret for ambiguous cp at BOL
	Set(cpMost, cpMost - cpMin);		// Set() validates cpMin, cpMost

	if (GetPed()->fInplaceActive())
	{
		// Inplace active - update the selection now.
		Update(TRUE);
	}
	else
	{
		// Update the selection data used for screen display
		// so whenever we get displayed the selection will be
		// displayed.
		_cpSel  = GetCp();
		_cchSel = _cch;	

		if (!GetPed()->fHideSelection())
		{
			// Selection is not hidden so tell container to 
			// update the display when it feels like.
        	GetPed()->TxInvalidateRect(NULL, FALSE);
			GetPed()->TxUpdateWindow();
		}
	}

	CancelModes();						// Cancel word selection mode
}

/*
 *	CTxtSelection::PointInSel(pt, prcClient)
 *
 *	@mfunc
 *		Figures whether a given point is within the selection
 *
 *	@rdesc
 *		TRUE if point inside selection, FALSE otherwise
 */
BOOL CTxtSelection::PointInSel (
	const POINT pt,			//@parm Point in containing window client coords
	const RECT *prcClient,	//@parm Client rectangle can be NULL if active
	HITTEST		Hit) const	//@parm Possibly computer Hit value
	
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PointInSel");

	LONG	cp;
	LONG	cpMin,  cpMost;
	POINT	temppt;
	CRchTxtPtr	rtp(GetPed(), 0);

	_TEST_INVARIANT_

	if (!_cch || Hit && Hit < HT_Text)	// Degenerate range (no selection):
		return FALSE;					//  mouse can't be in, or Hit not
										//  in text
	cp = _pdp->CpFromPoint(pt, prcClient, NULL, NULL, FALSE, &Hit);

	if(Hit < HT_Text)
		return FALSE;

	GetRange(cpMin, cpMost);

	// we have to test boundary cases separately--we want to make
	// sure the point is in the bounding box of the selection, but cp
	// from point will artificially extend that bounding box to half of
	// the next/previous character.  Think of it this way, when you click
	// in the middle of a character, the insertion point has to go
	// somewhere to the left or the right.
	if( cp > cpMin && cp < cpMost )
	{
		return TRUE;
	}
	else if( cp == cpMin )
	{
		rtp.SetCp(cp);
		_pdp->PointFromTp(rtp, prcClient, FALSE, temppt, NULL, TA_TOP);

		if( pt.x >= temppt.x )
		{
			return TRUE;
		}
	}
	else if( cp == cpMost )
	{
		rtp.SetCp(cp);
		_pdp->PointFromTp(rtp, prcClient, TRUE, temppt, NULL, TA_TOP);

		if( pt.x <= temppt.x )
		{
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////  Selection with the mouse  ///////////////////////////////////

/*
 * 	CTxtSelection::SetCaret(pt, fUpdate)
 *
 *	@mfunc
 *		Sets caret at a given point
 *
 *	@devnote
 *		In the plain-text case, placing the caret at the beginning of the
 *		line following the final EOP requires some extra code, since the
 *		underlying rich-text engine doesn't assign a line to a final EOP
 *		(plain-text doesn't currently have the rich-text final EOP).  We
 *		handle this by checking to see if the count of lines times the
 *		plain-text line height is below the actual y position.  If so, we
 *		move the cp to the end of the story.
 */
void CTxtSelection::SetCaret(
	const POINT pt,		//@parm Point of click
	BOOL fUpdate)		//@parm If TRUE, update the selection/caret
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCaret");

	_TEST_INVARIANT_

	LONG		cp;
    RECT		rcView;
	CLinePtr	rp(_pdp);
	CRchTxtPtr  rtp(GetPed());
	LONG		y;

	StopGroupTyping();

	// Set caret at point
	if (_pdp->CpFromPoint(pt, NULL, &rtp, &rp, FALSE) >= 0)
	{
		// Set the selection to the correct location.  If plain-text
		// multiline control, we need to check to see if pt.y is below
		// the last line of text.  If so and if the text ends with an EOP,
		// we need to set the cp at the end of the story and set up to
		// display the caret at the beginning of the line below the last
		// line of text
		cp = rtp.GetCp();
		if (!GetPed()->IsRich() &&					// Plain-text,
			GetPed()->TxGetMultiLine())				//  multiline control
		{
			_pdp->GetViewRect(rcView, NULL);
			y = pt.y + _pdp->GetYScroll() - rcView.top;
													
			if(y > (LONG)rp.Count()*rp->_yHeight)	// Below last line of
			{										//  text
				rtp.Advance(tomForward);			// Move rtp to end of text
				if(rtp._rpTX.IsAfterEOP())			// If text ends with an
				{									//  EOP, set up to move
					cp = rtp.GetCp();				//  selection there
					rp.AdvanceCp(-(LONG)rp.GetIch());// Set rp._ich = 0 to
				}									//  set _fCaretNotAtBOL
			}										//  = FALSE to display
		}											//  caret at next BOL


#ifndef TARGET_NT
		// GuyBark JupiterJ IME:
		// We must take action to ensure the ime knows the caret's moved.

		if(GetPed()->IsIMEComposition())
		{
		    // Select another clause if necessary. If we do highlight 
		    // another clause, then don't set the IME caret position.
		    // Otherwise the IME caret setting action interferes with
		    // the clause highlighting.

		    if(SetIMEHighlight(&cp, pt, &rtp))
		    {
		        // Didn't highlight another clause. We may freeze the 
		        // display beneath here, until we've finished walking 
		        // the caret if we need to.
		        SetIMECaret(&cp);

		        // Store the final caret cp here, so we can move any candidate
		        // list to that position if necessary later.
		        _pdp->PointFromTp(rtp, NULL, FALSE, GetPed()->_ime->_pt, NULL, TA_TOP);
		    }
		}
#endif // !TARGET_NT

		Set(cp, 0);

		_fCaretNotAtBOL = rp.RpGetIch() != 0;	// Caret OK at BOL if click
		if(fUpdate)
			Update(TRUE);
		else
			UpdateForAutoWord();

		_SelMode = smNone;						// Cancel word selection mode
	}
}

#ifndef TARGET_NT
/*
 *  CTxtSelection::SetIMECaret(pcpNew)
 *
 *  GUYBARK ADD THIS!
 *
 *  Without this, RichEdit know the caret's moved, but the ime doesn't.
 *
 *  Returns FALSE if no errors, else TRUE
 */
BOOL CTxtSelection::SetIMECaret(LONG *pcpNew)
{
    LONG cpMin, cpMost, cpMove, cpUndeterminedStart, cchUndetermined;
    UINT vkCode;
    HWND hWndRE;
    UINT nShift;
    UINT chCode;
    CTxtEdit *ped;

    if(!pcpNew || !(ped = GetPed()))
    {
        return TRUE;
    }

    // Don't do anything here if we're not in undetermined text.
    if(!ped->IsIMEComposition())
    {
        // No errors.
        return FALSE;
    }

    // Get the current cp.
    GetRange(cpMin, cpMost);

    // Force the caret to stay in the undetermined text.
    if(ped->_ime->GetUndeterminedInfo((INT*)&cpUndeterminedStart, (INT*)&cchUndetermined))
    {
        return TRUE;
    }

    if(*pcpNew < cpUndeterminedStart)
    {
        *pcpNew = cpUndeterminedStart;
    }
    else if(*pcpNew > cpUndeterminedStart + cchUndetermined)
    {
        *pcpNew = cpUndeterminedStart + cchUndetermined;
    }

    // Get the offset to the new caret position.
    cpMove = *pcpNew - cpMin;

    // Is the caret moving left or right?
    vkCode = cpMove < 0 ? VK_LEFT : VK_RIGHT;

    // Get the RichEdit window handle.
    if(ped->TxGetWindow(&hWndRE) != NOERROR)
    {
        return TRUE;
    }

    // Don't need to do anything if the user's tapped where the caret already sits.
    if(cpMove != 0)
    {
        HIMC hIMC = ped->TxImmGetContext();

        if(!hIMC || !pImmEscape((HKL)NULL, hIMC, IME_ESC_SETCURSOR, (LPVOID)cpMove))
        {
            // Now loop through as though the user pressed the arrow keys.
            cpMove = abs(cpMove);

            // Freeze the display to stop the caret running around. This
            // gets thawed later when we process the last key up.
            _pdp->Freeze();

            ped->_fDisplayFrozen = TRUE;

            for(; cpMove > 0; cpMove-- )
            {
                chCode = 0;

                const int LPINF1 = 0x003B0001;
                const int LPINF2 = 0xC03B0001;

                // Take the same action as the edit control does here.
                nShift = KeyStateDownFlag;

                // Key goes down...
                ::PostMessage(hWndRE, WM_KEYDOWN, vkCode, LPINF1);
                //PostKeybdMessage(hWndRE, vkCode, nShift, 1, &nShift, &chCode);

                nShift &= ~KeyStateDownFlag;
                nShift |= KeyStatePrevDownFlag | KeyShiftNoCharacterFlag;

                // Key goes up...
                ::PostMessage(hWndRE, WM_KEYUP, vkCode, LPINF2);
                //PostKeybdMessage(hWndRE, vkCode, nShift, 1, &nShift, &chCode);
            }
        }

        if(hIMC)
        {
            ped->TxImmReleaseContext(hIMC);
        }
    }
    else
    {
        HIMC hIMC = ped->TxImmGetContext();

        if(!hIMC || !pImmEscape((HKL)NULL, hIMC, IME_ESC_SETCURSOR, (LPVOID)cpMove))
        {
            // Freeze the display to stop the caret running around. This
            // gets thawed later when we process the last key up.
            _pdp->Freeze();

            ped->_fDisplayFrozen = TRUE;

            chCode = 0;

            const int LPINF1 = 0x003B0001;
            const int LPINF2 = 0xC03B0001;

            // Take the same action as the edit control does here.
            nShift = KeyStateDownFlag;

            // Key goes down...
            ::PostMessage(hWndRE, WM_KEYDOWN, VK_RIGHT, LPINF1);
            //PostKeybdMessage(hWndRE, VK_RIGHT, nShift, 1, &nShift, &chCode);

            nShift &= ~KeyStateDownFlag;
            nShift |= KeyStatePrevDownFlag | KeyShiftNoCharacterFlag;

            // Key goes up...
            ::PostMessage(hWndRE, WM_KEYUP, VK_RIGHT, LPINF2);
            //PostKeybdMessage(hWndRE, VK_RIGHT, nShift, 1, &nShift, &chCode);

            // Take the same action as the edit control does here.
            nShift = KeyStateDownFlag;

            // Key goes down...
            ::PostMessage(hWndRE, WM_KEYDOWN, VK_LEFT, LPINF1);
            //PostKeybdMessage(hWndRE, VK_LEFT, nShift, 1, &nShift, &chCode);

            nShift &= ~KeyStateDownFlag;
            nShift |= KeyStatePrevDownFlag | KeyShiftNoCharacterFlag;

            // Key goes up...
            ::PostMessage(hWndRE, WM_KEYUP, VK_RIGHT, LPINF2);
            //PostKeybdMessage(hWndRE, VK_LEFT, nShift, 1, &nShift, &chCode);
        }

        if(hIMC)
        {
            ped->TxImmReleaseContext(hIMC);
        }
	}

    return FALSE;
}

/*
 *  CTxtSelection::SetIMEHighlight(pcpNew, pt, ptp);
 *
 *  GUYBARK ADD THIS!
 *
 *  Select a clause in the undetermined ime text following a tap.
 *
 *  Returns FALSE if we highlighted a clause, else TRUE.
 */
BOOL CTxtSelection::SetIMEHighlight(LONG *pcpNew, const POINT pt, CRchTxtPtr *ptp)
{
    CTxtEdit *ped;
    HIMC      hIMC = 0;
    LONG      cpUndeterminedStart, cchUndetermined;
    POINT     ptNew; 
    LONG      clauseInfo[256];
    BYTE      attribInfo[256];
    int       i, cpStart = 0, cpEnd = 0, cbClause, cchAttrib, cClause;	
    BOOL      bRet = TRUE;

    // Validate input and current ime context.
    if(!pcpNew || !ptp || !(ped = GetPed()))
    {
        goto Error;
    }

    hIMC = ped->TxImmGetContext();

    if(!hIMC)
    {
        goto Error;
    }

    // Don't do anything here if we're not in undetermined text.
    if(!ped->IsIMEComposition())
    {
        goto Error;
    }


	//	Windows CE Platforms Bug #10084		v-holee
	//  
	//	MSPY2 : Complete undetermined text while L buttom is not in the undetermined text.
	//
	if( LANG_CHINESE == PRIMARYLANGID( LOWORD( GetKeyboardLayout(0) ) ))
    {

	    if(ped->_ime->GetUndeterminedInfo((INT*)&cpUndeterminedStart, (INT*)&cchUndetermined))
	        goto Error;

		if(*pcpNew < cpUndeterminedStart || *pcpNew > cpUndeterminedStart + cchUndetermined)
		{

			BOOL retCode;

			retCode = pImmNotifyIME( hIMC, NI_COMPOSITIONSTR,
				CPS_COMPLETE, 0);
			
			if ( !retCode && !ped->_fIMECancelComplete )
			{
				// CPS_COMPLETE fail, try CPS_CANCEL.  This happen with some ime which do not support
				// CPS_COMPLETE option (e.g. ABC IME version 4 with Win95 simplified Chinese)
				retCode = pImmNotifyIME( hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
			}
			bRet = FALSE;
		}

        goto Error;
    }

    // Get the current ime undetermined text attributes.
    if((cchAttrib = pImmGetCompositionString(hIMC, GCS_COMPATTR, attribInfo, 255)) <= 0)
    {
        // Couldn't get attributes.
        goto Error;
    }

    // Try to find some converted ime text.
    for(i = 0; i < cchAttrib; i++)
    {
        if(attribInfo[i] == ATTR_TARGET_CONVERTED ||
           attribInfo[i] == ATTR_CONVERTED)
        {
            break;
        }
    }

    // If we didn't find any converted text, then we don't need to bother 
    // settings ime attributes, (ie highlighting anything).
    if(i == cchAttrib)
    {
        goto Error;
    }

    // Round up the cp to always be to the right of where the user tapped.
    _pdp->PointFromTp(*ptp, NULL, FALSE, ptNew, NULL, TA_TOP);

    if(ptNew.x < pt.x)
    {
        // Move caret from left to right of character.
        *pcpNew += 1;
    }

    // Force the caret to stay in the undetermined text.
    if(ped->_ime->GetUndeterminedInfo((INT*)&cpUndeterminedStart, (INT*)&cchUndetermined))
    {
        goto Error;
    }

    if(*pcpNew < cpUndeterminedStart)
    {
        *pcpNew = cpUndeterminedStart;
    }
    else if(*pcpNew > cpUndeterminedStart + cchUndetermined)
    {
        *pcpNew = cpUndeterminedStart + cchUndetermined;
    }

    // Get the clause info for the undetermined text.
    if((cbClause = pImmGetCompositionString(hIMC, GCS_COMPCLAUSE, clauseInfo, 255 * sizeof(LONG))) <= 0)
    {
        goto Error;
    }

    // Each clause datum is a cp which is LONG big here.
    cClause = cbClause / sizeof(LONG);	

    // Figure out which clause the user tapped on.
    for(i = 0; i < (cClause - 1); i++)
    {
        // REMEMBER! The clause is relative to the START of the undetermined text!
        if((*pcpNew >= clauseInfo[i] + cpUndeterminedStart) && (*pcpNew <= clauseInfo[i + 1] + cpUndeterminedStart))
        {
            cpStart = clauseInfo[i];
            cpEnd   = clauseInfo[i+1];

            // Return the end of the clause, to display the caret where the user expects.
            *pcpNew = cpEnd + cpUndeterminedStart;

            break;
        }
    }

    // Now highlight the tapped clause.
    for(i = 0; i < cchAttrib; i++)
    {
        if((i >= cpStart) && (i < cpEnd))
        {
            // Highlight the clause we tapped on
            attribInfo[i] = ATTR_TARGET_CONVERTED;
        }
        else
        {
            // All other clauses are marked converted, but not highlighted.
            attribInfo[i] = ATTR_CONVERTED;
        }
    }

    //	Now tell IME about it.
    ImmSetCompositionString(hIMC, SCS_CHANGEATTR, attribInfo, cchAttrib, NULL, 0);

    // No errors.
    bRet = FALSE;

Error:

    if(hIMC)
    {
        ped->TxImmReleaseContext(hIMC);
    }

    return bRet;
}

#endif // !TARGET_NT

/*
 * 	CTxtSelection::SelectWord(pt)
 *
 *	@mfunc
 *		Select word around a given point
 */
void CTxtSelection::SelectWord (
	const POINT pt)			//@parm Point of click
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectWord");

	_TEST_INVARIANT_

	// Get rp where the hit is
	if (_pdp->CpFromPoint(pt, NULL, this, NULL, FALSE) >= 0)
	{	
		// Extend both active and dead ends on word boundaries
		_cch = 0;							// Start with IP at pt
		SetExtend(FALSE);
		FindWordBreak(WB_MOVEWORDRIGHT);	// Go to end of word
		SetExtend(TRUE);
		FindWordBreak(WB_MOVEWORDLEFT);		// Extend to start of word
		GetRange(_cpAnchorMin, _cpAnchorMost);
		GetRange(_cpWordMin, _cpWordMost);

		if(!_fInAutoWordSel)
			_SelMode = smWord;

		// cpMost needs to be the active end
		if( _cch < 0 )
		{
			FlipRange();
		}

		Update(FALSE);
	}
}

/*
 * 	CTxtSelection::SelectUnit(pt, Unit)
 *
 *	@mfunc
 *		Select line/paragraph around a given point and enter 
 *		line/paragraph selection mode. In Outline View, convert
 *		SelectLine to SelectPara, and SelectPara to SelectPara
 *		along with all subordinates
 */
void CTxtSelection::SelectUnit (
	const POINT pt,		//@parm Point of click
	LONG		Unit)	//@parm tomLine or tomParagraph
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectPara");

	_TEST_INVARIANT_

	AssertSz(Unit == tomLine || Unit == tomParagraph,
		"CTxtSelection::SelectPara: Unit must equal tomLine/tomParagraph");

	LONG	 nHeading;
	CLinePtr rp(_pdp);

	// Get rp and selection active end where the hit is
	if (_pdp->CpFromPoint(pt, NULL, this, &rp, FALSE) >= 0)
	{
		LONG cchBackward, cchForward;
		BOOL fOutline = IsInOutlineView();

		if(Unit == tomLine && !fOutline)			// SelectLine
		{
			_cch = 0;								// Start with insertion
			cchBackward = -rp.RpGetIch();			//  point at pt
			cchForward  = rp->_cch;
			_SelMode = smLine;
		}
		else										// SelectParagraph
		{
			cchBackward = rp.FindParagraph(FALSE);	// Go to start of para
			cchForward  = rp.FindParagraph(TRUE);	// Extend to end of para
			_SelMode = smPara;
		}
		SetExtend(FALSE);
		Advance(cchBackward);

		if(Unit == tomParagraph && fOutline)		// Move para in outline
		{											//  view
			rp.AdjustBackward();					// If heading, include
			nHeading = rp.GetHeading();				//  subordinate	paras
			if(nHeading)							
			{											
				for(; rp.NextRun(); cchForward += rp->_cch)
				{
					LONG n = rp.GetHeading();
					if(n && n <= nHeading)
						break;
				}
			}
		}
		SetExtend(TRUE);
		Advance(cchForward);
		GetRange(_cpAnchorMin, _cpAnchorMost);
		Update(FALSE);
	}
}

/*
 * 	CTxtSelection::SelectAll()
 *
 *	@mfunc
 *		Select all text in story
 */
void CTxtSelection::SelectAll()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SelectAll");

	_TEST_INVARIANT_

	StopGroupTyping();

	LONG cchText = GetTextLength();

	Set( cchText,  cchText );
	Update(FALSE);
}

/*
 * 	CTxtSelection::ExtendSelection(pt)
 *
 *	@mfunc
 *		Extend/Shrink selection (moves active end) to given point
 */
void CTxtSelection::ExtendSelection (
	const POINT pt)		//@parm Point to extend to
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ExtendSelection");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchPrev = _cch;
	LONG		cp;
	LONG		cpMin, cpMost;
	BOOL		fAfterEOP;
	const BOOL	fWasInAutoWordSel = _fInAutoWordSel;
	INT			iDir = 0;
	CTxtEdit *	ped = GetPed();
	CLinePtr	rp(_pdp);
	CRchTxtPtr	rtp(ped);

	// GuyBark JupiterJ IME: Mimic the edit control's behavior here, and do 
	// not allow the selection to be extended while undetermined text exists.
	if(ped->IsIMEComposition())
	{
	    return;
	}

	StopGroupTyping();

	// Get rp and rtp at the point pt
	if (_pdp->CpFromPoint(pt, NULL, &rtp, &rp, TRUE) < 0)
		return;

	// If we are in word, line, or paragraph select mode, we need to make
	// sure the active end is correct.  If we are extending backward from
	// the first Unit selected, we want the active end to be at cpMin. If
	// we are extending forward from the first Unit selected, we want the
	// active end to be at cpMost.
	if (_SelMode != smNone)
	{
		cch = _cpAnchorMost - _cpAnchorMin;
		GetRange(cpMin, cpMost);
		cp = rtp.GetCp();

		if(cp <= cpMin  && _cch > 0)			// If active end changes,
			Set(_cpAnchorMin, -cch);			//  select the original
												//  Unit (will be extended
		if(cp >= cpMost && _cch < 0)			//  below)
			Set(_cpAnchorMost, cch);
	}

	SetExtend(TRUE);
	cch = rp.RpGetIch();
	if((_SelMode > smWord) &&					// If in line or para select
		cch == (LONG)rp->_cch)					//  modes and pt at EOL,
	{											//  make sure we stay on that
		rtp.Advance(-cch);						//  line
		rp.RpAdvanceCp(-cch);
		cch = 0;
	}

	SetCp(rtp.GetCp());							// Move active end to pt
												// Caret OK at BOL _unless_
	_fCaretNotAtBOL = _cch > 0;					//  forward selection
												// Now adjust selection
	if(_SelMode == smLine)						//  depending on mode
	{											// Extend selection by line
		if(_cch >= 0)							// Active end at cpMost
			cch -= rp->_cch;					// Setup to add chars to EOL
		Advance(-cch);
	}
	else if(_SelMode == smPara)
		Advance(rp.FindParagraph(_cch >= 0));	// Extend selection by para

	else
	{
		// If the sign of _cch has changed this means that the direction
		// of the selection is changing and we want to reset the auto
		// selection information.
		if ((_cch ^ cchPrev) < 0)
		{
			_fAutoSelectAborted = FALSE;
			_cpWordMin  = _cpAnchorMin;
			_cpWordMost = _cpAnchorMost;

		}

		cp = rtp.GetCp();
		fAfterEOP = rtp._rpTX.IsAfterEOP();

		_fInAutoWordSel = _SelMode != smWord && GetPed()->TxGetAutoWordSel() 
			&& !_fAutoSelectAborted
			&& (cp < _cpWordMin || cp > _cpWordMost);
	
		if(_fInAutoWordSel && !fWasInAutoWordSel)
		{
			CTxtPtr txtptr(GetPed(), _cpAnchor);

			// Extend both ends dead to word boundaries
			ExtendToWordBreak(fAfterEOP,
				_cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT); 

			if (_cch < 0)
			{
				// Direction is left so update word border on left
				_cpWordPrev = _cpWordMin;
				_cpWordMin = GetCp();
			}
			else
			{
				// Direction is right so update word border on right
				_cpWordPrev = _cpWordMost;
				_cpWordMost = GetCp();
			}

			//If we are at the beginning of a word already, we don't
			//need to extend the selection in the other direction.
			if (!txtptr.IsAtBOWord() && txtptr.GetChar() != ' ')
			{
				FlipRange();

				// start extending from the anchor
				Advance(_cpAnchor - GetCp());

				FindWordBreak(_cch < 0 ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT);

				if (_cch > 0)
				{
					// Direction is right so update word border on right
					_cpWordMost = GetCp();
				}
				else
				{
					// Direction is left so update word border on left
					_cpWordMin = GetCp();
				}
				FlipRange();
			}
		}
		else if(_fInAutoWordSel || _SelMode == smWord)
		{
			// Save direction
			iDir = cp <= _cpWordMin ? WB_MOVEWORDLEFT : WB_MOVEWORDRIGHT;

			// Extend selection by word
			if (_SelMode == smWord)
			{
				if(cp > _cpAnchorMost || cp < _cpAnchorMin)
					FindWordBreak(iDir);
				else
					Set(_cpAnchorMin, _cpAnchorMin - _cpAnchorMost);
			}
			else
			{
				ExtendToWordBreak(fAfterEOP, iDir); 
			}

			if (_fInAutoWordSel)
			{
				if (WB_MOVEWORDLEFT == iDir)
				{
					// Direction is left so update word border on left
					_cpWordPrev = _cpWordMin;
					_cpWordMin = GetCp();
				}
				else
				{
					// Direction is right so update word border on right
					_cpWordPrev = _cpWordMost;
					_cpWordMost = GetCp();
				}
			}
		}
		else if(fWasInAutoWordSel)
		{
			// If we are in between where the previous word ended and
			// the cp we auto selected to, then we want to stay in 
			// auto select mode.
			if (_cch < 0)
			{
				if (cp >= _cpWordMin && cp < _cpWordPrev)
				{
					// Set direction for end of word search
					iDir = WB_MOVEWORDLEFT;

					// Mark that we are still in auto select mode
					_fInAutoWordSel = TRUE;
				}
			}
			else if (cp <= _cpWordMost && cp >= _cpWordPrev)
			{
				// Mark that we are still in auto select mode
				_fInAutoWordSel = TRUE;

				// Set direction for end of word search
				iDir = WB_MOVEWORDRIGHT;
			}

			//We have to check to see if we are on the boundary between
			//words because we don't want to extend the selection until
			//we are actually beyond the current word.
			if (cp != _cpWordMost && cp != _cpWordMin)
			{
				if (_fInAutoWordSel)
				{
					// Auto selection still on so make sure we have the
					// entire word we are on selected
					ExtendToWordBreak(fAfterEOP, iDir); 
				}
				else
				{
					// FUTURE: Word has a behavior where it extends the
					// selection one word at a time unless you back up
					// and then start extending the selection again, in
					// which case it extends one char at a time.  We
					// follow this behavior.  However, Word will resume
					// extending a word at a time if you continue extending
					// for several words.  We just keep extending on char
					// at a time.  We might want to change this sometime.
	
					_fAutoSelectAborted = TRUE;
				}
			}
		}

		if (_fAutoSelectAborted)
		{
			// If we are in the range of a word we previously selected
			// we want to leave that selected. If we have moved back
			// a word we want to pop back an entire word. Otherwise,
			// leave the cp were it is.
			if (_cch < 0)
			{
				if (cp > _cpWordMin && cp < _cpWordPrev)
				{
					// In the range leave the range at the beginning of the word
					ExtendToWordBreak(fAfterEOP, WB_MOVEWORDLEFT); 
				}
				else if (cp >= _cpWordPrev)
				{
					AutoSelGoBackWord(&_cpWordMin, 
						WB_MOVEWORDRIGHT, WB_MOVEWORDLEFT);
				}
			}
			else if (cp < _cpWordMost && cp >= _cpWordPrev)
			{
				// In the range leave the range at the beginning of the word
				ExtendToWordBreak(fAfterEOP, WB_MOVEWORDRIGHT); 
			}
			else if (cp < _cpWordPrev)
			{			
				AutoSelGoBackWord(&_cpWordMost,
					WB_MOVEWORDLEFT, WB_MOVEWORDRIGHT);
			}
		}
	}

	// Save the current state of the _fCaretNotAtBOL flag since it affects a
	// a number of places.
	BOOL fCaretNotAtBOLSave = _fCaretNotAtBOL;

	// Is the selection at the beginning of the line?
	if (rp.RpGetIch() == 0)
	{
		_fCaretNotAtBOL = FALSE;
	}

	// Restore the flag to its original state.
	_fCaretNotAtBOL = fCaretNotAtBOLSave;

	// An OLE object cannot have an anchor point <b> inside </b> it,
	// but sometimes we'd like it to behave like a word. So, if
	// the direction changed, the object has to stay selected --
	// this is the "right thing" (kind of word selection mode)

	// if we had something selected and the direction changed
	if (cchPrev && (_cch ^ cchPrev) < 0)
	{	
		FlipRange();
		
		// see if an object was selected on the other end	 
		BOOL fObjectWasSelected = (_cch > 0 ? _rpTX.GetChar() 
											: _rpTX.GetPrevChar()) == WCH_EMBEDDING;
		// if it was, we want it to stay selected		
		if (fObjectWasSelected)
			Advance(_cch > 0 ? 1 : -1);

		FlipRange();
	}

	Update(TRUE);
}

/*
 * 	CTxtSelection::ExtendToWordBreak (fAfterEOP, iAction)
 *
 *	@mfunc
 *		Moves active end of selection to the word break in the direction
 *		given by iDir unless fAfterEOP = TRUE.  When this is TRUE, the
 *		cursor just follows an EOP marker and selection should be suppressed.
 *		Otherwise moving the cursor to the left of the left margin would
 *		select the EOP on the line above, and moving the cursor to the
 *		right of the right margin would select the first word in the line
 *		below.
 */
void CTxtSelection::ExtendToWordBreak (
	BOOL fAfterEOP,		//@parm Cursor is after an EOP
	INT	 iAction)		//@parm Word break action (WB_MOVEWORDRIGHT/LEFT)
{
	if(!fAfterEOP)
		FindWordBreak(iAction);
}

/*
 * 	CTxtSelection::CancelModes(fAutoWordSel)
 *
 *	@mfunc
 *		Cancel either all modes or Auto Select Word mode only
 */
void CTxtSelection::CancelModes (
	BOOL fAutoWordSel)		//@parm TRUE cancels Auto Select Word mode only
{							//	   FALSE cancels word, line and para sel mode
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CancelModes");
	_TEST_INVARIANT_

	if(fAutoWordSel)
	{
		if(_fInAutoWordSel)
		{
			_fInAutoWordSel = FALSE;
			_fAutoSelectAborted = FALSE;
		}
	}
	else
		_SelMode = smNone;	
}


///////////////////////////////////  Keyboard movements  ////////////////////////////////////

/*
 *	CTxtSelection::Left(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad left-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Left/Right-arrow IPs can go to within one character (treating CRLF
 *		as a character) of EOL.  They can never be at the actual EOL, so
 *		_fCaretNotAtBOL is always FALSE for these cases.  This includes
 *		the case with a right-arrow collapsing a selection that goes to
 *		the EOL, i.e, the caret ends up at the next BOL.  Furthermore,
 *		these cases don't care whether the initial caret position is at
 *		the EOL or the BOL of the next line.  All other cursor keypad
 *		commands may care.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Left (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Left");

	_TEST_INVARIANT_

	LONG cp;

	CancelModes();
	StopGroupTyping();

	if(!_fExtend && _cch)						// Collapse selection to
	{											//  nearest whole Unit before
		if(fCtrl)								//  cpMin
			Expander(tomWord, FALSE, NULL, &cp, NULL);
		Collapser(tomStart);					// Collapse to cpMin
	}
	else										// Not collapsing selection
	{
		if (!GetCp() ||							// Already at beginning of
			!BypassHiddenText(tomBackward))		//  story
		{										
			Beep();
			return FALSE;
		}
		if(IsInOutlineView() && (_fSelHasEOP ||	// If outline view with EOP
			_fExtend && _rpTX.IsAfterEOP()))	//  now or will have after
		{										//  this command,
			return Up(FALSE);					//  treat as up arrow
		}
		if(fCtrl)								// WordLeft
			FindWordBreak(WB_MOVEWORDLEFT);
		else									// CharLeft
			BackupCRLF();
	}
	_fCaretNotAtBOL = FALSE;					// Caret always OK at BOL

	Update(TRUE);
	return TRUE;
}

/*
 *	CTxtSelection::Right(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad right-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Right-arrow selection can go to the EOL, but the cp of the other
 *		end identifies whether the selection ends at the EOL or starts at
 *		the beginning of the next line.  Hence here and in general for
 *		selections, _fCaretNotAtBOL is not needed to resolve EOL/BOL
 *		ambiguities.  It should be set to FALSE to get the correct
 *		collapse character.  See also comments for Left() above.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Right (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Right");

	_TEST_INVARIANT_
	
	LONG		cchText;
	LONG		cp;

	CancelModes();
	StopGroupTyping();

	if(!_fExtend && _cch)						// Collapse selection to
	{											//  nearest whole Unit after
		if(fCtrl)								//  cpMost
			Expander(tomWord, FALSE, NULL, NULL, &cp);
		Collapser(tomEnd);
	}
	else										// Not collapsing selection
	{
		cchText = _fExtend ? GetTextLength() : GetAdjustedTextLength();
		if (GetCp() >= cchText ||				// Already at end of story
			!BypassHiddenText(tomForward))
		{
			Beep();								// Tell the user
			return FALSE;
		}
		if(IsInOutlineView() && _fSelHasEOP)	// If outline view with EOP,
			return Down(FALSE);					//  treat as down arrow
		if(fCtrl)								// WordRight
			FindWordBreak(WB_MOVEWORDRIGHT);
		else									// CharRight
			AdvanceCRLF();
	}
	_fCaretNotAtBOL = _fExtend;					// If extending to EOL, need

	Update(TRUE);								//  TRUE to get _xCaretReally
	return TRUE;								//  at EOL
}

/*
 *	CTxtSelection::Up(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad up-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Up arrow doesn't go to EOL regardless of _xCaretPosition (stays
 *		to left of EOL break character), so _fCaretNotAtBOL is always FALSE
 *		for Up arrow.  Ctrl-Up/Down arrows always end up at BOPs or the EOD.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Up (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Up");

	_TEST_INVARIANT_

	LONG		cchSave = _cch;					// Save starting position for
	LONG		cpSave = GetCp();				//  change check
	BOOL		fCollapse = _cch && !_fExtend;	// Collapse nondegenerate sel
	BOOL		fPTNotAtEnd;
	CLinePtr	rp(_pdp);
	LONG		xCaretReally = _xCaretReally;	// Save desired caret x pos

	CancelModes();
	StopGroupTyping();

	if(fCollapse)								// Collapse selection at cpMin
	{
		Collapser(tomTrue);
		_fCaretNotAtBOL = FALSE;				// Selections can't begin at
	}											//  EOL
	rp.RpSetCp(GetCp(), _fCaretNotAtBOL);		// Initialize line ptr

	if(fCtrl)									// Move to beginning of para
	{
		if(!fCollapse && 						// If no selection collapsed
			rp > 0 && !rp.RpGetIch())			//  and are at BOL,
		{										//  backup to prev BOL to make
			rp--;								//  sure we move to prev. para
			Advance(-(LONG)rp->_cch);
		}
		Advance(rp.FindParagraph(FALSE));		// Go to beginning of para
		_fCaretNotAtBOL = FALSE;				// Caret always OK at BOL
	}
	else										// Move up a line
	{											// If on first line, can't go
		fPTNotAtEnd = !CheckPlainTextFinalEOP();//  up
		if(rp <= 0 && fPTNotAtEnd)				// (Don't use !rp, which means
		{										//  rp that's out of range)
			if(!_fExtend)// &&_pdp->GetYScroll())
				UpdateCaret(TRUE);				// Be sure caret in view
		}
		else
		{
			LONG cch;
			BOOL fSelHasEOPInOV = IsInOutlineView() && _fSelHasEOP;
			if(fSelHasEOPInOV && _cch > 0)
			{
				rp.AdjustBackward();
				cch = rp->_cch;
				rp.AdvanceCp(-cch);				// Go to start of line
				Assert(!rp.GetIch());
				cch -= rp.FindParagraph(FALSE);	// Be sure that's the start
			}									//  of para in case of wrap
			else
			{
				cch = 0;
				if(fPTNotAtEnd)
				{
					cch = rp.RpGetIch();
					rp--;
				}
				cch += rp->_cch;
			}
			Advance(-cch);						// Move to previous BOL
			if (fSelHasEOPInOV && !_fSelHasEOP)	// If sel had EOP but doesn't
			{									//  after Advance, must be IP
				Assert(!_cch);					// Suppress restore of
				xCaretReally = -1;				//  _xCaretReally
			}										
			else if(!SetXPosition(xCaretReally, rp))// Set this cp corresponding
				Set(cpSave, cchSave);			//  to xCaretReally here, but
		}									 	//	 agree on Down()
	}

	if(GetCp() == cpSave && _cch == cchSave)
	{
		// Continue to select to the beginning of the first line
		// This is what 1.0 is doing
		if (_fExtend)
			return Home(fCtrl);

		Beep();									// Nothing changed, so beep
		return FALSE;
	}

	Update(TRUE);								// Update and then restore
	if(!_cch && !fCtrl && xCaretReally >= 0)	//  _xCaretReally conditionally
		_xCaretReally = xCaretReally;			// Need to use _cch instead of
												//  cchSave in case of collapse
	return TRUE;
}

/*
 *	CTxtSelection::Down(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad down-arrow key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		Down arrow can go to the EOL if the _xCaretPosition (set by
 *		horizontal motions) is past the end of the line, so
 *		_fCaretNotAtBOL needs to be TRUE for this case.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Down (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Down");

	_TEST_INVARIANT_

	LONG		cch;
	LONG		cchSave = _cch;					// Save starting position for
	LONG		cpSave = GetCp();				//  change check
	BOOL		fCollapse = _cch && !_fExtend;	// Collapse nondegenerate sel
	CLinePtr	rp(_pdp);
	LONG		xCaretReally = _xCaretReally;	// Save _xCaretReally

	CancelModes();
	StopGroupTyping();

	if(fCollapse)								// Collapse at cpMost
	{
		Collapser(tomEnd);
		_fCaretNotAtBOL = TRUE;					// Selections can't end at BOL
	}

	rp.RpSetCp(GetCp(), _fCaretNotAtBOL);

	if(fCtrl)									// Move to next para
	{
		Advance(rp.FindParagraph(TRUE));		// Go to end of para
		_fCaretNotAtBOL = FALSE;				// Next para is never at EOL
	}
	else if(_pdp->WaitForRecalcIli(rp + 1))		// Go to next line
	{
		LONG cch2;
		BOOL fSelHasEOPInOV = IsInOutlineView() && _fSelHasEOP;
		if(fSelHasEOPInOV && _cch < 0)
			cch2 = rp.FindParagraph(TRUE);
		else
		{
			cch2 = rp.GetCchLeft();				// Advance selection to end
			rp++;								//  of current line
		}
		Advance(cch2);
		if (fSelHasEOPInOV && !_fSelHasEOP)		// If sel had EOP but doesn't
		{										//  after Advance, must be IP  
			Assert(!_cch);						// Suppress restore of
			xCaretReally = -1;					//  _xCaretReally
		}										
		else if(!SetXPosition(xCaretReally, rp))// Set *this to cp <--> x
			Set(cpSave, cchSave);				// If failed, restore sel
	}
	else if(!_fExtend)  						// No more lines to pass
		// && _pdp->GetYScroll() + _pdp->GetViewHeight() < _pdp->GetHeight())
	{
		if (!GetPed()->IsRich()					// Plain-text,
			&& GetPed()->TxGetMultiLine()		//  multiline control
			&& !_fCaretNotAtBOL)				//	with caret OK at BOL
		{
			cch = Advance(rp.GetCchLeft());		// Advance selection to end
			if(!_rpTX.IsAfterEOP())				// If control doesn't end
				Advance(-cch);					//  with EOP, go back
		}
		UpdateCaret(TRUE);						// Be sure caret in view
	}

	if(GetCp() == cpSave && _cch == cchSave)
	{
		// Continue to select to the end of the lastline
		// This is what 1.0 is doing.
		if (_fExtend)
			return End(fCtrl);

 		Beep();									// Nothing changed, so beep
		return FALSE;
	}

	Update(TRUE);								// Update and then
	if(!_cch && !fCtrl && xCaretReally >= 0)	//  restore _xCaretReally
		_xCaretReally = xCaretReally;			// Need to use _cch instead of
	return TRUE;								//  cchSave in case of collapse
}

/*
 *	CTxtSelection::SetXPosition(xCaret, rp)
 *
 *	@mfunc
 *		Put this text ptr at cp nearest to xCaret.  If xCaret is in right
 *		margin, we put caret either at EOL (for lines with no para mark),
 *		or just before para mark
 *
 *	@rdesc
 *		TRUE iff could create measurer
 */
BOOL CTxtSelection::SetXPosition (
	LONG		xCaret,		//@parm Desired horizontal coordinate
	CLinePtr&	rp)			//@parm Line ptr identifying line to check
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetXPosition");

	_TEST_INVARIANT_

	LONG		cch = 0;
	CMeasurer 	me(_pdp, *this);

	if(IsInOutlineView())
	{
		BOOL fSelHasEOP = _fSelHasEOP;
		rp.AdjustForward();
		_fCaretNotAtBOL = FALSE;				// Leave at start of line
		while(rp->_fCollapsed)
		{
			if(_fMoveBack)
			{
				if(!rp.PrevRun())				// No more uncollapsed text
					return FALSE;				//  before current cp
				cch -= rp->_cch;
			}
			else
			{
				cch += rp->_cch;
				if(!rp.NextRun())				// No more uncollapsed text
					return FALSE;				//  after current cp
				if(_fExtend)
					_fCaretNotAtBOL = TRUE;		// Leave at end of line
			}
		}
		if(cch)
			Advance(cch);
		if(fSelHasEOP)
			return TRUE;
		if(cch)
			me.Advance(cch);
	}

	cch = rp->CchFromXpos(me, xCaret, NULL);	// Move out from start of line
	if(!_fExtend && cch == (LONG)rp->_cch &&	// Not extending, at EOL,
		rp->_cchEOP)							//  and have EOP:
	{											//  backup before EOP
		cch += me._rpTX.BackupCpCRLF();			// Note: me._rpCF/_rpPF
	}											//  are now inconsistent
	SetCp(me.GetCp());							//  but doesn't matter since
	_fCaretNotAtBOL = cch != 0;					//  me.GetCp() doesn't care

	return TRUE;
}

/*
 *	CTxtSelection::GetXCaretReally()
 *
 *	@mfunc
 *		Get _xCaretReally - horizontal scrolling + left margin
 *
 *	@rdesc
 *		x caret really
 */
LONG CTxtSelection::GetXCaretReally()
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::GetXCaretReally");

	_TEST_INVARIANT_

	RECT rcView;

	_pdp->GetViewRect( rcView );

	return _xCaretReally - _pdp->GetXScroll() + rcView.left;
}

/*
 *	CTxtSelection::Home(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad Home key is supposed to do
 *
 *	@rdesc
 *			TRUE iff movement occurred
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Home (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Home");

	_TEST_INVARIANT_

	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();

	CancelModes();
	StopGroupTyping();

	if(fCtrl) 									// Move to start of document
		SetCp(0);
	else
	{
		CLinePtr rp(_pdp);

		if(_cch && !_fExtend)					// Collapse at cpMin
		{
			Collapser(tomStart);
			_fCaretNotAtBOL = FALSE;			// Selections can't start at
		}										//  EOL

		rp.RpSetCp(GetCp(), _fCaretNotAtBOL);	// Define line ptr for
		Advance(-rp.RpGetIch());				//  current state. Now BOL
	}
	_fCaretNotAtBOL = FALSE;					// Caret always goes to BOL
	
	if(GetCp() == cpSave && _cch == cchSave)	// Beep if no change
	{
		Beep();
		return FALSE;
	}
	Update(TRUE);

	return TRUE;
}

/*
 *	CTxtSelection::End(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad End key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		On lines without paragraph marks (EOP), End can go all the way
 *		to the EOL.  Since this character position (cp) is the same as
 *		that for the start of the next line, we need _fCaretNotAtBOL to
 *		distinguish between the two possible caret positions.
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::End (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::End");

	_TEST_INVARIANT_

	LONG		cch;
	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();
	CLinePtr	rp(_pdp);
	
	CancelModes();
	StopGroupTyping();

	if(fCtrl)									// Move to end of document
	{
		SetCp(GetTextLength());
		_fCaretNotAtBOL = FALSE;

		goto Exit;
	}
	else if(!_fExtend && _cch)					// Collapse at cpMost
	{
		Collapser(tomEnd);
		_fCaretNotAtBOL = TRUE;					// Selections can't end at BOL
	}

	rp.RpSetCp(GetCp(), _fCaretNotAtBOL);		// Initialize line ptr
	cch = rp->_cch;								// Default target pos in line
	Advance(cch - rp.RpGetIch());				// Move active end to EOL

	if(!_fExtend && rp->_cchEOP && _rpTX.IsAfterEOP())// Not extending and 
	{											//		 have EOP:
		cch += BackupCRLF();					//  backup before EOP
	}
	_fCaretNotAtBOL = cch != 0;					// Decide ambiguous caret pos
												//  by whether at BOL

Exit:

	if(GetCp() == cpSave && _cch == cchSave)
	{
		Beep();									// No change, so Beep
		return FALSE;
	}
	Update(TRUE);
	return TRUE;
}

/*
 *	CTxtSelection::PageUp(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad PgUp key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::PageUp (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageUp");

	_TEST_INVARIANT_

	const LONG	cchSave = _cch;
	const LONG	cpSave  = GetCp();
	LONG		xCaretReally = _xCaretReally;
	
	CancelModes();
	StopGroupTyping();

	if (_cch && !_fExtend)						// Collapse selection
	{
		Collapser(tomStart);
		_fCaretNotAtBOL = FALSE;
	}

	if(fCtrl)									// Ctrl-PgUp: move to top
	{											//  of visible view for
		SetCp(GetPed()->TxGetMultiLine()		//  multiline but top of
			? _pdp->GetFirstVisibleCp()			//  text for SL
			: 0);
		_fCaretNotAtBOL = FALSE;
	}
	else if(_pdp->GetFirstVisibleCp() == 0)		// PgUp in top Pg: move to
	{											//  start of document
		SetCp(0);
		_fCaretNotAtBOL = FALSE;
	}
	else										// PgUp with scrolling to go
	{											// Scroll up one windowful
		ScrollWindowful(SB_PAGEUP);				//  leaving caret at same
												//  position in window
	}

	if(GetCp() == cpSave && _cch == cchSave)	// Beep if no change
	{
		Beep();
		return FALSE;
	}

	Update(TRUE);
	if(GetCp())									// Maintain x offset on page
		_xCaretReally = xCaretReally;			//  up/down
	return TRUE;
}

/*
 *	CTxtSelection::PageDown(fCtrl)
 *
 *	@mfunc
 *		do what cursor-keypad PgDn key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::PageDown (
	BOOL fCtrl)		//@parm TRUE iff Ctrl key is pressed (or being simulated)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PageDown");

	_TEST_INVARIANT_

	const LONG	cchSave			= _cch;
	LONG		cpMostVisible;
	const LONG	cpSave			= GetCp();
	POINT		pt;
	CLinePtr	rp(_pdp);
	LONG		xCaretReally	= _xCaretReally;

	CancelModes();
	StopGroupTyping();
		
	if (_cch && !_fExtend)						// Collapse selection
	{
		Collapser(tomStart);
		_fCaretNotAtBOL = TRUE;
	}

	_pdp->GetCliVisible(&cpMostVisible, fCtrl);		
	
	if(fCtrl)									// Move to end of last
	{											//  entirely visible line
		RECT rcView;

		SetCp(cpMostVisible);

		if (_pdp->PointFromTp(*this, NULL, TRUE, pt, &rp, TA_TOP) < 0)
			return FALSE;

		_fCaretNotAtBOL = TRUE;

		_pdp->GetViewRect(rcView);

		if(rp > 0 && pt.y + rp->_yHeight > rcView.bottom)
		{
			Advance(-(LONG)rp->_cch);
			rp--;
		}

		if(!_fExtend && !rp.GetCchLeft() && rp->_cchEOP)
		{
			BackupCRLF();						// After backing up over EOP,
			_fCaretNotAtBOL = FALSE;			//  caret can't be at EOL
		}
	}
	else if(cpMostVisible == (LONG)GetTextLength())
	{											// Move to end of text
		SetCp(GetTextLength());
		_fCaretNotAtBOL = !_rpTX.IsAfterEOP();
	}
	else
	{
		if(!ScrollWindowful(SB_PAGEDOWN))		// Scroll down 1 windowful
			return FALSE;
	}

	if(GetCp() == cpSave && _cch == cchSave)	// Beep if no change
	{
		Beep();
		return FALSE;
	}

	Update(TRUE);
	_xCaretReally = xCaretReally;
	return TRUE;
}

/*
 *	CTxtSelection::ScrollWindowful(wparam)
 *
 *	@mfunc
 *		Sroll up or down a windowful
 *
 *	@rdesc
 *		TRUE iff movement occurred
 */
BOOL CTxtSelection::ScrollWindowful (
	WPARAM wparam)		//@parm SB_PAGEDOWN or SB_PAGEUP
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ScrollWindowful");
												// Scroll windowful
	_TEST_INVARIANT_

	POINT pt;									//  leaving caret at same
	CLinePtr rp(_pdp);							//  point on screen
	LONG cpFirstVisible = _pdp->GetFirstVisibleCp();
	LONG cpLastVisible;
	LONG cpMin;
	LONG cpMost;

	GetRange(cpMin, cpMost);

	// Get last character in the view
	_pdp->GetCliVisible(&cpLastVisible, TRUE);

	// Is the active end in the visible area of the
	// control?
	if ((cpMin < cpFirstVisible && _cch <= 0) || (cpMost > cpLastVisible && _cch >= 0))
	{
		// Not in the view - we need to calculate a new range for the selection
		SetCp(cpFirstVisible);

		// The real caret postion is now at the beginning of the line.
		_xCaretReally = 0;
	}

	if(_pdp->PointFromTp(*this, NULL, _fCaretNotAtBOL, pt, &rp, TA_TOP) < 0)
		return FALSE;

	// The point is visible so use that
	pt.x = _xCaretReally;

	pt.y += rp->_yHeight / 2;

	_pdp->VScroll(wparam, 0);

	if( _fExtend )
	{
		// disable auto word select -- if we have to use ExtendSelection()
		// for non-mouse operations, let's try to get rid of its side-effects
		BOOL fInAutoWordSel = _fInAutoWordSel;
		_fInAutoWordSel = FALSE;
		ExtendSelection(pt);
		_fInAutoWordSel = fInAutoWordSel;
	}
	else
	{
		SetCaret(pt, FALSE);
	}
	return TRUE;
}

///////////////////////// Keyboard support by jonmat //////////////////////////////

void CTxtSelection::CheckChangeKeyboardLayout (
	BOOL fChangedFont )	// @parm TRUE if charformat has changed.
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeKeyboardLayout");

	W32->CheckChangeKeyboardLayout(this, fChangedFont);
}

void CTxtSelection::CheckChangeFont (
	CTxtEdit * const ped,
	BOOL fEnableReassign,	// @parm Do we enable CTRL key?
	const WORD lcID,		// @parm LCID from WM_ message
	UINT cpg )				// @parm code page to use (could be ANSI for far east with IME off)
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::CheckChangeFont");

	W32->CheckChangeFont(this, ped, fEnableReassign, lcID, cpg);
}


//////////////////////////// PutChar, Delete, Replace  //////////////////////////////////

/*
 *	CTxtSelection::PutChar(ch, fOver, publdr)
 *
 *	@mfunc
 *		Insert or overtype a character
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL CTxtSelection::PutChar (
	TCHAR		ch,			//@parm Char to put
	BOOL		fOver,		//@parm TRUE if overtype mode
	IUndoBuilder *publdr)	//@parm If non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PutChar");

	_TEST_INVARIANT_

	LONG iFormatSave = -1;

    const CParaFormat *ppf = GetPF();
    if (NULL == ppf)
        return FALSE;
	

	if(ch == TAB && ppf->InTable())
	{
		LONG cpMin, cpMost;
		LONG cch = GetRange(cpMin, cpMost);
		LONG cch0 = 0;
		LONG iDir = GetKeyState(VK_SHIFT) & 0x8000 ? -1 : 1;
		if(_fSelHasEOP)							// If selection has an EOP
		{										//  collapse to cpMin and
			Collapser(tomStart);				//  go forward
			iDir = 1;
			if(!ppf->InTable())
			{
				Update(TRUE);
				return TRUE;
			}
		}
		if((_cch ^ iDir) < 0)					// If at cpMost going back or
			FlipRange();						//  at cpMin going forward,
												//  switch active end
		CRchTxtPtr rtp(*this);

		CancelModes();
		StopGroupTyping();

		if(iDir < 0 || _cch)
			rtp.Advance(-1);

        const CParaFormat *rtppf = rtp.GetPF();
        if (NULL == rtppf)
            return FALSE;

		// Scan for start/end of next/prev cell
		do
		{
			ch  = rtp.GetChar();
		} while(rtp.Advance(iDir) && ch != CELL && rtppf->InTable());

		if(ch != CELL)
		{
			if(iDir < 0)
				return FALSE;
insertRow:	rtp.BackupCRLF();				// Tabbed past end of table:
			Set(rtp.GetCp(), 0);			//  insert new row
			return InsertEOP(publdr);
		}
		if(iDir > 0)						// Check for IP between CELL
		{									//  and CR at row end
			if(rtp.GetChar() == CR)
				rtp.AdvanceCRLF();			// Bypass row end
		}
		for(cch = 0;						// Determine cchSel
			(ch = rtp.GetChar()) != CELL && rtppf->InTable();
			cch += iDir)
		{
			cch0 = rtp.Advance(iDir);
			if(!cch0)
				break;
		}
		if(iDir > 0)						// Tabbing forward
		{
			if(ch != CELL)					// Went past end of table
				goto insertRow;				//  so go insert new row
		}
		else if(cch)						// Tabbing backward with
		{									//  nondegenerate selection
			if(cch0)						// Didn't run into start of story
			{
				rtp.Advance(1);				// Advance over CELL. Then if at
				if(rtp.GetChar() == CR)		//  end of row, advance over EOP
					cch += rtp.AdvanceCRLF();
			}
			else							// Ran into start of story
				cch -= 1;					// Include another char since
		}									//  broke out of for loop
		else if(cpMin > 1)					// Handles tabbing back over
			rtp.Advance(1);					//  adjacent null cells

		Set(rtp.GetCp(), cch);
		_fCaretNotAtBOL = FALSE; 
		Update(TRUE);
		return TRUE;
	}

	// EOPs might be entered by ITextSelection::TypeText()
	if(IsEOP(ch))
	{
		if(!GetPed()->TxGetMultiLine())				// EOP isn't allowed in
			return FALSE;							//  single line controls
		_fExtend = ch == VT ? TRUE : FALSE;			// VT is for Shift-Enter						
		return InsertEOP(publdr);					// This code treats FF as
	}												//  ordinary CR

	if( publdr )
	{
		publdr->SetNameID(UID_TYPING);
		publdr->StartGroupTyping();
	}

	if(!CheckTextLength(1))							// Return if we can't
		return FALSE;								//  add even 1 more char

	// The following if statement implements Word's "Smart Quote" feature.
	// To build this in, we still need an API to turn it on and off.  This
	// could be EM_SETSMARTQUOTES with wparam turning the feature on or off.
	// murrays. NB: this needs localization for French, German, and many
	// other languages (unless system can provide open/close chars given
	// an LCID).

	if ((ch == '\'' || ch == '"') &&				// Smart quotes
		SmartQuotesEnabled() &&
		PRIMARYLANGID(GetKeyboardLayout(0)) == LANG_ENGLISH)
	{
		LONG	cp = GetCpMin();					// Open vs close depends
		CTxtPtr tp(GetPed(), cp - 1);				//  on char preceding
													//  selection cpMin
		ch = (ch == '"') ? RDBLQUOTE : RQUOTE;		// Default close quote
													//  or apostrophe. If at
		if(!cp || IsWhiteSpace(tp.GetChar()))		//  BOStory or preceded
			ch--;									//  by whitespace, use
	}												//  open quote/apos

	// BEFORE we do "dual-font", we sync the keyboard and current font's
	// (_iFormat) charset if it hasn't been done.
	if ( !_fDualFontMode )
		CheckSynchCharSet();

	SetupDualFont(ch, &iFormatSave);

	SetExtend(TRUE);								// Tell Advance() to
	if(fOver)										//  select chars
	{												// If nothing selected and
		if(!_cch && !_rpTX.IsAtEOP())				//  not at EOP char, try
		{											//  to select char at IP
			LONG iFormatSave2 = Get_iCF();			// Remember char's format

			Advance(1);
			ReplaceRange( 0, NULL, publdr,
				SELRR_REMEMBERENDIP);				// Delete this character.
			ReleaseFormats(_iFormat, -1);
			_iFormat = iFormatSave2;					// Restore char's format.
		}
	}
	else if(_SelMode == smWord && ch != TAB && _cch)// Replace word selection
	{
		// the code below wants the active end to be at the end of the
		// word.  Make sure we're in that case.

		// FUTURE: (alexgo, andreib), _cch will only be less than zero
		// in certain weird timing situations where we get a mouse move
		// message in between the double click and mouse up messages.
		// we should rethink how we process messages && the ordering thereof.
		if( _cch < 0 )
		{
			FlipRange();
		}

													// Leave word break chars
		CTxtPtr tp(_rpTX);							//  at end of selection
		Assert(_cch > 0);

		tp.AdvanceCp(-1);
		if(tp.GetCp() && tp.FindWordBreak(WB_ISDELIMITER))// Delimeter at sel end
			FindWordBreak(WB_LEFTBREAK);			// Backspace over it, etc.
	}

	_fIsChar = TRUE;								// Give info to UpdateView

	AdjustEndEOP(NEWCHARS);
	ReplaceRange(1, &ch, publdr, SELRR_REMEMBERRANGE);
	_fIsChar = FALSE;

// We no longer restoring the previous font replaced by the CTxtSelection::SetupDualFont
// I leave the following code in just in case we want to return to this "feature" again
// in the future.  v-honwch

    // GuyBark JupiterJ 49758:
    // Quite right! We don't want to reset the font back to what it was
    // before our smart font changing work here. Otherwise the user doesn't
    // see the font name of the font they're using to add text. 
// #if 1
#if 0
	RestoreDualFont(iFormatSave);
#endif
	SetDualFontMode(FALSE);

	CheckUpdateWindow();							// Need to update display
													//  for pending chars.
	return TRUE;									
}													

/*
 *	CTxtSelection::SetIsChar(BOOL f)
 *
 *	@mfunc
 *		_fIsChar prevents replace range from resetting the
 *		update window flag. This function allows clients,
 *		like IME, to control the state of this flag.
 */
void CTxtSelection::SetIsChar(BOOL f)
{
	_fIsChar = f;
}

/*
 *	CTxtSelection::CheckUpdateWindow()
 *
 *	@mfunc
 *		If it's time to update the window,
 *		after pending-typed characters,
 *		do so now. This is needed because
 *		WM_PAINT has a lower priority than
 *		WM_CHAR.
 */
void CTxtSelection::CheckUpdateWindow()
{
	DWORD ticks = GetTickCount();
	DWORD delta = ticks - _ticksPending;

	if ( 0 == _ticksPending )
		_ticksPending = ticks;
	else if(delta >= ticksPendingUpdate)
		GetPed()->TxUpdateWindow();
}

/*
 *	CTxtSelection::BypassHiddenText(iDir)
 *
 *	@mfunc
 *		Bypass hidden text forward/backward for iDir positive/negative
 *
 *	@rdesc
 *		TRUE if succeeded or no hidden text. FALSE if at document limit
 *		(end/start for Direction positive/negative) or if hidden text between
 *		cp and that limit.
 */
BOOL CTxtSelection::BypassHiddenText(
	LONG iDir)
{
	if(iDir > 0)
		_rpCF.AdjustForward();
	else
		_rpCF.AdjustBackward();

	if(!(GetPed()->GetCharFormat(_rpCF.GetFormat())->dwEffects & CFE_HIDDEN))
		return TRUE;

	BOOL fExtendSave = _fExtend;
	SetExtend(FALSE);

	CCFRunPtr rp(*this);
	LONG cch = (iDir > 0)
			 ? rp.FindUnhiddenForward() : rp.FindUnhiddenBackward();

	BOOL bRet = !rp.IsHidden();				// Note whether still hidden
	if(bRet)								// It isn't:
		Advance(cch);						//  bypass hidden text
	SetExtend(fExtendSave);
	return bRet;
}

/*
 *	CTxtSelection::InsertEOP(publdr)
 *
 *	@mfunc
 *		Insert EOP character(s)
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL CTxtSelection::InsertEOP (
	IUndoBuilder *publdr)	//@parm If non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::InsertEOP");

	_TEST_INVARIANT_

	LONG	cchEOP = GetPed()->Get10Mode() ? 2 : 1;
	BOOL	fResult = FALSE;
	LONG	i, iFormatSave;
	WCHAR *	pch = _fExtend && IsRich() ? TEXT("\v") : szCRLF;
	const CParaFormat *pPF = GetPF();		// Get paragraph format
	WCHAR	szBlankRow[MAX_TAB_STOPS + 1];

    if (NULL == pPF)
    {
        return FALSE;  //nothing we can do
    }

	if( publdr )
	{
		publdr->StartGroupTyping();
		publdr->SetNameID(UID_TYPING);
	}

	if(pPF->InTable())
	{
		pch = szBlankRow;
		if(GetCp() && !_rpTX.IsAfterEOP())
		{
			while(!_rpTX.IsAtEOP() && Advance(1))
				;
			*pch++ = CR;
		}
		for(i = pPF->cTabCount; i--; *pch++ = CELL)
			;
		*pch++ = CR;
		pch = szBlankRow;
		cchEOP = pPF->cTabCount + 1;
	}

	if (CheckTextLength(cchEOP))			// If cchEOP chars can fit...
	{
		iFormatSave = Get_iCF();			// Save CharFormat before EOP
											// Get_iCF() does AddRefFormat()
		if (pPF->wNumbering)				// Bullet paragraph: EOP has
		{									//  desired bullet CharFormat
			CFormatRunPtr rpCF(_rpCF);		// Get run pointers for locating
			CTxtPtr		  rpTX(_rpTX);		//  EOP CharFormat

			rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
			rpCF.AdjustBackward();
			Set_iCF(rpCF.GetFormat());		// Set _iFormat to EOP CharFormat
		}

		// Put in approriate EOP mark
		fResult = ReplaceRange(cchEOP, pch,	// If Shift-Enter, insert VT
			publdr, SELRR_REMEMBERRANGE);

		Set_iCF(iFormatSave);				// Restore _iFormat if changed

		ReleaseFormats(iFormatSave, -1);	// Release iFormatSave
		if(pPF->InTable() && cchEOP > 1)
		{
			if(*pch == CR)
				cchEOP--;
			Advance(-cchEOP);
			_fCaretNotAtBOL = FALSE;
			Update(FALSE);
		}
	}
	return fResult;
}

/*
 *	CTxtSelection::Delete(fCtrl, publdr)
 *
 *	@mfunc
 *		Delete the selection. If fCtrl is true, this method deletes from
 *		min of selection to end of word
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL CTxtSelection::Delete (
	DWORD fCtrl,			//@parm If TRUE, Ctrl key depressed
	IUndoBuilder *publdr)	//@parm if non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Delete");

	_TEST_INVARIANT_

	SELRR	mode = SELRR_REMEMBERRANGE;

	AssertSz(!GetPed()->TxGetReadOnly(), "CTxtSelection::Delete(): read only");

	if(!_cch)
		BypassHiddenText(tomForward);

	if( publdr )
	{
		publdr->StopGroupTyping();
		publdr->SetNameID(UID_DELETE);
	}

	SetExtend(TRUE);						// Setup to change selection
	if(fCtrl)
	{										// Delete to word end from cpMin
		Collapser(tomStart);				//  (won't necessarily repaint,
		FindWordBreak(WB_MOVEWORDRIGHT);	//  since won't delete it)
	}

	if(!_cch)								// No selection
	{
		mode = SELRR_REMEMBERCPMIN;

		if(!AdvanceCRLF())					// Try to select char at IP
		{
			Beep();							// End of text, nothing to delete
			return FALSE;
		}
	}
	AdjustEndEOP(NONEWCHARS);
	ReplaceRange(0, NULL, publdr, mode);	// Delete selection
	return TRUE;
}

/*
 *	CTxtSelection::BackSpace(fCtrl, publdr)
 *
 *	@mfunc
 *		do what keyboard BackSpace key is supposed to do
 *
 *	@rdesc
 *		TRUE iff movement occurred
 *
 *	@comm
 *		This routine should probably use the Move methods, i.e., it's
 *		logical, not directional
 *
 *	@devnote
 *		_fExtend is TRUE iff Shift key is pressed or being simulated
 */
BOOL CTxtSelection::Backspace (
	BOOL fCtrl,		//@parm TRUE iff Ctrl key is pressed (or being simulated)
	IUndoBuilder *publdr)	//@parm If not-NULL, where to put the antievents
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::Backspace");

	_TEST_INVARIANT_

	SELRR	mode = SELRR_REMEMBERRANGE;
	
	AssertSz(!GetPed()->TxGetReadOnly(),
		"CTxtSelection::Backspace(): read only");

	_fCaretNotAtBOL = FALSE;

	if( publdr )
	{
		publdr->SetNameID(UID_TYPING);

		if( _cch || fCtrl)
			publdr->StopGroupTyping();
	}

#ifdef PWD_JUPITER
    // GuyBark Jupiter 33720: Try to mimic the Word97 behavior.
    // If the user is at the start of an indented paragraph, and 
    // they hit Backspace, then we should not delete any text, 
    // instead we should remove the indent. Kind of like backspacing 
    // over the indent. In this case, if the line was bulleted or 
    // part of a numbered list, the paragraph loses that attribute.

    // Are we at the start of a paragraph? (Or at the start of the document?)

    // Only do this is there's nothing selected. If there is a selection,
    // then the backspace should delete the selection.

    if(!_cchSel && (_rpTX.IsAfterEOP() || (GetCp() == 0)))
    {
        CParaFormat PF;

        // Yes. Get the paragraph attributes for this paragraph.
        GetParaFormat(&PF);

        // Does the paragraph have an indent? Ignore paragraphs in tables.
        if(!PF.InTable() && (PF.dxStartIndent || PF.dxOffset))
        {
            // Yes. So lose the indent. Also drop the bullet or
            // numbered list attribute if it had it.

            PF.dwMask = PFM_NUMBERING | PFM_OFFSET | PFM_NUMBERINGSTART | 
                        PFM_NUMBERINGSTYLE | PFM_NUMBERINGTAB | PFM_STARTINDENT;

            PF.dxStartIndent   = 0;
            PF.dxOffset        = 0;
            PF.wNumbering      = 0;
            PF.wNumberingStyle = 0;
            PF.wNumberingTab   = 0;

            // Now set the new attributes for the paragraph.
            SetParaFormat(&PF, publdr);

            // Don't change anything else here.
            return TRUE;
        }
    }
#endif // PWD_JUPITER

	// Japanese special fix for determined undo for IME3.
	// In pWord, CTRL+BS is "delete word left". But in Japanese IME3, CTRL+BS is "determined undo".
	// So in Japanese, if IME is ON, "delete word left" is set disable.
	UINT	uKbdCodePage = 0;
	uKbdCodePage = ConvertLanguageIDtoCodePage(GetKeyboardLCID());
	if(uKbdCodePage == _JAPAN_CP){				// JAPAN only
		CTxtEdit *ped = NULL;
		HIMC      hIMC = 0;
		ped = GetPed();
		if(ped){
			hIMC = ped->TxImmGetContext();
			if(hIMC){
				if(pImmGetOpenStatus(hIMC)){
					fCtrl = FALSE;
				}
				ped->TxImmReleaseContext(hIMC);
			}
		}
	}

	SetExtend(TRUE);						// Set up to extend range
	if(fCtrl)								// Delete word left
	{
		if(!GetCpMin())						// Beginning of story: no word
		{									//  to delete
			Beep();
			return FALSE;
		}
		Collapser(tomStart);				// First collapse to cpMin
		if(!BypassHiddenText(tomBackward))
			goto beep;
		FindWordBreak(WB_MOVEWORDLEFT);		// Extend word left
	}
	else if(!_cch)							// Empty selection
	{										// Try to select previous char
		if (!BypassHiddenText(tomBackward) ||
			!BackupCRLF())
		{									// Nothing to delete
beep:		Beep();
			return FALSE;
		}
		mode = SELRR_REMEMBERENDIP;

		if( publdr )
			publdr->StartGroupTyping();
	}

	ReplaceRange(0, NULL, publdr, mode);	// Delete selection
	return TRUE;
}

/*
 *	CTxtSelection::ReplaceRange(cchNew, pch, publdr, SELRRMode)
 *
 *	@mfunc
 *		Replace selected text by new given text and update screen according
 *		to _fShowCaret and _fShowSelection
 *
 *	@rdesc
 *		length of text inserted
 */
LONG CTxtSelection::ReplaceRange (
	LONG cchNew,			//@parm Length of replacing text or -1 to request
							// <p pch> sz length
	const TCHAR *pch,		//@parm Replacing text
	IUndoBuilder *publdr,	//@parm If non-NULL, where to put anti-events
	SELRR SELRRMode)		//@parm what to do about selection anti-events.
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::ReplaceRange");

	_TEST_INVARIANT_

	LONG		cchKeep;
	LONG		cchNewSave;
	LONG		cchOld;
	LONG		cchText		= GetTextLength();
	LONG		cpFirstRecalc;
	LONG		cpMin, cpMost;
	LONG		cpSave;
	BOOL		fDeleteAll = FALSE;
	BOOL		fHeading = FALSE;
	const BOOL	fShowCaret	= _fShowCaret;
	const BOOL	fUpdateView = _fShowSelection;

	CancelModes();

	if(cchNew < 0)
		cchNew = wcslen(pch);

	if(!_cch && !cchNew)						// Nothing to do
		return 0;

	if (!GetPed()->IsStreaming() &&				// If not pasting,
		(!_cch && *pch != CR &&					// Don't insert bet CELL & CR
		 CRchTxtPtr::GetChar() == CR && GetPrevChar() == CELL ||
		 _cch != cchText && (IsInOutlineView() && 
		  CRchTxtPtr::GetPF()->wEffects & PFE_COLLAPSED ||
		 GetCF()->dwEffects & CFE_HIDDEN)))
	{											// Don't insert into collapsed
		Beep();									//  or hidden region (should
		return 0;								//  only happen if whole story
	}											//  collapsed or hidden)

	GetPed()->GetCallMgr()->SetSelectionChanged();

	CheckTableSelection();
	cchOld = GetRange(cpMin, cpMost);

	if(	cpMin  > min(_cpSel, _cpSel + _cchSel) ||	// If new sel doesn't
		cpMost < max(_cpSel, _cpSel + _cchSel))		//  contain all of old
    {                                               //  sel, remove old sel
        ShowSelection(FALSE);
        _fShowCaret = TRUE;     
    }

	_fCaretNotAtBOL = FALSE;
	_fShowSelection = FALSE;					// Suppress the flashies
	cchKeep = cchText - cchOld;					// Number of untouched chars
	
	// If we are streaming in text or RTF data, don't bother with incremental
	// recalcs.  The data transfer engine will take care of a final recalc
	if( !GetPed()->IsStreaming() )
	{
		// Do this before calling ReplaceRange() so that UpdateView() works
		// AROO !!!	Do this before replacing the text or the format ranges!!!
		if(!_pdp->WaitForRecalc(cpMin, -1))
		{
			Tracef(TRCSEVERR, "WaitForRecalc(%ld) failed", cpMin);
			goto err;
		}
	}

	if( publdr )
	{
		Assert(SELRRMode != SELRR_IGNORE);

		// use the selection AntiEvent mode to determine what to do for undo
		LONG cp, cch = 0;

		if( SELRRMode == SELRR_REMEMBERRANGE )
		{
			cp = GetCp();
			cch = _cch;
		}
		else if( SELRRMode == SELRR_REMEMBERENDIP )
		{
			cp = cpMost;
		}
		else
		{
			Assert( SELRRMode == SELRR_REMEMBERCPMIN );
			cp = cpMin;
		}

		HandleSelectionAEInfo(GetPed(), publdr, cp, cch, cpMin + cchNew, 
			0, SELAE_MERGE);
	}
			
	if(_cch == cchText && !cchNew)				// For delete all, set
	{											//  up to choose Normal
		fDeleteAll = TRUE;						//  or Heading 1
		FlipRange();
		fHeading = IsInOutlineView() && IsHeadingStyle(GetPF()->sStyle);
	}

	cpSave		= cpMin;
	cpFirstRecalc = cpSave;
	cchNewSave	= cchNew;
	cchNew		= CTxtRange::ReplaceRange(cchNew, pch, publdr, SELRR_IGNORE);
    _cchSel     = 0;							// No displayed selection
    _cpSel      = GetCp();						
	cchText		= GetTextLength();				// Update total text length

	if(cchNew != cchNewSave)
	{
		Tracef(TRCSEVERR, "CRchTxtPtr::ReplaceRange(%ld, %ld, %ld) failed", GetCp(), cchOld, cchNew);
		goto err;
	}

	// The cp should be at *end* (cpMost) of replaced range (it starts
	// at cpMin of the prior range).	
	AssertSz(_cpSel == cpSave + cchNew && _cpSel <= cchText,
		"CTxtSelection::ReplaceRange() - Wrong cp after replacement");

	// If no new and no old, return value better be 0
	Assert(cchNew > 0 || cchOld > 0 || cchKeep == cchText);

	cchNew = cchText - cchKeep;					// Actual cchNew inserted
	_fShowSelection = fUpdateView;

	if(fDeleteAll)								// When all text is deleted
    {											//  use Normal style unless
        CParaFormat PF;							//  in Outline View and first
        PF.dwMask = PFM_STYLE;					//  para was a heading
        PF.sStyle = fHeading ? STYLE_HEADING_1 : STYLE_NORMAL;
        SetParaStyle(&PF, NULL);
		Update_iFormat(-1);
    }

	// Only update caret if inplace active
	if (GetPed()->fInplaceActive())
		UpdateCaret(fUpdateView);				// May need to scroll
	else										// Update caret when we get
		GetPed()->_fScrollCaretOnFocus = TRUE;	//  focus again

	return cchNew;

err:
	TRACEERRSZSC("CTxtSelection::ReplaceRange()", E_FAIL);
	Tracef(TRCSEVERR, "CTxtSelection::ReplaceRange(%ld, %ld)", cchOld, cchNew);
	Tracef(TRCSEVERR, "cchText %ld", cchText);

	return cchText - cchKeep;
}

/*
 *	CTxtSelection::GetPF()
 *	
 *	@mfunc
 *		Return ptr to CParaFormat at active end of this selection. If no PF
 *		runs are allocated, then return ptr to default format.  If active
 *		end is at cpMost of a nondegenerate selection, use the PF of the
 *		previous char (last char in selection). 
 *	
 *	@rdesc
 *		Ptr to CParaFormat at active end of this selection
 */
const CParaFormat* CTxtSelection::GetPF()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtSelection::GetPF");

	if(_cch > 0)
		_rpPF.AdjustBackward();
	const CParaFormat* pPF = GetPed()->GetParaFormat(_rpPF.GetFormat());
	if (NULL == pPF)
	{
	    Assert(pPF);
    }

	if(_cch > 0)
		_rpPF.AdjustForward();
	return pPF;
}

/*
 *	CTxtSelection::CheckTableSelection()
 *	
 *	@mfunc
 *		apply CCharFormat *pCF to this selection.  If range is an IP
 *		and fApplyToWord is TRUE, then apply CCharFormat to word surrounding
 *		this insertion point
 *
 *	@rdesc
 *		HRESULT = NOERROR if no error
 */
void CTxtSelection::CheckTableSelection ()
{
	if(!_fSelHasEOP && GetPF()->InTable())			// For now, don't let
	{												//  table CELLs be
		LONG	cpMin, cpMost;						//  deleted, unless in
		CTxtPtr tp(_rpTX);							//  complete rows

		GetRange(cpMin, cpMost);
		if(_cch > 0)								
			tp.AdvanceCp(-_cch);					// Start at cpMin

		while((LONG)tp.GetCp() < cpMost)
		{
			if(tp.GetChar() == CELL)				// Stop selection at CELL
			{
				Set(cpMin, cpMin - tp.GetCp());
				UpdateSelection();
				return;
			}
			tp.AdvanceCp(1);
		}
	}
}

/*
 *	CTxtSelection::SetCharFormat(pCF, fApplyToWord, publdr)
 *	
 *	@mfunc
 *		apply CCharFormat *pCF to this selection.  If range is an IP
 *		and fApplyToWord is TRUE, then apply CCharFormat to word surrounding
 *		this insertion point
 *
 *	@rdesc
 *		HRESULT = NOERROR if no error
 */
HRESULT CTxtSelection::SetCharFormat (
	const CCharFormat *pcf,	//@parm Ptr to CCharFormat to fill with results
	IUndoBuilder *publdr, 	//@parm the undo context
	DWORD flags)			//@parm If TRUE and this selection is an IP,
							//  use enclosing word
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetCharFormat");

	HRESULT hr = 0;
	LONG	iFormat = _iFormat;
	BOOL	fApplyToWord = (flags & SCF_WORD) ? CTxtRange::APPLYTOWORD : 0;

	if( publdr )
		publdr->StopGroupTyping();

	/*
	 * The code below applies character formatting to a double-clicked
	 * selection the way Word does it, that is, not applying the formatting to
	 * the last character in the selection if that character is a blank. 
	 *
	 * See also the corresponding code in CTxtRange::GetCharFormat().
	 */

	LONG		cch;
	LONG		cpMin, cpMost;
	CTxtRange   tr (GetPed());
	BOOL	    fCheckKeyboard = TRUE;

	cch = GetRange(cpMin, cpMost);
	
	if(_SelMode == smWord && (flags & SCF_USEUIRULES))
	{	
		// In word select mode,
		//  don't include final blank in SetCharFormat
		AssertSz(cch,
			"CTxtSelection::SetCharFormat: IP in word select mode");
		CTxtPtr tpLast(GetPed(), cpMost - 1);
		if(tpLast.GetChar() == ' ')			// Selection ends with a blank:
		{									
			cpMost--;						// Setup end point to end at last
			cch--;							//  char in selection
			fApplyToWord = FALSE;
			fCheckKeyboard = FALSE;
		}
	}

	BOOL fFontChange = 
		( (pcf->dwMask & (CFM_FACE | CFM_CHARSET)) == (CFM_FACE | CFM_CHARSET) );

	// Smart SB/DB Font Apply Feature
	if ( cch &&										// > 0 characters
		GetPed()->IsRich() &&						// In Rich Text Mode
		!GetPed()->_fSingleCodePage &&				// Not in single cp mode
		fFontChange &&								// font change
		pcf->bCharSet != THAI_CHARSET &&			// Not Thai
		!IsFECharset( pcf->bCharSet ))   			// Not FE
	{
		// Single byte 125x CharSet
		CFreezeDisplay fd(_pdp);					// Speed this up

		CTxtPtr     tp (GetPed(), cpMin);
		CCharFormat cf = *pcf;

		while (cpMin < cpMost)
		{
            // GuyBark Jupiter 36128:
            // Don't bypass any of the text here. The idea is that if the charset of
            // the font being set here is a 125* charset, but the character that it's
            // applied to lies in a different charset, then DON'T apply the font 
            // change to that character. We would then walk through all the subsequent
            // text making the same check. This is done for the following reason...
            //
            // Say the user has some Russian (or similar) text like unicode 0x0100,
            // and it's displayed in Arial (Baltic). The user does select all and 
            // changes the document to Courier New (Western). The western font has 
            // a different charset, and so the Russian text may not be displayed 
            // correctly. So this code is preventing the Russian text from changing
            // font and so the text is still displayed ok.
            //
            // But the problem is on the device, we don't have different charset 
            // variations of the fonts, (eg Arial (Baltic), Arial (Western) etc, 
            // we just have Arial). The regular font can display these extended 
            // characters ok. So if you entered the Russian character in TNR it's
            // displayed ok. But you then can't change the font to Arial, as the
            // Arial on the device is marked as ansi charset, and the character
            // we're trying to change lies in a different charset.
            //
            // So given that the following code is designed for a system with
            // the locale variations of the fonts, we should bypass it. This means
            // it's possible for a 125* character to be displayed incorrectly after
            // changing the font, but that's all the user can expect if they apply
            // a font to some text and the required glyphs are missing. Same thing
            // happens applying Symbol to the Russian text.
            //
            // All in all, this is a nice idea, but on the device will cause
            // more confusion that in will avoid.

#ifdef PWD_JUPITER
			BOOL  fFontFE = IsFECharset(pcf->bCharSet);
			BOOL  fInCharSet = (fFontFE || !IsFEChar(tp.GetChar()));

			while (fInCharSet == (fFontFE || !IsFEChar(tp.GetChar())) &&
				   (LONG) tp.GetCp() < cpMost)
#else
			BOOL fInCharSet = In125x( tp.GetChar(), pcf->bCharSet );
			while (fInCharSet == In125x( tp.GetChar(), pcf->bCharSet ) &&
				   (LONG) tp.GetCp() < cpMost)
#endif // PWD_JUPITER
			{
				tp.AdvanceCp(1);
			}

			cf.dwMask &= ~(CFM_FACE | CFM_CHARSET);
			if (fInCharSet)
				cf.dwMask |= (CFM_FACE | CFM_CHARSET);

			tr.SetRange(cpMin, tp.GetCp());
			HRESULT hr1 = cf.dwMask
						? tr.SetCharFormat(&cf, fApplyToWord | CTxtRange::IGNORESELAE, publdr)
						: 0;
			hr = FAILED(hr) ? hr : hr1;	
			cpMin = tp.GetCp();
		}
	}
	else if (_cch)
	{
		// It is OK to not update _iFormat for a non degenerate selection
		// even if UI rules and word select have reduced it.
		tr.SetRange( cpMin, cpMost );
		hr = tr.SetCharFormat(pcf, fApplyToWord, publdr);
	}
	else
	{
		// But for a degenerate selection, _iFormat must be updated
		hr = CTxtRange::SetCharFormat(pcf, fApplyToWord, publdr);
	}

	if ( fCheckKeyboard && _iFormat != iFormat )				// _iFormat changed
	{
		CheckChangeKeyboardLayout( TRUE );
	}

	UpdateCaret(TRUE);
	return hr;
}

/*
 *	CTxtSelection::SetParaFormat(pPF, publdr)
 *
 *	@mfunc
 *		apply CParaFormat *pPF to this selection.
 *
 *	#rdesc
 *		HRESULT = NOERROR if no error
 */
HRESULT CTxtSelection::SetParaFormat (
	const CParaFormat* pPF,	//@parm ptr to CParaFormat to apply
	IUndoBuilder *publdr)	//@parm the Undo context for this operation
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::SetParaFormat");

	if( publdr )
	{
		publdr->StopGroupTyping();
	}

	// Apply the format
	HRESULT hr = CTxtRange::SetParaFormat(pPF, publdr);

	UpdateCaret(TRUE);
	return hr;
}

/*
 *	CTxtSelection::SetSelectionInfo (pselchg)
 *
 *	@mfunc	Fills out data members in a SELCHANGE structure
 *
 *	@rdesc	void
 */
void CTxtSelection::SetSelectionInfo(
	SELCHANGE *pselchg)		//@parm the SELCHANGE structure to use
{
	LONG cpMin, cpMost;
	LONG cch;
	LONG cObjects;
	CObjectMgr *pobjmgr;

	cch = GetRange(cpMin, cpMost);

	pselchg->chrg.cpMin = cpMin;
	pselchg->chrg.cpMost = cpMost;

	// now fill out the selection type flags.
	// the flags mean the following things:
	//
	// SEL_EMPTY:	insertion point
	// SEL_TEXT:	at least one character selected
	// SEL_MULTICHAR:	more than one character selected
	// SEL_OBJECT:	at least one object selected
	// SEL_MULTIOJBECT:	more than one object selected
	//
	// note that the flags are OR'ed together.

	//This is the default
	pselchg->seltyp = SEL_EMPTY;

	if(cch)
	{
		cObjects = GetObjectCount();				// Total object count
		if(cObjects)								// There are objects:
		{											//  get count in range
			pobjmgr = GetPed()->GetObjectMgr();


            if (NULL == pobjmgr)
            {
			    Assert(pobjmgr);
			    return;
			}    
			

			cObjects = pobjmgr->CountObjectsInRange( cpMin, cpMost );

			if(cObjects > 0)
			{
				pselchg->seltyp |= SEL_OBJECT;
				if(cObjects > 1)
				{
					pselchg->seltyp |= SEL_MULTIOBJECT;
				}
			}
		}

		cch -= cObjects;

		AssertSz(cch >= 0, "objects are overruning the selection");

		if( cch > 0 )
		{
			pselchg->seltyp |= SEL_TEXT;
			if(cch > 1)
			{
				pselchg->seltyp |= SEL_MULTICHAR;
			}
		}
	}
}

/*
 *	CTxtSelection::UpdateForAutoWord ()
 *
 *	@mfunc	Update state to prepare for auto word selection
 *
 *	@rdesc	void
 */
void CTxtSelection::UpdateForAutoWord()
{
	AssertSz(!_cch,
		"CTxtSelection::UpdateForAutoWord: Selection isn't degenerate");

	// If enabled, prepare Auto Word Sel
	if(GetPed()->TxGetAutoWordSel())	
	{									
		CTxtPtr tp(_rpTX);

		// Move anchor to new location
		_cpAnchor = GetCp();

		// Remember that FindWordBreak moves tp's cp
		// (aren't side effects wonderful?
		tp.FindWordBreak(WB_MOVEWORDRIGHT);
		_cpAnchorMost =_cpWordMost = tp.GetCp();

		tp.FindWordBreak(WB_MOVEWORDLEFT);
		_cpAnchorMin = _cpWordMin = tp.GetCp();

		_fAutoSelectAborted = FALSE;
	}
}

/*
 *	CTxtSelection::AutoSelGoBackWord(pcpToUpdate, iDirToPrevWord, iDirToNextWord)
 *
 *	@mfunc	Backup a word in auto word selection
 *
 *	@rdesc	void
 */
void CTxtSelection::AutoSelGoBackWord(
	LONG *	pcpToUpdate,	//@parm end of word selection to update
	int		iDirToPrevWord,	//@parm direction to next word
	int		iDirToNextWord)	//@parm direction to previous word
{
	if (GetCp() >= _cpAnchorMin &&
		GetCp() <= _cpAnchorMost)
	{
		// We are back in the first word. Here we want to pop
		// back to a selection anchored by the original selection

		Set(GetCp(), GetCp() - _cpAnchor);
		_fAutoSelectAborted = FALSE;
		_cpWordMin  = _cpAnchorMin;
		_cpWordMost = _cpAnchorMost;
	}
	else
	{
		// pop back a word
		*pcpToUpdate = _cpWordPrev;

		CTxtPtr tp(_rpTX);

		_cpWordPrev = GetCp() + tp.FindWordBreak(iDirToPrevWord);
		FindWordBreak(iDirToNextWord);
	}
}

/*
 *	CTxtSelection::InitClickForAutWordSel (pt)
 *
 *	@mfunc	Init auto selection for click with shift key down
 *
 *	@rdesc	void
 */
void CTxtSelection::InitClickForAutWordSel(
	const POINT pt)		//@parm Point of click
{
	// If enabled, prepare Auto Word Sel
	if(GetPed()->TxGetAutoWordSel())	
	{
		// If auto word selection is occuring we want to pretend
		// that the click is really part of extending the selection.
		// Therefore, we want the auto word select data to look as
		// if the user had been extending the selection via the
		// mouse all along. So we set the word borders to the
		// word that would have been previously selected.

		// Need this for finding word breaks
		CRchTxtPtr	rtp(GetPed());
		LONG cpClick = _pdp->CpFromPoint(pt, NULL, &rtp, NULL, TRUE);
		int iDir = -1;

		if (cpClick < 0)
		{
			// If this fails what can we do? Prentend it didn't happen!
			// We can do this because it will only make the UI act a 
			// little funny and chances are the user won't even notice
			// this.
			return;
		}

		// Assume click is within anchor word
		_cpWordMost = _cpAnchorMost;
		_cpWordMin = _cpAnchorMin;

		if (cpClick > _cpAnchorMost)
		{
			// Click after the anchor word so set most and
			// prev appropriately.
			iDir = WB_MOVEWORDLEFT;
			rtp.FindWordBreak(WB_MOVEWORDLEFT);
			_cpWordMost = rtp.GetCp();
		}
		// Click is before the anchor word
		else if (cpClick < _cpAnchorMost)
		{
			// Click before the anchor word so set most and
			// prev appropriately.
			iDir = WB_MOVEWORDRIGHT;
			rtp.FindWordBreak(WB_MOVEWORDRIGHT);
			_cpWordMin = rtp.GetCp();
		}

		if (iDir != -1)
		{
			rtp.FindWordBreak(iDir);
			_cpWordPrev = rtp.GetCp();
		}
	}
}

/*
 *	CTxtSelection::CreateCaret ()
 *
 *	@mfunc	Create a caret
 *
 *	@rdesc	void
 */
void CTxtSelection::CreateCaret()
{
	CTxtEdit *ped = GetPed();

	// during IME composition, pickup the caret width from IME.
	// in order to display Korean block craet correctly.
	INT dx = dxCaret;

	ped->TxCreateCaret(0, dx, (INT)_yHeightCaret);
	ped->TxSetCaretPos((INT)_xCaret, (INT)_yCaret);
	_fCaretCreated = TRUE;
}

/*
 *	CTxtSelection::SetDelayedSelectionRange
 *
 *	@mfunc	sets the selection range such that it won't take effect until
 *			the control is "stable"
 */
void CTxtSelection::SetDelayedSelectionRange(
	LONG	cp,			//@parm the active end
	LONG	cch)		//@parm the signed extension
{
	CSelPhaseAdjuster *pspa;

	pspa = (CSelPhaseAdjuster *)GetPed()->GetCallMgr()->GetComponent(
						COMP_SELPHASEADJUSTER);

	Assert(pspa);

	pspa->CacheRange(cp, cch);
}

/*
 *	CTxtSelection::CheckPlainTextFinalEOP ()
 *
 *	@mfunc
 *		returns TRUE if this is a plain-text, multiline control with caret
 *		allowed at BOL and the selection at the end of the story following
 *		and EOP
 */
BOOL CTxtSelection::CheckPlainTextFinalEOP()
{
	CTxtEdit *ped = GetPed();

	return !ped->IsRich()							// Plain-text,
		&& ped->TxGetMultiLine()					//  multiline control,
		&& !_fCaretNotAtBOL							//  with caret OK at BOL,
		&& GetCp() == (LONG)ped->GetTextLength()	//  & cp at end of story
		&& _rpTX.IsAfterEOP();
}

/*
 *	CTxtSelection::StopGroupTyping()
 *
 *	@mfunc
 *		Tell undo manager to stop group typing
 */
void CTxtSelection::StopGroupTyping()
{
	IUndoMgr * pundo = GetPed()->GetUndoMgr();

	if(pundo)
		pundo->StopGroupTyping();
}

/*
 *	CTxtSelection::PrepareIMEOverstrike(fOver, publdr)
 *
 *	@mfunc
 *		Prepare for IME overtype by deleting the next character
 *
 *	@rdesc
 *		TRUE if successful
 */
void CTxtSelection::PrepareIMEOverstrike (
	IUndoBuilder *publdr)	//@parm If non-NULL, where to put anti-events
{
	TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtSelection::PrepareIMEOverstrike");

	_TEST_INVARIANT_
	
	// If nothing selected and not at EOP char, try
	// to select char at IP
	if( !_cch && !_rpTX.IsAtEOP() )  
	{											
		LONG iFormatSave = Get_iCF();			// Remember char's format

		if( publdr )
			publdr->StopGroupTyping();

		SetExtend(TRUE);						// Tell Advance() to select chars
		Advance(1);
		ReplaceRange( 0, NULL, publdr,
			SELRR_REMEMBERENDIP);				// Delete this character.
		ReleaseFormats(_iFormat, -1);

		_iFormat = iFormatSave;					// Restore char's format.
	}
}

// dual font support helpers
int CALLBACK FindWWEFont(
    const LOGFONT FAR *plf,
    const TEXTMETRIC FAR *ptm,
    ULONG nType,
    LPARAM lParam
    )
{
    // We're looking for a font which is ANSI codepage
    if((plf->lfCharSet == ANSI_CHARSET) &&
        (plf->lfPitchAndFamily & FF_SWISS))
    {
        // Bingo!  Return this one
        CopyMemory((LOGFONT *)lParam, plf, sizeof(LOGFONT));
        return 0;
    }

    return 1;
}

#define IFORMAT_IGNORE	-2

/*
 *	CTxtSelection::SetupDualFont
 *
 *	@mfunc	checks to see if dual font support is necessary; in this case,
 *			switching to an English font if English text is entered into
 *			an FE run
 *
 *	@rdesc	void
 *
 *	FUTURE (alexgo): this code can be integrated with CheckChangeFont.  Also,
 *	this is a temporary hack for 2.0, we need to build better support for
 *	this in the future.
 */
void CTxtSelection::SetupDualFont(
	TCHAR ch,				//@parm the UNICODE character to check for
	LONG *piFormatSave)		//@parm where to store the "old" format, if necessary
{
	
	// first, initialize the save to a safe default
	*piFormatSave = IFORMAT_IGNORE;

	// we only do this if the character looks to be English _and_ we are
	// on a FE system. 
	// only handle alpha cases
	if( !fHaveIMMProcs || !IsRich() || ch > 127 || !IsAlpha(ch) )
	{
		return;
	}

	SetDualFontMode(FALSE);	
	CTxtEdit	*ped = GetPed();
	LONG		iFormat = _iFormat;

// This piece of code has been moved to CheckSynchCharSet()
//
//	if (_cch)
//	{
//		// for selection, we need to get the character format at cpMin+1
//		CTxtRange rg( ped, GetCpMin()+1, 0 );
//		iFormat = rg.Get_iCF ();
//	}

	const CCharFormat *pcfCurrent = ped->GetCharFormat(iFormat);

	// the algorithm is straightforward:  if the character is less than 127
	// (an ASCII character) _and_ the current font is an FE font, then
	// we look for the closest English font and use that.

	if(IsFECharset(pcfCurrent->bCharSet))
	{
		// GuyBark JupiterJ 50980:
		// Say an invalid iFmtEnglish here is IFORMAT_IGNORE not -1.
		// An iFmtEnglish of -1 indicates that the text has the default
		// character format for the RichEdit window, not one explicitly
		// applied for this text run. Therefore -1 is not a format to 
		// be ignored. If the default font for the window is a Western
		// font, then we don't want to ignore it and revert to Arial.

		LONG iFmtEnglish = IFORMAT_IGNORE;
		LONG ifmttemp;
		LONG i = 0;
		CCharFormat cf;
		CFormatRunPtr	rp(_rpCF);
		ICharFormatCache *pcache = NULL;

		// first, initialize our temporary format
		pcfCurrent->Get(&cf);

		if( rp.IsValid() )
		{
			// look backwards, then forwards for an english font
			while( i < MAX_RUNTOSEARCH )
			{
				ifmttemp = rp.GetFormat();

				if( ped->GetCharFormat(ifmttemp)->bCharSet == ANSI_CHARSET )
				{
					iFmtEnglish = ifmttemp;
					break;
				}
				if( !rp.PrevRun() )
				{
					break;
				}
				i++;
			}

			if( iFmtEnglish == IFORMAT_IGNORE )
			{
				// didn't find anything going backwards, look forwards.
				// go back to the selection's current run and reset our
				// counter.
				rp.ChgRun(i);
				i = 0;

				while( i < MAX_RUNTOSEARCH )
				{
					ifmttemp = rp.GetFormat();
					if( ped->GetCharFormat(ifmttemp)->bCharSet == ANSI_CHARSET )
					{
						iFmtEnglish = ifmttemp;
						break;
					}
					if( !rp.NextRun() )
					{
						break;
					}
					i++;
				}
			}
		}

		// now, setup our new font with an English font name
		cf.bCharSet = ANSI_CHARSET;
		cf.bInternalEffects &= ~CFEI_FACENAMEISDBCS;

		if( iFmtEnglish != IFORMAT_IGNORE )
		{
			const CCharFormat *pcfEnglish = ped->GetCharFormat(iFmtEnglish);
			wcscpy_s(cf.szFaceName, pcfEnglish->szFaceName);

			// GuyBark JupiterJ: Don't forget the font family.
			cf.bPitchAndFamily = pcfEnglish->bPitchAndFamily;

			Assert(!(pcfEnglish->bInternalEffects & CFEI_FACENAMEISDBCS));
		}
		else
		{
            // Windows CE Platforms #30928
            // We need to ensure that the font actually exists on the system.
            // Therefore, we create a font with the properties we're looking
            // for (ANSI character set, Swiss if possible).
        	const CDevDesc *pdd = _pdp->GetDdRender();
            HDC hdc = pdd->GetDC();
            LOGFONT lf = {0};

            if(!EnumFontFamilies(hdc, NULL, FindWWEFont, (LPARAM)&lf))
            {
                // Found a font!
    			wcscpy_s(cf.szFaceName, lf.lfFaceName);
                cf.bPitchAndFamily = lf.lfPitchAndFamily;
            }
            else
            {
    			wcscpy_s(cf.szFaceName, TEXT("Tahoma"));
			    // GuyBark JupiterJ: Don't forget the font family.
			    cf.bPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
            }

            if(hdc)
            {
                pdd->ReleaseDC(hdc);
            }
		}

		if( FAILED(GetCharFormatCache(&pcache)) )
		{
			return;
		}

		// NB! ifmtEnglish is going to be reset to the new dual font value
		if( pcache->Cache(&cf, &iFmtEnglish) == NOERROR )
		{
			*piFormatSave = _iFormat;
			pcache->AddRefFormat(_iFormat);
			Set_iCF(iFmtEnglish);
			SetDualFontMode(TRUE);
		}
	}	
}

/*
 *	CTxtSelection::SetupJFont
 *
 *  GUYBARK: ADD THIS!
 *
 *  If the character being added is a FE character, and the current
 *  font's charset is not FE, then make a J font current.
 *
 *  Do the least amount of work here as possible. Do NOT get involved
 *  with "Dual Font Modes" and saving off the existing font for later.
 *
 */
void CTxtSelection::SetupJFont(LPTSTR psz)
{
    // Does this strng contain some FE text?
    if(!CheckDBCInUnicodeStr(psz))
    {
		LONG iFormatSave = -1;
		SetupDualFont(psz[0], &iFormatSave);

        return;
    }

    CTxtEdit *ped = GetPed();
    LONG      iFormat = _iFormat;
    int       iCharset = GetCharSet(GetKeyboardCodePage());
    const CCharFormat *pcfCurrent = ped->GetCharFormat(iFormat);

    // If the current font is FE, then assume we have nothing to do here.
    if(!IsFECharset(pcfCurrent->bCharSet))
    {
        LONG iFmtJ = -1;
        LONG ifmttemp;
        LONG i = 0;
        CCharFormat cf;
        CFormatRunPtr	rp(_rpCF);
        ICharFormatCache *pcache = NULL;

        // Ok, we have a FE character in a non-FE font. So hope that 
        // a J font will be sufficient.

        // first, initialize our temporary format
        pcfCurrent->Get(&cf);

        if( rp.IsValid() )
        {
            // look backwards, then forwards for a J font
            while( i < MAX_RUNTOSEARCH )
            {
                ifmttemp = rp.GetFormat();
                if( ped->GetCharFormat(ifmttemp)->bCharSet == SHIFTJIS_CHARSET )
                {
                    iFmtJ = ifmttemp;
                    break;
                }
                if( !rp.PrevRun() )
                {
                    break;
                }
                i++;
            }

            if( iFmtJ == -1 )
            {
                // didn't find anything going backwards, look forwards.
                // go back to the selection's current run and reset our
                // counter.
                rp.ChgRun(i);
                i = 0;

                while( i < MAX_RUNTOSEARCH )
                {
                    ifmttemp = rp.GetFormat();
                    if( ped->GetCharFormat(ifmttemp)->bCharSet == SHIFTJIS_CHARSET )
                    {
                        iFmtJ = ifmttemp;
                        break;
                    }
                    if( !rp.NextRun() )
                    {
                        break;
                    }
                    i++;
                }
            }
        }

        // now, setup our new font with a J font name

        cf.bCharSet = iCharset;
        cf.bInternalEffects &= ~CFEI_FACENAMEISDBCS;
		
        if( iFmtJ != -1 )
        {
            const CCharFormat *pcfJ = ped->GetCharFormat(iFmtJ);
            wcscpy_s(cf.szFaceName, pcfJ->szFaceName);
        }
        else
        {
            switch(iCharset)
            {
            case SHIFTJIS_CHARSET:
                wcscpy_s(cf.szFaceName,lfJapaneseFaceName );
                break;

            case JOHAB_CHARSET:
            case HANGEUL_CHARSET:
                wcscpy_s(cf.szFaceName, lfHangulFaceName);
                break;

            case GB2312_CHARSET:
                wcscpy_s(cf.szFaceName, lfGB2312FaceName);
                break;

            case CHINESEBIG5_CHARSET:
                wcscpy_s(cf.szFaceName, lfBig5FaceName);
                break;
            }

        }

        if( FAILED(GetCharFormatCache(&pcache)) )
        {
            return;
        }

        if( pcache->Cache(&cf, &iFmtJ) == NOERROR )
        {
            // All we need to do is set the font current now.
            Set_iCF(iFmtJ);
        }
    }	
}

// We no longer restoring the previous font replaced by the CTxtSelection::SetupDualFont
// I leave the following code in just in case we want to return to this "feature" again
// in the future.  -v-honwch

    // GuyBark JupiterJ 49758: No longer need this...
//#if 1
#if 0
/*
 *	CTxtSelection::RestoreDualFont
 *
 *	@mfunc	takes us out of dual font processing mode
 *
 *	@rdesc	void
 */
void CTxtSelection::RestoreDualFont(
	LONG iFormatSave)			//@parm old format to restore
{
	if( iFormatSave != IFORMAT_IGNORE )
	{		
		ICharFormatCache *pcache;
		
		SetDualFontMode(FALSE);
		if( FAILED(GetCharFormatCache(&pcache)) )
		{
			return;
		}

		Set_iCF(iFormatSave);

		// the AddRef is done in SetupDualFont
		pcache->ReleaseFormat(iFormatSave);
	}
}
#endif

//
//	CSelPhaseAdjuster methods
//

/* 
 *	CSelPhaseAdjuster::CSelPhaseAdjuster
 *
 *	@mfunc	constructor
 */
CSelPhaseAdjuster::CSelPhaseAdjuster(
	CTxtEdit *ped)		//@parm the edit context
{
	_cp = _cch = -1;
	_ped = ped;	

	_ped->GetCallMgr()->RegisterComponent((IReEntrantComponent *)this, 
							COMP_SELPHASEADJUSTER);
}

/* 
 *	CSelPhaseAdjuster::~CSelPhaseAdjuster
 *
 *	@mfunc	destructor
 */
CSelPhaseAdjuster::~CSelPhaseAdjuster()
{
	// Save some indirections
	CTxtEdit *ped = _ped;

	if( _cp != -1 )
	{
		ped->GetSel()->SetSelection(_cp - _cch, _cp);

		// If the selection is updated, then we invalidate the
		// entire display because the old selection can still
		// appear othewise because the part of the screen that
		// it was on is not updated.
		if (ped->fInplaceActive())
		{
			// Tell entire client rectangle to update.
			// FUTURE: The smaller we make this the better.
			ped->TxInvalidateRect(NULL, FALSE);
		}
	}

	ped->GetCallMgr()->RevokeComponent((IReEntrantComponent *)this);
}

/* 
 *	CSelPhaseAdjuster::CacheRange
 *
 *	@mfunc	tells this class the selection range to remember
 */
void CSelPhaseAdjuster::CacheRange(
	LONG	cp,			//@parm the active end to remember
	LONG	cch)		//@parm the signed extension to remember
{
	_cp		= cp;
	_cch	= cch;
}

