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
 *	@module	dispprt.cpp-- Special logic for printer object |
 *  
 *  Authors:
 *      Original RichEdit code: David R. Fulmer
 *      Christian Fortini
 *      Jon Matousek
 */
#include "_common.h"
#include "_dispprt.h"
#include "_edit.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_select.h"

ASSERTDATA

/*
 *	CDisplayPrinter::CDisplayPrinter(ped, hdc, x, y, prtcon)
 *
 *	@mfunc
 *		Contructs a object that can be used to print a text control
 *
 */
CDisplayPrinter::CDisplayPrinter (
	CTxtEdit* ped, 
	HDC hdc, 			//@parm HDC for drawing
	LONG x, 			//@parm Max width to draw
	LONG y, 			//@parm Max height to draw
	SPrintControl prtcon//@parm Special controls for this print object
)	
		: CDisplayML( ped ), _prtcon(prtcon)
{
	TRACEBEGIN(TRCSUBSYSPRT, TRCSCOPEINTERN, "CDisplayPrinter::CDisplayPrinter");

	Assert ( hdc );

	_fNoUpdateView = TRUE;

	_xWidthMax = x;
	_yHeightMax = y;
}

/*
 *	CDisplayPrinter::SetPrintDimensions
 *
 *	@mfunc
 *		Set area to print.
 *
 */
void CDisplayPrinter::SetPrintDimensions(
	RECT *prc)			//@parm dimensions of current area to print to.
{
	_xWidthMax = prc->right - prc->left;
	_yHeightMax = prc->bottom - prc->top;
}

/*
 *	CDisplayPrinter::FormatRange(cpFirst, cpMost)
 *
 *	@mfunc
 *		Format a range of text into this display and used only for printing.
 *
 *	@rdesc
 *		actual end of range position (updated)	
 *
 */
LONG CDisplayPrinter::FormatRange(
	LONG cpFirst, 		//@parm Start of text range
	LONG cpMost			//@parm End of text range
)
{
	TRACEBEGIN(TRCSUBSYSPRT, TRCSCOPEINTERN, "CDisplayPrinter::FormatRange");

	BOOL		fFirstInPara = TRUE;
	CLine		liTarget;
	CLine *		pliNew = NULL;
	LONG		yHeightRnd;
	LONG		yHeightTgt;
	LONG		cpLineStart;
	BOOL		fFirstLine = TRUE;
	BOOL		fBindCp = FALSE;
	WCHAR		ch;
	const CDevDesc *pdd = GetDdTarget() ? GetDdTarget() : this;

	// Set the client height for zooming
	_yHeightClient = this->LYtoDY(_yHeightMax);

	// Set maximum in terms of target DC.
	LONG	yMax = pdd->LYtoDY(_yHeightMax);

	if(cpMost < 0)
		cpMost = _ped->GetTextLength();

	CMeasurer me(this);
	
	me.SetCp(cpFirst);
	ch = me.GetChar();

	// COMPATIBILITY ISSUE:  Richedit 1.0 adjusted to before a
	// CRLF/CRCRLF boundary.  Since we only have CR's, we don't 
	// have to worry about this case.
 

	if (fBindCp)
	{
		cpFirst = me.GetCp();
		me._rpCF.BindToCp(cpFirst);
		me._rpPF.BindToCp(cpFirst);
	}

	_cpMin = cpFirst;
	_cpFirstVisible = cpFirst;
	
	yHeightTgt = 0;
	yHeightRnd = 0;
	if(me.GetCp())
		fFirstInPara = me._rpTX.IsAfterEOP();

	// Clear line CArray
	Clear(AF_DELETEMEM);

	// Assume that we will break on words
	UINT uiBreakAtWord = MEASURE_BREAKATWORD;

	if (_prtcon._fPrintFromDraw)
	{
		// This is from Draw so we want to take the inset into account
		LONG xWidthView = _xWidthMax;

		GetViewDim(xWidthView, yMax);
		_xWidthView = (SHORT) xWidthView;

		// restore the client hight
		_yHeightClient = this->LYtoDY(_yHeightMax);

		// We don't want to break at words if we are drawing a control and the
		// control would not break at words.
		if (!_ped->TxGetMultiLine())
		{
			SetWordWrap(FALSE);
			uiBreakAtWord = 0;
		}
	}
	else
	{
		// The message based printing always does word wrap.
		SetWordWrap(TRUE);
	}
	
	while((LONG)me.GetCp() < cpMost)
	{
		// Add one new line
		pliNew = Add(1, NULL);
		if (!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto err;
		}

		// Stuff some text into this new line
		me.NewLine(fFirstInPara);
		cpLineStart = (LONG)me.GetCp();
		if(!me.MeasureLine(-1, cpMost - (LONG)me.GetCp(), 
			uiBreakAtWord | (fFirstInPara ? MEASURE_FIRSTINPARA : 0), 
				&liTarget))
		{
			Assert(FALSE);
			goto err;
		}

#ifdef PWD_JUPITER
        // GuyBark Jupiter 35734: If the line is collapsed, 
        // then we mustn't account for its uncollapsed height.
        if(IsInOutlineView() && (me._pPF->wEffects & PFE_COLLAPSED))
        {
            // The line is collapsed. We should could ignore this line completely
            // for printing. But I don't want to change the code path more than I
            // have to. So just change the two temporary variables for the height 
            // of this line. 
            liTarget._yHeight = 0;
            me._yHeight       = 0;
        }
#endif // PWD_JUPITER

		// Note, we always put at least one line on a page. Otherwise, if the 
		// first line is too big, we would cause our client to infinite loop
		// because we would never advance the print cp.
		if (!fFirstLine && (yHeightTgt + liTarget._yHeight > yMax))
		{	
			// overflowed the display, move back to beginning of line.
			if (!_prtcon._fDoPrint)
			{
				// If not print from Draw we want to pick up this full
				// line.
				*pliNew = me;
			}

			me.SetCp(cpLineStart);
			break;
		}

        // V-GUYB:
        // Is this the first line in the format range and it's in a numbered list?

        // GuyBark Jupiter 33348: We used to call me.GetPF()->IsListNumbered() here.
        // But it seems that the cache used to get the PF here can be stale, and we
        // find the PF is not numbered, when in fact it is. So instead, look at
        // the PF held inside the me structure. That's up to date ok.

        // GuyBark Jupiter 33348: Remove the check to see if this is the first line too.
        // Originally I thought we only needed this if this was the first line on the 
        // page. But say we adjust the displayed number here, and then the list interally
        // restarts part way through the page. The wNumberingStart value doesn't realize
        // the numbers have already been bumped, and the printed numbers have a 
        // discontinuity in them. So make this check for everyline in a numbered list.
        // This makes the printing a tiny bit slower, but the user won't notice.
        if(me._pPF->IsListNumbered() && _ped->_pdp)
        {
            // Yes. Get details for this line as it is displayed in the document.

            // So first make sure the main line array has been calculated that far.
            if(!_ped->_pdp->WaitForRecalc(cpLineStart, -1))
            {
    			_ped->GetCallMgr()->SetOutOfMemory();
                goto err;
            }

      	    CLinePtr rp(_ped->_pdp);

        	rp.RpSetCp(cpLineStart, FALSE);

            CLine *pli = rp.GetLine();

            // Did we manage to get the details?

            // GuyBark Jupiter 33348: Previously we only used to take action here 
            // if _bNumber was > 1.  The idea was we didn't need to restart the 
            // number, if it's already 1. Turns out that _bNumber can be 1 here, 
            // but the number finally displayed is > 1. That means we still need 
            // to take action here. _line.h says _bNumber is the...
            // "Abstract paragraph number (0 is unnumbered)".

            if(pliNew && pli && (pli->_bNumber > 0))
            {
                // Set the printed line to be numbered as it is displayed on the screen.
                me._bNumber = pli->_bNumber;

                // The measure number was incremented beneath MeasureLine, so this is 
                // one greater than the value stored in the line.
                me._wNumber = pli->_bNumber + 1;
            }
        }

		fFirstLine = FALSE;
		*pliNew = me;

		// REMARK: the following looks suspicious: the first line can also be the last
		fFirstInPara = pliNew->_cchEOP;

		yHeightTgt += liTarget._yHeight;
		yHeightRnd += pliNew->_yHeight;
	}

	// If there was not text, then add a single blank line
	if (NULL == pliNew)
	{
		pliNew = Add(1, NULL);

		if (!pliNew)
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			goto err;
		}

		me.NewLine(fFirstInPara);

		*pliNew = me;
	}	

	// Update display height
	_yHeight = yHeightRnd;

	// Update display width
	_xWidth = CalcDisplayWidth();

	cpMost = me.GetCp();
	_cpCalcMax = cpMost;
	_yCalcMax = _yHeight;

	return cpMost;

err:
	Clear(AF_DELETEMEM);
	_xWidth = 0;
	_yHeight = 0;

	return -1;
}



/*
 *	CDisplayPrinter::GetNaturalSize(hdcDraw, hicTarget, dwMode, pwidth, pheight)
 *
 *	@mfunc
 *		Recalculate display to input width & height for TXTNS_FITTOCONTENT.
 *
 *
 *	@rdesc
 *		S_OK - Call completed successfully <nl>
 *
 *	@devnote
 *		This assumes that FormatRange was called just prior to this.
 *		
 */
HRESULT	CDisplayPrinter::GetNaturalSize(
	HDC hdcDraw,		//@parm DC for drawing
	HDC hicTarget,		//@parm DC for information
	DWORD dwMode,		//@parm Type of natural size required
	LONG *pwidth,		//@parm Width in device units to use for fitting 
	LONG *pheight)		//@parm Height in device units to use for fitting
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDisplayPrinter::GetNaturalSize");

	*pwidth = _xWidth;
	*pheight = _yHeight;
	return S_OK;
}


/*
 *	CDisplayPrinter::IsPrinter
 *
 *	@mfunc
 *		Returns whether this is a printer
 *
 *	@rdesc
 *		TRUE - is a display to a printer
 *		FALSE - is not a display to a printer
 *
 *
 */
BOOL CDisplayPrinter::IsPrinter() const
{
	AssertSz(_hdc != NULL, "CDisplayPrinter::IsPrinter no hdc set");
	
	return GetDeviceCaps(_hdc, TECHNOLOGY) == DT_RASPRINTER;
}

