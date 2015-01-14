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
 *	@module	dispml.cpp -- CDisplayML class |
 *
 *		This is the Multi-line display engine.  See disp.c for the base class
 *		methods and dispsl.c for the single-line display engine.
 *	
 *	Owner:<nl>
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini (initial conversion to C++)
 *		Murray Sargent
 *		Rick Sailor (for most of RE 2.0)
 */

#include "_common.h"
#include "_dispml.h"
#include "_edit.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_select.h"
#include "_dfreeze.h"

ASSERTDATA

//
//	Invariant support
//
#define DEBUG_CLASSNAME	CDisplayML
#include "_invar.h"

// Timer tick counts for background task
#define cmsecBgndInterval 	300
#define cmsecBgndBusy 		100

// Lines ahead
const LONG cExtraBeforeLazy = 60;

// if we need to calc at least this many characters, then put up a wait
// cursor.  NB!  4096 is not a measured number; it just seemed like a good
// one.
#define NUMCHARFORWAITCURSOR	4096	

// define only if we want to disable the horizontal scroll bar
#define DISABLE_HORZ

#ifndef DEBUG
#define CheckView()
#define	CheckLineArray()
#endif
	
// V-GUYB: Add a local MulDiv to prevent wrapping during scroll calculations in long docs.
#define MulDivI64(nNumber, nNumerator, nDenominator) (int)((nDenominator) ? (((__int64)(nNumber) * (__int64)(nNumerator)) / (__int64)(nDenominator)) : -1)

// ===========================  CDisplayML  =====================================================

#ifdef DEBUG
/*
 *	CDisplayML::Invariant
 *
 *	@mfunc	Make sure the display is in a valid state
 *
 *	@rdesc	TRUE if the tests succeeded, FALSE otherwise
 */
BOOL CDisplayML::Invariant(void) const
{
	CDisplay::Invariant();

	if( _prgliNew )
	{
		Assert(_prgliNew->Invariant());
	}

	return TRUE;
}
#endif // DEBUG

/*
 *	CDisplayML::CalcScrollHeight()
 *
 *	@mfunc	
 *		Calculate the maximum Y scroll position.
 *
 *	@rdesc
 *		Maximum possible scrolling position
 *
 *	@devnote
 *		This routine exists because plain text controls do not have
 *		the auto-EOP and so the scroll height is different than
 *		the height of the control if the text ends in an EOP type
 *		character.
 *
 */
LONG CDisplayML::CalcScrollHeight(LONG yHeight) const
{
	// The max scroll height for plain text controls is calculated differently
	// because they don't have an automatic EOP character.
	if (!_ped->IsRich() && (Count() > 0))
	{
		// Get the last line in the array
		CLine *lp = Elem(Count() - 1);

		// Is the last character an EOP? - if so we need to bump the scroll
		// height.
		if (lp->_cchEOP)
			yHeight += lp->GetHeight();
	}
	return yHeight;
}

/*
 *	CDisplayML::GetMaxYScroll()
 *
 *	@mfunc	
 *		Calculate the maximum Y scroll position.
 *
 *	@rdesc
 *		Maximum possible scrolling position
 *
 *	@devnote
 *		This routine exists because we may have to come back and modify this 
 *		calculation for 1.0 compatibility. If we do, this routine only needs
 *		to be changed in one place rather than the three at which it is used.
 *
 */
INLINE LONG CDisplayML::GetMaxYScroll() const
{
	// The following code is turn off because we don't want to support 
	// 1.0 mode unless someone complained about it.  
#if 0		
 	if (_ped->Get10Mode())
	{
		// Ensure last line is always visible
		// (use dy as temp to calculate max scroll)
		yScroll = Elem(max(0, Count() - 1))->_yHeight;

		if(yScroll > _yHeightView)
			yScroll = _yHeightView;

		yScroll = _yHeight - yScroll;
	}
#endif //0

	return CalcScrollHeight(_yHeight);
}

/*
 *	CDisplayML::ConvertYPosToMax()
 *
 *	@mfunc	
 *		Calculate the real scroll position from the scroll position
 *
 *	@rdesc
 *		Y position from scroll
 *
 *	@devnote
 *		This routine exists because the thumb position messages
 *		are limited to 16-bits so we extrapolate when the Y position
 *		gets greater than that.
 *
 */
LONG CDisplayML::ConvertScrollToYPos(
	LONG yPos)		//@parm Scroll position 
{
	// Get the maximum scroll range
	LONG yRange = GetMaxYScroll();

	// Has the maximum scroll range exceeded 16-bits?
	if (yRange >= _UI16_MAX)
	{
		// Yes - Extrapolate to the "real" yPos		

#ifndef UNDER_CE
		yPos = MulDiv(yPos, yRange, _UI16_MAX);
#else
        // V-GUYB: 4 byte values can wrap in MulDiv, so use int64 here.
		yPos = MulDivI64(yPos, yRange, _UI16_MAX);
#endif
	}

	return yPos;
}

/*
 *	CDisplayML::ConvertYPosToScrollPos()
 *
 *	@mfunc	
 *		Calculate the scroll position from the Y position in the document.
 *
 *	@rdesc
 *		Scroll position from Y position
 *
 *	@devnote
 *		This routine exists because the thumb position messages
 *		are limited to 16-bits so we extrapolate when the Y position
 *		gets greater than that.
 *
 */
INLINE LONG CDisplayML::ConvertYPosToScrollPos(
	LONG yPos)		//@parm Y position in document
{
	// Get the maximum scroll range
	LONG yRange = GetMaxYScroll();

	// Has the maximum scroll range exceeded 16-bits?
	if (yRange >= _UI16_MAX)
	{
		// Yes - Extrapolate to the "real" yPos		

#ifndef UNDER_CE
		yPos = MulDiv(yPos, _UI16_MAX, yRange);
#else
        // V-GUYB: 4 byte values can wrap in MulDiv, so use int64 here if necessary.
        if(yPos < _UI16_MAX)
        {
            // yPos * _UI16_MAX won't wrap, so use 32 bit MulDiv.
            yPos = MulDiv(yPos, _UI16_MAX, yRange);
        }
        else
        {
            // yPos * _UI16_MAX will wrap, so use 64 bit MulDiv.
            yPos = MulDivI64(yPos, _UI16_MAX, yRange);
        }
#endif

	}
	return yPos;
}


CDisplayML::CDisplayML (CTxtEdit* ped)
  : CDisplay (ped), _pddTarget(NULL)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CDisplayML");

	_xWidthMax = 0;
	_yHeightMax = 0;
}

CDisplayML::~CDisplayML()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::~CDisplayML");

	delete _prgliNew;

	delete _pddTarget;
}

/*
 *	CDisplayML::Init()
 *
 *	@mfunc	
 *		Init this display for the screen
 */
BOOL CDisplayML::Init()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Init");

	// Initialize our base class
	if (CDisplay::Init())
	{
		return FALSE;
	}

	DWORD dwScrollBars = _ped->TxGetScrollBars();
	int fnBarHorz = SB_HORZ;
	int fnBarVert = SB_VERT;

	AssertSz(_ped, "CDisplayML::Init(): _ped not initialized in display");
	Assert (_yCalcMax == 0);		// Verify allocation zeroed memory out
	Assert (_xWidth == 0);
	Assert (_yHeight == 0);
	Assert (_cpMin == 0);
	Assert (_fBgndRecalc == FALSE);
	Assert(!_fVScrollEnabled);
	Assert(!_fHScrollEnabled);

	// The printer view is not main, therefore we do this to make
	// sure scroll bars are not created for print views.
	if (IsMain())
	{
		if(dwScrollBars & WS_VSCROLL)
		{
			if(dwScrollBars & ES_DISABLENOSCROLL)
			{
				// This causes wlm to assert on the mac. something about the 
				// scrollbar being disabled
				_ped->TxSetScrollRange(fnBarVert, 0, 1, TRUE);

				_ped->TxEnableScrollBar(fnBarVert, ESB_DISABLE_BOTH);
			}
		}

		// Set horizontal scroll range and pos
		// ??? - CF need fixing for windowless case
		if(dwScrollBars & WS_HSCROLL)
		{
			if(dwScrollBars & ES_DISABLENOSCROLL)
			{
				_ped->TxSetScrollRange (fnBarHorz, 0, 1, TRUE);
				_ped->TxEnableScrollBar(fnBarHorz, ESB_DISABLE_BOTH);
			}
		}
	}

	SetWordWrap(_ped->TxGetWordWrap());

	_cpFirstVisible = _cpMin;
	
	Assert(_xScroll == 0);
	Assert(_yScroll == 0);
	Assert(_iliFirstVisible == 0);
	Assert(_cpFirstVisible == 0);
	Assert(_dyFirstVisible == 0);

    _TEST_INVARIANT_

	return TRUE;
}


//================================  Device drivers  ===================================
/*
 *	CDisplayML::SetMainTargetDC(hdc, xWidthMax)
 *
 *	@mfunc
 *		Sets a target device for this display and updates view 
 *
 *	@devnote
 *		Target device can't be a metafile (can get char width out of a 
 *		metafile)
 *
 *	@rdesc
 *		TRUE if success
 */
BOOL CDisplayML::SetMainTargetDC (
	HDC hdc,			//@parm Target DC, NULL for same as rendering device
	LONG xWidthMax)		//@parm Max line width (not used for screen)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::SetMainTargetDC");

	BOOL	result = FALSE;

	if ( SetTargetDC( hdc ) )
	{
		// This is here because this is what RE 1.0 did. 
		SetWordWrap(!(!hdc && (xWidthMax != 0)));

		// if xWidthMax is greater than zero, then the caller is
		// trying to set the maximum width of the window (for measuring,
		// line breaking, etc.)  However,in order to make our measuring
		// algorithms more reasonable, we'll force the max size to
		// be *at least* as wide as the width of a character.
		// Note that xWidthMax = 0 means use the view rect width
		_xWidthMax = (xWidthMax <= 0) ? 0 : max(DXtoLX(GetXWidthSys()), 
												xWidthMax);

		// need to do a full recalc
		// if it fails, it fails, the lines are left in a reasonable state
		// no need to call WaitForRecalc() because UpdateView()
		// start at position zero and we're always calced up to there
		CDisplay::UpdateView();

		// caret/selection has most likely moved
		CTxtSelection *psel = _ped->GetSelNC();

		if ( psel ) 
		{
			psel->UpdateCaret(FALSE);
		}
		result = TRUE;
	}
	return result;
}

// Useful for both main and printing devices. jonmat 6/08/1995
BOOL CDisplayML::SetTargetDC( HDC hdc )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::SetTargetDC");

	CDevDesc *pddTarget = NULL;

	// don't allow metafiles to be set as the target device
	if(hdc && GetDeviceCaps(hdc, TECHNOLOGY) == DT_METAFILE)
	{
		return FALSE;
	}

	if (hdc != NULL)
	{
		// Allocate the device first to see if we can. We 
		// don't want to change our state if this is going
		// to fail.
		pddTarget = new CDevDesc(_ped);

		if (NULL == pddTarget)
		{
			// We couldn't so we are done.
			return FALSE;
		}
	}

	// remove any cached information for the old target device
	if (_pddTarget != NULL)
	{
		delete _pddTarget;
		_pddTarget = NULL;
	}

	if(hdc != NULL)
	{
		// Update the device because we have one.
		_pddTarget = pddTarget;

		_pddTarget->SetDC(hdc);
	}
	return TRUE;
}

//=================================  Line recalc  ==============================
/*
 *	CDisplayML::RecalcScrollBars(void)
 *
 *	@mfunc
 *		Recalculate the scroll bars if the view has changed.
 *
 *
 *	@devnote	There is a possibility of recursion here, so we
 *				need to protect ourselves.
 *
 *	To visualize this, consider two types of characters, 'a' characters 
 *	which are small in height and 'A' 's which are really tall, but the same 
 *	width as an 'a'. So if I have
 *
 *	a a A						<nl>
 *	A							<nl>
 *
 *	I'll get a calced size that's basically 2 * heightof(A).
 *	With a scrollbar, this could wordwrap to 
 *
 *	a a							<nl>
 *	A A							<nl>
 *
 *	which is of calced size heightof(A) + heightof(a); this is
 *	obviously less than the height in the first case.
 */
void CDisplayML::RecalcScrollBars()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcScrollBars");

	if(_fViewChanged)
	{
  		_fViewChanged = FALSE;

		UpdateScrollBar(SB_VERT, TRUE);
    	UpdateScrollBar(SB_HORZ, TRUE);
    }
}

/*
 *	CDisplayML::RecalcLines(fWait)
 *
 *	@mfunc
 *		Recalc all line breaks. 
 *		This method does a lazy calc after the last visible line
 *		except for a bottomless control
 *
 *	@rdesc
 *		TRUE if success
 */
BOOL CDisplayML::RecalcLines (
	BOOL fWait)		//@parm Recalc lines down to _cpWait/_yWait; then be lazy
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcLines");

	LONG		cliWait = cExtraBeforeLazy;			// Extra lines before being lazy
	BOOL		fDone = TRUE;
	BOOL		fFirstInPara = TRUE;
	CLine *		pliNew = NULL;
	LONG		xWidth;
	LONG		yHeight = 0;
    LONG        cchText = _ped->GetTextLength();
	BOOL		fWaitingForFirstVisible = TRUE;
	LONG		yHeightView = _yHeightView;
	LONG		yHeightScrollOld = GetMaxYScroll();
	LONG		yHeightScrollNew;

	Remove(0, -1, AF_KEEPMEM);					// Remove all old lines from *this
	_yCalcMax = 0;							// Set both maxes to start of text
	_cpCalcMax = 0;

	// Don't stop at the bottom of the view if we're bottomless and we're active.
	if (!_ped->TxGetAutoSize() && IsActive())
	{
		// be lazy - don't bother going past visible portion
		_cpWait = -1;
		_yWait = -1;
		fWait = TRUE;
	}

	// Init measurer at cp = 0
	CMeasurer me(this);
 	
	// The following loop generates new lines
	while( (LONG)me.GetCp() < cchText )
	{
		// Add one new line
		pliNew = Add(1, NULL);
		if (!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			TRACEWARNSZ("Out of memory Recalc'ing lines");
			goto err;
		}

		// stuff text into new line
		UINT uiFlags = MEASURE_BREAKATWORD | (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		if(!pliNew->Measure(me, -1, uiFlags))
		{
			Assert(FALSE);
			goto err;
		}

		fFirstInPara = pliNew->_cchEOP;
		yHeight += pliNew->GetHeight();
		_cpCalcMax = me.GetCp();

		if (fWait)
		{
			// Do we want to do a background recalc? - the answer is yes if
			// three things are true: (1) We have recalc'd beyond the old first
			// visible character, (2) We have recalc'd beyond the visible 
			// portion of the screen and (3) we have gone beyond the next
			// cExtraBeforeLazy lines to make page down go faster.

			if (fWaitingForFirstVisible)
			{
				if ((LONG) me.GetCp() > _cpFirstVisible)
				{
					_yWait = yHeight + yHeightView;
					fWaitingForFirstVisible = FALSE;
				}
			}
			else if ((yHeight > _yWait) && cliWait-- <= 0)
			{
				fDone = FALSE;
				break;
			}
		}
	}

	_yCalcMax = yHeight;
	_fRecalcDone = fDone;
    _fNeedRecalc = FALSE;
	yHeightScrollNew = CalcScrollHeight(yHeight);

	if((fDone 
		&& ((yHeight != _yHeight) || (yHeightScrollNew != yHeightScrollOld)))
		|| (yHeightScrollNew > yHeightScrollOld))
	{
		_yHeight = yHeight;
		_fViewChanged = TRUE;
	}

	xWidth = CalcDisplayWidth();
    if((fDone && (xWidth != _xWidth)) || (xWidth > _xWidth))
    {
        _xWidth = xWidth;
		_fViewChanged = TRUE;
    }    

	Tracef(TRCSEVINFO, "CDisplayML::RecalcLine() - Done. Recalced down to line #%d", Count());

	if(!fDone)						// if not done, do rest in background
	{
		fDone = StartBackgroundRecalc();
	}

	if (fDone)
	{
		_yWait = -1;
		_cpWait = -1;
		CheckLineArray();
		_fLineRecalcErr = FALSE;
	}

#ifdef DEBUG
	if( 1 )
    {
		_TEST_INVARIANT_
	}
#endif // DEBUG

	return TRUE;

err:
	TRACEERRORSZ("CDisplayML::RecalcLines() failed");

	if(!_fLineRecalcErr)
	{
		_cpCalcMax = me.GetCp();
		_yCalcMax = yHeight;
		_fLineRecalcErr = TRUE;
		_ped->GetCallMgr()->SetOutOfMemory();
		_fLineRecalcErr = FALSE;			//  fix up CArray & bail
	}

	return FALSE;
}

/*
 *	CDisplayML::RecalcLines(tpFirst, cchOld, cchNew, fBackground, fWait, pled)
 *
 *	@mfunc
 *		Recompute line breaks after text modification
 *
 *	@rdesc
 *		TRUE if success
 */						     
BOOL CDisplayML::RecalcLines (
	const CRchTxtPtr &tpFirst,	//@parm Where change happened
	LONG cchOld,				//@parm Count of chars deleted
	LONG cchNew,				//@parm Count of chars added
	BOOL fBackground,			//@parm This method called as background process
	BOOL fWait,					//@parm Recalc lines down to _cpWait/_yWait; then be lazy
	CLed *pled)					//@parm Records & returns impact of edit on lines (can be NULL)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcLines");
			  		  
	LONG		cchEdit;
	LONG		cchSkip;
	LONG		cli = 0;
	LONG		cliBackedUp = 0;
	LONG		cliWait = cExtraBeforeLazy;	
	BOOL		fDone = TRUE;
	BOOL		fFirstInPara = TRUE;
	LONG		ili;
	CLed		led;
	LONG		lT;							// long Temporary
	CLine *		pliNew;
	CLine *		pli = NULL;
	CLinePtr	rpOld(this);
	LONG		xWidth;
	LONG		yHeight;
	LONG		yHeightPrev = 0;
    LONG        cchText = _ped->GetTextLength();
    UINT        uiFlags;
	BOOL 		fReplaceResult;
	BOOL		fTryForMatch = TRUE;
	LONG		yHeightScrollOld = GetMaxYScroll();
	LONG		yHeightScrollNew;
	WORD		wNumber = 0;

	if (!pled)
		pled = &led;

#ifdef DEBUG
	LONG		cp;

	if(tpFirst.GetCp() > _cpCalcMax)
		Tracef(TRCSEVERR, "tpFirst %ld, _cpCalcMax %ld", tpFirst.GetCp(), _cpCalcMax);

	AssertSz(tpFirst.GetCp() <= _cpCalcMax, 
		"CDisplayML::RecalcLines Caller didn't setup RecalcLines()");

	AssertSz(!(fWait && fBackground),
		"CDisplayML::RecalcLines wait and background both true");

	AssertSz(!(fWait && (-1 == _cpWait) && (-1 == _yWait)),
		"CDisplayML::RecalcLines background recalc parms invalid");
#endif

	// We will not use background recalc if this is already a background recalc,
	// or if the control is not active or if this is an auto sized control.
	if(!IsActive() || _ped->TxGetAutoSize())
	{
		fWait = FALSE;
	}

	// Init line pointer on old CLineArray and backup to start of line
	rpOld.RpSetCp(tpFirst.GetCp(), FALSE);
	cchSkip = rpOld.RpGetIch();
	rpOld.RpAdvanceCp(-cchSkip);			// Point rp at 1st char in line

	ili = rpOld;							// Save line # at change for
	if(ili && (IsInOutlineView() ||			//  numbering. Back up if not
		tpFirst.GetPF()->IsListNumbered()))	//  first number in list or if
	{										//  in OutlineView (Outline
		ili--;								//  symbol may change)
	}

	// Back up at least one line in case we can now fit more on it
	// If on a line border, e.g., just inserted an EOP, backup 2; else 1
	lT = !cchSkip + 1;

	while(lT-- > 0 && rpOld > 0 && (!rpOld[-1]._cchEOP || ili < rpOld))
	{
		cliBackedUp++;
		rpOld--;
		cchSkip += rpOld->_cch;
	}

	// Init measurer at tpFirst
	CMeasurer me(this, tpFirst);

	me.Advance(-cchSkip);					// Point at start of text to measure
	cchEdit = cchNew + cchSkip;				// Number of chars affected by edit
	me.SetNumber(rpOld.GetNumber());		// Initialize list number
	
	// Determine whether we're on first line of paragraph
	if(rpOld > 0)
	{
		fFirstInPara = rpOld[-1]._cchEOP;
		AssertSz(me.GetCp() >= _cpCalcMax ||
				 !(rpOld->_bFlags & fliFirstInPara) == !fFirstInPara, 
				 "fHasEOL & fFirstInPara disagree");
	}

	yHeight = YposFromLine(rpOld);

	// Update first-affected and pre-edit-match lines in pled
	pled->_iliFirst = rpOld;
	pled->_cpFirst	= pled->_cpMatchOld	= me.GetCp();
	pled->_yFirst	= pled->_yMatchOld	= yHeight;
	AssertSz(pled->_yFirst >= 0, "CDisplayML::RecalcLines _yFirst < 0");
	
	Tracef(TRCSEVINFO, "Start recalcing from line #%d, cp=%d", pled->_iliFirst, pled->_cpFirst);

	// If we are past the requested area to recalc and background recalc is
	// allowed, then just go directly to background recalc. If there is no
	// height, we just go a head and calculate some lines anyway. This
	// prevents any weird background recalcs from occuring when it is
	// unnecessary to go into background recalc.
	if(fWait && _yWait > 0 && yHeight > _yWait && (LONG)me.GetCp() > _cpWait)
	{
		// Reset the recalc max to where we would have started the recalc
		_cpCalcMax = (LONG) me.GetCp();

		// Remove all old lines from here on
		rpOld.Remove(-1, AF_KEEPMEM);

		// Start up the background recalc
		StartBackgroundRecalc();		
		pled->SetMax(this);
		return TRUE;
	}

	// In case of error, set both maxes to where we are now
	_yCalcMax = yHeight;
	_cpCalcMax = me.GetCp();

	// Create or reuse new CArray of lines
	if (!_prgliNew && !(_prgliNew = new CLineArray()))
	{
		TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc CLineArray");
		goto errspace;
	}
	_prgliNew->Clear(AF_KEEPMEM);
    pliNew = NULL;

	// The following loop generates new lines for each line we backed
	// up over and for lines directly affected by edit
	while(cchEdit > 0)
	{
		pliNew = _prgliNew->Add(1, NULL);		// Add one new line
		if (!pliNew)
		{
			TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLine in CLineArray");
			goto errspace;
		}

		// Can't reuse old results if we've got a target device
		// For SPEED: it'd be nice to cache a few values when we do have a
		// target device - a good caching heuristic could halve the measuring
		const LONG cchNonWhite = rpOld.IsValid() ? 
			(rpOld->_cch - rpOld->_cchWhite ) : 0;

		if(cchSkip > 0 && cchSkip >= cchNonWhite && !IsInOutlineView() && 
			(NULL == _pddTarget || !_pddTarget->IsValid()))
		{
			me.NewLine(*rpOld);			// Don't remeasure anything we
			me.Advance(cchNonWhite);	//  already have valid info on
			me._cch = cchNonWhite;
			me._xWidth = rpOld->_xWidth;

			// clear out any of the old flags _except_ for tabs and OLE or
			// of screen. Note that this algorithm is somewhat bogus; there 
			// is no guarantee that the line still matches the flag state.  
			// However,those flags are simply 'hints'--i.e. the line _may_ 
			// be in that state.  Keeping those flags set will result
			// in a minor slowdown for rendering the line.
			me._bFlags &= (fliHasTabs | fliHasOle | fliUseOffScreenDC);

			if (rpOld->_bFlags & fliOffScreenOnce)
				me._bFlags &= ~fliUseOffScreenDC;
			me._cchEOP = 0;
		}
		else
			me.NewLine(fFirstInPara);			// Remeasure from start of line

		// Stuff text into new line
		uiFlags = MEASURE_BREAKATWORD | (fFirstInPara ? MEASURE_FIRSTINPARA : 0);

    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		if (!me.MeasureLine(-1, -1, uiFlags))
		{
			Assert(FALSE);
			goto err;
		}

		*pliNew = me;
      
		if( pliNew->_cch == 0 )
		{
			TRACEWARNSZ(
           "CDisplayML::RecalcLines measure returned a zero length line");
			goto errspace;
		}

		// Calculate on what line the edit started. We do this because
		// we want to render the first edited line off screen so if
		// the line is being edited via the key board we don't clip
		// any characters.
		if(cchSkip > 0)
		{
			// Check whether we backed up and the line we are examining
			// changed at all. Even if it didn't change in outline view
			// have to redraw in case outline symbol changes
			if(cliBackedUp != 0 && cchSkip >= (LONG)pliNew->_cch 
				&& pliNew->IsEqual(*rpOld) && !IsInOutlineView())
			{
				// perfect match, this line was not the first edited.
               	Tracef(TRCSEVINFO, "New line matched old line #%d", (LONG)rpOld);

				cchSkip -= rpOld->_cch;

				// Update first affected line and match in pled
				pled->_iliFirst++;
				pled->_cpFirst	  += rpOld->_cch;
				pled->_cpMatchOld += rpOld->_cch;
				pled->_yFirst	  += rpOld->GetHeight();
				AssertSz(pled->_yFirst >= 0, "CDisplayML::RecalcLines _yFirst < 0");
				pled->_yMatchOld  += rpOld->GetHeight();
				cliBackedUp--;
			
				_prgliNew->Clear(AF_KEEPMEM);	// Discard new line
				if (!(rpOld++))					// Next line
					cchSkip = 0;
			}
			else								// No match in the line, so 
				cchSkip = 0;					//  this line is the first to
		}										//  be edited

		Assert( !!(pliNew->_bFlags & fliFirstInPara) == !!fFirstInPara);
		fFirstInPara = pliNew->_cchEOP;
		yHeightPrev	 = yHeight;
		yHeight		+= pliNew->GetHeight();
		cchEdit		-= pliNew->_cch;

		if(fBackground && (GetTickCount() >= _dwBgndTickMax))
		{
			fDone = FALSE;						// took too long, stop for now
			goto no_match;
		}

		if(fWait && yHeight > _yWait && (LONG) me.GetCp() > _cpWait
			&& cliWait-- <= 0)
		{
			// Not really done, just past region we're waiting for
			// so let background recalc take it from here
			fDone = FALSE;
			goto no_match;
		}
	}											// while(cchEdit > 0) { }

   	Tracef(TRCSEVINFO, "Done recalcing edited text. Created %d new lines", _prgliNew->Count());

	// Edit lines have been exhausted.  Continue breaking lines,
	// but try to match new & old breaks

	wNumber = me._wNumber;
	while( (LONG) me.GetCp() < cchText )
	{
		// Assume there are no matches to try for
		BOOL	frpOldValid = FALSE;

		// If we run out of runs, then no match is possible. Therefore, 
		// we only try for a match as long as we have runs.
		if (fTryForMatch)
		{
			// We are trying for a match so assume that there
			// is a match after all
			frpOldValid = TRUE;

			// Look for match in old line break CArray
			lT = me.GetCp() - cchNew + cchOld;
			while (rpOld.IsValid() && pled->_cpMatchOld < lT)
			{
				pled->_yMatchOld  += rpOld->GetHeight();
				pled->_cpMatchOld += rpOld->_cch;

				if( !rpOld.NextRun() )
				{
					// No more line array entries so we can give up on
					// trying to match for good.
					fTryForMatch = FALSE;
					frpOldValid = FALSE;
					break;
				}
			} 
		}

		// If perfect match, stop
		if (frpOldValid && rpOld.IsValid() && pled->_cpMatchOld == lT && 
			rpOld->_cch && (!wNumber || me._wNumber < wNumber))
		{
           	Tracef(TRCSEVINFO, "Found match with old line #%d", rpOld.GetLineIndex());

			// Update fliFirstInPara flag in 1st old line that matches.  Note
			// that if the new array doesn't have any lines, we have to look
			// into the line array preceding the current change.
			rpOld->_bFlags |= fliFirstInPara;
			if( _prgliNew->Count() > 0 ) 
			{
				if(!(_prgliNew->Elem(_prgliNew->Count() - 1)->_cchEOP))
					rpOld->_bFlags &= ~fliFirstInPara;
			}
			else if( rpOld > pled->_iliFirst && pled->_iliFirst )
			{
				if(!rpOld[pled->_iliFirst - rpOld -1]._cchEOP )
					rpOld->_bFlags &= ~fliFirstInPara;
			}

			pled->_iliMatchOld = rpOld;

			// Replace old lines by new ones
			lT = rpOld - pled->_iliFirst;
			rpOld = pled->_iliFirst;
			if(!rpOld.Replace (lT, _prgliNew))
			{
				TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLines in rpOld");
				goto errspace;
			}
			frpOldValid = rpOld.ChgRun(_prgliNew->Count());
			_prgliNew->Clear(AF_KEEPMEM);	 		// Clear aux array

			// Remember information about match after editing
			Assert((cp = rpOld.GetCp()) == (LONG) me.GetCp());
			pled->_yMatchNew	= yHeight;
			pled->_yMatchNewTop = yHeightPrev;
			pled->_iliMatchNew	= rpOld;
			pled->_cpMatchNew	= me.GetCp();

			// Compute height and cp after all matches
			_cpCalcMax = me.GetCp();

			if( frpOldValid && rpOld.IsValid() )
			{
				do
				{
					yHeight	   += rpOld->GetHeight();
					_cpCalcMax += rpOld->_cch;
				}
				while( rpOld.NextRun() );
			}

			// Make sure _cpCalcMax is sane after the above update
			AssertSz(_cpCalcMax <= cchText, 
				"CDisplayML::RecalcLines match extends beyond EOF");

			// We stop calculating here.Note that if _cpCalcMax < size 
			// of text, this means a background recalc is in progress.
			// We will let that background recalc get the arrays
			// fully in sync.  

			AssertSz(((_cpCalcMax == cchText) || _fBgndRecalc),
					"CDisplayML::Match less but no background recalc");

			if (_cpCalcMax != cchText)
			{
				// This is going to be finished by the background recalc
				// so set the done flag appropriately.
				fDone = FALSE;
			}
			goto match;
		}

		// Add a new line
		pliNew = _prgliNew->Add(1, NULL);
		if (!pliNew)
		{
			TRACEWARNSZ("CDisplayML::RecalcLines unable to alloc additional CLine in CLineArray");
			goto errspace;
		}

    	Tracef(TRCSEVINFO, "Measuring new line from cp = %d", me.GetCp());

		// Stuff some text into new line
		wNumber = me._wNumber;
		if (!pliNew->Measure(me, -1, MEASURE_BREAKATWORD | 
							(fFirstInPara ? MEASURE_FIRSTINPARA : 0)))
		{
			Assert(FALSE);
			goto err;
		}
		
		fFirstInPara = pliNew->_cchEOP;
		yHeight += pliNew->GetHeight();

		if(fBackground && GetTickCount() >= (DWORD)_dwBgndTickMax)
		{
			fDone = FALSE;			// Took too long, stop for now
			break;
		}

		if(fWait && yHeight > _yWait && (LONG) me.GetCp() > _cpWait
			&& cliWait-- <= 0)
		{							// Not really done, just past region we're
			fDone = FALSE;			//  waiting for so let background recalc
			break;					//  take it from here
		}
	}								// while(me < cchText) ...

no_match:
	// Didn't find match: whole line array from _iliFirst needs to be changed
	pled->_iliMatchOld	= Count(); 
	pled->_cpMatchOld	= cchText;
	pled->_yMatchNew	= yHeight;
	pled->_yMatchNewTop = yHeightPrev;
	pled->_yMatchOld	= _yHeight;
	_cpCalcMax			= me.GetCp();

	// Replace old lines by new ones
	rpOld = pled->_iliFirst;

	// We store the result from the replace because although it can fail the 
	// fields used for first visible must be set to something sensible whether 
	// the replace fails or not. Further, the setting up of the first visible 
	// fields must happen after the Replace because the lines could have 
	// changed in length which in turns means that the first visible position
	// has failed.

	fReplaceResult = rpOld.Replace(-1, _prgliNew);

	// _iliMatchNew & _cpMatchNew are used for first visible constants so we
	// need to set them to something reasonable. In particular the rendering
	// logic expects _cpMatchNew to be set to the first character of the first
	// visible line. rpOld is used because it is convenient.

	// Note we can't use RpBindToCp at this point because the first visible
	// information is screwed up because we may have changed the line that
	// the first visible cp is on. 
	rpOld.BindToCp(me.GetCp());
	pled->_iliMatchNew = rpOld.GetLineIndex();
	pled->_cpMatchNew = me.GetCp() - rpOld.RpGetIch();

	if (!fReplaceResult)
	{
		TRACEERRORSZ("CDisplayML::RecalcLines rpOld.Replace() failed");
		goto errspace;
	}

    // Adjust first affected line if this line is gone
    // after replacing by new lines
    
    if(pled->_iliFirst >= (LONG)Count() && Count() > 0)
    {
        Assert(pled->_iliFirst == (LONG)Count());
        pled->_iliFirst = Count() - 1;
		pliNew = Elem(pled->_iliFirst);
        pled->_yFirst -= pliNew->GetHeight();
		AssertSz(pled->_yFirst >= 0, "CDisplayML::RecalcLines _yFirst < 0");
        pled->_cpFirst -= pliNew->_cch;
    }
    
match:
	_fRecalcDone = fDone;
    _fNeedRecalc = FALSE;
	_yCalcMax = yHeight;

	Tracef(TRCSEVINFO, "CDisplayML::RecalcLine(tpFirst, ...) - Done. Recalced down to line #%d", Count() - 1);

	// Clear wait fields since we want caller's to set them up.
	_yWait = -1;
	_cpWait = -1;

	if(fDone && fBackground)
	{
		TRACEINFOSZ("Background line recalc done");
		_ped->TxKillTimer(RETID_BGND_RECALC);
		_fBgndRecalc = FALSE;
		_fRecalcDone = TRUE;
	}

	// Determine display height and update scrollbar
	yHeightScrollNew = CalcScrollHeight(yHeight);

	if (_fViewChanged ||
		fDone && (yHeight != _yHeight || yHeightScrollNew != yHeightScrollOld)
		|| yHeightScrollNew > yHeightScrollOld) 
	{
		_yHeight = yHeight;
   		UpdateScrollBar(SB_VERT, TRUE);
	}

	// Determine display width and update scrollbar
	xWidth = CalcDisplayWidth();
    if(_fViewChanged || (fDone && xWidth != _xWidth) || xWidth > _xWidth)
    {
        _xWidth = xWidth;
   		UpdateScrollBar(SB_HORZ, TRUE);
    }    

    _fViewChanged = FALSE;

	// If not done, do the rest in background
	if(!fDone && !fBackground)
		fDone = StartBackgroundRecalc();

	if(fDone)
	{
		CheckLineArray();
		_fLineRecalcErr = FALSE;
	}

#ifdef DEBUG
	if( 1 )
    {
		_TEST_INVARIANT_
	}
#endif // DEBUG

	return TRUE;

errspace:
	_ped->GetCallMgr()->SetOutOfMemory();
	_fNeedRecalc = TRUE;
	_cpCalcMax = _yCalcMax = 0;
	_fLineRecalcErr = TRUE;

err:
	if(!_fLineRecalcErr)
	{
		_cpCalcMax = me.GetCp();
		_yCalcMax = yHeight;
	}

	TRACEERRORSZ("CDisplayML::RecalcLines() failed");

	if(!_fLineRecalcErr)
	{
		_fLineRecalcErr = TRUE;
		_ped->GetCallMgr()->SetOutOfMemory();
		_fLineRecalcErr = FALSE;			//  fix up CArray & bail
	}
	pled->SetMax(this);

	return FALSE;
}

/*
 *	CDisplayML::CalcDisplayWidth()
 *
 *	@mfunc
 *		Computes and returns width of this display by walking line
 *		CArray and returning the widest line.  Used for
 *		horizontal scroll bar routines
 *
 */
LONG CDisplayML::CalcDisplayWidth ()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CalcDisplayWidth");

	LONG	ili = Count();
	CLine *	pli;
	LONG	xWidth = 0, lineWidth;

	if(ili)
	{
		// Note: pli++ breaks array encapsulation (pli = &(*this)[ili] doesn't)
		pli = Elem(0);
		for(xWidth = 0; ili--; pli++)
		{
			lineWidth = pli->_xLeft + pli->_xWidth + pli->_xLineOverhang;
			xWidth = max(xWidth, lineWidth);
		}
	}
    return xWidth;
}

/*
 *	CDisplayML::StartBackgroundRecalc()
 *
 *	@mfunc
 *		Starts background line recalc (at _cpCalcMax position)
 *
 */
BOOL CDisplayML::StartBackgroundRecalc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::StartBackgroundRecalc");

	if(_fBgndRecalc)
	{
		// Already in background recalc
		return FALSE;
	}

	AssertSz(_cpCalcMax <= (LONG)_ped->GetTextLength(),
		"CDisplayML::StartBackgroundRecalc _cpCalcMax > Text Length");

	if (_cpCalcMax == (LONG)_ped->GetTextLength())
	{
		// Enough characters are recalc'd.
		return TRUE;
	}

	if (!_ped->TxSetTimer(RETID_BGND_RECALC, cmsecBgndInterval))
	{
		// Could not instantiate a timer so wait for recalculation
		WaitForRecalc(_ped->GetTextLength(), -1);
		return TRUE;
	}

	_fRecalcDone = FALSE;
	_fBgndRecalc = TRUE;
	return FALSE;
}

/*
 *	CDisplayML::StepBackgroundRecalc()
 *
 *	@mfunc
 *		Steps background line recalc (at _cpCalcMax position)
 *		Called by timer proc
 *
 */
void CDisplayML::StepBackgroundRecalc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::StepBackgroundRecalc");

    _TEST_INVARIANT_

	if (!_fBgndRecalc)
	{
		// We aren't in a background recalc so we don't
		// do anything.
		return;
	}

	LONG cch = _ped->GetTextLength() - _cpCalcMax;

	// Don't try recalc when processing OOM or had an error doing recalc or
	// if we are asserting.
#ifdef DEBUG
	if(_fInBkgndRecalc || _fLineRecalcErr || fInAssert)
	{
		if(_fInBkgndRecalc)
			TRACEINFOSZ("avoiding reentrant background recalc");
		else
			TRACEINFOSZ("OOM: not stepping recalc");
		return;
	}
#else
	if(_fInBkgndRecalc || _fLineRecalcErr )
		return;
#endif

	_fInBkgndRecalc = TRUE;

	if (!IsActive())
	{
		// Background recalc is over if we are no longer active	because
		// we can no longer get the information we need for recalculating.
		// But, if we are half recalc'd we need to set ourselves up to 
		// recalc again when we go active.
		InvalidateRecalc();
		cch = 0;
	}

	// Background recalc is over if no more characters or we are no longer
	// active.
	if(cch <= 0)
	{
		TRACEINFOSZ("Background line recalc done");
		_ped->TxKillTimer(RETID_BGND_RECALC);
		_fBgndRecalc = FALSE;
		_fRecalcDone = TRUE;
		_fInBkgndRecalc = FALSE;
		CheckLineArray();
		return;
	}

	_dwBgndTickMax = GetTickCount() + cmsecBgndBusy;
	CRchTxtPtr tp(_ped, _cpCalcMax);
	RecalcLines(tp, cch, cch, TRUE, FALSE, NULL);

	_fInBkgndRecalc = FALSE;
}

/*
 *	CDisplayML::WaitForRecalc(cpMax, yMax)
 *
 *	@mfunc
 *		Ensures that lines are recalced until a specific character
 *		position or ypos.
 *
 *	@rdesc
 *		success
 */
BOOL CDisplayML::WaitForRecalc(
	LONG cpMax,		//@parm Position recalc up to (-1 to ignore)
	LONG yMax)		//@parm ypos to recalc up to (-1 to ignore)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalc");

    _TEST_INVARIANT_

	if (IsFrozen() || !_ped->fInplaceActive())
	{
		return TRUE;
	}

	BOOL fReturn = TRUE;
	LONG cch;

	if((yMax < 0 || yMax >= _yCalcMax) &&
	   (cpMax < 0 || cpMax >= _cpCalcMax))
    {
    	cch = _ped->GetTextLength() - _cpCalcMax;
    	if(cch > 0 || Count() == 0)
    	{
    		HCURSOR hcur = NULL;
			BOOL fSetCursor = (cch > NUMCHARFORWAITCURSOR);

    		_cpWait = cpMax;
    		_yWait = yMax;
		
			if( fSetCursor )
			{
    			hcur = SetCursor(LoadCursor(0, IDC_WAIT));
			}
    		TRACEINFOSZ("Lazy recalc");
		
    		if(!_cpCalcMax || _fNeedRecalc )
			{
    			fReturn = RecalcLines(TRUE);
				if( !fReturn )
				{
					InitVars();
				}
			}
    		else			
    		{
    			CRchTxtPtr tp(_ped, _cpCalcMax);
    			fReturn = RecalcLines(tp, cch, cch, FALSE, TRUE, NULL);
    		}

			if( fSetCursor )
			{
    			SetCursor(hcur);
			}
    	}
		else if( cch == 0 )
		{
			// if there was nothing else to calc, make sure that we think
			// recalc is done.
#ifdef DEBUG
			if( !_fRecalcDone )
			{
				TRACEWARNSZ("For some reason we didn't think background "
					"recalc was done, but it was!!");
			}
#endif // DEBUG
			_fRecalcDone = TRUE;
		}
    }

	// If the view rect changed, make sure to update the scrollbars
	RecalcScrollBars();

	return fReturn;
}

/*
 *	CDisplayML::WaitForRecalcIli(ili)
 *
 *	@mfunc
 *		Wait until line array is recalculated up to line <p ili>
 *
 *	@rdesc
 *		Returns TRUE if lines were recalc'd up to ili
 */
BOOL CDisplayML::WaitForRecalcIli (
	LONG ili)		//@parm Line index to recalculate line array up to
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalcIli");

	LONG cchGuess;

	while(!_fRecalcDone && ili >= (LONG)Count())
	{
		// just go ahead and recalc everything.
		cchGuess = _ped->GetTextLength();
		if(!WaitForRecalc(cchGuess, -1))
			return FALSE;
	}
	return ili < (LONG)Count();
}

/*
 *	CDisplayML::WaitForRecalcView()
 *
 *	@mfunc
 *		Ensure visible lines are completly recalced
 *
 *	@rdesc TRUE iff successful
 */
BOOL CDisplayML::WaitForRecalcView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::WaitForRecalcView");

	return WaitForRecalc(-1, _yScroll + _yHeightView);
}

/*
 *	CDisplayML::InitLinePtr ( CLinePtr & plp )
 *
 *	@mfunc
 *		Initialize a CLinePtr properly
 */
void CDisplayML::InitLinePtr (
	CLinePtr & plp )		//@parm Ptr to line to initialize
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InitLinePtr");

    plp.Init( *this );
}

/*
 *	CDisplayML::GetLineText(ili, pchBuff, cchMost)
 *
 *	@mfunc
 *		Copy given line of this display into a character buffer
 *
 *	@rdesc
 *		number of character copied
 */
LONG CDisplayML::GetLineText(
	LONG ili,			//@parm Line to get text of
	TCHAR *pchBuff,		//@parm Buffer to stuff text into
	LONG cchMost)		//@parm Length of buffer
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetLineText");
    
	_TEST_INVARIANT_

	CTxtPtr tp (_ped, 0);

	// FUTURE (alexgo, ricksa): This is called from EM_GETLINE whose parameter
	// is a WPARAM which is unsigned we need to fix the type of ili.

	if(ili >= 0 && (ili < (LONG)Count() || WaitForRecalcIli(ili)))
	{
		cchMost = min(cchMost, (LONG)Elem(ili)->_cch);
		if(cchMost > 0)
		{
			tp.SetCp(CpFromLine(ili, NULL));
			return tp.GetText(cchMost, pchBuff);
		}
	}
	*pchBuff = TEXT('\0');
	return 0;
}

/*
 *	CDisplayML::LineCount
 *
 *	@mfunc	returns the number of lines in this control.  Note that for plain
 *			text mode, we will add on an extra line of the last character is
 *			a CR.  This is for compatibility with MLE
 *
 *	@rdesc	LONG
 */
LONG CDisplayML::LineCount() const
{
	LONG cLine = Count();

	if (!_ped->IsRich() && (!cLine || 	   // If plain text with no lines
		 Elem(cLine - 1)->_cchEOP))		   //  or last line ending with a CR,
	{									   //  then inc line count
		cLine++;
	}
	return cLine;
}

// ================================  Line info retrieval  ====================================

/*
 *	CDisplayML::YposFromLine(ili)
 *
 *	@mfunc
 *		Computes top of line position
 *
 *	@rdesc
 *		top position of given line (relative to the first line)
 */
LONG CDisplayML::YposFromLine(
	LONG ili) 		//@parm Line we're interested in
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::YposFromLine");

    _TEST_INVARIANT_

	if(!WaitForRecalcIli(ili))			// out of range, use last valid line
	{
		ili = Count() -1;
		ili = (ili > 0) ? ili : 0;
	}

	LONG	cli	= ili - _iliFirstVisible;
	CLine *	pli = Elem(_iliFirstVisible);
	LONG	yPos = _yScroll + _dyFirstVisible;

	while(cli > 0)
	{
		yPos += pli->GetHeight();
		cli--;
		pli++;
	}
	while(cli < 0)
	{	
		pli--;
		yPos -= pli->GetHeight();
		cli++;
	}

	AssertSz(yPos >= 0, "CDisplayML::YposFromLine height less than 0");

	return yPos;
}

/*
 *	CDisplayML::CpFromLine(ili, pyHeight)
 *
 *	@mfunc
 *		Computes cp at start of given line 
 *		(and top of line position relative to this display)
 *
 *	@rdesc
 *		cp of given line
 */
#define __OPT_PLAT_FLAG (defined (ARMV4T))
#define __OPT_VER_OFF
#define	__OPT_BUGNUMSTRING	"26316"
//#include <optimizer.h>

LONG CDisplayML::CpFromLine (
	LONG ili,		//@parm Line we're interested in (if <lt> 0 means caret line)
	LONG *pyHeight)	//@parm Returns top of line relative to display 
					//  	(NULL if don't want that info)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CpFromLine");

    _TEST_INVARIANT_
						
	LONG cli;
	LONG y = _yScroll + _dyFirstVisible;
	LONG cp = _cpFirstVisible;
	CLine *pli;
	LONG iStart = _iliFirstVisible;

	cli = ili - _iliFirstVisible;
	if(cli < 0 && -cli >= ili)
	{
		// Closer to first line than to first visible line,
		// so start at the first line
		cli = ili;
		y = 0;
		cp = 0;
		iStart = 0;
	}
	else if( cli <= 0 )
	{
		CheckView();

		for(ili = _iliFirstVisible-1; cli < 0; cli++, ili--)
		{
			pli = Elem(ili);
			y -= pli->GetHeight();
			cp -= pli->_cch;
		}
		goto end;
	}

	for(ili = iStart; cli > 0; cli--, ili++)
	{
		pli = Elem(ili);
		if(!IsMain() || !WaitForRecalcIli(ili))
			break;
		y += pli->GetHeight();
		cp += pli->_cch;
	}

end:
	if(pyHeight)
		*pyHeight = y;

	return cp;
}

#define __OPT_PLAT_FLAG (defined (ARMV4T))
#define __OPT_VER_RESTORE
#define	__OPT_BUGNUMSTRING	"26316"
//#include <optimizer.h>

/*
 *	CDisplayML::LineFromYPos(yPos, pyLine, pcpFirst)
 *
 *	@mfunc
 *		Computes line at given y position. Returns top of line ypos
 *		cp at start of line cp, and line index.
 *
 *	@rdesc
 *		index of line found
 */
LONG CDisplayML::LineFromYpos (
	LONG yPos,			//@parm Ypos to look for (relative to first line)
	LONG *pyLine,		//@parm Returns ypos at top of line /r first line (can be NULL)
	LONG *pcpFirst)		//@parm Returns cp at start of line (can be NULL)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineFromYpos");

    _TEST_INVARIANT_

	LONG cpLi;
	LONG dy;
	LONG ili = 0;
	LONG yLi;
	CLine *pli;

	if(!WaitForRecalc(-1, _yScroll))
	{
		yLi = 0;
		cpLi = 0;
		goto done;
	}

	cpLi = _cpFirstVisible;
	ili = _iliFirstVisible;
	yLi = _yScroll + _dyFirstVisible;
	dy = yPos - yLi;
	
	if(dy < 0 && -dy <= _yScroll)
	{
		// closer to first visible line than to first line
		// go backwards from the first visible line
		while(yPos < yLi && ili > 0)
		{
			pli = Elem(--ili);
			yLi -= pli->GetHeight();
			cpLi -= pli->_cch;
		}
	}
	else
	{
		if(dy < 0)
		{
			// closer to first line than to first visible line
			// so start at the first line
			cpLi = _cpMin;
			yLi = 0;
			ili = 0;
		}
		pli = Elem(ili);
		while(yPos > yLi && ili < (LONG)Count()-1)
		{
			yLi += pli->GetHeight();
			cpLi += pli->_cch;
			ili++;
			pli++;
		}
		if(yPos < yLi && ili > 0)
		{
			ili--;
			pli--;
			yLi -= pli->GetHeight();
			cpLi -= pli->_cch;
		}
	}

done:
	if(pyLine)
		*pyLine = yLi;

	if(pcpFirst)
		*pcpFirst = cpLi;

	return ili;
}

/*
 *	CDisplayML::LineFromCp(cp, fAtEnd)
 *
 *	@mfunc
 *		Computes line containing given cp.
 *
 *	@rdesc
 *		index of line found, -1 if no line at that cp.
 */
LONG CDisplayML::LineFromCp(
	LONG cp,		//@parm cp to look for
	BOOL fAtEnd)	//@parm If true, return previous line for ambiguous cp
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineFromCp");
    
	_TEST_INVARIANT_

	CLinePtr rp(this);
	
	if(!WaitForRecalc(cp, -1) || !rp.RpSetCp(cp, fAtEnd))
		return -1;

	return (LONG)rp;
}


//==============================  Point <-> cp conversion  ==============================

/*
 *	CDisplayML::CpFromPoint(pt, prcClient, ptp, prp, fAllowEOL)
 *
 *	@mfunc
 *		Determine cp at given point
 *
 *	@devnote
 *      --- Use when in-place active only ---
 *
 *	@rdesc
 *		Computed cp, -1 if failed
 */
LONG CDisplayML::CpFromPoint(
	POINT		pt,			//@parm Point to compute cp at (client coords)
	const RECT *prcClient,	//@parm Client rectangle (can be NULL if active).
	CRchTxtPtr * const ptp,	//@parm Returns text pointer at cp (may be NULL)
	CLinePtr * const prp,	//@parm Returns line pointer at cp (may be NULL)
	BOOL		fAllowEOL,	//@parm Click at EOL returns cp after CRLF
	HITTEST *	pHit)		//@parm Out parm for hit-test value
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CpFromPoint");

    _TEST_INVARIANT_

	LONG		cp;
	LONG		cch = 0;
	LONG		dx = 0;
	LONG		ili;
	CLine *		pli;
	LONG		yLine;
    RECT        rcView;

    if(pHit)
		*pHit = HT_Nothing;

	GetViewRect(rcView, prcClient);

	// Get line under hit
	ili = LineFromYpos(pt.y + _yScroll - rcView.top, &yLine, &cp);
    if(ili < 0)
        return -1;

	yLine -= _yScroll;
	pli = Elem(ili);

	// Create measurer 
	CMeasurer me(this);

	AssertSz((pli != NULL) || (ili == 0),
		"CDisplayML::CpFromPoint invalid line pointer");

	if (pli != NULL)
	{
		// Get character in the line
		me.SetCp(cp);
		cch = pli->CchFromXpos(me, pt.x + _xScroll - rcView.left, &dx, pHit);
		if(pHit && (pt.x > rcView.right || pt.y > rcView.bottom))
			*pHit = HT_Nothing;

		// Don't allow click at EOL to select EOL marker and take into account
		// single line edits as well
		if(!fAllowEOL && cch == (LONG)pli->_cch && pli->_cchEOP)
		{
			// Adjust the position on the line by the amount backed up
			cch += me._rpTX.BackupCpCRLF();
			me._rpCF.BindToCp(me.GetCp());
			me._rpPF.BindToCp(me.GetCp());
		}
	}

	if(ptp)
		*ptp = me;
	if(prp)
		prp->RpSet(ili, cch);

	return me.GetCp();	
}

/*
 *	CDisplayML::PointFromTp(tp, prcClient, fAtEnd, pt, prp, taMode)
 *
 *	@mfunc
 *		Determine coordinates at given tp
 *
 *	@devnote
 *      --- Use when in-place active only ---
 *
 *	@rdesc
 *		line index at cp, -1 if error
 */
LONG CDisplayML::PointFromTp(
	const CRchTxtPtr &tp,	//@parm Text ptr to get coordinates at
	const RECT *prcClient,	//@parm Client rectangle (can be NULL if active).
	BOOL fAtEnd,			//@parm Return end of previous line for ambiguous cp
	POINT &pt,				//@parm Returns point at cp in client coords
	CLinePtr * const prp,	//@parm Returns line pointer at tp (may be null)
	UINT taMode)			//@parm Text Align mode: top, baseline, bottom
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::PointFromTp");

    _TEST_INVARIANT_

	CLinePtr rp(this);
	RECT rcView;

    if( !WaitForRecalc(tp.GetCp(), -1) )
	{
		return -1;
	}

	if(!rp.RpSetCp(tp.GetCp(), fAtEnd))
		return -1;

    AssertSz(_ped->_fInPlaceActive || (prcClient != NULL), 
		"CDisplayML::PointFromTp() called with invalid client rect");

    GetViewRect(rcView, prcClient);

	pt.y = YposFromLine(rp);
	pt.x = rp.IsValid() ? rp->_xLeft : 0;

	pt.x += rcView.left - _xScroll;
	pt.y += rcView.top  - _yScroll;

	if (rp.RpGetIch())
	{
		CMeasurer me(this, tp);

		// Backup to start of line
		me.Advance(-rp.RpGetIch());		
		
		// And measure from there to where we are
		me.NewLine(*rp);

		LONG xCalc = me.MeasureText(rp.RpGetIch());

		if ((pt.x + xCalc <= rcView.right) || !GetWordWrap())
		{
			// Width is in view or there is no wordwrap so just
			// add the length to the point.
			pt.x += xCalc;
		}
		else
		{
			// Remember we ignore trailing spaces at the end of the line
			// in the width, therefore the x value that MeasureText finds can 
			// be greater than the width in the line so we truncate to the 
			// previously calculated width which will ignore the spaces.
			pt.x += rp->_xWidth; // we *don't* worry about xLineOverhang here.
		}
	}	
	
	if (prp)
		*prp = rp;
	
	if(rp >= 0 && taMode != TA_TOP)
	{
		// Check for vertical calculation request
		if (taMode & TA_BOTTOM)
		{
			const CLine *pli = Elem(rp);

			if(pli && !pli->_fCollapsed)
			{
				pt.y += pli->_yHeight;

				if((taMode & TA_BASELINE) == TA_BASELINE)
					pt.y -= pli->_yDescent;
			}
		}

		// Do any special horizontal calculation
		if (taMode & TA_CENTER)
		{
			pt.x += ModeOffsetIntoChar(taMode, tp);
		}
	}
	return rp;
}


//====================================  Rendering  =======================================

/*
 *	CDisplayML::Render(rcView, rcRender)
 *
 *	@mfunc	
 *		Searches paragraph boundaries around a range
 */
void CDisplayML::Render (
	const RECT &rcView,		//@parm View RECT
	const RECT &rcRender)	//@parm RECT to render (must be container in
							//		client rect)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Render");

    _TEST_INVARIANT_
								
    LONG	cp;
	BOOL	fLinesToRender = TRUE;
	LONG	ili;
	LONG	lCount = Count();
	CLine *	pli;
	CTxtSelection *psel = _ped->GetSelNC();
	POINT	pt;
	LONG	yBottomOfRender;
	LONG	yHeightForBitmap = 0;
	LONG	yHeightForLine;
    LONG	yLi = 0;
    LONG	yLine;
	LONG	yRenderHeight = rcRender.top + _yScroll - rcView.top;

    if(psel)
		psel->ClearCchPending();
        
	// Fire event "updating"
	if(IsMain())
		_ped->TxNotify(EN_UPDATE, NULL);

	// Calculate line and cp to start the diplay at
   	ili = LineFromYpos(rcRender.top + _yScroll - rcView.top, &yLine, &cp);

	yLi = Elem(ili)->GetHeight();

	yLi = max(yLi, 0);

	if (yRenderHeight > yLine + yLi)
		fLinesToRender = FALSE;

	// Calculate the point where the text will start being displayed
   	pt.x = rcView.left - _xScroll;
   	pt.y = rcView.top - _yScroll + yLine;

	// We only need check for whether we want to off screen render if the
	// control is rich text and it is not transparent. Remember if the 
	// control is transparent, the rendering of mixed character formats
	// will work because characters in adjoining runs are only truncated
	// if ExtTextOut is trying to clear the display area at the same time.
	if (!IsMetafile() && IsMain() 
		&& (_fUpdateOffScreen || (_ped->IsRich() && !_ped->_fTransparent)))
	{
		// This is rich text - we may want to use off screen rendering. We
		// only decide to set up off screen rendering if some line in the
		// set of displayed lines has mixed format runs. The measurer is 
		// the one that determines this.

		// Make sure update flag is no longer set as it has done its job.
		_fUpdateOffScreen = FALSE;

		// Initialize height counter to first position to display
		yLi = pt.y;
		yBottomOfRender = BottomOfRender(rcView, rcRender);

		// Loop through the visible lines until we have examined the entire
		// line array or we have exceeded the visible height
		for (LONG iliLoop = ili; 
			(iliLoop < lCount) && (yLi < yBottomOfRender); iliLoop++)
		{
			pli = Elem(iliLoop);
			if(pli->_fCollapsed)
				continue;

			// Get a local copy of the height
			yHeightForLine = pli->_yHeight;

			if ((pli->_bFlags & fliUseOffScreenDC) != 0)
			{
				// We only want to set the height for the bitmap if this
				// line needs off screen rendering. Otherwise, we can do
				// just as well by rendering the line directly.

				if (yHeightForLine > yHeightForBitmap)
				{
					// We want to figure out what is the height of the tallest 
					// line to be displayed off screen. The purpose of this is 
					// to allow the rendering to create  a bit map big enough 
					// that it does not have to be reallocated during 
					// the rendering of the screen.
					yHeightForBitmap = yHeightForLine;
				}
			}
	        yLi += yHeightForLine;
		}

		if ((0 != yHeightForBitmap) && (rcView.top > rcRender.top))
		{
			// Bit map for first line needs to be big enough to take into account
			// the area above the first line so, add the difference between view
			// and render start into the bitmap size.
			yHeightForBitmap += (rcView.top - rcRender.top);
		}
	}

	// Create renderer
	CRenderer re(this);

	// Prepare renderer
	if(!re.StartRender(rcView, rcRender, yHeightForBitmap))
		return;
	
	// Init renderer at the start of the first line to render
	re.SetCurPoint(pt);
   	re.SetCp(cp);

#ifdef DEBUG
    LONG cpLi = re.GetCp();
    yLi = pt.y;
#endif

	if (fLinesToRender)
	{
		// Render each line in the update rectangle
		for (; ili < lCount; ili++)
		{
			if(!re.RenderLine(GetAt(ili), ili == lCount - 1))
				break;

#ifdef DEBUG
			cpLi += Elem(ili)->_cch;
			yLi  += Elem(ili)->GetHeight();

			// Rich controls with password characters do not process EOPs. 
			// Therefore, the following assert is only valid when the above
			// is not the case.
			if (!_ped->IsRich() || !_ped->fUsePassword())
			{
				AssertSz(re.GetCp() == cpLi, 
					"CDisplayML::RenderView() - cp out of sync. with line table");
			}

			AssertSz(re.GetCurPoint().y == yLi,
				"CDisplayML::RenderView() - y out of sync. with line table");
#endif // DEBUG
		}
	}
	
	re.EndRender();						  // Finish rendering
}


//===================================  View Updating  ===================================

/*
 *	CDisplayML::RecalcView(fUpdateScrollBars)
 *
 *	@mfunc
 *		Recalc all lines breaks and update first visible line
 *
 *	@rdesc
 *		TRUE if success
 */
BOOL CDisplayML::RecalcView(
	BOOL fUpdateScrollBars)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::RecalcView");

	BOOL fRet = TRUE;
	const LONG cp = _cpFirstVisible;
	LONG yHeightOld = _yHeight;
	LONG yScrollHeightOld = GetMaxYScroll();
	LONG xWidthOld = _xWidth;

	// full recalc lines
	if(!RecalcLines())
	{
		// the recalc failed
		// let's try to get out of this with our head still mostly attached
		InitVars();
		fRet = FALSE;
        goto Done;
	}

	if (_ped->GetTextLength() == 0)
	{
		// This is an empty control so create one empty line
		CreateEmptyLine();
	}


    // force _xScroll = 0 if x scroll range is smaller than the view width
    if(_xWidth <= _xWidthView)
	{
        _xScroll = 0;
	}

	// change first visible entries because CLinePtr::RpSetCp() and
	// YPosFromLine() use them, but they're not valid
	_dyFirstVisible = 0;
	_cpFirstVisible = 0;
	_iliFirstVisible = 0;
	_yScroll = 0;


	// recompute scrolling position and first visible values after edit
    // force _yScroll = 0 if y scroll range is smaller than the view height
    if(_yHeight > _yHeightView)
    {   	
    	CLinePtr rp(this);
   		rp.RpSetCp(cp, FALSE);
   		_yScroll = YposFromLine(rp);
		// we use rp.GetCp() instead of cp, because cp could now be
		// woefully out of date.  RpSetCp will set us to the closest
		// available cp.
   		_cpFirstVisible = rp.GetCp() - rp.RpGetIch();
   		_iliFirstVisible = rp;
	}

	CheckView();

	// We only need to resize if the size needed to display the object has 
	// changed.
	if ((yHeightOld != _yHeight) || (yScrollHeightOld != GetMaxYScroll())
		|| (xWidthOld != _xWidth))
	{
		if(FAILED(RequestResize()))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
		}
	}

Done:

    // Now update the scrollbars
	if (fUpdateScrollBars)
	{
		RecalcScrollBars();
	}
    return fRet;
}

/*
 *	CDisplayML::UpdateView(&tpFirst, cchOld, cchNew)
 *
 *	@mfunc
 *		Recalc lines and update the visible part of the display 
 *		(the "view") on the screen.
 *
 *	@devnote
 *      --- Use when in-place active only ---
 *
 *	@rdesc
 *		TRUE if success
 */
BOOL CDisplayML::UpdateView(
	const CRchTxtPtr &tpFirst,	//@parm Text ptr where change happened
	LONG cchOld,				//@parm Count of chars deleted
	LONG cchNew)				//@parm Count of chars inserted
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::UpdateView");

	BOOL fReturn = TRUE;
	BOOL fRecalcVisible = TRUE;
	RECT rcClient;
    RECT rcView;
	CLed led;
	CTxtSelection *psel = _ped->GetSelNC();
	LONG cpStartOfUpdate = tpFirst.GetCp();
	BOOL fNeedViewChange = FALSE;
	LONG yHeightOld = _yHeight;
	LONG yScrollHeightOld = GetMaxYScroll();
	LONG xWidthOld = _xWidth;
	LONG yScrollOld = _yScroll;
	LONG cpNewFirstVisible;

	if (_fNoUpdateView)
		return fReturn;

	AssertSz(_ped->_fInPlaceActive, "CDisplayML::UpdateView(...) called when inactive");

	if( tpFirst.GetCp() > _cpCalcMax || _fNeedRecalc)
	{
		// we haven't even calc'ed this far, so don't bother with updating
		// here.  Background recalc will eventually catch up to us.
		return TRUE;
	}

	AssertSz(tpFirst.GetCp() <= _cpCalcMax, "CDisplayML::UpdateView(...) - tpFirst > _cpCaclMax");

	_ped->TxGetClientRect(&rcClient);
	GetViewRect(rcView, &rcClient);

	if(psel && !psel->PuttingChar())
		psel->ClearCchPending();

	DeferUpdateScrollBar();

	// In general, background recalc should not start until both the scroll 
	// position is beyond the visible view and the cp is beyond the first visible 
	// character. However, for the recalc we will only wait on the height. 
	// Later calls to WaitForRecalc will wait on cpFirstVisible if that is 
	// necessary.
	_yWait = _yScroll + _yHeightView;
	_cpWait = -1;

	if(!RecalcLines(tpFirst, cchOld, cchNew, FALSE, TRUE, &led))
	{
		// we're in deep crap now, the recalc failed
		// let's try to get out of this with our head still mostly attached
		InitVars();
		fRecalcVisible = TRUE;
		fReturn = FALSE;
		_ped->TxInvalidateRect (NULL, FALSE);
		fNeedViewChange = TRUE;
		goto Exit;
	}

	if(_ped->GetTextLength() == 0)
	{
		if(LineCount() != 0)
		{
			// There are currently elements in the line array, so zap them.
			Clear(AF_DELETEMEM);
		}
		// This is an empty control so create one empty line
		CreateEmptyLine();
	}


	if(_xWidth <= _xWidthView)
    {
		// x scroll range is smaller than the view width
        // force x scrolling position = 0
		_xScroll = 0;
    }

	if(led._yFirst >= _yScroll + _yHeightView)
	{
		// update is after the view, don't do anything
		fRecalcVisible = FALSE;

		Assert(VerifyFirstVisible());

		goto finish;
	}
	else if(led._yMatchNew <= _yScroll + _dyFirstVisible &&
			led._yMatchOld <= _yScroll + _dyFirstVisible)
	{
		// update is entirely before the view
		// just update scroll position but don't touch the screen
		_yScroll += led._yMatchNew - led._yMatchOld;
		_iliFirstVisible += led._iliMatchNew - led._iliMatchOld;
		_iliFirstVisible = max((LONG)_iliFirstVisible, 0);

		_cpFirstVisible += led._cpMatchNew - led._cpMatchOld;
		_cpFirstVisible = min((LONG)_ped->GetTextLength(), _cpFirstVisible);
		_cpFirstVisible = max(0, _cpFirstVisible);
		fRecalcVisible = FALSE;

		Assert(VerifyFirstVisible());
	}
	else
	{
		// update overlaps the visible view
		RECT rc = rcClient;
        RECT rcUpdate;
		RECT *prc = &rc;

		// Do we need to resync the first visible?  Note that this if check
		// is mostly an optmization; we could decide to _always_ recompute
		// this _iliFirstVisible if we wanted to.
		if ( cpStartOfUpdate <= _cpFirstVisible || 
			led._iliMatchOld <= _iliFirstVisible ||
			led._iliMatchNew <= _iliFirstVisible ||
			led._iliFirst <= _iliFirstVisible )
		{
			// Edit overlaps the first visible. We try to maintain
			// approximately the same place in the file visible.
			cpNewFirstVisible = _cpFirstVisible;

			if (_iliFirstVisible - 1 == led._iliFirst)
			{
				// The edit occurred on the line before the visible view. Most
				// likely this means that the the first character got pulled
				// back to the previous line so we want that line to be 
				// visible.
				cpNewFirstVisible = led._cpFirst;
			}

			// change first visible entries because CLinePtr::RpSetCp() and
			// YPosFromLine() use them, but they're not valid
			_dyFirstVisible = 0;
			_cpFirstVisible = 0;
			_iliFirstVisible = 0;
			_yScroll = 0;

			// with certain formatting changes, it's possible for 
			// cpNewFirstVisible to be less that what's been calculated so far 
			// in RecalcLines above. Wait for things to catch up.

			WaitForRecalc(cpNewFirstVisible, -1);

			// recompute scrolling position and first visible values after edit
		    CLinePtr rp(this);
   			rp.RpSetCp(cpNewFirstVisible, FALSE);
   			_yScroll = YposFromLine(rp);
   			_cpFirstVisible = rp.GetCp() - rp.RpGetIch();
   			_iliFirstVisible = rp;

			Assert(VerifyFirstVisible());
		}

		Assert(VerifyFirstVisible());

		// Is there a match in the display area? - this can only happen if the
		// old match is on the screen and the new match will be on the screen
		if ((led._yMatchOld < yScrollOld + _yHeightView)
			&& (led._yMatchNew < _yScroll + _yHeightView))
		{
			// we have a match inside the visible view

			// scroll the part that is below the old y pos of the match
			// or invalidate if the new y of the match is now below the view
			rc.top = rcView.top + (INT) (led._yMatchOld - yScrollOld);
			if(rc.top < rc.bottom)
			{
				// Calculate difference between the new and old screen
				// position.
				const INT dy = (INT) ((led._yMatchNew - _yScroll)) 
					- (led._yMatchOld - yScrollOld);

				if(dy)
				{
				    if(!_ped->_fTransparent)
                   {
    					_ped->TxScrollWindowEx(0, dy, &rc, &rcView,
    						NULL, &rcUpdate, 0);
		    			_ped->TxInvalidateRect(&rcUpdate, FALSE);
						fNeedViewChange = TRUE;

    					if(dy < 0)
	    				{
		    				rc.top = rc.bottom + dy;
			    			_ped->TxInvalidateRect(&rc, FALSE);
							fNeedViewChange = TRUE;
				    	}
    				}
                    else
                    {
						// adjust the rect since we are not doing 
						// scrolling for transparent mode
						RECT	rcInvalidate = rc;
   						rcInvalidate.top += dy;

                        _ped->TxInvalidateRect(&rcInvalidate, FALSE);
						fNeedViewChange = TRUE;
                    }
				}
			}
			else
			{
				rc.top = rcView.top + led._yMatchNew - _yScroll;
				_ped->TxInvalidateRect(&rc, FALSE);
				fNeedViewChange = TRUE;
			}

			// Because above it was determined that the new match fell
			// within the screen, we can safely set the bottom to the
			// new match since this is the most that can have changed.
			rc.bottom = rcView.top 
				+ (INT) (max(led._yMatchNew, led._yMatchOld) - _yScroll);
		}

		rc.top = rcView.top + (INT) (led._yFirst - _yScroll);

		// Set the first line edited to be rendered using off screen
		// rendering. 

		if (led._iliFirst < (LONG) Count() && !_ped->_fTransparent
			&& (Elem(led._iliFirst)->_bFlags & fliUseOffScreenDC) == 0)
		{	
			Elem(led._iliFirst)->_bFlags |= 
				(fliOffScreenOnce |	fliUseOffScreenDC);
			_fUpdateOffScreen = TRUE;
		}
		
		// Invalidate part of update that is above match (if any)
		_ped->TxInvalidateRect (&rc, FALSE);
		fNeedViewChange = TRUE;
	}

finish:

	if(fRecalcVisible)
	{
		fReturn = WaitForRecalcView();
		if( !fReturn )
		{
			goto Exit;
		}
	}

	if (fNeedViewChange)
	{
		_ped->GetHost()->TxViewChange(FALSE);
	}

	CheckView();

	// We only need to resize if the size needed to display the object has 
	// changed.
	if ((yHeightOld != _yHeight) || (yScrollHeightOld != GetMaxYScroll())
		|| (xWidthOld != _xWidth))
	{
		if(FAILED(RequestResize()))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
		}
	}

	if(DoDeferredUpdateScrollBar())
	{
		if(FAILED(RequestResize()))
			_ped->GetCallMgr()->SetOutOfMemory();
		DoDeferredUpdateScrollBar();
	}

Exit:
	return fReturn;
}

void CDisplayML::InitVars()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InitVars");

	_yScroll = _xScroll = 0;
	_iliFirstVisible = 0;
	_cpFirstVisible = _cpMin = 0;
	_dyFirstVisible = 0;
}

/*
 *	CDisplayML::GetCliVisible(pcpMostVisible)
 *
 *	@mfunc	
 *		Get count of visible lines and update _cpMostVisible for PageDown()
 *
 *	@rdesc
 *		count of visible lines
 */
LONG CDisplayML::GetCliVisible(
	LONG* pcpMostVisible, 				//@parm Returns cpMostVisible
	BOOL fLastCharOfLastVisible) const 	//@parm Want cp of last visible char
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetCliVisible");

	LONG cli	 = 0;							// Initialize count
	LONG ili	 = _iliFirstVisible;			// Start with 1st visible line
	LONG yHeight = _dyFirstVisible;
    LONG cp;
	LONG cchWhite = 0;

	for(cp = _cpFirstVisible;
		yHeight < _yHeightView && ili < (LONG)Count();
		cli++, ili++)
	{
		const CLine* pli = Elem(ili);
		yHeight	+= pli->GetHeight();

		if (fLastCharOfLastVisible)
		{
			if (yHeight > _yHeightView)
			{
				// Back up cp to the last visible character
				cp -= cchWhite;
				break;
			}

			// Save last lines white space to adjust cp if
			// this is the last fully displayed line.
			cchWhite = pli->_cchWhite;
		}
		cp += pli->_cch;
	}

    if(pcpMostVisible)
        *pcpMostVisible = cp;

	return cli;
}

//==================================  Inversion (selection)  ============================

/*
 *	CDisplayML::InvertRange(cp, cch)
 *
 *	@mfunc
 *		Invert a given range on screen (for selection)
 *
 *	@devnote
 *      --- Use when in-place active only ---
 *
 *	@rdesc
 *		TRUE if success
 */
BOOL CDisplayML::InvertRange (
	LONG cp,					//@parm Active end of range to invert
	LONG cch,					//@parm Signed length of range
	SELDISPLAYACTION selAction)	//@parm Describes what we are doing to the selection
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::InvertRange");

	BOOL     fResult;
	LONG	 cpMost;
	RECT	 rc;
	RECT     rcView;
	CLinePtr rp(this);
	CRchTxtPtr  rtp(_ped);
	LONG	 y;

    AssertSz(_ped->_fInPlaceActive,
    	"CDisplayML::InvertRange() called when not in-place active");


	if(cch < 0)						// Define cpMost, set cp = cpMin,
	{								//  and cch = |cch|
		cpMost = cp - cch;
		cch = -cch;
	}
	else
	{
		cpMost = cp;
		cp -= cch;
	}

	// If an object is being inverted, and that is all that
	// is being inverted, delegate to the ObjectMgr.  If fIgnoreObj
	// is TRUE we highlight normally regardless.
	if( cch == 1 && _ped->GetObjectCount() &&
		(selAction == selSetNormal || selAction == selSetHiLite) )
	{
		CObjectMgr* pobjmgr = _ped->GetObjectMgr();

		rtp.SetCp(cp);
		if( rtp.GetChar() == WCH_EMBEDDING )
		{
			if (pobjmgr)
			{
				pobjmgr->HandleSingleSelect(_ped, cp, selAction == selSetHiLite);
			}

			fResult = TRUE;
			goto Cleanup;
		}
	}

	// if the display is frozen, just update the recalc region and
	// move on.
	if( _padc != NULL )
	{
		AssertSz(cp >= 0, "CDisplayML::InvertRange: range (cp) goes below"
				"zero!!" );
		// make sure these values are bounded.
		if( cp > (LONG)_ped->GetTextLength() )
		{
			// don't bother updating the region, it's out
			// of bounds
			return TRUE;
		}
		if( cp + cch > (LONG)_ped->GetTextLength() )
		{
			cch -= (cp + cch) - _ped->GetTextLength();
		}	
		_padc->UpdateRecalcRegion(cp, cch, cch);
		return TRUE;
	}

	if (!WaitForRecalcView ())			// Ensure all visible lines are
		return FALSE;					//  recalc'd

	GetViewRect(rcView);				// Get view rectangle
	
	// Compute first line to invert and where to start on it
	if (cp >= _cpFirstVisible)
	{
		POINT pt;
		rtp.SetCp(cp);

		if(PointFromTp(rtp, NULL, FALSE, pt, &rp, TA_TOP) < 0)
		{
			fResult = FALSE;
			goto Cleanup;
		}
		rc.left = pt.x;
		rc.top = pt.y;
	}
	else
	{
		cp = _cpFirstVisible;
		rp = _iliFirstVisible;
		rc.left = -1;
		rc.top = rcView.top + _dyFirstVisible;
	}				

	// loop on all lines of range
	while (cp < cpMost && rc.top < rcView.bottom && rp.IsValid())
	{
		// Calculate rc.bottom first because rc.top takes into account
		// the dy of the first visible on the first loop.
		y = rc.top;
		y += rp->GetHeight();
		rc.bottom = min(y, rcView.bottom);
        rc.top = max(rc.top, rcView.top);

		if (rc.left == -1)
			rc.left = rp->_xLeft - _xScroll + rcView.left;
	
		cp += rp->_cch - rp.RpGetIch();
	
        rc.left = rcView.left;
        rc.right = rcView.right;

	    _ped->TxInvalidateRect(&rc, TRUE);
		
		rc.top = rc.bottom;
		
		if( !rp.NextRun() )
			break;
	}


	// Make sure the window gets repainted.
	_ped->TxUpdateWindow();

	fResult = TRUE;

Cleanup:
	return fResult;
}


//===================================  Scrolling  =============================

/*
 *	CDisplay::GetYScroll()
 *
 *	@mfunc
 *		Returns vertical scrolling position
 */
LONG CDisplayML::GetYScroll() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetYScroll");

    return _yScroll;
}

/*
 *	CDisplay::VScroll(wCode, yPos)
 *
 *	@mfunc
 *		Scroll the view vertically in response to a scrollbar event
 *
 *	@devnote
 *      --- Use when in-place active only ---
 *
 *	@rdesc
 *		LRESULT formatted for WM_VSCROLL message		
 */
LRESULT CDisplayML::VScroll(
	WORD wCode,		//@parm Scrollbar event code
	LONG yPos)		//@parm Thumb position (yPos <lt> 0 for EM_SCROLL behavior)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::VScroll");

	LONG		cliVisible;
	LONG		dy = 0;
	BOOL		fTracking = FALSE;
	LONG		i;
	const LONG	ili = _iliFirstVisible;
	CLine *		pli = NULL;
	INT			yHeightSys = GetYHeightSys();
	LONG		yScroll = _yScroll;
    
    AssertSz(_ped->_fInPlaceActive, "CDisplay::VScroll() called when not in-place");

	if (yPos != 0)
	{
		// Convert this from 16-bit to 32-bit if necessary.
		yPos = ConvertScrollToYPos(yPos);
	}

	yPos = min(yPos, _yHeight);

	switch(wCode)
	{
	case SB_BOTTOM:
		if(yPos < 0)
			return FALSE;
		WaitForRecalc(_ped->GetTextLength(), -1);
		yScroll = _yHeight;
		break;

	case SB_LINEDOWN:
		cliVisible = GetCliVisible();
		if(_iliFirstVisible + cliVisible < (LONG)Count()
			&& 0 == _dyFirstVisible)
		{
			i = _iliFirstVisible + cliVisible;
			pli = Elem(i);
			if(IsInOutlineView())
			{	// Scan for uncollapsed line
				for(; pli->_fCollapsed && i < (LONG)Count();
					pli++, i++);
			}
			if(i < (LONG)Count())
				dy = pli->_yHeight;
		}
		else if(cliVisible > 1)
		{
			pli = Elem(_iliFirstVisible);
			dy = _dyFirstVisible;
			// TODO: scan until find uncollapsed line
			dy += pli->_yHeight;
		}
		else
			dy = _yHeight - _yScroll;

		if(dy >= _yHeightView)
			dy = yHeightSys;

		// nothing to scroll, early exit
		if ( !dy )
			return MAKELRESULT(0, TRUE); 

		yScroll += dy;
		break;

	case SB_LINEUP:
		if(_iliFirstVisible > 0)
		{
			pli = Elem(_iliFirstVisible - 1);
			// TODO: scan until find uncollapsed line
			dy = pli->_yHeight;
		}
		else if(yScroll > 0)
			dy = min(yScroll, yHeightSys);

		if(dy > _yHeightView)
			dy = yHeightSys;
		yScroll -= dy;
		break;

	case SB_PAGEDOWN:
		cliVisible = GetCliVisible();
		yScroll += _yHeightView;
		if(yScroll < _yHeight && cliVisible > 0)
		{
			// TODO: Scan until find uncollapsed line
			dy = Elem(_iliFirstVisible + cliVisible - 1)->_yHeight;
			if(dy >= _yHeightView)
			{
				dy = yHeightSys;
			}
			else if (dy > _yHeightView - dy)
			{
				// Go at least a line if the line
				// is very big.
				dy = _yHeightView - dy;
			}
			
			yScroll -= dy;
		}
		break;

	case SB_PAGEUP:
		cliVisible = GetCliVisible();
		yScroll -= _yHeightView;
		if(cliVisible > 0)
		{
			// TODO: Scan until find uncollapsed line
			dy = Elem(_iliFirstVisible)->_yHeight;
			if(dy >= _yHeightView)
			{
				dy = yHeightSys;
			}
			else if (dy > _yHeightView - dy)
			{
				// Go at least a line if the line
				// is very big.
				dy = _yHeightView - dy;
			}
			yScroll += dy;
		}
		break;

	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:

		if(yPos < 0)
			return FALSE;

		yScroll = yPos;
		fTracking = TRUE;

		break;

	case SB_TOP:
		if(yPos < 0)
			return FALSE;
		yScroll = 0;
		break;

	case SB_ENDSCROLL:
		UpdateScrollBar(SB_VERT);
		return MAKELRESULT(0, TRUE);;

	default:
		return FALSE;
	}

	ScrollView(_xScroll, yScroll, fTracking, FALSE);

	// force position update if we just finished a track
	if(wCode == SB_THUMBPOSITION)
		UpdateScrollBar(SB_VERT);

	// return how many lines we scrolled
	return MAKELRESULT((WORD) (_iliFirstVisible - ili), TRUE);
}

/*
 *	CDisplay::LineScroll(cli, cch)
 *
 *	@mfunc
 *		Scroll view vertically in response to a scrollbar event
 */
void CDisplayML::LineScroll(
	LONG cli,		//@parm Count of lines to scroll vertically
	LONG cch)		//@parm Count of characters to scroll horizontally
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::LineScroll");

	LONG yScroll;

	yScroll = CalcYLineScrollDelta(cli, FALSE);
	if ( yScroll)
		ScrollView(_xScroll, yScroll, FALSE, FALSE);
}

/*
 *	CDisplayML::FractionalScrollView (yDelta)
 *
 *	@mfunc
 *		Allow view to be scrolled by fractional lines.
 */
void CDisplayML::FractionalScrollView ( LONG yDelta )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::FractionalScrollView");

	if ( yDelta)
		ScrollView(_xScroll, yDelta + _yScroll, FALSE, TRUE);
}

/*
 *	CDisplayML::ScrollToLineStart(iDirection)
 *
 *	@mfunc
 *		If the view is scrolled so that only a partial line is at the
 *		top, then scroll the view so that the entire view is at the top.
 */
void CDisplayML::ScrollToLineStart(
	LONG iDirection)	//@parm the direction in which to scroll (negative
						// means down the screen
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::ScrollToLineStart");

	// if _dyFirstVisible is zero, then we're aligned on a line, so
	// nothing more to do.

	if( _dyFirstVisible )
	{
		if( iDirection > 0 )
		{
			ScrollView(_xScroll, _yScroll + _dyFirstVisible, FALSE, TRUE);
		}
		else
		{
			ScrollView(_xScroll, _yScroll + 
					Elem(_iliFirstVisible)->_yHeight + _dyFirstVisible , FALSE, 
					TRUE);
		}
	}
}

/*
 *	CDisplayML::CalcYLineScrollDelta (cli, fFractionalFirst)
 *
 *	@mfunc
 *		Given a count of lines, positive or negative, calc the number
 *		of vertical units necessary to scroll the view to the start of
 *		the current line + the given count of lines.
 */
LONG CDisplayML::CalcYLineScrollDelta (
	LONG cli,
	BOOL fFractionalFirst )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CalcYLineScrollDelta");

	LONG yScroll = 0;

	if ( fFractionalFirst && _dyFirstVisible )		// Scroll partial for 1st.
	{
		Assert ( _dyFirstVisible <= 0 ); // get jonmat

		if ( cli < 0 )
		{
			cli++;
			yScroll = _dyFirstVisible;
		}
		else
		{
			cli--;
			yScroll = Elem(_iliFirstVisible)->_yHeight + _dyFirstVisible;
		}
	}

	if(cli > 0)
		cli = min(cli, (LONG)Count() - _iliFirstVisible - 1);
	else if(cli < 0)
		cli = max(cli, -_iliFirstVisible);

	if(cli)
	{
		yScroll += YposFromLine(_iliFirstVisible + cli) - YposFromLine(_iliFirstVisible);
	}
	return yScroll;
}

/*
 *	CDisplayML::ScrollView(xScroll, yScroll, fTracking, fFractionalScroll)
 *
 *	@mfunc
 *		Scroll view to new x and y position
 *
 *	@devnote 
 *		This method tries to adjust the y scroll pos before
 *		scrolling to display complete line at top. x scroll 
 *		pos is adjusted to avoid scrolling all text off the 
 *		view rectangle.
 *
 *		Must be able to handle yScroll <gt> pdp->yHeight and yScroll <lt> 0
 *
 *	@rdesc
 *		TRUE if actual scrolling occurred, 
 *		FALSE if no change
 */
BOOL CDisplayML::ScrollView (
	LONG xScroll,		//@parm New x scroll position
	LONG yScroll,		//@parm New y scroll position
	BOOL fTracking,		//@parm TRUE indicates we are tracking scrollbar thumb
	BOOL fFractionalScroll)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::ScrollView");
						//		(don't update the scrollbar pos)
	BOOL fTryAgain = TRUE;
	RECT rcUpdate;      // ??? we may want use a region here but ScrollView is 
                        // rarely called with both a xScroll and yScroll value.
	LONG xWidthMax;
	LONG dx = 0;
	LONG dy = 0;
    RECT rcView;
	CTxtSelection *psel = _ped->GetSelNC();
	COleObject *pipo;
	BOOL fRestoreCaret = FALSE;


    AssertSz(_ped->_fInPlaceActive, "CDisplayML::ScrollView() called when not in-place");
    GetViewRect(rcView);

	if(xScroll == -1)
        xScroll = _xScroll;
	if(yScroll == -1)
        yScroll = _yScroll;

#ifdef PWD_JUPITER
  // Don't allow the dead zone contents to be scrolled left.
  /*
  if(xScroll == 0)
  {
      RECT rcInset;

      // GuyBark 34191 Jupiter: We must extent the right edge of 
      // the view, otherwise letters with overhangs get cut off. 
      // We take similar action during a print.
      _ped->TxGetViewInset(&rcInset, this);

      rcView.right += rcInset.right;
  }
*/
  // Bug #34194 Jupiter and #12349 Talisker
  if(yScroll != _yScroll){
      RECT rcInset;
      _ped->TxGetViewInset(&rcInset, this);

      rcView.right += rcInset.right;
      rcView.left  -= rcInset.left;
	}

	if(!_xScroll && xScroll){
      RECT rcInset;
      RECT rcTemp;
      int ret = 0;

      _ped->TxGetViewInset(&rcInset, this);

      SetRect(&rcTemp, 
              rcView.left   - rcInset.left, 
              rcView.top    - rcInset.top,
              rcInset.left,
              rcView.bottom + rcInset.bottom);
      ret = FillRect(_ped->TxGetDC(), &rcTemp, GetSysColorBrush(COLOR_WINDOW));
      _ped->TxInvalidateRect(&rcTemp, FALSE);

      SetRect(&rcTemp, 
              rcView.right, 
              rcView.top    - rcInset.top,
              rcView.right  + rcInset.right,
              rcView.bottom + rcInset.bottom);
      ret = FillRect(_ped->TxGetDC(), &rcTemp, GetSysColorBrush(COLOR_WINDOW));
      _ped->TxInvalidateRect(&rcTemp, FALSE);
      _ped->TxUpdateWindow();
	}
#endif // PWD_JUPITER
    
	// Determine vertical scrolling pos
	while(1)
	{
		BOOL fNothingBig = TRUE;
		LONG yFirst;
		LONG dyFirst;
		LONG cpFirst;
		LONG iliFirst;
		LONG yHeight;
		LONG iliT;

		yScroll = min(yScroll, GetMaxYScroll());
		yScroll = max(0, yScroll);
		dy = 0;

		// Ensure all visible lines are recalced
		if( !WaitForRecalcView() )
		{
			return FALSE;
		}

		// Compute new first visible line
		iliFirst = LineFromYpos(yScroll, &yFirst, &cpFirst);

		if( cpFirst < 0 )
		{
			// FUTURE (alexgo) this is pretty bogus, we should try to do
			// better in the next rel.

			TRACEERRORSZ("Display calc hosed, trying again");
			InitVars();
			_fNeedRecalc = TRUE;
			return FALSE;
		}

		if(iliFirst < 0)
		{
			// No line at _yScroll, use last line instead
			iliFirst = max(0, Count() - 1);
			cpFirst = _ped->GetTextLength() - Elem(iliFirst)->_cch;
			yScroll = _yHeight - Elem(iliFirst)->_yHeight;
			yFirst = _yScroll;
		}
		dyFirst = yFirst - yScroll;
		
		// Figure whether there is a big line
		// (more that a third of the view rect)
		for(iliT = iliFirst, yHeight = dyFirst;
			yHeight < _yHeightView && iliT < (LONG)Count();
			iliT++)
		{
			const CLine* pli = Elem(iliT);
			if(pli->_yHeight >= _yHeightView / 3)
				fNothingBig = FALSE;
			yHeight += pli->_yHeight;
		}

		// If no big line and first pass, try to adjust 
		// scrolling pos to show complete line at top
		if(!fFractionalScroll && fTryAgain && fNothingBig && dyFirst != 0)
		{
			fTryAgain = FALSE;		// prevent any infinite loop

			Assert(dyFirst < 0);

			Tracef(TRCSEVINFO, "adjusting scroll for partial line at %d", dyFirst);
			// partial line visible at top, try to get a complete line showing
			yScroll += dyFirst;

			LONG yHeightLine = Elem(iliFirst)->_yHeight;

			// Adjust the height of the scroll by the height of the first 
			// visible line if we are scrolling down or if we are using the 
			// thumb (tracking) and we are on the last page of the view.
			if ((fTracking && (yScroll + _yHeightView 
					+ yHeightLine > _yHeight))
				|| (!fTracking && (_yScroll <= yScroll)))
			{
				// Scrolling down so move down a little more
				yScroll += yHeightLine;
			}
		}
		else
		{
			dy = 0;
			if(yScroll != _yScroll)
			{
				_iliFirstVisible = iliFirst;
				_dyFirstVisible = dyFirst;
				_cpFirstVisible = cpFirst;
				dy = _yScroll - yScroll;
				_yScroll = yScroll;

				AssertSz(_yScroll >= 0, "CDisplayML::ScrollView _yScroll < 0");
				Assert(VerifyFirstVisible());
			}
			break;
		}
	}

	CheckView();

	// Determine horizontal scrolling pos.
	xWidthMax = _xWidth;
	xScroll = min(xScroll, xWidthMax);
	xScroll = max(0, xScroll);

	dx = _xScroll - xScroll;
	if(dx)
    {
		_xScroll = xScroll;
    }

	// Now perform the actual scrolling
	if(IsMain() && (dy || dx))
	{
		// Scroll only if scrolling < view dimensions and we are in-place
		if(IsActive() && !_ped->_fTransparent && 
		    dy < _yHeightView && dx < _xWidthView)
		{
			// FUTURE: (ricksa/alexgo): we may be able to get rid of 
			// some of these ShowCaret calls; they look bogus.
			if (psel && psel->IsCaretShown())
			{
				_ped->TxShowCaret(FALSE);
				fRestoreCaret = TRUE;
			}


			_ped->TxScrollWindowEx((INT) dx, (INT) dy, NULL, &rcView,
				NULL, &rcUpdate, 0);

			_ped->TxInvalidateRect(&rcUpdate, FALSE);

			if(fRestoreCaret)
			{
				_ped->TxShowCaret(FALSE);
			}
		}
		else
		{
			_ped->TxInvalidateRect(&rcView, FALSE);
		}

		if (psel)
			psel->UpdateCaret(FALSE);

		if(!fTracking && dy)
		{
			UpdateScrollBar(SB_VERT);
			_ped->SendScrollEvent(EN_VSCROLL);
		}
		if(!fTracking && dx)
		{
			UpdateScrollBar(SB_HORZ);
			_ped->SendScrollEvent(EN_HSCROLL);
		}
						
		_ped->TxUpdateWindow();

		// FUTURE: since we're now repositioning in place active 
		// objects every time we draw, this call seems to be 
		// superfluous (AndreiB)

		// Tell object subsystem to reposition any in place objects
		if( _ped->GetObjectCount() )
		{
		    CObjectMgr *pCobM = _ped->GetObjectMgr();
		    if (NULL != pCobM)
		    {
    			pipo = pCobM->GetInPlaceActiveObject();
    			if (NULL != pipo)
    			{
    				pipo->OnReposition( dx, dy );
			    }
			}
		}
	}
	return dy || dx;
}

/*
 *	CDisplayML::GetScrollRange(nBar)
 *
 *	@mfunc
 *		Returns the max part of a scrollbar range for scrollbar <p nBar>
 *
 *	@rdesc
 *		LONG max part of scrollbar range
 */
LONG CDisplayML::GetScrollRange(
	INT nBar) const		//@parm Scroll bar to interrogate (SB_VERT or SB_HORZ)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetScrollRange");

    Assert( IsMain() );

	LONG lRange = 0;
    
    if (nBar == SB_VERT && _fVScrollEnabled)
    {
	    if(_ped->TxGetScrollBars() & WS_VSCROLL)
			lRange = GetMaxYScroll();
    }
	else if((_ped->TxGetScrollBars() & WS_HSCROLL) && _fHScrollEnabled)
	{
		// Scroll range is maximum width plus room for the caret.
		lRange = max(0, _xWidth + dxCaret);
    }
	// Since thumb messages are limited to 16-bit, limit range to 16-bit
	lRange = min(lRange, _UI16_MAX);
	return lRange;
}

/*
 *	CDisplayML::UpdateScrollBar(nBar, fUpdateRange)
 *
 *	@mfunc
 *		Update either the horizontal or vertial scroll bar
 *		Also figure whether the scroll bar should be visible or not
 *
 *	@rdesc
 *		BOOL
 */
BOOL CDisplayML::UpdateScrollBar(
	INT nBar,				//@parm Which scroll bar : SB_HORZ, SB_VERT
	BOOL fUpdateRange)		//@parm Should the range be recomputed and updated
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::UpdateScrollBar");

	// Note: In the old days we didn't allow autosize & scroll bars, so to keep
	// forms working, we need this special logic with respect to autosize.
	if (!IsActive() || _fInRecalcScrollBars 
		|| (!_ped->fInOurHost() && _ped->TxGetAutoSize()))
	{
		// No scroll bars unless we are inplace active and we are not in the
		// process of updating scroll bars already.
		return TRUE;
	}

	const DWORD dwScrollBars = _ped->TxGetScrollBars();
	const BOOL fHide = !(dwScrollBars & ES_DISABLENOSCROLL);
	BOOL fReturn = FALSE;
	BOOL fEnabled = TRUE;
	BOOL fEnabledOld;
	LONG lScroll;
	CTxtSelection *psel = _ped->GetSelNC();
	BOOL fShowCaret = FALSE;

	// Get scrolling position
	if(nBar == SB_VERT)
	{
		if(!(dwScrollBars & WS_VSCROLL))
			return FALSE;

		fEnabledOld = _fVScrollEnabled;
        if(GetMaxYScroll() <= _yHeightView)
            fEnabled = FALSE;
    }
	else
	{
		if(!(dwScrollBars & WS_HSCROLL))
		{
			// Even if we don't have scrollbars, we may allow horizontal
			// scrolling.
			if(!_fHScrollEnabled && _xWidth > _xWidthView)
				_fHScrollEnabled = !!(dwScrollBars & ES_AUTOHSCROLL);

			return FALSE;
		}

		fEnabledOld = _fHScrollEnabled;
        if(_xWidth <= _xWidthView)
            fEnabled = FALSE;
	}

	// Don't allow ourselves to be re-entered.
	// Be sure to turn this to FALSE on exit
	_fInRecalcScrollBars = TRUE;

	// !s beforehand because all true values aren't necessarily equal
	if(!fEnabled != !fEnabledOld)
	{
		if(_fDeferUpdateScrollBar)
			_fUpdateScrollBarDeferred = TRUE;
		else
		{
			if (nBar == SB_HORZ)
				_fHScrollEnabled = fEnabled;
			else
				_fVScrollEnabled = fEnabled;
		}

		if(!_fDeferUpdateScrollBar)
		{
    		if(!fHide)
			{
				// Don't hide scrollbar, just disable
    			_ped->TxEnableScrollBar(nBar, fEnabled ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);

				if (!fEnabled)
				{
					// The scroll bar is disabled. Therefore, all the text fits
					// on the screen so make sure the drawing reflects this.
					_yScroll = 0;
					_dyFirstVisible = 0;
					_cpFirstVisible = 0;
					_iliFirstVisible = 0;
					_ped->TxInvalidateRect(NULL, FALSE);
				}
			}
    		else 
    		{
    			fReturn = TRUE;
    			// Make sure to hide caret before showing scrollbar
    			if(psel)
    				fShowCaret = psel->ShowCaret(FALSE);

    			// Hide or show scroll bar
    			_ped->TxShowScrollBar(nBar, fEnabled);
				// The scroll bar affects the window which in turn affects the 
				// display. Therefore, if word wrap, repaint
				_ped->TxInvalidateRect(NULL, TRUE);
				_ped->TxUpdateWindow();

    			if(fShowCaret)
    				psel->ShowCaret(TRUE);
            }
		}
	}
	
	// Set scrollbar range and thumb position
	if(fEnabled)
	{
        if (fUpdateRange)
        {
			if(!_fDeferUpdateScrollBar)
				_ped->TxSetScrollRange(nBar, 0, GetScrollRange(nBar), FALSE);
        }
        
		if(_fDeferUpdateScrollBar)
			_fUpdateScrollBarDeferred = TRUE;
		else
		{
			lScroll = (nBar == SB_VERT)
				? ConvertYPosToScrollPos(_yScroll)
				: ConvertXPosToScrollPos(_xScroll);

			_ped->TxSetScrollPos(nBar, lScroll, TRUE);
		}
	}

	_fInRecalcScrollBars = FALSE;
	return fReturn;
}

/*
 *	CDisplayML::GetNaturalSize(hdcDraw, hicTarget, dwMode, pwidth, pheight)
 *
 *	@mfunc
 *		Recalculate display to input width & height for TXTNS_FITTOCONTENT.
 *
 *	@rdesc
 *		S_OK - Call completed successfully <nl>
 */
HRESULT	CDisplayML::GetNaturalSize(
	HDC hdcDraw,		//@parm DC for drawing
	HDC hicTarget,		//@parm DC for information
	DWORD dwMode,		//@parm Type of natural size required
	LONG *pwidth,		//@parm Width in device units to use for fitting 
	LONG *pheight)		//@parm Height in device units to use for fitting
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetNaturalSize");

	HRESULT hr = S_OK;

	// Set the height temporarily so the zoom factor will work out
	LONG yOrigHeightClient = SetClientHeight(*pheight);

	// Adjust the height and width by the view inset
	LONG widthView = *pwidth;
	LONG heightView = *pheight;
	GetViewDim(widthView, heightView);

	// Store the adjustment so we can restore it to the height & width
	LONG widthAdj =	*pwidth - widthView;
	LONG heightAdj = *pheight - heightView;
 	
	// Init measurer at cp = 0
	CMeasurer me(this);
	CLine liNew;
	LONG xWidth = 0, lineWidth;
	LONG yHeight = 0;
   	LONG cchText = (LONG)_ped->GetTextLength();
	BOOL fFirstInPara = TRUE;

	LONG xWidthMax = GetWordWrap() ? widthView : -1;
	LONG yHeightMax = heightView;

	// The following loop generates new lines
	do 
	{
		// stuff text into new line
		UINT uiFlags = 0;

		// if word wrap is turned on, then we want to break on
		// words, otherwise, measure white space, etc.		
		if( GetWordWrap() )
		{
			uiFlags =  MEASURE_BREAKATWORD;
		}

		if( fFirstInPara )
		{
			uiFlags |= MEASURE_FIRSTINPARA;
		}
	
		me.NewLine(fFirstInPara);

		if(!me.MeasureLine (xWidthMax, -1, uiFlags))
		{
			hr = E_FAIL;
			goto Exit;
		}

		// Copy the data to get it our of the measurer
		liNew = me;

		fFirstInPara = liNew._cchEOP;

		// Keep track of width of widest line
		lineWidth = liNew._xWidth + liNew._xLineOverhang;
		xWidth = max(xWidth, lineWidth);

		// Bump height
		yHeight += liNew._yHeight;
	} while (me.GetCp() < cchText);

	// Add one the caret size to the width to guranatee that the
	// text will fix. We don't want to word break because the caret
	// won't fit when the caller tries a window this size.
	xWidth += dxCaret;

	*pwidth = xWidth;
	*pheight = yHeight;

	// Restore insets so the output reflects the true client rect needed.
	*pwidth += widthAdj;
	*pheight += heightAdj;
		
Exit:
	SetClientHeight(yOrigHeightClient);
	return hr;
}

/*
 *	CDisplayML::Clone()
 *
 *	@mfunc
 *		Make a copy of this object
 *
 *	@rdesc
 *		NULL - failed
 *		CDisplay *
 *
 *	@devnote
 *		Caller of this routine is the owner of the new display object.
 */
CDisplay *CDisplayML::Clone() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::Clone");

	CDisplayML *pdp = new CDisplayML(_ped);

	if (pdp != NULL)
	{
		// Initialize our base class
		if (!pdp->CDisplay::Init())
		{
			pdp->InitFromDisplay(this);
			pdp->_xScroll = _xScroll;
			pdp->_fVScrollEnabled = _fVScrollEnabled;
			pdp->_fHScrollEnabled = _fHScrollEnabled;
			pdp->_fWordWrap = _fWordWrap;
			pdp->_cpFirstVisible = _cpFirstVisible;
			pdp->_iliFirstVisible = _iliFirstVisible;
			pdp->_yScroll = _yScroll;
			pdp->ResetDrawInfo(this);

			if (_pddTarget != NULL)
			{
				// Create a duplicate target device for this object
				pdp->SetMainTargetDC(_pddTarget->GetDC(), _xWidthMax);
			}

			// This can't be the active view since it is a clone
			// of some view.
			pdp->SetActiveFlag(FALSE);
		}
	}
	return pdp;
}

void CDisplayML::DeferUpdateScrollBar()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::DeferUpdateScrollBar");

	_fDeferUpdateScrollBar = TRUE;
	_fUpdateScrollBarDeferred = FALSE;
}

BOOL CDisplayML::DoDeferredUpdateScrollBar()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::DoDeferredUpdateScrollBar");

	_fDeferUpdateScrollBar = FALSE;
	if(!_fUpdateScrollBarDeferred)
		return FALSE;

	_fUpdateScrollBarDeferred = FALSE;
    
	return UpdateScrollBar(SB_HORZ, TRUE) || UpdateScrollBar(SB_VERT, TRUE);
}

LONG CDisplayML::GetMaxPixelWidth(void) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetMaxPixelWidth");

	return _xWidthMax ? LXtoDX(_xWidthMax) : GetViewWidth();
}

/*
 *	CDisplayML::GetMaxXScroll()
 *
 *	@mfunc
 *		Get the maximum x scroll value
 *
 *	@rdesc
 *		Maximum x scroll value
 *
 */
LONG CDisplayML::GetMaxXScroll() const
{
	return _xWidth + dxCaret;
}

/*
 *	CDisplayML::CreateEmptyLine()
 *
 *	@mfunc
 *		Create an empty line
 *
 *	@rdesc
 *		TRUE - worked <nl>
 *		FALSE - failed
 *
 */
BOOL CDisplayML::CreateEmptyLine()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CreateEmptyLine");

	// Make sure that this is being called appropriately
	AssertSz(_ped->GetTextLength() == 0,
		"CDisplayML::CreateEmptyLine called inappropriately");

	// Assume failure
	BOOL fResult = FALSE;

	// Add one new line
	CLine *pliNew = Add(1, NULL);

	// Create a measurer
	CMeasurer me(this);

	if (!pliNew)
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		TRACEWARNSZ("CDisplayML::CreateEmptyLine unable to add CLine to CLineArray");
		goto err;
	}

	// Measure the empty line
	if(!pliNew->Measure(me, -1, MEASURE_BREAKATWORD | MEASURE_FIRSTINPARA))
	{
		Assert(FALSE);
		goto err;
	}

	// If we made it to here, everything worked.
	fResult = TRUE;

err:
	return fResult;
}

/*
 *	CDisplayML::AdjustToDisplayLastLine()
 *
 *	@mfunc
 *		Calculate the yscroll necessary to get the last line to display
 *
 *	@rdesc
 *		Updated yScroll
 *
 */
LONG CDisplayML::AdjustToDisplayLastLine(
	LONG yBase,			//@parm actual yScroll to display
	LONG yScroll)		//@parm proposed amount to scroll
{
	LONG iliFirst;
	LONG yFirst;

	if (yBase >= _yHeight)
	{
		// Want last line to be entirely displayed.

		// Compute new first visible line
		iliFirst = LineFromYpos(yScroll, &yFirst, NULL);

		// Is the top line partial?
		if (yScroll != yFirst)
		{
			// Yes - bump scroll to the next line so the ScrollView
			// won't bump the scroll back to display the entire 
			// partial line since we want the bottom to display.
			yScroll = YposFromLine(iliFirst + 1);
		}
	}
	return yScroll;
}

/*
 *	CDisplayML::GetResizeHeight()
 *
 *	@mfunc
 *		Calculates height to return for a request resize
 *
 *	@rdesc
 *		Updated yScroll
 */
LONG CDisplayML::GetResizeHeight() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::GetResizeHeight");

    return CalcScrollHeight(_yHeight);
}



// ================================  DEBUG methods  ============================================

#ifdef DEBUG
/*
 *	CDisplayML::CheckLineArray()
 *
 *	@mfunc	
 *		Check that sigma (*this)[]._cch == _ped->GetTextLength()
 *		and sigma (*this)[]._yHeight == _yHeight
 */
void CDisplayML::CheckLineArray()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CheckLineArray");

	LONG ili = Count();

	// If we are marked as needing a recalc or if we are in the process of a
	// background recalc, we cannot verify the line array 
	if(!_fRecalcDone || _fNeedRecalc || !ili)
		return;

	BOOL fErr = FALSE;
	LONG cchSum = 0;
	LONG yHeight = 0;
	CLine const *pli = Elem(0);
	BOOL fPrevLineEOP = TRUE;

	while(ili--)
	{
		AssertSz(!fPrevLineEOP || (pli->_bFlags & fliFirstInPara),
			"CheckLineArray Invalid first/prev flags");
		AssertSz(0 != pli->_cch, "CheckLineArray cch == 0");

		yHeight += pli->GetHeight();

		cchSum += pli->_cch;

		fPrevLineEOP = FALSE;

		if (pli->_cchEOP)
			fPrevLineEOP = TRUE;

		pli++;
	}

	if(cchSum != (LONG)_ped->GetTextLength())
	{
		Tracef(TRCSEVINFO, "sigma (*this)[]._cch = %ld, cchText = %ld", cchSum, _ped->GetTextLength());
		AssertSz(FALSE, "sigma (*this)[]._cch != cchText");
	}

	if(yHeight != _yHeight)
	{
		Tracef(TRCSEVINFO, "sigma (*this)[]._yHeight = %ld, _yHeight = %ld", yHeight, _yHeight);
		AssertSz(FALSE, "sigma (*this)[]._yHeight != _yHeight");
	}
}

void CDisplayML::DumpLines(
	LONG iliFirst,
	LONG cli)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::DumpLines");

	LONG cch;
	LONG ili;
	TCHAR rgch[512];

	if(Count() == 1)
		wcscpy_s(rgch, TEXT("1 line"));
	else
		wsprintf(rgch, TEXT("%d lines"), Count());
	
#ifdef UNICODE
    // TraceTag needs to take UNICODE...
#else
	TRACEINFOSZ(TRCSEVINFO, rgch);
#endif

	if(cli < 0)
		cli = Count();
	else
		cli = min(cli, (LONG)Count());
	if(iliFirst < 0)
		iliFirst = Count() - cli;
	else
		cli = min(cli, (LONG)Count() - iliFirst);

	for(ili = iliFirst; cli > 0; ili++, cli--)
	{
		const CLine * const pli = Elem(ili);

		wsprintf(rgch, TEXT("Line %d (%ldc%ldw%ldh%x): \""), ili, pli->_cch, 
			pli->_xWidth + pli->_xLineOverhang, pli->_yHeight, pli->_bFlags);
		cch = (LONG)wcslen(rgch);
		cch += GetLineText(ili, rgch + cch, CchOfCb(sizeof(rgch)) - cch - 4);
		rgch[cch++] = TEXT('\"');
		rgch[cch] = TEXT('\0');
#ifdef UNICODE
        // TraceTag needs to take UNICODE...
#else
    	TRACEINFOSZ(TRCSEVINFO, rgch);
#endif
	}
}

/*
 *	CDisplayML::CheckView()
 *
 *	@mfunc	
 *		Checks coherence between _iliFirstVisible, _cpFirstVisible 
 *      and _dyFirstVisible
 */
void CDisplayML::CheckView()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayML::CheckView");

	LONG yHeight;
	CLinePtr rp(this);

	VerifyFirstVisible();

	rp = _iliFirstVisible;

	for(yHeight = 0; rp--; yHeight += rp->GetHeight())
		;

	if(yHeight != _yScroll + _dyFirstVisible)
	{
		Tracef(TRCSEVINFO, "sigma CLine._yHeight = %ld, CDisplay.yFirstLine = %ld", yHeight, _yScroll + _dyFirstVisible);
		AssertSz(FALSE, "CLine._yHeight != VIEW.yFirstLine");
	}
}

/*
 *	CDisplayML::VerifyFirstVisible()
 *
 *	@mfunc	checks the coherence between _iliFirstVisible and _cpFirstVisible
 *
 *	@rdesc	TRUE if things are hunky dory; FALSE otherwise
 */
BOOL CDisplayML::VerifyFirstVisible()
{
	CLinePtr rp(this);
	LONG	 cchSum;

	rp = _iliFirstVisible;
	for(cchSum = 0; rp--; cchSum += rp->_cch)
	{
		if( rp )
		{
			AssertSz( !(rp[-1]._cchEOP) == !(rp->_bFlags & fliFirstInPara), 
				 "fHasEOL & fFirstInPara disagree");
		}
	}
	if(cchSum != _cpFirstVisible)
	{
		Tracef(TRCSEVINFO, "sigma CLine._cch = %ld, CDisplay.cpFirstVisible = %ld", cchSum, _cpMin);
		AssertSz(FALSE, "sigma CLine._cch != VIEW.cpMin");

		return FALSE;
	}
	return TRUE;
}

#endif // DEBUG



