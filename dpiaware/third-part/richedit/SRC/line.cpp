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
 *	LINE.C
 *	
 *	Purpose:
 *		CLine class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#include "_common.h"
#include "_line.h"
#include "_measure.h"
#include "_render.h"
#include "_disp.h"
#include "_edit.h"

ASSERTDATA

/*
 *	CLine::Measure(&me, cchMax, uiFlags)
 *
 *	@mfunc
 *		Computes line break (based on target device) and fills
 *		in this CLine with resulting metrics on rendering device
 *
 *	@rdesc 
 *		TRUE if OK
 *
 *	@devnote
 *		me is moved to end of line
 */
BOOL CLine::Measure(
	CMeasurer& me,
	LONG cchMax,
	UINT uiFlags)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::Measure");

	me.NewLine(uiFlags & MEASURE_FIRSTINPARA);
	if(!me.MeasureLine (-1, cchMax, uiFlags))
		return FALSE;
	*this = me;
	return TRUE;
}
	
/*
 *	CLine::GetHeight()
 *
 *	@mfunc
 *		Get line height unless in outline mode and collasped, in
 *		which case get 0.
 *
 *	@rdesc
 *		Line height (_yHeight), unless in outline mode and collapsed,
 *		in which case 0.
 */
LONG CLine::GetHeight() const
{
	return _fCollapsed ? 0 : _yHeight;
}

BOOL CLine::IsEqual(CLine& li)
{
	// CF - I dont know which one is faster
	// MS3 - CompareMemory is certainly smaller
	// return !CompareMemory (this, pli, sizeof(CLine) - 4);
	return _xLeft == li._xLeft &&
		   _xWidth == li._xWidth && 
		   _yHeight == li._yHeight &&
		   _yDescent == li._yDescent &&
		   _cchWhite == li._cchWhite;	
}

/*
 *	CLine::CchFromXPos(&me, x, pdx, pHit)
 *
 *	@mfunc
 *		Computes cp corresponding to a x position in a line
 *
 *	@rdesc 
 *		cp of character found	
 *
 *	@devnote
 *		me is moved to returned cp
 */
LONG CLine::CchFromXpos(
	CMeasurer& me,		//@parm measurer position at start of line
	LONG	 x,			//@parm xpos to search for
	LONG *	 pdx,		//@parm returns adjustment to x at returned cp
	HITTEST *pHit) const//@parm returns hit type at x	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLine::CchFromXpos");

	LONG		dx = 0;
	const BOOL	fFirst = _bFlags & fliFirstInPara;
	HITTEST		Hit = HT_Text;

	if(x < _xLeft)
	{
		Hit = HT_SelectionBar;			// Default selection bar
		if(x > 0)						// It isn't. Ask measurer
		{								//  what's being hit
			me = *this;
			Hit = me.HitTest(x);
		}
	}
	x -= _xLeft;

	me._cch = 0;						// Default zero count

	if(x > 0)							// To right of left margin
	{
		me.NewLine(fFirst);
		if(me.Measure(x, _cch,
			MEASURE_BREAKATWIDTH | MEASURE_IGNOREOFFSET 
				| (fFirst ? MEASURE_FIRSTINPARA : 0)) >= 0)
		{
			dx = me._xWidth - x;
		}
		if(x > _xWidth)
			Hit = HT_RightOfText;
		else
		{
			me._rpCF.AdjustBackward();
			DWORD dwEffects = me.GetCF()->dwEffects;
			if(dwEffects & CFE_LINK)
				Hit = HT_Link;

			else if(dwEffects & CFE_ITALIC)
				Hit = HT_Italic;
		}
	}

	if(pdx)
		*pdx = dx;

	if(pHit)
		*pHit = Hit;

	return me._cch;
}


// =====================  CLinePtr: Line Run Pointer  ==========================


CLinePtr::CLinePtr(CDisplay *pdp)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::CLinePtr");

	_pdp = pdp;
	_pdp->InitLinePtr( * this );
}

void CLinePtr::Init ( CLine & line )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::Init");

	_prgRun = 0;
	_pLine = &line;
	_iRun = 0;
	_ich = 0;
}

void CLinePtr::Init ( CLineArray & line_arr )
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::Init");

	_prgRun = (CRunArray *) & line_arr;
	_iRun = 0;
	_ich = 0;
}

void CLinePtr::RpSet(LONG iRun, LONG ich)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSet");

	// See if this is a multi-line ptr
    if(_prgRun)
        CRunPtr<CLine>::SetRun(iRun, ich);
    else
    {
        // single line, just reinit and set _ich
        AssertSz(iRun == 0, "CLinePtr::RpSet() - single line and iRun != 0");
	    _pdp->InitLinePtr( * this );		//  to line 0
	    _ich = ich;
    }
}

// Move runptr by a certain number of cch/runs

BOOL CLinePtr::RpAdvanceCp(LONG cch)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpAdvanceCp");

	// See if this is a multi-line ptr

	if (_prgRun)
		return (cch == CRunPtr<CLine>::AdvanceCp(cch));
	else
		return RpAdvanceCpSL( cch );
}
	
BOOL CLinePtr::operator --(int)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator --");

	if (_prgRun)
		return PrevRun();
	else
		return OperatorPostDeltaSL(-1);
}

BOOL CLinePtr::operator ++(int)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator ++");

	if (_prgRun)
		return NextRun();
	else
		return OperatorPostDeltaSL(+1);
}

/*
 *	CLinePtr::RpAdvanceCpSL(cch)
 *
 *	@mfunc
 *		move this line pointer forward or backward on the line
 *
 *	@rdesc
 *		TRUE iff could advance cch chars within current line
 */
BOOL CLinePtr::RpAdvanceCpSL(
	LONG cch)	 //@parm signed count of chars to advance by
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpAdvanceCpSL");

	Assert( !_prgRun );
	
	if (!_pLine)
		return FALSE;

	_ich += cch;

	if((LONG) _ich < 0)
	{
		_ich = 0;
		return FALSE;
	}

	if(_ich > _pLine->_cch)
	{
		_ich = _pLine->_cch;
		return FALSE;
	}

	return TRUE;
}

/*
 *	CLinePtr::OperatorPostDeltaSL(Delta)
 *
 *	Purpose:
 *		Implement line-ptr ++ and -- operators for single-line case
 *
 *	Arguments:
 *		Delta	1 for ++ and -1 for --
 *
 *	Return:
 *		TRUE iff this line ptr is valid
 */
BOOL CLinePtr::OperatorPostDeltaSL(LONG Delta)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::OperatorPostDeltaSL");

	AssertSz( (DWORD) _iRun <= 1 && !_prgRun,
		"LP::++: inconsistent line ptr");

	if ((LONG)_iRun == -Delta)					// Operation validates an
	{										//  invalid line ptr by moving
		_pdp->InitLinePtr( * this );		//  to line 0
		return TRUE;
	}
	
	_iRun = Delta;							// Operation invalidates this line
	_ich = 0;								//  ptr (if it wasn't already)

	return FALSE;
}

CLine *	CLinePtr::operator ->() const		
{
	return (_prgRun) ? (CLine *)_prgRun->Elem(_iRun) : _pLine;
}

CLine * CLinePtr::GetLine() const
{	
    return (_prgRun) ? (CLine *)_prgRun->Elem(_iRun) : _pLine;
}

CLine &	CLinePtr::operator *() const      
{	
    return *((_prgRun) ? (CLine *)_prgRun->Elem(_iRun) : _pLine);
}

CLine & CLinePtr::operator [](LONG dRun)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::operator []");

	if (_prgRun)
		return *(CLine *)CRunPtr<CLine>::GetRun(dRun);

	AssertSz( dRun + _iRun == 0 ,
		"LP::[]: inconsistent line ptr");

	return  *(CLine *)CRunPtr<CLine>::GetRun(_iRun);
}

BOOL CLinePtr::IsValid() 
{ 
	return (!_prgRun) ? _pLine != NULL : CRunPtrBase::IsValid(); 
}

/*
 *	CLinePtr::RpSetCp(cp, fAtEnd)
 *
 *	Purpose	
 *		Set this line ptr to cp allowing for ambigous cp and taking advantage
 *		of _cpFirstVisible and _iliFirstVisible
 *
 *	Arguments:
 *		cp		position to set this line ptr to
 *		fAtEnd	if ambiguous cp:
 *				if fAtEnd = TRUE, set this line ptr to end of prev line;
 *				else set to start of line (same cp, hence ambiguous)
 *	Return:
 *		TRUE iff able to set to cp
 */
BOOL CLinePtr::RpSetCp(LONG cp, BOOL fAtEnd)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::RpSetCp");

	if (!_prgRun)
	{
		// This is a single line so just go straight to the single
		// line advance logic. It is important to note that the
		// first visible character is irrelevent to the cp advance
		// for single line displays.
		return RpAdvanceCpSL(cp);
	}

	BOOL fRet;
	LONG cpFirstVisible = _pdp->GetFirstVisibleCp();

	if(cp > cpFirstVisible / 2)
	{											// cpFirstVisible closer than 0
		_iRun = _pdp->GetFirstVisibleLine();
		_ich = 0;
		fRet = RpAdvanceCp(cp - cpFirstVisible);
	}
	else
		fRet = (cp == (LONG)CRunPtr<CLine>::BindToCp(cp));	// Start from 0

	if(fAtEnd)									// Ambiguous-cp caret position
		AdjustBackward();						//  belongs at prev EOL

	return fRet;
}

/*
 *	CLinePtr::FindParagraph(fForward)
 *
 *	Purpose	
 *		Move this line ptr to paragraph (fForward) ? end : start, and return
 *		change in cp
 *
 *	Arguments:
 *		fForward	TRUE move this line ptr to para end; else to para start
 *
 *	Return:
 *		change in cp
 */
LONG CLinePtr::FindParagraph(BOOL fForward)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CLinePtr::FindParagraph");

	LONG	cch;
	CLine *	pLine = GetLine();

	if(!fForward)							// Go to para start
	{
		cch = 0;							// Default already at para start
		if(RpGetIch() != (LONG)pLine->_cch
		   || !pLine->_cchEOP)				// It isn't at para start
		{
			cch = -RpGetIch();				// Go to start of current line
			while(!(pLine->_bFlags & fliFirstInPara) && (*this) > 0)
			{
				(*this)--;					// Go to start of prev line
				pLine = GetLine();
				cch -= pLine->_cch;			// Subtract # chars in line
			}
			_ich = 0;						// Leave *this at para start
		}
	}
	else									// Go to para end
	{
		cch = GetCchLeft();					// Go to end of current line

		while(((*this) < _pdp->LineCount() - 1 ||
				_pdp->WaitForRecalcIli((LONG)*this + 1))
			  && !(*this)->_cchEOP)
		{
			(*this)++;						// Go to start of next line
			cch += (*this)->_cch;			// Add # chars in line
		}
		_ich = (*this)->_cch;				// Leave *this at para end
	}
	return cch;
}

/*
 *	CLinePtr::GetAdjustedLineLength
 *
 *	@mfunc	returns the length of the line _without_ EOP markers
 *
 *	@rdesc	LONG; the length of the line
 */
LONG CLinePtr::GetAdjustedLineLength()
{
	CLine * pline = GetLine();

	return pline->_cch - pline->_cchEOP;
}


/*
 *	CLinePtr::GetCchLeft()
 *
 *	@mfunc
 *		Calculate length of text left in run starting at the current cp.
 *		Complements GetIch(), which	is length of text up to this cp. 
 *
 *	@rdesc
 *		length of text so calculated
 */
DWORD CLinePtr::GetCchLeft() const
{
	if (_prgRun != NULL)
	{
		return CRunPtrBase::GetCchLeft();
	}

	// Single line case 
	return _pLine->_cch - GetIch();
}

/*
 *	CLinePtr::GetNumber()
 *
 *	@mfunc
 *		Calculate length of text left in run starting at the current cp.
 *		Complements GetIch(), which	is length of text up to this cp. 
 *
 *	@rdesc
 *		length of text so calculated
 */
WORD CLinePtr::GetNumber()
{
	if(!IsValid())
		return 0;

	_pLine = GetLine();
	if(!_iRun && _pLine->_bNumber > 1)
		_pLine->_bNumber = 1;

	return _pLine->_bNumber;
}