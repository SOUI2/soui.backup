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
 *	_LINE.H
 *	
 *	Purpose:
 *		CLine class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 *
 */

#ifndef _LINE_H
#define _LINE_H

#include "_runptr.h"

class CDisplay;
class CMeasurer;
class CRenderer;


// ============================	 CLine	=====================================
// line - keeps track of a line of text
// All metrics are in rendering device units

class CLine : public CTxtRun
{
public:
	LONG	_xLeft;			// line left position (line indent + line shift)
	LONG	_xWidth;		// line width not incl _xLeft, trailing whitespace
	SHORT	_yHeight;		// line height
	SHORT	_yDescent;		// distance from baseline to bottom of line
	SHORT	_xLineOverhang;	// Overhang for the line. 
	WORD	_cchWhite;		// Count of white chars at end of line

	BYTE	_cchEOP;		// Count of EOP chars; 0 if no EOP this line
	BYTE	_bFlags;		// Flags defined below

	BYTE	_bNumber;		// Abstract paragraph number (0 is unnumbered)
	BYTE	_nHeading:4;	// Heading level (0 if not heading)
	BYTE	_fCollapsed:1;	// TRUE if line is collapsed
	BYTE	_fStartsList:1;	// TRUE if line starts a list
	BYTE	_fNextInTable:1;// TRUE if next line is in table

public:
	CLine ()	{}
	
	// !!!!! CLine should not have any virtual method !!!!!!

	LONG CchFromXpos(CMeasurer& me, LONG x, LONG *pdx, HITTEST *pHit = NULL) const;
	LONG GetHeight() const;
	void Init()	{ZeroMemory(this, sizeof(CLine));}
	BOOL IsEqual(CLine& li);
	BOOL Measure(CMeasurer& me, LONG cchMax, UINT uiFlags);
};

// Line flags
#define fliHasTabs			0x0004		// set if tabs, *not* iff tabs
#define fliHasOle			0x0008
#define fliFirstInPara		0x0010
#define fliUseOffScreenDC	0x0020		// Line needs to be rendered off
										//  screen to handle change in fonts
#define fliOffScreenOnce	0x0040		// Only render off screen once. Used
										//  for rendering 1st line of an edit


// ==========================  CLineArray  ===================================
// Array of lines

typedef CArray<CLine>	CLineArray;

// ==========================  CLinePtr	 ===================================
// Maintains position in a array of lines

class CLinePtr : public CRunPtr<CLine>
{
protected:
	CDisplay *	_pdp;
	CLine *		_pLine;	

public:
	CLinePtr (CDisplay *pdp);
	CLinePtr (CLinePtr& rp) : CRunPtr<CLine> (rp)	{}

	void Init ( CLine & );
	void Init ( CLineArray & );
    
	// Alternate initializer
	void 	RpSet(LONG iRun, LONG ich);

	// Direct cast to a run index
	operator LONG() const			{return _iRun;}

	// Get the run index (line number)
	LONG GetLineIndex(void)			{return _iRun;}
	LONG GetAdjustedLineLength();

	DWORD GetCchLeft() const;

	// Dereferencing
	BOOL	IsValid(); 
	CLine *	operator ->() const;		
    CLine &	operator *() const;      
	CLine & operator [](LONG dRun);
	CLine * GetLine() const;
	WORD	GetNumber();
	WORD	GetHeading()	{return GetLine()->_nHeading;}
    
	// Pointer arithmetic
	BOOL	operator --(int);
	BOOL	operator ++(int);

	// Character position control
	LONG	RpGetIch() const		{return _ich;}
	BOOL	RpAdvanceCp(LONG cch);
	BOOL	RpSetCp(LONG cp, BOOL fAtEnd);
    BOOL	OperatorPostDeltaSL(LONG Delta);
    BOOL	RpAdvanceCpSL(LONG cch);

	// Array management 
    // These should assert, but gotta be here
    
    // Strictly speaking, these members should never be called for the single
    // line case.  The base class better assert
    
	void Remove (LONG cRun, ArrayFlag flag)
    {
        CRunPtr<CLine>::Remove(cRun, flag);
    }

	BOOL Replace(LONG cRun, CArrayBase *parRun)
    {
        return CRunPtr<CLine>::Replace(cRun,parRun);
    }
	
	// Assignment from a run index
	CRunPtrBase& operator =(LONG iRun) {SetRun(iRun, 0); return *this;}

	LONG	FindParagraph(BOOL fForward);
};

#endif

