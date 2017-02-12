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
 *	_MEASURE.H
 *	
 *	Purpose:
 *		CMeasurer class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#ifndef _MEASURE_H
#define _MEASURE_H

#include "_rtext.h"
#include "_line.h"

class CCcs;
class CDisplay;
class CDevDesc;
class CPartialUpdate;

#define BITMAP_WIDTH_SUBTEXT    4
#define BITMAP_HEIGHT_SUBTEXT   4

#define BITMAP_WIDTH_HEADING    10
#define BITMAP_HEIGHT_HEADING   10


// ===========================  CMeasurer  ===============================
// CMeasurer - specialized text pointer used to compute text metrics.
// All metrics are computed and stored in device units for the device indicated
// by _pdd.

class CMeasurer : public CRchTxtPtr, protected CLine
{
	friend class CDisplay;
	friend class CDisplayML;
	friend class CDisplayPrinter;
	friend class CDisplaySL;
	friend class CLine;

public:
	CMeasurer (const CDisplay* const pdp);
	CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &rtp);
	virtual ~CMeasurer();

	void	operator =(const CLine& li)  {*(CLine*)this = li;}

	const CDisplay* GetDp()	const 		{return _pdp;}
	const CDevDesc* GetDd()	const 		{return _pdd;}

	TCHAR	GetPasswordChar() const { return _chPassword;}
	HITTEST	HitTest(LONG x);

	void	NewLine(BOOL fFirstInPara);
	void	NewLine(const CLine &li);
	LONG    MeasureLeftIndent();
	LONG	MeasureRightIndent();
	LONG 	MeasureLineShift();
	LONG	MeasureText(LONG cch);
	BOOL 	MeasureLine(
					LONG xWidthMax,
					LONG cchMax, 
					UINT uiFlags, 
					CLine* pliTarget = NULL);
	LONG	MeasureTab(unsigned ch);
	void	SetNumber(WORD wNumber);

protected:
	void 	AdjustLineHeight();
	LONG 	Measure(LONG xWidthMax, LONG cchMax, UINT uiFlags);
	LONG	MeasureBulletHeight();
	LONG	MeasureBulletWidth();
	LONG	GetBullet(WCHAR *pch, CCcs *pccs, LONG *pxWidth);
	CCcs*	GetCcsBullet(CCharFormat *pcfRet);

	BOOL	FormatIsChanged();
	void	ResetCachediFormat();
	LONG	DXtoLX(LONG x);	
	LONG	LXtoDX(LONG x);
	LONG	LYtoDY(LONG y);

private:
    void 	RecalcLineHeight(void);		// Helper to recalc max line height
	LONG	MaxWidth();					// Helper for calc max width

protected:
	const CDisplay*	_pdp;	// display we are operating in
	const CDevDesc*	_pdd;	// device we are measuring for (target device)
          HDC       _hdc;   // cached device context of _pdd
		  HDC		_hdcMeasure; // measuring DC (in case the render device is
							// is a metafile!
		  LONG		_yMeasurePerInch; // yPerInch for hdcMeasure
		  CCcs*		_pccs;	// current font cache
		  const CParaFormat *_pPF;	// Current CParaFormat

private:
		  TCHAR		_chPassword;	// Password character if any
		  WORD		_wNumber;		// Number offset
		  LONG		_iFormat;		// Current format
};


// Values for uiFlags in MeasureLine()
#define MEASURE_FIRSTINPARA 	((UINT) 0x0001)
#define MEASURE_BREAKATWORD 	((UINT) 0x0002)
#define MEASURE_BREAKATWIDTH	((UINT) 0x0004)
#define MEASURE_IGNOREOFFSET	((UINT) 0x0008)

// Returned error codes for Measure(), MeasureText(), MeasureLine()
#define MRET_FAILED		-1
#define MRET_NOWIDTH	-2

inline BOOL CMeasurer::FormatIsChanged()
{
	return _iFormat != _rpCF.GetFormat();
}

inline void CMeasurer::ResetCachediFormat()
{
	_iFormat = _rpCF.GetFormat();
}

#endif

