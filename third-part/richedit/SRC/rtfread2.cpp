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
 *	rtfread2.cpp
 *
 *	Description:
 *		This file contains the object functions for RichEdit RTF reader
 *
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 *
 *	* NOTE:
 *	*	All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *	*	noted as a szW.
 *
 */

#include "_common.h"

#include "_rtfread.h"
#include "_coleobj.h"

const char szFontsel[]="\\f";

ASSERTDATA


/*
 *		CRTFRead::HandleFieldInstruction()
 *
 *		Purpose:
 *			Handle field instruction
 *
 *		Returns:
 *			EC					The error code
 */
EC CRTFRead::HandleFieldInstruction()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldInstruction");

//TODO rewrite this function for common case
//FUTURE save field instruction

	BYTE *pch, *pch1;

	for(pch1 = _szText; *pch1 == ' '; pch1++)	// Bypass any leading blanks
		;
	for(pch = pch1; *pch && *pch != ' '; pch++)
		;

	_fHyperlinkField = FALSE;

	if(W32->ASCIICompareI(pch1, (BYTE *) "SYMBOL", 6))
		HandleFieldSymbolInstruction(pch);	//  SYMBOL

	else if (W32->ASCIICompareI(pch1, (BYTE *) "HYPERLINK", 9))
	{
		_fHyperlinkField = TRUE;
		HandleFieldHyperlink(pch);
	}
#ifdef PWD_JUPITER
	// GuyBark JupiterJ 49674: This field instruction is an Equation field!
	// These are used in FE RTF to specify Rubi text and the text beneath it.
	// I've seen the field results for such fields to be empty, which means 
	// that unless we take special action here, we'll lose the text completely.
	// So note that we're in an equation field.
	else if (W32->ASCIICompareI(pch1, (BYTE *) "eq", 2))
	{
	    LPSTR pOverstrikeStart = "\\o";

	    // We can only handle this group if it contains the overstrike group.
	    if(strstr((LPSTR)pch1, pOverstrikeStart))
	    {
	        _fEquationField = TRUE;

	        // Store the cp at the start of the equation field.
	        _cpFieldInstruction = _prg->GetCp();

	        // Set up the current text y offset from this equation string.
	        SetupEquationOffset((LPSTR)pch1);
	    }
	}
#endif // PWD_JUPITER

	// save the current formatting for the field result
	_FieldCF = _CF;
	_ptfField = _pstateStackTop->ptf;
	_nFieldCodePage = _pstateStackTop->nCodePage;


	TRACEERRSZSC("HandleFieldInstruction()", - _ecParseError);
	return _ecParseError;
}

/*
 *	HandleFieldSymbolInstruction(pch)
 *
 *	@mfunc
 *		Handle specific  symbol field
 *
 *	@rdesc
 *		EC	The error code
 *
 *	@devnote 
 *		FUTURE: the two whiles below can be combined into one fairly easily;
 *		Look at the definitions of IsXDigit() and IsDigit() and introduce
 *		a variable flag as well as a variable base multiplier (= 10 or 16).
 */
EC CRTFRead::HandleFieldSymbolInstruction(
	BYTE *pch )		//@parm Pointer to SYMBOL field instruction
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldInstruction");

	BYTE	ch;
	BYTE	chSymbol = 0;
	const char *pchFontsel = szFontsel;
	STATE *	pstate = _pstateStackTop;

	while (*pch == ' ')						// Eat spaces
		++pch;
											// Collect symbol char's code 
	if (*pch == '0' &&						//  which may be in decimal
 		(*++pch | ' ') == 'x')				//  or hex
	{										// It's in hex
		ch = *++pch;
	   	while (ch && ch != ' ')
	   	{
	   		if (IsXDigit(ch))
			{
				chSymbol <<= 4;
				chSymbol += (ch <= '9') ? ch - '0' : (ch & 0x4f) - 'A' + 10;
			}
			else
			{
			 	_ecParseError = ecUnexpectedChar;
				goto CleanUp;
			}
			ch = *pch++;
	   	}
	}
	else									// Decimal
	{
	   ch = *pch;
	   while (ch && ch != ' ')
	   {
	    	if (IsDigit(ch))
			{
				chSymbol *= 10;
				chSymbol += ch - '0' ;
			}
			else
			{
			 	_ecParseError = ecUnexpectedChar;
				goto CleanUp;
			}
			ch = *++pch;
	   }
	}
	_szSymbolFieldResult = (BYTE *)PvAlloc(2, GMEM_ZEROINIT);
    if (NULL != _szSymbolFieldResult)
    {
	    _szSymbolFieldResult[0] = chSymbol;
    }    
    else
    {
        Assert(_szSymbolFieldResult);  //give us a chance to debug
    }
    

	// now check for the \\f "Facename" construct 
	// and deal with it

	while (*pch == ' ')						// Eat spaces
	{
		++pch;
	}

	while (*pch && *pch == *pchFontsel)		// Make sure *pch is a \f
	{										
		++pch;
		++pchFontsel;
	}
	if	(! (*pchFontsel) )
	{
		_ecParseError = HandleFieldSymbolFont(pch);	//  \\f "Facename"
	}

// ASSERTION   font & font size  will be in field result \flds
// BUGBUG: A more robust implementation would parse the font
// and font size from both \fldinst and \fldrslt (RE1.0 does this)
	
CleanUp:
	TRACEERRSZSC("HandleFieldInstruction()", - _ecParseError);
	return _ecParseError;
}

/*
 *	HandleFieldSymbolFont()
 *
 *	@mfunc
 *		Handle the \\f "Facename" instruction in the SYMBOL field
 *
 *	@rdesc
 *		EC	The error code
 *
 *	@devnote WARNING: may change _szText
 */
EC CRTFRead::HandleFieldSymbolFont(BYTE *pch)
{
	SHORT iFont = _fonts.Count();
	TEXTFONT tf;
	TEXTFONT *ptf = &tf;

	_pstateStackTop->ptf = &tf;
	// ReadFontName tries to append
	tf.szName[0] = '\0';

	// skip the initial blanks and quotes
	while (*pch && (*pch == ' ' || *pch == '\"'))
	{
		++pch;
	}

	// DONT WORRY, we'll get it back to normal
	// ReadFontName depends on _szText, so we need to alter it and then restore
	// it's just too bad we have to do it ...
	BYTE* szTextBAK = _szText;
	BOOL fAllAscii = TRUE;

	_szText = pch;

	// transform the trailing quote into ';'
	while (*pch)
	{
		if (*pch == '\"')
		{
			*pch = ';';
			break;
		}

		if(*pch > 0x7f)
		{
			fAllAscii = FALSE;
		}
		++pch;
	}

	// NOW we can read the font name!!
	ReadFontName(_pstateStackTop, fAllAscii ? ALL_ASCII : CONTAINS_NONASCII);

	// Try to find this face name in the font table
	BOOL fFontFound = FALSE;
    SHORT i = 0;
	for (i = 0; i < iFont; ++i)
	{
		TEXTFONT *ptfTab = _fonts.Elem(i);
		if (0 == wcscmp(ptf->szName, ptfTab->szName))
		{
			fFontFound = TRUE;
			i = ptfTab->sHandle;
			break;
		}
	}

	// did we find the face name?
	if (!fFontFound)
	{
		Assert(i == iFont);
		i+= RESERVED_FONT_HANDLES;

		// Make room in font table for
		//  font to be inserted
		if (!(ptf =_fonts.Add(1,NULL)))
		{									
			_ped->GetCallMgr()->SetOutOfMemory();
			_ecParseError = ecNoMemory;
			goto exit;
		}

		// repeating inits from tokenFontSelect
		ptf->sHandle	= i;				// Save handle
		wcscpy_s(ptf->szName, tf.szName); 
		ptf->bPitchAndFamily = 0;
		ptf->fNameIsDBCS = FALSE;
		ptf->sCodePage = _nCodePage;
		ptf->bCharSet = DEFAULT_CHARSET;	// SYMBOL_CHARSET ??
	}

	SelectCurrentFont(i);
	
exit:
	// needs to go back to normal
	_szText = szTextBAK;

	return _ecParseError;
}

/*
 *	HandleFieldHyperlink(pch)
 *
 *	@mfunc
 *		Handle HYPERLINK field
 *
 *	@rdesc
 *		EC	The error code
 */
EC CRTFRead::HandleFieldHyperlink(
	BYTE *pch )		//@parm Pointer to HYPERLINK field instruction
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldHyperlink");

	BYTE *pBuffer;

	if ( *pch )
	{
		for( ; *pch == ' '; pch++) ;				// Skip leading blanks
	}

	// allocate the buffer and add the string to it
	_cchHyperlinkFldinst = MAX_PATH;
	_cchHyperlinkFldinstUsed = 1;
	pBuffer = (BYTE *)PvAlloc( MAX_PATH, GMEM_FIXED );
	
	if ( !pBuffer )
	{
		return ( _ecParseError = ecNoMemory );		 
	}

	pBuffer[0] = ' ';
	pBuffer[1] = '\0';
	_szHyperlinkFldinst = pBuffer;
	
	if ( *pch )
	{	
		_ecParseError = AppendString( &_szHyperlinkFldinst, pch, &_cchHyperlinkFldinst, &_cchHyperlinkFldinstUsed );
	}

	return _ecParseError;
}

/*
 *		CRTFRead::ReadData(pbBuffer, cbBuffer)
 *
 *		Purpose:
 *			Read in object data. This must be called only after all initial
 *			object header info has been read.
 *
 *		Arguments:
 *			pbBuffer		pointer to buffer where to put data
 *			cbBuffer		how many bytes to read in
 *
 *		Returns:
 *			LONG			number of bytes read in
 *
 */
LONG CRTFRead::ReadData(BYTE * pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadData");

	LONG cbLeft = cbBuffer;

	BYTE bChar0;
	BYTE bChar1;

	while (cbLeft && (bChar0 = GetHexSkipCRLF()) < 16 && 
						(bChar1 = GetHexSkipCRLF()) < 16)
	{	
		*pbBuffer++ = bChar0 << 4 | bChar1;
		cbLeft--;
	}							   

	return cbBuffer - cbLeft ; 
}

/*
 *		CRTFRead::ReadBinaryData(pbBuffer, cbBuffer)
 *
 *		Purpose:
 *
 *		Arguments:
 *			pbBuffer		pointer to buffer where to put data
 *			cbBuffer		how many bytes to read in
 *
 *		Returns:
 *			LONG			number of bytes read in
 */
LONG CRTFRead::ReadBinaryData(BYTE *pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadBinaryData");

	LONG cbLeft = min(_cbBinLeft, cbBuffer);

	cbBuffer = cbLeft;

	for (; cbLeft >0 ; cbLeft--)
	{
		*pbBuffer++ = GetChar();
	}

	_cbBinLeft -= cbBuffer; 

	return cbBuffer ;
}

/*
 *		CRTFRead::SkipBinaryData(cbBuffer)
 *
 *		Purpose:
 *
 *		Arguments:
 *			cbBuffer		how many bytes to skip
 *
 *		Returns:
 *			LONG			number of bytes skipped
 */
LONG CRTFRead::SkipBinaryData(LONG cbSkip)
{
	BYTE rgb[1024];

	_cbBinLeft = cbSkip;

	while(ReadBinaryData(rgb, sizeof(rgb)) > 0) 
	{
	}

	return cbSkip;
}

/*
 *		CRTFRead::StrAlloc(ppsz, sz)
 *
 *		Purpose:
 *			Set up a pointer to a newly allocated space to hold a string
 *
 *		Arguments:
 *			ppsz			Ptr to ptr to string that needs allocation
 *			sz				String to be copied into allocated space
 *
 *		Returns:
 *			EC				The error code
 */
EC CRTFRead::StrAlloc(TCHAR ** ppsz, BYTE * sz)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::StrAlloc");

	int Length =  (int)strlen((CHAR *)sz)+1 ;

	*ppsz = (TCHAR *) PvAlloc((Length + 1)*sizeof(TCHAR), GMEM_ZEROINIT);
	if (!*ppsz)
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		goto Quit;
	}
	
	MultiByteToWideChar(CP_ACP,0,(char *)sz,-1,*ppsz,Length) ;

Quit:
	return _ecParseError;
}

/*
 *		CRTFRead::FreeRtfObject()
 *
 *		Purpose:
 *			Cleans up memory used by prtfobject
 */
void CRTFRead::FreeRtfObject()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FreeRtfObject");

	if (_prtfObject)
	{
		FreePv(_prtfObject->szClass);
		FreePv(_prtfObject->szName);
		FreePv(_prtfObject);
		_prtfObject = NULL;
	}
}

/*
 *	CRTFRead::ObjectReadSiteFlags
 *
 *	Purpose:
 *		Read the dwFlags and dwUser bytes from a container specific stream
 *
 *	Arguments:
 *		preobj			The REOBJ from where to copy the flags this preobj is
 *						then later put out in a site
 *
 *	Returns:
 *		BOOL			TRUE if successfully read the bytes
 */
BOOL CRTFRead::ObjectReadSiteFlags( REOBJECT * preobj)
{
	return (::ObjectReadSiteFlags(preobj) == NOERROR);
}

/*
 *	ObjectReadFromStream
 *
 *	Purpose:
 *		Reads an OLE object from the RTF output stream.
 *
 *
 *	Returns:
 *		BOOL		TRUE on success, FALSE on failure.
 */
BOOL CRTFRead::ObjectReadFromEditStream(void)
{
	HRESULT hr;
	BOOL fRet = FALSE;
	REOBJECT reobj = { 0 };
	LPRICHEDITOLECALLBACK  precall=NULL;
	WCHAR 	ch = WCH_EMBEDDING;
	LPOLECACHE polecache = NULL;
	LPENUMSTATDATA penumstatdata = NULL;
	STATDATA statdata;
	BOOL fGotClsid = TRUE;

	CObjectMgr *ObjectMgr = _ped->GetObjectMgr();

	if (! ObjectMgr)
	   goto Cleanup;
	
	precall = ObjectMgr->GetRECallback();

	// If no IRichEditOleCallback exists, then fail
	if (!precall)
		goto Cleanup;

//	AssertSz(_prtfObject->szClass,"ObFReadFromEditstream: reading unknown class");

	//$ REVIEW: MAC This call is incorrect for the Mac.  It may not matter though

	//          if ole support in RichEdit is not needed for the Mac.
	if (!(_prtfObject->szClass && 
		pCLSIDFromProgID(_prtfObject->szClass, &reobj.clsid)
		== NOERROR))
	{
		fGotClsid = FALSE;
	}

	// Get storage for the object from the application
	if (FAILED(precall->GetNewStorage(&reobj.pstg)))
	{
		goto Cleanup;
	}

	hr = pOleConvertOLESTREAMToIStorage((LPOLESTREAM) &RTFReadOLEStream, reobj.pstg, NULL);
	if (FAILED(hr))					   
		goto Cleanup;		  


	// Create another object site for the new object
	_ped->GetClientSite(&reobj.polesite) ;
	if (!reobj.polesite )
	{
		goto Cleanup;
	}

	if (FAILED(pOleLoad(reobj.pstg, IID_IOleObject, reobj.polesite,
				(LPVOID *) &reobj.poleobj)))
	{
		goto Cleanup;
	}

	if(!fGotClsid) {
		// we weren't able to obtain a clsid from the progid
		// in the \objclass RTF tag	

		reobj.poleobj->GetUserClassID(&reobj.clsid);
	}
	
	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = _prg->GetCp();
	reobj.sizel.cx = HimetricFromTwips(_prtfObject->xExt)
						* _prtfObject->xScale / 100;
	reobj.sizel.cy = HimetricFromTwips(_prtfObject->yExt)
						* _prtfObject->yScale / 100;

	// Read any container flags which may have been previously saved
	if (!ObjectReadSiteFlags(&reobj))
	{
		// If no flags, make best guess
		reobj.dwFlags = REO_RESIZABLE;
	}
	reobj.dvaspect = DVASPECT_CONTENT;		// OLE 1 forces DVASPECT_CONTENT

	// Ask the cache if it knows what to display
	if (SUCCEEDED(reobj.poleobj->QueryInterface(IID_IOleCache, (void**)&polecache)) &&
		SUCCEEDED(polecache->EnumCache(&penumstatdata)))
	{
		// Go look for the best cached presentation CF_METAFILEPICT
		while (penumstatdata->Next(1, &statdata, NULL) == S_OK)
		{
			if (statdata.formatetc.cfFormat == CF_METAFILEPICT)
			{
				LPDATAOBJECT pdataobj = NULL;
				STGMEDIUM med;
				BOOL fUpdate;

				ZeroMemory(&med, sizeof(STGMEDIUM));
                if (SUCCEEDED(polecache->QueryInterface(IID_IDataObject, (void**)&pdataobj)) &&
					SUCCEEDED(pdataobj->GetData(&statdata.formatetc, &med)))
                {
					HANDLE	hGlobal;

					hGlobal = med.hGlobal;

					if( FIsIconMetafilePict(hGlobal) )
				    {
						// BUGBUG: was !OleStdSwitchDisplayAspect(...)
					    OleStdSwitchDisplayAspect(reobj.poleobj, &reobj.dvaspect, 
							  					    DVASPECT_ICON, med.hGlobal,
												    TRUE, FALSE, NULL, &fUpdate);
				    }
				}

				pReleaseStgMedium(&med);
				if (pdataobj)
				{
					pdataobj->Release();
				}
				break;
			}
		}

		polecache->Release();
		penumstatdata->Release();
	}

	// This code is borrowed from RichEdit1.0; Word generates
	// bogus objects, so we need to compensate.

	if( reobj.dvaspect == DVASPECT_CONTENT )
	{
		IStream *pstm = NULL;
		BYTE bT;
		BOOL fUpdate;

		if (SUCCEEDED(reobj.pstg->OpenStream(OLESTR("\3ObjInfo"), 0, STGM_READ |
									   STGM_SHARE_EXCLUSIVE, 0, &pstm)) &&
		   SUCCEEDED(pstm->Read(&bT, sizeof(BYTE), NULL)) &&
		   (bT & 0x40))
		{
		   _fNeedIcon = TRUE;
		   _fNeedPres = TRUE;
		   _pobj = (COleObject *)reobj.polesite;
		   OleStdSwitchDisplayAspect(reobj.poleobj, &reobj.dvaspect, DVASPECT_ICON,
									   NULL, TRUE, FALSE, NULL, &fUpdate);
		}
		if( pstm )
		{
			pstm->Release();
		}
   }

	// Since we are loading an object, it shouldn't be blank
	reobj.dwFlags &= ~REO_BLANK;

	_prg->Set_iCF(-1);	
	_prg->ReplaceRange(1, &ch, NULL, SELRR_IGNORE);  
	hr = ObjectMgr->InsertObject(reobj.cp, &reobj, NULL);

	if (FAILED(hr))
	{
		goto Cleanup;
	}

	// Word doesn't give us objects with presenation
	// caches; as a result, we can't draw them!  In order to get around this,
	// we check to see if there is a presentation cache (via the same way
	// RE1.0 did) using a GetExtent call.  If that fails, we'll just use
	// the presentation stored in the RTF.  
	//
	// COMPATIBILITY ISSUE: RE1.0, instead of using the presenation stored
	// in RTF, would instead call IOleObject::Update.  There are two _big_
	// drawbacks to this approach: 1. it's incredibly expensive (potentially,
	// MANY SECONDS per object), and 2. it doesn't work if the object server
	// is not installed on the machine.

	SIZE sizeltemp;

	if( reobj.poleobj->GetExtent(reobj.dvaspect, &sizeltemp) != NOERROR )
	{
		_fNeedPres = TRUE;
		_pobj = (COleObject *)reobj.polesite;
	}

	fRet = TRUE;

Cleanup:
	if (reobj.pstg)	reobj.pstg->Release();
	if (reobj.polesite) reobj.polesite->Release();
	if (reobj.poleobj) reobj.poleobj->Release();

	return fRet;
}

/*
 *	ObHBuildMetafilePict(prtfobject, hBits)
 *
 *	Purpose:
 *		Build a METAFILEPICT from RTFOBJECT and the raw data.
 *
 *	Arguments:
 *		RTFOBJECT *	The details we picked up from RTF
 *		HGLOBAL		A handle to the raw data
 *
 *	Returns:
 *		HGLOBAL		Handle to a METAFILEPICT
 */
HGLOBAL ObHBuildMetafilePict(RTFOBJECT *prtfobject, HGLOBAL hBits)
{
#ifndef NOMETAFILES
	HGLOBAL hmfp = NULL;
	LPMETAFILEPICT pmfp = NULL;
	SCODE sc = E_OUTOFMEMORY;
	LPBYTE pbBits;
	ULONG cbBits;

	// Allocate the METAFILEPICT structure
    hmfp = GlobalAlloc(GHND, sizeof(METAFILEPICT));
	if (!hmfp)
		goto Cleanup;

	// Lock it down
	pmfp = (LPMETAFILEPICT) GlobalLock(hmfp);
	if (!pmfp)
		goto Cleanup;

	// Put in the header information
	pmfp->mm = prtfobject->sPictureType;
	pmfp->xExt = prtfobject->xExt;
	pmfp->yExt = prtfobject->yExt;

	// Set the metafile bits
	pbBits = (LPBYTE) GlobalLock(hBits);
	cbBits = GlobalSize(hBits);
	pmfp->hMF = SetMetaFileBitsEx(cbBits, pbBits);
	
	// We can throw away the data now since we don't need it anymore
	GlobalUnlock(hBits);
	GlobalFree(hBits);

	if (!pmfp->hMF)
		goto Cleanup;
	GlobalUnlock(hmfp);
	sc = S_OK;

Cleanup:
	if (sc && hmfp)
	{
		if (pmfp)
			GlobalUnlock(hmfp);
		GlobalFree(hmfp);
		hmfp = NULL;
	}
	TRACEERRSZSC("ObHBuildMetafilePict", sc);
	return hmfp;
#else
	return NULL;
#endif
}

/*
 *	ObHBuildBitmap
 *
 *	Purpose:
 *		Build a BITMAP from RTFOBJECT and the raw data
 *
 *	Arguments:
 *		RTFOBJECT *	The details we picked up from RTF
 *		HGLOBAL		A handle to the raw data
 *
 *	Returns:
 *		HGLOBAL		Handle to a BITMAP
 */
HGLOBAL ObHBuildBitmap(RTFOBJECT *prtfobject, HGLOBAL hBits)
{
	HBITMAP hbm = NULL;
	LPVOID	pvBits = GlobalLock(hBits);

	if (!pvBits)
		goto Cleanup;
	hbm = CreateBitmap(prtfobject->xExt, prtfobject->yExt,
						prtfobject->cColorPlanes, prtfobject->cBitsPerPixel,
						pvBits);

Cleanup:
	GlobalUnlock(hBits);
	GlobalFree(hBits);
	return hbm;
}

/*
 *	ObHBuildDib
 *
 *	Purpose:
 *		Build a DIB from RTFOBJECT and the raw data
 *
 *	Arguments:
 *		RTFOBJECT *	The details we picked up from RTF
 *		HGLOBAL		A handle to the raw data
 *
 *	Returns:
 *		HGLOBAL		Handle to a DIB
 */
HGLOBAL ObHBuildDib(RTFOBJECT *prtfobject, HGLOBAL hBits)
{
	// Apparently DIB's are just a binary dump
	return hBits;
}

/*
 *	CRTFRead::StaticObjectReadFromEditstream
 *
 *	Purpose:
 *		Reads an picture from the RTF output stream.
 *
 *	Returns:
 *		BOOL		TRUE on success, FALSE on failure.
 */
#define cbBufferMax	16384
#define cbBufferStep 1024
#define cbBufferMin 1024
BOOL CRTFRead::StaticObjectReadFromEditStream(int cb)
{
	HRESULT hr;
	BOOL fRet = FALSE;
	LPPERSISTSTORAGE pperstg = NULL;
	LPOLECACHE polecache = NULL;
	REOBJECT reobj = { 0 };
	LPSTREAM pstm = NULL;
	LPBYTE pbBuffer = NULL;
	LONG cbRead;
	LONG cbBuffer;
	FORMATETC formatetc;
	STGMEDIUM stgmedium;
	DWORD dwConn;
	HGLOBAL hBits = NULL;
	HGLOBAL (*pfnBuildPict)(RTFOBJECT *, HGLOBAL) = NULL;
	LPRICHEDITOLECALLBACK  precall ;
	DWORD dwAdvf;
	WCHAR 	ch = WCH_EMBEDDING;

	CObjectMgr *ObjectMgr = _ped->GetObjectMgr();

	if (! ObjectMgr)
	   goto Cleanup;
	
	// precall may end up being null (e.g. Windows CE).
	precall = ObjectMgr->GetRECallback();

	// Initialize various data structures
	formatetc.ptd = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	switch (_prtfObject->sType)
	{
	case ROT_Metafile:
		reobj.clsid = CLSID_StaticMetafile;
		formatetc.cfFormat = CF_METAFILEPICT;
		formatetc.tymed = TYMED_MFPICT;
		pfnBuildPict = ObHBuildMetafilePict;
		break;

	case ROT_Bitmap:
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_BITMAP;
		formatetc.tymed = TYMED_GDI;
		pfnBuildPict = ObHBuildBitmap;
		break;

	case ROT_DIB:
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_DIB;
		formatetc.tymed = TYMED_HGLOBAL;
		pfnBuildPict = ObHBuildDib;
		break;

    default:
        AssertSz(0, "Bad ObjectType in CRTFRead::StaticObjectReadFromEditStream");
        goto Cleanup;
	}

	reobj.sizel.cx = (LONG) HimetricFromTwips(_prtfObject->xExtGoal)
						* _prtfObject->xScale / 100;
	reobj.sizel.cy = (LONG) HimetricFromTwips(_prtfObject->yExtGoal)
						* _prtfObject->yScale / 100;
	stgmedium.tymed = formatetc.tymed;
	stgmedium.pUnkForRelease = NULL;

	if (precall)
	{
		if( !_fNeedPres )
		{
			// Get storage for the object from the application
			if (FAILED(precall->GetNewStorage(&reobj.pstg)))
			{
				goto Cleanup;
			}
		}
		// Let's create a stream on HGLOBAL
		if (FAILED(hr = pCreateStreamOnHGlobal(NULL, FALSE, &pstm)))
		{
			goto Cleanup;
		}
		// Allocate a buffer, preferably a big one
		for (cbBuffer = cbBufferMax;
			 cbBuffer >= cbBufferMin;
			cbBuffer -= cbBufferStep)
		{
			pbBuffer = (unsigned char *)PvAlloc(cbBuffer, 0);
			if (pbBuffer)
				break;
		}
	}
	else
	{
		cbBuffer = cb;
		if (!cb)
		{
			// this means we didn't understand the picture type; so just
			// skip it without failing.
			fRet = TRUE;
			goto Cleanup;
		};

        hBits = GlobalAlloc(GMEM_FIXED, cb);
		pbBuffer = (BYTE *) GlobalLock(hBits);
	}
		
	if (!pbBuffer)
	{
		goto Cleanup;
	}
	
	// Copy the data from RTF into our HGLOBAL

	while ((cbRead = RTFReadOLEStream.lpstbl->Get(&RTFReadOLEStream,pbBuffer,cbBuffer)) > 0)
	{
		if (pstm && (hr = pstm->Write( pbBuffer, cbRead, NULL)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: Write", GetScode(hr));
			goto Cleanup;
		}
	}

	if (hBits)
	{
		Assert(!precall);
		GlobalUnlock(hBits);
		pbBuffer = NULL;		// To avoid free below
	}

	if (pstm && (hr = pGetHGlobalFromStream(pstm, &hBits)))
	{
		TRACEERRSZSC("ObFReadStaticFromEditstream: no hglobal from stm", GetScode(hr));
		goto Cleanup;
	}

	// Build the picture
	if( pfnBuildPict )
	{
		stgmedium.hGlobal = pfnBuildPict(_prtfObject, hBits);
	}
	else
	{
		// this means we didn't understand the picture type; so just
		// skip it without failing.
		fRet = TRUE;
		goto Cleanup;
	}

	if( precall && !stgmedium.hGlobal )
		goto Cleanup;

	if( precall )
	{
		if( !_fNeedPres )
		{
			// Create the default handler
			hr = pOleCreateDefaultHandler(reobj.clsid, NULL, IID_IOleObject,(void **) &reobj.poleobj);
			if (FAILED(hr))
			{
				TRACEERRSZSC("ObFReadStaticFromEditstream: no def handler", GetScode(hr));
				goto Cleanup;
			}

			// Get the IPersistStorage and initialize it
			if ((FAILED(hr = reobj.poleobj->QueryInterface(IID_IPersistStorage,(void **)&pperstg))) ||
				(FAILED(hr = pperstg->InitNew(reobj.pstg))))
			{
				TRACEERRSZSC("ObFReadStaticFromEditstream: InitNew", GetScode(hr));
				goto Cleanup;
			}
			dwAdvf = ADVF_PRIMEFIRST;
		}
		else
		{
			Assert(_pobj);
			_pobj->GetIUnknown()->QueryInterface(IID_IOleObject, (void **)&(reobj.poleobj));
			dwAdvf = ADVF_NODATA;
			formatetc.dwAspect = _fNeedIcon ? DVASPECT_ICON : DVASPECT_CONTENT;
		}

		// Get the IOleCache and put the picture data there
		if (FAILED(hr = reobj.poleobj->QueryInterface(IID_IOleCache,(void **)&polecache)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: QI: IOleCache", GetScode(hr));
			goto Cleanup;
		}

		if (FAILED(hr = polecache->Cache(&formatetc, dwAdvf, &dwConn)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: Cache", GetScode(hr));
			goto Cleanup;
		}

		if (FAILED(hr = polecache->SetData(&formatetc, &stgmedium, TRUE)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: SetData", GetScode(hr));
			goto Cleanup;
		}
	}

	if( !_fNeedPres )
	{
		// Create another object site for the new object
		_ped->GetClientSite(&reobj.polesite) ;
		if (!reobj.polesite )
		{
			goto Cleanup;
		}

		// Set the client site
		if (reobj.poleobj && (hr = reobj.poleobj->SetClientSite(reobj.polesite)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: SetClientSite", GetScode(hr));
			goto Cleanup;
		}
		else if (!reobj.poleobj)
		{
			// Windows CE static object Save the data and mark it.
			COleObject *pobj = (COleObject *)reobj.polesite;
			COleObject::ImageInfo *pimageinfo = new COleObject::ImageInfo;
			pobj->SetHdata(hBits);
			pimageinfo->xScale = _prtfObject->xScale;
			pimageinfo->yScale = _prtfObject->yScale;
			pimageinfo->xExtGoal = _prtfObject->xExtGoal;
			pimageinfo->yExtGoal = _prtfObject->yExtGoal;
			pimageinfo->cBytesPerLine = _prtfObject->cBytesPerLine;
			pobj->SetImageInfo(pimageinfo);
		}

		// Put object into the edit control
		reobj.cbStruct = sizeof(REOBJECT);
		reobj.cp = _prg->GetCp();
		reobj.dvaspect = DVASPECT_CONTENT;
		reobj.dwFlags = REO_RESIZABLE;
		// Since we are loading an object, it shouldn't be blank
		reobj.dwFlags &= ~REO_BLANK;


		_prg->Set_iCF(-1);	
		_prg->ReplaceRange(1, &ch, NULL, SELRR_IGNORE);  
		hr = ObjectMgr->InsertObject(reobj.cp, &reobj, NULL);
		if (FAILED(hr))
		{
			goto Cleanup;
		}
	}
	else
	{
		// the new presentation may have a different idea about how big the
		// object is supposed to be.  Make sure the object stays the correct
		// size.
		_pobj->ResetSizel(reobj.sizel);
	}

	fRet = TRUE;

Cleanup:

    // V-GUYB: 
    // If we failed to stream in the object, then set the oom error here.
    // Note, the above code can fail for other reasons, but oom is most 
    // likely. By setting the error here, the user will see the oom message 
    // and the stream in will be aborted.
    if(!fRet)
    {
        _ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
    }

	if (polecache) polecache->Release()	;
	if (reobj.pstg)	reobj.pstg->Release();
	if (reobj.polesite) reobj.polesite->Release();
	if (reobj.poleobj) reobj.poleobj->Release();
	if (pperstg) pperstg->Release();
	if (pstm) pstm->Release();
	FreePv(pbBuffer);

	_fNeedIcon = FALSE;
	_fNeedPres = FALSE;
	_pobj = NULL;

	return fRet;
}



