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
 *	@doc
 *
 *	@module - MEASURE.C	  |
 *	
 *		CMeasurer class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini, Murray Sargent, Rick Sailor
 *
 */

#include "_common.h"
#include "_measure.h"
#include "_font.h"
#include "_disp.h"
#include "_edit.h"
#include "_frunptr.h"
#include "_objmgr.h"
#include "_coleobj.h"

ASSERTDATA

// Default character format for a bullet
const CHARFORMAT cfBullet = 
{
	sizeof(CHARFORMAT),
    0, 
	CFE_AUTOCOLOR + CFE_AUTOBACKCOLOR, 
	0, 
	0, 
	0,
    SYMBOL_CHARSET, 
	(BYTE) FF_DONTCARE, 
	TEXT("Symbol")
};

const TCHAR chBullet = TEXT('\xB7');
// Note we set this maximum length as appropriate for Win95 since Win95 GDI can 
// only handle 16 bit values. We don't special case this so that both NT and
// Win95 will behave the same way. 
// Note that the following obscure constant was empirically determined on Win95.
const LONG lMaximumWidth = (3 * (LONG) SHRT_MAX) / 4;

CMeasurer::CMeasurer (const CDisplay* const pdp) :
	CRchTxtPtr (pdp->GetED())	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::CMeasurer");

	_pdp = pdp;
	_pdd = pdp;
	_pccs = NULL;
	_hdc = NULL;
	_hdcMeasure = NULL;
	_chPassword = GetPed()->TxGetPasswordChar();
	SetNumber(0);
}

CMeasurer::CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &tp) :
	CRchTxtPtr (tp)	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::CMeasurer");

	_pdp = pdp;
	_pdd = pdp;
	_pccs = NULL;
	_hdc = NULL;
	_hdcMeasure = NULL;
	_chPassword = GetPed()->TxGetPasswordChar();
	SetNumber(0);
}

CMeasurer::~CMeasurer()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::~CMeasurer");

	if(_pccs)
		_pccs->Release();

	// Releases the rendering dc
	if (_hdc)
	{
		_pdd->ReleaseDC(_hdc);
	}

	// Releases the measuring dc
	if (_hdcMeasure)
	{
		_pdd->ReleaseMeasureDC(_hdcMeasure);
	}
}

/*
 *	CMeasurer::NewLine (fFirstInPara)
 *
 *	@mfunc
 *		Initialize this measurer at the start of a new line
 */
void CMeasurer::NewLine(
	BOOL fFirstInPara)		//@parm Flag for setting up _bFlags
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	CLine::Init();						// Zero all members
	if(fFirstInPara)
		_bFlags = fliFirstInPara;		// Need to know if first in para
}

/*
 *	CMeasurer::NewLine(&li)
 *
 *	@mfunc
 *		Initialize this measurer at the start of a given line
 */
void CMeasurer::NewLine(
	const CLine &li)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::NewLine");

	*this		= li;
	_cch		= 0;
	_cchWhite	= 0;
	_xWidth		= 0;

	// Can't calculate xLeft till we get an HDC
	_xLeft		= 0;
	if(!GetPF()->IsListNumbered())
		_bNumber = 0;

	_wNumber	= _bNumber;
}

/*
 *	CMeasurer::MaxWidth()
 *
 *	@mfunc
 *		Get maximum width for line
 *
 *	@rdesc
 *		Maximum width for a line
 */
LONG CMeasurer::MaxWidth()
{
	LONG xWidth = lMaximumWidth;

	if (_pdp->GetWordWrap() && !GetPF()->InTable())
	{
		// There is a caret only on the main display
		LONG xCaret = _pdp->IsMain() ? dxCaret : 0;

		// Calculate the width of the display
		LONG xDispWidth = _pdp->GetMaxPixelWidth();

		if (!_pdp->SameDevice(_pdd))
		{
			// xWidthMax is calculated to the size of the screen DC. If
			// there is a target device with different characteristics
			// we need to convert the width to the target device's width
			xDispWidth = _pdd->ConvertXToDev(xDispWidth, _pdp);
		}
		xWidth = xDispWidth - MeasureRightIndent() - _xLeft - xCaret;
	}
	return (xWidth > 0) ? xWidth : 0;
}

/*
 *	CMeasurer::MeasureText (cch)
 *
 *	@mfunc
 *		Measure a stretch of text from current running position.
 *
 *	@rdesc
 *		width of text (in device units), < 0 if failed
 */
LONG CMeasurer::MeasureText(
	LONG cch)		//@parm Number of characters to measure
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureText");

	LONG xWidth = _xWidth;
	
	if (Measure (0x7fffffff, cch, 0) == MRET_FAILED)
		return -1;

	LONG xMaxWidth = MaxWidth();

	return min(_xWidth - xWidth, xMaxWidth);
}

/*
 *	CMeasurer::MeasureLine (xWidthMax, cchMax, uiFlags, ppu, pliTarget)
 *
 *	@mfunc
 *		Measure a line of text from current cp and determine line break.
 *		On return *this contains line metrics for _pdd device.
 *
 *	@rdesc
 *		TRUE if success, FALSE if failed
 */
BOOL CMeasurer::MeasureLine(
	LONG xWidthMax,		//@parm max width to process (-1 uses CDisplay width)
	LONG cchMax, 		//@parm Max chars to process (-1 if no limit)
	UINT uiFlags,  		//@parm Flags controlling the process (see Measure())
	CLine *pliTarget)	//@parm Returns target-device line metrics (optional)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLine");

	LONG lRet;
	LONG cp = GetCp();
	const CDevDesc *pddTarget = NULL;

	if (_pdp->GetWordWrap())
	{
		// Target devices are only interesting if word wrap is on because the 
		// only really interesting thing a target device can tell us is where
		// the word breaks will occur.
		pddTarget = _pdp->GetTargetDev();

		if (pddTarget != NULL)
		{
			// If there is a target device, use that device to compute line 
			// breaks. This is followed with a recompute to get the actual 
			// measurements on the rendering device.
			// ... but first releases the measuring dc
			if (_hdcMeasure)
				_pdd->ReleaseMeasureDC(_hdcMeasure);
			_hdcMeasure = pddTarget->GetDC();
			_yMeasurePerInch = GetDeviceCaps(_hdcMeasure, LOGPIXELSY);
			_pdd = pddTarget;

			if (_pccs != NULL)
			{
				_pccs->Release();
				_pccs = NULL;
			}
		}
	}

	// Compute line break
	lRet = Measure(xWidthMax, cchMax, uiFlags);

	// Stop here if failed
	if(lRet == MRET_FAILED)
		return FALSE;

	// Return target metrics if requested
	if(pliTarget)
	{
		*pliTarget = *this;
	}

	if (pddTarget)
	{
		// First computation was with the target device so we need to recompute
		// with the rendering device. Here we set the device to measure with
		// back to the render device.
		// ... first releases the current measuring dc
		if (_hdcMeasure)
			_pdd->ReleaseMeasureDC(_hdcMeasure);
		_pdd = _pdp;
		_hdcMeasure = _pdp->GetMeasureDC(&_yMeasurePerInch);

		if (_pccs != NULL)
		{
			_pccs->Release();
			_pccs = NULL;
		}

		// We just use this flag as an easy way to get the recomputation to occur.
		lRet = MRET_NOWIDTH;
	}

	// Recompute metrics on rendering device
	if(lRet == MRET_NOWIDTH)
	{
		LONG cch = _cch;
		SHORT cchWhite = _cchWhite;

		// Save the flags for this line because at least the EOP flag gets 
		// screwed up in the recalc and none of the flags will change based
		// on the second recalc of the line.  Ditto for numbering variables
		BYTE bFlags  = _bFlags;
		BYTE bNumber = _bNumber;
		BYTE cchEOP  = _cchEOP;
		WORD wNumber = _wNumber;

#ifdef PWD_JUPITER
        // GuyBark Jupiter 31112: Store the current line height here.
        int yHeightOld = _yHeight;
#endif // PWD_JUPITER

		Advance(-cch);				// move back to BOL

		NewLine(uiFlags & MEASURE_FIRSTINPARA);

#ifdef PWD_JUPITER
        // GuyBark Jupiter 31112: Say the line consists only of an EOP mark.
        // This means Measure() will end up giving the line a default height.
        // This is incorrect as the line should have the height of the EOP.
        // So give the line the it's previous value here as a starting point.
        // Measure() can set it to something else later if it wants to.
        if(cch == cchWhite)
        {
            _yHeight = yHeightOld;
        }
#endif // PWD_JUPITER
        
		// (AndreiB) EOP flag would not get reset below because we excluded it 
		// from the second measure ( - cchWhite) By restoring it right here
		// we ensure we know whether the line is at the end of para or not
		// right from the beginning. This may seem somewhat hacky but it works.
		_cchEOP = cchEOP;

#ifdef PWD_JUPITER
        // GuyBark Jupiter 34903: 
        // I don't really know how this can happen, but the paragraph (_pPF)
        // can be numbered here, but the measured details don't realize it.
        // As a failsafe, make sure the measured details reflect the true
        // paragraph attributes.

        if(!_bNumber &&             // The measure details think this isn't numbered.
            _pPF->wNumbering != 0)  // But the paragraph details says it is numbered!
        {
            // Make all the measure numbering details current.
            _bNumber = _pPF->wNumbering;
            _wNumber = _pPF->wNumbering;
        }
#endif // PWD_JUPITER

   		lRet = Measure(0x7fffffff, cch - cchWhite, uiFlags & MEASURE_FIRSTINPARA);

		if(lRet != 0)
		{
			Assert(lRet != MRET_NOWIDTH);
			return FALSE;
		}

		// Restore the flags and numbering variables
		_bFlags  = bFlags;
		_bNumber = bNumber;
		_cchEOP  = cchEOP;
		_wNumber = wNumber;

		Assert((LONG)_cch == cch - cchWhite);
		_cchWhite = cchWhite;
		_cch = cch;				// account for the white stuff at EOL
		Advance(cchWhite);		// skip white stuff that we did not remeasure
	}
	
	// Now that we know the line width, compute line shift due
	// to alignment, and add it to the left position 
	_xLeft += MeasureLineShift();

	return TRUE;
}

/*
 *	CMeasurer::RecalcLineHeight ()
 *
 *	@mfunc
 *	  Reset the height of the the line we are measuring if the new run of 
 *	  text is taller than the current maximum in the line.
 */
void CMeasurer::RecalcLineHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::RecalcLineHeight");

	// Compute line height
	LONG	yOffset = _pccs->_yOffset;
	SHORT	yHeight = _pccs->_yHeight;
	SHORT 	yDescent = _pccs->_yDescent;
	SHORT	yAscent = yHeight - yDescent;
	SHORT	yAboveBase;
	SHORT	yBelowBase;

	yAboveBase = max(yAscent, yAscent + yOffset);
	yBelowBase = max(yDescent, yDescent - yOffset);
	_yHeight = max(yAboveBase, _yHeight - _yDescent) +
		       max(yBelowBase, _yDescent);
	_yDescent = max(yBelowBase, _yDescent);
}

/*
 *	CMeasurer::Measure (xWidthMax, cchMax, uiFlags)
 *
 *	@mfunc
 *		Measure given amount of text, start at current running position
 *		and storing # chars measured in _cch. 
 *		Can optionally determine line break based on a xWidthMax and 
 *		break out at that point.
 *
 *	@rdesc
 *		0 success
 *		MRET_FAILED	 if failed 
 *		MRET_NOWIDTH if second pass is needed to compute correct width
 *
 *	@devnote
 *		The uiFlags parameter has the following meanings:
 *			MEASURE_FIRSTINPARA		this is first line of paragraph
 *			MEASURE_BREAKATWORD		break out on a word break
 *			MEASURE_BREAKATWIDTH	break closest possible to xWidthMax
 */
LONG CMeasurer::Measure(
	LONG xWidthMax,			//@parm Max width of line (-1 uses CDisplay width)
	LONG cchMax,			//@parm Max chars to process (-1 if no limit)
	UINT uiFlags)			//@parm Flags controlling the process (see above)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::Measure");

	LONG		cch;				// cchChunk count down
	LONG		cchChunk;			// cch of cst-format contiguous run
	LONG		cchNonWhite;		// cch of last nonwhite char in line
	LONG		cchText = GetTextLength();
	unsigned	ch;					// Temporary char
	BOOL		fFirstInPara = uiFlags & MEASURE_FIRSTINPARA;
	BOOL        fLastChObj = FALSE;
	LONG		lRet = 0;
	const TCHAR*pch;
	CTxtEdit *	ped = _pdp->GetED();
	COleObject *pobj;
	LONG		objheight;
	LONG		yHeightBullet = 0;
	LONG		xCaret = dxCaret;
	LONG		xAdd;				// Character width
#ifdef SOFTHYPHEN
	LONG		xSoftHyphen = 0;	// Most recent soft hyphen width
#endif
	LONG		xWidthNonWhite;		// xWidth for last nonwhite char in line
	LONG		xWidthMaxOverhang;	// The max with the current run's overhang
									//  taken into consideration.
	// These two variables are used to keep track of whether there is a height change
	// so that we know whether we need to recalc the line in certain line break cases.
	BOOL		fHeightChange = FALSE;
	LONG		yHeightOld = 0;

	const INT	MAX_SAVED_WIDTHS = 31;	// power of 2 - 1
	INT			i, index, iSavedWidths = 0;
	struct {
		LONG	width;
		LONG	xLineOverhang;
		LONG	yHeight;
		LONG	yDescent;
	} savedWidths[MAX_SAVED_WIDTHS+1];

	_pPF = GetPF();							// Be sure current CParaFormat
											//  ptr is up to date
	BOOL fInTable = _pPF->InTable();
	WORD wNumbering		 = _pPF->wNumbering;
	WORD wNumberingStyle = _pPF->wNumberingStyle;

	if(IsInOutlineView())
	{
		if(IsHeadingStyle(_pPF->sStyle))	// Store heading number if relevant
			_nHeading = -_pPF->sStyle - 1;

		if(_pPF->wEffects & PFE_COLLAPSED)	// Cache collapsed bit
			_fCollapsed = TRUE;
	}

	if(!_hdcMeasure)
	{
		_hdcMeasure = _pdd->GetMeasureDC(&_yMeasurePerInch);
		if(!_hdcMeasure)
		{
			AssertSz(FALSE, "CMeasurer::Measure could not get DC");
			return MRET_FAILED;
		}
	}

	// Init fliFirstInPara flag for new line
	if(fFirstInPara)
	{
		_bFlags |= fliFirstInPara;
		if(fInTable)
			_bFlags |= fliUseOffScreenDC;

		// Because measure bullet depends on the cp we need
		// to measure it before we do the reset of our measuring
		if(wNumbering)	
#ifdef PWD_JUPITER // GuyBark Jupiter 34728: Pay attention to the returned bullet height!
			yHeightBullet = MeasureBulletHeight();
#else
			MeasureBulletHeight();
#endif // PWD_JUPITER

		else if(IsInOutlineView() && IsHeadingStyle(_pPF->sStyle))
			yHeightBullet = BITMAP_HEIGHT_HEADING + 1;
	}

	// If line spacing or space before/after, measure from beginning of line
	if(_cch && (_pPF->bLineSpacingRule || _pPF->dySpaceBefore
		|| _pPF->dySpaceAfter || fInTable))					
	{										
		 Advance(-(LONG)_cch);
		 NewLine(fFirstInPara);
	}

	_bNumber = _wNumber < 256 ? _wNumber : 255;	// Store current para # offset
	_xLeft = MeasureLeftIndent();				// Set left indent

	// Compute width to break out at
	if(xWidthMax < 0)
	{					
		xWidthMax = MaxWidth();

		// MaxWidth includes the caret size.
		xCaret = 0;
	}
	else
	{							  
		// (AndreiB) xWidthMax that's coming down to us is always calculated
		// with respect to the screen DC. The only scenario it comes into play
		// however is in TxGetNaturalSize, which may output a slightly
		// different result because of that.
		if (!_pdp->SameDevice(_pdd))
		{
			// xWidthMax is calculated to the size of the screen DC. If
			// there is a target device with different characteristics
			// we need to convert the width to the target device's width
			xWidthMax = _pdd->ConvertXToDev(xWidthMax, _pdp);
		}
	}

	// For overhang support, we test against this adjusted widthMax.
	xWidthMaxOverhang = xWidthMax;

	// Are we ignoring the offset of the characters for the measure?
	if ((uiFlags & MEASURE_IGNOREOFFSET) == 0)
	{
		// No - then take it from the max
		xWidthMaxOverhang -= (_xLineOverhang + xCaret);
	}

	// Compute max number of characters to process
	cch = cchText - GetCp();
	if(cchMax < 0 || cchMax > cch)
		cchMax = cch;

	cchNonWhite		= _cch;							// Default nonwhite parms
	xWidthNonWhite	= _xWidth;
	for( ; cchMax > 0;								// Measure up to cchMax
		cchMax -= cchChunk, Advance(cchChunk))		//  chars
	{												
		pch = GetPch(cch);
		cch = min(cch, cchMax);						// Compute constant-format
		cchChunk = GetCchLeftRunCF();
		cch = min(cch, cchChunk);					// Counter for next while
		cchChunk = cch;								// Save chunk size

		if(GetCF()->dwEffects & CFE_HIDDEN)			// Ignore hidden text
		{
			_cch += cchChunk;
			continue;
		}

		if (_chPassword != 0)
		{
			// WARNING: pch cannot be incremented after this point.
			// Be sure that any new code does not modify pch after
			// it is set here when we deal with passwords.
			pch = &_chPassword;
		}

		// Check if new character format run or whether we don't yet have
		// a font.
		if ( !_pccs || FormatIsChanged() )
		{
			// New CF run or format for this line not yet initialized
			ResetCachediFormat();

			// If the format has changed,we release our old Format cache
			if( _pccs != NULL )
			{
				// Release previous char cache if this is a new CF run
				_pccs->Release();
			}
				
			_pccs = fc().GetCcs(_hdcMeasure, GetCF(), 
								_pdp->GetZoomNumerator(),
								_pdp->GetZoomDenominator(), 
								_yMeasurePerInch);

			// If we couldn't get one, we are dead.
			if(!_pccs)
			{
				// When zooming, we may fail in low mem situations in which case
				// the text won't display. If there was a zoom factor try
				// getting the character metrics without the zoom factor.
				if (_pdp->GetZoomNumerator() != _pdp->GetZoomDenominator()) {
					_pccs = fc().GetCcs(_hdcMeasure, GetCF(), 
										1, 1, _yMeasurePerInch);
				}
				if (!_pccs) {
					AssertSz(FALSE, "CMeasurer::Measure could not get _pccs");
					return MRET_FAILED;
				}
			}

			if (_cch && (0 == GetIchRunCF()))
			{
				// This is not the first character in the line,
				// therefore there are multiple character formats
				// on the line so we want to remember to render
				// this off screen.
				_bFlags |= fliUseOffScreenDC;
			}
		}

		// NOTE: Drawing with a dotted pen on the screen and in a
		// compatible bit map does not seem to match on some hardware.
		// If at some future point we do a better job of drawing the
		// dotted underline, this statement block can be removed.
		if (CFU_UNDERLINEDOTTED == _pccs->_bUnderlineType)
		{
			// We draw all dotted underline lines off screen to get
			// a consistent display of the dotted line.
			_bFlags |= fliUseOffScreenDC;
		}

		_xLineOverhang = _pccs->_xOverhang;

		xWidthMaxOverhang = xWidthMax;			// Overhang reduces max.

		// Are we ignoring the offset of the characters for the measure?
		if ((uiFlags & MEASURE_IGNOREOFFSET) == 0)
		{
			// No - then take it from the max
			xWidthMaxOverhang -= (_pccs->_xOverhang + xCaret);
		}

		// Adjust line height for new format run
		if(cch > 0 && *pch)
		{
			// Note: the EOP only contributes to the height calculation for the
			// line if there are no non-white space characters on the line or 
			// the paragraph is a bullet paragraph. The bullet paragraph 
			// contribution to the line height is done in AdjustLineHeight.
			if ((0 == cchNonWhite) || (*pch != CR && *pch != LF))
			{
				// Determine if the current run is the tallest text on this
				// line and if so, increase the height of the line.
				yHeightOld = _yHeight;
				RecalcLineHeight();

				// Test for a change in line height. This only happens when
				// this is not the first character in the line and (surprise)
				// the height changes.
				if (yHeightOld && yHeightOld != _yHeight)
					fHeightChange = TRUE;
			}
		}

		while(cch > 0)
		{											// Process next char
			xAdd = 0;								// Default zero width
			AssertSz(_chPassword == 0 || pch == &_chPassword,
				"CMeasurer::Measure pch set to invalid data");
			ch = *pch;	  

#ifdef PWD_JUPITER
			// GuyBark Jupiter: If this is the special character for preserving
			// CRLF in table cells, measure it as if it were a space.
			if(ch == 0xFFFA)
			{
			    ch = ' ';
			}
#endif // PWD_JUPITER

			if(ch == WCH_EMBEDDING)
			{
				_bFlags |= fliHasOle;
				pobj = ped->GetObjectMgr()->GetObjectFromCp
								(GetCp() + cchChunk - cch);
				if( pobj )
				{
					pobj->MeasureObj(_pdp, xAdd, objheight, _yDescent);

					// Only update height for line if the object is going
					// to be on this line.
					if((!_cch || _xWidth + xAdd <= xWidthMaxOverhang) 
						&& objheight > _yHeight)
					{
						_yHeight = (short)objheight;
					}
				}
				if(_xWidth + xAdd > xWidthMaxOverhang)
					fLastChObj = TRUE;
			}
			// The following if succeeds if ch isn't a CELL, BS, TAB, LF,
			// VT, FF, or CR
			else if(!IN_RANGE(CELL, ch, CR))		// Not TAB or EOP
			{										
				if(!_pccs->Include(ch, xAdd))		// Get char width	
				{
					AssertSz(FALSE, "CMeasurer::Measure char not in font");
					return MRET_FAILED;
				}
#ifdef SOFTHYPHEN
				if(ch == SOFTHYPHEN)
				{
					_bFlags |= fliHasTabs;			// Setup RenderChunk()
					if(_xWidth + xAdd < xWidthMaxOverhang || !_cch)
					{
						xSoftHyphen = xAdd;			// Save soft hyphen width
						xAdd = 0;					// Use 0 unless at EOL
					}
				}
#endif
			}
			else if(ch == TAB || ch == CELL)		
			{
				_bFlags |= fliHasTabs;
				xAdd = MeasureTab(ch);
			}
			else									// Done with line
				goto eop;							// Go process EOP chars

			index = iSavedWidths++ & MAX_SAVED_WIDTHS;
			savedWidths[ index ].width = xAdd;
			savedWidths[ index ].xLineOverhang = _xLineOverhang;
			savedWidths[ index ].yHeight = _yHeight;
			savedWidths[ index ].yDescent = _yDescent;
			_xWidth += xAdd;

			if(_xWidth > xWidthMaxOverhang && _cch > 0)
				goto overflow;

			_cch++;
			if (_chPassword == 0)
			{
				pch++;
			}
			cch--;
			if((ped->IsIMEComposition() && 0 == cch) || ch != TEXT(' ') /*&& ch != TAB*/)	// If not whitespace char,
			{
				cchNonWhite		= _cch;				//  update nonwhitespace
				xWidthNonWhite	= _xWidth;			//  count and width
			}
		}											// while(cch > 0)
	}												// for(;cchMax > 0;...)
	goto eol;										// All text exhausted 


// End Of Paragraph	char encountered (CR, LF, VT, or FF, but mostly CR)
eop:
	Advance(cchChunk - cch);				// Position tp at EOP
	cch = AdvanceCRLF();					// Bypass possibly multibyte EOP
	_cchEOP = cch;							// Store EOP cch
	_cch   += cch;							// Increment line count
											
	if(ch == CR)							// Get new para number
	{
		const CParaFormat *pPF = GetPF();
		_wNumber = pPF->UpdateNumber(_wNumber, _pPF);
		_fNextInTable = pPF->InTable() && GetCp() < cchText;
	}

	AssertSz(ped->Get10Mode() || cch == 1,
		"CMeasurer::Measure: EOP isn't a single char");
	AssertSz(ped->TxGetMultiLine() || GetCp() == cchText,
		"CMeasurer::Measure: EOP in single-line control");

eol:										// End of current line
	if(uiFlags & MEASURE_BREAKATWORD)		// Compute count of whitespace
	{										//  chars at EOL
		_cchWhite = (SHORT)(_cch - cchNonWhite);
		_xWidth = xWidthNonWhite;
	}
	goto done;


overflow:									// Went past max width for line
	_xWidth -= xAdd;
	--iSavedWidths;
	_xLineOverhang = savedWidths[iSavedWidths & MAX_SAVED_WIDTHS].xLineOverhang;
	Advance(cchChunk - cch);				// Position *this at overflow
											//  position
	if(uiFlags & MEASURE_BREAKATWORD)		// If required, adjust break on
	{										//  word boundary
		// We should not have the EOP flag set here.  The case to watch out
		// for is when we reuse a line that used to have an EOP.  It is the
		// responsibility of the measurer to clear this flag as appropriate.
	
		Assert(_cchEOP == 0);

		_cchEOP = 0;							// Just in case

		if (ch == TAB || ch == CELL)
		{
			// If the last character measured is a tab,	leave it on the
			// next line to allow tabbing off the end of line as in Word
			goto done;
		}

		LONG cpStop = GetCp();					// Remember current cp

		cch = -FindWordBreak(WB_LEFTBREAK, _cch+1);

		if (cch == 0 && fLastChObj)				// If preceding char is an
		{										//  object,	put current char
			goto done;							//  on next line
		}

		Assert(cch >= 0);
		if(cch + 1 < (LONG)_cch)				// Break char not at BOL
		{
			ch = _rpTX.GetPrevChar();
			if (ch == TAB || ch == CELL)		// If break char is a TAB,
			{									//  put it on the next line
				cch++;							//  as in Word
				Advance(-1);					
			}
#ifdef SOFTHYPHEN
			else if(ch == SOFTHYPHEN)
				_xWidth += xSoftHyphen;
#endif
			_cch -= cch;
		}
		else if(cch == (LONG)_cch && cch > 1 &&
			_rpTX.GetChar() == ' ')				// Blanks all the way back to
		{										//  BOL. Bypass first blank
			Advance(1);
			cch--;
			_cch = 1;
		}
		else									// Advance forward to end of
			SetCp(cpStop);						//  measurement

		Assert(_cch > 0);

		// Now search at start of word to figure how many white chars at EOL
		if(GetCp() < cchText)
		{
			pch = GetPch(cch);
			cch = 0;
			if(ped->_pfnWB((TCHAR *)pch, 0, sizeof(TCHAR), WB_ISDELIMITER))
			{
				cch = FindWordBreak(WB_RIGHT);
				Assert(cch >= 0);
			}

			_cchWhite = (SHORT)cch;
			_cch += cch;

			if(_rpTX.IsAtEOP())					// skip *only* 1 EOP -jOn
			{
				_cchEOP = AdvanceCRLF();
				_cch += _cchEOP;
				goto done;
			}
		}

		i = cpStop - GetCp();
		if( i )
		{
			if ( i > 0 )
				i += _cchWhite;
			if (i > 0 && i < iSavedWidths && i < MAX_SAVED_WIDTHS)
			{
				while (i-- > 0)
				{
					iSavedWidths = (iSavedWidths - 1) & MAX_SAVED_WIDTHS;
					_xWidth -= savedWidths[iSavedWidths].width;
				}
				_xLineOverhang = savedWidths[iSavedWidths].xLineOverhang;
				_yHeight	   = savedWidths[iSavedWidths].yHeight;
				_yDescent	   = savedWidths[iSavedWidths].yDescent;
			}
			else
			{
				// Need to recompute width from scratch.
				_xWidth = -1;
				lRet = MRET_NOWIDTH;
			}
		}
		else
		{
			// i == 0 means that we are breaking on the first letter in a word.
			// Therefore, we want to set the width to the total non-white space
			// calculated so far because that does not include the size of the
			// character that caused the break nor any of the white space 
			// preceeding the character that caused the break.
			if (!fHeightChange)
			{
				_xWidth = xWidthNonWhite;
			}
			else
			{
				// Need to recompute from scratch so that we can get the 
				// correct height for the control
				_xWidth = -1;
				lRet = MRET_NOWIDTH;
			}
		}
	}
	else if(uiFlags & MEASURE_BREAKATWIDTH)
	{
		// Breaks at character closest to target width
		if((_cch == 1) && (xWidthMax < _xWidth / 2))
		{
			_cch = 0;
			_xWidth = 0;
			Advance(-1);
		}
		if(xAdd && xWidthMax - _xWidth >= xAdd / 2)
		{
			_cch++;
			_xWidth += xAdd;
			Advance(1);
		}
	}

done:
	// If no height yet, use default height
	if(_yHeight == 0)
	{
		const CCharFormat *pcf = ped->GetCharFormat(-1);
		CCcs * const pccs = fc().GetCcs(_hdcMeasure, pcf, _pdp->GetZoomNumerator(),
			_pdp->GetZoomDenominator(), _yMeasurePerInch);
		_yHeight = pccs->_yHeight;
		_yDescent = pccs->_yDescent;
		pccs->Release();
	}

	// Allow last minute adjustment to line height
	if (yHeightBullet > _yHeight)
	{
		_yHeight = yHeightBullet;
	}

	_fStartsList = (_bNumber == 1 && fFirstInPara);
	AdjustLineHeight();
	return lRet;
}

/*
 *	CMeasurer::AdjustLineHeight()
 *
 *	@mfunc
 *		Adjust for space before/after and line spacing rules.
 *		No effect for plain text.
 *
 *	@future
 *		Base multiple line height calculations on largest font height rather
 *		than on line height (_yHeight), since the latter may be unduly large
 *		due to embedded objects.  Word does this correctly.
 */
void CMeasurer::AdjustLineHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::AdjustLineHeight");

	if(!IsRich() || IsInOutlineView())			// Plain text and outline mode
		return;									//  don't use special line
												//  spacings
	const CParaFormat * pPF = _pPF;
	DWORD	dwRule	  = pPF->bLineSpacingRule;
	LONG	dyAfter	  = 0;						// Default no space after
	LONG	dyBefore  = 0;						// Default no space before
	LONG	dySpacing = pPF->dyLineSpacing;
	LONG	yHeight	  = LYtoDY(dySpacing);

	if(_bFlags & fliFirstInPara)
		dyBefore = LYtoDY(pPF->dySpaceBefore);	// Space before paragraph

	if(yHeight < 0)								// Negative heights mean use
		_yHeight = -(SHORT)yHeight;				//  the magnitude exactly

	else if(dwRule)								// Line spacing rule is active
	{
		switch (dwRule)
		{
		case tomLineSpace1pt5:
			dyAfter = _yHeight >> 1;			// Half-line space after
			break;								//  (per line)
	
		case tomLineSpaceDouble:
			dyAfter = _yHeight;					// Full-line space after
			break;								//  (per line)
	
		case tomLineSpaceAtLeast:
			if(_yHeight >= yHeight)
				break;
												// Fall thru to space exactly
		case tomLineSpaceExactly:
			_yHeight = (SHORT)yHeight;
			break;
	
		case tomLineSpaceMultiple:				// Multiple-line space after
			dyAfter = (_yHeight*dySpacing)/20	// (20 units per line)
						- _yHeight;
		}
	}

	if(_cchEOP)	
		dyAfter += LYtoDY(pPF->dySpaceAfter);	// Space after paragraph end

	_yHeight  += (SHORT)(dyBefore + dyAfter);	// Add in any space before
	_yDescent += (SHORT)dyAfter;				//  and after

	if(_pPF->InTable())
	{
		_yHeight++;
		if(!_fNextInTable)
		{
			_yHeight++;
			_yDescent++;
		}
	}
}

/*
 *	CMeasurer::MeasureLeftIndent()
 *
 *	@mfunc
 *		Compute and return left indent of line in device units
 *
 *	@rdesc
 *		Left indent of line in device units
 *
 *	@comm
 *		Plain text is sensitive to StartIndent and RightIndent settings,
 *		but usually these are zero for plain text. 
 */
LONG CMeasurer::MeasureLeftIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLeftIndent");

	AssertSz(_pPF != NULL, "CMeasurer::MeasureLeftIndent _pPF not set!");

	LONG xLeft = _pPF->dxStartIndent;				// Use logical units
													//  up to return
	if(IsRich())
	{
		LONG dxOffset = _pPF->dxOffset;
		BOOL fFirstInPara = _bFlags & fliFirstInPara;

		if(IsInOutlineView())
		{
			xLeft = lDefaultTab/2 * (_pPF->bOutlineLevel + 1);
			if(!fFirstInPara)
				dxOffset = 0;
		}
		if(fFirstInPara)
		{
			if(_pPF->wNumbering)						// Add offset to text
			{											//  on first line	 
				LONG dx = DXtoLX(MeasureBulletWidth());	// Use max of bullet
				dx = max(dx, _pPF->wNumberingTab);		//  width, numbering tab,
				dxOffset = max(dxOffset, dx);			//  and para offset
			}
			else if(_pPF->InTable())					// For tables, need to
				xLeft += dxOffset;						//  add in trgaph twice
														//  since dxStartIndent
			else										//  subtracts one
				dxOffset = 0;
		}
		xLeft += dxOffset;								
	}
	return LXtoDX(max(xLeft, 0));
}

/*
 *	CMeasurer::HitTest(x)
 *
 *	@mfunc
 *		Return HITTEST for displacement x in this line. Can't be specific
 *		about text area (_xLeft to _xLeft + _xWidth), since need to measure
 *		to get appropriate cp (done elsewhere)
 *
 *	@rdesc
 *		HITTEST for a displacement x in this line
 */
HITTEST CMeasurer::HitTest(
	LONG x)			//@parm Displacement to test hit
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::HitTest");

	if(x < 0)
		return HT_SelectionBar;

	if(x > _xLeft + _xWidth)
		return HT_RightOfText;

	if(x > _xLeft)								// Caller can refine this
		return HT_Text;							//  with CLine::CchFromXpos()

	if(IsRich() && (_bFlags & fliFirstInPara))
	{
		_pPF = GetPF();
	
		LONG dx;
	
		if(_pPF->wNumbering)
		{
			// Doesn't handle case where Bullet is wider than following dx
			dx = LXtoDX(max(_pPF->dxOffset, _pPF->wNumberingTab));
			if(x >= _xLeft - dx)
				return HT_BulletArea;
		}
		if(IsInOutlineView())
		{
			dx = LXtoDX(lDefaultTab/2 * _pPF->bOutlineLevel);
			if(x >= dx && x < dx + (_pPF->bOutlineLevel & 1
				? LXtoDX(lDefaultTab/2) : _pdp->Zoom(BITMAP_WIDTH_HEADING)))
			{
				return HT_OutlineSymbol;
			}
		}
	}
	return HT_LeftOfText;
}

/*
 *	CMeasurer::MeasureRightIndent()
 *
 *	@mfunc
 *		Compute and return right indent of line in device units
 *
 *	@rdesc
 *		right indent of line in device units
 *
 *	@comm
 *		Plain text is sensitive to StartIndent and RightIndent settings,
 *		but usually these are zero for plain text. 
 */
LONG CMeasurer::MeasureRightIndent()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureRightIndent");

	return LXtoDX(max(_pPF->dxRightIndent, 0));
}

/*
 *	CMeasurer::MeasureTab()
 *
 *	@mfunc
 *		Computes and returns the width from the current position to the
 *		next tab stop (in device units).
 */
LONG CMeasurer::MeasureTab(unsigned ch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureTab");

	LONG			xCur = _xWidth + MeasureLeftIndent();
	const CParaFormat *	pPF = _pPF;
 	LONG			cTab = pPF->cTabCount;
	LONG			dxDefaultTab = lDefaultTab;
	LONG			dxIndent = LXtoDX(pPF->dxStartIndent + pPF->dxOffset);
	LONG			dxOffset = pPF->dxOffset;
	LONG			dxOutline = 0;
	BOOL			fInTable = pPF->InTable();
	LONG			h = 0;
	LONG const *	pl = pPF->rgxTabs;
	LONG			xT;
	LONG			xTab;

	AssertSz(cTab >= 0 || cTab <= MAX_TAB_STOPS,
		"CMeasurer::MeasureTab: illegal tab count");

	if(fInTable)
	{
		h = LXtoDX(dxOffset);
		dxOffset = 0;
	}

	if(IsInOutlineView())
		dxOutline = lDefaultTab/2 * (pPF->bOutlineLevel + 1);

	if(cTab && (!fInTable || ch == CELL))		// Use default TAB for TAB in
	{											//  table
		for(xTab = 0; cTab--; pl++)				// Try explicit tab stops 1st
		{
			xT = GetTabPos(*pl) + dxOutline;	// (2 most significant nibbles
			xT = LXtoDX(xT);					//  are for type/style)

			if(xT > _pdp->GetMaxPixelWidth())	// Ignore tabs wider than
				break;							//  display

			if(xT + h > xCur)					// Allow text in table cell to
			{									//  move into cell gap (h > 0)									
				if(dxOffset > 0 && xT < dxIndent)// Explicit tab in a hanging
					return xT - xCur;			//  indent takes precedence
				xTab = xT;
				break;
			}
		}
		if(dxOffset > 0 &&	xCur < dxIndent)	// If no tab before hanging
			return dxIndent - xCur;				//  indent, tab to indent

		if(xTab)								// Else use tab position
		{
			if(fInTable)
			{
				xTab += h;
				if(cTab)						// Don't include the cell gap
					xTab += h;					//  in the last cell
				if(IsInOutlineView() && cTab < pPF->cTabCount)
					xTab += h;
			}
			return xTab - xCur;
		}

		if(pPF->cTabCount)
			dxDefaultTab = pPF->rgxTabs[0];

		dxDefaultTab = GetPed()->GetDefaultTab();
		if(!dxDefaultTab)
			dxDefaultTab = lDefaultTab;
	
		dxDefaultTab = GetTabPos(dxDefaultTab);
	}

	AssertSz(dxDefaultTab > 0, "Default tab is bad");

	dxDefaultTab = LXtoDX(dxDefaultTab);
	return dxDefaultTab - xCur%dxDefaultTab;	// Round up to nearest
}

/*
 *	CMeasurer::MeasureLineShift ()
 *
 *	@mfunc
 *		Computes and returns the line x shift due to alignment
 *
 *	@comm
 *		Plain text is sensitive to StartIndent and RightIndent settings,
 *		but usually these are zero for plain text. 
 */
LONG CMeasurer::MeasureLineShift()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureLineShift");

	WORD wAlignment = _pPF->wAlignment;
	LONG xShift;

	if (IsInOutlineView() ||
		(wAlignment != PFA_RIGHT && wAlignment != PFA_CENTER))
	{
		return 0;
	}

	// Normal view with center or flush-right para. Move right accordingly
	xShift = _pPF->InTable()
		   ? -LXtoDX(GetTabPos(_pPF->rgxTabs[_pPF->cTabCount - 1]) - _pPF->dxStartIndent)
		   : -_xLineOverhang - _xWidth;

	xShift += _pdp->GetMaxPixelWidth() - _xLeft - MeasureRightIndent()
			  - GetCaretDelta();

	xShift = max(xShift, 0);			// Don't allow alignment to go < 0
										// Can happen with a target device
	if(wAlignment == PFA_CENTER)
		xShift /= 2;

	return xShift;
}

/*
 *	CMeasurer::MeasureBulletHeight()
 *
 *	@mfunc
 *		Computes bullet/numbering height
 *
 *	@rdesc
 *		return bullet/numbering string height
 */
LONG CMeasurer::MeasureBulletHeight()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureBulletHeight");

	CCcs *pccs = GetCcsBullet(NULL);
	LONG yHeight = 0;

	if(pccs && pccs != (CCcs *)(-1))	// NB: pccs = -1 if bullet
	{									//  display is suppressed
		yHeight = pccs->_yHeight;
		pccs->Release();
	}
	return yHeight;
}

/*
 *	CMeasurer::MeasureBulletWidth()
 *
 *	@mfunc
 *		Computes bullet/numbering width
 *
 *	@rdesc
 *		return bullet/numbering string width
 */
LONG CMeasurer::MeasureBulletWidth()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::MeasureBulletWidth");

	CCcs *pccs = GetCcsBullet(NULL);			
	LONG xWidth = 0;

	if(pccs && pccs != (CCcs *)(-1))			// NB: pccs = -1 if bullet
	{											//  display is suppressed
		WCHAR szBullet[CCHMAXNUMTOSTR];
		GetBullet(szBullet, pccs, &xWidth);
		pccs->Release();
	}
	return xWidth;
}

/*
 *	CMeasurer::GetBullet(pch, pccs, pxWidth)
 *
 *	@mfunc
 *		Computes bullet/numbering string, string length, and width
 *
 *	@rdesc
 *		return bullet/numbering string length
 */
LONG CMeasurer::GetBullet(
	WCHAR *pch,			//@parm Bullet string to receive bullet text
	CCcs  *pccs,		//@parm CCcs to use
	LONG  *pxWidth)		//@parm Out parm for bullet width
{
	Assert(pccs && pch);

	LONG cch = _pPF->NumToStr(pch, _bNumber);
	LONG dx;
	LONG i;
	LONG xWidth = 0;

	pch[cch++] = ' ';					// Ensure a little extra space
	for(i = cch; i--; xWidth += dx)
	{
		if(!pccs->Include(*pch++, dx))
		{
			TRACEERRSZSC("CMeasurer::MeasureBulletWidth(): Error filling CCcs", E_FAIL);
		}
	}
	xWidth += pccs->_xUnderhang + pccs->_xOverhang;
	if(pxWidth)
		*pxWidth = xWidth;

	return cch;
}

/*
 *	CMeasurer::GetCcsBullet(pCFRet)
 *
 *	@mfunc
 *		Get CCcs for numbering/bullet font. If bullet is suppressed because
 *		preceding EOP is a VT (Shift-Enter), then returns -1.  If GetCcs()
 *		fails, it returns NULL.
 *
 *	@rdesc
 *		ptr to bullet CCcs, or NULL (GetCcs() failed), or -1 (bullet suppressed)
 *
 *	@comm
 *		This approach is crazy: the bullet charformat is constructed every
 *		time a bullet paragraph gets measured or rendered.  Would be better
 *		if bullet is stored in line and can have a special CHARFORMAT run if
 *		the user applies one
 */
CCcs * CMeasurer::GetCcsBullet(
	CCharFormat *pCFRet)	//@parm option character format to return
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CMeasurer::GetCcsBullet");

	// Make sure the static bullet charformat still makes sense
	AssertSz(sizeof(CHARFORMAT) == cfBullet.cbSize, 
		"CMeasurer::GetCcsBullet Bullet charformat size corrupt");
	AssertSz(CFE_AUTOCOLOR + CFE_AUTOBACKCOLOR == cfBullet.dwEffects,
		"CMeasurer::GetCcsBullet Bullet dwEffects size corrupt");
	AssertSz(SYMBOL_CHARSET == cfBullet.bCharSet,
		"CMeasurer::GetCcsBullet Bullet bCharSet size corrupt");

	if((_pPF->wNumberingStyle & 0xF00) == PFNS_NONUMBER)
		return (CCcs *)(-1);					// Number/bullet suppressed

	CCharFormat			CF;
	CCcs *			    pccs;
	const CCharFormat *	pCF;
	CCharFormat *		pCFUsed = (pCFRet) ? pCFRet : &CF;
	CTxtPtr				rpTX(_rpTX);

	// Bullet CF is given by that for EOP in bullet's paragraph.

	if(GetCp())									// Not at beginning of story
	{											// If preceding EOP is a VT
		if(rpTX.PrevChar() == VT)				//  (Shift-Enter), suppress
			return (CCcs *)(-1);				//  display of bullet
		rpTX.AdvanceCp(1);						// Restore rpTX to _rpTX
	}
	
	CFormatRunPtr rpCF(_rpCF);
	rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
	rpCF.AdjustBackward();
	pCF = GetPed()->GetCharFormat(rpCF.GetFormat());

	if (NULL == pCF)
	{
	    Assert(pCF);
	    return NULL;
	}    

	// Construct bullet (or numbering) CCharFormat for the bullet
	if(_pPF->wNumbering > PFN_BULLET)			// All numbering and Unicode
		*pCFUsed = *pCF;						//  bullets use CF at EOP

	else										// Traditional bullet uses
	{											//  Symbol font bullet, but
		pCFUsed->Set(&cfBullet);				//  some attributes of CF at
		pCFUsed->yHeight		= pCF->yHeight;	//  EOP		
		pCFUsed->dwEffects		= pCF->dwEffects;
		pCFUsed->crTextColor	= pCF->crTextColor;

        // GUYBARK: Can have highlighted background now.
        pCFUsed->crBackColor = pCF->crBackColor;
	}

	// Since we always cook up the bullet character format, we don't need
	// to cache it. 
	pccs = fc().GetCcs(_hdcMeasure, pCFUsed, _pdp->GetZoomNumerator(),
		_pdp->GetZoomDenominator(), _yMeasurePerInch);

#ifdef _DEBUG
	if(!pccs)
	{
		TRACEERRSZSC("CMeasurer::GetCcsBullet(): no CCcs", E_FAIL);
	}
#endif // DEBUG

	return pccs;
}

/*
 *	CMeasurer::SetNumber(wNumber)
 *
 *	@mfunc
 *		Store number if numbered paragraph
 */
void CMeasurer::SetNumber(WORD wNumber)
{
	if(!GetPF()->IsListNumbered())
		wNumber = 0;

	else if (!wNumber)
		wNumber = 1;

	_wNumber = wNumber;
}

/*
 *	CMeasurer::DXtoLX(x), LXtoDX(x), LYtoDY(y)
 *
 *	@mfunc
 *		Functions that return dezoomed and zoomed scaled coordinates
 *
 *	@rdesc
 *		Scaled coordinate
 */
LONG CMeasurer::DXtoLX(LONG x)
{
	return _pdp->UnZoom(_pdd->DXtoLX(x));
}

LONG CMeasurer::LXtoDX(LONG x)
{
	return _pdd->LXtoDX(_pdp->Zoom(x));
}

LONG CMeasurer::LYtoDY(LONG y)
{
	return _pdd->LYtoDY(_pdp->Zoom(y));
}

