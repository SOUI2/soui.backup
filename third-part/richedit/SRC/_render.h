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
 *	_RENDER.H
 *	
 *	Purpose:
 *		CRenderer class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#ifndef _RENDER_H
#define _RENDER_H

#include "_measure.h"
#include "_rtext.h"
#include "_osdc.h"

class CDisplay;

// ==========================  CRenderer  ==================================
// CRenderer - specialized text pointer used for rendering text

class CRenderer : public CMeasurer
{
private:

    RECT        _rcView;        // view rect (in _hdc logical coords)
    RECT        _rcRender;      // rendered rect. (in _hdc logical coords)
    RECT        _rc;		    // running clip/erase rect. (in _hdc logical coords)
    LONG        _xWidthLine;    // total width of line
	COLORREF	_crForeDisabled;// foreground color for disabled text
	COLORREF	_crShadowDisabled;// the shadow color for disabled text
	LONG		_cpAccelerator;	// Accelerator cp if any (-1 if none).
	COLORREF	_crBackground;	// Default background color
	COLORREF	_crCurBackground;// Current background color
	COLORREF	_crTextColor;	// Default text color
	COffScreenDC _osdc;			// Manager for off screen DC
	COLORREF	_crCurTextColor;// Current text color

	union
	{
	  DWORD		_dwFlags;			// All together now
	  struct
	  {
		DWORD	_fDisabled:1;		// draw text with disabled effects?
		DWORD	_fErase:1;	    	// erase background (non transparent)
    	DWORD	_fSelected:1;   	// draw selected text (inverted)
		DWORD	_fComp:1;	    	// composition string is being rendered
    
		DWORD	_fLastChunk:1;		// rendering first line (inside rendering rect)
		DWORD	_fLastLine:1;		// rendering last line

    	DWORD	_fClipLeftToView:1;
    	DWORD	_fClipRightToView:1;
    	DWORD	_fClipTopToView:1;
    	DWORD	_fClipBottomToView:1;

		DWORD	_fFirstChunk : 1;	// Whether we have written anything to line
									// This is used for displaying justified
									// lines to tell us whether to clear to the.
									// beginning of the line
		DWORD	_fRecalcRectForInvert : 1;	// Whether we need to recalc the 
									// rectangle for inverting beacause of a
									// the display rectangle was used to clear
									// the leading part of a justified line.
		DWORD	_fSelectToEOL:1;	// Whether selection runs to end of line
		DWORD	_fUseOffScreenDC:1;	// Using off screen DC
		DWORD	_fRenderSelection:1;// Render selection?
		DWORD	_fBackgroundColor:1;// Some text in the line has non-default 
									// background color.
		DWORD	_fEnhancedMetafileDC:1;	// Use ExtTextOutA to hack around all
										// sort of Win95FE EMF or font problems
		DWORD	_fFEFontOnNonFEWin95:1; // have to use ExtTextOutW even for EMF.
	  };
	};

	BYTE		_bUnderlineType;	// Part of CF2, needed to render text.

	LOGPALETTE *_plogpalette;

	void			Init();			// initialize everything to zero

	LONG			CalcHeightBitmap(LONG yHeightToRender);

	HDC				SetUpOffScreenDC(
						COffScreenDC& osdc,
						LONG& xAdj,			
						LONG& yAdj,
						LONG& yRcIgnored);

	void			RenderOffScreenBitmap(
						COffScreenDC& osdc,
						HDC hdc,
						LONG yAdj,
						LONG xAdj,
						LONG yRcIgnored);

	void			UpdatePalette(COleObject *pobj);

protected:

	POINT 	_ptCur;		// current rendering position on screen (in hdc logical coord.)

			LONG 	TextOut(const WCHAR* pch, LONG cch, BOOL bIMEHighlight = FALSE);
			BOOL	SetNewFont();
			void	SetFontAndColor(const CCharFormat *pcf);
            void    SetClipLeftRight(LONG xWidth);
		
			void	CheckEraseUptoMargin(LONG xWidth, BOOL fIMEHighlight);
			BOOL	RenderStartLine();
			BOOL 	RenderChunk(LONG &cchChunk, const WCHAR *pszToRender, LONG cch);
			LONG	RenderTabs(LONG cchChunk);
			BOOL	RenderBullet();
			BOOL	RenderOutlineSymbol();
			void	RenderUnderline(LONG xStart, LONG xWidth);
			void	RenderStrikeOut(LONG xStart, LONG xWidth);
			void	FillRectWithTextColor(RECT *prc);

public:
	CRenderer (const CDisplay * const pdp);
	CRenderer (const CDisplay * const pdp, const CRchTxtPtr &rtp);
	~CRenderer (){}

	        void    operator =(const CLine& li)     {*(CLine*)this = li;}

			void	SetCurPoint(const POINT &pt)		{_ptCur = pt;}
	const	POINT&	GetCurPoint() const					{return _ptCur;}

			HDC		GetDC()	const						{return _hdc;}

	virtual BOOL	StartRender(
						const RECT &rcView, 
						const RECT &rcRender,
						const LONG yHeightBitmap);

	virtual VOID	EndRender();

	virtual	VOID 	NewLine (const CLine &li);
	virtual BOOL	RenderLine(CLine &li, BOOL fLastLine = FALSE);
};

/*
 * 	BottomOfRender (rcView, rcRender)
 *
 *	@mfunc
 *		Calculate maximum logical unit to render.
 *
 *	@rdesc
 *		Maximum pixel to render
 *
 *	@devnote
 *		This function exists to allow the renderer and dispml to be able
 *		to calculate the maximum pixel for rendering in exactly the same
 *		way.
 */
inline LONG BottomOfRender(const RECT& rcView, const RECT& rcRender)
{
	return min(rcView.bottom, rcRender.bottom);
}		

#endif

