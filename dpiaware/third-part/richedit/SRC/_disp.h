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
 *  _DISP.H
 *  
 *  Purpose:
 *		DISP class
 *  
 *  Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#ifndef _DISP_H
#define _DISP_H

#include "_devdsc.h"
#include "_line.h"
#include "_edit.h"

class CDisplay;
class CLed;
class CLinePtr;
class CTxtStory;
class CTxtEdit;
class CRchTxtPtr;
class CTxtRange;
class CTxtSelection;

#define INVALID_ZOOM_DENOMINATOR 0

// Auto scroll timing
#define cmsecScrollDelay	500
#define cmsecScrollInterval	50


class CDrawInfo;

// ============================  CLed  ====================================
// Line Edit Descriptor - describes impact of an edit on line breaks

class CLed
{
public:
	LONG _cpFirst;			// cp of first affected line
	LONG _iliFirst;			// index of first affected line
	LONG _yFirst;			// y offset of first affected line

	LONG _cpMatchOld;		// pre-edit cp of first matched line
	LONG _iliMatchOld;		// pre-edit index of first matched line
	LONG _yMatchOld;		// pre-edit y offset of first matched line

	LONG _cpMatchNew;		// post-edit cp of first matched line
	LONG _iliMatchNew;		// post-edit index of first matched line
	LONG _yMatchNew;		// post-edit y offset of bottom of first matched line
	LONG _yMatchNewTop;		// post-edit y offset of top of first matched line

public:
	CLed();
	
	VOID	SetMax(const CDisplay * const pdp);
};

inline CLed::CLed()
{
#ifdef DEBUG
	// We set this invalid so that we can assert on it in debug builds.
	_yMatchNewTop = -1;

#endif // DEBUG
}

// An enumeration describing the various display actions we can perform on the selection
enum SELDISPLAYACTION
{
	selSetHiLite,
	selSetNormal,
	selUpdateHiLite,
	selUpdateNormal
};

// Forward declaration to prevent recursion of definitions
class CAccumDisplayChanges;

// ==========================  CDisplay  ====================================
// display - keeps track of line breaks for a device
// all measurements are in pixels on rendering device,
// EXCEPT xWidthMax and yHeightMax which are in twips

class CDisplay : public CDevDesc, public ITxNotify
{
	friend class CLinePtr;
	friend class CLed;

#ifdef DEBUG
public:
	BOOL Invariant ( void ) const;
private:
#endif

public:
	// average char width of system font
	static INT GetXWidthSys() { return sysparam.GetXWidthSys(); }

	// height of system font
	static INT GetYHeightSys() { return sysparam.GetYHeightSys(); }

private:
	static DWORD _dwTimeScrollNext; // time for next scroll step
	static DWORD _dwScrollLast;	// last scroll action
	
	CDrawInfo *	 _pdi;			// Draw info parameters

protected:
	CAccumDisplayChanges *_padc;// Accumulated display changes if frozen

	USHORT _fBgndRecalc				:1; // background recalc is running
	USHORT _fDeferUpdateScrollBar	:1; // currently defering updating scroll bars
	USHORT _fHScrollEnabled			:1; // horizontal scrolling enabled
	USHORT _fInBkgndRecalc			:1; // avoid reentrant background recalc
	USHORT _fLineRecalcErr			:1; // error occured during background recalc
	USHORT _fNoUpdateView			:1; // don't update visible view
	USHORT _fWordWrap				:1; // word wrap text
	USHORT _fNeedRecalc				:1; // recalc line is needed
	USHORT _fRecalcDone				:1; // is line recalc done ?
	USHORT _fViewChanged			:1; // visible view rectangle has changed since last Draw
	USHORT _fUpdateScrollBarDeferred:1; // scroll bars need to be updated
	USHORT _fVScrollEnabled			:1; // vertical scrolling enabled
	USHORT _fUpdateCaret			:1; // Whether Draw needs to update the cursor
	USHORT _fActive					:1; // Whether this display is active
	USHORT _fUpdateOffScreen		:1;	// Whether we should do an off screen 
										// update. Note, this is currenly only
										// used by ML. However, it saves space
										// to put it here.
	USHORT _fRectInvalid			:1; // Entire client rectangle has been 
										// invalidated. Used only in SL. Put
										// here, as usual, to save space.
	USHORT _fSmoothVScroll			:1;	// timer for smooth scrolling installed.
	USHORT _fFinishSmoothVScroll	:1;	// TRUE if we're winding down the current smooth scroll.

	SHORT   _xWidthView;	  		// view rect width
	LONG	_yHeightView;	 		// view rect height
	LONG	_yHeightClient;   		// height of client rect unmodified by inset.

	LONG	_xScroll;		 		// horizontal scroll position of visible view

	LONG	_lTempZoomDenominator;	// Zoom for GetNaturalSize
	LONG	_cpFirstVisible;		// cp at start of first visible line

 	// smooth scroll support.
	int			_smoothYDelta;			// Current # pixels * 1000 to smooth scroll by.
	int			_continuedSmoothYDelta;	// At end of one smooth scroll cycle, start new with this.
	int			_nextSmoothVScroll;		// Maintains fractional amount not yet smooth scrolled.
	int			_totalSmoothVScroll;	// Remaining number of device units to smooth scroll.
	int			_continuedSmoothVScroll;// At end of one smooth scroll cycle, start new with this.

private:

	void 	UpdateViewRectState(const RECT *prcClient);

	LONG	GetSelBarInPixels();

protected:
	
	friend class CLinePtr;

	LONG			SetClientHeight(LONG yNewClientHeight);
	
	virtual void	InitLinePtr ( CLinePtr & ) = 0;
	
	// Line break recalc.
	virtual BOOL	RecalcView(BOOL fUpdateScrollBars) = 0;

	// Rendering
	virtual VOID	Render(const RECT &rcView, const RECT &rcRender) = 0;

	// Scrollbar
	virtual BOOL	UpdateScrollBar(INT nBar, BOOL fUpdateRange = FALSE) = 0;

	void			GetViewDim(LONG& widthView, LONG& heightView);

	void			SetCpFirstVisible(LONG cp)		{_cpFirstVisible = cp;};

	LONG			ConvertScrollToXPos(LONG xPos);

	LONG			ConvertXPosToScrollPos(LONG xPos);

	virtual LONG	GetMaxXScroll() const = 0;

public:
	virtual	LONG	ConvertYPosToScrollPos(LONG yPos);

			CDisplay (CTxtEdit* ped);
	virtual CDisplay::~CDisplay();

	virtual BOOL	Init();
			void	InitFromDisplay(const CDisplay *pdp);

	// Device context management
	virtual BOOL	SetMainTargetDC(HDC hdc, LONG xWidthMax);
	virtual BOOL	SetTargetDC( HDC hdc );

	// Getting properties
			CTxtEdit*		GetED() const			{ return _ped;}
			CTxtStory*		GetStory() const		{ return _ped->GetTxtStory();}
			const CDevDesc*	GetDdRender() const		{ return this;}
	virtual const CDevDesc*	GetDdTarget() const		{ return NULL; }
			const CDevDesc*	GetTargetDev() const;
	
	virtual BOOL	IsMain() const = 0;
	virtual BOOL	IsPrinter() const;
			BOOL	IsRecalcDone() const			{ return _fRecalcDone;}
	virtual BOOL	GetWordWrap() const;
			void	SetWordWrap(BOOL fNoWrap);

			HRESULT	GetCachedSize(DWORD *pdwWidth, DWORD *pdwHeight);


	// maximum height and width
	virtual LONG	GetMaxWidth() const = 0;
	virtual LONG	GetMaxHeight() const = 0;
	virtual LONG 	GetMaxPixelWidth() const = 0;

	// Width, height and line count (all text)
	virtual LONG	GetWidth() const = 0;
	virtual LONG	GetHeight() const = 0;
	virtual LONG	GetResizeHeight() const = 0;
	virtual LONG	LineCount() const = 0;

	// View rectangle
			void	GetViewRect(RECT &rcView, LPCRECT prcClient = NULL);
			LONG	GetViewWidth() const			{ return _xWidthView;}
			LONG	GetViewHeight() const			{ return _yHeightView;}

	// Visible view properties
	virtual LONG	GetCliVisible(
						LONG *pcpMostVisible = NULL,
						BOOL fLastCharOfLastVisible = FALSE) const = 0;

			LONG	GetFirstVisibleCp() const		{return _cpFirstVisible;};
	virtual LONG	GetFirstVisibleLine() const = 0;

	// Line info
	virtual LONG	GetLineText(LONG ili, TCHAR *pchBuff, LONG cchMost) = 0;
	virtual LONG	CpFromLine(LONG ili, LONG *pyLine = NULL) = 0;
	
	virtual LONG	LineFromCp(LONG cp, BOOL fAtEnd) = 0;

	// Point <-> cp conversion
	virtual LONG	CpFromPoint(POINT pt, 
						const RECT *prcClient,
						CRchTxtPtr * const ptp, 
						CLinePtr * const prp, 
						BOOL fAllowEOL,
						HITTEST *pHit = NULL) = 0;

	virtual LONG	PointFromTp (
						const CRchTxtPtr &tp, 
						const RECT *prcClient,
						BOOL fAtEnd,	
						POINT &pt,
						CLinePtr * const prp, 
						UINT taMode) = 0;

	// View recalc and updating
			VOID	SetUpdateCaret() { _fUpdateCaret = TRUE; }
			VOID	SetViewChanged() { _fViewChanged = TRUE; }
			VOID	InvalidateRecalc()			  {_fNeedRecalc = TRUE;}
			BOOL	RecalcView (const RECT &rcView);
			BOOL	UpdateView();
	virtual BOOL	UpdateView(const CRchTxtPtr &tpFirst, LONG cchOld, LONG cchNew) = 0;

	// Rendering
			VOID	EraseBkgnd(HDC hdc);
			HRESULT Draw(HDC hicTargetDev,
						 HDC hdcDraw,
						 LPCRECT prcClient,
						 LPCRECT prcWBounds,
						 LPCRECT prcUpdate,
						 BOOL (CALLBACK * pfnContinue) (DWORD),
						 DWORD dwContinue);

	// Background recalc
	virtual VOID	StepBackgroundRecalc();
	virtual BOOL	WaitForRecalc(LONG cpMax, LONG yMax);
	virtual BOOL	WaitForRecalcIli(LONG ili);
	virtual BOOL	WaitForRecalcView();

	// Scrolling 
			LONG	GetXScroll() const			  {return _xScroll;} 
	virtual LONG	GetYScroll() const			  {return 0;}
			VOID	HScroll(WORD wCode, LONG xPos);
	virtual LRESULT VScroll(WORD wCode, LONG yPos);
	virtual VOID	LineScroll(LONG cli, LONG cch);
	virtual VOID	FractionalScrollView ( LONG yDelta );
	virtual VOID	ScrollToLineStart(LONG iDirection);
	virtual LONG	CalcYLineScrollDelta ( LONG cli, BOOL fFractionalFirst );
			BOOL	DragScroll(const POINT * ppt);	 // outside of client rect.
			BOOL	AutoScroll( POINT pt, const WORD xScrollInset, const WORD yScrollInset );
	virtual BOOL	ScrollView(LONG xScroll, LONG yScroll, BOOL fTracking, BOOL fFractionalScroll) = 0;
	virtual	LONG	AdjustToDisplayLastLine(LONG yBase,	LONG yScroll);

     // Smooth Scrolling 
			VOID	SmoothVScroll ( int direction, WORD cLines, int speedNum, int speedDenom, BOOL fMouseRoller );
			VOID	SmoothVScrollUpdate();
			BOOL	CheckInstallSmoothVScroll();
			VOID	CheckRemoveSmoothVScroll();
			VOID	FinishSmoothVScroll();
			BOOL	IsSmoothVScolling() { return _fSmoothVScroll; }

	// Scrollbars
	virtual LONG	GetScrollRange(INT nBar) const;
			BOOL	IsHScrollEnabled();
			BOOL	IsVScrollEnabled() {return _fVScrollEnabled; }

	// Resizing
			VOID	OnClientRectChange(const RECT &rcClient);
			VOID	OnViewRectChange(const RECT &rcView);
			HRESULT RequestResize();

	// Selection 
	virtual BOOL	InvertRange(LONG cp,
		                        LONG cch,
								SELDISPLAYACTION selAction) = 0;

	// Natural size calculation
	virtual HRESULT	GetNaturalSize(
						HDC hdcDraw,
						HDC hicTarget,
						DWORD dwMode,
						LONG *pwidth,
						LONG *pheight) = 0;

	LONG			GetZoomDenominator() const;
	LONG			GetZoomNumerator() const;
	LONG			Zoom(LONG x) const;
	LONG			UnZoom(LONG x) const;

	LONG		 	HimetricXtoDX(LONG xHimetric) const;
	LONG		 	HimetricYtoDY(LONG yHimetric) const;
	LONG			DXtoHimetricX(LONG dx)  const;
	LONG			DYtoHimetricY(LONG dy) const;

	void 			ReDrawOnRectChange(HDC hicTarget, const RECT *prcClient);

	HRESULT 		TransparentHitTest(
						HDC hdc,
						LPCRECT prcClient,
						POINT pt,
						DWORD *pHitResult);

	HRESULT 		RoundToLine(HDC hdc, LONG width, LONG *pheight);

	void			SetTempZoomDenominator(LONG lZoomDenominator)
					{ 
						_lTempZoomDenominator = lZoomDenominator;
					}

	LONG			GetTempZoomDenominator()
					{ 
						return _lTempZoomDenominator;
					}

	void			ResetTempZoomDenominator() 
					{ 
						_lTempZoomDenominator = INVALID_ZOOM_DENOMINATOR;
					}

	void			SetDrawInfo(
						CDrawInfo *pdi, 
						DWORD dwDrawAspect,	//@parm draw aspect
						LONG  lindex,		//@parm currently unused
						void *pvAspect,		//@parm info for drawing optimizations (OCX 96)
						DVTARGETDEVICE *ptd,//@parm information on target device								
						HDC hicTargetDev);	//@parm	target information context

	void			ReleaseDrawInfo();

	void 			ResetDrawInfo(const CDisplay *pdp);

	DWORD 			GetDrawAspect() const;

	LONG 			GetLindex() const;

	void *			GetAspect() const;

	DVTARGETDEVICE *GetPtd() const;

	void			SetActiveFlag(BOOL fActive) { _fActive = fActive; }

	BOOL			IsActive()	{ return _fActive; }

	virtual CDisplay *Clone() const = 0;
	
	LONG			ModeOffsetIntoChar(
						LONG taMode,
						const CRchTxtPtr& tp);

	// Support for freezing the display
	BOOL			IsFrozen();

	void			SaveUpdateCaret(BOOL fScrollIntoView);

	void			Freeze();

	void			Thaw();

	//
	// ITxNotify Interface
	//
	virtual void 	OnPreReplaceRange( 
						DWORD cp, 
						DWORD cchDel, 
						DWORD cchNew,
						DWORD cpFormatMin, DWORD cpFormatMax);

	virtual void 	OnPostReplaceRange( 
						DWORD cp, 
						DWORD cchDel, 
						DWORD cchNew,
						DWORD cpFormatMin, 
						DWORD cpFormatMax);

	virtual void	Zombie();
};

// Defines the draw info class. It is included here to prevent loops
// in dependencies that would require no inlining for functions dealing
// with this
#include	"_drwinfo.h"

/*
 *	CDisplay::ResetDrawInfo
 *
 *	@mfunc	Sets draw info using different display
 *
 *	@rdesc	void
 *
 */
inline void CDisplay::ResetDrawInfo(
	const CDisplay *pdp)	//@parm Display to use for draw information
{
	_pdi = pdp->_pdi;
}

/*
 *	CDisplay::ResetDrawInfo
 *
 *	@mfunc	Gets lindex as passed most recently from the host.
 *
 *	@rdesc	draw aspect
 *
 */
inline DWORD CDisplay::GetDrawAspect() const
{
	return (_pdi != NULL) ? _pdi->GetDrawAspect() : DVASPECT_CONTENT; 
}

/*
 *	CDisplay::GetLindex
 *
 *	@mfunc	Gets lindex as passed most recently from the host.
 *
 *	@rdesc	lindex
 *
 */
inline LONG CDisplay::GetLindex() const
{
	return (_pdi != NULL) ? _pdi->GetLindex() : -1; 
}

/*
 *	CDisplay::GetAspect
 *
 *	@mfunc	Gets aspect as passed most recently from the host.
 *
 *	@rdesc	Aspect data
 *
 */
inline void *CDisplay::GetAspect() const
{
	return (_pdi != NULL) ? _pdi->GetAspect() : NULL; 
}

/*
 *	CDisplay::GetPtd
 *
 *	@mfunc	Gets device target as passed most recently from the host.
 *
 *	@rdesc	DVTARGETDEVICE or NULL
 *
 */
inline DVTARGETDEVICE *CDisplay::GetPtd() const
{
	return (_pdi != NULL) ? _pdi->GetPtd() : NULL; 
}

/*
 *	CDisplay::IsFrozen
 *
 *	@mfunc	Return whether display is currently frozen
 *
 *	@rdesc	
 *		TRUE - display is frozen <nl>
 *		FALSE - display is not frozen
 *
 */
inline BOOL CDisplay::IsFrozen()
{
	return _padc != NULL;
}


/*
 *	CFreezeDisplay
 *	
 * 	@class	This class is used to freeze and guranatee that the display
 *			unfreeze a display when it passes out of its context.
 *
 *
 */
class CFreezeDisplay
{
public:	
						CFreezeDisplay(CDisplay *pdp); //@cmember Constructor Freezes

						~CFreezeDisplay();			//@cmember Destructor - Thaws

private:

	CDisplay *			_pdp;						//@cmember Display to freeze
};

/*
 *	CFreezeDisplay::CFreezeDisplay()
 *
 *	@mfunc
 *		Initialize object and tell the input display to freeze
 *
 *	@rdesc
 *		void
 *
 */
inline CFreezeDisplay::CFreezeDisplay(CDisplay *pdp) : _pdp(pdp)
{
	pdp->Freeze();
}

/*
 *	CFreezeDisplay::CFreezeDisplay()
 *
 *	@mfunc
 *		Free object and tell display to thaw.
 *
 *	@rdesc
 *		void
 *
 */
inline CFreezeDisplay::~CFreezeDisplay()
{
	_pdp->Thaw();
}


#endif

