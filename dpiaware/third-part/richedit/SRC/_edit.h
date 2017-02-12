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
/*	_EDIT.H
 *	
 *	Purpose:
 *		Base classes for rich-text manipulation
 *	
 *	Authors:
 *		Christian Fortini
 *		Murray Sargent
 *
 */

#ifndef _EDIT_H
#define _EDIT_H

#include "textserv.h"
#include "_ldte.h"
#include "_m_undo.h"
#include "_notmgr.h"
#include "_doc.h"
#include "_objmgr.h"
#include "_cfpf.h"
#include "_callmgr.h"
#include "_magelln.h"


// Forward declarations
class CRchTxtPtr;
class CTxtSelection;
class CTxtStory;
class CTxtUndo;
class CMeasurer;
class CRenderer;
class CDisplay;
class CDisplayPrinter;
class CDrawInfo;
class CDetectURL;
class CBiDiEngine;

// Macro for finding parent "this" of embedded class. If this turns out to be
// globally useful we should move it to _common.h. 
#define GETPPARENT(pmemb, struc, membname) (\
				(struc FAR *)(((char FAR *)(pmemb))-offsetof(struc, membname)))

// These wonderful constants are for backward compatibility. They are the 
// sizes used for initialization and reset in RichEdit 1.0
const LONG cInitTextMax	 = (32 * 1024) - 1;
const LONG cResetTextMax = (64 * 1024);

// NB! Global variable that tells us wether we're on a BiDi system or not  	
extern WORD		g_wLang;				// Current LID of the system:
										// Arabic, Hebrew or "nonBiDi"
#define IsBiDiSystem()	(g_wLang != sLanguageNonBiDi)

extern WORD g_wFlags;								// Toggled by Ctrl-"
#define	KF_SMARTQUOTES	0x0001						// Enable smart quotes
#define SmartQuotesEnabled()	(g_wFlags & KF_SMARTQUOTES)

struct SPrintControl
{
	union
	{
		DWORD		_dwAllFlags;				// Make it easy to set all flags at once.

		struct
		{
			ULONG	_fDoPrint:1;				// Whether actual print is required
			ULONG	_fPrintFromDraw:1;			// Whether draw is being used to print
		};
	};

	SPrintControl(void) { _dwAllFlags = 0; }
};

enum DOCUMENTTYPE
{
	DT_LTRDOC	= 1,			// DT_LTRDOC and DT_RTLDOC are mutually
	DT_RTLDOC	= 2,			//  exclusive
};

class CDocInfo					// Contains ITextDocument info
{
public:
	BSTR	pName;				// Document filename
	HANDLE	hFile;				// Handle used unless full file sharing
	WORD	wFlags;				// Open, share, create, and save flags
	WORD	wCpg;				// Code page
	LONG	dwDefaultTabStop;	// TOM settable default tab stop
	LCID	lcid;				// Document lcid (for RTF \deflang)
	LCID	lcidfe;				// Document FE lcid (for RTF \deflangfe)
	LPWSTR	lpstrLeadingPunct;	// Leading kinsoku characters
	LPWSTR	lpstrFollowingPunct;// Following kinsoku characters
	BYTE	bDocType;			// 0-1-2: export none-\ltrdoc-\rtldoc
								// If 0x80 or'd in, PWD instead of RTF

	CDocInfo() {InitDocInfo();}	// constructor
	~CDocInfo();				// destructor

	void	InitDocInfo();
};

const DWORD tomInvalidCpg = 0xFFFF;
const DWORD tomInvalidLCID = 0xFFFE;

#define tomOutline 1

// This depends on the number of property bits defined in textserv.h. However, this is
// for private use by the text services so it is defined here.
#define MAX_PROPERTY_BITS	21
#define SPF_SETDEFAULT		4
#define	SFF_ADJUSTENDEOP	0x80000000
#define SFF_KEEPDOCINFO		0x40000000

// IDispatch global declarations
extern ITypeInfo *	g_pTypeInfoDoc;
extern ITypeInfo *	g_pTypeInfoSel;
extern ITypeInfo *	g_pTypeInfoFont;
extern ITypeInfo *	g_pTypeInfoPara;
HRESULT GetTypeInfoPtrs();
HRESULT GetTypeInfo(UINT iTypeInfo, ITypeInfo *&pTypeInfo,
							ITypeInfo **ppTypeInfo);

LONG Cleanse(TCHAR * pchD, const TCHAR * pchS, LONG cchS);

/*
 *	@enum	flags indicating how protection should be dealt with
 *			in CTxtEdit::SetText
 */
enum SetTextFlags
{
	CHECK_PROTECTION = 1,	//@emem	check for protection
	IGNORE_PROTECTION = 2,	//@emem ignore any protection (even if found)
};

enum AccentIndices
{
	ACCENT_GRAVE = 1,
	ACCENT_ACUTE,
	ACCENT_CARET,
	ACCENT_TILDE,
	ACCENT_UMLAUT,
	ACCENT_CEDILLA
};

// ==================================  CTxtEdit  ============================================
// Outer most class for a Text Control.

class CTxtEdit : public ITextServices, public IRichEditOle, public ITextDocument
{
public:
	friend class CCallMgr;
	friend class CMagellanBMPStateWrap;
	friend void CheckDestroyIME ( CTxtEdit &ed );

	CTxtEdit(ITextHost *phost, IUnknown *punkOuter);
	 ~CTxtEdit ();

	// Initialization 
	BOOL 		Init(const RECT *prcClient);

	// A helper function
	DWORD GetTextLength() const	{return _story.GetTextLength();}
	DWORD GetAdjustedTextLength();
   
	// Access to ActiveObject members

	IUnknown *		GetPrivateIUnknown() 	{ return &_unk; }
	CLightDTEngine *GetDTE() 				{ return &_ldte; }

	IUndoMgr *		GetUndoMgr() 			{ return _pundo; }
	IUndoMgr *		GetRedoMgr()			{ return _predo; }
	IUndoMgr *		CreateUndoMgr(DWORD dwLim, USFlags flags);
	CCallMgr *		GetCallMgr()			{ 
												Assert(_pcallmgr);
												return _pcallmgr;
											}	

	CObjectMgr *	GetObjectMgr();
					// the callback is provided by the client
					// to help with OLE support
	BOOL			HasObjects()			{return !!_pobjmgr;}
	IRichEditOleCallback *GetRECallback()
		{ return _pobjmgr ? _pobjmgr->GetRECallback() : NULL; }	
	LRESULT 		HandleSetUndoLimit(DWORD dwLim);
	LRESULT			HandleSetTextMode(DWORD mode);		

	CNotifyMgr *	GetNotifyMgr();

	CDetectURL *	GetDetectURL()			{return _pdetecturl;}

	CBiDiEngine *	GetBiDiEngine()			{return NULL;}
	BOOL			IsBiDiEngineValid()		{return _pBiDiEngine != NULL;}

#if !defined(NOMAGELLAN)
	CMagellan		mouse;
	LRESULT			HandleMouseWheel(WPARAM wparam, LPARAM lparam);
#endif

	// Misc helpers
	BOOL			Get10Mode()					{return _f10Mode;}
	LONG			GetCpAccelerator() const	{return _cpAccelerator;}
	BOOL			fInOurHost() const			{return _fInOurHost;}
	BOOL			fInplaceActive() const		{return _fInPlaceActive;}
	BOOL			fHideSelection() const		{return _fHideSelection;}
	BOOL			fUsePassword() const		{return _fUsePassword;}
	BOOL			IsInOutlineView() const		{return _fOutlineView;}
	BOOL			IsMouseDown() const			{return _fMouseDown;}
	BOOL			IsRich() const				{return _fRich;}
	LONG			GetZoomNumerator() const	{return _wZoomNumerator;}
	LONG			GetZoomDenominator() const	{return _wZoomDenominator;}
	void			SetZoomNumerator(LONG x)	{_wZoomNumerator = x;}
	void			SetZoomDenominator(LONG x)	{_wZoomDenominator = x;}

	void			CheckUnicode(LONG lStreamFormat);
	void			HandleKbdContextMenu(void);
	void			Set10Mode();
	void			Sound();
	HRESULT			UpdateAccelerator();
	HRESULT			UpdateOutline();

	HRESULT			PopAndExecuteAntiEvent(
						IUndoMgr *pundomgr, 
						DWORD dwDoToCookie);

	HRESULT			CutOrCopySelection(UINT msg, WPARAM wparam, LPARAM lparam,
									   IUndoBuilder *publdr);

	HRESULT			PasteDataObjectToRange( 
						IDataObject *pdo, 
						CTxtRange *prg, 
						CLIPFORMAT cf, 
						REPASTESPECIAL *rps,
						IUndoBuilder *publdr, 
						DWORD dwFlags );

	HRESULT			MoveSelection(
						LPARAM lparam,
						IUndoBuilder *publdr);

	HDC				CreateMeasureDC(
						HDC hdcMetaFile,
						const RECT *prcClient,
						BOOL fUseTwips,
						LONG xWindowOrg,
						LONG yWindowOrg,
						LONG xWindowExt,
						LONG yWindowExt,
						LONG *pxPerInch,
						LONG *pyPerInch);

	void			SetContextDirection();

	// Story access
	CTxtStory * GetTxtStory () { return &_story; }

	// Get access to cached CCharFormat and CParaFormat structures
	const CCharFormat* 	GetCharFormat(LONG icf);
	const CParaFormat* 	GetParaFormat(LONG ipf);

	HRESULT		HandleStyle(CCharFormat *pCFTarget, const CCharFormat *pCF);
	HRESULT		HandleStyle(CParaFormat *pPFTarget, const CParaFormat *pPF);

	// Get the host interface pointer
	ITextHost *	GetHost() { return _phost; }

	// Helper for getting the CDocInfo ptr and creating it if it's NULL 
	CDocInfo *	GetDocInfo();
	HRESULT		InitDocInfo();

	LONG		GetDefaultTab()	
					{return _pDocInfo ? _pDocInfo->dwDefaultTabStop : lDefaultTab;};
	HRESULT		SetDefaultLCID	 (LCID lcid);
	HRESULT		GetDefaultLCID	 (LCID *pLCID);
	HRESULT		SetDefaultLCIDFE (LCID lcid);
	HRESULT		GetDefaultLCIDFE (LCID *pLCID);
	HRESULT		SetDocumentType  (LONG DocType);
	HRESULT		GetDocumentType  (LONG *pDocType);
	HRESULT		GetFollowingPunct(LPWSTR *plpstrFollowingPunct);
	HRESULT		SetFollowingPunct(LPWSTR lpstrFollowingPunct);
	HRESULT		GetLeadingPunct	 (LPWSTR *plpstrLeadingPunct);
	HRESULT		SetLeadingPunct	 (LPWSTR lpstrLeadingPunct);
	HRESULT		GetViewKind		 (long *pValue);
	HRESULT		SetViewKind		 (long Value);
	HRESULT		GetViewScale	 (long *pValue);
	HRESULT		SetViewScale	 (long Value);

	// Notification Management Methods.  In principle, these methods 
	// could form a separate class, but for space savings, they are part
	// of the CTxtEdit class

	HRESULT		TxNotify(DWORD iNotify, void *pv);	//@cmember General-purpose
													// notification
	void		SendScrollEvent(DWORD iNotify);		//@cmember Send scroll
													//  event
	void		SendUpdateEvent();					//@cmember Send EN_UPDATE
													//  event
													//@cmember Use EN_PROTECTED
	BOOL		QueryUseProtection( CTxtRange *prg, //  to query protection
					UINT msg,WPARAM wparam, LPARAM lparam);//  usage
													//@cmember indicates whether
													// or not protection checking
													// enabled
	BOOL		IsProtectionCheckingEnabled() 
				{ return !!(_dwEventMask & ENM_PROTECTED); }

	// FUTURE (alexgo): maybe we can use just one method :-)
	BOOL	 	IsntProtectedOrReadOnly(UINT msg, WPARAM wparam, LPARAM lparam);

	BOOL 		IsProtected(UINT msg, WPARAM wparam, LPARAM lparam);
	BOOL 		IsProtectedRange(UINT msg, WPARAM wparam, LPARAM lparam, CTxtRange *prg);


	DWORD		GetEventMask(){return _dwEventMask;}//@cmember Get event mask

	void		SetStreaming(BOOL flag)
				{ _fStreaming = flag; }

	BOOL		IsStreaming() { return _fStreaming; }
													//@cmember Handles EN_LINK
													// notifications.
	BOOL		HandleLinkNotification(UINT msg, WPARAM wparam, LPARAM lparam,
					BOOL *pfInLink = NULL);

	HRESULT CloseFile (BOOL bSave);



	//--------------------------------------------------------------
	// Inline proxies to ITextHost methods
	//--------------------------------------------------------------

	// Persisted properties (persisted by the host)
	// Get methods: called by the Text Services component to get 
	// the value of a given persisted property

	// FUTURE (alexgo) !! some of these need to get cleaned up
	
	BOOL		TxGetAutoSize() const;
	BOOL 	 	TxGetAutoWordSel() const;				
	COLORREF 	TxGetBackColor() const			{return _phost->TxGetSysColor(COLOR_WINDOW);}
	TXTBACKSTYLE TxGetBackStyle() const;					
	HRESULT 	TxGetDefaultCharFormat(CCharFormat *pCF);

	void		TxGetClientRect(LPRECT prc) const {_phost->TxGetClientRect(prc);}
	HRESULT		TxGetExtent(SIZEL *psizelExtents) 
					{return _phost->TxGetExtent(psizelExtents);}
	COLORREF 	TxGetForeColor() const			{return _phost->TxGetSysColor(COLOR_WINDOWTEXT);}
	DWORD		TxGetMaxLength() const;
	void		TxSetMaxToMaxText();
	BOOL		TxGetModified() const			{return _fModified;}
	BOOL		TxGetMultiLine() const;					
	HRESULT		TxGetDefaultParaFormat(CParaFormat *pPF);
	TCHAR		TxGetPasswordChar() const;				
	BOOL		TxGetReadOnly() const			{return _fReadOnly;}
	BOOL		TxGetSaveSelection() const;
	DWORD		TxGetScrollBars() const	;				
	BOOL		TxGetSelectionBar() const;
//	LONG		TxGetSelectionBarWidth() const;
	void		TxGetViewInset(LPRECT prc, CDisplay *pdp) const;
	BOOL		TxGetWordWrap() const;

	BOOL		TxClientToScreen (LPPOINT lppt)	{return _phost->TxClientToScreen(lppt); }
	BOOL		TxScreenToClient (LPPOINT lppt)	{return _phost->TxScreenToClient(lppt); }


	//	ITextServices 2 wrappers				
	BOOL		TxIsDoubleClickPending();
	HRESULT		TxGetWindow(HWND *phwnd);
	HRESULT		TxSetForegroundWindow();
	HPALETTE	TxGetPalette();

	// Host service API
	// Called by Text Services component 
	
	HRESULT	 TxActivate( LONG * pOldState ) { return _phost->TxActivate( pOldState ); }
	HRESULT	 TxDeactivate( LONG NewState ) { return _phost->TxDeactivate( NewState ); }
	
	
	// Allowed only when in in-place 
	// The host will fail if not in-place
	
	HDC 		TxGetDC()				
										{return _phost->TxGetDC();}
	INT			TxReleaseDC(HDC hdc)	
										{return _phost->TxReleaseDC(hdc);}
	
	// Helper functions for metafile support
	INT			TxReleaseMeasureDC( HDC hMeasureDC );

	void 		TxUpdateWindow()								
				{
					_phost->TxViewChange(_fInPlaceActive ? TRUE : FALSE);
				}
	void		TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip,
								HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);

	void		TxSetCapture(BOOL fCapture)
										{_phost->TxSetCapture(fCapture);}
	void		TxSetFocus()			
										{_phost->TxSetFocus();}

	// Allowed any-time
	
	BOOL 		TxShowScrollBar(INT fnBar, BOOL fShow)		
										{return _phost->TxShowScrollBar(fnBar, fShow);}
	BOOL 		TxEnableScrollBar (INT fuSBFlags, INT fuArrowFlags)	
										{return _phost->TxEnableScrollBar(fuSBFlags, fuArrowFlags);}
	BOOL 		TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
										{return _phost->TxSetScrollRange(fnBar, nMinPos, nMaxPos, fRedraw);}
	BOOL 		TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw)
										{return _phost->TxSetScrollPos(fnBar, nPos, fRedraw);}
	void		TxInvalidateRect(const LPRECT prc, BOOL fMode)
										{_phost->TxInvalidateRect(prc, fMode);}
	BOOL		TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
										{return _phost->TxCreateCaret(hbmp, xWidth, yHeight);}
	BOOL		TxShowCaret(BOOL fShow)			
										{return _phost->TxShowCaret(fShow);}
	BOOL		TxSetCaretPos(INT x, INT y)			
										{return _phost->TxSetCaretPos(x, y);}
	BOOL 		TxSetTimer(UINT idTimer, UINT uTimeout)
										{return _phost->TxSetTimer(idTimer, uTimeout);}
	void 		TxKillTimer(UINT idTimer)
										{_phost->TxKillTimer(idTimer);}
	COLORREF	TxGetSysColor(int nIndex){ return _phost->TxGetSysColor(nIndex);}

	// IME
	HIMC		TxImmGetContext()		{return _phost->TxImmGetContext();}
	void		TxImmReleaseContext(HIMC himc)
										{_phost->TxImmReleaseContext( himc );}
	BOOL		IsIMEComposition()		{return (_ime != NULL);};
	BOOL		IsAutoFont()			{return _fAutoFont;};
	BOOL		IsAutoKeyboard()		{return _fAutoKeyboard;};

	// Selection access
	CTxtSelection *GetSel();
	CTxtSelection *GetSelNC() { return _psel; }
	LONG 	GetSelMin() const;
	LONG 	GetSelMost() const;
	void 	GetSelRangeForRender(LONG *pcpSelMin, LONG *pcpSelMost);
	void	DiscardSelection();


	// Property Change Helpers
	HRESULT OnRichEditChange(BOOL fFlag);
	HRESULT	OnTxMultiLineChange(BOOL fMultiLine);
	HRESULT	OnTxReadOnlyChange(BOOL fReadOnly);
	HRESULT	OnShowAccelerator(BOOL fPropertyFlag);
	HRESULT	OnUsePassword(BOOL fPropertyFlag);
	HRESULT	OnTxHideSelectionChange(BOOL fHideSelection);
	HRESULT	OnSaveSelection(BOOL fPropertyFlag);
	HRESULT	OnAutoWordSel(BOOL fPropertyFlag);
	HRESULT	OnTxVerticalChange(BOOL fVertical);
	HRESULT	NeedViewUpdate(BOOL fPropertyFlag);
	HRESULT	OnWordWrapChange(BOOL fPropertyFlag);
	HRESULT	OnAllowBeep(BOOL fPropertyFlag);
	HRESULT OnDisableDrag(BOOL fPropertyFlag);
	HRESULT	OnTxBackStyleChange(BOOL fPropertyFlag);
	HRESULT	OnMaxLengthChange(BOOL fPropertyFlag);
	HRESULT OnCharFormatChange(BOOL fPropertyFlag);
	HRESULT OnParaFormatChange(BOOL fPropertyFlag);
	HRESULT	OnClientRectChange(BOOL fPropertyFlag);
	HRESULT OnScrollChange(BOOL fProperyFlag);
	
	// Helpers
	HRESULT	TxCharFromPos(LPPOINT ppt, LONG *pcp);
	HRESULT	OnTxUsePasswordChange(BOOL fUsePassword);
	HRESULT FormatAndPrint(
				HDC hdcDraw,		
				HDC hicTargetDev,	
				DVTARGETDEVICE *ptd,
				RECT *lprcBounds,
				RECT *lprcWBounds);

	HRESULT RectChangeHelper(
				CDrawInfo *pdi,
				DWORD dwDrawAspect,
				LONG  lindex,
				void *pvAspect,
				DVTARGETDEVICE *ptd,
				HDC hdcDraw,
				HDC hicTargetDev,
				const RECT *lprcClient, 
				RECT *prcLocal);

	//
	// PUBLIC INTERFACE METHODS
	//

	// -----------------------------
	//	IUnknown interface
	// -----------------------------

	virtual HRESULT 	WINAPI QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG 		WINAPI AddRef(void);
	virtual ULONG 		WINAPI Release(void);

	//--------------------------------------------------------------
	// ITextServices methods
	//--------------------------------------------------------------
	//@cmember Generic Send Message interface
	virtual HRESULT 	TxSendMessage(
							UINT msg, 
							WPARAM wparam, 
							LPARAM lparam,
							LRESULT *plresult);
	
	//@cmember Rendering
	virtual HRESULT		TxDraw(	
							DWORD dwDrawAspect,		// draw aspect
							LONG  lindex,			// currently unused
							void * pvAspect,		// info for drawing 
													// optimizations (OCX 96)
							DVTARGETDEVICE * ptd,	// information on target 
													// device								'
							HDC hdcDraw,			// rendering device context
							HDC hicTargetDev,		// target information 
													// context
							LPCRECTL lprcBounds,	// bounding (client) 
													// rectangle
							LPCRECTL lprcWBounds,	// clipping rect for 
													// metafiles
			   				LPRECT lprcUpdate,		// dirty rectange insde 
			   										// lprcBounds
							BOOL (CALLBACK * pfnContinue) (DWORD), // for 
													// interupting 
							DWORD dwContinue,		// long displays (currently 
													// unused) 
							LONG lViewID);			// Specifies view to redraw

	//@cmember Horizontal scrollbar support
	virtual HRESULT		TxGetHScroll(
							LONG *plMin, 
							LONG *plMax, 
							LONG *plPos, 
							LONG *plPage,
							BOOL * pfEnabled );

   	//@cmember Horizontal scrollbar support
	virtual HRESULT		TxGetVScroll(
							LONG *plMin, 
							LONG *plMax, 
							LONG *plPos, 
							LONG *plPage, 
							BOOL * pfEnabled );

	//@cmember Setcursor
	virtual HRESULT 	OnTxSetCursor(
							DWORD dwDrawAspect,		// draw aspect
							LONG  lindex,			// currently unused
							void * pvAspect,		// info for drawing 
													// optimizations (OCX 96)
							DVTARGETDEVICE * ptd,	// information on target 
													// device								'
							HDC hdcDraw,			// rendering device context
							HDC hicTargetDev,		// target information 
													// context
							LPCRECT lprcClient, 
							INT x, 
							INT y);

	//@cmember Hit-test
	virtual HRESULT 	TxQueryHitPoint(
							DWORD dwDrawAspect,		// draw aspect
							LONG  lindex,			// currently unused
							void * pvAspect,		// info for drawing 
													// optimizations (OCX 96)
							DVTARGETDEVICE * ptd,	// information on target 
													// device								'
							HDC hdcDraw,			// rendering device context
							HDC hicTargetDev,		// target information 
													// context
							LPCRECT lprcClient, 
							INT x, 
							INT y, 
							DWORD * pHitResult);

	//@member Inplace activate notification
	virtual HRESULT		OnTxInPlaceActivate(const RECT *prcClient);

	//@member Inplace deactivate notification
	virtual HRESULT		OnTxInPlaceDeactivate();

	//@member UI activate notification
	virtual HRESULT		OnTxUIActivate();

	//@member UI deactivate notification
	virtual HRESULT		OnTxUIDeactivate();

	//@member Get text in control
	virtual HRESULT		TxGetText(BSTR *pbstrText);

	//@member Set text in control
	virtual HRESULT		TxSetText(LPCTSTR pszText);
	
	//@member Get x position of 
	virtual HRESULT		TxGetCurTargetX(LONG *);
	//@member Get baseline position
	virtual HRESULT		TxGetBaseLinePos(LONG *);

	//@member Get Size to fit / Natural size
	virtual HRESULT		TxGetNaturalSize(
							DWORD dwAspect,
							HDC hdcDraw,
							HDC hicTargetDev,
							DVTARGETDEVICE *ptd,
							DWORD dwMode, 
							const SIZEL *psizelExtent,
							LONG *pwidth, 
							LONG *pheight);

	//@member Drag & drop
	virtual HRESULT		TxGetDropTarget( IDropTarget **ppDropTarget );

	//@member Bulk bit property change notifications
	virtual HRESULT		OnTxPropertyBitsChange(DWORD dwMask, DWORD dwBits);

	//@cmember Fetch the cached drawing size 
	virtual	HRESULT		TxGetCachedSize(DWORD *pdwWidth, DWORD *pdwHeight);
	
	//	IDispatch methods

   	STDMETHOD(GetTypeInfoCount)( UINT * pctinfo);

	STDMETHOD(GetTypeInfo)(
	  
	  UINT itinfo,
	  LCID lcid,
	  ITypeInfo **pptinfo);

	STDMETHOD(GetIDsOfNames)(
	  
	  REFIID riid,
	  OLECHAR **rgszNames,
	  UINT cNames,
	  LCID lcid,
	  DISPID * rgdispid);

	STDMETHOD(Invoke)(
	  
	  DISPID dispidMember,
	  REFIID riid,
	  LCID lcid,
	  WORD wFlags,
	  DISPPARAMS * pdispparams,
	  VARIANT * pvarResult,
	  EXCEPINFO * pexcepinfo,
	  UINT * puArgErr);


	// ITextDocument methods
	STDMETHOD(GetName)(BSTR *pName);
	STDMETHOD(GetSelection)(ITextSelection **ppSel);
	STDMETHOD(GetStoryCount)(long *pCount);
	STDMETHOD(GetStoryRanges)(ITextStoryRanges **ppStories);
	STDMETHOD(GetSaved)(long *pValue);
	STDMETHOD(SetSaved)(long Value);
	STDMETHOD(GetDefaultTabStop)(float *pValue);
	STDMETHOD(SetDefaultTabStop)(float Value);
	STDMETHOD(New)();
	STDMETHOD(Open)(VARIANT *pVar, long Flags, long CodePage);
	STDMETHOD(Save)(VARIANT *pVar, long Flags, long CodePage);
	STDMETHOD(Freeze)(long *pCount);
	STDMETHOD(Unfreeze)(long *pCount);
	STDMETHOD(BeginEditCollection)();
	STDMETHOD(EndEditCollection)();
	STDMETHOD(Undo)(long Count, long *prop);
	STDMETHOD(Redo)(long Count, long *prop);
	STDMETHOD(Range)(long cpFirst, long cpLim, ITextRange ** ppRange);
	STDMETHOD(RangeFromPoint)(long x, long y, ITextRange **ppRange);

	// IRichEditOle methods
	STDMETHOD(GetClientSite) ( LPOLECLIENTSITE  *lplpolesite);
	STDMETHOD_(LONG,GetObjectCount) (THIS);
	STDMETHOD_(LONG,GetLinkCount) (THIS);
	STDMETHOD(GetObject) ( LONG iob, REOBJECT  *lpreobject,
						  DWORD dwFlags);
	STDMETHOD(InsertObject) ( REOBJECT  *lpreobject);
	STDMETHOD(ConvertObject) ( LONG iob, REFCLSID rclsidNew,
							  LPCSTR lpstrUserTypeNew);
	STDMETHOD(ActivateAs) ( REFCLSID rclsid, REFCLSID rclsidAs);
	STDMETHOD(SetHostNames) ( LPCSTR lpstrContainerApp, 
							 LPCSTR lpstrContainerObj);
	STDMETHOD(SetLinkAvailable) ( LONG iob, BOOL fAvailable);
	STDMETHOD(SetDvaspect) ( LONG iob, DWORD dvaspect);
	STDMETHOD(HandsOffStorage) ( LONG iob);
	STDMETHOD(SaveCompleted) ( LONG iob, LPSTORAGE lpstg);
	STDMETHOD(InPlaceDeactivate) (THIS);
	STDMETHOD(ContextSensitiveHelp) ( BOOL fEnterMode);
	STDMETHOD(GetClipboardData) ( CHARRANGE  *lpchrg, DWORD reco,
									LPDATAOBJECT  *lplpdataobj);
	STDMETHOD(ImportDataObject) ( LPDATAOBJECT lpdataobj,
									CLIPFORMAT cf, HGLOBAL hMetaPict);


private:

	// Get text helper
	LONG	GetTextRange(LONG cpFirst, LONG cch, TCHAR *pch);
	LONG	GetTextEx(GETTEXTEX *pgt, TCHAR *pch);
	LONG	GetTextLengthEx(GETTEXTLENGTHEX *pgtl);

	//--------------------------------------------------------------
	// WinProc dispatch methods
	// Internally called by the WinProc
	//--------------------------------------------------------------

	// Keyboard
	HRESULT	OnTxKeyDown(WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxChar(WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxSysKeyDown(WORD vkey, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT	OnTxSpecialKeyDown(
		WORD vkey,
		DWORD dwFlags,
		IUndoBuilder *publdr
	);

	// Mouse 
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
	HRESULT	OnTxLButtonDblClk(INT x, INT y, DWORD dwFlags, IUndoBuilder *publdr);
#else
	HRESULT	OnTxLButtonDblClk(INT x, INT y, DWORD dwFlags);
#endif // PWD_JUPITER
	HRESULT	OnTxLButtonDown(INT x, INT y, DWORD dwFlags);
	HRESULT	OnTxLButtonUp(INT x, INT y, DWORD dwFlags, BOOL fReleaseCapture);
	HRESULT	OnTxRButtonDown(INT x, INT y, DWORD dwFlags);
	HRESULT	OnTxRButtonUp(INT x, INT y, DWORD dwFlags);
	HRESULT	OnTxMouseMove(INT x, INT y, DWORD dwFlags, IUndoBuilder *publdr);
	HRESULT OnTxMButtonDown (INT x, INT y, DWORD dwFlags);
	HRESULT OnTxMButtonUp (INT x, INT y, DWORD dwFlags);
	
	// Timer
	HRESULT	OnTxTimer(UINT idTimer);
	void CheckInstallContinuousScroll ();
	void CheckRemoveContinuousScroll ();

	// Scrolling
	HRESULT	TxHScroll(WORD wCode, int xPos);
	LRESULT	TxVScroll(WORD wCode, int yPos);
	HRESULT	TxLineScroll(LONG cli, LONG cch);
	
	// Paint, size message
	LRESULT OnSize(HWND hwnd, WORD fwSizeType, int nWidth, int nHeight);

	// Selection commands
	LONG	OnGetSelText(TCHAR *psz);
	LRESULT OnGetSel(LONG *pcpMin, LONG *pcpMost) const;
	void	OnSetSel(LONG cpMin, LONG cpMost);
	void	OnExGetSel(CHARRANGE *pcr) const;
	
	// Editing commands
	void	OnClear(IUndoBuilder *publdr);
	LONG	OnReplaceSel(LONG cchNew, const TCHAR *pch, IUndoBuilder *publdr);
	
	// Format range related commands
	LONG	OnFormatRange(
				FORMATRANGE *pfr, 
				SPrintControl prtcon,
				HDC hdcMeasure = NULL,
				LONG xMeasurePerInch = 0,
				LONG yMeasurePerInch = 0);
	
public:
	BOOL	OnDisplayBand(const RECT *prc, BOOL fPrintFromDraw);

private:
	// Scrolling commands
	void	OnScrollCaret();

	// Focus messages
	LRESULT OnSetFocus();
	LRESULT OnKillFocus();

	// System notifications
	HRESULT	OnContextMenu(LPARAM lparam);

	// Get/Set other properties commands
	LRESULT OnFindText(UINT msg, DWORD flags, FINDTEXTEX *pftex);
	LRESULT OnGetWordBreakProc();
	LRESULT OnSetWordBreakProc();

	// Richedit stuff
	LRESULT	OnGetCharFormat(CCharFormat *pcf, DWORD flags);
	LRESULT	OnGetParaFormat(CParaFormat *ppf);
	LONG 	OnSetCharFormat(WPARAM wparam, CCharFormat *pCF, IUndoBuilder *publdr);
	LONG 	OnSetParaFormat(WPARAM wparam, CParaFormat *pPF, IUndoBuilder *publdr);
	LONG 	OnSetFont(HFONT hfont);

	LRESULT	OnDropFiles(HANDLE hDropFiles);

	// Other services
	HRESULT TxPosFromChar(LONG cp, LPPOINT ppt);
	HRESULT TxGetLineCount(LONG *pcli);
	HRESULT	TxLineFromCp(LONG cp, LONG *pli);
	HRESULT	TxLineLength(LONG cp, LONG *pcch);
	HRESULT	TxLineIndex(LONG ili, LONG *pcp);
	HRESULT TxFindText(DWORD flags, LONG cpMin, LONG cpMost, TCHAR *pch, LONG *pcp);
	HRESULT TxFindWordBreak(INT nFunction, LONG cp, LONG *plRet);

	HRESULT SetText(LPCWSTR pwszText, SetTextFlags flags);


	// Other miscelleneous 
#ifdef DEBUG
	void	OnDumpPed();
#endif

#ifdef PENWIN20
	void	OnFeedbackCursor(HCURSOR hcur);
	LONG	OnHitTest(POINT pt);
	void	OnCorrectText(LPRC prc);
#endif // PENWIN20

	HRESULT OnInsertDIB(LPARAM lparam, IUndoBuilder *publdr);

	COleObject * ObjectFromIOB(LONG iob);

	// Only when the selection is going away should this value be NULLed. We 
	// use SelectionNull function rather than CTxtSelection::~CTxtSelection 
	// to avoid circular dependencies.
	friend void SelectionNull(CTxtEdit *ped);
	void	SetSelectionToNull() { _psel = NULL; }

	// Helper for converting a rich text object to plain text.
	void HandleRichToPlainConversion();

	// Helper for when plain text IME composition needs to clean up.
	void HandleIMEToPlain();

	// Helper for clearing the undo buffers.
	void ClearUndo(IUndoBuilder *publdr);

	// Helper for setting the automatic EOP
	void SetRichDocEndEOP(LONG cchToReplace);

#ifndef MACPORT
	// Helper for OnDropFiles.  Not present on the Macintosh
	LRESULT CTxtEdit::InsertFromFile ( LPCTSTR lpFile );
#endif
//
//	Data Members
//

public:	
	static DWORD 		_dwTickDblClick;	// time of last double-click
	static POINT 		_ptDblClick;		// position of last double-click

	static HCURSOR 		_hcurArrow;
//	static HCURSOR 		_hcurCross;			// OutlineSymbol drag not impl
	static HCURSOR 		_hcurHand;
	static HCURSOR 		_hcurIBeam;
	static HCURSOR 		_hcurItalic;
	static HCURSOR 		_hcurSelBar;

	typedef HRESULT (CTxtEdit::*FNPPROPCHG)(BOOL fPropFlag);

	static FNPPROPCHG 	_fnpPropChg[MAX_PROPERTY_BITS];

	// Only wrapper functions should use this member...
	ITextHost*			_phost;		// host 

	// word break procedure
	EDITWORDBREAKPROC 	_pfnWB;		// word break procedure

	// display subsystem
	CDisplay *			_pdp;		// display
	CDisplayPrinter *	_pdpPrinter;// display for printer

	// undo
	IUndoMgr *			_pundo;		// the undo stack
	IUndoMgr *			_predo;		// the redo stack

	// data transfer
	CLightDTEngine 		_ldte;		// the data transfer engine

	CNotifyMgr 			_nm;		// the notification manager (for floating

	// OLE support
	CObjectMgr *		_pobjmgr;	// handles most high-level OLE stuff

	// Re-entrancy && Notification Management
	CCallMgr *			_pcallmgr;

	// URL detection
	CDetectURL *		_pdetecturl;// manages auto-detection of URL strings
	
	// IME support
	class CIme	*_ime;				// non-NULL when IME level 2 or 3 composition active
	CDocInfo *	_pDocInfo;			// Document info (name, flags, code page)

	// BiDi support
	CBiDiEngine *		_pBiDiEngine;	// handles various bidi text functions.

	union
	{
	  DWORD _dwFlags;				// All together now
	  struct
	  {

#define TXTBITS (TXTBIT_RICHTEXT	  | \
				 TXTBIT_READONLY	  | \
				 TXTBIT_USEPASSWORD   | \
				 TXTBIT_HIDESELECTION | \
				 TXTBIT_VERTICAL	  | \
				 TXTBIT_ALLOWBEEP	 | \
				 TXTBIT_DISABLEDRAG   )

		//	State information. Flags in TXTBITS must appear in same bit
		//	positions as the following (saves code in Init())

		//	TXTBIT_RICHTEXT			0	_fRich
		//	TXTBIT_MULTILINE		1
		//	TXTBIT_READONLY			2	_fReadOnly
		//	TXTBIT_SHOWACCELERATOR	3
		//	TXTBIT_USEPASSWORD		4	_fUsePassword
		//	TXTBIT_HIDESELECTION	5	_fHideSelection
		//	TXTBIT_SAVESELECTION	6
		//	TXTBIT_AUTOWORDSEL		7		
		//	TXTBIT_VERTICAL			8	
		//	TXTBIT_SELECTIONBAR		9
		//	TXTBIT_WORDWRAP  		10
		//	TXTBIT_ALLOWBEEP		11	_fAllowBeep
		//  TXTBIT_DISABLEDRAG	  12	_fDisableDrag
		//  TXTBIT_VIEWINSETCHANGE	13
		//  TXTBIT_BACKSTYLECHANGE	14
		//  TXTBIT_MAXLENGTHCHANGE	15
		//  TXTBIT_SCROLLBARCHANGE	16
		//  TXTBIT_CHARFORMATCHANGE 17
		//  TXTBIT_PARAFORMATCHANGE	18
		//  TXTBIT_EXTENTCHANGE		19
		//  TXTBIT_CLIENTRECTCHANGE	20

#ifdef MACPORT 
//NOTE: Different BIT ordering on the MAC requires that we flip the following bit fields.
//		This is because they are unioned with _dwFlags which is bit compared using the
//		TXTBIT_xxx flags
//		IMPORTANT: For Mac, all 32 bits must be filled out, else they'll be shifted.
	
		DWORD	_fUpdateSelection	:1;	// 31: If true, update sel at level 0
		DWORD	_fDragged			:1;	// 30: Was the selection actually dragged?
		DWORD	_fKANAMode			:1;	// 29: Japanese KANA input mode	
		DWORD	_fIMECancelComplete	:1;	// 28: If aborting IME, cancel the comp string, else complete.
		DWORD	_fInOurHost			:1; // 27: Whether we are in our host
		DWORD	_fSaved				:1;	// 26: ITextDocument Saved property
		DWORD	_fFiller			:1;	// 25: UNUSED!!!.
		DWORD	_fMButtonCapture	:1;	// 24: captured mButton down.
 		DWORD	_fContinuousScroll	:1;	// 23: We have a timer running to support scrolling.

		DWORD	_fAutoKeyboard		:1;	// 22: auto switching keyboard
		
		// Miscellaneous bits used by IME
		DWORD	_fAutoFont			:1;	// 21: auto switching font

		// Miscellaneous bits
		DWORD 	_fUseUndo			:1;	// 20: Only set to zero if undo limit is 0
		DWORD	_f10Mode			:1;	// 19: Use Richedit10 behavior

		DWORD	_fRichPrevAccel		:1; // 18: Rich state previous to accelerator
		DWORD	_fWantDrag			:1;	// 17: Want to initiate drag & drop
		DWORD	_fStreaming			:1;	// 16: Currently streaming text in or out
		DWORD	_fScrollCaretOnFocus:1;	// 15: Scroll caret into view on set focus
		DWORD	_fModified			:1;	// 14: Control text has been modified
		DWORD	_fIconic			:1;	// 13: Control/parent window is iconized
		DWORD   _fDisableDrag		:1;	// 12: Disable Drag
		DWORD	_fAllowBeep			:1; // 11: Allow beep at doc boundaries
		DWORD	_fTransparent		:1;	// 10: Background transparency
		DWORD	_fMouseDown			:1;	// 9: One mouse button is current down
		DWORD	_fEatLeftDown		:1;	// 8: Eat the next left down?
		DWORD	_fFocus				:1;	// 7: Control has keyboard focus
		DWORD	_fOverstrike		:1;	// 6: Overstrike mode vs insert mode
		DWORD	_fHideSelection		:1; // 5: Hide selection when inactive
		DWORD	_fUsePassword		:1; // 4: Whether to use password char
		DWORD 	_fInPlaceActive		:1;	// 3: Control is in place active
		DWORD	_fReadOnly			:1;	// 2: Control is read only
		DWORD	_fCapture			:1;	// 1: Control has mouse capture
		DWORD	_fRich				:1;	// 0: Use rich-text formatting

#else

		DWORD	_fRich				:1;	// 0: Use rich-text formatting
		DWORD	_fCapture			:1;	// 1: Control has mouse capture
		DWORD	_fReadOnly			:1;	// 2: Control is read only
		DWORD 	_fInPlaceActive		:1;	// 3: Control is in place active
		DWORD	_fUsePassword		:1; // 4: Whether to use password char
		DWORD	_fHideSelection		:1; // 5: Hide selection when inactive
		DWORD	_fOverstrike		:1;	// 6: Overstrike mode vs insert mode
		DWORD	_fFocus				:1;	// 7: Control has keyboard focus
		DWORD	_fEatLeftDown		:1;	// 8: Eat the next left down?
		DWORD	_fMouseDown			:1;	// 9: One mouse button is current down
		DWORD	_fTransparent		:1;	// 10: Background transparency
		DWORD	_fAllowBeep			:1; // 11: Allow beep at doc boundaries
		DWORD   _fDisableDrag		:1;	// 12: Disable Drag

		DWORD	_fIconic			:1;	// 13: Control/parent window is iconized
		DWORD	_fModified			:1;	// 14: Control text has been modified
		DWORD	_fScrollCaretOnFocus:1;	// 15: Scroll caret into view on set focus
		DWORD	_fStreaming			:1;	// 16: Currently streaming text in or out
		DWORD	_fWantDrag			:1;	// 17: Want to initiate drag & drop
		DWORD	_fRichPrevAccel		:1; // 18: Rich state previous to accelerator
	
		// Miscellaneous bits
		DWORD	_f10Mode			:1;	// 19: Use Richedit10 behavior
		DWORD 	_fUseUndo			:1;	// 20: Only set to zero if undo limit is 0
	

		// Miscellaneous bits used by IME
		DWORD	_fAutoFont			:1;	// 21: auto switching font
		DWORD	_fAutoKeyboard		:1;	// 22: auto switching keyboard

		DWORD	_fContinuousScroll	:1;	// 23: Timer runs to support scrolling
		DWORD	_fMButtonCapture	:1;	// 24: captured mButton down
		DWORD	_fFiller			:1;	// 25: UNUSED!!!
		DWORD	_fSaved				:1;	// 26: ITextDocument Saved property
		DWORD	_fInOurHost			:1; // 27: Whether we are in our host
		DWORD	_fIMECancelComplete	:1; // 28: If aborting IME, cancel comp string, else complete
		DWORD	_fKANAMode			:1;	// 29: Japanese KANA input mode

		// Drag/Drop UI refinement.
		DWORD	_fDragged			:1;	// 30: Was the selection actually dragged?
		DWORD	_fUpdateSelection	:1;	// 31: If true, update sel at level 0
#endif
	  };
	};

#define	CD_LTR	2
#define	CD_RTL	3

	WORD		_nContextDir		:2;	// 0: no context; else CD_LTR or CD_RTL
	WORD		_fSelfDestruct		:1;	// This CTxtEdit is selfdestructing
	WORD		_fIMEAlwaysNotify	:1;	// Send Notification during IME undetermined string

	WORD		_cActiveObjPosTries :2; // this little counter protects us from an infinite 
										// loop of repaints when trying to put a dragged-away 
										// in-place active object to where it belongs
	WORD		_fSingleCodePage	:1;	// If TRUE, only allow single code page
										// for editing

	// Miscellaneous bits used for BiDi input
	WORD		_fControl			:1;	// Control key is held down
	WORD		_fLShift			:1;	// Left shift key is held down
	WORD		_fRShift			:1;	// Right shift key is held down
	WORD		_fHbrCaps			:1;	// Intialization of state of hebrew and caps lock status
 
	WORD		_fActivateKbdOnFocus:1;	// Activate new keyboard layout on WM_SETFOCUS
	WORD		_fOutlineView		:1;	// Outline mode is active
	BYTE		_bDeadKey;				// Dead key (`'^~:) to apply to next char

	BYTE		_DeleteBeforeConvert;	// Fix for WinCEOS RAID #12736

 	DWORD		_dwEventMask;			// Event mask
	DWORD		_cBiDiTxtLength;

	// GuyBark JupiterJ IME: Freeze the display while we walk the caret.
	BOOL		_fDisplayFrozen;

private:
	HKL			_ActiveKbdLayer;
	SHORT		_cpAccelerator;			// Range for accelerator
	SHORT		_iCF;					// Default CCharFormat index
	SHORT		_iPF;					// Default CParaFormat index
	WORD		_wZoomNumerator;
	WORD		_wZoomDenominator;

	POINT		_mousePt;				// Last known mouse position.

	// We need to have at least some of the following members on a per-instance
	// basis to handle simultaneous scrolling of two or more controls.
	// NOTE: the int's can be switched to SHORTs, since pixels are used and
	// 32768 pixels is a mighty big screen! 
	DWORD		_dwFkeys;				// State of function keys

	LONG		_cchTextMost;			// Maximum allowed text


	friend class CRchTxtPtr;

	IUnknown *	_punk;					// IUnknown to use

	class CUnknown : public IUnknown
	{
		friend class CCallMgr;
	private:

		DWORD	_cRefs;					// Reference count

	public:

		void	Init() {_cRefs = 1; }

		HRESULT WINAPI QueryInterface(REFIID riid, void **ppvObj);
		ULONG WINAPI	AddRef();
		ULONG WINAPI Release();
	};

	friend class CUnknown;

	CUnknown 	_unk;					// Object that implements IUnknown	
	CTxtStory _story;
	CTxtSelection *_psel;				// Selection object

};

#endif

