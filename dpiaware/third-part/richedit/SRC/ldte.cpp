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
 *	@doc INTERNAL
 *
 *	@module	LDTE.C - RichEdit Light Data Transfer Engine |
 *
 *		This file contains data transfer code using IDataObject
 *
 *	Author: <nl>
 *		alexgo (4/25/95)
 *
 *	Revisions: <nl>
 *		murrays (7/6/95) auto-doc'd and added RTF support
 *
 *	FUTURE (AlexGo): <nl>
 *		Maybe merge this class with CTxtRange to make more efficient use of
 *		the this ptr.  All but two methods use a CTxtRange and one of these
 *		could be global.  The two are:
 *
 *		GetDropTarget( IDropTarget **ppDropTarget )
 *		GetDataObjectInfo(IDataObject *pdo, DWORD *pDOIFlags) // Can be global
 *
 *		In general, a range can spawn data objects, which need to have a clone
 *		of the range in case the range is moved around.  The contained range
 *		is used for delayed rendering.  A prenotification is sent to the data
 *		object just before the data object's data is to be changed.  The data
 *		object then renders the data in its contained range, whereupon the
 *		object becomes independent of the range and destroys the range.
 *
 *	@devnote
 *		We use the word ANSI in a general way to mean any multibyte character
 *		system as distinguished from 16-bit Unicode.  Technically, ANSI refers
 *		to a specific single-byte character system (SBCS).  We translate
 *		between "ANSI" and Unicode text using the Win32
 *		MultiByteToWideChar() and WideCharToMultiByte() APIs.
 *
 */

#include "_common.h"
#include "_range.h"
#include "_ldte.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_edit.h"
#include "_disp.h"
#include "_select.h"
#include "_dragdrp.h"
#include "_dxfrobj.h"
#include "_rtfwrit.h"
#include "_rtfread.h"
#include "_urlsup.h"

ASSERTDATA


//Local Prototypes
DWORD CALLBACK ReadHGlobal (DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
DWORD CALLBACK WriteHGlobal(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

#define	SFF_ADJUSTENDEOP	0x80000000
//
// LOCAL METHODS
//

/*
 *	void PasteSetupDBCFont ( )
 *
 *	@func
 *		This routine is to setup the DBC font by calling CheckChangeFont() with 
 *		the current system LCID.  This is used during pasting/inserting
 *		plain text.
 */
void PasteSetupDBCFont ( CTxtEdit *ped )
{
	WORD lcidSystem;
	CTxtSelection *psel;

	if (ped->_fSingleCodePage || !ped->IsRich())
		{
		goto leave;
		}

	psel = ped->GetSel();
	if(!psel)
		{
		goto leave;
		}

	if (psel->GetCch())
	{
		// for selection, we need to get the character format at cpMin+1
		CTxtRange rg( ped, psel->GetCpMin()+1, 0 );
		LONG	iFormat;

		iFormat = rg.Get_iCF ();
		psel->Set_iCF( iFormat );

		// release format that has been AddRef in Get_iCF()
		ReleaseFormats(iFormat, -1);		

		psel->SetUseiFormat( TRUE );
	}

	lcidSystem = GetSystemDefaultLCID();
	psel->CheckChangeFont( ped, FALSE, lcidSystem,
		ConvertLanguageIDtoCodePage(lcidSystem) );	
leave:
	return;
}

/*
 *	ReadHGlobal(dwCookie, pbBuff, cb, pcb)
 *
 *	@func
 *		EDITSTREAM callback for reading from an hglobal
 *
 *	@rdesc
 *		es.dwError
 */
DWORD CALLBACK ReadHGlobal(
	DWORD	dwCookie,			// @parm READHGLOBAL *
	LPBYTE	pbBuff,				// @parm Buffer to fill
	LONG	cb,					// @parm Buffer length
	LONG *	pcb)				// @parm Out parm for # bytes stored
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "ReadHGlobal");

	READHGLOBAL * const prhg = (READHGLOBAL *)dwCookie;

	cb = min(cb, prhg->cbLeft);
	CopyMemory(pbBuff, prhg->ptext, cb);
	prhg->cbLeft	-= cb;
	prhg->ptext		+= cb;
	if(pcb)
		*pcb = cb; 
	return NOERROR;	
}

/*
 *	WriteHGlobal(dwCookie, pbBuff, cb, pcb)
 *
 *	@func
 *		EDITSTREAM callback for writing ASCII to an hglobal
 *
 *	@rdesc
 *		error (E_OUTOFMEMORY or NOERROR)
 */
DWORD CALLBACK WriteHGlobal(
	DWORD	dwCookie,			// @parm WRITEHGLOBAL *
	LPBYTE	pbBuff,				// @parm Buffer to write from
	LONG	cb,					// @parm Buffer length
	LONG *	pcb)				// @parm Out parm for # bytes written
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "WriteHGlobal");

	WRITEHGLOBAL * const pwhg = (WRITEHGLOBAL *)dwCookie;
	HGLOBAL		hglobal = pwhg->hglobal;
	LPSTR		pstr;

	if (pwhg->cch < 0 || cb < 0 || pwhg->cch + cb < pwhg->cch)
		return (DWORD)E_OUTOFMEMORY;
	if(pwhg->cch + cb > pwhg->cb)			// Less than requested cb in
	{										//  current Alloc
		if (pwhg->cb < 0 || 2 * pwhg->cb < pwhg->cb)
			return (DWORD)E_OUTOFMEMORY;
		ULONG cbNewSize = GROW_BUFFER(pwhg->cb, cb);
		hglobal = GlobalReAlloc(hglobal, cbNewSize, GMEM_MOVEABLE);
		if(!hglobal)	
			return (DWORD)E_OUTOFMEMORY;
		pwhg->hglobal = hglobal;			// May be superfluous...
		pwhg->cb = cbNewSize;
	}
	pstr = (LPSTR)GlobalLock(hglobal);
	if(!pstr)
		return (DWORD)E_OUTOFMEMORY;
	CopyMemory(pstr + pwhg->cch, pbBuff, cb);
	GlobalUnlock(hglobal);
	pwhg->cch += cb;
	if(pcb)
		*pcb = cb; 
	return NOERROR;	
}


//
// PUBLIC METHODS
//

/*
 *	Cleanse(pchD, pchS, cchS)
 *
 *	@func
 *		"Cleanses" the source string pchS of length cch by copying it to the
 *		destination string pchD ignoring LFs and replacing Unicode paragraph
 *		(0x2029) and line (0x2028) separators by Word/TOM-compatible CR and
 *		VT, respectively. Returns resulting clensed string length.  Also
 *		replaces CRCRLFs (MLE soft line breaks) by blanks.
 *
 *	@rdesc
 *		Length of cleansed string.
 *
 *	@devnote
 *		Caller needs to be sure that destination string is as long as the
 *		source string, in case no LFs are encountered.  pchD can point to
 *		the same string as pchS, since the cleansed string cannot be longer
 *		than the input string.  However if pchS points at a buffer that
 *		doesn't contain the whole input string, then the caller has to deal
 *		with CRLF and CRCRLF combos that overlap buffers.
 */
LONG Cleanse(
	TCHAR *			pchD,	//@parm Source string
	const TCHAR *   pchS,	//@parm Destination string
	LONG			cchS)	//@parm Length of source string
{
	LONG	 cchD = 0;
	unsigned ch;			// Current character value
	LONG	 i = cchS;		// Loop counter

	while(i-- > 0)
	{
		ch = *pchS++;						// Examine next char
		if(IsASCIIEOP(ch))					// Handle CR and LF combos
		{
			if(ch == CR)
			{
				if(i > 1 && *pchS == CR && *(pchS+1) == LF)
				{
					ch = ' ';				// Translate CRCRLF to ' '
					pchS += 2;
					i -= 2;
				}
				else if(i && *pchS == LF)	// Bypass LF of CRLF
				{
					pchS++;
					i--;
				}
			}
			else if(ch == LF)				// Treat lone LFs as EOPs, i.e.,
				ch = CR;					//  be nice to Unix text files
		}
		else if((ch | 1) == PS)				// Translate Unicode para/line
			ch = (ch == PS) ? CR : VT;		//  separators into CR/VT

		*pchD++ = ch;
		cchD++;
	}

	return cchD;
}

/*
 *	CLightDTEngine::CLightDTEngine()
 *
 *	@mfunc
 *		Constructor for Light Data Transfer Engine
 */
CLightDTEngine::CLightDTEngine()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CLightDTEngine");

	_ped = NULL;
	_pdt = NULL;
	_pdo = NULL;
	_fUseLimit = FALSE;
	_fOleless = FALSE;
}

/*
 *	CLightDTEngine::~CLightDTEngine 
 *
 *	@mfunc
 *		Handles all necessary clean up for the object..
 */
CLightDTEngine::~CLightDTEngine()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::~CLightDTEngine");

	if( _pdt )
	{
		_pdt->Zombie();
		_pdt->Release();
		_pdt = NULL;
	}
	Assert(_pdo == NULL);
}

/*
 *	CLightDTEngine::Destroy()
 *
 *	@mfunc
 *		Deletes this instance
 */
void CLightDTEngine::Destroy()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::Destroy");

	delete this;
}

/*
 *	CLightDTEngine::CopyRangeToClipboard ( prg )
 *
 *	@mfunc
 *		Copy the text of the range prg to the clipboard using Win32 APIs
 *
 *	@rdesc
 *		HRESULT
 */
HRESULT CLightDTEngine::CopyRangeToClipboard(
	CTxtRange *prg )				// @parm range to copy to clipboard
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CopyRangeToClipboard");

	HRESULT hresult = E_FAIL;
	IDataObject *pdo;
	IRichEditOleCallback * precall = _ped->GetRECallback();
	BOOL fSingleObject;
	CHARRANGE chrg;

	prg->GetRange(chrg.cpMin, chrg.cpMost);
	fSingleObject = chrg.cpMost - chrg.cpMin == 1 &&
		_ped->HasObjects() &&
		_ped->_pobjmgr->CountObjectsInRange(chrg.cpMin, chrg.cpMost);
	if(precall)
	{
		// give the callback a chance to give us it's own IDataObject
		hresult = precall->GetClipboardData(&chrg, RECO_COPY, &pdo);
	}

	// If we didn't get an IDataObject from the callback, build our own
	if(hresult != NOERROR)
	{
		// if the range is empty, don't bother creating it.  Just
		// leave the clipboard alone and return
		if( prg->GetCch() == 0 )
		{
			_ped->Sound();
			return NOERROR;
		}

		hresult = RangeToDataObject(prg, SF_TEXT | SF_RTF, &pdo);
	}

	// NB: it's important to check both hresult && pdo; it is legal for
	// our client to say "yep, I handled the copy, but there was nothing
	// to copy".
	if( hresult == NOERROR && pdo )
	{
		hresult = pOleSetClipboard(pdo);
		if( hresult != NOERROR )
		{
			HWND hwnd;
			_fOleless = TRUE;
			// Ole less clipboard support
			if (_ped->TxGetWindow(&hwnd) == NOERROR &&
				::OpenClipboard(hwnd) &&
				::EmptyClipboard()
			)
			{
				::SetClipboardData(CF_UNICODETEXT, NULL);
				::SetClipboardData(cf_RTF, NULL);
				::SetClipboardData(cf_RTFUTF8, NULL);
				if (fSingleObject)
					::SetClipboardData(CF_DIB, NULL);
				::CloseClipboard();
				hresult = NOERROR;				// To cause replace range to happen
			}
		}
        if (_pdo)
		{
			_pdo->Release();
		}
		_pdo = pdo;
	}
	return hresult;
}

/* 
 *	CLightDTEngine::CutRangeToClipboard( prg, publdr );
 *	
 *	@mfunc
 *		Cut text of the range prg to the clipboard
 *
 *	@devnote
 *		If publdr is non-NULL, anti-events for the cut operation should be
 *		stuffed into this collection
 *
 *	@rdesc
 *		HRESULT from CopyRangeToClipboard()
 *
 *	@devnote
 *		First copy the text to the clipboard, then delete it from the range
 */
HRESULT CLightDTEngine::CutRangeToClipboard(
	CTxtRange *prg,				// @parm range to cut to clipboard
	IUndoBuilder *publdr )		// @parm undo builder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CutRangeToClipboard");

	HRESULT hresult = E_ACCESSDENIED;

	Assert(!_ped->TxGetReadOnly());

	prg->AdjustEndEOP(NONEWCHARS);				// Don't include trailing EOP
												//  in some selection cases
	hresult = CopyRangeToClipboard(prg);

	if( publdr )
	{
		publdr->SetNameID(UID_CUT);
		publdr->StopGroupTyping();
	}

	if( hresult == NOERROR )
	{
		// Delete contents of range
		prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);	
	}

	return hresult;
}


/*
 *	CLightDTEngine::FlushClipboard()
 *
 *	@mfunc	flushes the clipboard (if needed).  Typically called during
 *			shutdown.
 *
 *	@rdesc	void
 */
void CLightDTEngine::FlushClipboard()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::FlushClipboard");
	ENSAVECLIPBOARD ens;

	if( _pdo )
	{
		if( pOleIsCurrentClipboard(_pdo) == NOERROR )
		{
			CDataTransferObj *pdo = NULL;

			// check to see if we have to flush the clipboard.
			ZeroMemory(&ens, sizeof(ENSAVECLIPBOARD));

			// check to make sure the object is one of ours before accessing
			// the memory.  

			if( _pdo->QueryInterface(IID_IRichEditDO, (void **)&pdo ) 
				== NOERROR && pdo  )
			{
				ens.cObjectCount = pdo->_cObjs;
				ens.cch = pdo->_cch;
				pdo->Release();
			}

			if( _ped->TxNotify(EN_SAVECLIPBOARD, &ens) == NOERROR )
			{
				pOleFlushClipboard();
			}
			else
			{
				pOleSetClipboard(NULL);
			}
		}
		_pdo->Release();
		_pdo = NULL;
	}
}

/*
 *	CLightDTEngine::CanPaste(pdo, cf, flags)
 *
 *	@mfunc
 *		Determines if clipboard format cf is one we can paste.
 *
 *	@rdesc
 *		BOOL - true if we can paste cf into range prg OR DF_CLIENTCONTROL
 *		if the client is going to handle this one.
 *
 *	@devnote
 *		we check the clipboard ourselves if cf is 0. Primarily, this
 *		is for backwards compatibility with Richedit1.0's EM_CANPASTE
 *		message.
 *
 */
DWORD CLightDTEngine::CanPaste(
	IDataObject *pdo,	// @parm Data object to check; if NULL use clipboard
	CLIPFORMAT cf, 		// @parm clipboard format to query about; if 0, use
						//		 best available.
	DWORD flags)		// @parm flags 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CanPaste");

	IRichEditOleCallback *precall = _ped->GetRECallback();
	CLIPFORMAT	cf0 = cf;
	DWORD		cFETC = CFETC;
	HRESULT		hr = NOERROR;
	DWORD		ret = FALSE;

#ifndef MACPORT								 
	if( pdo == NULL && precall )
#else
	if( pdo == NULL)
#endif
	{
		// don't worry about errors
		pOleGetClipboard(&pdo);
	}
	else if( pdo )
	{
		// so we can make just one 'Release' call below
		pdo->AddRef();
	}
	
	if( precall && pdo )
	{
		hr = precall->QueryAcceptData(pdo, &cf, flags, 0, NULL);

		if( SUCCEEDED(hr) && (hr != S_OK && hr != DATA_S_SAMEFORMATETC ) )
		{
			ret = DF_CLIENTCONTROL;
			goto Exit;
		}
		else if( FAILED(hr) && hr != E_NOTIMPL )
		{
			goto Exit;
		}
		else if(SUCCEEDED(hr))
		{
			// we should go on and check ourselves unless the client
			// modified the format when it shouldn't have
			if( cf0 && cf0 != cf )
			{
				goto Exit;
			}
		}

		// otherwise, continue with our normal checks
	}

    if (_ped->TxGetReadOnly())		    // Can't paste if read only
		goto Exit;

	while(cFETC--)						// Does cf = format we can paste or
	{									//  is selection left up to us?
		cf0 = g_rgFETC[cFETC].cfFormat;
	    if( cf == cf0 || !cf )
		{
			// either we hit the format requested, or no format
			// was requested.  Now see if the format matches what
			// we could handle in principle.  There are three
			// basic categories:
			//		1. we are rich text and have an OLE callback;
			//		then we can handle pretty much everything.
			//		2. we are rich text but have no OLE callback.
			//		then we can handle anything but OLE specific
			//		formats.
			//		3. we are plain text only.  Then we can only
			//		handle plain text formats.

			if( (_ped->_fRich || (g_rgDOI[cFETC] & DOI_CANPASTEPLAIN)) &&
				(precall || !(g_rgDOI[cFETC] & DOI_CANPASTEOLE)))
			{
				// once we get this far, make sure the data format
				// is actually available.
				if( (pdo && pdo->QueryGetData(&g_rgFETC[cFETC]) == NOERROR ) ||
					(!pdo && IsClipboardFormatAvailable(cf0)) )
				{
					ret = TRUE;			// Return arbitrary non zero value.
					break;
				}
			}
		}
    }	

Exit:
	if( pdo )
	{
		pdo->Release();
	}
	return ret;
}

/*
 *	CLightDTEngine::RangeToDataObject (prg, lStreamFormat, ppdo)
 *
 *	@mfunc
 *		Create data object (with no OLE-formats) for the range prg
 *
 *	@rdesc
 *		HRESULT	= !ppdo ? E_INVALIDARG :
 *				  pdo ? NOERROR : E_OUTOFMEMORY
 */
HRESULT CLightDTEngine::RangeToDataObject(
	CTxtRange *		prg,			// @parm Range to get DataObject for
	LONG			lStreamFormat,	// @parm stream format to use for loading
	IDataObject **	ppdo)			// @parm Out parm for DataObject
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::RangeToDataObject");

	if(!ppdo)
		return E_INVALIDARG;

	CDataTransferObj *pdo = CDataTransferObj::Create(_ped, prg, lStreamFormat);

	*ppdo = pdo;
	return pdo ? NOERROR : E_OUTOFMEMORY;
}

/*
 *	CLightDTEngine::RenderClipboardFormat(wFmt)
 *
 *	@mfunc
 *		Renders current clipboard data object in specified format. (Ole less transfer)
 *
 *	@rdesc
 *		HRESULT
 */
HRESULT CLightDTEngine::RenderClipboardFormat(
	WPARAM wFmt)
{
	HRESULT hr = E_FAIL;

    // GuyBark Jupiter: Added utf8 handling here. The first format that the 
    // RichEdit pasting code tries is UTF8. Given that RichEdit can copy that 
    // ok to the clipboard, handle it here. We used to ignore the request for
    // utf8 here and then accept the next request which was RTF.
	if (_fOleless && (wFmt == cf_RTFUTF8 || wFmt == cf_RTF || wFmt == CF_UNICODETEXT || wFmt == CF_DIB))
	{
		Assert(_pdo);
		STGMEDIUM med;
		DWORD iFETC = iUnicodeFETC;
		if (wFmt == cf_RTF)
			iFETC = iRtfFETC;
		else if (wFmt == cf_RTFUTF8)
			iFETC = iRtfUtf8;
		else if (wFmt == CF_DIB)
			iFETC = iDIB;
		med.tymed = TYMED_HGLOBAL;
		med.pUnkForRelease = NULL;
		med.hGlobal = NULL;
		hr = _pdo->GetData(&g_rgFETC[iFETC], &med);
        if (SUCCEEDED(hr) && ::SetClipboardData(wFmt, med.hGlobal) != NULL)
        {
            hr = S_OK;
        }    
	}
	return hr;		
}

/*
 *	CLightDTEngine::RenderAllClipboardFormats(wFmt)
 *
 *	@mfunc
 *		Renders current clipboard data object (text and RTF). (Ole less transfer)
 *
 *	@rdesc
 *		HRESULT
 */
HRESULT CLightDTEngine::RenderAllClipboardFormats()
{
	HRESULT hr;
	if (_fOleless)
	{
		HWND howner = ::GetClipboardOwner();
		HWND hwnd;
		if (howner &&
			_ped->TxGetWindow(&hwnd) == NOERROR &&
			howner == hwnd &&
			::OpenClipboard(hwnd)
		)
		{
			::EmptyClipboard();
			hr = RenderClipboardFormat(cf_RTF);
			if (SUCCEEDED(hr))
			    hr = RenderClipboardFormat(CF_UNICODETEXT);
			//hr = hr || RenderClipboardFormat(CF_UNICODETEXT);
			if (SUCCEEDED(hr))
                hr = RenderClipboardFormat(CF_DIB);
			//hr = hr || RenderClipboardFormat(CF_DIB);
			::CloseClipboard();
			return hr;
		}
	}
	return S_OK;		// Pretend we did the right thing.
}

/*
 *	CLightDTEngine::DestroyClipboard()
 *
 *	@mfunc
 *		Destroys the clipboard data object
 *
 *	@rdesc
 *		HRESULT
 *
 */
HRESULT CLightDTEngine::DestroyClipboard()
{
	// Nothing to do.  This should work together with our Flush clipboard logic
	return S_OK;
}

/*
 *	CLightDTEngine::HGlobalToRange(dwFormatIndex, hGlobal, ptext, prg, publdr, bDBCString)
 *
 *	@mfunc
 *		Copies the contents of the given string (ptext) to the given range.
 *		The global memory handle may or may not point to the string depending
 *		on the format
 *
 *	@rdesc
 *		HRESULT
 */
HRESULT CLightDTEngine::HGlobalToRange(
	DWORD		dwFormatIndex,
	HGLOBAL		hGlobal,
	LPTSTR		ptext,
	CTxtRange *	prg,
	IUndoBuilder *publdr,
	BOOL		bDBCString)
{
	READHGLOBAL	rhg;
        EDITSTREAM      es;
        BOOL            fSetCur=FALSE;
        HCURSOR         hcur;

	// If RTF, wrap EDITSTREAM around hGlobal & delegate to LoadFromEs()
	if (dwFormatIndex == iRtfNoObjs || dwFormatIndex == iRtfFETC ||
		dwFormatIndex == iRtfUtf8)
	{
		Assert(hGlobal != NULL);
		rhg.ptext		= (LPSTR)ptext;			// Start at beginning
		rhg.cbLeft		= GlobalSize(hGlobal);	//  with full length
		es.dwCookie		= (DWORD)&rhg;			// The read "this" ptr
		es.dwError		= NOERROR;				// No errors yet
        es.pfnCallback  = ReadHGlobal;                  // The read method

		// DBoone: Want wait cursor to display sooner
        if (rhg.cbLeft > 1024) {
			hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
            fSetCur = TRUE;
		}

         LoadFromEs(prg, SFF_SELECTION | SF_RTF, &es, TRUE, publdr);

         if (fSetCur)
			SetCursor(hcur);

		return es.dwError;
	}

	Assert( dwFormatIndex == iRtfAsTextFETC ||
			dwFormatIndex == iAnsiFETC ||
			dwFormatIndex == iUnicodeFETC );


	prg->CleanseAndReplaceRange(-1, ptext, TRUE, publdr);
 	if(prg->_rpTX.IsAfterEOP()) 			// If new text ends
	{										//  with EOP, select
		prg->SetExtend(TRUE);				//  and	delete that
		prg->BackupCRLF();					//  EOP 
		prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);
	}
	return NOERROR;
}

/* 
 *	CLightDTEngine::DIBToRange(hGlobal, prg, publdr)
 *
 *	@mfunc
 *		Inserts dib data from the clipboard into range in the control
*
 *	@rdesc
 *		HRESULT
 *
 */
HRESULT CLightDTEngine::DIBToRange(HGLOBAL hGlobal,	CTxtRange *prg,	IUndoBuilder *	publdr)
{
	HRESULT         hresult = DV_E_FORMATETC;
	REOBJECT        reobj = { 0 };
	LPBITMAPINFO	pbmi = (LPBITMAPINFO) GlobalLock(hGlobal);
	WCHAR           ch = WCH_EMBEDDING;

	reobj.clsid = CLSID_StaticDib;
	reobj.sizel.cx =
		(LONG) _ped->_pdp->DXtoHimetricX( pbmi->bmiHeader.biWidth );
	reobj.sizel.cy =
		(LONG) _ped->_pdp->DYtoHimetricY( pbmi->bmiHeader.biHeight );
	_ped->GetClientSite(&reobj.polesite);

	COleObject *pobj = (COleObject *)reobj.polesite;
	COleObject::ImageInfo *pimageinfo = new COleObject::ImageInfo;
	if(!pimageinfo)
		{
		hresult = E_OUTOFMEMORY;
		goto leave;
		}

	pobj->SetHdata(hGlobal);
	pimageinfo->xScale = 100;
	pimageinfo->yScale = 100;
	pimageinfo->xExtGoal = reobj.sizel.cx;
	pimageinfo->yExtGoal = reobj.sizel.cy;
	pimageinfo->cBytesPerLine = 0;
	pobj->SetImageInfo(pimageinfo);
	
	if (!reobj.polesite )
	{
		goto leave;
	}

	// Put object into the edit control
	reobj.cbStruct = sizeof(REOBJECT);

    //V-GUYB: 
    // If there is currently a selection, this will get replaced with the
    // WCH_EMBEDDING character. Therefore by the time the object in inserted,
    // it will get inserted at what is currently the start of any selection,
    // NOT the end of it. So reobj.cp MUST be set to cpMin here. Calling 
    // prg->GetCp here can return what is currently end of the selection.
#ifdef NEVER
	reobj.cp = prg->GetCp();
#else
	reobj.cp = prg->GetCpMin();
#endif // NEVER

	reobj.dvaspect = DVASPECT_CONTENT;
	reobj.dwFlags = REO_RESIZABLE;

	// Since we are loading an object, it shouldn't be blank
	reobj.dwFlags &= ~REO_BLANK;

	prg->Set_iCF(-1);	
	prg->ReplaceRange(1, &ch, publdr, SELRR_IGNORE);

	hresult = _ped->GetObjectMgr()->InsertObject(reobj.cp, &reobj, NULL);
leave:
	return hresult;
}

/* 
 *	CLightDTEngine::PasteDataObjectToRange (pdo, prg, cf, rps, pubdlr, dwFlags)
 *
 *	@mfunc
 *		Inserts data from the data object pdo into the range prg. If the
 *		clipboard format cf is not NULL, that format is used; else the highest
 *		priority clipboard format is used.  In either case, any text that
 *		already existed in the range is replaced.  If pdo is NULL, the
 *		clipboard is used.
 *
 *	@rdesc
 *		HRESULT
 *
 */
HRESULT CLightDTEngine::PasteDataObjectToRange(
	IDataObject *	pdo,		// @parm Data object to paste
	CTxtRange *		prg,		// @parm Range into which to paste
	CLIPFORMAT		cf,			// @parm ClipBoard format to paste
	REPASTESPECIAL *rps,		// @parm Special paste info
	IUndoBuilder *	publdr,		// @parm Undo builder to receive antievents
	DWORD			dwFlags)	// @parm DWORD packed flags
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::PasteDataObjectToRange");

	HGLOBAL		hGlobal = NULL;
	HRESULT		hresult = DV_E_FORMATETC;
	HGLOBAL		hUnicode = NULL;
	DWORD		i;
	STGMEDIUM	medium = {0, NULL};
	IDataObject *pdoSave = pdo;
	FORMATETC *	pfetc = g_rgFETC;
	LPTSTR		ptext = NULL;
	LPRICHEDITOLECALLBACK const precall = _ped->GetRECallback();
	BOOL		fThawDisplay = FALSE;
	BOOL		bDBCString = FALSE;	// use in pasting plain text
	DWORD		dwTemp;

	if(!pdo)								// No data object: use clipboard
	{
		hresult = pOleGetClipboard(&pdo);
		if(FAILED(hresult))
		{
			// Ooops.  No Ole clipboard support
			// Need to use direct clipboard access
			HWND howner = ::GetClipboardOwner();
			HWND hwnd;
			if (howner &&
				_ped->TxGetWindow(&hwnd) == NOERROR &&
				howner == hwnd
			)
			{
				// We are cut/pasting within the same richedit instance
				// Use our cached clipboard data object
				pdo = _pdo;
				if (!pdo) {		// Som failure
					_ped->Sound();
					return hresult;
				}
				pdo->AddRef();
			}
			else
			{
				// Oh Oh We need to transfer from clipboard without data object
				// Data must be coming from another window instance
				if (_ped->TxGetWindow(&hwnd) == NOERROR &&
					::OpenClipboard(hwnd)
				)
				{
					DWORD dwFmt = iRtfUtf8;				// Try for UTF8 RTF
					_ped->_pdp->Freeze();

					// GuyBark Jupiter: 
					// Don't check for rtf here if the control is in plain text mode.
					// hGlobal is initialized to NULL above.
#ifdef PWD_JUPITER
					if(_ped->IsRich())
					{
#endif // PWD_JUPITER
					hGlobal = ::GetClipboardData(cf_RTFUTF8);
					if (hGlobal == NULL)				// Wasn't there, so
					{									//  try for RTF
						hGlobal = ::GetClipboardData(cf_RTF);
						dwFmt = iRtfFETC;
					}
#ifdef PWD_JUPITER
					}
#endif // PWD_JUPITER
					if (hGlobal == NULL)				// Wasn't there either
					{									//  so try for plain
						hGlobal = ::GetClipboardData(CF_UNICODETEXT);
						dwFmt = iUnicodeFETC;
					}
					if (hGlobal)
					{
						ptext = (LPTSTR)GlobalLock(hGlobal);
						hresult = HGlobalToRange(dwFmt, hGlobal, ptext, prg, publdr, bDBCString);
					}
					else								// hGlobal == NULL Try for bitmaps
					{
						hGlobal = ::GetClipboardData(CF_DIB);
						if (hGlobal) {
							hGlobal = DuplicateHGlobal(hGlobal);
							if (hGlobal) {
								hresult =  DIBToRange(hGlobal, prg, publdr);
							} else {
								hresult = E_OUTOFMEMORY;
							}
						}
					}
					_ped->_pdp->Thaw();
					::CloseClipboard();
				}
				if (FAILED(hresult)) {
					_ped->Sound();
				}
				return hresult;
			}
		}
	}

	// Paste an object uses the limit text calculation
	_fUseLimit = TRUE;

	if(cf == CF_TEXT)
	{
		FORMATETC fetc = {CF_UNICODETEXT, NULL, 0, -1, TYMED_NULL};
		  
		if(pdo->QueryGetData(&fetc) == S_OK)
			cf = CF_UNICODETEXT;
	}

	//Call QueryAcceptData unless caller has specified otherwise
	if (!(dwFlags & PDOR_NOQUERY) && precall)
	{
		CLIPFORMAT cfReq = cf;
		HGLOBAL hmeta = NULL;

		if (rps)
		{
			hmeta = (HGLOBAL)((rps->dwAspect == DVASPECT_ICON) ? rps->dwParam : NULL);
		}

		// Ask the callback if it likes the data object, and cfReq.

		hresult = precall->QueryAcceptData(
			pdo,
			&cfReq,
			(dwFlags & PDOR_DROP) ? RECO_DROP : RECO_PASTE,
			TRUE,
			hmeta);

		if(hresult == DATA_S_SAMEFORMATETC)
		{
			// Allow callback to return DATA_S_SAMEFORMATETC if it only
			// wants cf as passed in - we don't really care because
			// any non-zero CLIPFORMAT causes us to only accept that format.
			hresult = S_OK;
		}

		if(hresult == S_OK || hresult == E_NOTIMPL)
		{
			// Callback either liked it or didn't implement the method.
			// It may have changed the format while it was at it.
			// Treat a change of cf to zero as acceptance of the original.
			// In any event, we will try to handle it.

			// If a specific CLIPFORMAT was originally requested and the
			// callback changed it, don't accept it.
			if(cfReq && cf && (cf != cfReq))
			{
				hresult = DV_E_FORMATETC;
				goto Exit;
			}

			// If a specific CLIPFORMAT was originally requested and the
			// callback either left it alone or changed it to zero,
			// make sure we use the original.  If no CLIPFORMAT was
			// originally requested, make sure we use what came back
			// from the callback.
			if(!cf)
			{
				cf = cfReq;
			}
		}
		else
		{
			// Some success other than S_OK && DATA_S_SAMEFORMATETC.
			// The callback has handled the paste.  OR some error
			// was returned.
			goto Exit;
		}
	}

	if (_ped->TxGetReadOnly())			// Should check for range protection
	{
		hresult = E_ACCESSDENIED;
		goto Exit;
	}

	// At this point we freeze the display
	fThawDisplay = TRUE;
	_ped->_pdp->Freeze();

	if( publdr )
	{
		publdr->StopGroupTyping();
		publdr->SetNameID(UID_PASTE);
	}

	for( i = 0; i < CFETC; i++, pfetc++ )
	{
		// make sure the format is either 1.) a plain text format 
		// if we are in plain text mode or 2.) a rich text format
		// or 3.) matches the requested format.

		if( cf && cf != pfetc->cfFormat )
		{
			continue;
		}

		if( _ped->IsRich() || (g_rgDOI[i] & DOI_CANPASTEPLAIN) )
		{
			// make sure the format is available
			if( pdo->QueryGetData(pfetc) != NOERROR )
			{
				continue;
			}

			//If we have a format that uses an hGlobal get and lock it
			if(i == iRtfFETC || i == iRtfAsTextFETC || i == iAnsiFETC ||
			   i == iUnicodeFETC || i == iRtfNoObjs || i == iRtfUtf8)
			{
				if( pdo->GetData(pfetc, &medium) != NOERROR )
					continue;

                hGlobal = medium.hGlobal;
				ptext = (LPTSTR)GlobalLock(hGlobal);
				if( !ptext )
				{
					pReleaseStgMedium(&medium);

					hresult = E_OUTOFMEMORY;
					goto Exit;
				}

				// 1.0 COMPATBILITY HACK ALERT!  RichEdit 1.0 has a bit of
				// "error recovery" for parsing rtf files; if they aren't
				// valid rtf, it treats them as just plain text.
				// Unfortunately, apps like Exchange depend on this behavior,
				// i.e., they give RichEdit plain text data, but call it rich
				// text anyway.  Accordingly, we emulate 1.0 behavior here by
				// checking for an rtf signature.
				if( (i == iRtfFETC || i == iRtfNoObjs || i == iRtfUtf8) &&
					(GlobalSize(hGlobal) < cbRTFSig	||
					 memcmp(ptext, szRTFSig, cbRTFSig) != 0 ))
				{
					// uh-oh, data is not RTF, make it ANSI text
					i = iAnsiFETC;
				}
			}

			// Don't delete trail EOP in some cases
			prg->AdjustEndEOP(NONEWCHARS);

			switch(i)									
			{											
			case iRtfNoObjs:							
			case iRtfFETC:								
			case iRtfUtf8:						
                
				hresult = HGlobalToRange(i, hGlobal, ptext, prg, publdr, bDBCString);
				break;
	
			case iRtfAsTextFETC:
			case iAnsiFETC:								// ANSI plain text		

				hUnicode = TextHGlobalAtoW(hGlobal, &bDBCString);
				ptext	 = (LPTSTR)GlobalLock(hUnicode);
				if(!ptext)
				{
					hresult = E_OUTOFMEMORY;			// Unless out of RAM,
					break;								//  fall thru to
				}										//  Unicode case
														
			case iUnicodeFETC:							// Unicode plain text

				// Ok to pass in NULL for hglobal since argument won't be used
				hresult = HGlobalToRange(i, NULL, ptext, prg, publdr, bDBCString);
				if(hUnicode)							// For iAnsiFETC case
				{
					GlobalUnlock(hUnicode);
					GlobalFree(hUnicode);
				}			
				break;

			case iObtDesc:	 // Object Descriptor
				*(volatile DWORD*)&dwTemp = 0; // Work around compiler bug (#14740)
				continue;    // to search for a good format.
							 // the object descriptor hints will be used
				             // when the format is found.

			case iEmbObj:	 // Embedded Object
			case iEmbSrc:	 // Embed Source
			case iLnkSrc:	 // Link Source
			case iMfPict:	 // Metafile
			case iDIB:		 // DIB
			case iBitmap:	 // Bitmap
			case iFilename:	 // Filename
				hresult = CreateOleObjFromDataObj(pdo, prg, rps, i, publdr);
				break;

			//COMPATIBILITY ISSUE (v-richa) iTxtObj is needed by Exchange and 
			//as a flag for Wordpad.  iRichEdit doesn't seem to be needed by 
			//anyone but might consider implementing as a flag.
			case iRichEdit:	 // RichEdit
			case iTxtObj:	 // Text with Objects
				break;
			}

			//If we used the hGlobal unlock it and free it.
			if(hGlobal)
			{
				GlobalUnlock(hGlobal);
				pReleaseStgMedium(&medium);
			}
			
			//Break out of the for loop
			break;
		}
	}

Exit:

	if (fThawDisplay)
	{
		_ped->_pdp->Thaw();
	}

	if(!pdoSave)										// Release data object
		pdo->Release();									//  used for clipboard

	return hresult;						
}	

/*
 *	CLightDTEngine::GetDropTarget (ppDropTarget)
 *
 *	@mfunc
 *		creates an OLE drop target
 *
 *	@rdesc
 *		HRESULT
 *
 *	@devnote	The caller is responsible for AddRef'ing this object
 *				if appropriate.
 */
HRESULT CLightDTEngine::GetDropTarget(
	IDropTarget **ppDropTarget)		// @parm outparm for drop target
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::GetDropTarget");

	if( !_pdt )
	{
		_pdt = new CDropTarget(_ped);
		// the AddRef done by the constructor will be
		// undone by the destructor of this object
	}

	if( ppDropTarget )
	{
		*ppDropTarget = _pdt;
	}

	return _pdt ? NOERROR : E_OUTOFMEMORY;
}

/*
 *	CLightDTEngine::StartDrag (prg, publdr)
 *
 *	@mfunc
 *		starts the main drag drop loop
 *
 */	
HRESULT CLightDTEngine::StartDrag(
	CTxtSelection *psel,		// @parm Selection to drag from
	IUndoBuilder *publdr)		// @parm undo builder to receive antievents
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::StartDrag");

	LONG			cch, cch1;
	LONG			cp1, cpMin, cpMost;
	DWORD			dwEffect = 0;
	HRESULT			hr;
	IDataObject *	pdo = NULL;
	IDropSource *	pds;
	IRichEditOleCallback * precall = _ped->GetRECallback();

	// If we're doing drag drop's, we should have our own drop target
	// It's possible that _pdt will be NULL at this point--some clients
	// will delay instantiation of our drop target until a drop target
	// in the parent window decides that ours is needed.  However, since
	// we need it just to initiate drag drop, go ahead and create one
	// here.

	if( _pdt == NULL )
	{
		if( (hr = GetDropTarget(NULL)) != NOERROR )
		{
			return hr;
		}
	}

	psel->CheckTableSelection();

	if(precall)
	{
		CHARRANGE chrg;

		// give the callback a chance to give us its own IDataObject
		psel->GetRange(chrg.cpMin, chrg.cpMost);
		hr = precall->GetClipboardData(&chrg, RECO_COPY, &pdo);
	}
	else
	{
		// we need to build our own data object.
		hr = S_FALSE;
	}

	// If we didn't get an IDataObject from the callback, build our own
	if(hr != NOERROR || pdo == NULL)
	{										// Don't include trailing EOP
		psel->AdjustEndEOP(NONEWCHARS);		//  in some selection cases
		hr = RangeToDataObject(psel, SF_TEXT | SF_RTF, &pdo);
		if(hr != NOERROR)
			return hr;
	}

	cch = psel->GetRange(cpMin, cpMost);	// NB: prg is the selection
	cp1 = psel->GetCp();					// Save active end and signed
	cch1 = psel->GetCch();					//  length for Undo antievent
	CTxtRange rg(_ped, cpMost, cch);		// Use range copy to float over
											// mods made to backing store
	// The floating range that we just created on the stack needs to
	// think that it's protected, so it won't change size.
	rg.SetDragProtection(TRUE);

	pds = new CDropSource();
	if( pds == NULL )
	{
		pdo->Release();
		return E_OUTOFMEMORY;
	}

	// cache some info with our own drop target
	_pdt->SetDragInfo(publdr, cpMin, cpMost);


	// Set allowable effects
	dwEffect = DROPEFFECT_COPY;
	if (!_ped->TxGetReadOnly())
		dwEffect |= DROPEFFECT_MOVE;
	
	// Let the client decide what it wants.
	if (precall)
	{
		hr = precall->GetDragDropEffect(TRUE, 0, &dwEffect);
	}

	if (!FAILED(hr) || hr == E_NOTIMPL)
	{
		// start drag-drop operation
		hr = pDoDragDrop(pdo, pds, dwEffect, &dwEffect);
	}

	// clear drop target
	_pdt->SetDragInfo(NULL, -1, -1);

	// handle 'move' operations	
	if( hr == DRAGDROP_S_DROP && (dwEffect & DROPEFFECT_MOVE) )
	{
		// we're going to delete the dragged range, so turn off protection.

		rg.SetDragProtection(FALSE);

		if( publdr )
		{
			LONG cpNext, cchNext;

			if(_ped->GetCallMgr()->GetChangeEvent() )
			{
				cpNext = cchNext = -1;
			}
			else
			{
				cpNext = rg.GetCpMin();
				cchNext = 0;
			}

			HandleSelectionAEInfo(_ped, publdr, cp1, cch1, cpNext, cchNext,
								  SELAE_FORCEREPLACE);
		}
		
		// delete the data that was moved.  The selection will float
		// to the new correct location.
		rg.ReplaceRange(0, NULL, publdr, SELRR_IGNORE);

		// The update that happens implicitly by the update of the range may
		// have the effect of scrolling the window. This in turn may have the
		// effect in the drag drop case of scrolling non-inverted text into
		// the place where the selection was. The logic in the selection 
		// assumes that the selection is inverted and so reinverts it to turn
		// off the selection. Of course, it is obvious what happens in the
		// case where non-inverted text is scrolled into the selection area.
		// To simplify the processing here, we just say the whole window is
		// invalid so we are guaranteed to get the right painting for the
		// selection.
		// FUTURE: (ricksa) This solution does have the disadvantage of causing
		// a flash during drag and drop. We probably want to come back and
		// investigate a better way to update the screen.
		_ped->TxInvalidateRect(NULL, FALSE);

		// Display is updated via notification from the range

		// Update the caret
		psel->Update(TRUE);
	}
	else if( hr == DRAGDROP_S_DROP && _ped->GetCallMgr()->GetChangeEvent() &&
		(dwEffect & DROPEFFECT_COPY) && publdr)
	{
		// if we copied to ourselves, we want to restore the selection to
		// the original drag origin on undo
		HandleSelectionAEInfo(_ped, publdr, cp1, cch1, -1, -1, 
				SELAE_FORCEREPLACE);
	}


	if( SUCCEEDED(hr) )
	{	
		hr = NOERROR;
	}

	pdo->Release();
	pds->Release();

	// we do this last since we may have re-used some 'paste' code which
	// will stomp the undo name to be UID_PASTE.
	if( publdr )
	{
		publdr->SetNameID(UID_DRAGDROP);
	}

	if( (_ped->GetEventMask() & ENM_DRAGDROPDONE) )
	{
		NMHDR	hdr;

		ZeroMemory(&hdr, sizeof(NMHDR));
		_ped->TxNotify(EN_DRAGDROPDONE, &hdr);
	}

	return hr;
#else
	return 0;
#endif
}

/*
 *	CLightDTEngine::LoadFromEs (prg, lStreamFormat, pes, fTestLimit, publdr)
 *
 *	@mfunc
 *		Load data from the stream pes into the range prg according to the
 *		format lStreamFormat
 *
 *	@rdesc
 *		LONG -- count of characters read
 */
LONG CLightDTEngine::LoadFromEs(
	CTxtRange *	prg,			// @parm range to load into
	LONG		lStreamFormat,	// @parm stream format to use for loading
	EDITSTREAM *pes,			// @parm edit stream to load from
	BOOL		fTestLimit,		// @parm Whether to test text limit
	IUndoBuilder *publdr)		// @parm undo builder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::LoadFromEs");

#ifdef DEBUG
	// FUTURE: Currently freezing the display prior to loading text
	// is simply an optimization. This may become a requirement in the
	// future. If this does become a requirement then we'll want to
	// exit with an error.
	if( !_ped->_pdp->IsFrozen() )
	{
		TRACEWARNSZ("CLightDTEngine::LoadFromEs	display not frozen");
	}
#endif // DEBUG

	LONG		cch = 0;				// Default no chars read
	CTxtSelection *psel;
	IAntiEvent *pae = NULL;

	if( publdr )
	{
		publdr->StopGroupTyping();
	}

	_ped->CheckUnicode(lStreamFormat);			// If Unicode, set code page
												//  1200
	// other components, such as the display and backing store, will
	// be able to make optimizations if they know that we are streaming
	// in text or RTF data.

	_ped->SetStreaming(TRUE);

	if( lStreamFormat & SF_RTF )					// RTF case must precede
	{												//  TEXT case (see SF_x
		if( !_ped->IsRich() )						//  values)
		{
			Assert(cch == 0);
			goto Exit;
		}

		LONG cpMin, cpMost;

		// Here we do something a bit unusual for performance reasons.
		// instead of letting the rtf reader generate it's own undo actions,
		// we'll take care of it ourselves.  Instead of generating actions
		// for each little operation, we simply generate a "big" anti-event
		// for the whole shebang

		// There is a subtlty w.r.t. to paragraph format runs.  By inserting
		// text with para formatting, it's possible that we will modify the
		// para formatting of the _current_ paragraph.  Thus, it's necessary
		// to remember what the formatting currently is for undo.  Note that
		// it may actually not be changed; but we go ahead and generate an
		// anti-event anyways.  Note that we only need to do this if cpMin is
		// the middle of a paragraph
		
		CTxtPtr tp(prg->_rpTX);
		if(prg->GetCch() > 0)
			tp.AdvanceCp(-prg->GetCch());
		
		if( publdr && prg->_rpPF.IsValid() && !tp.IsAfterEOP() )
		{
			IParaFormatCache *ppfc;
			GetParaFormatCache(&ppfc);

			tp.FindEOP(tomBackward);
			cpMin = tp.GetCp();
			tp.FindEOP(tomForward);
			cpMost = tp.GetCp();
			
			// We must be in rich text mode, so we must be able to always
			// find a paragraph.
			Assert(cpMost > cpMin);

			CFormatRunPtr rpPF(prg->_rpPF);
			rpPF.AdvanceCp(cpMin - prg->GetCp());
			
			pae = gAEDispenser.CreateReplaceFormattingAE( _ped, rpPF, 
						cpMost - cpMin, ppfc, ParaFormat);
			if( pae )
				publdr->AddAntiEvent(pae);
		} 

		// First, clear the range
		if( prg->GetCch() )
			prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);

		Assert(prg->GetCch() == 0);

		cpMin = prg->GetCp();
		CRTFRead rtfRead(prg, pes, lStreamFormat);
	
		cch	= rtfRead.ReadRtf();

		if (lStreamFormat & SFF_ADJUSTENDEOP &&		// If range end EOP
			pes->dwError == NOERROR &&				//  wasn't deleted and
			prg->_rpTX.IsAfterEOP())	 			//  new text ends with
		{											//  an EOP,
			prg->SetExtend(TRUE);					//  select and delete
			prg->BackupCRLF();						//  the latter
			prg->ReplaceRange(0, NULL, NULL, SELRR_IGNORE);
		}

		cpMost = prg->GetCp();

		Assert(cpMost >= cpMin);

		// If nothing changed, get rid of any anti-events (like the formatting
		// one) that we may have "speculatively" added

		if( publdr && !_ped->GetCallMgr()->GetChangeEvent() )
		{
			publdr->Discard();
		}

		if( publdr && cpMost > cpMin )
		{
			// If some text was added, create an anti-event for
			// it and add it in.

			AssertSz(_ped->GetCallMgr()->GetChangeEvent(),
				"Something changed, but nobody set the change flag");
				
			pae = gAEDispenser.CreateReplaceRangeAE(_ped, cpMin, cpMost, 0, 
						NULL, NULL, NULL);

			HandleSelectionAEInfo(_ped, publdr, -1, -1, cpMost, 0, 
						SELAE_FORCEREPLACE);
			if( pae )
				publdr->AddAntiEvent(pae);
		}
	}
	else if( lStreamFormat & SF_TEXT )
	{
		cch = ReadPlainText( prg, pes, fTestLimit, publdr );
	}

	// Before updating the selection, try the auto-URL detect.  This makes
	// two cases better: 1. a long drag drop is now faster and 2. the
	// selection _iFormat will now be udpated correctly for cases of
	// copy/paste of a URL.

	if( _ped->GetDetectURL() )
	{
		_ped->GetDetectURL()->ScanAndUpdate(publdr);
	}

	// The caret belongs in one of two places:
	//		1. if we loaded into a selection, at the end of the new text
	//		2. otherwise, we loaded an entire document, set it to cp 0
	//
	// ReadPlainText() and ReadRtf() set prg to an insertion point
	// at the end, so if we loaded a whole document, reset it.

	if( (psel = _ped->GetSel()) )
	{
		if( !(lStreamFormat & SFF_SELECTION) )
		{
			psel->Set(0,0);
		}
		psel->Update_iFormat(-1);
	}

Exit:

	_ped->SetStreaming(FALSE);

	if (!fTestLimit)
	{
		// If we don't limit the text then we adjust the text limit
		// if we have exceeded it.
		_ped->TxSetMaxToMaxText();
	}

	return cch;
}

/*
 *	CLightDTEngine::SaveToEs (prg, lStreamFormat, pes)
 *
 *	@mfunc
 *		save data into the given stream
 *
 *	@rdesc
 *		LONG -- count of characters written
 */
LONG CLightDTEngine::SaveToEs(
	CTxtRange *	prg,			// @parm range to drag from
	LONG		lStreamFormat,	// @parm stream format to use for saving
	EDITSTREAM *pes )			// @parm edit stream to save to
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::SaveToEs");

	LONG cch = 0;								// Default no chars written

	_ped->CheckUnicode(lStreamFormat);			// If Unicode, set code page

	if( lStreamFormat & SF_RTF )				// Be sure to check for SF_RTF
	{											//  before checking for SF_TEXT
		CRTFWrite rtfWrite( prg, pes, lStreamFormat );
	
		cch = rtfWrite.WriteRtf();
	}
	else if(lStreamFormat & (SF_TEXT | SF_TEXTIZED))
	{
		cch = WritePlainText(prg, pes, lStreamFormat);
	}
	else
	{
		Assert(FALSE);
	}

	return cch;
}

/*
 *	CLightDTEngine::UnicodePlainTextFromRange (prg)
 *
 *	@mfunc
 *		fetch the plain text from a range and puts it in an hglobal
 *
 *	@rdesc
 *		an allocated HGLOBAL.
 *
 *	@devnote
 *		FUTURE: Export bullets as does Word for plain text
 */
HGLOBAL CLightDTEngine::UnicodePlainTextFromRange(
	CTxtRange *prg)				// @parm range to get text from
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::UnicodePlainTextFromRange");

	LONG	cpMin, cpMost;
	LONG	cch = prg->GetRange(cpMin, cpMost);
	LONG	cchT = 2*(cch + 1);
	HGLOBAL	hText;
	HGLOBAL	hTextNew;
	TCHAR *	pText;
	CTxtPtr tp(_ped, cpMin);

	hText = GlobalAlloc(GMEM_FIXED,						// Allocate 2* in
						cchT * sizeof(TCHAR) );			//  case all CRs
	if( !hText )
		return NULL;

	pText = (TCHAR *)GlobalLock(hText);
	if( !pText || 0 == GlobalSize(hText))
		return NULL;

	if( cch )
	{
		cch = tp.GetPlainText(cchT, pText, cpMost, FALSE);
		AssertSz(cch <= cchT,
			"CLightDTEngine::UnicodePlainTextFromRange: got too much text");
	}

	*(pText + cch) = '\0';

	// GuyBark Jupiter: Unlock before the realloc, (even though the lock 
	// didn't do anything anyway), and beware of oom.
	GlobalUnlock(hText);

	if(!(hTextNew = GlobalReAlloc(hText, 2*(cch + 1), GMEM_MOVEABLE)))
	{
	    GlobalFree(hText);
	}

	hText = hTextNew;

	return hText;
}

/*
 *	CLightDTEngine::AnsiPlainTextFromRange (prg)
 *
 *	@mfunc
 *		Retrieve an ANSI copy of the text in the range prg
 *
 *	@rdesc
 *		HRESULT
 */
HGLOBAL CLightDTEngine::AnsiPlainTextFromRange(
	CTxtRange *prg)				// @parm range to get text from
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::AnsiPlainTextFromRange");

	HGLOBAL hUnicode;
	HGLOBAL hAnsi;

	// FUTURE (alexgo): if we implement the option to store text as 8-bit
	// chars, then we can make this routine more efficient

	hUnicode = UnicodePlainTextFromRange(prg);
	hAnsi = TextHGlobalWtoA(hUnicode);

	GlobalFree(hUnicode);
	return hAnsi;
}

/*
 *	CLightDTEngine::RtfFromRange (prg, lStreamFormat)
 *
 *	@mfunc
 *		Fetch RTF text from a range and put it in an hglobal
 *
 *	@rdesc
 *		an allocated HGLOBAL.  
 */
HGLOBAL CLightDTEngine::RtfFromRange(
	CTxtRange *	prg,			// @parm Range to get RTF from
	LONG 		lStreamFormat)	// @parm stream format to use for loading
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::RtfFromRange");

	WRITEHGLOBAL whg;
	EDITSTREAM	 es = {(DWORD)&whg, NOERROR, WriteHGlobal};
	DWORD		 cb	= 2*abs(prg->GetCch()) + 100;	// Rough estimate
	HGLOBAL      hgNew;
 
	whg.cb			= cb;
	whg.hglobal		= GlobalAlloc(GMEM_FIXED, cb);
	if(!whg.hglobal)
		return NULL;		
	whg.cch			= 0;					// Nothing written yet
	SaveToEs(prg, lStreamFormat & ~SF_TEXT, &es);
	if(es.dwError)
	{
		GlobalFree(whg.hglobal);
		return NULL;
	}

	// GuyBark Jupiter: Don't leak here.

	if(!(hgNew = GlobalReAlloc(whg.hglobal, whg.cch, GMEM_MOVEABLE)))
	{
	    GlobalFree(whg.hglobal);
	}

	whg.hglobal = hgNew;

    return whg.hglobal;
}


//
// PROTECTED METHODS
//

#define READSIZE 	4096 - 2
#define WRITESIZE	2048

/*
 *	CLightDTEngine::ReadPlainText (prg, pes, publdr)
 *
 *	@mfunc
 *		Replaces contents of the range prg with the data given in the edit
 *		stream pes. Handles multibyte sequences that overlap stream buffers.
 *
 *	@rdesc
 *		Count of bytes read (to be compatible with RichEdit 1.0)
 *
 *	@devnote
 *		prg is modified; at the return of the call, it will be a degenerate
 *		range at the end of the read in text.
 *
 *		Three kinds of multibyte/char sequences can overlap stream buffers:
 *		DBCS, UTF-8, and CRLF/CRCRLF combinations. DBCS and UTF-8 streams are
 *		converted by MultiByteToWideChar(), which cannot convert a lead byte
 *		(DBCS and UTF-8) that occurs at the end of the buffer, since the
 *		corresponding trail byte(s) will be in the next buffer.  Similarly,
 *		in RichEdit 2.0 mode, we convert CRLFs to CRs and CRCRLFs to blanks,
 *		so one or two CRs at the end of the buffer require knowledge of the
 *		following char to determine if they are part of a CRLF or CRCRLF.
 *
 *		To handle these overlapped buffer cases, we move the ambiguous chars
 *		to the start of the next buffer, rather than keeping them as part of
 *		the current buffer.  At the start of the buffer, the extra char(s)
 *		needed for translation follow immediately.
 */
LONG CLightDTEngine::ReadPlainText(
	CTxtRange *	  prg, 			// @parm range to read to
	EDITSTREAM *  pes,			// @parm edit stream to read from
	BOOL		  fTestLimit,	// @parm whether limit testing is needed
	IUndoBuilder *publdr )		// @parm undo builder to receive antievents
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::ReadPlainText");

	CTxtEdit *ped = _ped;
	LONG	  cbRead;
	LONG 	  cbReadTotal = 0;	// No bytes read yet
	LONG	  cch;
	LONG	  cchConv;
	LONG	  cchLen;
	DWORD	  cchMax = ped->TxGetMaxLength();
	LONG	  cCR = 0;			// Count of CRs from preceding buffer
	LONG	  cCRPrev = 0;		// Count used while calc'ing new cCR
	BOOL	  fContinue = TRUE;	// Keep reading so long as TRUE
	BYTE *	  pb;				// Byte ptr to szBuf or wszBuf
	TCHAR *	  pch;				// Ptr to wszBuf
	UINT	  uCpg = CP_ACP;	// Default system Ansi code page
	BOOL	  bCheckFont = (OnWin95FE() || OnWinNTFE());		// check DB font on FE systems

	// just put a big buffer on the stack.  Thankfully, we only
	// run on 32bit OS's.  4K is a good read size for NT file caching.
	char 	szBuf[READSIZE];
	WCHAR	wszBuf[READSIZE+2];	// Allow for moving end CRs to start

	// first, empty the range
	if( prg->GetCch() )
	{
		// Delete text in range
		prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);			
	}

	if(ped->_pDocInfo && 
		ped->_pDocInfo->wCpg != tomInvalidCpg)		// Update code page if
	{												//  defined
		uCpg = ped->_pDocInfo->wCpg;
	}

	pb = (uCpg == 1200) ? (BYTE *)(wszBuf + 2)		// Setup Unicode or MBCS
						: (BYTE *)szBuf;
	LONG j = 0;										// Haven't read anything,
													//  so no lead byte left
	while(fContinue)								//  from previous read
	{
		LONG	prevChar = j;						// Save byte(s) left over
													//  from previous read
		pes->dwError = (*pes->pfnCallback)(			// Read next bufferful,
				pes->dwCookie, pb + j, 				//  bypassing any lead
				READSIZE - j, &cbRead);				//  bytes

		if(pes->dwError || !cbRead && !cCR)
			break;									// Error or done

		// adjust cbRead with previous leading byte(s)
		cbRead += j;

		j = 0;										
		
		cchConv = cbRead/2;							// Default Unicode cch
		if(uCpg != 1200 && cbRead)					// Multibyte of some kind
		{
			Assert(pb == (BYTE *)szBuf);			// Just in case...

			BOOL	bCopyTwo = FALSE;

			// check the last byte if it is a leading byte
			if (uCpg == CP_UTF8)
			{
				// Note: UTF-7 can be in the middle of a long sequence, so
				// it can't be converted effectively in chunks
				LONG	cbLocalRead = cbRead - 1;
				while((BYTE)szBuf[cbLocalRead -j] > 127)	// Find UTF-8 lead byte
				{
					j++;
					if((BYTE)szBuf[cbLocalRead -j] > 0xC0)
						break;						// Break on UTF-8 lead 
				}									//  byte
				if(j > 1)
				{
					if (j == 3 || (BYTE)			// Three-byte char or
						szBuf[cbLocalRead - 1] < 0xE0)	//  on second byte of 2
					{								//  byte char:
						j = 0;						// Finished full char
					}
					else							// Landed on first trail
					{								//  byte of 3-byte char
						bCopyTwo = TRUE;			// So copy last two bytes
					}								//  down to buf start
				}
			}
			else 
			{
				LONG temp = cbRead - 1; 

				// IsDBCSLeadByte can return TRUE for some trail bytes
				// esp. for GBX.  So, we need to keep on checking until
				// we hit a non-lead byte character.  Then, based on
				// how many bytes we went back, we can determine if the
				// last byte is really a Lead byte.
				while (temp && IsDBCSLeadByte((BYTE)szBuf[temp]))
					temp--;

				if (temp && ((cbRead-1-temp) & 1))
					j = 1;
			}

			// we don't want to pass the Lead byte or partial UTF-7 to MultiByteToWideChar
			// because it will return bad char.
		    cchConv = MultiByteToWideChar(uCpg,	0,
				szBuf, cbRead - j, &wszBuf[2], READSIZE);

            // Prefix warns here, but I don't think it can happen
            // but the check doesn't hurt
            if (cbRead < 0)
            {
			    Assert(cbRead >= 0);	
			    return 0;
		    }
			    

			// On FE System, check if we have DBC in the text
			if (bCheckFont)
			{				
				if (cchConv > 0 && cchConv < cbRead - j)
				{
					// Once we setup the DB Font, no need to do it again
					// for future buffer.
					bCheckFont = FALSE;
					PasteSetupDBCFont ( ped );
				}
			}

			szBuf[0] = szBuf[--cbRead];				// Copy last char read
													//  down to buffer start
			if(bCopyTwo)							// UTF-8. Handle breaks
			{										//  in middle of char
				szBuf[1] = szBuf[0];				// So copy last two bytes
				szBuf[0] = szBuf[cbRead-1];			//  down to buf start
			}										// If last char in buffer
		}
		cbReadTotal += cbRead - j - prevChar;

		// Cleanse (CRLFs -> CRs, etc.), limit, and insert the data. Have
		// to handle CRLFs and CRCRLFs that overlap two successive buffers.
		// This code is similar to CTxtRange::CleanseAndReplaceRange(), but
		// deals with a series of stream chunks instead of a single string.
		Assert(cCR <= 2);
		pch = &wszBuf[2 - cCR];						// Include CRs from prev

		if(!ped->TxGetMultiLine())					// Single-line control
		{											// Truncate at 1st EOP to
			for(cch = 0; cch < cchConv &&			//  be compatible with RE
				!IsASCIIEOP(*pch++);				//  1.0 and user's SLE,
				cch++)								//  and to give consistent
				;									//  display behavior
			if(cch < cchConv)						// Stop reading stream if
				fContinue = FALSE;					//  EOP encountered 
			cchConv = cch;							
			pch = &wszBuf[2];						// Restore pch
		}
		else if(!ped->Get10Mode())					// Unless RE 1.0 mode,
		{											//  convert to simple EOPs
			wszBuf[0] = wszBuf[1] = CR;				// Store CRs for cchCR > 0
			cCRPrev = cCR;							// Save prev cchCR
			cCR = 0;								// Default no CR this buf

			Assert((LONG)(sizeof(wszBuf)/sizeof(WCHAR)) >= cchConv + 2);

			// Need to +2 since we are moving data into wszBuf[2]

			// GuyBark Jupiter 37651:
			// If the text stream ended on a CR, (ie not CRLF), then we'd loop
			// forever. This has been fixed in the newest RichEdit by adding
			// cchConv to the checks below. So do the same thing here.
			if(cchConv && wszBuf[cchConv + 2 - 1] == CR)		// There's at least one
			{										// Set it up for next buf
				cCR++;								//  in case CR of CRLF
				if((cchConv > 1) && wszBuf[cchConv + 2 - 2] == CR)	// Got 2nd CR; might be
					cCR++;							//  first CR of CRCRLF so
			}										//  setup for next buffer
			cchConv += cCRPrev - cCR;				// Add in count from prev
			cchConv = Cleanse(pch, pch, cchConv);	//  and sub that for next
		}
		Assert(!prg->GetCch());						// Range is IP
		cch = cchConv;

		if (fTestLimit)
		{
			// Have to adjust size because it is 
			// changed in this loop.
			cchLen = ped->GetAdjustedTextLength();

			if(cch && (DWORD)(cch + ped->GetAdjustedTextLength()) > cchMax) // DWORD takes care of
			{										//  "negative" cchMax's
				cch = cchMax - cchLen;				//  which are considered
				cch = max(cch, 0);					//  "infinite", i.e., if
				ped->GetCallMgr()->SetMaxText();	//  will fail with such
			}										// (Can't edit > 2G files)
		}

		if(prg->ReplaceRange(cch, pch, publdr, SELRR_REMEMBERRANGE) < cchConv)
		{
			// Out of memory or reached the max size of our text control.
			// In either case, return STG_E_MEDIUMFULL (for compatibility
			// with RichEdit 1.0)
			pes->dwError = (DWORD)STG_E_MEDIUMFULL;
			break;
		}
	}
	return cbReadTotal;
}

/*
 *	CLightDTEngine::WritePlainText (prg, pes, fTextize)
 *
 *	@mfunc
 *		Writes plain text from the range into the given edit stream
 *
 *	@rdesc
 *		Count of bytes written
 */
LONG CLightDTEngine::WritePlainText(
	CTxtRange *	prg,			// @parm range to write from
	EDITSTREAM *pes,			// @parm edit stream to write to
	LONG		lStreamFormat)	// @parm Stream format
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::WritePlainText");

	LONG		cbConverted;		// Bytes for output stream
	LONG		cbWrite;			// Incremental byte count
	LONG		cbWriteTotal = 0;	// No chars written yet
	LONG		cpMin, cpMost;
	LONG		cch = prg->GetRange(cpMin, cpMost);
	BOOL		fTextize = lStreamFormat & SF_TEXTIZED;
	LPBYTE		pb;					// Byte ptr to szBuf or wszBuf
	COleObject *pobj;				// Ptr to embedded object
	CTxtPtr		tp(_ped, cpMin);	// tp to walk prg with
	UINT		uCpg = CP_ACP;		// Default system Ansi code page

	// DBCS has up to 2 times as many chars as WCHARs. UTF-8 has 3 BYTES for
	// all codes above 0x7ff. UTF-7 has even more due to shift in/out codes.
	// We don't support UTF-7, since can't use WCTMB with UTF-7 chunks

	char		szBuf[3*WRITESIZE];	// Factor of 2 works with DBCS, 3 with UTF-8
	WCHAR		wszBuf[WRITESIZE];

	pes->dwError = NOERROR;							// No error yet

	if(lStreamFormat & SFF_UTF8)
		uCpg = CP_UTF8;

	else if(_ped->_pDocInfo && 
		_ped->_pDocInfo->wCpg != tomInvalidCpg)		// Update code page if
	{												//  defined
		uCpg = _ped->_pDocInfo->wCpg;
	}

	pb = (uCpg == 1200) ? (BYTE *)wszBuf			// Setup Unicode or MBCS
						: (BYTE *)szBuf;

	LONG cchText = _ped->GetAdjustedTextLength();
	cpMost = min(cpMost, cchText);					// Don't write final CR
	while((LONG)tp.GetCp() < cpMost)
	{
		if (fTextize && tp.GetChar() == WCH_EMBEDDING)
		{
			Assert(_ped->GetObjectCount());

			pobj = _ped->GetObjectMgr()->GetObjectFromCp(tp.GetCp());
			tp.AdvanceCp(1);						// Advance past object
			if(pobj)
			{
				cbWriteTotal += pobj->WriteTextInfoToEditStream(pes);
				continue;							// If no object at cp,
			}										//  just ignore char
		}											
		cch	= tp.GetPlainText(WRITESIZE, wszBuf, cpMost, fTextize);
		if(!cch)
			break;									// No more to do

		cbConverted = 2*cch;						// Default Unicode byte ct
		if(uCpg != 1200)							// Multibyte of some kind
		{
			cbConverted = MbcsFromUnicode(szBuf, 3*WRITESIZE, wszBuf, cch, uCpg,
								UN_CONVERT_WCH_EMBEDDING);

			// FUTURE: report some kind of error if default char used,
			// i.e., data lost in conversion
		
			// Did the conversion completely fail? As a fallback, we might try 
			// the system code page, or just plain ANSI...
		
			if (!cbConverted)
			{
				uCpg = GetLocaleCodePage();
				cbConverted = MbcsFromUnicode(szBuf, 3*WRITESIZE, wszBuf, cch, uCpg,
												UN_CONVERT_WCH_EMBEDDING);
			}

			if (!cbConverted)
			{
				uCpg = CP_ACP;
				cbConverted = MbcsFromUnicode(szBuf, 3*WRITESIZE, wszBuf, cch, uCpg,
												UN_CONVERT_WCH_EMBEDDING);
			}
		}

		pes->dwError = (*pes->pfnCallback)(pes->dwCookie, pb,
							cbConverted,  &cbWrite);
		if(!pes->dwError && cbConverted != cbWrite)	// Error or ran out of
			pes->dwError = (DWORD)STG_E_MEDIUMFULL;	//  target storage

		if(pes->dwError)
			break;
		cbWriteTotal += cbWrite;
	}

	AssertSz((LONG)tp.GetCp() >= cpMost,
		"CLightDTEngine::WritePlainText: not all text written");

	return cbWriteTotal;
}

/* 
 *	CLightDTEngine::CreateOleObjFromDataObj ( pdo, prg, rps, pubdlr )
 *
 *	@mfunc
 *		Creates an ole object based on the data object pdo, and
 *		pastes the object into the range prg. Any text that already
 *		existed in the range is replaced.
 *
 *	@rdesc
 *		HRESULT
 *
 */
HRESULT CLightDTEngine::CreateOleObjFromDataObj(
	IDataObject *	pdo,		// @parm Data object from which to create
	CTxtRange *		prg,		// @parm Range in which to place
	REPASTESPECIAL *rps,		// @parm Special paste info
	INT				iformatetc,	// @parm Index in g_rgFETC 
	IUndoBuilder *	publdr)		// @parm Undo builder to receive antievents
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CreateOleObjFromDataObj");

	HRESULT			hr = NOERROR;
	REOBJECT		reobj;
	SIZEL			sizel;
	FORMATETC		formatetc;
	DWORD			dwDrawAspect = 0;
	HGLOBAL			hMetaPict = NULL;
	LPRICHEDITOLECALLBACK const precall = _ped->GetRECallback();
	LPOBJECTDESCRIPTOR lpod = NULL;
	STGMEDIUM		medObjDesc;
	BOOL			fStatic = (iformatetc == iMfPict || iformatetc == iDIB ||
							   iformatetc == iBitmap);
	BOOL			fFilename = (iformatetc == iFilename);
    DUAL_FORMATETC	tmpFormatEtc;

	if(!precall)
	{
		return E_NOINTERFACE;
	}

	ZeroMemory(&medObjDesc, sizeof(STGMEDIUM));
	ZeroMemory(&sizel, sizeof(SIZEL));
	ZeroMemory(&reobj, sizeof(REOBJECT));

	if(fStatic)
	{
		dwDrawAspect = DVASPECT_CONTENT;
	}

	if(fFilename)
	{
		dwDrawAspect = DVASPECT_ICON;
	}

	if(rps && !dwDrawAspect)
	{
		dwDrawAspect = rps->dwAspect;
		if(rps->dwAspect == DVASPECT_ICON)
			hMetaPict = (HGLOBAL)rps->dwParam;
	}

	// If no aspect was specified, pick up the object descriptor hints
	if(!dwDrawAspect)
	{
		// Define ObjectDescriptor data
		formatetc.cfFormat = cf_OBJECTDESCRIPTOR;
		formatetc.ptd = NULL;
		formatetc.dwAspect = DVASPECT_CONTENT;
		formatetc.lindex = -1;
		formatetc.tymed = TYMED_HGLOBAL;

		if(pdo->GetData(&formatetc, &medObjDesc) == NOERROR)
		{
			HANDLE			hGlobal;

			hGlobal = medObjDesc.hGlobal;

			lpod = (LPOBJECTDESCRIPTOR)GlobalLock(hGlobal);
			if(lpod)
			{
				dwDrawAspect = lpod->dwDrawAspect;
			}
			GlobalUnlock(hGlobal);
			pReleaseStgMedium(&medObjDesc);
		}
	}

	if(!dwDrawAspect)
	{
		dwDrawAspect = DVASPECT_CONTENT;
	}

	if(fStatic)
	{
		reobj.clsid	= ((iformatetc == iMfPict) ?
			CLSID_StaticMetafile : CLSID_StaticDib);
	}

	// COMPATIBILITY ISSUE: Compatibility Issue from Richedit 1.0 - Raid 16456: 
	// Don't call GetData(CF_EMBEDSOURCE)
	// on 32-bit Excel. Also clsidPictPub.
	//	if(iformatetc == iformatetcEmbSrc && (ObFIsExcel(&clsid) || 
	//		IsEqualCLSID(&clsid, &clsidPictPub)))
	//	else
	//		ObGetStgFromDataObj(pdataobj, &medEmbed, iformatetc);

	// Get storage for the object from the application
	hr = precall->GetNewStorage(&reobj.pstg);
	if(hr)
	{
		TRACEERRORSZ("GetNewStorage() failed.");
		goto err;
	}

	// Create an object site for the new object
	_ped->GetClientSite(&reobj.polesite);
	if(hr)
	{
		TRACEERRORSZ("GetClientSite() failed.");
		goto err;
	}


	ZeroMemory(&tmpFormatEtc, sizeof(DUAL_FORMATETC));
	tmpFormatEtc.ptd = NULL;
	tmpFormatEtc.dwAspect = dwDrawAspect;
	tmpFormatEtc.lindex = -1;

	//Create the object
	if(fStatic)
	{
		hr = pOleCreateStaticFromData(pdo, IID_IOleObject, OLERENDER_DRAW,
				&tmpFormatEtc, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);
	}
	else if(iformatetc == iLnkSrc)
	{
		hr = pOleCreateLinkFromData(pdo, IID_IOleObject, OLERENDER_DRAW,
				&tmpFormatEtc, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);
	}
	else
	{
		hr = pOleCreateFromData(pdo, IID_IOleObject, OLERENDER_DRAW,
				&tmpFormatEtc, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);
	}

	if(hr)
	{
		TRACEERRORSZ("Failure creating object.");
		goto err;
	}


	//Get the clsid of the object.
	if(!fStatic)
	{
		hr = reobj.poleobj->GetUserClassID(&reobj.clsid);
		if(hr)
		{
			TRACEERRORSZ("GetUserClassID() failed.");
			goto err;
		}
	}

	//Deal with iconic aspect if specified.
	if(hMetaPict)
	{
		BOOL fUpdate;

		hr = OleStdSwitchDisplayAspect(reobj.poleobj, &dwDrawAspect,
										DVASPECT_ICON, hMetaPict, FALSE,
										FALSE, NULL, &fUpdate);
		if(hr)
		{
			TRACEERRORSZ("OleStdSwitchDisplayAspect() failed.");
			goto err;
		}

		// If we successully changed the aspect, recompute the size.
		hr = reobj.poleobj->GetExtent(dwDrawAspect, &sizel);

		if(hr)
		{
			TRACEERRORSZ("GetExtent() failed.");
			goto err;
		}
	}

	// Try to retrieve the previous saved RichEdit site flags.
	if( ObjectReadSiteFlags(&reobj) != NOERROR )
	{
		// Set default for site flags
		reobj.dwFlags = REO_RESIZABLE;
	}

	// first, clear the range

	prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);

	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = prg->GetCp();
	reobj.dvaspect = dwDrawAspect;
	reobj.sizel = sizel;

	//COMPATIBILITY ISSUE: from Richedit 1.0 - don't Set the Extent,
	//instead Get the Extent below in ObFAddObjectSite
	//hr = reobj.poleobj->SetExtent(dwDrawAspect, &sizel);

	hr = reobj.poleobj->SetClientSite(reobj.polesite);

	if(hr)
	{
		TRACEERRORSZ("SetClientSite() failed.");
		goto err;
	}

	if(hr = _ped->InsertObject(&reobj))
	{
		TRACEERRORSZ("InsertObject() failed.");
	}

err:
	if(reobj.poleobj)
	{
		reobj.poleobj->Release();
	}
	if(reobj.polesite)
	{
		reobj.polesite->Release();
	}
	if(reobj.pstg)
	{
		reobj.pstg->Release();
	}

	return hr;
#else
    return 0;
#endif
}

