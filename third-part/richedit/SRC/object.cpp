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
 *	@module	object.cpp	IRichEditOle implementation |
 *
 *	Author: alexgo 8/15/95
 */

#include "_common.h"
#include "_edit.h"
#include "_objmgr.h"
#include "_coleobj.h"
#include "_rtext.h"
#include "_select.h"
#include "_m_undo.h"



// 	IUnknown is implemented elsewhere

/*
 *	CTxtEdit::GetClientSite
 *
 *	@mfunc	returns the client site 
 */
STDMETHODIMP CTxtEdit::GetClientSite(
	LPOLECLIENTSITE FAR * lplpolesite)		//@parm where to return 
											//the client site
{
	COleObject *pobj;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetClientSite");

	if( !lplpolesite )
	{
		return E_INVALIDARG;
	}

	pobj = new COleObject(this);
	// should start with a ref count of 1.
	if( pobj )
	{
		*lplpolesite = (IOleClientSite *)pobj;
		return NOERROR;
	}
	return E_OUTOFMEMORY;
}

/* 
 *	CTxtEdit::GetObjectCount
 *
 *	@mfunc	return the number of objects in this edit instance
 */
STDMETHODIMP_(LONG) CTxtEdit::GetObjectCount()
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetObjectCount");
	
	return _pobjmgr ? _pobjmgr->GetObjectCount() : 0;
}

/*
 *	CTxtEdit::GetLinkCount
 *
 *	@mfunc	return the number of likns in this edit instance
 */
STDMETHODIMP_(LONG) CTxtEdit::GetLinkCount()
{
	CObjectMgr *pobjmgr;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetLinkCount");

	pobjmgr = GetObjectMgr();

	if( pobjmgr )
	{
		return pobjmgr->GetLinkCount();
	}
	else
	{
		return 0;
	}

}

/*
 *	CTxtEdit::GetObject
 *
 *	@mfunc	returns an object structure for the indicated object
 */
STDMETHODIMP CTxtEdit::GetObject( 
	LONG iob, 					//@parm index of the object
	REOBJECT * preobj,			//@parm where to put object info
	DWORD dwFlags)				//@parm flags
{
	COleObject *pobj = NULL;
	CObjectMgr *pobjmgr;
	CTxtSelection *psel;
	CCallMgr callmgr(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetObject");
	if( !preobj || preobj->cbStruct != sizeof(REOBJECT) )
	{
		return E_INVALIDARG;
	}
	pobjmgr = GetObjectMgr();

	if( !pobjmgr )
	{
		return E_OUTOFMEMORY;
	}


	//
	// there are three cases of intestest; get the object at
	// an index, at a given cp, or at the selection.
	//
	if( iob == REO_IOB_USE_CP )
	{
		pobj = pobjmgr->GetObjectFromCp(preobj->cp);
	}
	else if( iob == REO_IOB_SELECTION )
	{
		// use the selection cp
		psel = GetSel();

		if( psel )
		{
			pobj = pobjmgr->GetObjectFromCp(psel->GetCpMin());
		}
	}
	else
	{
		pobj = pobjmgr->GetObjectFromIndex(iob);
	}

	if( pobj )
	{
		return pobj->GetObjectData(preobj, dwFlags);
	}
	//
	// This return code is a bit of stretch, but basially 
	return E_INVALIDARG;
}

/*
 *	CTxtEdit::InsertObject
 *
 *	@mfunc	inserts a new object
 */
STDMETHODIMP CTxtEdit::InsertObject( 
	REOBJECT * preobj)		//@parm object info
{
	CTxtSelection *psel;
	CObjectMgr *pobjmgr;
	COleObject *pobj = NULL;
	DWORD	cp;
	CCallMgr		callmgr(this);
	CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT);
	CRchTxtPtr	rtp(this, 0);
	WCHAR 	ch = WCH_EMBEDDING;
	HRESULT hr;
	CNotifyMgr *		pnm;					// For notifying of changes
	LONG	iFormat = -1;


	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::InsertObject");

	// do some boundary case checking

	if( !preobj )
	{
		return E_INVALIDARG;
	}

	psel = GetSel();

	if( !psel )
	{
		return E_OUTOFMEMORY;
	}

	// if the insertion of this character would cause
	// us to exceed the text limit, fail
	if( GetAdjustedTextLength() + 1 > TxGetMaxLength() )
	{

		// if we're not replacing a selection (or the
		// selection is degenerate, then we will have  exceeded
		// our limit
		if( preobj->cp != REO_CP_SELECTION || psel->GetCch() == 0)
		{
			GetCallMgr()->SetMaxText();
			return E_OUTOFMEMORY;
		}
	}
	
	pobjmgr = GetObjectMgr();
	
	if( pobjmgr )
	{
		LONG	selectRange = 0;

		undobldr.StopGroupTyping();

		DWORD cpFormat;

		if( preobj->cp == REO_CP_SELECTION )
		{
			LONG	a, b;

			selectRange = psel->GetRange(a, b);
			cp = psel->GetCpMin();

			// Get the cp of the active end of the selection from which we
			// 	will obtain the CF for the object.
			cpFormat = psel->GetCch() > 0 ? psel->GetCpMost() : cp;

			HandleSelectionAEInfo(this, &undobldr, psel->GetCp(), psel->GetCch(), 
					cp + 1, 0, SELAE_FORCEREPLACE);
		}
		else
		{
			cpFormat = cp = preobj->cp;
		}
		
		// Get the format for the ReplaceRange:  for cp semantics, use format
		//	at the cp; for selection semantics, use the format at the active
		//	end of the selection.
		CTxtRange rgFormat(this, cpFormat, 0);
		iFormat = rgFormat.Get_iCF();

		rtp.SetCp(cp);
		
		rtp.ReplaceRange(selectRange, 1, &ch, &undobldr, iFormat);  
		//Don't want object selected.
		psel->SetSelection(cp+1, cp+1);

		hr = pobjmgr->InsertObject(cp, preobj, &undobldr);

		pobj = (COleObject *)(preobj->polesite);

		pobj->EnableGuardPosRect();

		if (pnm = GetNotifyMgr())			// Get the notification mgr
		{
			pnm->NotifyPostReplaceRange(NULL, 	// Notify interested parties
				INFINITE, 0, 0, cp, cp + 1);	// of the change.
		}

		pobj->DisableGuardPosRect();

		ReleaseFormats(iFormat, -1);

		TxUpdateWindow();

		return hr;
	}
	
	return E_OUTOFMEMORY;		
}

/*
 *	CTxtEdit::ConvertObject
 *
 *	@mfunc	Converts the specified object to the specified class.  Does reload
 *		the object but does NOT force an update (caller must do this).
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::ConvertObject( 
	LONG iob, 					//@parm index of the object
	REFCLSID rclsidNew,			//@parm the destination clsid
	LPCSTR lpstrUserTypeNew)	//@parm the new user type name
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ConvertObject");
	CCallMgr callmgr(this);

	COleObject * pobj = NULL;
	HRESULT hr;

	pobj = ObjectFromIOB(iob);

	// If iob was invalid return
	if (!pobj)
	{
		return E_INVALIDARG;
	}

	//Delegate to the object.
	hr = pobj->Convert(rclsidNew, lpstrUserTypeNew);

	return hr;
}

/*
 *	CTxtEdit::ActivateAs
 *
 *	@mfunc	Handles a request by the user to activate all objects of a particular
 *		class as objects of another class.
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::ActivateAs( 
	REFCLSID rclsid, 			//@parm clsid which we're going to change
	REFCLSID rclsidAs)			//@parm clsid to activate as
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ActivateAs");
	CCallMgr callmgr(this);

	CObjectMgr * pobjmgr = NULL;

	pobjmgr = GetObjectMgr();

	if( !pobjmgr )
	{
		return E_OUTOFMEMORY;
	}

	return pobjmgr->ActivateObjectsAs(rclsid, rclsidAs);
}

/* 
 *	CTxtEdit::SetHostNames
 *
 *	@mfunc	Sets the host names for this instance
 */
STDMETHODIMP CTxtEdit::SetHostNames( 
	LPCSTR lpstrContainerApp, 	//@parm App name
	LPCSTR lpstrContainerDoc)	//@parm	Container Object (doc) name
{
	CObjectMgr *pobjmgr;
	CCallMgr callmgr(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SetHostNames");
	
 	WCHAR *pwsContainerApp = W32->ConvertToWideChar( lpstrContainerApp );
	WCHAR *pwsContainerDoc = W32->ConvertToWideChar( lpstrContainerDoc );
	pobjmgr = GetObjectMgr();

	if( pobjmgr && pwsContainerApp && pwsContainerDoc)
	{
		HRESULT hr = pobjmgr->SetHostNames(pwsContainerApp, pwsContainerDoc);
		delete pwsContainerApp;
		delete pwsContainerDoc;
		return hr;
	}
	return E_OUTOFMEMORY;
}

/*
 *	CTxtEdit::SetLinkAvailable
 *
 *	@mfunc
 *		Allows client to tell us whether the link is available or not.
 */
STDMETHODIMP CTxtEdit::SetLinkAvailable( 
	LONG iob, 					//@parm index of the object
	BOOL fAvailable)			//@parm if TRUE, make object linkable
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SetLinkAvailable");

	COleObject * pobj = ObjectFromIOB(iob);

	// If iob was invalid, return
	if (!pobj)
	{
		return E_INVALIDARG;
	}

	// Delegate this to the object.
	return pobj->SetLinkAvailable(fAvailable);
}

/*
 *	CTxtEdit::SetDvaspect
 *
 *	@mfunc	Allows client to tell us which aspect to use and force us
 *		to recompute positioning and redraw.
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::SetDvaspect( 
	LONG iob, 					//@parm index of the object
	DWORD dvaspect)				//@parm	the aspect to use
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SetDvaspect");
	CCallMgr callmgr(this);
	COleObject * pobj = ObjectFromIOB(iob);

	// If iob was invalid, return
	if (!pobj)
	{
		return E_INVALIDARG;
	}

	// Delegate this to the object.
	pobj->SetDvaspect(dvaspect);

	return NOERROR;
}

/*
 *	CTxtEdit::HandsOffStorage
 *
 *	@mfunc	see IPersistStorage::HandsOffStorage
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::HandsOffStorage( 
	LONG iob)					//@parm index of the object
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::HandsOffStorage");
	CCallMgr callmgr(this);

	COleObject * pobj = ObjectFromIOB(iob);

	// If iob was invalid, return
	if (!pobj)
	{
		return E_INVALIDARG;
	}

	// Delegate this to the object.
	pobj->HandsOffStorage();

	return NOERROR;
}

/*
 *	CTxtEdit::SaveCompleted
 *
 *	@mfunc	see IPersistStorage::SaveCompleted
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::SaveCompleted( 
	LONG iob, 					//@parm index of the object
	LPSTORAGE lpstg)			//@parm new storage
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::SaveCompleted");
	CCallMgr callmgr(this);

	COleObject * pobj = ObjectFromIOB(iob);

	// If iob was invalid, return
	if (!pobj)
	{
		return E_INVALIDARG;
	}

	// Delegate this to the object.
	pobj->SaveCompleted(lpstg);

	return NOERROR;
}

/*
 *	CTxtEdit::InPlaceDeactivate
 *
 *	@mfunc	Deactivate 
 */
STDMETHODIMP CTxtEdit::InPlaceDeactivate()
{
	COleObject *pobj;
	CObjectMgr *pobjmgr;
	HRESULT hr = NOERROR;
	CCallMgr callmgr(this);

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::InPlaceDeactivate");
	
	pobjmgr = GetObjectMgr();

	if( pobjmgr )
	{
		pobj = pobjmgr->GetInPlaceActiveObject();

		if( pobj )
		{
			hr = pobj->DeActivateObj();
		}
	}

	return hr;
}

/*
 *	CTxtEdit::ContextSensitiveHelp
 *
 *	@mfunc enter/leave ContextSensitiveHelp mode
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::ContextSensitiveHelp( 
	BOOL fEnterMode)			//@parm enter/exit mode
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ContextSensitiveHelp");

	COleObject * pobj = NULL;
	CObjectMgr * pobjmgr = NULL;
	HRESULT hr = NOERROR;
	IOleWindow * pow;
	CCallMgr callmgr(this);

	pobjmgr = GetObjectMgr();

	if( !pobjmgr )
	{
		return E_OUTOFMEMORY;
	}

	// if the mode changes
	if( pobjmgr->GetHelpMode() != fEnterMode )
	{
		pobjmgr->SetHelpMode(fEnterMode);

		pobj = pobjmgr->GetInPlaceActiveObject();

		if (pobj)
		{
			hr = pobj->GetIUnknown()->QueryInterface(IID_IOleWindow,
				(void **)&pow);

			if( hr == NOERROR )
			{
				hr = pow->ContextSensitiveHelp(fEnterMode);
				pow->Release();
			}
		}
	}

	return hr;
}

/*
 *	CTxtEdit::GetClipboardData
 *
 *	@mfunc	return an data transfer object for the indicated
 *	range
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::GetClipboardData( 
	CHARRANGE *lpchrg, 			//@parm the range of text to use
	DWORD reco,					//@parm operation the data is for
	LPDATAOBJECT *lplpdataobj)	//@parm where to put the data object
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::GetClipboardData");

	CCallMgr callmgr(this);
	HRESULT hr;
	LONG cpMin, cpMost;
	CLightDTEngine * pldte = GetDTE();

	//Make sure cpMin and cpMost are within the current text limits.
	//Interpret neg. value for cpMin as the beginning of the text,
	//and neg. value for cpMax as the end of the text.  If a char range
	//is not given use the current selection.
	if(lpchrg)
	{
		LONG cchText;

		cchText = (LONG)GetTextLength();

		cpMin = max(0, lpchrg->cpMin);
		cpMin = min(cchText, lpchrg->cpMin);
		cpMost = lpchrg->cpMost;
		if(lpchrg->cpMost < 0 || lpchrg->cpMost > cchText)
			cpMost = cchText;
	}
	else
	{
		CTxtSelection * psel = GetSel();
		if(!psel)
			{
			return E_OUTOFMEMORY;
			}
		psel->GetRange(cpMin, cpMost);
	}

	//Make sure this is a valid range.
	if(cpMin >= cpMost)
	{
		*lplpdataobj = NULL;
		return cpMin == cpMost
					? NOERROR
					: ResultFromScode(E_INVALIDARG);
	}

	CTxtRange rg(this, cpMin, cpMin-cpMost);

	//We don't use reco for anything.
	hr = pldte->RangeToDataObject(&rg, SF_RTF, lplpdataobj);

#ifdef DEBUG
	if(hr != NOERROR)
		TRACEERRSZSC("GetClipboardData", E_OUTOFMEMORY);
#endif

	return hr;
}

/*
 *	CTxtEdit::ImportDataObject
 *
 *	@mfunc	morally equivalent to paste, but with a data object
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
STDMETHODIMP CTxtEdit::ImportDataObject( 
	LPDATAOBJECT lpdataobj,		//@parm the data object to use
	CLIPFORMAT cf, 				//@parm the clibpoard format to use
	HGLOBAL hMetaPict)			//@parm the metafile to use
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CTxtEdit::ImportDataObject");
	CCallMgr callmgr(this);
	CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT);

	REPASTESPECIAL rps;

	rps.dwAspect = DVASPECT_CONTENT;
	rps.dwParam = NULL;

	if (hMetaPict)
	{
		rps.dwAspect = DVASPECT_ICON;
		rps.dwParam = (DWORD) (LPVOID) hMetaPict;
	}

	return PasteDataObjectToRange(lpdataobj, GetSel(), cf,
		&rps, &undobldr, PDOR_NOQUERY);
}

/*
 *	CTxtEdit::ObjectFromIOB
 *
 *	@mfunc	Gets an object based on an IOB type index.
 *
 *	@rdesc:
 *		pointer to COleObject or NULL if none.
 */
COleObject * CTxtEdit::ObjectFromIOB(LONG iob)
{
	COleObject * pobj = NULL;
	CObjectMgr * pobjmgr = NULL;

	pobjmgr = GetObjectMgr();

	if (!pobjmgr)
	{
		return NULL;
	}

	// Figure out the index of the selection
	if (iob == REO_IOB_SELECTION)
	{
		CTxtSelection * psel = GetSel();
		if(!psel)
			{
			return NULL;
			}
		pobj = pobjmgr->GetFirstObjectInRange(psel->GetCpMin(),
			psel->GetCpMost());
	}
	else
	{
		// Make sure the IOB is in range
		if ((0 <= iob) && (iob < GetObjectCount()))
		{
			pobj = pobjmgr->GetObjectFromIndex(iob);
		}
	}
	return pobj;
}


