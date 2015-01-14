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
 *	@doc	INTERNAL
 *
 *	@module	_COLEOBJ.H		The OLE Object management class | 
 *
 *	Author:	alexgo 10/24/95
 */

#ifndef __COLEOBJ_H__
#define __COLEOBJ_H__

#include "_notmgr.h"

class CDisplay;
class CDevDesc;
class CTxtEdit;
class IUndoBuilder;

/* 
 *	COleObject
 *
 *	@class	This class manages an individual OLE object embedding.
 *
 */

class COleObject :  public IOleClientSite, public IOleInPlaceSite, 
					public IAdviseSink, public CSafeRefCount, public ITxNotify
{
//@access Public methods
public:
	//
	// IUnknown methods
	//

	STDMETHOD(QueryInterface)(REFIID riid, void **pv);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);
	
	//
	// IOleClientSite methods
	//
	
	STDMETHOD(SaveObject)(void);
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker,
							IMoniker **ppmk);
	STDMETHOD(GetContainer)(IOleContainer **ppContainer);
	STDMETHOD(ShowObject)(void);
	STDMETHOD(OnShowWindow)(BOOL fShow);
	STDMETHOD(RequestNewObjectLayout)(void);

	//
	//	IOleInPlaceSite methods
	//
	STDMETHOD(GetWindow)(HWND *phwnd);
	STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
	STDMETHOD(CanInPlaceActivate)(void);
	STDMETHOD(OnInPlaceActivate)(void);
	STDMETHOD(OnUIActivate)(void);
	STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame,
							IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect,
							LPRECT lprcClipRect, 
							LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHOD(Scroll)(SIZE scrollExtant);
	STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
	STDMETHOD(OnInPlaceDeactivate)(void);
	STDMETHOD(DiscardUndoState)(void);
	STDMETHOD(DeactivateAndUndo)(void);
	STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

	//
	// IAdviseSink methods
	//

	STDMETHOD_(void, OnDataChange)(FORMATETC *pfetc, STGMEDIUM *pmedium);
	STDMETHOD_(void, OnViewChange)(DWORD dwAspect, LONG lindex);
	STDMETHOD_(void, OnRename)(IMoniker *pmk);
	STDMETHOD_(void, OnSave)(void);
	STDMETHOD_(void, OnClose)(void);

	//
	//	ITxNotify methods
	//
    virtual void    OnPreReplaceRange( DWORD cp, DWORD cchDel, DWORD cchNew,
                        DWORD cpFormatMin, DWORD cpFormatMax);
    virtual void    OnPostReplaceRange( DWORD cp, DWORD cchDel, DWORD cchNew,
                        DWORD cpFormatMin, DWORD cpFormatMax);
	virtual void	Zombie();

	//
	//	internal public methods
	//

			COleObject(CTxtEdit *ped);	//@cmember Constructor
	DWORD	GetCp()						//@cmember Get the cp for this object
				{return _cp;}
										//@cmember Fill out the REOBJECT struct
	HRESULT	GetObjectData(REOBJECT *preobj, DWORD dwFlags);
	BOOL	IsLink();					//@cmember returns TRUE if the object
										// is a link object.
										//@cmember Initialize from the given
										// object data
	HRESULT InitFromREOBJECT(DWORD cp, REOBJECT *preobj);
										//@cmember Measures the object (no
										// outgoing calls)
	void 	MeasureObj(const CDisplay *pdp, LONG& xWidth, LONG& yHeight, SHORT yDescent);
										//@cmember Draws the object
	void	DrawObj(const CDisplay *pdp, HDC hdc, BOOL fMetafile, POINT *ppt, RECT *prcRender);
										//@cmember Handle the object being
										// deleted from the backing store
	void	Delete(IUndoBuilder *publdr);
	void	Restore();					//@cmember restore an object into
										// the backing store
										//@cmember Force a close on the
										// object
	void	Close(DWORD dwSaveOption);
										//@cmember Zombie the object.
    void    MakeZombie(void);
										//@cmember Activate the object
	BOOL	ActivateObj(UINT uiMsg,	WPARAM wParam, LPARAM lParam);
	HRESULT	DeActivateObj(void);		//@cmember Aeactivate the object
										//@cmember Set individual selection
										//state of object

	RECT *	GetPosRect()				//@cmember return the position rect
				{ return &_rcPos; }

	void ResetPosRect(LONG *pAdjust = NULL);

	void FetchObjectExtents(void);//@cmember Gets the size of the object
									//@cmember return REO_SELECTED state
	BOOL	GetREOSELECTED(void) {return (_pi.dwFlags & REO_SELECTED);}
										//@cmember set REO_SELECTED state
	void	SetREOSELECTED(BOOL fSelect);
								//@cmember checks for hits on frame handles
	LPTSTR	CheckForHandleHit(const POINT &pt);
								//@cmember handle object resize
	BOOL    HandleResize(const POINT &pt);
								//@cmember sets object size
	void    Resize(const RECT &rcNew);
								//@cmember update _sizel
	void	ResetSizel(const SIZEL &sizel)
						{_sizel = sizel;}
								//@cmember called when object position changes
	void    OnReposition( LONG dx, LONG dy );
								//@cmember gets objects IUnknown
	IUnknown *	GetIUnknown(void) {return _punkobj;}
								//@cmember converts to the specified class
	HRESULT	Convert(REFCLSID rclsidNew, LPCSTR lpstrUserTypeNew);
								//@cmember activate as the specified class
	HRESULT	ActivateAs(REFCLSID rclsid, REFCLSID rclsidAs);
								//@cmember set aspect to use
	void	SetDvaspect(DWORD dvaspect);
								//@cmember get current aspect
	DWORD	GetDvaspect()
				{ return _pi.dvaspect; }
								//@cmember see IPersistStore::HandsOffStorage
	void	HandsOffStorage(void);
								//@cmember see IPersistStore::SaveCompleted
	void	SaveCompleted(LPSTORAGE lpstg);
								//@cmember set REO_LINKAVAILABLE flag
	HRESULT SetLinkAvailable(BOOL fAvailable);
								//@cmember Used for textize support
	LONG	WriteTextInfoToEditStream(EDITSTREAM *pes);
								//@cmember Scroll this object if appropriate
	void	ScrollObject(LONG dx, LONG dy, LPCRECT prcScroll);

								//@cmember guard the position rectangle
								// from bad updates
	void	EnableGuardPosRect()	{_fGuardPosRect = TRUE;}
								//@cmember clear the guard bit
	void	DisableGuardPosRect()	{_fGuardPosRect = FALSE;}
								//@cmember set the _hdata member
	void    SetHdata(HGLOBAL hg)	{_hdata = hg;}
								//@cmember get the _hdata member
	HGLOBAL GetHdata()				{return _hdata;}
	struct ImageInfo
	{
		LONG	xScale, yScale;		// @field scaling percentage along axes
		SHORT	xExtGoal, yExtGoal;	// @field desired dimensions in twips for pictures
		SHORT	cBytesPerLine;		// @field # bytes per raster line, if bitmap
	};
								//@cmember set the _pimageinfo member
	void    SetImageInfo(struct ImageInfo *pim)	{_pimageinfo = pim;}
								//@cmember get the _pimageinfo member
	ImageInfo *GetImageInfo()				{return _pimageinfo;}

#ifdef DEBUG
	void    DbgDump(DWORD id);
#endif

//@access Private methods and data
private:
	virtual ~COleObject();		//@cmember Destructor

	void SavePrivateState(void);//@cmember Saves private information
	HRESULT ConnectObject(void);//@cmember setup advises, etc.
	void DisconnectObject(void);//@cmember tear down advises, etc.
	void CleanupState(void);	//@cmember cleans up our member data, etc.
								//@cmember draws frame around object
	void DrawFrame(const CDisplay *pdp, HDC hdc, RECT* prc);
								//@cmember helper to draw frame handles
	void DrawHandle(HDC hdc, int x, int y);
								//@cmember helper to check for hit on a handle
	void CreateDib(HDC hdc);
								//@cmember helper to create DIB for WinCE bitmaps
	void DrawDib(HDC hdc, RECT *prc);
								//@cmember helper to draw WinCE bitmaps
	BOOL InHandle(int x, int y, const POINT &pt);
								//@member to restablish rectangle position
	enum { SE_NOTACTIVATING, SE_ACTIVATING };  // used by SetExtent to indicate
												// the context of the SetExtent call
	HRESULT SetExtent(int iActivating);
								//@member attempts to set the extents of the
								// object

	CTxtEdit *		_ped;		//@cmember edit context for this object
	IUnknown *		_punkobj;	//@cmember pointer to the objects IUnknown.
	IStorage *		_pstg;		//@cmember storage for the object
	RECT	 		_rcPos	;	//@cmember position rect in client coords
	SIZEL			_sizel;		//@cmember the cached "real" size of the object
	DWORD			_cp;		//@cmember the position of this object
	DWORD			_dwConn;	//@cmember the advise connection cookie
	HGLOBAL			_hdata;
	HBITMAP			_hdib;
	ImageInfo *		_pimageinfo;

	struct PersistedInfo
	{
		DWORD	dwFlags;		//@cmember see richole.h
		DWORD	dwUser;			//@cmember user defined
		DWORD	dvaspect;		//@cmember from the DVASPECT enumeration
	};

	PersistedInfo	_pi;

	SHORT			_dxyFrame;	//@cmember the object frame width
	unsigned int	_fInPlaceActive:1;	//@cmember inplace active?
	unsigned int	_fInUndo:1;	//@cmember in the undo stack?
	unsigned int	_fIsWordArt2:1;		//@cmember Is this object a WordArt 2.0
								// object? (need to do hacks for it)
	unsigned int 	_fIsPaintBrush:1;	//@cmember Is this object a PaintBrush
								// object? (need to do hacks for it)
	unsigned int	_fPBUseLocalSizel:1;	
								//@cmember Signals that SetExtent
								// for a PaintBrush object failed and that
								// _sizel is an accurate indication of object size
	unsigned int    _fDraw:1;	//@cmember Should object be drawn?
	unsigned int	_fSetExtent:1; //@cmember Does SetExtent need to be called
								   //when activating.
	unsigned int	_fGuardPosRect:1;//@cmember Guards the position rect from
								// updating.
};

#endif // __COLEOBJ_H_


	

