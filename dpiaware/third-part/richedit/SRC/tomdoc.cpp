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
 *	@doc TOM
 *
 *	@module tomdoc.cpp - Implement the ITextDocument interface on CTxtEdit |
 *	
 *		This module contains the implementation of the TOM ITextDocument
 *		class as well as the global TOM type-info routines
 *
 *	History: <nl>
 *		sep-95	MurrayS: stubs and auto-doc created <nl>
 *		nov-95	MurrayS: upgrade to top-level TOM interface
 *		dec-95	MurrayS: implemented file I/O methods
 *
 *	@future
 *		1. Begin/EndEditCollection
 *		2. Freeze/Unfreeze
 *
 */

#include "_common.h"
#include "_range.h"
#include "_edit.h"
#include "_disp.h"
#include "_rtfconv.h"

ASSERTDATA

// TOM Type Info HRESULT and pointers
HRESULT		g_hrGetTypeInfo = NOERROR;
ITypeInfo *	g_pTypeInfoDoc;
ITypeInfo *	g_pTypeInfoSel;
ITypeInfo *	g_pTypeInfoFont;
ITypeInfo *	g_pTypeInfoPara;
ITypeLib  *	g_pTypeLib;


EXTERN_C const GUID LIBID_TOM = 
{ 0x8CC497C9, 0xA1DF, 0x11ce, { 0x80, 0x98, 0x0, 0xaa, 0x0, 0x47, 0xBE, 0x5D } };


//------------------------ Global TOM Type Info Methods -----------------------------

/*
 *	GetTypeInfoPtrs()
 *
 *	@func
 *		Ensure that global TOM ITypeInfo ptrs are valid (else g_pTypeInfoDoc
 *		is NULL).  Return NOERROR immediately if g_pTypeInfoDoc is not NULL,
 *		i.e., type info ptrs are already valid.
 *
 *	@rdesc
 *		HRESULT = (success) ? NOERROR
 *				: (HRESULT from LoadTypeLib or GetTypeInfoOfGuid)
 *
 *	@comm
 *		This routine should be called by any routine that uses the global
 *		type info ptrs, e.g., IDispatch::GetTypeInfo(), GetIDsOfNames, and
 *		Invoke.  That way if noone is using the type library info, it doesn't
 *		have to be loaded.
 *
 */
HRESULT GetTypeInfoPtrs()
{
	HRESULT	hr;
	CLock	lock;							// Only one thread at a time...

	if(g_pTypeInfoDoc)						// Type info ptrs already valid
		return NOERROR;

	if(g_hrGetTypeInfo != NOERROR)			// Tried to get before and failed
		return g_hrGetTypeInfo;

	if (pLoadRegTypeLib(LIBID_TOM, 1, 0, LANG_NEUTRAL, &g_pTypeLib) != NOERROR)
	{
		hr = pLoadTypeLib(OLESTR("RICHED20.DLL"), &g_pTypeLib);
		if(hr != NOERROR)
			goto err;
	}

	// Get ITypeInfo pointers with g_pTypeInfoDoc last
	hr = g_pTypeLib->GetTypeInfoOfGuid(IID_ITextSelection, &g_pTypeInfoSel);
	if(hr == NOERROR)
	{
	    g_pTypeLib->GetTypeInfoOfGuid(IID_ITextFont,	 &g_pTypeInfoFont);
		g_pTypeLib->GetTypeInfoOfGuid(IID_ITextPara,	 &g_pTypeInfoPara);
		g_pTypeLib->GetTypeInfoOfGuid(IID_ITextDocument, &g_pTypeInfoDoc);

		if(g_pTypeInfoFont && g_pTypeInfoPara && g_pTypeInfoDoc)
			return NOERROR;					// Got 'em all
	}
	hr = E_FAIL;

err:
	Assert("Error getting type info pointers");

	g_pTypeInfoDoc	= NULL;					// Type info ptrs not valid
	g_hrGetTypeInfo	= hr;					// Save HRESULT in case called
	return hr;								//  again
}

/*
 *	ReleaseTypeInfoPtrs()
 *
 *	@func
 *		Release TOM type info ptrs in case they have been defined.
 *		Called when RichEdit dll is being unloaded.
 */
void ReleaseTypeInfoPtrs()
{
	if(g_pTypeInfoDoc)
	{
		g_pTypeInfoDoc->Release();
		g_pTypeInfoSel->Release();
		g_pTypeInfoFont->Release();
		g_pTypeInfoPara->Release();
	}
	if(g_pTypeLib)
		g_pTypeLib->Release();
}

/*
 *	GetTypeInfo(iTypeInfo, &pTypeInfo, ppTypeInfo)
 *
 *	@func
 *		IDispatch helper function to check parameter validity and set
 *		*ppTypeInfo = pTypeInfo if OK
 *
 *	@rdesc
 *		HRESULT
 */
HRESULT GetTypeInfo(
	UINT		iTypeInfo,		//@parm Index of type info to return
	ITypeInfo *&pTypeInfo,		//@parm Address of desired type info ptr
	ITypeInfo **ppTypeInfo)		//@parm Out parm to receive type info
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetTypeInfo");

	if(!ppTypeInfo)
		return E_INVALIDARG;

	*ppTypeInfo = NULL;

	if(iTypeInfo > 1)
		return DISP_E_BADINDEX;

	HRESULT hr = GetTypeInfoPtrs();				// Ensure TypeInfo ptrs are OK
	if(hr == NOERROR)
	{
		*ppTypeInfo = pTypeInfo;				// Have to use reference in
		pTypeInfo->AddRef();					//  case defined in this call
	}
	return hr;
}

/*
 *	MyRead(dwCookie, pbBuffer, cb, pcb)
 *
 *	@func
 *		Callback function for converting a file into an editstream for
 *		input.
 *
 *	@rdesc
 *		(DWORD)HRESULT
 */
DWORD CALLBACK MyRead(DWORD dwCookie, BYTE *pbBuffer, long cb, long *pcb)
{
	if(!dwCookie)								// No handle defined
		return (DWORD)E_FAIL;

	Assert(pcb);
	*pcb = 0;

	if(!ReadFile((HANDLE)dwCookie, (void *)pbBuffer, (DWORD)cb,
					(DWORD *)pcb, NULL))
		return HRESULT_FROM_WIN32(GetLastError());

	return (DWORD)NOERROR;
}

/*
 *	MyWrite(dwCookie, pbBuffer, cb, pcb)
 *
 *	@func
 *		Callback function for converting a file into an editstream for
 *		output.
 *
 *	@rdesc
 *		(DWORD)HRESULT
 */
DWORD CALLBACK MyWrite(DWORD dwCookie, BYTE *pbBuffer, long cb, long *pcb)
{
	if(!dwCookie)								// No handle defined
		return (DWORD)E_FAIL;

	Assert(pcb);
	*pcb = 0;

	if(!WriteFile((HANDLE)dwCookie, (void *)pbBuffer, (DWORD)cb,
					(DWORD *)pcb, NULL))
		return HRESULT_FROM_WIN32(GetLastError());

	return (DWORD)(*pcb ? NOERROR : E_FAIL);
}


//-----------------CTxtEdit IUnknown methods: see textserv.cpp -----------------------------


//------------------------ CTxtEdit IDispatch methods -------------------------

/*
 *	CTxtEdit::GetTypeInfoCount(pcTypeInfo)
 *
 *	@mfunc
 *		Get the number of TYPEINFO elements (1)
 *
 *	@rdesc
 *		HRESULT = (pcTypeInfo) ? NOERROR : E_INVALIDARG;
 */
STDMETHODIMP CTxtEdit::GetTypeInfoCount(
	UINT *pcTypeInfo)	//@parm Out parm to receive count
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetTypeInfoCount");

	if(!pcTypeInfo)
		return E_INVALIDARG;

	*pcTypeInfo = 1;
	return NOERROR;
}

/*
 *	CTxtEdit::GetTypeInfo(iTypeInfo, lcid, ppTypeInfo)
 *
 *	@mfunc
 *		Return ptr to type information object for ITextDocument interface
 *
 *	@rdesc
 *		HRESULT
 */
STDMETHODIMP CTxtEdit::GetTypeInfo(
	UINT		iTypeInfo,		//@parm Index of type info to return
	LCID		lcid,			//@parm Local ID of type info
	ITypeInfo **ppTypeInfo)		//@parm Out parm to receive type info
 {
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetTypeInfo");

	return ::GetTypeInfo(iTypeInfo, g_pTypeInfoDoc, ppTypeInfo);
}

/*
 *	CTxtEdit::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid)
 *
 *	@mfunc
 *		Get DISPIDs for all TOM methods and properties
 *
 *	@rdesc
 *		HRESULT
 *
 *	@devnote
 *		This routine tries to find DISPIDs using the type information for
 *		ITextDocument. If that fails, it asks the selection to find the
 *		DISPIDs.
 */
STDMETHODIMP CTxtEdit::GetIDsOfNames(
	REFIID		riid,			//@parm Interface ID to interpret names for
	OLECHAR **	rgszNames,		//@parm Array of names to be mapped
	UINT		cNames,			//@parm Count of names to be mapped
	LCID		lcid,			//@parm Local ID to use for interpretation
	DISPID *	rgdispid)		//@parm Out parm to receive name mappings
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetIDsOfNames");

	HRESULT hr = GetTypeInfoPtrs();				// Ensure TypeInfo ptrs are OK
	if(hr != NOERROR)
		return hr;
		
	hr = g_pTypeInfoDoc->GetIDsOfNames(rgszNames, cNames, rgdispid);

	if(hr == NOERROR)							// Succeeded in finding an
		return NOERROR;							//  ITextDocument method

	IDispatch *pSel = (IDispatch *)GetSel();	// See if the selection knows
												//  the desired method
	if(!pSel)
		return hr;								// No selection

	return pSel->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

/*
 *	CTxtEdit::Invoke(dispidMember, riid, lcid, wFlags, pdispparams,
 *					  pvarResult, pexcepinfo, puArgError)
 *	@mfunc
 *		Invoke members for all TOM DISPIDs, i.e., for ITextDocument,
 *		ITextSelection, ITextRange, ITextFont, and ITextPara.  TOM DISPIDs
 *		for all but ITextDocument are delegated to the selection object.
 *
 *	@rdesc
 *		HRESULT
 *
 *	@devnote
 *		This routine trys to invoke ITextDocument members if the DISPID is
 *		in the range 0 thru 0xff.  It trys to invoke ITextSelection members if
 *		the DISPID is in the range 0x100 thru 0x4ff (this includes
 *		ITextSelection, ITextRange, ITextFont, and ITextPara).  It returns
 *		E_MEMBERNOTFOUND for DISPIDs outside these ranges.
 */
STDMETHODIMP CTxtEdit::Invoke(
	DISPID		dispidMember,	//@parm Identifies member function
	REFIID		riid,			//@parm Pointer to interface ID
	LCID		lcid,			//@parm Locale ID for interpretation
	USHORT		wFlags,			//@parm Flags describing context of call
	DISPPARAMS *pdispparams,	//@parm Ptr to method arguments
	VARIANT *	pvarResult,		//@parm Out parm for result (if not NULL)
	EXCEPINFO * pexcepinfo,		//@parm Out parm for exception info
	UINT *		puArgError)		//@parm Out parm for error
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Invoke");

	HRESULT hr = GetTypeInfoPtrs();				// Ensure TypeInfo ptrs are OK
	if(hr != NOERROR)
		return hr;
		
	if((DWORD)dispidMember < 0x100)				// ITextDocment method
		return g_pTypeInfoDoc->Invoke((IDispatch *)this, dispidMember, wFlags,
							 pdispparams, pvarResult, pexcepinfo, puArgError);

	IDispatch *pSel = (IDispatch *)GetSel();	// See if the selection has
												//  the desired method
	if(pSel && (DWORD)dispidMember <= 0x4ff)
		return pSel->Invoke(dispidMember, riid, lcid, wFlags,
							 pdispparams, pvarResult, pexcepinfo, puArgError);

	return DISP_E_MEMBERNOTFOUND;
}


//--------------------- ITextDocument Methods/Properties -----------------------

/*
 *	ITextDocument::BeginEditCollection()
 *
 *	@mfunc
 *		Method that turns on undo grouping
 *
 *	@rdesc
 *		HRESULT = (undo enabled) ? NOERROR : S_FALSE
 */
STDMETHODIMP CTxtEdit::BeginEditCollection ()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::BeginEditCollection");

	return E_NOTIMPL;
}

/*
 *	ITextDocument::EndEditCollection() 
 *
 *	@mfunc
 *		Method that turns off undo grouping
 *
 *	@rdesc
 *		HRESULT = NOERROR
 */
STDMETHODIMP CTxtEdit::EndEditCollection () 
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::EndEditCollection");

	return E_NOTIMPL;
}

/*
 *	ITextDocument::Freeze(long *pValue) 
 *
 *	@mfunc
 *		Method to increment the freeze count. If this count is nonzero,
 *		screen updating is disabled.  This allows a sequence of editing
 *		operations to be performed without the performance loss and
 *		flicker of screen updating.  See Unfreeze() to decrement the
 *		freeze count.
 *
 *	@rdesc
 *		HRESULT = (screen updating disabled) ? NOERROR : S_FALSE
 *
 *	@devnote
 *		The ifdef'd code works in principle (need to add _cFreeze to
 *		_edit.h), but we don't enable it pending dealing with APIs like
 *		EM_LINEFROMCHAR that don't yet know how to react to a frozen display.
 */
STDMETHODIMP CTxtEdit::Freeze (
	long *pCount)		//@parm Out parm to receive updated freeze count
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Freeze");

#ifdef FUTURE
	if(_pdp)
	{
		_pdp->Freeze();
		if(_pdp->IsFrozen())
			_cFreeze++;
		else
			_cFreeze = 0;
	}

	if(pCount)
		*pCount = _cFreeze;

	return _cFreeze ? NOERROR : S_FALSE;

#endif
	return E_NOTIMPL;
}

/*
 *	ITextDocument::GetDefaultTabStop (pValue) 
 *
 *	@mfunc
 *		Property get method that gets the default tab stop to be
 *		used whenever the explicit tabs don't extend far enough.
 *
 *	@rdesc
 *		HRESULT = (!pValue) ? E_INVALIDARG : NOERROR
 */
STDMETHODIMP CTxtEdit::GetDefaultTabStop (
	float *	pValue)		//@parm Out parm to receive default tab stop
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDefaultTabStop");

	if(!pValue)
		return E_INVALIDARG;
                                                                        
	const LONG lTab = GetDefaultTab();

	*pValue = TWIPS_TO_FPPTS(lTab);

	return NOERROR;
}

/*
 *	CTxtEdit::GetName (pName)
 *
 *	@mfunc
 *		Retrieve ITextDocument filename
 *
 *	@rdesc
 *		HRESULT = (!<p pName>) ? E_INVALIDARG :
 *				  (no name) ? S_FALSE :
 *				  (if not enough RAM) ? E_OUTOFMEMORY : NOERROR
 */
STDMETHODIMP CTxtEdit::GetName (
	BSTR * pName)		//@parm Out parm to receive filename
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetName");

	if(!pName)
		return E_INVALIDARG;

	*pName = NULL;
	if(!_pDocInfo || !_pDocInfo->pName)
		return S_FALSE;

	*pName = pSysAllocString(_pDocInfo->pName);
	
	return *pName ? NOERROR : E_OUTOFMEMORY;
}

/*
 *	ITextDocument::GetSaved (pValue) 
 *
 *	@mfunc
 *		Property get method that gets whether this instance has been
 *		saved, i.e., no changes since last save
 *
 *	@rdesc
 *		HRESULT = (!pValue) ? E_INVALIDARG : NOERROR
 *
 *	@comm
 *		Next time to aid C/C++ clients, we ought to make pValue optional
 *		and return S_FALSE if doc isn't saved, i.e., like our other
 *		boolean properties (see, e.g., ITextRange::IsEqual())
 */
STDMETHODIMP CTxtEdit::GetSaved (
	long *	pValue)		//@parm Out parm to receive Saved property
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetSaved");

	if(!pValue)
		return E_INVALIDARG;

	*pValue = _fSaved ? tomTrue : tomFalse;
	return NOERROR;
}

/*
 *	ITextDocument::GetSelection (ITextSelection **ppSel) 
 *
 *	@mfunc
 *		Property get method that gets the active selection. 
 *
 *	@rdesc
 *		HRESULT = (!ppSel) ? E_INVALIDARG :
 *				  (if active selection exists) ? NOERROR : S_FALSE
 */
STDMETHODIMP CTxtEdit::GetSelection (
	ITextSelection **ppSel)	//@parm Out parm to receive selection pointer
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetSelection");

	if(!ppSel)
		return E_INVALIDARG;

	CTxtSelection *psel = GetSel();

	*ppSel = (ITextSelection *)psel;

	if( psel )
	{
		(*ppSel)->AddRef();
		return NOERROR;
	}

	return S_FALSE;
}

/*
 *	CTxtEdit::GetStoryCount(pCount)
 *
 *	@mfunc
 *		Get count of stories in this document.
 *
 *	@rdesc
 *		HRESULT = (!<p pCount>) ? E_INVALIDARG : NOERROR
 */
STDMETHODIMP CTxtEdit::GetStoryCount (
	LONG *pCount)		//@parm Out parm to receive count of stories
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetStoryCount");

	if(!pCount)
		return E_INVALIDARG;

	*pCount = 1;
	return NOERROR;
}

/*
 *	ITextDocument::GetStoryRanges(ITextStoryRanges **ppStories) 
 *
 *	@mfunc
 *		Property get method that gets the story collection object
 *		used to enumerate the stories in a document.  Only invoke this
 *		method if GetStoryCount() returns a value greater than one.
 *
 *	@rdesc
 *		HRESULT = (if Stories collection exists) ? NOERROR : E_NOTIMPL
 */
STDMETHODIMP CTxtEdit::GetStoryRanges (
	ITextStoryRanges **ppStories) 	//@parm Out parm to receive stories ptr
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetStoryRanges");

	return E_NOTIMPL;
}

/*
 *	ITextDocument::New() 
 *
 *	@mfunc
 *		Method that closes the current document and opens a document
 *		with a default name.  If changes have been made in the current
 *		document since the last save and document file information exists,
 *		the current document is saved.
 *
 *	@rdesc
 *		HRESULT = NOERROR
 */
STDMETHODIMP CTxtEdit::New ()
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::New");

	CloseFile(TRUE);	 					// Save and close file
	return SetText(NULL, IGNORE_PROTECTION);
}

/*
 *	ITextDocument::Open(pVar, Flags, CodePage)
 *
 *	@mfunc
 *		Method that opens the document specified by pVar.
 *
 *	@rdesc
 *		HRESULT = (if success) ? NOERROR : E_OUTOFMEMORY
 *
 *	@future
 *		Handle IStream
 */
STDMETHODIMP CTxtEdit::Open (
	VARIANT *	pVar,		//@parm Filename or IStream
	long		Flags,		//@parm Read/write, create, and share flags
	long		CodePage)	//@parm Code page to use
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Open");

	LONG		cb;								// Byte count for RTF check
	EDITSTREAM	es		= {0, NOERROR, MyRead};
	BOOL		fReplaceSel = Flags & tomPasteFile;
	HCURSOR		hcur;
	LRESULT		lres;
	TCHAR		szType[cchRTFSig + 1];

	if(!pVar || CodePage && CodePage != 1200 && !IsValidCodePage(CodePage))
		return E_INVALIDARG;					// IsValidCodePage(0) fails
												//  even tho CP_ACP = 0 (!)
	if((Flags & 0xf) >= tomHTML)				// RichEdit only handles auto,
		return E_NOTIMPL;						//  plain text, & RTF formats

	if(!fReplaceSel)							// If not replacing selection,
		New();									//  save current file and
												//  delete current text
	CDocInfo * pDocInfo = GetDocInfo();
	if(!pDocInfo)
		return E_OUTOFMEMORY;

	pDocInfo->wCpg	 = (WORD)CodePage;			// Save new code page and
	pDocInfo->wFlags = (WORD)Flags & ~0xf0;		//  flags (w/o creation)

	// Process access, share, and create flags
	DWORD dwAccess = (Flags & tomReadOnly)
		? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);

	DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if(Flags & tomShareDenyRead)
		dwShare &= ~FILE_SHARE_READ;

	if(Flags & tomShareDenyWrite)
		dwShare &= ~FILE_SHARE_WRITE;

	DWORD dwCreate = (Flags >> 4) & 0xf;
	if(!dwCreate)								// Flags nibble 2 must contain
		dwCreate = OPEN_ALWAYS;					//  CreateFile's dwCreate

	if(pVar->vt == VT_BSTR && pSysStringLen(pVar->bstrVal))
	{
		es.dwCookie = (DWORD)CreateFile(pVar->bstrVal, dwAccess, dwShare,
							 NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL);
		if((HANDLE)es.dwCookie == INVALID_HANDLE_VALUE)
			return HRESULT_FROM_WIN32(GetLastError());

		if(!fReplaceSel)						// If not replacing selection,
		{										//  allocate new pName
			pDocInfo->pName = pSysAllocString(pVar->bstrVal);
			pDocInfo->hFile = (HANDLE)es.dwCookie;
			pDocInfo->wFlags |= tomTruncateExisting;	// Setup Saves
		}
	}
	else
	{
		// FUTURE: check for IStream; if not, then fail
		return E_INVALIDARG;
	}

	Flags &= 0xf;								// Isolate conversion flags

	// Get first few bytes of file to check for RTF and Unicode BOM
	(*es.pfnCallback)(es.dwCookie, (LPBYTE)szType, cbRTFSig, &cb);

	if(!Flags || Flags == tomRTF)				// Auto or RTF file
	{											// See if it's really RTF
		Flags = tomRTF;							// Suppose it is
		if(cb != cbRTFSig || (memcmp(szRTFSig, szType, cbRTFSig) != 0))
			Flags = tomText;					// Not RTF, so use tomText
	}

	LONG j = 0;									// Default rewind to 0
	if (Flags == tomRTF)						// RTF
		Flags = SF_RTF;							// Setup EM_STREAMIN for RTF
	else
	{											// If it starts with
		if(cb > 1 && *(WORD *)szType == BOM)	//  Unicode byte-order mark
		{										//  (BOM) file is Unicode, so
			pDocInfo->wCpg = 1200;				//  use Unicode code page and
			j = 2;								//  bypass the BOM
		}
		Flags = SF_TEXT;						// Setup EM_STREAMIN for text
	}

	SetFilePointer((HANDLE)es.dwCookie, j, NULL, FILE_BEGIN);	// Rewind

	if(fReplaceSel)
		Flags |= SFF_SELECTION;

	Flags |= SFF_KEEPDOCINFO;

	hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	TxSendMessage(EM_STREAMIN, Flags, (LPARAM)&es, &lres);
	SetCursor(hcur);

	if(dwShare == (FILE_SHARE_READ | FILE_SHARE_WRITE) || fReplaceSel)
	{											// Full sharing or replaced
		CloseHandle((HANDLE)es.dwCookie);		//  selection, so close file
		if(!fReplaceSel)						// If replacing selection,
			pDocInfo->hFile = NULL;				//  leave _pDocInfo->hFile
	}
	_fSaved = fReplaceSel ? FALSE : TRUE;		// No changes yet unless
	return (HRESULT)es.dwError;
}

/*
 *	ITextDocument::Range(long cpFirst, long cpLim, ITextRange **ppRange)  
 *
 *	@mfunc
 *		Method that gets a text range on the active story of the document
 *
 *	@rdesc
 *		HRESULT = (!ppRange) ? E_INVALIDARG : 
 *				  (if success) ? NOERROR : E_OUTOFMEMORY
 */
STDMETHODIMP CTxtEdit::Range (
	long cpFirst, 				//@parm	Non active end of new range
	long cpLim, 				//@parm Active end of new range
	ITextRange ** ppRange)		//@parm Out parm to receive range
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Range");

	if(!ppRange)
		return E_INVALIDARG;

	*ppRange = new CTxtRange(this, cpFirst, cpFirst - cpLim);
	
	if( *ppRange )
	{
		(*ppRange)->AddRef();		// CTxtRange() doesn't AddRef() because
		return NOERROR;				//  it's used internally for things
	}								//  besides TOM

	return E_OUTOFMEMORY;
}

/*
 *	ITextDocument::RangeFromPoint(long x, long y, ITextRange **ppRange) 
 *
 *	@mfunc
 *		Method that gets the degenerate range corresponding (at or nearest)
 *		to the point with the screen coordinates x and y.
 *
 *	@rdesc
 *		HRESULT = (!ppRange) ? E_INVALIDARG :
 *				  (if out of RAM) ? E_OUTOFMEMORY :
 *				  (if range exists) ? NOERROR : S_FALSE
 */
STDMETHODIMP CTxtEdit::RangeFromPoint (
	long	x,				//@parm Horizontal coord of point to use
	long	y,				//@parm	Vertical   coord of point to use
	ITextRange **ppRange)	//@parm Out parm to receive range
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::RangeFromPoint");

	if(!ppRange)
		return E_INVALIDARG;

	*ppRange = (ITextRange *) new CTxtRange(this, 0, 0);

	if(!*ppRange)
		return E_OUTOFMEMORY;

	(*ppRange)->AddRef();				// CTxtRange() doesn't AddRef()
	return (*ppRange)->SetPoint(x, y, 0, 0);
}

/*
 *	ITextDocument::Redo(long Count, long *pCount) 
 *
 *	@mfunc
 *		Method to perform the redo operation Count times
 *
 *	@rdesc
 *		HRESULT = (if Count redos performed) ? NOERROR : S_FALSE
 */
STDMETHODIMP CTxtEdit::Redo (
	long	Count,		//@parm Number of redo operations to perform
	long *	pCount)		//@parm Number of redo operations performed
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Redo");
	CCallMgr	callmgr(this);

	LONG i = 0;

	if (_predo)
	{
		// Freeze the display during the execution of the anti-events
		CFreezeDisplay fd(_pdp);

		for ( ; i < Count; i++)
			PopAndExecuteAntiEvent(_predo, 0);
	}

	if(pCount)
		*pCount = i;

	return i == Count ? NOERROR : S_FALSE;
}

/*
 *	ITextDocument::Save(pVar, Flags, CodePage) 
 *
 *	@mfunc
 *		Method that saves this ITextDocument to the target pVar,
 *		which is a VARIANT that can be a filename, an IStream, or NULL.  If
 *		NULL, the filename given by this document's name is used.  It that,
 *		in turn, is NULL, the method fails.  If pVar specifies a filename,
 *		that name should replace the current Name property.
 *
 *	@rdesc
 *		HRESULT = (!pVar) ? E_INVALIDARG : 
 *				  (if success) ? NOERROR : E_FAIL
 *
 *	@devnote
 *		This routine can be called with NULL arguments
 */
STDMETHODIMP CTxtEdit::Save (
	VARIANT *	pVar,		//@parm Save target (filename or IStream)
	long		Flags,		//@parm Read/write, create, and share flags
	long		CodePage)	//@parm Code page to use
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Save");

	LONG		cb;			// Byte count for writing Unicode BOM
	EDITSTREAM	es		= {0, NOERROR, MyWrite};
	BOOL		fChange	= FALSE;				// No doc info change yet
	HCURSOR		hcur;
	CDocInfo *	pDocInfo = GetDocInfo();
	WORD		wBOM = BOM;

	if(CodePage && CodePage != 1200 && !IsValidCodePage(CodePage) ||
	   (DWORD)Flags > 0x1fff)
	{
		return E_INVALIDARG;
	}
	if((Flags & 0xf) >= tomHTML)				// RichEdit only handles auto,
		return E_NOTIMPL;						//  plain text, & RTF formats

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	if (pVar && pVar->vt == VT_BSTR &&			// Filename string
		pVar->bstrVal &&
		pSysStringLen(pVar->bstrVal) &&			// NonNULL filename specified
		(!pDocInfo->pName ||
		 OLEstrcmp(pVar->bstrVal, pDocInfo->pName)))
	{											// Filename differs
		fChange = TRUE;							// Force write to new file
		CloseFile(FALSE);						// Close current file; no save
		pDocInfo->pName = pSysAllocString(pVar->bstrVal);
		if( !pDocInfo->pName )
			return E_OUTOFMEMORY;
		pDocInfo->wFlags &= ~0xf0;				// Kill previous create mode
	}

	DWORD flags = pDocInfo->wFlags;
	if(!(Flags & 0xf))							// If convert flags are 0,									
		Flags |= flags & 0xf;					//  use values in doc info
	if(!(Flags & 0xf0))							// If create flags are 0,									
		Flags |= flags & 0xf0;					//  use values in doc info
	if(!(Flags & 0xf00))						// If share flags are 0,									
		Flags |= flags & 0xf00;					//  use values in doc info
	if(!CodePage)								// If code page is 0,
		CodePage = pDocInfo->wCpg;				//  use code page in doc info

	if((DWORD)Flags != flags ||					// If flags or code page	
	   (WORD)CodePage != pDocInfo->wCpg)		//  changed, force write
	{
		fChange = TRUE;
	}
	pDocInfo->wCpg	 = (WORD)CodePage;			// Save new code page and
	pDocInfo->wFlags = (WORD)Flags;				//  flags


	// yikes, nowhere to save.  bail-out now.
	if( !_pDocInfo->pName )
		return E_FAIL;

	if(_fSaved && !fChange)						// No changes, so assume
		return NOERROR;							//  saved file is up to date

	// Process access, share, and create flags
	DWORD dwAccess = (Flags & tomReadOnly)
		? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);

	DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
	if(Flags & tomShareDenyRead)
		dwShare &= ~FILE_SHARE_READ;

	if(Flags & tomShareDenyWrite)
		dwShare &= ~FILE_SHARE_WRITE;

	DWORD dwCreate = (Flags >> 4) & 0xf;
	if(!dwCreate)
		dwCreate = CREATE_NEW;

	if(pDocInfo->hFile)
	{
		CloseHandle(pDocInfo->hFile);			// Close current file handle
		pDocInfo->hFile = NULL;
	}

	es.dwCookie = (DWORD)CreateFile(pDocInfo->pName, dwAccess, dwShare, NULL,
							dwCreate, FILE_ATTRIBUTE_NORMAL, NULL);
	if((HANDLE)es.dwCookie == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());

	pDocInfo->hFile = (HANDLE)es.dwCookie;

	Flags &= 0xf;								// Isolate conversion flags
	if(Flags == tomRTF)							// RTF
		Flags = SF_RTF;							// Setup EM_STREAMOUT for RTF
	else
	{											// If Unicode, start file with
		if(pDocInfo->wCpg == 1200)				//  Unicode byte order mark
			(*es.pfnCallback)(es.dwCookie, (LPBYTE)&wBOM, 2, &cb);
		Flags = SF_TEXT;						// Setup EM_STREAMOUT for text
	}

	hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	TxSendMessage(EM_STREAMOUT, Flags, (LPARAM)&es, NULL);
	SetCursor(hcur);

	if(dwShare == (FILE_SHARE_READ | FILE_SHARE_WRITE))
	{											// Full sharing, so close
		CloseHandle(pDocInfo->hFile);			//  current file handle
		pDocInfo->hFile = NULL;
	}
	_fSaved = TRUE;								// File is saved
	return (HRESULT)es.dwError;
}

/*
 *	ITextDocument::SetDefaultTabStop (Value) 
 *
 *	@mfunc
 *		Property set method that sets the default tab stop to be
 *		used whenever the explicit tabs don't extend far enough.
 *
 *	@rdesc
 *		HRESULT = (Value < 0) ? E_INVALIDARG : NOERROR
 */
STDMETHODIMP CTxtEdit::SetDefaultTabStop (
	float Value)		//@parm Out parm to receive default tab stop
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDefaultTabStop");

	if(Value < 0)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	pDocInfo->dwDefaultTabStop = FPPTS_TO_TWIPS(Value);
	return NOERROR;
}

/*
 *	ITextDocument::SetSaved (Value) 
 *
 *	@mfunc
 *		Property set method that sets whether this instance has been
 *		saved, i.e., no changes since last save
 *
 *	@rdesc
 *		HRESULT = NOERROR
 */
STDMETHODIMP CTxtEdit::SetSaved (
	long	Value)		//@parm New value of Saved property
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetSaved");

	_fSaved = Value ? TRUE : FALSE;
	return NOERROR;
}

/*
 *	ITextDocument::Undo(long Count, long *pCount) 
 *
 *	@mfunc
 *		Method to perform the undo operation Count times.
 *
 *	@rdesc
 *		HRESULT = (if Count undos performed) ? NOERROR : S_FALSE
 */
STDMETHODIMP CTxtEdit::Undo (
	long	Count,		//@parm Number of undo operations to perform
						//		0 suspends undo processing
						//		-1 turns restores it
	long *	pCount)		//@parm Number of undo operations performed
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Undo");
	CCallMgr callmgr(this);

	LONG i = 0;

	if (_pundo)
	{
		// Freeze the display during the execution of the anti-events
		CFreezeDisplay fd(_pdp);

		for ( ; i < Count; i++)
			PopAndExecuteAntiEvent(_pundo, 0);
	}

	if(pCount)
		*pCount = i;

	if(Count <= 0)							// Note: for Count <= 0, for
	{										//  loop does nothing, since
		if(Count < 0)
			Count = DEFAULT_UNDO_SIZE;
		i = HandleSetUndoLimit(Count);
	}

	return i == Count ? NOERROR : S_FALSE;
}

/*
 *	ITextDocument::Unfreeze(long *pCount) 
 *
 *	@mfunc
 *		Method to decrement freeze count.  If this count goes to zero,
 *		screen updating is enabled.  This method cannot decrement the
 *		count below zero.
 *
 *	@rdesc
 *		HRESULT = (screen updating enabled) ? NOERROR : S_FALSE
 *
 *	@devnote
 *		The display maintains its own private reference count which may
 *		temporarily exceed the reference count of this method.  So even
 *		if this method indicates that the display is unfrozen, it may be
 *		for a while longer.
 */
STDMETHODIMP CTxtEdit::Unfreeze (
	long *pCount)		//@parm Out parm to receive updated freeze count
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Unfreeze");

#ifdef FUTURE
	if(_cFreeze)
	{
		AssertSz(_pdp && _pdp->IsFrozen(),
			"CTxtEdit::Unfreeze: screen not frozen but expected to be");
		_cFreeze--;
		_pdp->Thaw();
	}

	if(pCount)
		*pCount = _cFreeze;

	return _cFreeze ? S_FALSE : NOERROR;
#endif
	return E_NOTIMPL;
}

//----------------------- ITextDocument Helper Functions -----------------------
/*
 *	ITextDocument::CloseFile ()
 *
 *	@mfunc
 *		Method that closes the current document. If changes have been made
 *		in the current document since the last save and document file
 *		information exists, the current document is saved.
 *
 *	@rdesc
 *		HRESULT = NOERROR
 */
HRESULT CTxtEdit::CloseFile (BOOL bSave)
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::Close");

	CDocInfo *pDocInfo = _pDocInfo;

	if(pDocInfo)
	{
		if(bSave)									// Save current file if
			Save(NULL, 0, 0);						//  any changes made
	
		// FUTURE(BradO):  This code is very similar to the destructor code.
		// We have a problem here in that some of the CDocInfo information
		// should persist from Open to Close to Open (ex. default tab stop)
		// mixed with other per-Open/Close info.  A better job of abstracting
		// these two types of info would really clean up this code.

		if(pDocInfo->pName)
		{
			pSysFreeString(pDocInfo->pName);			// Free filename BSTR
			pDocInfo->pName = NULL;
		}

		if(pDocInfo->hFile)
		{
			CloseHandle(pDocInfo->hFile);			// Close file if open
			pDocInfo->hFile = NULL;
		}
		pDocInfo->wFlags = 0;
		pDocInfo->wCpg = 0;

		pDocInfo->lcid = 0;
		pDocInfo->lcidfe = 0;

		if(pDocInfo->lpstrLeadingPunct)
		{
			FreePv(pDocInfo->lpstrLeadingPunct);
			pDocInfo->lpstrLeadingPunct = NULL;
		}

		if(pDocInfo->lpstrFollowingPunct)
		{
			FreePv(pDocInfo->lpstrFollowingPunct);
			pDocInfo->lpstrFollowingPunct = NULL;
		}
	}
	return NOERROR;
}

/*
 *	SetDefaultLCID (lcid) 
 *
 *	@mfunc
 *		Property set method that sets the default LCID
 *
 *	@rdesc
 *		HRESULT = NOERROR
 *
 *	@comm
 *		This property should be part of TOM
 */
HRESULT CTxtEdit::SetDefaultLCID (
	LCID lcid)		//@parm New default LCID value
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDefaultLCID");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	pDocInfo->lcid = lcid;
	return NOERROR;
}

/*
 *	GetDefaultLCID (pLCID) 
 *
 *	@mfunc
 *		Property get method that gets the default LCID
 *
 *	@rdesc
 *		HRESULT = (!pLCID) ? E_INVALIDARG : NOERROR
 */
HRESULT CTxtEdit::GetDefaultLCID (
	LCID *pLCID)		//@parm Out parm with default LCID value
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDefaultLCID");

	if(!pLCID)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	*pLCID = _pDocInfo->lcid;
	return NOERROR;
}

/*
 *	SetDefaultLCIDFE (lcid) 
 *
 *	@mfunc
 *		Property set method that sets the default FE LCID
 *
 *	@rdesc
 *		HRESULT = NOERROR
 *
 *	@comm
 *		This property should be part of TOM
 */
HRESULT CTxtEdit::SetDefaultLCIDFE (
	LCID lcid)		//@parm New default LCID value
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDefaultLCIDFE");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	pDocInfo->lcidfe = lcid;
	return NOERROR;
}

/*
 *	GetDefaultLCIDFE (pLCID) 
 *
 *	@mfunc
 *		Property get method that gets the default FE LCID
 *
 *	@rdesc
 *		HRESULT = (!pLCID) ? E_INVALIDARG : NOERROR
 */
HRESULT CTxtEdit::GetDefaultLCIDFE (
	LCID *pLCID)		//@parm Out parm with default LCID value
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDefaultLCID");

	if(!pLCID)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	*pLCID = _pDocInfo->lcidfe;
	return NOERROR;
}

/*
 *	CTxtEdit::SetDocumentType(bDocType) 
 *
 *	@mfunc
 *		Property set method that sets the document's type (none-\ltrdoc-\rtldoc)
 *
 *	@rdesc
 *		HRESULT = NOERROR
 */
HRESULT CTxtEdit::SetDocumentType (
	LONG DocType)		//@parm New document-type value
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetDocumentType");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	pDocInfo->bDocType = (BYTE)DocType;
	return NOERROR;
}

/*
 *	GetDocumentType (pDocType) 
 *
 *	@mfunc
 *		Property get method that gets the document type
 *
 *	@rdesc
 *		HRESULT = (!pDocType) ? E_INVALIDARG : NOERROR
 */
HRESULT CTxtEdit::GetDocumentType (
	LONG *pDocType)		//@parm Out parm with document type value
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetDocumentType");

	if(!pDocType)
		return E_INVALIDARG;

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)								// Doc info doesn't exist
		return E_OUTOFMEMORY;

	*pDocType = _pDocInfo->bDocType;
	return NOERROR;
}

/*
 *	CTxtEdit::GetLeadingPunct (plpstrLeadingPunct)
 *
 *	@mfunc
 *		Retrieve leading kinsoku punctuation for document
 *
 *	@rdesc
 *		HRESULT = (!<p plpstrLeadingPunct>) ? E_INVALIDARG :
 *				  (no leading punct) ? S_FALSE :
 *				  (if not enough RAM) ? E_OUTOFMEMORY : NOERROR
 */
HRESULT CTxtEdit::GetLeadingPunct (
	LPWSTR * plpstrLeadingPunct)		//@parm Out parm to receive leading 
								//	kinsoku punctuation
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetLeadingPunct");

	if(!plpstrLeadingPunct)
		return E_INVALIDARG;

	*plpstrLeadingPunct = NULL;
	if(!_pDocInfo || !_pDocInfo->lpstrLeadingPunct)
		return S_FALSE;

	*plpstrLeadingPunct = _pDocInfo->lpstrLeadingPunct;
	
	return NOERROR;
}


/*
 *	CTxtEdit::SetLeadingPunct (lpstrLeadingPunct)
 *
 *	@mfunc
 *		Set leading kinsoku punctuation for document
 *
 *	@rdesc
 *		HRESULT = (if not enough RAM) ? E_OUTOFMEMORY : NOERROR
 */
HRESULT CTxtEdit::SetLeadingPunct (
	LPWSTR lpstrLeadingPunct)	//@parm In parm containing leading 
								//	kinsoku punctuation
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetLeadingPunct");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)
		return E_OUTOFMEMORY;

	if(pDocInfo->lpstrLeadingPunct)
	{
		FreePv(pDocInfo->lpstrLeadingPunct);
	}

	if(lpstrLeadingPunct && *lpstrLeadingPunct)
	{
        size_t nLength = wcslen(lpstrLeadingPunct) + 1;
		pDocInfo->lpstrLeadingPunct = 
			(WCHAR *)PvAlloc(nLength * sizeof(WCHAR), 
				GMEM_ZEROINIT);

		if(!pDocInfo->lpstrLeadingPunct)
			return E_OUTOFMEMORY;

		wcscpy_s(pDocInfo->lpstrLeadingPunct, nLength, lpstrLeadingPunct);
	}
	else
	{
		pDocInfo->lpstrLeadingPunct = NULL;
	}

	return NOERROR;
}


/*
 *	CTxtEdit::GetFollowingPunct (plpstrFollowingPunct)
 *
 *	@mfunc
 *		Retrieve following kinsoku punctuation for document
 *
 *	@rdesc
 *		HRESULT = (!<p plpstrFollowingPunct>) ? E_INVALIDARG :
 *				  (no following punct) ? S_FALSE :
 *				  (if not enough RAM) ? E_OUTOFMEMORY : NOERROR
 */
HRESULT CTxtEdit::GetFollowingPunct (
	LPWSTR * plpstrFollowingPunct)		//@parm Out parm to receive following 
								//	kinsoku punctuation
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::GetFollowingPunct");

	if(!plpstrFollowingPunct)
		return E_INVALIDARG;

	*plpstrFollowingPunct = NULL;
	if(!_pDocInfo || !_pDocInfo->lpstrFollowingPunct)
		return S_FALSE;

	*plpstrFollowingPunct = _pDocInfo->lpstrFollowingPunct;
	
	return NOERROR;
}


/*
 *	CTxtEdit::SetFollowingPunct (lpstrFollowingPunct)
 *
 *	@mfunc
 *		Set following kinsoku punctuation for document
 *
 *	@rdesc
 *		HRESULT = (if not enough RAM) ? E_OUTOFMEMORY : NOERROR
 */
HRESULT CTxtEdit::SetFollowingPunct (
	LPWSTR lpstrFollowingPunct)		//@parm In parm containing following 
									//	kinsoku punctuation
{
	TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtEdit::SetFollowingPunct");

	CDocInfo *pDocInfo = GetDocInfo();

	if(!pDocInfo)
		return E_OUTOFMEMORY;

	if(pDocInfo->lpstrFollowingPunct)
	{
		FreePv(pDocInfo->lpstrFollowingPunct);
	}

	if(lpstrFollowingPunct && *lpstrFollowingPunct)
	{
        size_t nLength = wcslen(lpstrFollowingPunct) + 1;
		pDocInfo->lpstrFollowingPunct = 
			(WCHAR *)PvAlloc(nLength * sizeof(WCHAR),
				GMEM_ZEROINIT);

		if(!pDocInfo->lpstrFollowingPunct)
			return E_OUTOFMEMORY;

		wcscpy_s(pDocInfo->lpstrFollowingPunct, nLength, lpstrFollowingPunct);
	}
	else
	{
		pDocInfo->lpstrFollowingPunct = NULL;
	}

	return NOERROR;
}

/*
 *	CTxtEdit::InitDocInfo()
 *
 *	@mfunc	constructor for the docinfo class
 */
HRESULT CTxtEdit::InitDocInfo()
{
	_wZoomNumerator = _wZoomDenominator = 0;
	if(_pDocInfo)
	{
		_pDocInfo->InitDocInfo();
		return NOERROR;
	}

	return GetDocInfo() ? NOERROR : E_OUTOFMEMORY;
}


//----------------------- CDocInfo related Functions -----------------------
/*
 *	CDocInfo::InitDocInfo()
 *
 *	@mfunc	constructor for the docinfo class
 */
void CDocInfo::InitDocInfo()
{
	wCpg = GetACP();
	lcid = GetSystemDefaultLCID();

	if(IsFELCID(lcid))
	{
		lcidfe = lcid;
		lcid = MAKELCID(sLanguageEnglishUS, SORT_DEFAULT);
	}

	dwDefaultTabStop = lDefaultTab;
	bDocType = 0;
}

/*
 *	CDocInfo::~CDocInfo
 *
 *	@mfunc	desctructor for the docinfo class
 */
CDocInfo::~CDocInfo()
{
	if( pName )
	{
		pSysFreeString(pName);
		pName = NULL;
	}

	if( hFile )
	{
		CloseHandle(hFile);
		hFile = NULL;
	}

	if(lpstrLeadingPunct)
	{
		FreePv(lpstrLeadingPunct);
		lpstrLeadingPunct = NULL;
	}

	if(lpstrFollowingPunct)
	{
		FreePv(lpstrFollowingPunct);
		lpstrFollowingPunct = NULL;
	}
}

/*
 *	CTxtEdit::GetDocInfo ()
 *
 *	@mfunc
 *		If _pDocInfo is NULL, equate it to a new CDocInfo.  In either case
 *		return _pDocInfo
 *
 *	@rdesc
 *		CTxtEdit::_pDocInfo, the ptr to the CDocInfo object
 */
CDocInfo * CTxtEdit::GetDocInfo()
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "CTxtEdit::GetDocInfo");

	if (!_pDocInfo)
		_pDocInfo = new CDocInfo();

	// It is the caller's responsiblity to notice that an error occurred
	// in the allocation of the CDocInfo object.
	return _pDocInfo;
}


