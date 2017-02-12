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
 *  _DISPML.H
 *  
 *  Purpose:
 *      CDisplaySL class. Single-line display.
 *  
 *  Authors:
 *      Eric Vasilik
 */

#ifndef _DISPSL_H
#define _DISPSL_H

#include "_disp.h"

// Forward declaration
class CTxtSelection;

class CDisplaySL : public CDisplay, private CLine
{
public:

    CDisplaySL ( CTxtEdit* ped );
    
protected:


    // The following are pure functions from the base

    // Helpers
    virtual BOOL Init();
    void InitVars();
    
    // Line breaking
    BOOL RecalcLine();

    // Rendering
    virtual VOID Render(const RECT &rcView, const RECT &rcRender);
    
    // Scrolling and scroller bars
    virtual BOOL UpdateScrollBar(INT nBar, BOOL fUpdateRange = FALSE);
    virtual LONG GetScrollRange(INT nBar) const;
    
    // Getting properties
    virtual void InitLinePtr ( CLinePtr & plp );
    virtual BOOL IsMain() const;
    
    // maximum height and width
    virtual LONG GetMaxWidth() const;
    virtual LONG GetMaxHeight() const;
	virtual LONG GetMaxPixelWidth() const;

    // Width, height and line count (of all text)
    virtual LONG GetWidth() const;
    virtual LONG GetHeight() const;
	virtual LONG GetResizeHeight() const;
    virtual LONG LineCount() const;

    // Visible view properties
    virtual LONG GetCliVisible(
					LONG *pcpMostVisible = NULL,
					BOOL fLastCharOfLastVisible = FALSE) const;

    virtual LONG GetFirstVisibleLine() const;

    // Line info
    virtual LONG GetLineText(LONG ili, TCHAR *pchBuff, LONG cchMost);
    virtual LONG CpFromLine(LONG ili, LONG *pyLine = NULL);
    virtual LONG LineFromCp(LONG cp, BOOL fAtEnd);
    
    // Point <-> cp conversion
    virtual LONG CpFromPoint(
    				POINT pt, 
					const RECT *prcClient,
    				CRchTxtPtr * const ptp, 
    				CLinePtr * const prp, 
    				BOOL fAllowEOL,
					HITTEST *pHit = NULL);

    virtual LONG PointFromTp (
					const CRchTxtPtr &tp, 
					const RECT *prcClient,
					BOOL fAtEnd, 
					POINT &pt,
					CLinePtr * const prp, 
					UINT taMode);

    // Line break recalc
    virtual BOOL RecalcView(BOOL fUpdateScrollBars);
    virtual BOOL WaitForRecalcIli(LONG ili);

    // Complete updating (recalc + rendering)
    virtual BOOL UpdateView(const CRchTxtPtr &tpFirst, LONG cchOld, LONG cchNew);

    // Scrolling 
    virtual BOOL ScrollView(LONG xScroll, LONG yScroll, BOOL fTracking, BOOL fFractionalScroll);
    
    // Selection 
    virtual BOOL InvertRange(LONG cp, LONG cch, SELDISPLAYACTION selAction);

	// Natural size calculation
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight);

    virtual BOOL    GetWordWrap() const;

	virtual CDisplay *Clone() const;

	virtual LONG	GetMaxXScroll() const;
};

#endif

