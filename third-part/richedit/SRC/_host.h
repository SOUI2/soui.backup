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
/*	@doc INTERNAL
 *
 *	@module _HOST.H  Text Host for Window's Rich Edit Control |
 *	
 *
 *	Original Author: <nl>
 *		Christian Fortini
 *		Murray Sargent
 *
 *	History: <nl>
 *		8/1/95	ricksa	Revised interface definition
 */
#ifndef _HOST_H
#define _HOST_H

#include "textserv.h"

/*
 *	TXTEFFECT
 *
 *	@enum	Defines different background styles control
 */
enum TXTEFFECT {
	TXTEFFECT_NONE = 0,				//@emem	no special backgoround effect
	TXTEFFECT_SUNKEN,				//@emem	draw a "sunken 3-D" look
};


// @doc EXTERNAL 

// ============================  CTxtWinHost  ================================================
// Implement the windowed version of the Plain Text control

/*
 *	CTxtWinHost
 *	
 * 	@class	Text Host for Window's Rich Edit Control implementation class
 *
 *
 */
class CTxtWinHost : public ITextHost2
{
protected:
    static LONG _xWidthSys;
    static LONG _yHeightSys;

protected:
	HWND		_hwnd;					// control window
	HWND		_hwndParent;			// parent window

	ITextServices	*_pserv;			// pointer to Text Services object

	ULONG		_crefs;					// reference count

// Properties

	DWORD		_dwStyle;				// style bits
	DWORD		_dwExStyle;				// extended style bits

	unsigned	_fBorder			;	// control has border
	unsigned	_fInBottomless		;	// inside bottomless callback
	unsigned	_fInDialogBox		;	// control is in a dialog box
	unsigned	_fEnableAutoWordSel	;	// enable Word style auto word selection?
	unsigned	_fIconic			;	// control window is iconic
	unsigned	_fHidden			;	// control window is hidden
	unsigned	_fNotSysBkgnd		;	// not using system background color
	unsigned	_fWindowLocked		;	// window is locked (no update)
	unsigned	_fRegisteredForDrop	;   // whether host has registered for drop
	unsigned	_fVisible			;	// Whether window is visible or not.
	unsigned	_fResized			;	// resized while hidden
	unsigned	_fDisabled			;	// Window is disabled.
	unsigned	_fKeyMaskSet		;	// if ENM_KEYEVENTS has been set
	unsigned	_fMouseMaskSet		;	// if ENM_MOUSEEVENTS has been set
	unsigned	_fScrollMaskSet		;	// if ENM_SCROLLEVENTS has been set
	unsigned	_fUseSpecialSetSel	;   // TRUE = use EM_SETSEL hack to not select
										// empty controls to make dialog boxes work.
	unsigned	_fEmSetRectCalled	;	// TRUE - application called EM_SETRECT
	unsigned	_fAccumulateDBC		;	// TRUE - need to cumulate ytes from 2 WM_CHAR msgs
										// we are in this mode when we receive VK_PROCESSKEY

	TCHAR		_chPassword;			// Password char. If null, no password
	COLORREF 	_crBackground;			// background color
    RECT        _rcViewInset;           // view rect inset /r client rect

	HIMC		_oldhimc;				// previous IME Context
	DWORD		_usIMEMode;				// mode of IME operation
										// either 0 or ES_SELFIME or ES_NOIME
	LONG		_yInset;
	LONG		_xInset;

	HPALETTE	_hpal;					// Logical palette to use.

	TCHAR		_chLeadByte;			// use when we are in _fAccumulateDBC mode


protected:
	// Initialization
	BOOL	Init(HWND hwnd, const CREATESTRUCT *pcs);

	void	ResizeInset();

	void	SetScrollBarsForWmEnable(BOOL fEnable);


	void	OnSetMargins(
				DWORD fwMargin,
				DWORD xLeft,
				DWORD xRight);

	void	SetScrollInfo(
				INT fnBar,
				BOOL fRedraw);

	// helpers
	void *	CreateNmhdr(UINT uiCode, LONG cb);
	void	RevokeDragDrop(void);
	void	RegisterDragDrop(void);
	void	DrawSunkenBorder(HWND hwnd, HDC hdc);
	VOID    OnSunkenWindowPosChanging(HWND hwnd, WINDOWPOS *pwndpos);
	LRESULT OnSize(HWND hwnd, WORD fwSizeType, int nWidth, int nHeight);
	TXTEFFECT TxGetEffects() const;
	HRESULT	OnTxVisibleChange(BOOL fVisible);
	void	SetDefaultInset();
	void	ImmAssociateNULLContext(BOOL fReadOnly);
	BOOL	IsTransparentMode() 
			{
				return (_dwExStyle & WS_EX_TRANSPARENT);
			}


	// Keyboard messages
	LRESULT	OnKeyDown(WORD vKey, DWORD dwFlags);
	LRESULT	OnChar(WORD vKey, DWORD dwFlags);
	
	// System notifications
	void 	OnSysColorChange();
	LRESULT OnGetDlgCode(WPARAM wparam, LPARAM lparam);

	// Other messages
	LRESULT OnGetOptions() const;
	void	OnSetOptions(WORD wOp, DWORD eco);
	void	OnSetReadOnly(BOOL fReadOnly);
	void	OnGetRect(LPRECT prc);
	void	OnSetRect(LPRECT prc, BOOL fNewBehavior);


public:
	CTxtWinHost();
	~CTxtWinHost();
	void	Shutdown();

	// Window creation/destruction
	static 	LRESULT OnNCCreate(HWND hwnd, const CREATESTRUCT *pcs);
	static 	void 	OnNCDestroy(CTxtWinHost *ped);
			LRESULT OnCreate(const CREATESTRUCT *pcs);

	// Window proc
	virtual LRESULT	TxWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

 	// Accumulate two WM_CHARs for ANSI DBC 
	BOOL	IsAccumulateDBCMode() { return _fAccumulateDBC; }

	// Set/Clear Cumulate mode
	void	SetAccumulateDBCMode(WORD fSetClearMode) { _fAccumulateDBC = fSetClearMode; }

	// -----------------------------
	//	IUnknown interface
	// -----------------------------

    virtual HRESULT 		WINAPI QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG 			WINAPI AddRef(void);
    virtual ULONG 			WINAPI Release(void);

	// -----------------------------
	//	ITextHost interface
	// -----------------------------
	//@cmember Get the DC for the host
	virtual HDC 		TxGetDC();

	//@cmember Release the DC gotten from the host
	virtual INT			TxReleaseDC(HDC hdc);
	
	//@cmember Show the scroll bar
	virtual BOOL 		TxShowScrollBar(INT fnBar, BOOL fShow);

	//@cmember Enable the scroll bar
	virtual BOOL 		TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);

	//@cmember Set the scroll range
	virtual BOOL 		TxSetScrollRange(
							INT fnBar, 
							LONG nMinPos, 
							INT nMaxPos, 
							BOOL fRedraw);

	//@cmember Set the scroll position
	virtual BOOL 		TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);

	//@cmember InvalidateRect
	virtual void		TxInvalidateRect(LPCRECT prc, BOOL fMode);

	//@cmember Send a WM_PAINT to the window
	virtual void 		TxViewChange(BOOL fUpdate);
	
	//@cmember Create the caret
	virtual BOOL		TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);

	//@cmember Show the caret
	virtual BOOL		TxShowCaret(BOOL fShow);

	//@cmember Set the caret position
	virtual BOOL		TxSetCaretPos(INT x, INT y);

	//@cmember Create a timer with the specified timeout
	virtual BOOL 		TxSetTimer(UINT idTimer, UINT uTimeout);

	//@cmember Destroy a timer
	virtual void 		TxKillTimer(UINT idTimer);

	//@cmember Scroll the content of the specified window's client area
	virtual void		TxScrollWindowEx (
							INT dx, 
							INT dy, 
							LPCRECT lprcScroll, 
							LPCRECT lprcClip,
							HRGN hrgnUpdate, 
							LPRECT lprcUpdate, 
							UINT fuScroll);
	
	//@cmember Get mouse capture
	virtual void		TxSetCapture(BOOL fCapture);

	//@cmember Set the focus to the text window
	virtual void		TxSetFocus();

	//@cmember Establish a new cursor shape
	virtual void 		TxSetCursor(HCURSOR hcur, BOOL fText);

	//@cmember Converts screen coordinates of a specified point to the client coordinates 
	virtual BOOL 		TxScreenToClient (LPPOINT lppt);

	//@cmember Converts the client coordinates of a specified point to screen coordinates
	virtual BOOL		TxClientToScreen (LPPOINT lppt);

	//@cmember Request host to activate text services
	virtual HRESULT		TxActivate( LONG * plOldState );

	//@cmember Request host to deactivate text services
   	virtual HRESULT		TxDeactivate( LONG lNewState );

	//@cmember Retrieves the coordinates of a window's client area
	virtual HRESULT		TxGetClientRect(LPRECT prc);

	//@cmember Get the view rectangle relative to the inset
	virtual HRESULT		TxGetViewInset(LPRECT prc);

	//@cmember Get the default character format for the text
	virtual HRESULT 	TxGetCharFormat(const CHARFORMAT **ppCF );

	//@cmember Get the default paragraph format for the text
	virtual HRESULT		TxGetParaFormat(const PARAFORMAT **ppPF);

	//@cmember Get the background color for the window
	virtual COLORREF	TxGetSysColor(int nIndex);

	//@cmember Get the background (either opaque or transparent)
	virtual HRESULT		TxGetBackStyle(TXTBACKSTYLE *pstyle);

	//@cmember Get the maximum length for the text
	virtual HRESULT		TxGetMaxLength(DWORD *plength);

	//@cmember Get the bits representing requested scroll bars for the window
	virtual HRESULT		TxGetScrollBars(DWORD *pdwScrollBar);

	//@cmember Get the character to display for password input
	virtual HRESULT		TxGetPasswordChar(TCHAR *pch);

	//@cmember Get the accelerator character
	virtual HRESULT		TxGetAcceleratorPos(LONG *pcp);

	//@cmember Get the native size
    virtual HRESULT		TxGetExtent(LPSIZEL lpExtent);

	//@cmember Notify host that default character format has changed
	virtual HRESULT 	OnTxCharFormatChange (const CHARFORMAT * pcf);

	//@cmember Notify host that default paragraph format has changed
	virtual HRESULT		OnTxParaFormatChange (const PARAFORMAT * ppf);

	//@cmember Bulk access to bit properties
	virtual HRESULT		TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);

	//@cmember Notify host of events
	virtual HRESULT		TxNotify(DWORD iNotify, void *pv);

	// FE Support Routines for handling the Input Method Context
//#ifdef WIN95_IME
	virtual HIMC		TxImmGetContext(void);
	virtual void		TxImmReleaseContext(HIMC himc);
//#endif

	//@cmember Returns HIMETRIC size of the control bar.
	virtual HRESULT		TxGetSelectionBarWidth (LONG *lSelBarWidth);

	// ITextHost2 methods
	virtual BOOL		TxIsDoubleClickPending();
	virtual HRESULT		TxGetWindow(HWND *phwnd);
	virtual HRESULT		TxSetForegroundWindow();
	virtual HPALETTE	TxGetPalette();
};


#ifndef DEBUG
#define AttCheckRunTotals(_fCF)
#define AttCheckPFRuns(_fCF)
#endif

#define ECO_STYLES (ECO_AUTOVSCROLL | ECO_AUTOHSCROLL | ECO_NOHIDESEL | \
						ECO_READONLY | ECO_WANTRETURN | ECO_SAVESEL | \
						ECO_SELECTIONBAR | ES_NOIME | ES_SELFIME )

#endif // _HOST_H

