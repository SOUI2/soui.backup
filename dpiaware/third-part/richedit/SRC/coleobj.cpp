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
 *	@module	COLEOBJ.CPP	OLE Object management class implemenation |
 *
 * 	Author:		alexgo 10/24/95
 *
 *	Note:	Much of this code is a port from Richedit1.0 sources
 *			(cleaned up a bit, ported to C++, etc.)  So if there's any
 *			bit of strangeness, it's probably there for a reason.
 *
 */

#include "_common.h"
#include "_edit.h"
#include "_coleobj.h"
#include "_objmgr.h"
#include "_select.h"
#include "_rtext.h"
#include "_disp.h"
#include "_dispprt.h"
#include "_antievt.h"
#include "_dxfrobj.h"

ASSERTDATA

#ifndef WM_NCMOUSEFIRST
#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#endif
#ifndef WM_NCMOUSELAST
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK
#endif

// 
// data private to this file
//
static const OLECHAR szSiteFlagsStm[] = OLESTR("RichEditFlags");	

// 
// EXCEL clsid's.  We have to make some special purpose hacks
// for XL.
const CLSID rgclsidExcel[] =
{
    { 0x00020810L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },  // Excel Worksheet
    { 0x00020811L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },  // Excel Chart
    { 0x00020812L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },  // Excel App1
    { 0x00020841L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} },  // Excel App2
};
const INT cclsidExcel = sizeof(rgclsidExcel) / sizeof(rgclsidExcel[0]);


//
//	WordArt CLSID for more special purpose hacks.
//
const GUID CLSID_WordArt = 
    { 0x000212F0L, 0, 0, {0xC0, 0, 0, 0, 0, 0, 0, 0x46} };
const GUID CLSID_PaintbrushPicture = 
    { 0x0003000AL, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
const GUID CLSID_BitmapImage = 
    { 0xD3E34B21L, 0x9D75, 0x101A, { 0x8C, 0x3D, 0x00, 0xAA, 0x00, 0x1A, 0x16, 0x52 } };


#define dxyHandle (6) // Object frame handle size
#define dxyFrameDefault  (1) // Object frame width

// 
// utility functions
//

/*
 *	IsExcelCLSID
 *
 *	@func	checks to see if the given clsid is one of XL's
 *
 *	@rdesc	TRUE/FALSE
 *
 */
BOOL IsExcelCLSID(REFGUID clsid)
{
	DWORD i;

    for( i = 0; i < cclsidExcel; i++ )
    {
        if (IsEqualCLSID(clsid, rgclsidExcel[i]))
        {
			return TRUE;
        }
    }

    return FALSE;
}

//
//	PUBLIC methods
//

/*
 *	COleObject::QueryInterface
 *
 *	@mfunc	the standard OLE QueryInterface
 *
 *	@rdesc	NOERROR		<nl>
 *			E_NOINTERFACE
 *
 */
STDMETHODIMP COleObject::QueryInterface(
	REFIID		riid,		//@parm the requested interface ID
	void **	ppv)		//@parm out param for the result
{
	HRESULT hr = NOERROR;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::QueryInterface");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	if( !ppv )
	{
		return E_INVALIDARG;
	}
	else
	{
		*ppv = NULL;
	}

	if( IsEqualIID(riid, IID_IUnknown) )
	{
		*ppv = (IUnknown *)(IOleClientSite *)this;
	}
	else if( IsEqualIID(riid, IID_IOleClientSite) )
	{
		*ppv = (IOleClientSite *)this;
	}
	else if( IsEqualIID(riid, IID_IOleInPlaceSite) )
	{
		*ppv = (IOleInPlaceSite *)this;
	}
	else if( IsEqualIID(riid, IID_IAdviseSink) )
	{
		*ppv = (IAdviseSink *)this;
	}
	else if( IsEqualIID(riid, IID_IOleWindow) )
	{
		*ppv = (IOleWindow *)this;
	}
	else if( IsEqualIID(riid, IID_IRichEditOleCallback) )
	{
		//
		// NB!! Returning this pointer in our QI is 
		// phenomenally bogus; it breaks fundamental COM
		// identity rules (granted, not many understand them!).
		// Anyway, RichEdit 1.0 did this, so we better.
		//

		TRACEWARNSZ("Returning IRichEditOleCallback interface, COM "
			"identity rules broken!");

		*ppv = _ped->GetRECallback();
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	if( *ppv )
	{
		(*(IUnknown **)ppv)->AddRef();
	}

	return hr;
}


/*
 *	COleObject::AddRef
 *
 *	@mfunc	increments the reference count
 *
 *	@rdesc	the new reference count
 *
 */
STDMETHODIMP_(ULONG) COleObject::AddRef(void)
{
    ULONG cRef;
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::AddRef");

    cRef = SafeAddRef();

	return cRef;
}

/*
 *	COleObject::Release
 *
 *	@mfunc	decrements the reference count
 *
 *	@rdesc	the new reference count
 *
 */
STDMETHODIMP_(ULONG) COleObject::Release(void)
{
    ULONG cRef;
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Release");

	cRef = SafeRelease();

	return cRef;
}

/*
 *	COleObject::SaveObject
 *
 *	@mfunc	implemtenation of IOleClientSite::SaveObject
 *
 *	@rdesc	HRESULT
 *
 */
STDMETHODIMP COleObject::SaveObject(void)
{
	IPersistStorage *pps;
	HRESULT hr;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SaveObject");

	if( !_punkobj || !_pstg )
	{
		TRACEWARNSZ("SaveObject called on invalid object");
		return E_UNEXPECTED;
	}

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	hr = _punkobj->QueryInterface(IID_IPersistStorage, (void **)&pps);

	TESTANDTRACEHR(hr);

	if( hr == NOERROR )
	{
        if( IsZombie() )
		{
            return CO_E_RELEASED;
		}
        
		SavePrivateState();
		
        if( IsZombie() )
		{
            return CO_E_RELEASED;
		}
        
		hr = pOleSave(pps, _pstg, TRUE);
	
	    if( IsZombie() )
		{
	        return CO_E_RELEASED;
		}
        
		TESTANDTRACEHR(hr);

		// note that SaveCompleted is called even if OleSave fails.
		// If both OleSave and SaveCompleted succeed, then go ahead
		// and commit the changes

		if( pps->SaveCompleted(NULL) == NOERROR && hr == NOERROR )
		{
		    if( IsZombie() )
			{
		        return CO_E_RELEASED;
			}
			
			hr = _pstg->Commit(STGC_DEFAULT);

			TESTANDTRACEHR(hr);
		}

        pps->Release();
	}

	return hr;
}

/*
 *	COleObject::GetMoniker
 *
 *	@mfunc	implementation of IOleClientSite::GetMoniker
 *
 *	@rdesc	E_NOTIMPL
 *
 */
STDMETHODIMP COleObject::GetMoniker(
	DWORD	dwAssign,			//@parm	force an assignment?
	DWORD	dwWhichMoniker,	//@parm	kind of moniker to get
	IMoniker **ppmk)			//@parm 	out param for result
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetMoniker");

	TRACEWARNSZ("method not implemented!");

	if( ppmk )
	{
		*ppmk = NULL;
	}

	return E_NOTIMPL;
}
	
/*
 *	COleObject::GetContainer
 *
 *	@mfunc	implementation of IOleClientSite::GetContainer
 *
 *	@rdesc	E_NOINTERFACE
 */
STDMETHODIMP COleObject::GetContainer(
	IOleContainer **ppcont)	//@parm	out parm for the result
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetContainer");

	TRACEWARNSZ("method not implemented!");

	if( ppcont )
	{
		*ppcont = NULL;
	}

	// richedit 1.0 returns E_NOINTERFACE instead of E_NOTIMPL.  Do
	// the same.

	return E_NOINTERFACE;
}

/*
 *	COleObject::ShowObject
 *
 *	@mfunc	Implementation of IOleClientSite::ShowObject.  
 *
 *	@rdesc	E_NOTIMPL
 *
 */
STDMETHODIMP COleObject::ShowObject(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::ShowObject");

	TRACEWARNSZ("method not implemented!");

	return E_NOTIMPL;
}

/*
 *	COleObject::OnShowWindow
 *
 *	@mfunc	implementation of IOleClientSite::OnShowWindow -- notifies
 *	the client site that the object is or is not being shown in it's
 *	own application window.  This govens whether or not hatching should
 *	appear around the object in richedit.
 *
 *	@rdesc	HRESULT
 *
 */
STDMETHODIMP COleObject::OnShowWindow(
	BOOL fShow)		//@parm if TRUE, the object is being drawn in it's
					//own window
{
	DWORD dwFlags = _pi.dwFlags;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnShowWindow");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	if( fShow )
	{
		_pi.dwFlags |= REO_OPEN;
	}
	else
	{
		_pi.dwFlags &= ~REO_OPEN;
	}

	// if something changed, redraw the object
	
	if( dwFlags != _pi.dwFlags )
	{
		// invalidate the rect that we're in.
		_ped->TxInvalidateRect(&_rcPos, FALSE);
		// We're not allowed to call invalidate rect by itself
		// without terminating it with a call to update window.
		// However, we don't care at this point if things are
		// redrawn right away.
		_ped->TxUpdateWindow();

		// COMPATIBILITY ISSUE: (alexgo) the RE1.0 code did some funny 
		// stuff with undo here.  I don't believe it's necessary to 
		// repeat that code with our multi-level undo model,
	}

	return NOERROR;
}

/*
 *	COleObject::RequestNewObjectLayout
 *
 *	@mfunc	Implementation of IOleClientSite::RequestNewObjectLayout
 *
 *	@rdesc	E_NOTIMPL
 */
STDMETHODIMP COleObject::RequestNewObjectLayout(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::RequestNewObjectLayout");

	TRACEWARNSZ("method not implemented!");

	return E_NOTIMPL;
}

/*
 *	COleObject::GetWindow
 *
 *	@mfunc	Implementation of IOleInPlaceSite::GetWindow
 *
 *	@rdesc	HRESULT
 */
STDMETHODIMP COleObject::GetWindow(
	HWND *phwnd)	//@parm where to put the window
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetWindow");

	// NB! this method is not stabilized.

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	if( phwnd )
	{
		return _ped->TxGetWindow(phwnd);
	}
	return E_INVALIDARG;
}

/*
 *	COleObject::ContextSensitiveHelp
 *
 *	@mfunc	Implemenation of IOleInPlaceSite::ContextSensitiveHelp
 *
 *	@rdesc	HRESULT
 */
 STDMETHODIMP COleObject::ContextSensitiveHelp(
 	BOOL fEnterMode)	//@parm, if TRUE, then we're in help mode
 {
 	IRichEditOleCallback *precall;
	CCallMgr	callmgr(_ped);
	CStabilize	stabilize(this);
	CObjectMgr  *pCObM = NULL;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::ContextSensitiveHelp");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	// if the mode changes
	if (pCObM = _ped->GetObjectMgr())
	{
    	if( pCObM->GetHelpMode() != fEnterMode )
    	{
    		pCObM->SetHelpMode(fEnterMode);

    		precall = _ped->GetRECallback();

    		if( precall )
    		{
    			return precall->ContextSensitiveHelp(fEnterMode);
    		}
	    }
	}
	return NOERROR;
}

/*
 *	COleObject::CanInPlaceActivate
 *
 *	@mfunc	implementation of IOleInPlaceSite::CanInPlaceActivate
 *
 *	@rdesc	NOERROR or S_FALSE
 */
STDMETHODIMP COleObject::CanInPlaceActivate(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::CanInPlaceActivate");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	// if we have a callback && the object is willing to show
	// content, then we can in-place activate

	if( _ped->GetRECallback() && 
		_pi.dvaspect == DVASPECT_CONTENT )
	{
		return NOERROR;
	}
	else
	{
		return S_FALSE;
	}
}

/*
 *	COleObject::OnInPlaceActivate
 *
 *	@mfunc	implementation of IOleInPlaceSite::OnInPlaceActivate
 *
 *	@rdesc	noerror
 */
STDMETHODIMP	COleObject::OnInPlaceActivate(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnInPlaceActivate");
	// assume that in-place objects can never be blank.
	_pi.dwFlags &= ~REO_BLANK;
	_fInPlaceActive = TRUE;

	return NOERROR;
}

/*
 *	COleObject::OnUIActivate
 *
 *	@mfunc	implementation of IOleInPlaceSite::OnUIActivate.  Notifies
 *			the container that the object is about to be activated in
 *			place with UI elements suchs as merged menus
 *
 *	@rdesc	HRESULT
 */
STDMETHODIMP COleObject::OnUIActivate(void)
{
	IRichEditOleCallback *precall;
	CTxtSelection *psel;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);
	CObjectMgr *pobjmgr;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnUIActivate");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	pobjmgr = _ped->GetObjectMgr();
	if(!pobjmgr)
		{
		return E_OUTOFMEMORY;
		}
	precall = pobjmgr->GetRECallback();

	if( precall )
	{
		precall->ShowContainerUI(FALSE);
	    if( IsZombie() )
		{
	        return CO_E_RELEASED;
		}
        
		// this is an optimization for activating multiple
		pobjmgr->SetShowUIPending(FALSE);

		Assert(!pobjmgr->GetInPlaceActiveObject());	
		pobjmgr->SetInPlaceActiveObject(this);
		_pi.dwFlags |= REO_INPLACEACTIVE;
		// force this object to be selected, if it isn't already
		if( !(_pi.dwFlags & REO_SELECTED) )
		{
			psel = _ped->GetSel();

			if( psel )
			{
				psel->SetSelection(_cp, _cp + 1);
			}
		}

		return NOERROR;
	}

	return E_UNEXPECTED;
}

/*
 *	COleObject::GetWindowContext
 *
 *	@mfunc	implementation of IOleInPlaceSite::GetWindowContext.
 *			Enables the in-place object to retrieve the window
 *			interfaces that form the window object hierarchy.
 *
 *	@rdesc	HRESULT
 */
STDMETHODIMP COleObject::GetWindowContext(
	IOleInPlaceFrame **ppipframe,	//@parm	where to put the in-place frame
	IOleInPlaceUIWindow **ppipuidoc,//@parm where to put the ui window
	LPRECT prcPos,					//@parm position rect
	LPRECT prcClip,					//@parm clipping rect
	LPOLEINPLACEFRAMEINFO pipfinfo)	//@parm accelerator information
{
	IRichEditOleCallback *precall;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);
	
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::GetWindowContext");
	
    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	// let the container verify other parameters; we don't use them
	if( !prcPos || !prcClip )
	{
		return E_INVALIDARG;
	}
		
	precall = _ped->GetRECallback();

	if( precall )
	{
		// recall that there are two rects here in client coordiantes:
		// the rect for this object (_rcPos) and the rect for
		// our main display;
		*prcPos = _rcPos;

		// FUTURE (alexgo); we may need to get this from the
		// display instead to handle the inactive state if we ever
		// want to support embedded objects with the inactive state.
		_ped->TxGetClientRect(prcClip);
		return precall->GetInPlaceContext(ppipframe, ppipuidoc, pipfinfo);
	}
	return E_UNEXPECTED;
}

/*
 *	COleObject::Scroll
 *
 *	@mfunc	implementation of IOleInPlaceSite::Scroll
 *
 *	@rdesc 	E_NOTIMPL;
 */
STDMETHODIMP COleObject::Scroll(
	SIZE sizeScroll)	//@parm the amount to scroll
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Scroll");

	TRACEWARNSZ("method not implemented!");

	return E_NOTIMPL;
}

/*
 *	COleObject::OnUIDeactivate
 *
 *	@mfunc	implementation of IOleInPlaceSite::OnUIDeactivate.  Notifies
 *			the container that it should re-install it's user interface
 *
 *	@rdesc	HRESULT
 */
STDMETHODIMP COleObject::OnUIDeactivate(
	BOOL fUndoable)		//@parm -- whether or not you can undo anything here
{
	IRichEditOleCallback *precall;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);
	CObjectMgr *pobjmgr;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnUIDeactivate");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	pobjmgr = _ped->GetObjectMgr();
	precall = _ped->GetRECallback();

	if (_fIsPaintBrush)
	{
		// Hack for RAID 3293.  Bitmap object disappears after editing.
		// Apparently paint only triggers OnUIDeactivate and not OnInPlaceDeactivate
		// assume that in-place objects can never be blank.
		_fInPlaceActive = FALSE;
		//Reset REO_INPLACEACTIVE
		_pi.dwFlags &= ~REO_INPLACEACTIVE;
	}

	if( !precall )
	{
		return E_UNEXPECTED;
	}

	if( _ped->TxIsDoubleClickPending() )
	{
		_ped->GetObjectMgr()->SetShowUIPending(TRUE);
	}
	else
	{
		// ignore any errors; the old code did.
		precall->ShowContainerUI(TRUE);

	    if( IsZombie() )
		{
	        return CO_E_RELEASED;
		}
	}
	
	pobjmgr->SetInPlaceActiveObject(NULL);

	// get the focus back
	_ped->TxSetFocus();

#ifdef DEBUG
	// the OLE undo model is not very compatible with multi-level undo.
	// For simplicity, just ignore stuff.
	if( fUndoable )
	{
		TRACEWARNSZ("Ignoring a request to keep undo from an OLE object");
	}
#endif

	// some objects are lame and draw outside the
	// areas they are supposed to.  So we need to 
	// just invalidate everything and redraw.

	_ped->TxInvalidateRect(NULL, TRUE);

	return NOERROR;
}

/*
 *	COleObject::OnInPlaceDeactivate
 *
 *	@mfunc	implementation of IOleInPlaceSite::OnInPlaceDeactivate
 *
 *	@rdesc	NOERROR
 */
STDMETHODIMP COleObject::OnInPlaceDeactivate(void)
{
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::OnInPlaceDeactivate");

	_fInPlaceActive = FALSE;

	//Reset REO_INPLACEACTIVE
	_pi.dwFlags &= ~REO_INPLACEACTIVE;

	if( !_punkobj )
	{
		return E_UNEXPECTED;
	}

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	// apparently, WordArt 2.0 had some sizing problems.  The original
	// code has this call to GetExtent-->SetExtent, so I've kept it
	// here.
	
	if( _fIsWordArt2 )
	{
		//ignore errors.  If anything fails, too bad.
		FetchObjectExtents();	// this will reset _sizel
		SetExtent(SE_NOTACTIVATING);
	}

	// some objects are lame and draw outside the
	// areas they are supposed to.  So we need to 
	// just invalidate everything and redraw.

	// Note that we do this in UIDeactivate as well; however, the
	// double invalidation is necessary to cover some re-entrancy 
	// cases where we might be painted before everything is ready.

	_ped->TxInvalidateRect(NULL, TRUE);

	return NOERROR;
}

/*
 *	COleObject::DiscardUndoState
 *
 *	@mfunc	implementation of IOleInPlaceSite::DiscardUndoState.
 *
 *	@rdesc	NOERROR
 */
STDMETHODIMP COleObject::DiscardUndoState(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, 
			"COleObject::DiscardUndoState");

	// nothing to do here; we don't keep any OLE-undo state as it's
	// note very compatible with multi-level undo.
	
	return NOERROR;
}

/*
 *	COleObject::DeactivateAndUndo
 *
 *	@mfunc	implementation of IOleInPlaceSite::DeactivateAndUndo--
 *			called by an active object when the user invokes undo
 *			in the active object
 *
 *	@rdesc	NOERROR	(yep, richedit1.0 ignored all the errors here)
 */
STDMETHODIMP COleObject::DeactivateAndUndo(void)
{
	CStabilize	stabilize(this);

  	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::DeactivateAndUndo");

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	// ignore the error
	_ped->InPlaceDeactivate();

	// COMPATIBILITY ISSUE: we don't bother doing any undo here, as 
	// a multi-level undo model is incompatible with OLE undo.

	return NOERROR;
}

/*
 *	COleObject::OnPosRectChange
 *
 *	@mfunc	implementation of IOleInPlaceSite::OnPosRectChange.  This
 *			method is called by an in-place object when its extents have
 *			changed
 *
 *	@rdesc	HRESULT
 */
STDMETHODIMP COleObject::OnPosRectChange(LPCRECT prcPos)
{
	IOleInPlaceObject *pipo;
 	RECT rcClip;
	RECT rcNewPos;
	HRESULT hr;
	CCallMgr	callmgr(_ped);
	CStabilize stabilize(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnPosRectChange");
	
	if( !prcPos )
	{
		return E_INVALIDARG;
	}

	if( !_punkobj )
	{
		return E_UNEXPECTED;
	}
		
    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	if( !_ped->fInplaceActive() )
	{
		return E_UNEXPECTED;
	}
        
	// check to see if the rect moved; we don't allow this, but
	// do allow the object to keep the new size

	rcNewPos = *prcPos;

	if( prcPos->left != _rcPos.left	|| prcPos->top != _rcPos.top )
	{
		rcNewPos.right = rcNewPos.left + (prcPos->right - prcPos->left);
		rcNewPos.bottom = rcNewPos.top + (prcPos->bottom - prcPos->top);
	}

	_ped->TxGetClientRect(&rcClip);

	if( (hr = _punkobj->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo)) 
		== NOERROR )
	{
		hr = pipo->SetObjectRects(&rcNewPos, &rcClip);
        pipo->Release();
	}

	return hr;
}

/*
 *	COleObject::OnDataChange 
 *
 *	@mfunc	implementation of IAdviseSink::OnDataChange
 *
 *	@rdesc	NOERROR
 */
STDMETHODIMP_(void) COleObject::OnDataChange(
	FORMATETC *pformatetc,		//@parm the format of the data that changed
	STGMEDIUM *pmedium)			//@parm the data that changed
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnDataChange");
	CCallMgr	callmgr(_ped);

    if( IsZombie() )
	{
        return;
	}
	_pi.dwFlags &= ~REO_BLANK;
	// this will also set the modified flag
	_ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);

	return;
}

/*
 *	COleObject::OnViewChange
 *
 *	@mfunc	implementation of IAdviseSink::OnViewChange.  Notifies
 *			us that the object's view has changed.
 *
 *	@rdesc	HRESULT
 *
 */
STDMETHODIMP_(void) COleObject::OnViewChange(
	DWORD	dwAspect,		//@parm the aspect that has changed
	LONG	lindex)			//@parm unused
{
	CStabilize	stabilize(this);
	CCallMgr	callmgr(_ped);
	CDisplay *pdp;
		
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnViewChange");
	
	if( !_punkobj )
	{
		return;		// E_UNEXPECTED
	}

    if( IsZombie() )
	{
        return;
	}
	_pi.dwFlags &= ~REO_BLANK;
	// Richedit1.0 ignored errors on getting the object extents
	
	FetchObjectExtents();

    if( IsZombie() )
	{
        return;
	}
        
	pdp = _ped->_pdp;

	if( pdp )
	{
		pdp->OnPostReplaceRange(INFINITE, 0, 0, _cp, _cp + 1);
	}

	//_ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);

	return;
}
	
/*
 *	COleObject::OnRename
 *
 *	@mfunc	implementation of IAdviseSink::OnRename.  Notifies the container
 *			that the object has been renamed
 *
 *	@rdesc	E_NOTIMPL
 */
STDMETHODIMP_(void) COleObject::OnRename(
	IMoniker *pmk)			//@parm the object's new name
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnRename");
	
	TRACEWARNSZ("IAdviseSink::OnRename not implemented!");

	return;	// E_NOTIMPL;
}

/*
 *	COleObject::OnSave
 *
 *	@mfunc	implementation of IAdviseSink::OnSave.  Notifies the container
 *			that an object has been saved
 *
 *	@rdesc	NOERROR
 */
STDMETHODIMP_(void) COleObject::OnSave(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnSave");
	_pi.dwFlags &= ~REO_BLANK;
}

/*
 *	COleObject::OnClose
 *
 *	@mfunc	implementation of IAdviseSink::OnClose.  Notifies the container
 *			that an object has been closed.
 *
 *	@rdesc	NOERROR
 */
STDMETHODIMP_(void) COleObject::OnClose(void)
{
	
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::OnClose");
	
    if( IsZombie() )
	{
        return;
	}
        
	// if the object is blank (i.e. no data in it),we don't want to leave
	// it in the backing store--there is nothing for us to draw && therefore
	// nothing for the user to click on!  So just delete the object with
	// a space.  Note that 1.0 actually deleted the object; we'll just
	// replace it with a space to make everything work out right.
	if( (_pi.dwFlags & REO_BLANK) )
	{
		CCallMgr	callmgr(_ped);
		CStabilize	stabilize(this);
		CRchTxtPtr	rtp(_ped, _cp);

		// we don't want the delete of this object to go on the undo
		// stack.  We use a space so that cp's will work out right for
		// other undo actions.
		rtp.ReplaceRange(1, 1, L" ", NULL, -1);
	}
	_ped->TxSetForegroundWindow();
}
				
/*
 *	COleObject::OnPreReplaceRange
 *
 *	@mfunc	implementation of ITxNotify::OnPreReplaceRange
 *			called before changes are made to the backing store
 *
 *	@rdesc		void
 */
void COleObject::OnPreReplaceRange(
	DWORD cp, 			//@parm cp of the changes
	DWORD cchDel,		//@parm #of chars deleted
	DWORD cchNew,		//@parm # of chars added
	DWORD cpFormatMin, 	//@parm min cp of formatting changes
	DWORD cpFormatMax)	//@parm max cp of formatting changes
{
	Assert(_fInUndo == FALSE);
}

/*
 *	COleObject::OnPostReplaceRange
 *
 *	@mfunc	implementation of ITxNotify::OnPostReplaceRange
 *			called after changes are made to the backing store
 *
 *	@rdesc	void
 *	
 *	@comm	we use this method to keep our cp's up-to-date
 */
void COleObject::OnPostReplaceRange(
	DWORD cp, 			//@parm cp of the changes
	DWORD cchDel,		//@parm #of chars deleted
	DWORD cchNew,		//@parm # of chars added
	DWORD cpFormatMin, 	//@parm min cp of formatting changes
	DWORD cpFormatMax)	//@parm max cp of formatting changes
{
	// the only case we need to worry about is when the changes
	// come before our object

	Assert(_fInUndo == FALSE);

	_fDraw = TRUE;
	if( cp <= _cp )
	{		
		if( cp + cchDel > _cp )
		{
			_fDraw = FALSE;
			return;
		}
		else
		{
			_cp += (cchNew - cchDel);
		}
	}
}
		
/*
 *	COleObject::Zombie ()
 *
 *	@mfunc
 *		Turn this object into a zombie
 *
 */
void COleObject::Zombie ()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Zombie");

	_ped = NULL;
}

/*
 *	COleObject::COleObject
 *
 *	@mfunc	constructor
 *
 *	@rdesc	void
 */
COleObject::COleObject(
	CTxtEdit *ped)	//@parm context for this object
{
	CNotifyMgr *pnm;
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::COleObject");

	AddRef();

	// most of the values will be NULL by virtue of the allocator
	_ped 		= ped;

	pnm = ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Add( (ITxNotify *)this );
	}
}

/*
 *	COleObject::GetObjectData
 *
 *	@mfunc	fills out an REOBJECT structure with information relevant
 *			to this object
 *
 *	@rdesc	HRESULT
 */
HRESULT	COleObject::GetObjectData(
	REOBJECT *preobj, 		//@parm struct to fill out
	DWORD dwFlags)			//@parm indicate what data is requested
{
	IOleObject *poo = NULL;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::GetObjectData");

	Assert(preobj);
	Assert(_punkobj);

	preobj->cp = _cp;
	
	if( _punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR )
	{
		// don't worry about failures here
		poo->GetUserClassID(&(preobj->clsid));
	}
	
	preobj->dwFlags 	= _pi.dwFlags;
	preobj->dvaspect 	= _pi.dvaspect;
	preobj->dwUser 		= _pi.dwUser;
	preobj->sizel		= _sizel;		

   	if( (dwFlags & REO_GETOBJ_POLEOBJ) )
	{
		preobj->poleobj = poo;
		if( poo )
		{
			poo->AddRef();
		}
	}
	else
	{
		preobj->poleobj = NULL;
	}

    if( poo )
        poo->Release();

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	if( (dwFlags & REO_GETOBJ_PSTG) )
	{
		preobj->pstg = _pstg;
		if( _pstg )
		{
			_pstg->AddRef();
		}
	}
	else
	{
		preobj->pstg = NULL;
	}

	if( (dwFlags & REO_GETOBJ_POLESITE) )
	{
		// COMPATIBILITY HACK!!  Note that we don't 'release' any pointer that
		// may already be in the stored in the site.  RichEdit1.0 always sets
		// the value, consequently several apps pass in garbage for the site.
		//
		// If the site was previously set, we will get a reference counting
		// bug, so be sure that doesn't happen!
     
       	preobj->polesite = (IOleClientSite *)this;
       	AddRef();
 	}
	else
	{
		preobj->polesite = NULL;
	}
	return NOERROR;
}	

/*
 *	COleObject::IsLink
 *
 *	@mfunc	returns TRUE if the object is a link
 *
 *	@rdesc	BOOL
 */
BOOL COleObject::IsLink()
{
	return !!(_pi.dwFlags & REO_LINK);
}


/*
 *	COleObject::InitFromREOBJECT
 *
 *	@mfunc	initializes this object's state from the given
 *			REOBJECT data structure
 *
 *	@rdesc	HRESULT
 */
HRESULT COleObject::InitFromREOBJECT(
	DWORD	cp,			//@parm the cp for the object
	REOBJECT *preobj)	//@parm	the data to use for initialization
{
	IOleLink *plink;
	HRESULT	hr = E_INVALIDARG;
	CRchTxtPtr rtp(_ped, 0);
	POINT pt;
	
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::InitFromREOBJECT");
	
	Assert(_punkobj == NULL);
    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	_cp = cp;

	if( preobj->poleobj )
	{
		hr = preobj->poleobj->QueryInterface(IID_IUnknown, (void **)&_punkobj);
	}
	else
	{
		_punkobj = (IOleClientSite *) this;
		AddRef();
		hr = NOERROR;
	}
        
	if( hr != NOERROR )
	{
		return hr;
	}
	
	_pstg = preobj->pstg;
	if( _pstg )
	{
		_pstg->AddRef();
	}

	_pi.dwFlags = preobj->dwFlags & REO_READWRITEMASK;
	_pi.dwUser = preobj->dwUser;
	_pi.dvaspect = preobj->dvaspect;

	_sizel = preobj->sizel;		// COMPATIBILITY ISSUE: the RE1.0 code had some
								// stuff to deal with REO_DYNAMICSIZE here.  We
								// do not currently support that.
	
	if( _punkobj->QueryInterface(IID_IOleLink, (void **)&plink) == NOERROR )
	{
		_pi.dwFlags |= REO_LINK | REO_LINKAVAILABLE;
		plink->Release();
	}

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	if( IsEqualCLSID(preobj->clsid, CLSID_StaticMetafile) ||
		IsEqualCLSID(preobj->clsid, CLSID_StaticDib) ||
		IsEqualCLSID(preobj->clsid, CLSID_Picture_EnhMetafile) )
	{
		_pi.dwFlags |= REO_STATIC;
	}
	else if( IsExcelCLSID(preobj->clsid) )
	{
		_pi.dwFlags |= REO_GETMETAFILE;
	}
	else if( IsEqualCLSID(preobj->clsid, CLSID_WordArt ) )
	{
		_fIsWordArt2 = TRUE;
	}
	else if(IsEqualCLSID(preobj->clsid, CLSID_PaintbrushPicture) ||
			IsEqualCLSID(preobj->clsid, CLSID_BitmapImage))
	{
		_fIsPaintBrush = TRUE;

		// These calls will initialize the flag, _fPBUseLocalSizel, which
		// indicates that for this PB object, SetExtent calls are not 
		// acknowledged by the object, and we are to use our local value
		// of _sizel as the object extents.
		FetchObjectExtents();
		SetExtent(SE_NOTACTIVATING);
	}

	hr = ConnectObject();

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
        
	// this is a bit non-intuitive, but we need to figure out
	// where the object would be so that it can inplace activate correctly.

	if( cp )
	{
		cp--;
	}

	rtp.SetCp(cp);

	_ped->_pdp->PointFromTp(rtp, NULL, FALSE, pt, NULL, TA_TOP);
	_rcPos.top = _rcPos.bottom = pt.y;	//bottom will be set below in
		                                    // FetchExtents
	_rcPos.left = _rcPos.right = pt.x;

	if (preobj->sizel.cx || preobj->sizel.cy)
	{
		_sizel = preobj->sizel;
	}
	else
	{
		FetchObjectExtents();
	}
	ResetPosRect();

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
    
    // finally, lock down Link objects so they we don't try to refetch their
	// extents from the server.  After initialization, link object size is
	// entirely determined by the container.
	if( (_pi.dwFlags & REO_LINK) )
    {
        // so we don't call GetExtents on remeasuring.
        _fSetExtent = TRUE;
	}
	return NOERROR;
}


/*
 *	COleObject::MeasureObj(pdp, xWidth, yHeight, yDescent)
 *
 *	@mfunc	calculates the size of this object in device units
 *
 *	@rdesc	void
 */
void COleObject::MeasureObj(
	const CDisplay *pdp,	//@parm	the device to measure for 
	LONG &xWidth,			//@parm the width of the object 
	LONG &yHeight,
	SHORT yDescent)		   	//@parm the height of the object
{
	xWidth = pdp->HimetricXtoDX(_sizel.cx);
	yHeight = pdp->HimetricYtoDY(_sizel.cy);
	if (!(_pi.dwFlags & REO_BELOWBASELINE))
	{
		yHeight += yDescent;
	}
}

/* 
 * COleObject::InHandle
 *
 * @mfunc  See if a point is in the rectangle defined by the handle at
 *		the given coordinates.
 *
 * @rdesc True if point is in handle.
 * 
 */
BOOL COleObject::InHandle(
	int x,	//@parm x pos of upper left corner coordinate of the handle box.
	int y,	//@parm y pos of upper left corner coordinate of the handle box.
	const POINT &pt)	//@parm point to check
{
    RECT    rc;
	BOOL	fRet;
    
    rc.left = x;
    rc.top = y;
	//Add one to bottom right because PtInRect does not consider
	//points on bottom or right to be in rect.
    rc.right = x + dxyHandle + 1;
    rc.bottom = y + dxyHandle + 1;
    fRet = PtInRect(&rc, pt);

	return fRet;
}  

/*
 *	COleObject::CheckForHandleHit
 *
 *	@mfunc	Check for a hit on any of the frame handles.
 *
 *	@rdesc	 NULL if no hit, cursor resource ID if there is a hit.
 *
 */
LPTSTR COleObject::CheckForHandleHit(
	const POINT &pt)	//@parm POINT containing client coord. of the cursor.
{
	RECT	rc;

	// if the object is not resizeable, no chance of hitting a resize
	// handle!
	if( !(_pi.dwFlags & REO_RESIZABLE) )
	{
		return NULL;
	}

	CopyRect(&rc, &_rcPos);

	if (!_dxyFrame)
	{
		_dxyFrame = dxyFrameDefault;
	}

	//Check to see if point is farther into the interior of the
	//object than the handles extent. If it is we can just bail.
	InflateRect(&rc, -(_dxyFrame + dxyHandle), -(_dxyFrame + dxyHandle));
	if (PtInRect(&rc, pt))
	{
		return NULL;
	}

	//Check to see if point is in any of the handles and
	//return the proper cursor ID if it is.
	InflateRect(&rc, dxyHandle, dxyHandle);

	if(InHandle(rc.left, rc.top, pt) ||
	   InHandle(rc.right-dxyHandle, rc.bottom-dxyHandle, pt))
	{
		return IDC_SIZENWSE;
	}
	if(InHandle(rc.left, rc.top+(rc.bottom-rc.top-dxyHandle)/2, pt) ||
	   InHandle(rc.right-dxyHandle,
			rc.top+(rc.bottom-rc.top-dxyHandle)/2, pt))
	{
		return IDC_SIZEWE;
	}
	if(InHandle(rc.left, rc.bottom-dxyHandle, pt) ||
	   InHandle(rc.right-dxyHandle, rc.top, pt))
	{
		return IDC_SIZENESW;
	}
	if(InHandle(rc.left+(rc.right-rc.left-dxyHandle)/2, rc.top, pt) ||
	   InHandle(rc.left+(rc.right-rc.left-dxyHandle)/2,
			rc.bottom-dxyHandle, pt))
	{
		return IDC_SIZENS;
	}
	return NULL;
}

/* 
 * COleObject::DrawHandle
 *
 * @mfunc  Draw a handle on the object frame at the specified coordinate
 *
 * @rdesc void
 * 
 */
void COleObject::DrawHandle(
	HDC hdc,	//@parm HDC to be drawn into
	int x,		//@parm x pos of upper left corner coordinate of the handle box
	int y)		//@parm y pos upper left corner coordinate of the handle box
{
    RECT    rc;
    
	//Draw the handle by inverting.
    rc.left = x;
    rc.top = y;
    rc.right = x + dxyHandle;
    rc.bottom = y + dxyHandle;
    InvertRect(hdc, (LPRECT)&rc);
}  

/*
 *	COleObject::DrawFrame
 *
 *	@mfunc	Draw a frame around the object.  Invert if required and
 *		include handles if required.
 *
 *	@rdesc	void
 *
 */
void COleObject::DrawFrame(
	const CDisplay *pdp,    //@parm the display to draw to
	HDC             hdc,	//@parm the device context
	RECT           *prc)  //@parm the rect around which to draw
{
	RECT	rc;

	CopyRect(&rc, prc);

	if (_pi.dwFlags & REO_INVERTEDSELECT)
	{
		//Invert entire object
		InvertRect(hdc, &rc);
	}
	else
	{
		// Just the border, so use a null brush
		SaveDC(hdc);
		SetROP2(hdc, R2_NOT);
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
		RestoreDC(hdc, -1);
	}

	if (_pi.dwFlags & REO_RESIZABLE)
	{
		int     bkmodeOld;
		HPEN	hpen;
		LOGPEN	logpen;

		bkmodeOld = SetBkMode(hdc, TRANSPARENT);
		Assert(bkmodeOld);

		//Get the frame width
		_dxyFrame = dxyFrameDefault;
		hpen = (HPEN)GetCurrentObject(hdc, OBJ_PEN);
		if( W32->GetObject(hpen, sizeof(LOGPEN), &logpen) )
		{
			if( logpen.lopnWidth.x )
			{
				_dxyFrame = (SHORT)logpen.lopnWidth.x;
			}
		}

		// Draw the handles inside the rectangle boundary
 		InflateRect(&rc, -_dxyFrame, -_dxyFrame);

		DrawHandle(hdc, rc.left, rc.top);
		DrawHandle(hdc, rc.left, rc.top	+ (rc.bottom-rc.top-dxyHandle)/2);
		DrawHandle(hdc, rc.left, rc.bottom-dxyHandle);
		DrawHandle(hdc, rc.left + (rc.right - rc.left - dxyHandle)/2, rc.top);
		DrawHandle(hdc, rc.left+(rc.right-rc.left-dxyHandle)/2,
			rc.bottom-dxyHandle);
		DrawHandle(hdc, rc.right-dxyHandle, rc.top);
		DrawHandle(hdc, rc.right-dxyHandle,
			rc.top+(rc.bottom-rc.top-dxyHandle)/2);
		DrawHandle(hdc, rc.right-dxyHandle, rc.bottom-dxyHandle);

		SetBkMode(hdc, bkmodeOld);
	}
}


/*
 *	COleObject::CreateDib
 *
 *	@mfunc	Create DIB for Windows CE display
 *
 *	@rdesc	void
 *
 */
void COleObject::CreateDib(HDC hdc)
{
    BYTE            *pbDib;
	HGLOBAL			hnew = NULL;
	BYTE			*pbSrcBits;
	LPBITMAPINFO	pbmi = (LPBITMAPINFO) GlobalLock(_hdata);
	int				iBitsPerPix, iAdjustedWidth, iNumColors;
	DWORD			dwColors, dwImage;

	iBitsPerPix = pbmi->bmiHeader.biBitCount;

	ASSERT(iBitsPerPix == 1 || iBitsPerPix == 4 ||
		iBitsPerPix == 8 || iBitsPerPix == 16 || iBitsPerPix == 24 || iBitsPerPix == 32);

	iAdjustedWidth = ((pbmi->bmiHeader.biWidth * iBitsPerPix + 31) & ~31) / 8;

	iNumColors = pbmi->bmiHeader.biClrUsed;

	if ((iNumColors == 0) && (iBitsPerPix <= 8)){
		iNumColors = 1 << iBitsPerPix;
	}

	dwColors = iNumColors * sizeof(RGBQUAD) + (pbmi->bmiHeader.biCompression == BI_BITFIELDS ? 3 * sizeof(DWORD) : 0);		
	dwImage = pbmi->bmiHeader.biHeight * iAdjustedWidth;

	// Bitmap bits location
	pbSrcBits = (BYTE*)(pbmi) + sizeof(BITMAPINFOHEADER) + dwColors;

	if(16 == iBitsPerPix &&
		(pbmi->bmiHeader.biCompression == BI_BITFIELDS))	
	{
		// Sixteen-bit case: fill in the bitfields mask for 565
		#define MASK565_0    0x0000F800
		#define MASK565_1   0x000007E0
		#define MASK565_2   0x0000001F

		((DWORD*)(pbmi->bmiColors))[0] = MASK565_0;
		((DWORD*)(pbmi->bmiColors))[1] = MASK565_1;
		((DWORD*)(pbmi->bmiColors))[2] = MASK565_2;
	}

	_hdib = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, (void**)&pbDib, NULL, 0);
	if (_hdib == NULL)
	{
		DWORD dwle = ::GetLastError();

        _ped->GetCallMgr()->SetOutOfMemory();

        // V-GUYB:
        // Do not attempt to repaint this picture until the user starts typing in the
        // control. This allows the user to dismiss the oom that will appear and then
        // save the document, and then free up some space. If we don't do this here, 
        // every time the oom msg is dismissed it will appear again. This doesn't allow 
        // the user to save the document unless they can find some memory to free.
        _fDraw = FALSE;

		TRACEWARNSZ("Out of memory creating DIB");
		return;
	}

	// Move Bitmap bits
	CopyMemory(pbDib, pbSrcBits, dwImage);

	GlobalUnlock(pbmi);
	GlobalFree(hnew);
}

/*
 *	COleObject::DrawDib : Auxiliary function
 *
 *	@mfunc	draws the dib in the given dc
 *
 *	@rdesc void
 */
void COleObject::DrawDib(
	HDC hdc,
	RECT *prc
)
{
//	HDC hdcMem = CreateCompatibleDC(hdc);
	HDC hdcMem; // V-GUYB: Create hdcMem later in case we return early.
	LPBITMAPINFO	pbmi;

	if (!_hdib)
		{
		CreateDib(hdc);
		}
	// If _hdib is still NULL, just return.  Maybe out of memory.
	if (!_hdib)
		{
		goto leave;
		}

    // V-GUYB: NOW create the mem dc.
	hdcMem = CreateCompatibleDC(hdc);
	if(!hdcMem)
		{
		goto leave;
		}     

	pbmi = (LPBITMAPINFO) LocalLock(_hdata);
	SelectObject(hdcMem, _hdib);

    StretchBlt(hdc, prc->left, prc->top,
			prc->right - prc->left, prc->bottom - prc->top,
			hdcMem, 0, 0, pbmi->bmiHeader.biWidth, pbmi->bmiHeader.biHeight, SRCCOPY);

	GlobalUnlock(pbmi);
	DeleteDC(hdcMem);
leave:
	return;
}

/*
 *	COleObject::DrawObj
 *
 *	@mfunc	draws the object
 *
 *	@rdesc void
 */
void COleObject::DrawObj(
const CDisplay *pdp,	//@parm the display object for the view
	HDC hdc,				//@parm the drawing HDC (can be different
							//than the display.
	BOOL fMetafile,			//@parm whether the HDC is a metafile
	POINT *ppt,			 	//@parm top left corner of where to draw
	RECT  *prcRender)       //@parm pointer to render rectangle
{
	RECT rc, rc1;
	IViewObject *pvo;
	CDisplayPrinter *pdpPrint;
	LONG adjust = 0;
	CObjectMgr * pobjmgr = _ped->GetObjectMgr();

    if (NULL == pobjmgr)
    {
        Assert(pobjmgr);
        SetLastError(ERROR_OUTOFMEMORY);
        return;
    }
    

	// if we aren't running in transparent mode, 
	// Clear the space in the render rectangle
	// This includes space for the object
	// It also includes any surrounding space
	if( !_ped->_fTransparent )
	{
		rc1.left = prcRender->left;
		rc1.top = prcRender->top;
		rc1.right = ppt->x + _rcPos.right - _rcPos.left;
		rc1.bottom = prcRender->bottom;

		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc1, NULL, 0, NULL);
	}

	if( _fInPlaceActive || !_fDraw)
	{
		// if we're inplace active, don't do anything, the
		// server is drawing for us.
		// We also don't do anything prior to the fDraw property being set
		return;
	}

 	ResetPosRect(&adjust);		// Update position rectangle

	// Draw the object where we are asked within the rendering rectangle
   	rc.top = ppt->y;
	rc.top += adjust;

	// Note we always convert bottom and top because this code is executed for
	// all draws which can in turn be for the screen, the printer or a metafile.
	rc.bottom = rc.top + MulDiv(_rcPos.bottom - _rcPos.top, pdp->GetXPerInch(), 
		sysparam.GetXPerInchScreenDC());

	rc.left = ppt->x;

	rc.right = rc.left + MulDiv(_rcPos.right - _rcPos.left, pdp->GetYPerInch(), 
		sysparam.GetYPerInchScreenDC());

	SaveDC(hdc);
	SetTextAlign(hdc, TA_TOP);

	SaveDC(hdc);  // calls to OLE object (IViewObject::Draw or OleDraw) might change HDC
	if (_hdata)
	{
		// This is some Windows CE Dib, let's try the direct approach
		DrawDib( hdc, &rc );
	}
	else if( fMetafile )
	{
		if( _punkobj->QueryInterface(IID_IViewObject, (void **)&pvo) 
				== NOERROR )
		{
			pdpPrint = (CDisplayPrinter *)pdp;
			rc1 = pdpPrint->GetPrintPage();

			// fix up rc for Draw()
			rc1.bottom = rc1.bottom - rc1.top;			    
			rc1.right = rc1.right - rc1.left;

			pvo->Draw(_pi.dvaspect, -1, NULL, NULL, 0, hdc, (RECTL *)&rc,
					(RECTL *)&rc1, NULL, 0);
			pvo->Release();
		}
	}
	else
	{
		pOleDraw(_punkobj, _pi.dvaspect, hdc, &rc);
	}
	RestoreDC(hdc, -1);

	// Do selection stuff if the this is for the main (screen) view.
	if (pdp->IsMain())
	{
		if( _pi.dwFlags & REO_OPEN )
		{
			OleUIDrawShading(&rc, hdc);
		}

		//If the object has been selected by clicking on it, draw
		//a frame and handles around it.  Otherwise, if we are selected
		//as part of a range, invert ourselves.
		if (!fMetafile && pobjmgr->GetSingleSelect() == this)
		{
			DrawFrame(pdp, hdc, &rc);
		}
		else
		{
			LONG cpMin, cpMost;

			_ped->GetSelRangeForRender(&cpMin, &cpMost);
			if ((LONG)_cp >= cpMin && 
				(LONG)_cp < cpMost &&
				cpMost - cpMin > 1)
			{
				InvertRect(hdc, &rc);
			}
		}
	}
	RestoreDC(hdc, -1);
}

/*
 *	COleObject::Delete
 *
 *	@mfunc	deletes this object from the backing store _without_
 *			making outgoing calls.  The commit on generated anti-events
 *			will handle the outgoing calls
 */
void COleObject::Delete(IUndoBuilder *publdr)
{
	CNotifyMgr *pnm;
	IAntiEvent *pae;

	Assert(_fInUndo == FALSE);
	_fInUndo = TRUE;

	pnm = _ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Remove( (ITxNotify *)this );
	}

	if( publdr )
	{
		// the anti-event will take care of calling IOO::Close
		// for us.
		pae = gAEDispenser.CreateReplaceObjectAE(_ped, this);

		if( pae )
		{
			publdr->AddAntiEvent(pae);
		}
	}
	else
	{
		Close(OLECLOSE_NOSAVE);
		MakeZombie();
	}

	// if we're being deleted, we can't be selected anymore
	_pi.dwFlags &= ~REO_SELECTED;
	_fDraw = 0;

}

/*
 *	COleObject::Restore
 *
 *	@mfunc	restores the object from the undo state back into the
 *			backing store
 *
 *			No outgoing calls will be made
 */
void COleObject::Restore(void)
{
	CNotifyMgr *pnm;

	Assert(_fInUndo == TRUE);

	_fInUndo = FALSE;
	_fDraw = TRUE;

	pnm = _ped->GetNotifyMgr();

	if( pnm )
	{
		pnm->Add( (ITxNotify *)this );
	}
}

/*
 *	COleObject::SetREOSELECTED
 *
 *	@mfunc	cmember set REO_SELECTED state
 *
 *	@rdesc	void
 */
void COleObject::SetREOSELECTED(BOOL fSelect)
{
	if( fSelect )
	{
		_pi.dwFlags |= REO_SELECTED;
	}
	else
	{								
		_pi.dwFlags &= ~REO_SELECTED;
	}
}
    
/*
 *	COleObject::Close
 *
 *	@mfunc	closes this object
 *
 *	@rdesc	void
 */
void COleObject::Close(
	DWORD	dwSave)		//same as IOleObject::Close
{
	IOleObject *poo;
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::Close");

	if( !_punkobj )
	{
		return;
	}

	if( _punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR )
	{
		poo->Close(dwSave);
		poo->Release();
	}
}

/*
 *	COleObject::ScrollObject
 *
 *	@mfunc	updates _rcPos if we were scrolled
 *
 *	@rdesc	void
 */
void COleObject::ScrollObject(
	LONG dx,			//@parm change in the x direction
	LONG dy,			//@parm change in the y direction
	LPCRECT prcScroll)	//@parm the rect that is being scrolled
{
	RECT rcInter;

	// if we're inplace active, OnReposition will handle the scrolling
	if( !_fInPlaceActive && !_fGuardPosRect &&
		IntersectRect(&rcInter, &_rcPos, prcScroll) )
	{
		OffsetRect(&_rcPos, dx, dy);
	}
}

				
//
//	PRIVATE methods
//


/*
 *	COleObject::~COleObject
 *
 *	@mfunc	destructor
 *
 *	@rdesc	void
 */
COleObject::~COleObject(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::~COleObject");

	CleanupState();
}

/*
 *	COleObject::SavePrivateState
 *
 *	@mfunc	Saves information such as the aspect and various flags
 *	into the object's storage.
 *
 *	@devnote	This method is used mostly for compatibility with 
 *	richedit 1.0--we save the same information they did.
 *
 *	Also note that this method returns void--even if any particular
 *	call failes, we should be able to "recover" and limp along.
 *	Richedit 1.0 also had this behavior.
 */
void COleObject::SavePrivateState(void)
{
	HRESULT	hr;
	IStream *	pstm;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::SavePrivateState");

	Assert(_pstg);

	hr = _pstg->CreateStream(szSiteFlagsStm, STGM_READWRITE |
					STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, 0, &pstm );

    if( IsZombie() )
	{
        return;
	}
        
	if( hr == NOERROR )
	{
		pstm->Write(&_pi, sizeof(PersistedInfo), NULL);
		pstm->Release();
	}
}

/*
 *	COleObject::FetchObjectExtents
 *
 *	@mfunc 	determines the object's size in himetric.  Typically, this
 *			is achieved via IOleObject::GetExtent, but some error 
 *			recovery is implemented
 *
 *	@rdesc	void.  _sizel is updated
 */
void COleObject::FetchObjectExtents(void)
{
	HRESULT hr = NOERROR;
	IOleObject *poo;
	IViewObject2 *pvo;
	CDisplay *pdp;

    if( IsZombie() )
	{
        return;
	}
        
	// We _don't_ want to make calls to GetExtent if:
	// (1) We have outstanding updates to _sizel for which we
	//		haven't successfully called SetExtent
	// (2) This is a PaintBrush object and the most recent call 
	//		to SetExtent for this PB object failed

	if(!(_fSetExtent || (_fIsPaintBrush && _fPBUseLocalSizel)))
	{	
		// try IOleObject::GetExtent as long as we shouldn't try for
		// the metafile first.

		if( !(_pi.dwFlags & REO_GETMETAFILE) )
		{
			hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);


			if( hr == NOERROR )
			{
				hr = poo->GetExtent(_pi.dvaspect, &_sizel);
				poo->Release();
			}
		
			if( IsZombie() )
			{
				return;
			}
		}
		else
		{
			hr = E_FAIL;
		}
        
		if( hr != NOERROR )
		{
			if( _punkobj->QueryInterface(IID_IViewObject2, (void **)&pvo) == NOERROR )
			{
				hr = pvo->GetExtent(_pi.dvaspect, -1, NULL, &_sizel);
				pvo->Release();
			}
		}

	    if( IsZombie() )
		{
	        return;
		}
        
		if( hr != NOERROR || _sizel.cx == 0 || _sizel.cy == 0 )
		{
			_sizel.cx = _sizel.cy = 2000;
		}
	}
	// If _fSetExtent==TRUE, we've made a change to _sizel for which
	// we haven't called IOleObject::SetExtent successfully.  Therefore
	// fall through with existing _sizel.

	//update our position rectangle
	pdp = _ped->_pdp;

	_rcPos.right = _rcPos.left + pdp->HimetricYtoDY(_sizel.cx);
	_rcPos.bottom = _rcPos.top + pdp->HimetricXtoDX(_sizel.cy);
}

/*
 *	COleObject::ConnectObject
 *
 *	@mfunc	setup the necessary advises to the embedded object.
 *
 *	@rdesc 	HRESULT
 *
 *	@comm	This code is similar to ole2ui's OleStdSetupAdvises
 *
 */
HRESULT COleObject::ConnectObject()
{
	IViewObject *pvo;
	IOleObject *poo;
	HRESULT hr;
	CObjectMgr *pobjmgr;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::ConnectObject");
	
    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
	
	Assert(_punkobj);

	if( _punkobj->QueryInterface(IID_IViewObject, (void **)&pvo) == NOERROR )
	{
		pvo->SetAdvise(_pi.dvaspect, ADVF_PRIMEFIRST, (IAdviseSink *)this);
		pvo->Release();
	}

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
	
	if( (hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo)) 
		== NOERROR )
	{
		hr = poo->Advise((IAdviseSink *)this, &_dwConn);

		pobjmgr = _ped->GetObjectMgr();
		if (NULL == pobjmgr)
		{
		    Assert(pobjmgr);
		    poo->Release();
		    return E_OUTOFMEMORY;
		}    

		// the doc may be NULL, but not the app.  Don't do anything
		// if the app name is NULL
		if( pobjmgr->GetAppName())
		{

			hr = poo->SetHostNames(pobjmgr->GetAppName(), 
						pobjmgr->GetDocName());
		}
		poo->Release();
	}

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}
	
	pOleSetContainedObject(_punkobj, TRUE);

	return hr;
}

/*
 *	COleObject::DisconnectObject
 *
 *	@mfunc	reverses the connections made in ConnectObject and releases
 *			the object.  Note that the object's storage is _NOT_
 *			released.
 */
void COleObject::DisconnectObject()
{
	IOleObject *poo = NULL;
	IViewObject *pvo = NULL;

	if (IsZombie())
	{
		return;		// Already Disconnected.
	}

	if( _punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR )
	{
		poo->SetClientSite(NULL);

		if( _dwConn )
		{
			poo->Unadvise(_dwConn );
		}
	
		poo->Release();
	}

	if( _punkobj->QueryInterface(IID_IViewObject, (void **)&pvo) == NOERROR )
	{
		pvo->SetAdvise(_pi.dvaspect, ADVF_PRIMEFIRST, NULL);
		pvo->Release();
	}

	pCoDisconnectObject(_punkobj, NULL);
	SafeReleaseAndNULL(&_punkobj);
}

/*
 *	COleObject::MakeZombie()
 *
 *	@mfunc	Force this object to enter a zombie state.  This
 *      is called when we should be gone but aren't.  It cleans
 *      up our state and flags us so we don't do nasty things
 *		between now and the time were are deleted.
 *
 */
void COleObject::MakeZombie()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::MakeZombie");

	CleanupState();

    Zombie();
}

/*
 *	COleObject::CleanupState()
 *
 *	@mfunc	Called on delete and when we become zombied.  It cleans
 *		up our member data and any other dependencies that need to
 *		be resolved.
 *
 */
void COleObject::CleanupState()
{
	CNotifyMgr *pnm;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::CleanupState");

    if( _ped && !_fInUndo )
	{
		pnm = _ped->GetNotifyMgr();

		if( pnm )
		{
			pnm->Remove( (ITxNotify *)this );
		}

		_ped = NULL;
	}

	DisconnectObject();

	if( _pstg )
	{
		SafeReleaseAndNULL((IUnknown**)&_pstg);
	}

	if (_hdib)
	{
		::DeleteObject(_hdib);
		_hdib = NULL;
	}
	GlobalFree(_hdata);
	_hdata = NULL;
	if (_pimageinfo)
	{
		delete _pimageinfo;
		_pimageinfo = NULL;
        }
}

/*
 *	COleObject::ActivateObj
 *	
 *	@mfunc Activates the object.
 *		args:
 *			message components of message initiating activation
 *				UINT uiMsg
 *				WPARAM wParam
 *				LPARAM lParam
 *	@rdesc
 *		BOOL		Whether the object has been activated.
 */
BOOL COleObject::ActivateObj(UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	LPOLEOBJECT		poo;
	HWND			hwnd;
	MSG				msg;
	DWORD			dwPos;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::AcitvateObj");

	if (_ped->TxGetWindow(&hwnd) != NOERROR)
	{
		return FALSE;
	}

	ResetPosRect();

	// Fill in the message structure
	msg.hwnd = hwnd;
	msg.message = uiMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	dwPos = GetMessagePos();
	msg.pt.x = (LONG) LOWORD(dwPos);
	msg.pt.y = (LONG) HIWORD(dwPos);

	// Execute the primary verb
	if( _punkobj->QueryInterface(IID_IOleObject, (void **)&poo) == NOERROR )
	{
		//Make sure we tell the object it's size has changed if we have not
		//already notified it.
		if (_fSetExtent)
		{
			SetExtent(SE_ACTIVATING);
		}

		HRESULT			hr;
		hr = poo->DoVerb(OLEIVERB_PRIMARY, &msg, (LPOLECLIENTSITE)this, 0, hwnd, &_rcPos);

#ifndef MACPORT
		if (FAILED(hr))
		{
			ENOLEOPFAILED	enoleopfailed;

			enoleopfailed.iob = _ped->_pobjmgr->FindIndexForCp(GetCp());
			enoleopfailed.lOper = OLEOP_DOVERB;
			enoleopfailed.hr = hr;
	        _ped->TxNotify( EN_OLEOPFAILED, &enoleopfailed );
		}
#endif
	    poo->Release();
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

/*
 *	COleObject::DeActivateObj
 *	
 *	@mfunc Deactivates the object.
 *
 */
HRESULT COleObject::DeActivateObj(void)
{
	IOleInPlaceObject * pipo;
	IOleObject *poo;
	MSG msg;
	HRESULT hr;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "COleObject::DeActivateObj");

	ResetPosRect();

	if( _punkobj->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo) 
		== NOERROR )
	{

		if( (hr  =_punkobj->QueryInterface(IID_IOleObject, (void **)&poo)) ==
				NOERROR ) 
		{
			// this code is a bit different from 1.0, but seems to 
			// make things work a bit better.  Basically, we've taken a leaf
			// from various sample apps and do the most brute force "de-activate"
			// possible (you'd think just one call would be enough ;-)

			// don't bother with the error return here.
			pipo->UIDeactivate();
			
			//fake something
			ZeroMemory(&msg, sizeof(MSG));
			msg.message = WM_LBUTTONDOWN;	
			_ped->TxGetWindow(&msg.hwnd);

			// again, don't bother checking for errors; we need to
			// plow through and get rid of stuff as much as possible.
			poo->DoVerb(OLEIVERB_HIDE, &msg, (IOleClientSite *)this,
				-1, msg.hwnd, &_rcPos);

			// COMPATIBILITY ISSUE (alexgo): the RE1.0 code did some funny
			// stuff with undo here, but I don't think it's necessary now
			// with our multi-level undo model.
			hr = pipo->InPlaceDeactivate();

			poo->Release();
		}

	    pipo->Release();

		return hr;
	}
	return NOERROR; 
}

/*
 *	COleObject::Convert
 *
 *	@mfunc	Converts the object to the specified class.  Does reload
 *		the object but does NOT force an update (caller must do this).
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
HRESULT COleObject::Convert( 
	REFCLSID rclsidNew,			//@parm the destination clsid
	LPCSTR lpstrUserTypeNew)	//@parm the new user type name
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::Convert");

	CLIPFORMAT cfOld;
	CLSID clsidOld;
	LPOLESTR szUserTypeOld = NULL;
	HRESULT hr;
	HRESULT hrLatest;
	UsesMakeOLESTR;


	// If object has no storage, return
	if (!_pstg)
	{
		return ResultFromScode(E_INVALIDARG);
	}

	// Read the old class, format, and user type in
	if ((hr = pReadClassStg(_pstg, &clsidOld)) ||
		(hr = pReadFmtUserTypeStg(_pstg, &cfOld, &szUserTypeOld)))
	{
		return hr;
	}

	// Unload the object
	Close(OLECLOSE_SAVEIFDIRTY);
	_punkobj->Release();

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	// Write the new class and user type, but old format, into the storage
	if ((hr = pWriteClassStg(_pstg, rclsidNew)) ||
		(hr = pWriteFmtUserTypeStg(_pstg, cfOld,
			(LPOLESTR) MakeOLESTR(lpstrUserTypeNew))) ||
		(hr = pSetConvertStg(_pstg, TRUE)) ||
		((hr = _pstg->Commit(0)) && (hr = _pstg->Commit(STGC_OVERWRITE))))
	{
		// Uh oh, we're in a bad state; rewrite the original info
		(VOID) pWriteClassStg(_pstg, clsidOld);
		(VOID) pWriteFmtUserTypeStg(_pstg, cfOld, szUserTypeOld);
	}

    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	// Reload the object and connect. If we can't reload it, delete it.
	hrLatest = pOleLoad(_pstg, IID_IOleObject, (LPOLECLIENTSITE) this,
			(void **)&_punkobj);

	if (hrLatest != NOERROR)
	{
		CRchTxtPtr	rtp(_ped, _cp);

		// we don't want the delete of this object to go on the undo
		// stack.  We use a space so that cp's will work out right for
		// other undo actions.
		rtp.ReplaceRange(1, 1, L" ", NULL, -1);
	}
	else
	{
		ConnectObject();
	}

	// Free the old
	pCoTaskMemFree(szUserTypeOld);
	return hr ? hr : hrLatest;
}

/*
 *	COleObject::ActivateAs
 *
 *	@mfunc	Handles a request by the user to activate all objects of a particular
 *		class as objects of another class.
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
HRESULT COleObject::ActivateAs(REFCLSID rclsid, REFCLSID rclsidAs)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::ActivateAs");

	HRESULT hr = NOERROR;
	IOleObject * poo = NULL;
	CLSID	clsid;


	//Get the clsid of the object.
	hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo);
	if( hr == NOERROR )
	{
		//NOTE:  We are depending on the behavior of GetUserClassID to
		//return the current clsid of the object (not the TreatAs id).
		//This should hold true as long as long as we haven't reloaded
		//it yet.  If there are problems with ActivateAs in the future,
		//this might be a suspect.
		hr = poo->GetUserClassID(&clsid);
		poo->Release();
	}

	if( hr != NOERROR )
	{
		return hr;
	}
	
    if( IsZombie() )
	{
        return CO_E_RELEASED;
	}

	//Check to see if the clsid of the object matches the clsid to be
	//treated as something else. If it is we need to unload and reload
	//the object.
	if( IsEqualCLSID(clsid, rclsid) )
	{
		// Unload the object
		Close(OLECLOSE_SAVEIFDIRTY);
		_punkobj->Release();

		if( IsZombie() )
		{
			return CO_E_RELEASED;
		}

		// Reload the object and connect. If we can't reload it, delete it.
		hr = pOleLoad(_pstg, IID_IOleObject, (LPOLECLIENTSITE) this,
				(void **)&_punkobj);

		if (hr != NOERROR)
		{
			CRchTxtPtr	rtp(_ped, _cp);

			// We don't want the delete of this object to go on the undo
			// stack.  We use a space so that cp's will work out right for
			// other undo actions.
			rtp.ReplaceRange(1, 1, L" ", NULL, -1);
		}
		else
		{
			ConnectObject();
		}
	}

	return hr;
}

/*
 *	COleObject::SetLinkAvailable
 *
 *	@mfunc
 *		Allows client to tell us whether the link is available or not.
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
HRESULT COleObject::SetLinkAvailable( 
	BOOL fAvailable)	//@parm	if TRUE, make object linkable
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SetLinkAvailable");
	
	// If this is not a link, return
	if (!(_pi.dwFlags & REO_LINK))
	{
		return E_INVALIDARG;
	}

	// Set the flag as appropriate
	if (fAvailable)
	{
		_pi.dwFlags |= REO_LINKAVAILABLE;
	}
	else
	{
		_pi.dwFlags &= ~REO_LINKAVAILABLE;
	}
	return NOERROR;
}

/*
 *	COleObject::WriteTextInfoToEditStream
 *
 *	@mfunc
 *		Used for textize support,  Tries to determine the text
 *		representation for an object and then writes that info
 *		to the given stream.  The only thing this is particularly useful
 *		for is to support richedit1.0's TEXTIZED data format.
 *
 *	@rdesc
 *		LONG				Number of chras written..
 */
LONG COleObject::WriteTextInfoToEditStream(
	EDITSTREAM *pes)
{
	LONG cch;
	LONG cbWritten = 0;
	HRESULT hr;
	IOleObject *poo;
	IDataObject *pdataobj;
	STGMEDIUM med;
	char *pch;			//we only deal with ANSI data here

	HANDLE		hGlobal;


	if((hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo)) == NOERROR )
	{
		hr = poo->GetClipboardData( 0, &pdataobj);
        poo->Release();
	}

	if(FAILED(hr))
	{
		hr = _punkobj->QueryInterface(IID_IDataObject, (void **)&pdataobj);
		if(FAILED(hr))
		{
			pes->dwError = (DWORD) E_FAIL;
			goto Default;
		}
	}

	med.tymed = TYMED_HGLOBAL;
	med.pUnkForRelease = NULL;
	med.hGlobal = NULL;

	hr = pdataobj->GetData(&g_rgFETC[iAnsiFETC], &med);
	if(FAILED(hr))
	{
		pes->dwError = (DWORD)hr;
	}
	else
	{
		hGlobal = med.hGlobal;
		pch = (char *)GlobalLock(hGlobal);
		if( pch )
		{
			for (cch = 0; pch[cch]; cch++);
			pes->dwError = pes->pfnCallback(pes->dwCookie, (BYTE *)pch, cch,
												&cbWritten);
			GlobalUnlock(hGlobal);
		}

		pReleaseStgMedium(&med);
	}

Default:

	if(cbWritten <= 0)
	{
		char ch = ' ';

		pes->pfnCallback(pes->dwCookie, (BYTE *)&ch, sizeof(char), &cbWritten);
		pes->dwError = 0;
	}

    pdataobj->Release();
	return cbWritten;
}

/*
 *	COleObject::SetDvaspect
 *
 *	@mfunc	Allows client to tell us which aspect to use and force us
 *		to recompute positioning and redraw.
 *
 */
void COleObject::SetDvaspect( 
	DWORD dvaspect)	//@parm	the aspect to use
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SetDvaspect");

	_pi.dvaspect = dvaspect;
	
	// Cause ourselves to redraw and update
	OnViewChange(dvaspect, (DWORD) -1);
}

/*
 *	COleObject::HandsOffStorage
 *
 *	@mfunc	See IPersistStore::HandsOffStorage.
 *
 */
void COleObject::HandsOffStorage(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::HandsOffStorage");

	// Free the storage we currently have, if we have one.
	SafeReleaseAndNULL((IUnknown**)&_pstg);
}

/*
 *	COleObject::SaveCompleted
 *
 *	@mfunc	See IPersistStore::SaveCompleted.
 *
 */
void COleObject::SaveCompleted(
	LPSTORAGE lpstg)	//@parm	new storage
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "COleObject::SaveCompleted");

	// Did our caller give us a new storage to remember?
	if (lpstg)
	{
		// Free the storage we currently have, if we have one
		if (_pstg)
		{
			SafeReleaseAndNULL((IUnknown**)&_pstg);
		}

		// Remember the storage we are given, since we are given one
		lpstg->AddRef();
		_pstg = lpstg;
	}
}

/*
 *	SetAllowedResizeDirections
 *	
 *	@func Resizing helper function
 *
 */
static void SetAllowedResizeDirections(
	const POINT  & pt,
	const RECT   & rc,
	      LPTSTR   lphand,
	      BOOL   & fTop,
	      BOOL   & fBottom,
	      BOOL   & fLeft,
	      BOOL   & fRight
)
{
   	fTop = abs(pt.y - rc.top) < abs(pt.y - rc.bottom);
	fBottom = !fTop;
	fLeft = abs(pt.x - rc.left) < abs(pt.x - rc.right);
	fRight = !fLeft;
	if (lphand == IDC_SIZENS)
	{
		fLeft = fRight = FALSE; 
	}
	else if (lphand == IDC_SIZEWE)
	{
		fTop = fBottom = FALSE;
	}
	return;
}

/*
*	SetRestriction.
 *	
 *	@func Resizing helper function determines bounding rectangle for resizing.
 *
 */
static void SetRestriction(
    RECT  & rc,
	HWND    hwnd,
	DWORD   dwScroll
)
{
	GetClientRect(hwnd, &rc);
	InflateRect(&rc, -1, -1);			// So rectangle is visible

	// allow objects to grow larger than the window in the
	// directions which have scrollbars
	if(dwScroll & WS_HSCROLL)
	{
		rc.right = MAXLONG;
	}
	if(dwScroll & WS_VSCROLL)
	{
		rc.bottom = MAXLONG;
	}
	return;
}

/*
*	Restrict
 *	
 *	@func Resizing helper function bounds a point within a rectangle
 *
 */
static void Restrict(
	POINT  &pt,
	RECT   &rc
)
{
	if (pt.x < rc.left)
	{
		pt.x = rc.left;
	}
	else if (pt.x > rc.right)
	{
		pt.x = rc.right;
	}
	if (pt.y < rc.top)
	{
		pt.y = rc.top;
	}
	else if (pt.y > rc.bottom)
	{
		pt.y = rc.bottom;
	}
	return;
}

/*
 *	COleObject::HandleResize
 *	
 *	@mfunc Deal with object resizing.
 *
 */
BOOL COleObject::HandleResize(const POINT &pt)
{
	LPTSTR lphand;
	DWORD  dwFlags = _pi.dwFlags;
	HDC    hdc;
 	HWND   hwnd;
	RECT   rcOld;
	RECT   rcRestrict;
	BOOL   fTop, fBottom, fLeft, fRight;
	BOOL   fEscape;
	CDisplay *pdp = _ped->_pdp;

	if (!(dwFlags & REO_SELECTED)	||
		!(dwFlags & REO_RESIZABLE)	||
		(lphand = CheckForHandleHit(pt)) == NULL || !pdp)
	{
		return FALSE;
	}
 	
	hdc = pdp->GetDC();
	rcOld = _rcPos;				// Save old size
	_ped->TxGetWindow(&hwnd);
	ASSERT(IsWindow(hwnd));
	SetCapture(hwnd);
	
	SetRestriction( rcRestrict, hwnd, _ped->TxGetScrollBars() );

	SetAllowedResizeDirections(pt, _rcPos, lphand,
		                       fTop, fBottom, fLeft, fRight);
	
	// Erase and redraw frame without handles.
	DrawFrame(pdp, hdc, &_rcPos);
	_pi.dwFlags = REO_NULL;
	DrawFrame(pdp, hdc, &_rcPos);

	fEscape = FALSE;
	const INT vkey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
	while (GetAsyncKeyState(vkey) & 0x8000)
	{		
		POINT ptLast = pt;
		POINT ptCur;
		MSG msg;

		// Stop if the ESC key has been pressed
		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
		{
			fEscape = TRUE;
			break;
		}
		
		GetCursorPos(&ptCur);
		ScreenToClient(hwnd, &ptCur);

// GetCursorPos() isn't supported on WinCE. We have  it hacked to
// be GetMessagePos() which unfortunately in this case will cause
// ptCur to never change. By removing this check we end up drawing
// multiple times when the user pauses during a resize. 
#ifndef UNDER_CE
		// If mouse hasn't moved, try again
		if ((ptCur.x == ptLast.x) && (ptCur.y == ptLast.y))
		{
			continue;
        }
#endif
		ptLast = ptCur;

		Restrict( ptCur, rcRestrict );

		// Erase old rectangle, update rectangle, and redraw
		DrawFrame(pdp, hdc, &_rcPos);	
		if (fLeft)   _rcPos.left   = ptCur.x;
		if (fRight)  _rcPos.right  = ptCur.x;
		if (fTop)    _rcPos.top    = ptCur.y;
		if (fBottom) _rcPos.bottom = ptCur.y;
		// Keep a minimun width and height
		INT xWidthSys = pdp->GetXWidthSys();
		INT yHeightSys = pdp->GetYHeightSys();
		if (_rcPos.right - _rcPos.left < xWidthSys)
		{
			if (fLeft) _rcPos.left = _rcPos.right - xWidthSys;
			if (fRight) _rcPos.right = _rcPos.left + xWidthSys;
		}
		if (_rcPos.bottom - _rcPos.top < yHeightSys)
		{
			if (fTop) _rcPos.top = _rcPos.bottom - yHeightSys;
			if (fBottom) _rcPos.bottom = _rcPos.top + yHeightSys;
		}

		DrawFrame(pdp, hdc, &_rcPos);
		// FUTURE: (joseogl): It would be cool if we could do something
		// bettter here, but for now, it appears to be necessary.
		Sleep(100);
		
		// Eat input messages
		if (PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE) ||
			PeekMessage(&msg, 0, WM_MOUSEFIRST,
			                      WM_MOUSELAST, PM_REMOVE | PM_NOYIELD) ||
			PeekMessage(&msg, 0, WM_NCMOUSEFIRST,
			                      WM_NCMOUSELAST, PM_REMOVE | PM_NOYIELD))
		{
			// Break out of the loop if the Escape key was pressed
		    if ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_ESCAPE))
			{
	        	fEscape = TRUE;
				break;
			}
		}
	}

	DrawFrame(pdp, hdc, &_rcPos);
  	ReleaseCapture();
	RECT rcNew = _rcPos;
	_rcPos = rcOld;
 	_pi.dwFlags = dwFlags;

	// If user aborted, then we skip the resizing stuff
	if (fEscape)
	{
		DrawFrame(pdp, hdc, &_rcPos);
	}
	else
	{
		EnableGuardPosRect();
		Resize( rcNew );
		DrawFrame(pdp, hdc, &_rcPos);
		DisableGuardPosRect();
	}
	pdp->ReleaseDC( hdc );
	return TRUE;
}

/*
 *	COleObject::Resize
 *	
 *	@mfunc Set new object size.  Handle undo details.
 *
 */
void COleObject::Resize(const RECT &rcNew)
{
	CDisplay *	pdp = _ped->_pdp;
	SIZEL		sizelold = _sizel;

	// Change the size of our internal representation.
	_sizel.cx = pdp->DXtoHimetricX(rcNew.right - rcNew.left);
	_sizel.cy = pdp->DXtoHimetricX(rcNew.bottom - rcNew.top);

	//If the size didn't really change, don't do anything else.
	if (_sizel.cx != sizelold.cx || _sizel.cy != sizelold.cy)
	{
		CGenUndoBuilder undobldr(_ped, UB_AUTOCOMMIT);
		IAntiEvent *pae;

		pae = gAEDispenser.CreateResizeObjectAE(_ped, this, _rcPos);

		if( pae )
		{
			undobldr.AddAntiEvent(pae);
		}

		_rcPos.bottom = _rcPos.top + pdp->HimetricXtoDX(_sizel.cy);
		_rcPos.right = _rcPos.left + pdp->HimetricYtoDY(_sizel.cx);

		SetExtent(SE_NOTACTIVATING);

		// Force a redraw that will stretch the object.
		pdp->OnPostReplaceRange(INFINITE, 0, 0, _cp, _cp + 1);

		_ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);
	}
}

/*
 *	COleObject::OnReposition
 *	
 *	@mfunc Set object's new position.  May have changed as a result of scrolling.
 *
 */
void COleObject::OnReposition( LONG dx, LONG dy )
{
	IOleInPlaceObject *pipo;
	RECT rcClip;

	if( !_fInPlaceActive )
	{
		// if we're not inplace active, don't do anything
		return;
	}

	_ped->_pdp->GetViewRect(rcClip);
	_rcPos.left += dx;
	_rcPos.right += dx;
	_rcPos.top += dy;
	_rcPos.bottom += dy;

	if( _punkobj->QueryInterface(IID_IOleInPlaceObject, (void **)&pipo) 
		== NOERROR )
	{
		pipo->SetObjectRects(&_rcPos, &rcClip);
        pipo->Release();
	}
}

 /*
 *	COleObject::ResetPosRect(void)
 *	
 *	@mfunc Recompute the object's position rectangle from its cp.
 *
 */
 void COleObject::ResetPosRect(
	 LONG *pAdjust)		//@parm output adjustment needed for positioning below baseline
{
	CRchTxtPtr rtp(_ped, 0);
	POINT pt, pt1;
	LONG yHeight = _ped->_pdp->HimetricXtoDX(_sizel.cy);
	
	rtp.SetCp(_cp);
	if (_ped->_pdp->PointFromTp(rtp, NULL, FALSE, pt, NULL, TA_TOP) == -1)
		return;
	_rcPos.top = pt.y;
	if (_pi.dwFlags & REO_BELOWBASELINE)
	{
		_ped->_pdp->PointFromTp(rtp, NULL, FALSE, pt1, NULL, TA_BOTTOM);
	}
	else
	{
		_ped->_pdp->PointFromTp(rtp, NULL, FALSE, pt1, NULL, TA_BASELINE);
	}

	if (pAdjust)
	{
		*pAdjust = 0;
	}

	if (pt1.y - pt.y > yHeight)
	{
		// If line is bigger than object move object down.
		_rcPos.top += pt1.y - pt.y - yHeight;
		if (pAdjust)
		{
			*pAdjust = pt1.y - pt.y - yHeight;
		}
	}

	_rcPos.bottom = _rcPos.top + yHeight;
	_rcPos.left = pt.x;
	_rcPos.right = _rcPos.left + _ped->_pdp->HimetricYtoDY(_sizel.cx);
}

#ifdef DEBUG
void COleObject::DbgDump(DWORD id){
	Tracef(TRCSEVNONE, "Object #%d %X: cp = %d , rect = (%d, %d, %d, %d)",id,this,_cp,_rcPos.top,_rcPos.left,_rcPos.bottom,_rcPos.right);
}
#endif

/*	
 *	COleObject:SetExtent()
 *
 *	@mfunc A wrapper around IOleObject::SetExtent which makes some additional
 *			checks if the first call to IOleObject::SetExtent fails.  
 *
 */
HRESULT COleObject::SetExtent(int iActivating) //@parm indicates if the object
												//	is currently being activated
{
	LPOLEOBJECT poo;
	HRESULT hr;

	// if we are connected to a link object, the native extent can't be change,
	// so don't bother doing anything here.
	if( (_pi.dwFlags & REO_LINK) )
	{
		// so we don't call GetExtents on remeasuring.
		_fSetExtent = TRUE;
		return NOERROR;
	}

	if((hr = _punkobj->QueryInterface(IID_IOleObject, (void **)&poo)) != NOERROR)
	{
		return hr;
	}

	// If we are about to activate the object, fall through and OleRun the
	// object prior to attempting to SetExtent.  Otherwise, attempt a SetExtent
	// directly.
        SIZEL sizelsave = _sizel;
	if(iActivating == SE_NOTACTIVATING)
	{
		//By default, we will call SetExtent when the object is next activated.
		_fSetExtent = TRUE;

		hr = poo->SetExtent(_pi.dvaspect, &_sizel);

		DWORD dwStatus;

		// If the server is not running we need to to some additional
		// checking. If it was, we do not need to call SetExtent again.

		//Find out if OLEMISC_RECOMPOSEONRESIZE is set.  If it is, we should
		//run the object and call setextent.  If not, we defer calling set
		//extent until we are ready to activate the object.
		if(!(hr == OLE_E_NOTRUNNING &&
			poo->GetMiscStatus(_pi.dvaspect, &dwStatus) == NOERROR &&
			(dwStatus & OLEMISC_RECOMPOSEONRESIZE)))
		{
			goto DontRunAndSetAgain;
		}
		// fall through and attempt the SetExtent again after running the object
	}

	sizelsave = _sizel;
	pOleRun(_punkobj);		// This call causes _sizel to be reset 
							// via OLE and FetchObjectExtents.
	_sizel = sizelsave;
	poo->SetExtent(_pi.dvaspect, &_sizel);

DontRunAndSetAgain:
	if((hr == NOERROR) || 
		(iActivating == SE_NOTACTIVATING && hr != OLE_E_NOTRUNNING))
	{
		_fSetExtent = FALSE;
	}
	// If the server is still not running, we try again at
	// activation time.  Otherwise the server has either 
	// done it's thing or it doesn't do resize.  Either way
	// we don't bother trying again at activation time.

	if(hr == NOERROR && _fIsPaintBrush)
	{
		SIZEL sizelChk;

		poo->GetExtent(_pi.dvaspect, &sizelChk);
		_fPBUseLocalSizel = !(sizelChk.cx == _sizel.cx && 
								sizelChk.cy == _sizel.cy);
		// HACK:  Calls to SetExtent on PaintBrush objects may not
		// 	actually change the object extents.  In such cases, 
		//	we will rely on local _sizel for PaintBrush object extents.
	}

	poo->Release();

	return hr;
}


	

