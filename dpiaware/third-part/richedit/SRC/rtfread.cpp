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
 *	@module	RTFREAD.CPP - RichEdit RTF reader (w/o objects) |
 *
 *		This file contains the nonobject code of RichEdit RTF reader.
 *		See rtfread2.cpp for embedded-object code.
 *
 *	Authors:<nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0 w/o objects:  Murray Sargent
 *		Lots of enhancements/maintenance: Brad Olenick
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szW.
 *
 *	@todo
 *		1. Unrecognized RTF. Also some recognized won't round trip <nl>
 *		2. In font.c, add overstrike for CFE_DELETED and underscore for
 *			CFE_REVISED.  Would also be good to change color for CF.bRevAuthor
 *
 */
#include "_common.h"

#ifdef PWD_JUPITER
// GuyBark JupiterJ 49674: Must be able to select and remove equation field text.
#include "_select.h"
#endif // PWD_JUPITER

#include "_rtfread.h"
#include "_util.h"

ASSERTDATA

/*
 *		Global Variables
 */

#define	PFM_ALLRTF		(PFM_ALL2 | PFM_COLLAPSED | PFM_OUTLINELEVEL | PFM_BOX)
#pragma BEGIN_CODESPACE_DATA

// for object attachment placeholder list
#define cobPosInitial 8
#define cobPosChunk 8

#pragma END_CODESPACE_DATA

#ifdef PWD_JUPITER
// GuyBark: Strip font decorations during stream in, and add them again on stream out.
// PWord on the device doesn't need the decorations as the font can display text from
// multiple character sets. But we must stream out the decorated fonts as Word95 uses 
// them in determining the character set of the font.
FontDecorations const fontDec[] = { {" Baltic",  BALTIC_CHARSET},
                                    {" CE",      EASTEUROPE_CHARSET},
                                    {" Cyr",     RUSSIAN_CHARSET},
                                    {" Greek",   GREEK_CHARSET},
                                    {" Tur",     TURKISH_CHARSET},
                                    {" Turkish", TURKISH_CHARSET},
                                    {NULL, 0}    }; // <- This indicates the end of the array.                    
#endif // PWD_JUPITER

#if CFE_SMALLCAPS != 0x40 || CFE_ALLCAPS != 0x80 || CFE_HIDDEN != 0x100 \
 || CFE_OUTLINE != 0x200  || CFE_SHADOW != 0x400
#error "Need to change RTF char effect conversion routines
#endif

// for RTF tag coverage testing
#if defined(DEBUG)
#define TESTPARSERCOVERAGE() \
	{ \
		if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGE", 0)) \
		{ \
			TestParserCoverage(); \
		} \
	}
#define PARSERCOVERAGE_CASE() \
	{ \
		if(_fTestingParserCoverage) \
		{ \
			return ecNoError; \
		} \
	}
#define PARSERCOVERAGE_DEFAULT() \
	{ \
		if(_fTestingParserCoverage) \
		{ \
			return ecStackOverflow; /* some bogus error */ \
		} \
	}
#else
#define TESTPARSERCOVERAGE()
#define PARSERCOVERAGE_CASE()
#define PARSERCOVERAGE_DEFAULT()
#endif


// FF's should not have paragraph number prepended to them
inline BOOL CharGetsNumbering(WORD ch) { return ch != FF; }

// V-GUYB: PWord Converter requires loss notification.
#ifdef REPORT_LOSSAGE
typedef struct
{
    IStream *pstm;
    BOOL     bFirstCallback;
    LPVOID  *ppwdPWData;
    BOOL     bLoss;
} LOST_COOKIE;
#endif


//======================== OLESTREAM functions =======================================

DWORD CALLBACK RTFGetFromStream (
	RTFREADOLESTREAM *OLEStream,	//@parm OleStream
	void FAR *		  pvBuffer,		//@parm Buffer to read 
	DWORD			  cb)			//@parm Bytes to read
{
	return OLEStream->Reader->ReadData ((BYTE *)pvBuffer, cb);
}

DWORD CALLBACK RTFGetBinaryDataFromStream (
	RTFREADOLESTREAM *OLEStream,	//@parm OleStream
	void FAR *		  pvBuffer,		//@parm Buffer to read 
	DWORD			  cb)			//@parm Bytes to read
{
	return OLEStream->Reader->ReadBinaryData ((BYTE *)pvBuffer, cb);
}


//============================ STATE Structure =================================
/*
 *	STATE::AddPF(PF, lDefTab, lDocType)
 *
 *	@mfunc
 *		If the PF contains new info, this info is applied to the PF for the
 *		state.  If this state was sharing a PF with a previous state, a new
 *		PF is created for the state, and the new info is applied to it.
 *
 *	@rdesc
 *		TRUE unless needed new PF and couldn't allocate it 
 */
BOOL STATE::AddPF(
	const CParaFormat &PF,	//@parm Current RTFRead _PF
	LONG lDefTab,			//@parm	Default tab to use if no previous state
	LONG lDocType)			//@parm	Default doc type to use if no prev state
{
	// Create a new PF if:  
	//	1.  The state doesn't have one yet
	//	2.  The state has one, but it is shared by the previous state and
	//		there are PF deltas to apply to the state's PF
	if(!pPF || (PF.dwMask && pstatePrev && pPF == pstatePrev->pPF))
	{
		Assert(!pstatePrev || pPF);

		pPF = new CParaFormat;
		if(!pPF)
			return FALSE;

		// Give the new PF some initial values - either from the previous
		// state's PF or by CParaFormat initialization
		if(pstatePrev)
		{
			// Copy the PF from the previous state
			*pPF = *pstatePrev->pPF;
		}
		else
		{
			// We've just created a new PF for the state - there is no
			// previous state to copy from.  Use default values.
			pPF->InitDefault(lDefTab, lDocType == DT_RTLDOC ? PFE_RTLPARA : 0);
			pPF->dwMask	= PFM_ALLRTF;
		}
	}

	// Apply the new PF deltas to the state's PF
	if(PF.dwMask)
		pPF->Apply(&PF);

	return TRUE;
}

/*
 *	STATE::DeletePF()
 *
 *	@mfunc
 *		If the state's PF is not shared by the previous state, the PF for this
 *		state is deleted.
 *
 */
void STATE::DeletePF()
{
	if(pPF && (!pstatePrev || (pPF != pstatePrev->pPF)))
	{
		delete pPF;
	}
	pPF = NULL;
}

/*
 *	STATE::SetCodePage(CodePage, ansicpg)
 *
 *	@mfunc
 *		If N of \ansicpgN is CP_UTF8, use it for all conversions (yes, even
 *		for SYMBOL_CHARSET). Else use CodePage.
 */
void STATE::SetCodePage(
	LONG CodePage)
{
	if(nCodePage != CP_UTF8)
		nCodePage = CodePage;
}

//============================ CRTFRead Class ==================================
/*
 *	CRTFRead::CRTFRead()
 *
 *	@mfunc
 *		Constructor for RTF reader
 */
CRTFRead::CRTFRead (
	CTxtRange *		prg,			// @parm CTxtRange to read into
	EDITSTREAM *	pes,			// @parm Edit stream to read from
	DWORD			dwFlags			// @parm Read flags
)
	: CRTFConverter(prg, pes, dwFlags, TRUE)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::CRTFRead");

	Assert(prg->GetCch() == 0);

	//TODO(BradO):  We should examine the member data in the constructor
	//	and determine which data we want initialized on construction and
	//	which at the beginning of every read (in CRTFRead::ReadRtf()).

#ifdef BIDI
	_cpThisDirection = _cpThisJoiner = prg->GetCp();
#endif
	_sDefaultFont	= -1;					// No \deff n control word yet
	_sDefaultLanguage = INVALID_LANGUAGE;
	_sDefaultLanguageFE = INVALID_LANGUAGE;
	_sDefaultTabWidth = 0;
	_CF.dwMask		= 0;					// No char format changes yet
	_FieldCF.dwMask = 0;
	_nFieldCodePage = 0;
	_ptfField 		= NULL;
	_fRestoreFieldFormat = FALSE;
	_dwFlagsUnion	= 0;					// No flags yet
	_pes->dwError	= 0;					// No error yet
	_cchUsedNumText	= 0;					// No numbering text yet
	_cCell			= 0;					// No table cells yet
	_iCell			= 0;
	_pstateStackTop	= NULL;
	_pstateLast		= NULL;
	_szText			=
	_pchRTFBuffer	=						// No input buffer yet
	_pchRTFCurrent	=
	_szSymbolFieldResult  = 
	_pchRTFEnd		= NULL;
	_prtfObject		= NULL;
	_pcpObPos		= NULL;
	_bTabLeader		= 0;
	_bTabType		= 0;
	_pobj			= 0;
	_wAlignment		= PFA_LEFT;

	_szHyperlinkFldinst	= NULL;
	_szHyperlinkFldrslt	= NULL;

	// Does story size exceed the maximum text size?
	_cchMax = _ped->TxGetMaxLength() - _ped->GetAdjustedTextLength();
	_cchMax = max((LONG)_cchMax, 0);

	ZeroMemory(_rgStyles, sizeof(_rgStyles)); // No style levels yet

	_bBiDiCharSet = (g_wLang == sLanguageHebrew) ? HEBREW_CHARSET : ARABIC_CHARSET;

#ifdef PWD_JUPITER
	// GuyBark Jupiter 31960: We haven't found any collappsed text yet.
	_fFoundCollapsedText = FALSE;
#endif // PWD_JUPITER
	
	// init OleStream
	RTFReadOLEStream.Reader = this;
    
    if (NULL == RTFReadOLEStream.lpstbl)
    {
        //something very bizarre happened during init of this object
        TRACEWARNSZ("RTFReadOLEStream.lpstbl is NOT initialized");
        Assert(RTFReadOLEStream.lpstbl);  //bad news to do this in the constructor - debug this
    }
    else
    {
    	RTFReadOLEStream.lpstbl->Get = (DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
    							   RTFGetFromStream;
    	RTFReadOLEStream.lpstbl->Put = NULL;
    }	

#ifdef DEBUG

// TODO: Implement RTF tag logging for the Mac
#if !defined(MACPORT)
	_fTestingParserCoverage = FALSE;
	_prtflg = NULL;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFLOG", 0))
	{
		_prtflg = new CRTFLog;
	
		if(_prtflg)
		{
			if(!_prtflg->FInit())
			{
				delete _prtflg;
				_prtflg = NULL;
			}
		}

		AssertSz(_prtflg, "CRTFRead::CRTFRead:  Error creating RTF log");
	}
#endif
#endif // DEBUG
}

/*
 *	CRTFRead::HandleStartGroup()
 *	
 *	@mfunc
 *		Handle start of new group. Alloc and push new state onto state
 *		stack
 *
 *	@rdesc
 *		EC					The error code
 */
EC CRTFRead::HandleStartGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleStartGroup");

	STATE *	pstate	   = _pstateStackTop;
	STATE *	pstateNext = NULL;

	if (pstate)									// At least one STATE already
	{											//  allocated
		Apply_CF();								// Apply any collected char
		// Note (igorzv) we don't Apply_PF() here so as not to change para 
		// properties before we run into \par i.e. not to use paragraph 
		// properties if we copy only one word from paragraph. We can use an
		// assertion here that neither we nor Word use end of group for
		// restoring paragraph properties. So everything will be OK with stack
		pstate->iCF = _prg->Get_iCF();			// Save current CF
		pstate = pstate->pstateNext;			// Use previously allocated
		if(pstate)								//  STATE frame if it exists
			pstateNext = pstate->pstateNext;	// It does; save its forward
	}											//  link for restoration below

	if(!pstate)									// No new STATE yet: alloc one
	{
		pstate = new STATE(_dwFlags & SFF_UTF8 ? CP_UTF8 : _nCodePage);
		if (!pstate)							// Couldn't alloc new STATE
			goto memerror;

		_pstateLast = pstate;					// Update ptr to last STATE
	}											//  alloc'd

	STATE *pstateGetsPF;

	// Apply the accumulated PF delta's to the old current state or, if there
	//	is no current state, to the newly created state.
	pstateGetsPF = _pstateStackTop ? _pstateStackTop : pstate;
	if(!pstateGetsPF->AddPF(_PF, _sDefaultTabWidth, _bDocType))
	{
		goto memerror;
	}
	_PF.dwMask = 0;  // _PF contains delta's from *_pstateStackTop->pPF

	if(_pstateStackTop)							// There's a previous STATE
	{
		*pstate = *_pstateStackTop;				// Copy current state info
		// N.B.  This will cause the current and previous state to share
		// 	the same PF.  PF delta's are accumulated in _PF.  A new PF
		// 	is created for _pstateStackTop when the _PF deltas are applied.

		_pstateStackTop->pstateNext = pstate;
#ifdef BIDI
		pstate->fModDirection = FALSE;
		pstate->fModJoiner = FALSE;
#endif
	}

	pstate->pstatePrev = _pstateStackTop;		// Link STATEs both ways
	pstate->pstateNext = pstateNext;
	_pstateStackTop = pstate;					// Push stack

done:
	TRACEERRSZSC("HandleStartGroup()", -_ecParseError);
	return _ecParseError;

memerror:
	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecStackOverflow;
	goto done;
}

/*
 *	CRTFRead::HandleEndGroup()
 *
 *	@mfunc
 *		Handle end of new group
 *
 *	@rdesc
 *		EC					The error code
 */
EC CRTFRead::HandleEndGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEndGroup");

	STATE *	pstate = _pstateStackTop;
	STATE *	pstatePrev;

	if (!pstate)								// No stack to pop
	{
		_ecParseError = ecStackUnderflow;
		goto done;
	}

	_pstateStackTop =							// Pop stack
	pstatePrev		= pstate->pstatePrev;

	if(!pstatePrev)
	{
		Assert(pstate->pPF);

		// We're ending the parse.  Copy the final PF into _PF so that 
		// 	subsequent calls to Apply_PF will have a PF to apply.
		_PF = *pstate->pPF;
	}

	// Adjust the PF for the new _pstateStackTop and delete unused PF's.
	if(pstate->sDest == destParaNumbering || pstate->sDest == destParaNumText)
	{
		if(pstatePrev && pstate->pPF != pstatePrev->pPF)
		{
			// Bleed the PF of the current state into the previous state for
			// paragraph numbering groups
			Assert(pstatePrev->pPF);
			pstatePrev->DeletePF();
			pstatePrev->pPF = pstate->pPF;
			pstate->pPF = NULL;
		}
		else
			pstate->DeletePF();
		// N.B.  Here, we retain the _PF diffs since they apply to the
		//	enclosing group along with the PF of the group we are leaving
	}
	else
	{
		// We're popping the state, so delete its PF and discard the _PF diffs
		Assert(pstate->pPF);
		pstate->DeletePF();

		// If !pstatePrev, we're ending the parse, in which case the _PF
		//	structure contains final PF (so don't toast it).
		if(pstatePrev)
		{
			_PF.dwMask = 0;
		}
	}

	if(pstatePrev)
	{
		_CF.dwMask = 0;							// Discard any CF deltas

		switch(pstate->sDest)
		{
			case destParaNumbering:
				// {\pn ...}
				pstatePrev->sIndentNumbering = pstate->sIndentNumbering;
				pstatePrev->fBullet = pstate->fBullet;
				break;

			case destObject:
				// clear our object flags just in case we have corrupt RTF
				if(_fNeedPres)
				{
					_fNeedPres = FALSE;
					_fNeedIcon = FALSE;
					_pobj = NULL;
				}
				break;

			case destFontTable:
				if(pstatePrev->sDest == destFontTable)
				{
					// We're actually leaving a sub-group within the \fonttbl
					// group.
					break;
				}

				// We're leaving the {\fonttbl...} group.

#ifdef DEBUG
				_fSeenFontTable = TRUE;
#endif

				// Default font should now be defined, so select it
				//  (this creates CF deltas)
				SetPlain(pstate);

				// Ensure that a document-level codepage has been determined and
				// then scan the font names and retry the conversion to Unicode,
				// if necessary.

				if(_nCodePage == INVALID_CODEPAGE)
				{
					// We haven't determined a document-level codepage
					// from the \ansicpgN tag, nor from the font table
					// \fcharsetN and \cpgN values.  As a last resort,
					// let's use the \deflangN and \deflangfeN tags

					LANGID langid;

					if(_sDefaultLanguageFE != INVALID_LANGUAGE)
					{
						langid = _sDefaultLanguageFE;
					}
					else if(_sDefaultLanguage != INVALID_LANGUAGE &&
								_sDefaultLanguage != sLanguageEnglishUS)
					{
						// _sDefaultLanguage == sLanguageEnglishUS is inreliable
						// in the absence of \deflangfeN.  Many FE RTF writers
						// write \deflang1033 (sLanguageEnglishUS).

						langid = _sDefaultLanguage;
					}
					else 
					{
						goto NoLanguageInfo;
					}

					_nCodePage = ConvertLanguageIDtoCodePage(langid);
				}

NoLanguageInfo:
				if(_nCodePage == INVALID_CODEPAGE)
				{
					break;
				}

				// Fixup mis-converted font face names

				TEXTFONT *ptf;
				UINT i;

				for(i = 0; i < _fonts.Count(); i++)
				{
					ptf = _fonts.Elem(i);

					if(ptf->sCodePage == INVALID_CODEPAGE ||
						ptf->sCodePage == SYMBOL_CODEPAGE)
					{
						if(ptf->fNameIsDBCS)
						{
							char szaTemp[LF_FACESIZE];
							BOOL fMissingCodePage;

							// un-convert mis-converted face name
							SideAssert(WCTMB(ptf->sCodePage, 0, 
												ptf->szName, -1,
												szaTemp, sizeof(szaTemp),
												NULL, NULL, &fMissingCodePage) > 0);
							Assert(ptf->sCodePage == SYMBOL_CODEPAGE || 
										fMissingCodePage);

							// re-convert face name using new codepage info
							SideAssert(MBTWC(_nCodePage, 0,
										szaTemp, -1,
										ptf->szName, sizeof(ptf->szName) / sizeof(ptf->szName[0]),
										&fMissingCodePage) > 0);

							if(!fMissingCodePage)
							{
								ptf->fNameIsDBCS = FALSE;
							}
						}
					}
				}
				break;

			default:;
				// nothing
		}

		_prg->Set_iCF(pstatePrev->iCF);			// Restore previous CharFormat
		ReleaseFormats(pstatePrev->iCF, -1);
	}

done:
	TRACEERRSZSC("HandleEndGroup()", - _ecParseError);
	return _ecParseError;
}

/*
 *	CRTFRead::SelectCurrentFont(iFont)
 *
 *	@mfunc
 *		Set active font to that with index <p iFont>. Take into account
 *		bad font numbers.
 */
void CRTFRead::SelectCurrentFont(
	INT iFont)					// @parm font handle of font to select
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::SelectCurrentFont");

	LONG		i		= _fonts.Count();
	STATE *		pstate	= _pstateStackTop;
	TEXTFONT *	ptf		= _fonts.Elem(0);

	AssertSz(i,	"CRTFRead::SelectCurrentFont: bad font collection");

    if (NULL == pstate)
    {
	    AssertSz(pstate, "CRTFRead::SelectCurrentFont: bad state pointer");
	    return;
    }
    

	
	for(; i-- && iFont != ptf->sHandle; ptf++)	// Search for font with handle
		;										//  iFont

	// Font handle not found: use default, which is valid
	//  since \rtf copied _prg's
	if(i < 0)									
		ptf = _fonts.Elem(0);
												
	BOOL fDefFontFromSystem = (i == (LONG)_fonts.Count() - 1 || i < 0) &&
								!_fReadDefFont;

	wcscpy_s(_CF.szFaceName, sizeof(_CF.szFaceName) / sizeof(_CF.szFaceName[0]), ptf->szName);
	_CF.bInternalMask	 |=  CFMI_FACENAMEISDBCS;
	_CF.bInternalEffects &= ~CFEI_FACENAMEISDBCS;
	if( ptf->fNameIsDBCS )
		_CF.bInternalEffects |= CFEI_FACENAMEISDBCS;

	if(pstate->sDest != destFontTable)
	{
		_CF.bCharSet		= ptf->bCharSet;
		_CF.bPitchAndFamily	= ptf->bPitchAndFamily;
		_CF.dwMask			|= CFM_FACE | CFM_CHARSET;
	}

	// Ensure that the state's codepage is not supplied by the system.
	// That is, if we are using the codepage info from the default font,
	// be sure that the default font info was read from the RTF file.
	pstate->SetCodePage(((fDefFontFromSystem || ptf->sCodePage == INVALID_CODEPAGE) 
						? _nCodePage : ptf->sCodePage));

	pstate->ptf = ptf;

#ifdef CHICAGO
	// Win95c 1719: try to match a language to the char set when RTF
	// 				doesn't explicitly set a language

	if (!pstate->fExplicitLang && ptf->bCharSet != ANSI_CHARSET &&
		(!pstate->sLanguage || pstate->sLanguage == sLanguageEnglishUS))
	{
		i = AttIkliFromCharset(_ped, ptf->bCharSet);
		if (i >= 0)
			pstate->sLanguage = LOWORD(rgkli[i].hkl);
	}
#endif	// CHICAGO
}

/*
 *	CRTFRead::SetPlain()
 *
 *	@mfunc
 *		Setup _CF for \plain
 */
void CRTFRead::SetPlain(STATE *pstate)
{
	ZeroMemory(&_CF, _CF.cbSize);
	_CF.cbSize		= sizeof(CHARFORMAT2);
	_CF.dwMask		= CFM_ALL2;
	_CF.dwEffects	= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR; // Set default effects
	_CF.yHeight		= PointsToFontHeight(yDefaultFontSize);
	if(_sDefaultLanguage == INVALID_LANGUAGE)
	{
		_CF.dwMask &= ~CFM_LCID;
	}
	else
	{
		_CF.lcid = MAKELCID((WORD)_sDefaultLanguage, SORT_DEFAULT);
	}
	_CF.bUnderlineType = CFU_UNDERLINE;
	SelectCurrentFont(_sDefaultFont);

	// TODO: get rid of pstate->sLanguage, since CHARFORMAT2 has lcid
	pstate->sLanguage	  = _sDefaultLanguage;
	pstate->fExplicitLang = FALSE;

#ifdef BIDI
	// Character inherits para's direction
	//$ REVIEW: What happens on \rtlpar\ltrch\plain ? Is this even legal ?
	FlushDirection();
	pstate->fRightToLeft = pstate->fRightToLeftPara;
#endif
}

/*
 *	CRTFRead::ReadFontName(pstate)
 *
 *	@mfunc
 *		read font name _szText into <p pstate>->ptf->szName and deal with
 *		tagged fonts
 */
void CRTFRead::ReadFontName(
	STATE *	pstate,			// @parm state whose font name is to be read into
	int iAllASCII)			// @parm indicates that _szText is all ASCII chars
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadFontName");

	if (pstate->ptf)
	{
		INT		cchName = LF_FACESIZE - 1;
		TCHAR *	pchDst = pstate->ptf->szName;
		char  * pachName =  (char *)_szText ;
        
		// Append additional text from _szText to TEXTFONT::szName

		// We need to append here since some RTF writers decide
		// to break up a font name with other RTF groups
		while(*pchDst && cchName > 0)
		{
			pchDst++;
			cchName--;
		}

		INT cchLimit = cchName;
		while (*pachName &&
			   *pachName != ';' &&
			   cchLimit)		// Remove semicolons
		{
			pachName++;
			cchLimit--;
		}
		*pachName = '\0';

#ifdef PWD_JUPITER
        // GuyBark:
        // The Office converters can output font decorations to the font names. PWord does
        // not want these decorations, as the fonts on the device are capable of displaying
        // characters from multiple character sets. So strip the decorations from the font
        // names if there are any. Note: this is not the same action as RichEdit does later
        // with the font substitution, that looks in the .ini files for replacing some 
        // fonts with other fonts.

        // We don't need to do this if the font's charset is plain ol' Western ANSI.
        if(pstate->ptf->bCharSet)
        {
            int i = 0, cchFont, cchDecoration;

            // Get the length of the incoming font name. All font names here are MBCS.
            cchFont = strlen((LPSTR)_szText);

            // For all the decorations we can expect...
            while(fontDec[i].pszName)
            {
                // Get the length of this decoration. Assume that the decoration is always preceded 
                // by a space. This reduces the chance of doing something the user doesn't expect.

                cchDecoration = strlen(fontDec[i].pszName);

                // If this charset of the incoming font and the decoration match, 
                // check for the decoration in the font name.

                if((pstate->ptf->bCharSet == fontDec[i].charset) &&
                   (cchFont > cchDecoration) &&
                   !strcmp((LPSTR)&_szText[cchFont - cchDecoration], fontDec[i].pszName))
                {
                    // It has the decoration! So chop it off.
                    _szText[cchFont - cchDecoration] = '\0';

                    break;
                }

                // Oh well, try the next font decoration.
                ++i;
            }
        }
#endif // PWD_JUPITER

		// Use the codepage of the font in all cases except where the font uses
		// the symbol charset (and the codepage has been mapped from the charset)
		// and UTF-8 isn't being used
		LONG nCodePage = pstate->nCodePage != SYMBOL_CODEPAGE 
						? pstate->nCodePage : _nCodePage;

		BOOL fMissingCodePage;
		Assert(!(_dwFlags & SFF_UTF8) || nCodePage == CP_UTF8);
		INT cch = MBTWC(nCodePage, 0, 
						(char *)_szText, -1, 
						pchDst, cchName, &fMissingCodePage);

		if(cch > 0 && fMissingCodePage && iAllASCII == CONTAINS_NONASCII)
			pstate->ptf->fNameIsDBCS = TRUE;

		// Make sure destination is null terminated
		if(cch > 0)
			pchDst[cch] = 0;

		// Fall through even if MBTWC <= 0, since we may be appending text to an
		// existing font name.

		if (pstate->ptf == _fonts.Elem(0))		// If it's the default font,
			SelectCurrentFont(_sDefaultFont);	//  update _CF accordingly

		TCHAR *	szNormalName;

		if (pstate->ptf->bCharSet && pstate->fRealFontName)
		{
			// if we don't know about this font don't use the real name
			if (!FindTaggedFont(pstate->ptf->szName,
							pstate->ptf->bCharSet, &szNormalName))
			{
				pstate->fRealFontName = FALSE;
				pstate->ptf->szName[0] = 0;
			}
		}
		else if (IsTaggedFont(pstate->ptf->szName,
							&pstate->ptf->bCharSet, &szNormalName))
		{
			wcscpy_s(pstate->ptf->szName, szNormalName);
			pstate->ptf->sCodePage = GetCodePage(pstate->ptf->bCharSet);
			pstate->SetCodePage(pstate->ptf->sCodePage);
		}
	}
}

/*
 *	CRTFRead::GetColor (dwMask)
 *
 *	@mfunc
 *		Store the autocolor or autobackcolor effect bit and return the
 *		COLORREF for color _iParam
 *
 *	@rdesc
 *		COLORREF for color _iParam
 *
 *	@devnote
 *		If the entry in _colors corresponds to tomAutoColor, gets the value
 *		RGB(0,0,0) (since no \red, \green, and \blue fields are used), but
 *		isn't used by the RichEdit engine.  Entry 1 corresponds to the first
 *		explicit entry in the \colortbl and is usually RGB(0,0,0). The _colors
 *		table is built by HandleToken() when it handles the token tokenText
 *		for text consisting of a ';' for a destination destColorTable.
 */
COLORREF CRTFRead::GetColor(
	DWORD dwMask)		//@parm Color mask bit
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetColor");

	if((DWORD)_iParam >= _colors.Count())		// Illegal _iParam
		return RGB(0,0,0);

	_CF.dwMask	  |= dwMask;					// Turn on appropriate mask bit
	_CF.dwEffects &= ~dwMask;					// auto(back)color off: color is to be used

	COLORREF Color = *_colors.Elem(_iParam);
	if(Color == tomAutoColor)
	{
		_CF.dwEffects |= dwMask;				// auto(back)color on				
		Color = RGB(0,0,0);
	}		
	return Color;
}

/*
 *	CRTFRead::GetStandardColorIndex ()
 *
 *	@mfunc
 *		Return the color index into the standard 16-entry Word \colortbl
 *		corresponding to the color index _iParam for the current \colortbl
 *
 *	@rdesc
 *		Standard color index corresponding to the color associated with _iParam
 */
LONG CRTFRead::GetStandardColorIndex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetColorIndex");

	if((DWORD)_iParam >= _colors.Count())		// Illegal _iParam:
		return 0;								//  use autocolor

	COLORREF Color = *_colors.Elem(_iParam);
	LONG	 i;

	for(i = 0; i < 16; i++)
	{
		if(Color == g_Colors[i])
			return i + 1;
	}
	return 0;									// Not there: use autocolor
}

/*
 *	CRTFRead::HandleChar(ch)
 *
 *	@mfunc
 *		Handle single Unicode character <p ch>
 *
 *	@rdesc
 *		EC			The error code
 */
EC CRTFRead::HandleChar(
	WORD ch)			// @parm char token to be handled
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleChar");

	if(!_ped->TxGetMultiLine() && IsASCIIEOP(ch))
	{
		_ecParseError = ecTruncateAtCRLF;
	}
 	else
	{
		Assert(ch <= 0x7F || ch > 0xFF || FTokIsSymbol(ch));
		_CF.bInternalMask |= CFMI_RUNISDBCS;
		_CF.bInternalEffects &= ~CFEI_RUNISDBCS;
		AddText((TCHAR*)&ch, 1, CharGetsNumbering(ch));
	}

	TRACEERRSZSC("HandleChar()", - _ecParseError);

	return _ecParseError;
}

/*
 *	CRTFRead::HandleEndOfPara()
 *
 *	@mfunc
 *		Insert EOP and apply current paraformat
 *
 *	@rdesc
 *		EC	the error code
 */
EC CRTFRead::HandleEndOfPara()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleEndOfPara");

	if(_pstateStackTop->fInTable)			// Our simple table model can't
	{										//  have numbering
		_PF.wNumbering = 0;	
		_PF.dwMask |= PFM_NUMBERING;
	}

	if(!_ped->TxGetMultiLine())				// No EOPs permitted in single-
	{										//  line controls
		Apply_PF();							// Apply any paragraph formatting
		_ecParseError = ecTruncateAtCRLF;	// Cause RTF reader to finish up
		return ecTruncateAtCRLF;
	}

	Apply_CF();								// Apply _CF and save iCF, since
	LONG iFormat = _prg->Get_iCF();			//  it may get changed to iCF
											//  that follows EOP
	EC ec  = _ped->Get10Mode()				// If RichEdit 1.0 compatibility
			? HandleText((BYTE *)szaCRLF, ALL_ASCII)	//  mode, use CRLF; else CR or VT
			: HandleChar((unsigned)(_token == tokenLineBreak ? VT : CR));

	if(ec == ecNoError)
	{
		Apply_PF();
		_cpThisPara = _prg->GetCp();		// New para starts after CRLF
	}
	_prg->Set_iCF(iFormat);					// Restore iFormat if changed
	ReleaseFormats(iFormat, -1);			// Release iFormat (AddRef'd by
											//  Get_iCF())
	return _ecParseError;
}

/*
 *	CRTFRead::HandleText(szText, iAllASCII)
 *
 *	@mfunc
 *		Handle the string of Unicode characters <p szText>
 *
 *	@rdesc
 *		EC			The error code
 */
EC CRTFRead::HandleText(
	BYTE * szText,			// @parm string to be handled
	int iAllASCII)			// @parm enum indicating if string is all ASCII chars
							// 	(<= 0x7F)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleText");

	LONG	cch;
       LONG     cch_szUnicode = cachTextMax;
	TCHAR *	pch;
	STATE *	pstate = _pstateStackTop;

	// TODO: what if szText cuts off in middle of DBCS?

	if(!*szText)
	{
		goto CleanUp;
	}

	if(iAllASCII == ALL_ASCII || pstate->nCodePage == SYMBOL_CODEPAGE)
	{
		// Don't use MBTWC() in cases where text contains
		// only ASCII chars (which don't require conversion)
		for(cch = 0, pch = _szUnicode; *szText && (cch < cch_szUnicode); cch++)
		{
			Assert(*szText <= 0x7F || _CF.bCharSet == SYMBOL_CHARSET);
			*pch++ = (TCHAR)*szText++;
		}
              *pch = 0;

		_CF.bInternalMask |= CFMI_RUNISDBCS;
		_CF.bInternalEffects &= ~CFEI_RUNISDBCS;

		// fall through to AddText at end of HandleText()
	}
	else
	{
		TEXTFONT *ptf;
		BOOL fMissingCodePage;

		ptf = pstate->ptf;

		// Run of text contains bytes > 0x7F.
		// Ensure that we have the correct codepage with which to interpret 
		// these (possibly DBCS) bytes.

		if(ptf->sCodePage == INVALID_CODEPAGE && !ptf->fCpgFromSystem)
		{
			// Determine codepage from the font name
			if(CpgInfoFromFaceName(pstate->ptf))
			{
				SelectCurrentFont(pstate->ptf->sHandle);
				Assert(ptf->sCodePage != INVALID_CODEPAGE);
			}
			else
			{
				// Here, we were not able to determine a cpg/charset value
				// from the font name.  We have two choices: (1) either choose
				// some fallback value like 1252/0 or (2) rely on the 
				// document-level cpg value.
				//
				// I think choosing the document-level cpg value will give
				// us the best results.  In FE cases, it will likely err
				// on the side of tagging too many runs as CFEI_RUNISDBCS, but
				// that is safer than using a western cpg and potentially missing
				// runs which should be CFEI_RUNISDBCS.
			}

			Assert(ptf->fCpgFromSystem);
		}

		Assert(!(_dwFlags & SFF_UTF8) || pstate->nCodePage == CP_UTF8);

#ifdef PWD_JUPITER
		// GuyBark JupiterJ:
		// Always treat a MBCS string which is invalid for the code page
		// as something that should generate an error. Otherwise we lose
		// the text. At least when the error is trapped we take action 
		// to try to compensate.
		cch = MBTWC(pstate->nCodePage, MB_ERR_INVALID_CHARS,
#else
		cch = MBTWC(pstate->nCodePage, 0,
#endif // PWD_JUPITER
					(char *)szText,	-1, 
					_szUnicode, cachTextMax, &fMissingCodePage);

		AssertSz(cch > 0, "CRTFRead::HandleText():  MBTWC implementation changed"
							" such that it returned a value <= 0");

        if(!fMissingCodePage || pstate->nCodePage == INVALID_CODEPAGE)
		{
			// Use result of MBTWC if:
			//	(1) we converted some chars and did the conversion with the codepage
			//		provided.
			//	(2) we converted some chars but couldn't use the codepage provided,
			//		but the codepage is invalid.  Since the codepage is invalid,
			//		we can't do anything more sophisticated with the text before
			//		adding to the backing store

			cch--;  // don't want char count to including terminating NULL

			_CF.bInternalMask |= CFMI_RUNISDBCS;
			if(fMissingCodePage)
			{
				_CF.bInternalEffects |= CFEI_RUNISDBCS;
			}
			else
			{
				_CF.bInternalEffects &= ~CFEI_RUNISDBCS;
			}

			// fall through to AddText at end of HandleText()
		}
		else
		{
			// Conversion to Unicode failed.  Break up the string of
			// text into runs of ASCII and non-ASCII characters.

			// FUTURE(BradO):  Here, I am saving dwMask and restoring it before
			//		each AddText.  I'm not sure this is neccessary.  When I have
			//		the time, I should revisit this save/restoring and 
			//		determine that it is indeed neccessary.

			BOOL fPrevIsASCII = ((*szText <= 0x7F) ? TRUE : FALSE);
			BOOL fCurrentIsASCII = FALSE;
			BOOL fLastChunk = FALSE;
			DWORD dwMaskSave = _CF.dwMask;
#ifdef DEBUG
			CCharFormat cfSave = _CF;
#endif

			pch = _szUnicode;
			cch = 0;

			// (!*szText && *pch) is the case where we do the AddText for the
			//	last chunk of text
			while((*szText || fLastChunk) && (cch<cch_szUnicode))
			{
				// fCurrentIsASCII assumes that no byte <= 0x7F is a 
				//	DBCS lead-byte
				if(fLastChunk ||
					(fPrevIsASCII != (fCurrentIsASCII = ((*szText <= 0x7F) ? 
															TRUE : FALSE))))
				{
					_CF.dwMask = dwMaskSave;
#ifdef DEBUG
					_CF = cfSave;
#endif

					*pch = 0;

					_CF.bInternalMask |= CFMI_RUNISDBCS;
					if( fPrevIsASCII )
					{
						_CF.bInternalEffects &= ~CFEI_RUNISDBCS;
					}
					else
					{
						_CF.bInternalEffects |= CFEI_RUNISDBCS;
					}

					Assert(cch);
					pch = _szUnicode;

					AddText(pch, cch, TRUE);

					cch = 0;
					fPrevIsASCII = fCurrentIsASCII;

					// My assumption in saving _CF.dwMask is that the remainder
					// of the _CF is unchanged by AddText.  This assert verifies
					// this assumption.
					AssertSz(!memcmp(&cfSave.dwEffects, &_CF.dwEffects,
										((BYTE *)((CHARFORMAT2 *)&_CF + 1)) - 
											(BYTE *)&_CF.dwEffects),
						"CRTFRead::HandleText():  AddText has been changed "
						"and now alters the _CF structure.");

					if(fLastChunk)
					{
						// Last chunk of text was AddText'd
						break;
					}
				}

				// Not the last chunck of text.
				Assert(*szText);

				// Advance szText pointer
				if(!fCurrentIsASCII && *(szText + 1) && 
						IsLeadByte(*szText, pstate->nCodePage))
				{
					// current byte is a lead-byte of a DBCS character
					*pch++ = *szText++;
					++cch;
				}
                            if (cch<cch_szUnicode)
                            {
                                *pch++ = *szText++;
				    ++cch;
                            }
				// Must do an AddText for the last chunk of text.
				if(!*szText)
				{
					fLastChunk = TRUE;
				}
			}
			goto CleanUp;
		}
	}

	if(cch > 0)
	{
		pch = _szUnicode;
		AddText(pch, cch, TRUE);
	}

CleanUp:
	TRACEERRSZSC("HandleText()", - _ecParseError);
	return _ecParseError;
}

/*
 *	CRTFRead::AddText(pch, cch, fNumber)
 *	
 *	@mfunc
 *		Add <p cch> chars of the string <p pch> to the range _prg
 *
 *	@rdesc
 *		error code placed in _ecParseError
 */
EC CRTFRead::AddText(
	TCHAR *	pch,		// @parm text to add
	LONG	cch,		// @parm count of chars to add
	BOOL	fNumber)	// @parm indicates whether or not to prepend numbering
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::AddText");

	LONG			cchAdded;
	LONG			cchT;
	STATE *	const	pstate = _pstateStackTop;
	TCHAR *			szDst;
	DWORD			cchLen = _ped->GetAdjustedTextLength();

	// AROO: No saving state before this point (other than pstate)
	// AROO: This would mess up recursion below

	AssertSz(pstate, "CRTFRead::AddText: no state");

	if((DWORD)cch > _cchMax)
	{
		cch = (LONG)_cchMax;
		_ecParseError = ecTextMax;
	}

	if (!cch)
		return _ecParseError;

	// FUTURE(BradO):  This strategy for \pntext is prone to bugs, I believe.
	// The recursive call to AddText to add the \pntext will trounce the 
	// accumulated _CF diffs associated with the text for which AddText is
	// called.  I believe we should save and restore _CF before and after
	// the recursive call to AddText below.  Also, it isn't sufficient to
	// accumulate bits of \pntext as below, since each bit might be formatted
	// with different _CF properties.  Instead, we should accumulate a mini-doc
	// complete with multiple text, char and para runs (or some stripped down
	// version of this strategy).

	if (pstate->sDest == destParaNumText)
	{
		szDst = _szNumText + _cchUsedNumText;
		cch = min(cch, cchMaxNumText - 1 - _cchUsedNumText);
		if (cch > 0)
		{
			MoveMemory((BYTE *)szDst, (BYTE *)pch, cch*2);
			szDst[cch] = TEXT('\0');		// HandleText() takes sz
			_cchUsedNumText += cch;
		}
		return ecNoError;
	}

	if(_cchUsedNumText && fNumber)			// Some \pntext available
	{
		// Bug 3496 - The fNumber flag is an ugly hack to work around RTF 
		//	commonly written by Word.  Often, to separate a numbered list
		// 	by page breaks, Word will write:
		//		<NUMBERING INFO> \page <PARAGRAPH TEXT>
		// 	The paragraph numbering should precede the paragraph text rather
		//	than the page break.  The fNumber flag is set to FALSE when the
		//	the text being added should not be prepended with the para-numbering,
		//	as is the case with \page (mapped to FF).

		cchT = _cchUsedNumText;
		_cchUsedNumText = 0;				// Prevent infinite recursion

		if(!pstate->fBullet)
		{
			// If there are any _CF diffs to be injected, they will be trounced
			// by this recursive call (see FUTURE comment above).

			// Since we didn't save _CF data from calls to AddText with
			// pstate->sDest == destParaNumText, we have no way of setting up
			// CFEI_RUNISDBCS and CFMI_RUNISDBCS (see FUTURE comment above).

			AddText(_szNumText, cchT, FALSE);
		}
		else if(_PF.IsListNumbered() && _szNumText[cchT - 1] == TAB)
		{
			WCHAR ch = _szNumText[cchT - 2];

			_wNumberingStyle = (_wNumberingStyle & ~0xF00)
				 | (ch == '.' ? PFNS_PERIOD : 
					ch != ')' ? PFNS_PLAIN  :
					_szNumText[0] == '(' ? PFNS_PARENS : PFNS_PAREN);

		}
	}

	if (_cpFirst && _prg->GetCp() == _cpFirst && _prg->GetPF()->InTable() &&
		_cCell && !_prg->_rpTX.IsAfterEOP())
	{
		// FUTURE: handle more general table insertions into other tables
		return _ecParseError = ecGeneralFailure;
	}

	Apply_CF();								// Apply formatting changes in _CF

	// BUGS 1577 & 1565 - 
	// CTxtRange::ReplaceRange will change the character formatting
	// and possibly adjust the _rpCF forward if the current char
	// formatting includes protection.  The changes affected by 
	// CTxtRange::ReplaceRange are necessary only for non-streaming 
	// input, so we save state before and restore it after the call 
	// to CTxtRange::ReplaceRange

	LONG iFormatSave = _prg->Get_iCF();		// Save state

	cchAdded = _prg->ReplaceRange(cch, pch, NULL, SELRR_IGNORE);

	_prg->Set_iCF(iFormatSave);				// Restore state 
	ReleaseFormats(iFormatSave, -1);
	Assert(_prg->GetCch() == 0);

	if (cchAdded != cch)
	{
		Tracef(TRCSEVERR, "AddText(): Only added %d out of %d", cchAdded, cch);
		_ecParseError = ecGeneralFailure;
		if (cchAdded <= 0)
			return _ecParseError;
	}
	_cchMax -= cchAdded;

	return _ecParseError;
}

/*
 *	CRTFRead::Apply_CF()
 *	
 *	@mfunc
 *		Apply character formatting changes collected in _CF
 */
void CRTFRead::Apply_CF()
{
	// If any CF changes, update range's _iFormat
	if( _CF.dwMask || _CF.bInternalMask )		
	{
		AssertSz(_prg->GetCch() == 0,
			"CRTFRead::Apply_CF: nondegenerate range");

		_prg->SetCharFormat(&_CF, FALSE, NULL);	// Note: nothing here to undo
		_CF.dwMask = 0;							//  i.e., only changed char
												//  format of insertion point
		_CF.bInternalMask = 0;
	}
}

/*
 *	CRTFRead::Apply_PF()
 *	
 *	@mfunc
 *		Apply paragraph format given by _PF
 */
void CRTFRead::Apply_PF()
{
	LONG cp;

	if(_pstateStackTop)
	{
		Assert(_pstateStackTop->pPF);

		// Add PF diffs to *_pstateStackTop->pPF
		if(!_pstateStackTop->AddPF(_PF, _sDefaultTabWidth, _bDocType))
		{
			_ped->GetCallMgr()->SetOutOfMemory();
			_ecParseError = ecNoMemory;
			return;
		}
		_PF.dwMask = 0;  // _PF contains delta's from *_pstateStackTop->pPF

		CParaFormat *pPF = _pstateStackTop->pPF;

		Assert(pPF->dwMask == PFM_ALLRTF);
		if(pPF->wNumbering)
		{
			pPF->wNumberingTab	 = _pstateStackTop->sIndentNumbering;
			pPF->wNumberingStyle = _wNumberingStyle;
		}
	}
		
	cp = _prg->GetCp();
	_prg->Set(cp, cp - _cpThisPara);		// Select back to _cpThisPara
	_prg->SetParaFormat(_pstateStackTop ? _pstateStackTop->pPF : &_PF, NULL);
	_prg->Set(cp, 0);						// Restore _prg to an IP
}

/*
 *	CRTFRead::SetBorderParm(&Parm, Value)
 *
 *	@mfunc
 *		Set the border pen width in half points for the current border
 *		(_bBorder)
 */
void CRTFRead::SetBorderParm(
	WORD&	Parm,
	LONG	Value)
{
	Assert(_bBorder <= 3);

	Value = min(Value, 15);
	Value = max(Value, 0);
	Parm &= ~(0xF << 4*_bBorder);
	Parm |= Value << 4*_bBorder;
	_PF.dwMask |= PFM_BORDER;
}

/*
 *	CRTFRead::HandleToken()
 *
 *	@mfunc
 *		Grand switch board that handles all tokens. Switches on _token
 *
 *	@rdesc
 *		EC		The error code
 *
 *	@comm
 *		Token values are chosen contiguously (see tokens.h and tokens.c) to
 *		encourage the compiler to use a jump table.  The lite-RTF keywords
 *		come first, so that	an optimized OLE-free version works well.  Some
 *		groups of keyword tokens are ordered so as to simplify the code, e.g,
 *		those for font family names, CF effects, and paragraph alignment.
 */
EC CRTFRead::HandleToken()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleToken");

	BYTE				bT;						// Temporary BYTE
	DWORD				dwT;					// Temporary DWORD
	LONG				dy, i;
	LONG				iParam = _iParam;
	const CCharFormat *	pcf;
	COLORREF *			pclrf;
	STATE *				pstate = _pstateStackTop;
	TEXTFONT *			ptf;
	WORD				wT;						// Temporary WORD

#if defined(DEBUG)
	if(!_fTestingParserCoverage)
#endif
	{
		if(_cbSkipForUnicode &&
			_token != tokenText &&
			_token != tokenStartGroup &&
			_token != tokenEndGroup &&
			_token != tokenBinaryData)
		{
			_cbSkipForUnicode--;
			goto done;
		}
	}

	switch (_token)
	{
	case tokenPocketWord:						// \pwd
		_dwFlags |= SFF_PWD;

	case tokenRtf:								// \rtf
		PARSERCOVERAGE_CASE();
		pstate->sDest = destRTF;
		if (!_fonts.Count() && !_fonts.Add(1, NULL))	// If can't add a font,
			goto OutOfRAM;						//  report the bad news
		_sDefaultFont = 0;						// Set up valid default font
		ptf = _fonts.Elem(0);
		pstate->ptf			  = ptf;			// Get char set, pitch, family
		pcf					  = _prg->GetCF();	//  from current range font
		ptf->bCharSet		  = pcf->bCharSet;	// These are guaranteed OK
		ptf->bPitchAndFamily  = pcf->bPitchAndFamily;
		ptf->sCodePage		  = GetCodePage(pcf->bCharSet);
		Assert(pstate->nCodePage == INVALID_CODEPAGE);
		wcscpy_s(ptf->szName, pcf->szFaceName);
		ptf->fNameIsDBCS = (pcf->bInternalEffects & CFEI_FACENAMEISDBCS);

		pstate->cbSkipForUnicodeMax = iUnicodeCChDefault;
		break;

	case tokenViewKind:							// \viewkind N
		if(!(_dwFlags & SFF_SELECTION))			// RTF applies to document:
			_ped->SetViewKind(iParam);
		break;

	case tokenViewScale:						// \viewscale N
		if(!(_dwFlags & SFF_SELECTION))			// RTF applies to document:
			_ped->SetViewScale(iParam);
		break;

	case tokenCharacterDefault:					// \plain
		PARSERCOVERAGE_CASE();

		dy = _CF.yOffset;

		SetPlain(pstate);

		// GuyBark JupiterJ: Special case the offset to preserve it in a field equation.
#ifdef PWD_JUPITER
        if(_fEquationField)
        {
            _CF.yOffset = dy;
        }
#endif
		
		break;

	case tokenCharSetAnsi:						// \ansi
		PARSERCOVERAGE_CASE();
		_bCharSet = 0;
		break;

	case tokenUTF:								// \utf
		PARSERCOVERAGE_CASE();
		if(iParam == 8)
		{
			_dwFlags |= SFF_UTF8;				// Save bit for Asserts
			pstate->SetCodePage(CP_UTF8);
		}
		break;

	case tokenDefaultLanguage:					// \deflang
		PARSERCOVERAGE_CASE();
		_sDefaultLanguage = (SHORT)iParam;
		if (!pstate->sLanguage)
			pstate->sLanguage = _sDefaultLanguage;
		break;

	case tokenDefaultLanguageFE:				// \deflangfe
		PARSERCOVERAGE_CASE();
		_sDefaultLanguageFE = (SHORT)iParam;
		break;

	case tokenDefaultTabWidth:					// \deftab
		PARSERCOVERAGE_CASE();
		_sDefaultTabWidth = (SHORT)iParam;
		if (!pstate->sDefaultTabWidth)
			pstate->sDefaultTabWidth = _sDefaultTabWidth;
		break;


//--------------------------- Font Control Words -------------------------------

	case tokenDefaultFont:						// \deff n
		PARSERCOVERAGE_CASE();
		if(iParam >= 0)
			_fonts.Elem(0)->sHandle = _sDefaultFont = (SHORT)iParam;
		TRACEERRSZSC("tokenDefaultFont: Negative value", iParam);
		break;

	case tokenFontTable:						// \fonttbl
		PARSERCOVERAGE_CASE();
		pstate->sDest = destFontTable;
		pstate->ptf = NULL;
		break;

	case tokenFontFamilyBidi:					// \fbidi
	case tokenFontFamilyTechnical:				// \ftech
	case tokenFontFamilyDecorative:				// \fdecor
	case tokenFontFamilyScript:					// \fscript
	case tokenFontFamilyModern:					// \fmodern
	case tokenFontFamilySwiss:					// \fswiss
	case tokenFontFamilyRoman:					// \froman
	case tokenFontFamilyDefault:				// \fnil
		PARSERCOVERAGE_CASE();
		AssertSz(tokenFontFamilyRoman - tokenFontFamilyDefault == 1,
			"CRTFRead::HandleToken: invalid token definition"); 

		if (pstate->ptf)
		{
			pstate->ptf->bPitchAndFamily
				= (_token - tokenFontFamilyDefault) << 4
				| (pstate->ptf->bPitchAndFamily & 0xF);

			// setup SYMBOL_CHARSET charset for \ftech if there isn't any chaset info
			if ( tokenFontFamilyTechnical == _token && pstate->ptf->bCharSet == DEFAULT_CHARSET )
			{	 
				pstate->ptf->bCharSet = SYMBOL_CHARSET;
			}
		}
		break;

	case tokenPitch:							// \fprq
		PARSERCOVERAGE_CASE();
		if(pstate->ptf)
			pstate->ptf->bPitchAndFamily
				= iParam |	(pstate->ptf->bPitchAndFamily & 0xF0);
		break;

	case tokenAnsiCodePage:						// \ansicpg
		PARSERCOVERAGE_CASE();
#ifdef DEBUG
		if(_fSeenFontTable && _nCodePage == INVALID_CODEPAGE)
			TRACEWARNSZ("CRTFRead::HandleToken():  Found an \ansicpgN tag after "
							"the font table.  Should have code to fix-up "
							"converted font names and document text.");
#endif
		_nCodePage = iParam;
		Assert(!(_dwFlags & SFF_UTF8) || pstate->nCodePage == CP_UTF8);
		pstate->SetCodePage(iParam);
		break;

	case tokenCodePage:							// \cpg
		PARSERCOVERAGE_CASE();
		pstate->SetCodePage(iParam);
		if(pstate->sDest == destFontTable && pstate->ptf)
		{
			pstate->ptf->sCodePage = (SHORT)iParam;
			pstate->ptf->bCharSet = GetCharSet(iParam);

			// also, if a document-level code page has not been specified
			// grab this from the first font table entry containing a 
			// \fcharsetN or \cpgN
			if(_nCodePage == INVALID_CODEPAGE)
			{
				_nCodePage = (SHORT)iParam;
			}
		}
		break;

	case tokenCharSet:							// \fcharset n
		PARSERCOVERAGE_CASE();
		if(pstate->sDest == destFontTable && pstate->ptf)
		{
			pstate->ptf->bCharSet = (BYTE) iParam;
			pstate->ptf->sCodePage = GetCodePage((BYTE)iParam);
			pstate->SetCodePage(pstate->ptf->sCodePage);

			// also, if a document-level code page has not been specified
			// grab this from the first font table entry containing a 
			// \fcharsetN or \cpgN
			if (pstate->nCodePage != SYMBOL_CODEPAGE && 
				_nCodePage == INVALID_CODEPAGE)
			{
				_nCodePage = pstate->nCodePage;
			}
		}
		break;

	case tokenRealFontName:						// \fname
		PARSERCOVERAGE_CASE();
		pstate->sDest = destRealFontName;
		break;

	case tokenFontSelect:						// \f n
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destFontTable)		// Building font table
		{
			if (iParam == _sDefaultFont)
			{
				_fReadDefFont = TRUE;
				ptf = _fonts.Elem(0);
			}
			else if (!(ptf =_fonts.Add(1,NULL)))// Make room in font table for
			{									//  font to be parsed
OutOfRAM:
				_ped->GetCallMgr()->SetOutOfMemory();
				_ecParseError = ecNoMemory;
				break;
			}
			pstate->ptf		= ptf;
			ptf->sHandle	= (SHORT)iParam;	// Save handle
			ptf->szName[0]	= '\0';				// Start with null string so
			ptf->bPitchAndFamily = 0;
			ptf->fNameIsDBCS = FALSE;
			ptf->sCodePage = INVALID_CODEPAGE;
			ptf->fCpgFromSystem = FALSE;
			bT = DEFAULT_CHARSET;				//  appending works right

#ifdef DBCS
			switch(HIWORD(UlGGetLang())			// TODO: use table lookup
			{
				case LangJpn:
					bT = SHIFTJIS_CHARSET;
					break;
				case LangKor:
					bT = HANGEUL_CHARSET;
					break;
				case LangCht:
					bT = CHINESEBIG5_CHARSET;
					break;
				case LangPrc:
					bT = GB2312_CHARSET;
					break;
				default:
					bT = DEFAULT_CHARSET;
			}			
#endif
			ptf->bCharSet = bT;

#ifdef PWD_JUPITER

			// GuyBark JupiterJ 50029: Some RTF we get has no character set info 
			// in the font table. This will mess up the text handling later. So 
			// assume that the character set is the default system one. This will
			// get overridden later if we find the true character set info the 
			// the font table later. Note: All we need to do is set the code page
			// in the font data here. This will get used later in the text handling
			// and it doesn't matter that the character set is still set to default.
			// We're using the default system locale here, so we're not guaranteed 
			// to get the correct code page, but we guessing at what the missing 
			// info is anyway.
            
			ptf->sCodePage = ConvertLanguageIDtoCodePage( GetSystemDefaultLCID());
			ptf->bCharSet  = GetCharSet(ptf->sCodePage);

			pstate->SetCodePage(ptf->sCodePage);

#endif // PWD_JUPITER
		}
		else									// Font switch in text
			SelectCurrentFont(iParam);
		break;

	case tokenFontSize:							// \fs
		PARSERCOVERAGE_CASE();
		_CF.yHeight = PointsToFontHeight(iParam);	// Convert font size in
		_CF.dwMask |= CFM_SIZE;					//  half points to logical
		break; 									//  units

	// NOTE: \*\fontemb and \*\fontfile are discarded. The font mapper will
	//		 have to do the best it can given font name, family, and pitch.
	//		 Embedded fonts are particularly nasty because legal use should
	//		 only support read-only which parser cannot enforce.

	case tokenLanguage:							// \lang
		PARSERCOVERAGE_CASE();
		pstate->sLanguage = (SHORT)iParam;		// These 2 lines may not be
		pstate->fExplicitLang = TRUE;			//  needed with the new lcid
		_CF.lcid = MAKELCID(iParam, SORT_DEFAULT);
		_CF.dwMask |= CFM_LCID;
		break;


//-------------------------- Color Control Words ------------------------------

	case tokenColorTable:						// \colortbl
		PARSERCOVERAGE_CASE();
		pstate->sDest = destColorTable;
		_fGetColorYet = FALSE;
		break;

	case tokenColorRed:							// \red
		PARSERCOVERAGE_CASE();
#ifdef PWD_JUPITER
        // GuyBark Jupiter 37275: Special case output from the Office converters.
        pstate->bRed = (iParam == 127 ? 128 : (BYTE)iParam);
#else
        pstate->bRed = (BYTE)iParam;
#endif // PWD_JUPITER
        _fGetColorYet = TRUE;
		break;

	case tokenColorGreen:						// \green
		PARSERCOVERAGE_CASE();
#ifdef PWD_JUPITER
		pstate->bGreen = (iParam == 127 ? 128 : (BYTE)iParam);
#else
		pstate->bGreen = (BYTE)iParam;
#endif // PWD_JUPITER
		_fGetColorYet = TRUE;
		break;

	case tokenColorBlue:						// \blue
		PARSERCOVERAGE_CASE();
#ifdef PWD_JUPITER
		pstate->bBlue = (iParam == 127 ? 128 : (BYTE)iParam);
#else
		pstate->bBlue = (BYTE)iParam;
#endif // PWD_JUPITER
		_fGetColorYet = TRUE;
		break;

	case tokenColorForeground:					// \cf
		PARSERCOVERAGE_CASE();
		_CF.crTextColor = GetColor(CFM_COLOR);
// V-GUYB: Table cell backgrounds (\clcbpat) are not handled in Richedit 2.0.
// This means all table cells will have a white background. Therefore change
// any white text to black here.
		if (_pstateStackTop->fInTable && _CF.crTextColor == RGB(0xFF, 0xFF, 0xFF))
		{
			_CF.crTextColor = RGB(0x00, 0x00, 0x00);
		}
		break;

	case tokenColorBackground:					// \highlight
		PARSERCOVERAGE_CASE();
		_CF.crBackColor = GetColor(CFM_BACKCOLOR);
		break;

	case tokenExpand:							// \expndtw N
		PARSERCOVERAGE_CASE();
		_CF.sSpacing = (SHORT) iParam;
		_CF.dwMask |= CFM_SPACING;
		break;


	case tokenCharStyle:						// \cs N
		PARSERCOVERAGE_CASE();
 		/*	FUTURE (alexgo): we may want to support character styles
		in some future version.
		_CF.sStyle = (SHORT)iParam;
		_CF.dwMask |= CFM_STYLE;  */

		if(pstate->sDest == destStyleSheet)
			goto skip_group;
		break;			   

	case tokenAnimText:							// \animtext N
		PARSERCOVERAGE_CASE();
		_CF.bAnimation = (BYTE)iParam;
		_CF.dwMask |= CFM_ANIMATION;
		break;

	case tokenKerning:							// \kerning N
		PARSERCOVERAGE_CASE();
		_CF.wKerning = (WORD)(10 * iParam);		// Convert to twips
		_CF.dwMask |= CFM_KERNING;
		break;

	case tokenFollowingPunct:					// \*\fchars
		PARSERCOVERAGE_CASE();
		pstate->sDest = destFollowingPunct;
		break;

	case tokenLeadingPunct:						// \*\lchars
		PARSERCOVERAGE_CASE();
		pstate->sDest = destLeadingPunct;
		break;

	case tokenDocumentArea:						// \info
		PARSERCOVERAGE_CASE();
		pstate->sDest = destDocumentArea;
		break;

#ifdef FE
	USHORT		usPunct;						// Used for FE word breaking

	case tokenNoOverflow:						// \nooverflow
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Overflow");
		usPunct = ~WBF_OVERFLOW;
		goto setBrkOp;

	case tokenNoWordBreak:						// \nocwrap
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Word Break" );
		usPunct = ~WBF_WORDBREAK;
		goto setBrkOp;

	case tokenNoWordWrap:						// \nowwrap
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("No Word Word Wrap" );
		usPunct = ~WBF_WORDWRAP;

setBrkOp:
		if (!(_dwFlags & fRTFFE))
		{
			usPunct &= UsVGetBreakOption(_ped->lpPunctObj);
			UsVSetBreakOption(_ped->lpPunctObj, usPunct);
		}
		break;

	case tokenVerticalRender:					// \vertdoc
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("Vertical" );
		if (pstate->sDest == destDocumentArea && !(_dwFlags & fRTFFE))
			_ped->fModeDefer = TRUE;
		break;

	case tokenHorizontalRender:					// \horzdoc
		PARSERCOVERAGE_CASE();
		TRACEINFOSZ("Horizontal" );
		if (pstate->sDest == destDocumentArea && !(_dwFlags & fRTFFE))
			_ped->fModeDefer = FALSE;
		break;

#endif
//-------------------- Character Format Control Words -----------------------------

	case tokenUnderlineHairline:				// \ulhair			[10]
	case tokenUnderlineThick:					// \ulth			[9]
	case tokenUnderlineWave:					// \ulwave			[8]
	case tokenUnderlineDashDotDotted:			// \uldashdd		[7]
	case tokenUnderlineDashDotted:				// \uldashd			[6]
	case tokenUnderlineDash:					// \uldash			[5]
	case tokenUnderlineDotted:					// \uld				[4]
	case tokenUnderlineDouble:					// \uldb			[3]
	case tokenUnderlineWord:					// \ulw				[2]
		PARSERCOVERAGE_CASE();
		_CF.bUnderlineType = _token - tokenUnderlineWord + 2;
		_token = tokenUnderline;				// Except for their type, these
		goto under;								//  control words behave like
												//  \ul (try them with Word)
	case tokenUnderline:						// \ul			[Effect 4]
		PARSERCOVERAGE_CASE();					//  (see handleCF)
		_CF.bUnderlineType = CFU_UNDERLINE;		// Fall thru to \b
under:	_CF.dwMask |= CFM_UNDERLINETYPE;
		goto handleCF;

	// These effects are turned on if their control word parameter is missing
	// or nonzero. They are turned off if the parameter is zero. This
	// behavior is usually identified by an asterisk (*) in the RTF spec.
	// The code uses fact that CFE_xxx = CFM_xxx
	case tokenDeleted:							// \deleted
		PARSERCOVERAGE_CASE();
		_token = tokenStrikeOut;				// fall through and handle 
												// this as a tokenStrikeOut

handleCF:
	case tokenRevised:							// \revised			[4000]
	case tokenDisabled:							// \disabled		[2000]
	case tokenImprint:							// \impr			[1000]
	case tokenEmboss:							// \embo			 [800]
 	case tokenShadow:							// \shad			 [400]
	case tokenOutline:							// \outl			 [200]
	case tokenHiddenText:						// \v				 [100]
	case tokenCaps:								// \caps			  [80]
	case tokenSmallCaps:						// \scaps			  [40]
	case tokenLink:								// \link			  [20]
	case tokenProtect:							// \protect			  [10]
	case tokenStrikeOut:						// \strike			   [8]
	case tokenItalic:							// \i				   [2]
	case tokenBold:								// \b				   [1]
		PARSERCOVERAGE_CASE();
		dwT = 1 << (_token - tokenBold);		// Generate effect mask
		_CF.dwEffects &= ~dwT;					// Default attribute off
		if (!*_szParam || _iParam)				// Effect is on
			_CF.dwEffects |= dwT;
		_CF.dwMask |= dwT;						// In either case, the effect
		break;									//  is defined

	case tokenStopUnderline:					// \ulnone
		PARSERCOVERAGE_CASE();
		_CF.dwEffects &= ~CFE_UNDERLINE;		// Kill all underlining
		_CF.dwMask |= CFM_UNDERLINE;
		break;

	case tokenRevAuthor:						// \revauth
		PARSERCOVERAGE_CASE();
		/* FUTURE: (alexgo) this doesn't work well now since we don't support
		revision tables.  We may want to support this better in the future. 
		So what we do now is the 1.0 technique of using a color for the
		author */
		if( iParam > 0 )
		{
			_CF.dwEffects &= ~CFE_AUTOCOLOR;
			_CF.dwMask |= CFM_COLOR;
			_CF.crTextColor = rgcrRevisions[(iParam - 1) & REVMASK];
		}
		break;

	case tokenUp:								// \up
		PARSERCOVERAGE_CASE();
		dy = 10;
		goto StoreOffset;

	case tokenDown:								// \down
		PARSERCOVERAGE_CASE();
		dy = -10;

StoreOffset:
		if (!*_szParam)
			iParam = dyDefaultSuperscript;
		_CF.yOffset = iParam * dy;				// Half points->twips
		_CF.dwMask |= CFM_OFFSET;
		break;

	case tokenSuperscript:						// \super
		PARSERCOVERAGE_CASE();
	     dwT = CFE_SUPERSCRIPT; 
		 goto SetSubSuperScript;

	case tokenSubscript:						// \sub
		PARSERCOVERAGE_CASE();
		 dwT = CFE_SUBSCRIPT;
		 goto SetSubSuperScript;

	case tokenNoSuperSub:						// \nosupersub
		PARSERCOVERAGE_CASE();
		 dwT = 0;
SetSubSuperScript:
		 _CF.dwMask	   |=  (CFE_SUPERSCRIPT | CFE_SUBSCRIPT);
		 _CF.dwEffects &= ~(CFE_SUPERSCRIPT | CFE_SUBSCRIPT);
		 _CF.dwEffects |= dwT;
		 break;



//--------------------- Paragraph Control Words -----------------------------

	case tokenStyleSheet:						// \stylesheet
		PARSERCOVERAGE_CASE();
		pstate->sDest = destStyleSheet;
		_Style = 0;								// Default normal style
		break;

	case tokenTabBar:							// \tb
		PARSERCOVERAGE_CASE();
		_bTabType = PFT_BAR;					// Fall thru to \tx

	case tokenTabPosition:						// \tx
		PARSERCOVERAGE_CASE();
		if(!pstate->fInTable)
		{
			_PF.AddTab(iParam, _bTabType, _bTabLeader, FALSE);
			_PF.dwMask |= PFM_TABSTOPS;
		}
		break;

	case tokenDecimalTab:						// \tqdec
	case tokenFlushRightTab:					// \tqr
	case tokenCenterTab:						// \tqc
		PARSERCOVERAGE_CASE();
		_bTabType = _token - tokenCenterTab + PFT_CENTER;
		break;

	case tokenTabLeaderEqual:					// \tleq
	case tokenTabLeaderThick:					// \tlth
	case tokenTabLeaderUnderline:				// \tlul
	case tokenTabLeaderHyphen:					// \tlhyph
	case tokenTabLeaderDots:					// \tldot
		PARSERCOVERAGE_CASE();
		_bTabLeader = _token - tokenTabLeaderDots + PFTL_DOTS;
		break;

	// The following need to be kept in sync with PFE_xxx
	case tokenCollapsed:						// \collapsed

#ifdef PWD_JUPITER
		// GuyBark Jupiter 31960: Make a note we've found some collapsed text.
		_fFoundCollapsedText = TRUE;
#endif // PWD_JUPITER

	case tokenSideBySide:						// \sbys
	case tokenHyphPar:							// \hyphpar
	case tokenNoWidCtlPar:						// \nowidctlpar
	case tokenNoLineNumber:						// \noline
	case tokenPageBreakBefore:					// \pagebb
	case tokenKeepNext:							// \keepn
	case tokenKeep:								// \keep
	case tokenRToLPara:							// \rtlpar
		PARSERCOVERAGE_CASE();
		wT = 1 << (_token - tokenRToLPara);
		_PF.wEffects |= wT;
		_PF.dwMask |= (wT << 16);
		break;

	case tokenLToRPara:							// \ltrpar
		PARSERCOVERAGE_CASE();
		_PF.wEffects &= ~PFE_RTLPARA;
		_PF.dwMask |= PFM_RTLPARA;
		break;

	case tokenLineSpacing:						// \sl N
		PARSERCOVERAGE_CASE();
		_PF.dyLineSpacing = abs(iParam);
		_PF.bLineSpacingRule					// Handle nonmultiple rules 
				= (!iParam || iParam == 1000) ? 0
				: tomLineSpaceAtLeast;
				/* FUTURE (joseogl): we don't handle exact linespacing correctly in
				   some situations (e.g. dropped caps cause partial lines).  It seems
				   wise to treat these as "at least" line space requests */
				   //old code : (iParam > 0) ? tomLineSpaceAtLeast
				   //old code : tomLineSpaceExactly;
												// \slmult can change (has to
												//  follow if it appears)
		_PF.dwMask |= PFM_LINESPACING;
		break;

	case tokenLineSpacingRule:					// \slmult N
		PARSERCOVERAGE_CASE();					
		if(iParam)
		{										// It's multiple line spacing
			_PF.bLineSpacingRule = tomLineSpaceMultiple;
			_PF.dyLineSpacing /= 12;			// RE line spacing multiple is
			_PF.dwMask |= PFM_LINESPACING;		//  given in 20ths of a line,
		}										//  while RTF uses 240ths	
		break;

	case tokenSpaceBefore:						// \sb N
		PARSERCOVERAGE_CASE();
		_PF.dySpaceBefore = iParam;
		_PF.dwMask |= PFM_SPACEBEFORE;
		break;

	case tokenSpaceAfter:						// \sa N
		PARSERCOVERAGE_CASE();
		_PF.dySpaceAfter = iParam;
		_PF.dwMask |= PFM_SPACEAFTER;
		break;

	case tokenStyle:							// \s N
		PARSERCOVERAGE_CASE();
		_Style = iParam;						// Save it in case in StyleSheet
		if(pstate->sDest != destStyleSheet)
		{										// Select possible heading level
			_PF.sStyle = STYLE_NORMAL;			// Default Normal style
			_PF.bOutlineLevel |= 1;

			for(i = 0; i < NSTYLES && iParam != _rgStyles[i]; i++)
				;								// Check for heading style
			if(i < NSTYLES)						// Found one
			{
				_PF.sStyle = -i - 1;			// Store desired heading level
				_PF.bOutlineLevel = 2*(i-1);	// Update outline level for
			}									//  nonheading styles
			_PF.dwMask |= PFM_ALLRTF;
		}
		break;

	case tokenIndentFirst:						// \fi N
		PARSERCOVERAGE_CASE();
		_PF.dxStartIndent += _PF.dxOffset		// Cancel current offset
							+ iParam;			//  and add in new one
		_PF.dxOffset = -iParam;					// Offset for all but 1st line
												//  = -RTF_FirstLineIndent
		_PF.dwMask |= (PFM_STARTINDENT | PFM_OFFSET);
		break;						

	case tokenIndentLeft:						// \li N
	case tokenIndentRight:						// \ri N
		PARSERCOVERAGE_CASE();
		// AymanA: For RtL para indents has to be flipped.
		Assert(PFE_RTLPARA == 0x0001);
		if((_token == tokenIndentLeft) ^ (_PF.wEffects & PFE_RTLPARA))
		{
			_PF.dxStartIndent = iParam - _PF.dxOffset;
			_PF.dwMask |= PFM_STARTINDENT;
		}
		else
		{
			_PF.dxRightIndent = iParam;
			_PF.dwMask |= PFM_RIGHTINDENT;
		}
		break;

	case tokenAlignLeft:						// \ql
	case tokenAlignRight:						// \qr
	case tokenAlignCenter:						// \qc
	case tokenAlignJustify:						// \qj
		PARSERCOVERAGE_CASE();
		_PF.wAlignment = _token - tokenAlignLeft + PFA_LEFT;
		_PF.dwMask |= PFM_ALIGNMENT;
		break;

	case tokenParaNum:							// \pn
		PARSERCOVERAGE_CASE();
		pstate->sDest = destParaNumbering;
		pstate->fBullet = FALSE;
		_PF.wNumberingStart = 1;
		_PF.dwMask |= PFM_NUMBERINGSTART;
		break;

	case tokenParaNumCont:						// \pnlvlcont
		PARSERCOVERAGE_CASE();					// Ignore if margins changed
		_prg->_rpPF.AdjustBackward();
		i = pstate->pPF->dxStartIndent - _prg->GetPF()->dxStartIndent;
		_prg->_rpPF.AdjustForward();
		if(i)
			goto skip_group;
		_wNumberingStyle = PFNS_NONUMBER;
		break;

	case tokenBorderOutside:					// \brdrbar
	case tokenBorderBetween:					// \brdrbtw
	case tokenBorderShadow:						// \brdrsh
		PARSERCOVERAGE_CASE();
		_PF.dwBorderColor |= 1 << (_token - tokenBorderShadow + 20);
		_dwBorderColor = _PF.dwBorderColor;
		break;

	// Paragraph and cell border segments
	case tokenBox:								// \box
		PARSERCOVERAGE_CASE();
		_PF.wEffects |= PFE_BOX;
		_PF.dwMask	 |= PFM_BOX;
		_bBorder = 0;							// Store parms as if for
		break;									//  \brdrt

	case tokenCellBorderRight:					// \clbrdrr
	case tokenCellBorderBottom:					// \clbrdrb
	case tokenCellBorderLeft:					// \clbrdrl
	case tokenCellBorderTop:					// \clbrdrt
	case tokenBorderRight:						// \brdrr
	case tokenBorderBottom:						// \brdrb
	case tokenBorderLeft:						// \brdrl
	case tokenBorderTop:						// \brdrt
		PARSERCOVERAGE_CASE();
		_bBorder = _token - tokenBorderTop;
		break;

	// Paragraph border styles
	case tokenBorderTriple:						// \brdrtriple
	case tokenBorderDoubleThick:				// \brdrth
	case tokenBorderSingleThick:				// \brdrs
	case tokenBorderHairline:					// \brdrhair
	case tokenBorderDot:						// \brdrdot
	case tokenBorderDouble:						// \brdrdb
	case tokenBorderDashSmall:					// \brdrdashsm
	case tokenBorderDash:						// \brdrdash
		PARSERCOVERAGE_CASE();
		if(_bBorder < 4)						// Only for paragraphs
			SetBorderParm(_PF.wBorders, _token - tokenBorderDash);
		break;

	case tokenBorderColor:						// \brdrcf
		PARSERCOVERAGE_CASE();
		if(_bBorder < 4)						// Only for paragraphs
		{
			iParam = GetStandardColorIndex();
			_PF.dwBorderColor &= ~(0x1F << (5*_bBorder));
			_PF.dwBorderColor |= iParam << (5*_bBorder);
			_dwBorderColor = _PF.dwBorderColor;
		}
		break;

	case tokenBorderWidth:						// \brdrw
		PARSERCOVERAGE_CASE();					// Width is in half pts
		if(_bBorder < 4)						// For paragraphs
		{										// iParam is in twips
			if(IN_RANGE(1, iParam, 4))			// Give small but nonzero 
				iParam = 1;						//  values our minimum
			else								//  size
				iParam = (iParam + 5)/10;

			SetBorderParm(_PF.wBorderWidth, iParam);
		}
		else									// For cells only have 2 bits
		{
			iParam = (iParam + 10)/20;
			iParam = max(iParam, 1);
			iParam = min(iParam, 3);
			_bCellBrdrWdths |= iParam << 2*(_bBorder - 4);
		}
		break;

	case tokenBorderSpace:						// \brsp
		PARSERCOVERAGE_CASE();					// Space is in pts
		if(_bBorder < 4)						// Only for paragraphs
			SetBorderParm(_PF.wBorderSpace, iParam/20);// iParam is in twips
		break;

	// Paragraph shading
	case tokenBckgrndVert:						// \bgvert
	case tokenBckgrndHoriz:						// \bghoriz
	case tokenBckgrndFwdDiag:					// \bgfdiag
	case tokenBckgrndDrkVert:	   				// \bgdkvert
	case tokenBckgrndDrkHoriz:					// \bgdkhoriz
	case tokenBckgrndDrkFwdDiag:				// \bgdkfdiag
	case tokenBckgrndDrkDiagCross:				// \bgdkdcross
	case tokenBckgrndDrkCross:					// \bgdkcross
	case tokenBckgrndDrkBckDiag:				// \bgdkbdiag
	case tokenBckgrndDiagCross:					// \bgdcross
	case tokenBckgrndCross:						// \bgcross
	case tokenBckgrndBckDiag:					// \bgbdiag
		PARSERCOVERAGE_CASE();
		_PF.wShadingStyle = (_PF.wShadingStyle & 0xFFC0)
				    | (_token - tokenBckgrndBckDiag + 1);
		_PF.dwMask |= PFM_SHADING;
		break;

	case tokenColorBckgrndPat:					// \cbpat
		PARSERCOVERAGE_CASE();
		iParam = GetStandardColorIndex();
		_PF.wShadingStyle = (_PF.wShadingStyle & 0x07FF) | (iParam << 11);
		_PF.dwMask |= PFM_SHADING;
		break;

	case tokenColorForgrndPat:					// \cfpat
		PARSERCOVERAGE_CASE();
		iParam = GetStandardColorIndex();
		_PF.wShadingStyle = (_PF.wShadingStyle & 0xF83F) | (iParam << 6);
		_PF.dwMask |= PFM_SHADING;
		break;

	case tokenShading:							// \shading
		PARSERCOVERAGE_CASE();
		_PF.wShadingWeight = iParam;
		_PF.dwMask |= PFM_SHADING;
		break;

	// Paragraph numbering
	case tokenParaNumIndent:					// \pnindent N
		PARSERCOVERAGE_CASE();
		if (pstate->sDest == destParaNumbering)	// sIndentNumbering
		{										//  gets added into offset
			pstate->sIndentNumbering = (SHORT)iParam;
		}
		break;

	case tokenParaNumStart:						// \pnstart N
		PARSERCOVERAGE_CASE();
		if(pstate->sDest == destParaNumbering)
		{
			_PF.wNumberingStart = (WORD)iParam;
			_PF.dwMask |= PFM_NUMBERINGSTART;
		}
		break;

	case tokenParaNumBody:						// \pnlvlbody
		PARSERCOVERAGE_CASE();
		_wNumberingStyle = PFNS_PAREN;
		_token = tokenParaNumDecimal;			// Default to decimal
		goto setnum;
		
	case tokenParaNumBullet:					// \pnlvlblt
		_wNumberingStyle = 0;					// Reset numbering styles
		goto setnum;

	case tokenParaNumDecimal:					// \pndec
	case tokenParaNumLCLetter:					// \pnlcltr
	case tokenParaNumUCLetter:					// \pnucltr
	case tokenParaNumLCRoman:					// \pnlcrm
	case tokenParaNumUCRoman:					// \pnucrm
// GuyBark JupiterJ: added these for JupiterJ
	case tokenParaNumKatakanaAIUEOdbl:          // \pnaiueod
	case tokenParaNumKatakanaIROHAdbl:          // \pnirohad
		PARSERCOVERAGE_CASE();
		if(_PF.wNumbering == PFN_BULLET && pstate->fBullet)
			break;								// Ignore above for bullets
setnum:	if(pstate->sDest == destParaNumbering)
		{
			_PF.wNumbering = PFN_BULLET + _token - tokenParaNumBullet;
			_PF.dwMask |= PFM_NUMBERING;
			pstate->fBullet	= TRUE;				// We do bullets, so don't
		}										//  output the \pntext group
		break;

	case tokenParaNumText:						// \pntext
		PARSERCOVERAGE_CASE();
		// Throw away previously read paragraph numbering and use
		//	the most recently read to apply to next run of text.
		_cchUsedNumText = 0;
		pstate->sDest = destParaNumText;
		break;

	case tokenParaNumAfter:						// \pntxta
	case tokenParaNumBefore:					// \pntxtb
	case tokenPictureQuickDraw:					// \macpict
	case tokenPictureOS2Metafile:				// \pmmetafile
		PARSERCOVERAGE_CASE();

skip_group:
		if (!SkipToEndOfGroup())
		{
			// During \fonttbl processing, we may hit unknown destinations,
			// e.g., \panose, that cause the HandleEndGroup to select the
			// default font, which may not be defined yet.  So,	we change
			// sDest to avoid this problem.
			if(pstate->sDest == destFontTable || pstate->sDest == destStyleSheet)
				pstate->sDest = destNULL;
			HandleEndGroup();
		}
		break;

	// Tables
	case tokenInTable:							// \intbl
		PARSERCOVERAGE_CASE();

		// GuyBark Jupiter 36591: Move \intbl processing to separate routine.
		HandleIntblToken();

		pstate->fInTable = TRUE;				

		break;

	case tokenCell:								// \cell
		PARSERCOVERAGE_CASE();
		if(_iCell < _cCell)						// Don't add more cells than
		{										//  defined, since Word crashes
			_iCell++;							// Count cells inserted
			HandleChar(CELL);					// Insert cell delimiter
		}
		break;

	case tokenCellHalfGap:						// \trgaph N
		PARSERCOVERAGE_CASE();					// Save half space between
		_dxCell = iParam;						//  cells to add to tabs
		break;									// Roundtrip value at end of
												//  tab array
	case tokenCellX:							// \cellx N
		PARSERCOVERAGE_CASE();
		if(_cCell < MAX_TAB_STOPS)				// Save cell right boundaries
		{										//  for tab settings in our
			if(!_cCell)							//  primitive table model
			{									// Save border info
				_wBorders = _PF.wBorders;
				_wBorderSpace = _PF.wBorderSpace;
				_wBorderWidth = _PF.wBorderWidth;
			}

#ifdef PWD_JUPITER
			// GuyBark Jupiter 16419:
			// Check for zero width cells. We don't allow these, as it may lead to
			// the cell being lost in RichEdit. So force a cell to be a twip big.
			if((_cCell > 0) && (iParam == (_rgCellX[_cCell - 1] & 0x00FFFFFF))) 
			{
			    // The right edge of this cell is at the same position as
			    // the right edge of the previous cell. So bump it a bit.
			    ++iParam;
			}
#endif // PWD_JUPITER

			_rgCellX[_cCell++] = iParam + (_bCellBrdrWdths << 24);
			_bCellBrdrWdths = 0;
		}
		break;

	case tokenRowDefault:						// \trowd
		PARSERCOVERAGE_CASE();
		_cCell = 0;								// No cell right boundaries

#ifdef TARGET_NT // V-GUYB: Don;t change device code at this late stage.

        // V-GUYB: Don't reset the cell count here, in case the input file
        // incorrectly had a \cell before the \trowd for the same row.
//		_iCell = 0;

#endif // TARGET_NT

		_dxCell = 0;							//  or half gap defined yet
		_xRowOffset = 0;
		_bCellBrdrWdths = 0;
		_wBorderWidth = 0;						// No borders yet
		_dwBorderColor	= 0;
		_wAlignment = PFA_LEFT;

		break;

	case tokenRowLeft:							// \trleft
		PARSERCOVERAGE_CASE();
		_xRowOffset = iParam;
		break;
												
	case tokenRowAlignCenter:					// \trqc
	case tokenRowAlignRight:					// \trqr
		PARSERCOVERAGE_CASE();
		_wAlignment = _token - tokenRowAlignRight + PFA_RIGHT;
		break;

	case tokenEndParagraph:						// \par
	case tokenLineBreak:						// \line
		PARSERCOVERAGE_CASE();
		if(_pstateStackTop->fInTable)
#ifdef PWD_JUPITER
        {
            // GuyBark: Preserve CRLF in table cells. Use two characters in
            // the text stream to ensure that when this is streamed out, we
            // never risk array overflow by inserting two characters which
            // can only fit one.
            HandleChar(PWD_CRLFINCELL);
            HandleChar(' ');					// Just use a blank for \par
        }
#else
            HandleChar(' ');					// Just use a blank for \par
#endif // PWD_JUPITER
		else									//  in table
			HandleEndOfPara();
		break;								

	case tokenRow:								// \row. Treat as hard CR
		PARSERCOVERAGE_CASE();

#ifdef PWD_JUPITER
		// GuyBark Jupiter 36591:
		// We've hit the end of a table row. Did we think we were in a table?
		if(!pstate->fInTable)
		{   
		    // No! This shouldn't happen, but we have to be able to handle it.
		    // Take action for the \intbl token first here. This sets everything
		    // up so that we're in a table row. We can then properly end the row.
		    HandleIntblToken();
		}
#endif // PWD_JUPITER

		for( ; _iCell < _cCell; _iCell++)		// If not enuf cells, add
			HandleChar(CELL);					//  them since Word crashes
		_iCell = 0;								//  if \cellx count differs
		HandleEndOfPara();						//  from \cell count
		break;

	case tokenParagraphDefault:					// \pard
		PARSERCOVERAGE_CASE();

		if (pstate->sDest == destParaNumText)	// Ignore if \pn destination
			break;
												// Else fall thru to \secd
#ifndef PWD_JUPITER
        // GuyBark Jupiter 38748:
        // I have seen documents where the \sect and \sectd lie between 
        // the \pn and \pntext tokens for the same line! This means the
        // numbered lists attributes for a paragraph must NOT get reset
        // due to these section tokens. Assume that if the numbering
        // attributes aren't cleared, then we should leave all the
        // paragraph attributes alone.
	case tokenEndSection:						// \sect
	case tokenSectionDefault:					// \sectd
#endif // !PWD_JUPITER

		PARSERCOVERAGE_CASE();
		bT = _PF.bOutlineLevel;					// Save outline level
		_PF.InitDefault(_sDefaultTabWidth,
						_bDocType == DT_RTLDOC ? PFE_RTLPARA : 0);
												// Reset para formatting
		pstate->fInTable = FALSE;				// Reset in table flag
		pstate->fBullet = FALSE;
		pstate->sIndentNumbering = 0;
		_bTabLeader		= 0;
		_bTabType		= 0;
		_bBorder		= 0;
		_PF.bOutlineLevel = bT | 1;
		_PF.dwMask		= PFM_ALLRTF;
		break;


//----------------------- Field and Group Control Words --------------------------------
	// Note that we currently don't support nested fields.  For nested
	// fields, the usage of _szSymbolFieldResult, _FieldCF, _ptfField
	// and _sFieldCodePage needs to be rethought. 

	case tokenField:							// \field
		PARSERCOVERAGE_CASE();

		if(pstate->sDest == destDocumentArea ||
			pstate->sDest == destLeadingPunct ||
			pstate->sDest == destFollowingPunct)
		{
			// We're not equipped to handle symbols in these destinations, and
			// we don't want the fields added accidentally to document text.
			goto skip_group;
		}

		pstate->sDest = destField;
		
		_nFieldCodePage = pstate->nCodePage;	// init, for safety
		_ptfField = NULL;
		_FieldCF.dwMask = 0;
		_fRestoreFieldFormat = TRUE;
		break;

	case tokenFieldResult:						// \fldrslt
		PARSERCOVERAGE_CASE();

		// restore the formatting from the field instruction
		if (_fRestoreFieldFormat)
		{
			_fRestoreFieldFormat = FALSE;
			_CF = _FieldCF;
			pstate->ptf = _ptfField;
			pstate->SetCodePage(_nFieldCodePage);
		}

#ifdef PWD_JUPITER
		// GuyBark JupiterJ 49674: Sometimes equation field results can be empty.
		// This means unless we keep what we found in the field instruction group,
		// text will be lost completely. So set the destination here to be "Field
		// Result", to track whether the result had anything in it.
		if(_fEquationField )
		{
			pstate->sDest = destFieldResult;
			break;
		}
#endif // PWD_JUPITER

		if ( !_fHyperlinkField )
		{
			// for SYMBOL
			pstate->sDest = destField;
			break;
		}

		// for HYPERLINK
		
		// By now, we should have the whole hyperlink fldinst string
		if ( _szHyperlinkFldinst )
		{
			// V-GUYB: PWord Converter requires loss notification.
			// (Hyperlinks are NOT streamed out later)
			#ifdef REPORT_LOSSAGE
        	if(!(_dwFlags & SFF_SELECTION)) // SFF_SELECTION is set if any kind of paste is being done.
        	{
                ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = TRUE;
            }
			#endif // REPORT_LOSSAGE
		
			BYTE * pNewBuffer = NULL;

			// check if this is a friendly name
			if ( _szHyperlinkFldinst[1] == '\"' )
			{	
				// This is a friendly name, replace quotes with <>.
				// Also, for an unknown reason, Word escapes some chars in its HYPERLINK presentation
				// we have to get rid of the backslashes 

				BYTE * pSrc = &_szHyperlinkFldinst[2];
				BYTE * pBuffer;
 				BOOL fLeadByte = FALSE;
				LONG CodePage;

				CodePage = IsFECharset(_bInstFieldCharSet) ? GetCodePage(_bInstFieldCharSet): 0;

				pNewBuffer = (BYTE *)PvAlloc(_cchHyperlinkFldinstUsed+1, GMEM_ZEROINIT);

				if (!pNewBuffer)
				{
					_ecParseError = ecNoMemory;
					break;
				}

				pBuffer = pNewBuffer;
				*pBuffer++ = ' ';
				*pBuffer++ = '<';

				do 
				{
					if ( !fLeadByte && *pSrc == '\\' )
					{
						// get rid of the backslashes
						pSrc++;
					}
					else if ( *pSrc == '\"')
					{
						// find the end qoute
						*pBuffer = '>';
						break;
					}
					else if ( CodePage )
					{
						// check if this is a valid Lead byte.
						fLeadByte = fLeadByte ? FALSE : IsLeadByte(*pSrc, CodePage);
					}
				} while ( *pBuffer++ = *pSrc++ );						
			}

			// No longer need this buffer...
			FreePv (_szHyperlinkFldinst);

			// setup for the new scanned buffer
			_szHyperlinkFldinst = pNewBuffer;
			_cchHyperlinkFldinst = _cchHyperlinkFldinstUsed+1;
		}

		pstate->sDest = destFieldResult;
		if ( _szHyperlinkFldinst )
		{			
			// pre-alloc a buffer for the fldrslt strings
			_cchHyperlinkFldrslt = MAX_PATH;
			_cchHyperlinkFldrsltUsed = 0;
			_szHyperlinkFldrslt = (BYTE *)PvAlloc(_cchHyperlinkFldrslt, GMEM_FIXED);

			if ( !_szHyperlinkFldrslt )
			{
				_ecParseError = ecNoMemory;
				break;
			}

            // GUYBARK: Raid 21369
            // Must initialize this in case incoming rtf is bad 
            // and we never get any field result text.
            _szHyperlinkFldrslt[0] = '\0';
		}
		else
		{
			// no friendly HYPERLINK name, no need to accumulate the
			// fldrslt strings.  
			_szHyperlinkFldrslt = 0;
			_fHyperlinkField = FALSE;
		}
				
		break;

	case tokenFieldInstruction:					// \fldinst
		PARSERCOVERAGE_CASE();
		pstate->sDest = destFieldInstruction;

#ifdef PWD_JUPITER
		// GuyBark JupiterJ 49674: We're not parsing an equation field as far as we know.
		// This will be set true later if necessary beneath HandleFieldInstruction().
		_fEquationField  = FALSE;
#endif // PWD_JUPITER

		break;

	case tokenStartGroup:						// Save current state by
		PARSERCOVERAGE_CASE();					//  pushing it onto stack
		_cbSkipForUnicode = 0;
		HandleStartGroup();
		break;

	case tokenEndGroup:
		PARSERCOVERAGE_CASE();
		_cbSkipForUnicode = 0;
		if (pstate->sDest == destField)
		{
			// for SYMBOLS
			if (!_fHyperlinkField ) 
			{
				if ( _szSymbolFieldResult )	 // there is a new field result
				{
					if (_fRestoreFieldFormat)
					{
						_fRestoreFieldFormat = FALSE;
						_CF = _FieldCF;
						pstate->ptf = _ptfField;
						pstate->SetCodePage(_nFieldCodePage);
					}
					HandleText(_szSymbolFieldResult, CONTAINS_NONASCII);
					FreePv(_szSymbolFieldResult);
					_szSymbolFieldResult =NULL;
				}

#ifdef PWD_JUPITER
				// GuyBark JupiterJ 49674: We've hit the end of a field group. This group 
				// contains the field instruction group and the field result group. If we 
				// were parsing equations, then we're not any more.
				_fEquationField = FALSE;
#endif // PWD_JUPITER
			}
			else if (pstate->pstateNext)
			{
				// setup formatting for the field result
				_CF = _FieldCF;
				pstate->ptf = _ptfField;
				pstate->SetCodePage(_nFieldCodePage);

				// for HYPERLINK
				if (pstate->pstateNext->sDest == destFieldResult)
				{
					// we have the final hyperlink fldrslt string
					// check if it is the same as the friendly name
			
					if (_szHyperlinkFldrslt && _szHyperlinkFldinst && _szHyperlinkFldinst[1] == '<' &&
						!strncmp((char*)_szHyperlinkFldrslt, (char*)&_szHyperlinkFldinst[2], _cchHyperlinkFldrsltUsed-1))
					{
						// they are the same, only need to output the friedly name
						HandleText(&_szHyperlinkFldinst[1], CONTAINS_NONASCII);						
					}
					else
					{
						// output the result string
						if (_szHyperlinkFldrslt)
						{
							HandleText(_szHyperlinkFldrslt, CONTAINS_NONASCII);
						}

						// output the friendly name 
						if (_szHyperlinkFldinst)
						{
							HandleText(_szHyperlinkFldinst, CONTAINS_NONASCII);
						}
					}
					
					FreePv (_szHyperlinkFldinst);
					_szHyperlinkFldinst = NULL;

					FreePv (_szHyperlinkFldrslt);
					_szHyperlinkFldrslt = NULL;
					
					_fHyperlinkField = FALSE;
				}
			}
		}
		else if ( pstate->sDest == destFieldResult && _fHyperlinkField )
		{
			// save the current formatting for the field result if dwMask is valid.
			// NOTE: HandleEndGroup will zero out _CF.dwMask
			if ( _CF.dwMask )
			{
				// we should use FE charset in case of mixed of FE and non-FE in the url
				// Also, only use codepage other than English in case of a mixed of English
				// and non-English (e.g. English and Russian ) 
				if ( (!IsFECharset(_FieldCF.bCharSet) && IsFECharset(_CF.bCharSet)) ||
					(_nFieldCodePage != pstate->nCodePage && _nFieldCodePage == 1252 ) ||
					(_FieldCF.bCharSet == _CF.bCharSet && _nFieldCodePage == pstate->nCodePage) )
				{
					_FieldCF = _CF;
					_ptfField = pstate->ptf;
					_nFieldCodePage = pstate->nCodePage;
				}
			}
		}

		HandleEndGroup();						// Restore save state by
		break;									//  popping stack

	case tokenOptionalDestination:				// \* (see case tokenUnknown)
		PARSERCOVERAGE_CASE();
		break;

	case tokenNullDestination:					// We've found a destination
		PARSERCOVERAGE_CASE();
        // tokenNullDestination triggers a loss notifcation here for...
        //      Footer related tokens - "footer", "footerf", "footerl", "footerr", 
        //                              "footnote", "ftncn", "ftnsep", "ftnsepc"
        //      Header related tokens - "header", "headerf", "headerl", "headerr"
        //      Table of contents     - "tc"
        //      Index entries         - "xe"

		// V-GUYB: PWord Converter requires loss notification.
		#ifdef REPORT_LOSSAGE
        if(!(_dwFlags & SFF_SELECTION)) // SFF_SELECTION is set if any kind of paste is being done.
        {
            ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = TRUE;
        }
		#endif // REPORT_LOSSAGE
		
		goto skip_group;						// for which we should ignore
												// the remainder of the group
	case tokenUnknownKeyword:
		PARSERCOVERAGE_CASE();
		if (_tokenLast == tokenOptionalDestination)
			goto skip_group;
		break;									// Nother place for
												//  unrecognized RTF


//-------------------------- Text Control Words --------------------------------

	case tokenUnicode:							// \u <n>
		PARSERCOVERAGE_CASE();

#ifdef PWD_JUPITER
		// GuyBark JupiterJ 49674: We mustn't store this Unicode if it lies inside
		// a field group, as we'll pick it up again later when we parse the field
		// result. So ignore it.
		if(pstate->sDest == destFieldInstruction || pstate->sDest == destFieldResult)
		{
			// Wait! Equation fields containing Rubi sometimes have empty field
			// results, so we need this text here. We'll drop it later if we 
			// find a non-empty field result for this equation.
			if(!_fEquationField)
			{
			    break;
			}
		}

		// GuyBark JupiterJ 51012: Ignore text in the same way as ASCII text.
		if(pstate->sDest == destDocumentArea)
		{
		    if(_tokenLast != tokenFollowingPunct && _tokenLast != tokenLeadingPunct)
		    {
		        break;
		    }
		}

#endif // PWD_JUPITER

		_CF.bInternalMask |= CFMI_RUNISDBCS;
		_CF.bInternalEffects &= ~CFEI_RUNISDBCS;
		wT = (WORD)iParam;						// Treat as unsigned integer
		if (IN_RANGE(0xF000, wT, 0xF0FF) &&		
			pstate->ptf->bCharSet == SYMBOL_CHARSET)
		{										// Compensate for converters
			wT -= 0xF000;						//  that write symbol codes
		}										//  up high

#ifdef PWD_JUPITER
        // GuyBark: If this is J text, and no J font has been selected,
        // select a J font now. This is a workaround for the fact that
        // some Office converters output J text with a font with the 
        // 1252 code page still selected.
        
        // Is the current char set a FE charset?
        if(!IsFECharset(_CF.bCharSet))
        {
            // No, so check if this unicode character lies in the FE range.
            if(IN_RANGE(0x3000, wT, 0x33FF) ||   // "CJK Symbols and Punctuation", "Hiragana", "Katakana", "Bopomofo", "Hangul Compatibility Jamo", "Kanbun", "Enclosed CJK Letters and Months", "CJK Compatibility"
               IN_RANGE(0x4E00, wT, 0x9FFF) ||   // "CJK Unified Ideographs"
               IN_RANGE(0xF900, wT, 0xFAFF) ||   // "CJK Compatibility Ideographs"
               IN_RANGE(0xFE30, wT, 0xFE4F) ||   // "CJK Compatibility Forms"
               IN_RANGE(0xFF00, wT, 0xFFEF))     // "Halfwidth and Fullwidth Forms"
            {
                UINT      j;
                TEXTFONT *ptf2;

                // We must select a J font now. So use the first J font in the font 
                // table if there is one.
                for(j = 0; j < _fonts.Count(); j++)
                {
                    ptf2 = _fonts.Elem(j);

                    // ACTUALLY check for J, PRC or Taiwan. Assume the first FE font found 
                    // is ok for this text. Strictly that may not be good enough, as a 
                    // Chinese font can't display all J characters. But this is a workaround
                    // and in most cases we won't get a mix of J, PRC and Taiwanese fonts.

                    if(ptf2->sCodePage == 932 ||
                       ptf2->sCodePage == 936 ||
                       ptf2->sCodePage == 950)
                    {
                        // Now select the FE font.
                        SelectCurrentFont(ptf2->sHandle);
                        break;
                    }
                }
            }
        }

        // GuyBark JupiterJ: Is this text inside an equation field?
        if(_fEquationField)
        {
            // Yes. If we have a y offset for this, make sure the required mask is set.
            if(_CF.yOffset)
            {
                _CF.dwMask |= CFM_OFFSET;
            }
        }
#endif // PWD_JUPITER

		AddText((TCHAR *)&wT, 1, TRUE);			//  (avoids endian problems)
		_cbSkipForUnicode = pstate->cbSkipForUnicodeMax;
		break;

	case tokenUnicodeCharByteCount:				// \ucN
		PARSERCOVERAGE_CASE();
		pstate->cbSkipForUnicodeMax = iParam;
		break;

	case tokenText:								// Lexer concludes tokenText
	case tokenASCIIText:
		PARSERCOVERAGE_CASE();
		switch (pstate->sDest)
		{
		case destColorTable:
			pclrf = _colors.Add(1, NULL);
			if (!pclrf)
				goto OutOfRAM;

			*pclrf = _fGetColorYet ? 
				RGB(pstate->bRed, pstate->bGreen, pstate->bBlue) : tomAutoColor;

			// Prepare for next color table entry
			pstate->bRed =						
			pstate->bGreen =					
			pstate->bBlue = 0;
			_fGetColorYet = FALSE;				// in case more "empty" color
			break;

		case destFontTable:
			if(!pstate->fRealFontName)
			{
				ReadFontName(pstate, 
								_token == tokenASCIIText ? 
										ALL_ASCII : CONTAINS_NONASCII);
			}
			break;

		case destRealFontName:
		{
			STATE * const pstatePrev = pstate->pstatePrev;

			if (pstatePrev && pstatePrev->sDest == destFontTable)
			{
				// Mark previous state so that tagged font name will be ignored
				// AROO: Do this before calling ReadFontName so that
				// AROO: it doesn't try to match font name
				pstatePrev->fRealFontName = TRUE;
				ReadFontName(pstatePrev, 
						_token == tokenASCIIText ? ALL_ASCII : CONTAINS_NONASCII);
			}

			break;
		}

		case destFieldInstruction:
			if (_szHyperlinkFldinst)
			{
				if (!IsFECharset(_bInstFieldCharSet) && IsFECharset(_CF.bCharSet))
				{
					_bInstFieldCharSet = _CF.bCharSet;
				}
				_ecParseError = AppendString(& _szHyperlinkFldinst, _szText, &_cchHyperlinkFldinst, &_cchHyperlinkFldinstUsed );
			}
#ifdef PWD_JUPITER
			// GuyBark JupiterJ 49674: 
			else if(_fEquationField)
			{
				// We have some text inside an equation field instruction. This is used for
				// Rubi text and the text it sits on. It's possible later that we'll find
				// the accompanying field result is empty. If so, then we'll stick with the
				// text we found here. If there is a result, then we'll remove the text we
				// handled here and replace it with the result.

				// Note: The text is full of equation related stuff which we don't want. So
				// just handle non-ASCII text here. This may not give quite the expected 
				// result, but it's much better than relying on result text that's missing.

				if(_token == tokenASCIIText)
				{
				    // Set up the current text y offset from this equation string.
				    SetupEquationOffset((LPSTR)_szText);
				}
				else
				{
				    // Assume we're only interested in non ASCII text here.

				    // Yes. If we have a y offset for this, make sure the required mask is set.
				    if(_CF.yOffset)
				    {
				        _CF.dwMask |= CFM_OFFSET;
				    }

				    HandleText(_szText, CONTAINS_NONASCII);
				}
			}
#endif // PWD_JUPITER
			else
			{
				HandleFieldInstruction();
				_bInstFieldCharSet = _CF.bCharSet;
			}
			break;

		case destObjectClass:
			if(StrAlloc(&_prtfObject->szClass, _szText))
				goto OutOfRAM;
			break;
			
		case destObjectName:
			if (StrAlloc(&_prtfObject->szName, _szText))
				goto OutOfRAM;
			break;

		case destStyleSheet:
			// _szText has style name, e.g., "heading 1"
			if(W32->ASCIICompareI(_szText, (unsigned char *)"heading", 7))
			{
				dwT = (unsigned)(_szText[8] - '0');
				if(dwT < NSTYLES)
					_rgStyles[dwT] = _Style;
			}
#ifdef PWD_JUPITER
			// GuyBark JupiterJ 51008: Watch for J heading style names.
			else if(!strncmp((LPSTR)_szText, "\x8c\xa9\x8f\x6f\x82\xb5", 6))
			{
				dwT = (unsigned)(_szText[7] - '0');
				if(dwT < NSTYLES)
					_rgStyles[dwT] = _Style;
			}
#endif // PWD_JUPITER

			break;

		case destDocumentArea:
			if (_tokenLast != tokenFollowingPunct &&
				_tokenLast != tokenLeadingPunct)
			{
				break;
			}										// Else fall thru to
													//  destFollowingPunct
		case destFollowingPunct:
		case destLeadingPunct:
			// TODO(BradO):  Consider some kind of merging heuristic when
			//	we paste FE RTF (for lead and follow chars, that is).
			if(!(_dwFlags & SFF_SELECTION))
			{
				WCHAR *pwchBuf;
				int cwch;

				cwch = MBTWC(INVALID_CODEPAGE, 0,
										(char *)_szText, -1,
										NULL, 0,
										NULL);
				Assert(cwch);

				pwchBuf = (WCHAR *)PvAlloc(cwch * sizeof(WCHAR), GMEM_ZEROINIT);

				if(!pwchBuf)
				{
					goto OutOfRAM;
				}

				SideAssert(MBTWC(INVALID_CODEPAGE, 0,
									(char *)_szText, -1,
									pwchBuf, cwch,
									NULL) > 0);

				if(pstate->sDest == destFollowingPunct)
				{
					_ped->SetFollowingPunct(pwchBuf);
				}
				else
				{
					Assert(pstate->sDest == destLeadingPunct);
					_ped->SetLeadingPunct(pwchBuf);
				}
				FreePv(pwchBuf);
			}
			break;

		case destFieldResult:

			if (_szSymbolFieldResult)     		// Field has been recalculated
				break;							// old result out of use
			else if (_szHyperlinkFldrslt)		// Append _szText to _szHyperlinkFldrslt
			{
				_ecParseError = AppendString( &_szHyperlinkFldrslt, _szText, &_cchHyperlinkFldrslt, &_cchHyperlinkFldrsltUsed );
				break;
			}
#ifdef PWD_JUPITER
			// GuyBark JupiterJ 49674: Does this text lie in am equation field result?
			else if(_fEquationField)
			{
				// Yes. So if we added some text from the equation field instruction, we need
				// to remove it and use the field result instead. Do we have a stored cp for
				// the start of the equation field instruction text?

				if(_cpFieldInstruction != (LONG)-1)
				{
					// Yes. Get the current cp, (ie the end of the equation field instruction text).
					LONG cpCurrent = _prg->GetCp();

					// Select the text to be removed.
					CTxtSelection *psel = _ped->GetSel();

					psel->SetSelection(_cpFieldInstruction, cpCurrent);

					// Now delete the equation field instruction text.
					psel->ReplaceRange(0, NULL, NULL, SELRR_IGNORE);

					// Make sure we don't do that again if we find more field result text.
					_cpFieldInstruction = (LONG)-1;
				}
			}
#endif // PWD_JUPITER

			// FALL THRU to default case

		default:
			if(pstate->fltrch || pstate->frtlch)
			{
				if(_CF.bCharSet != DEFAULT_CHARSET)
				{
					BOOL	fBiDiCharSet = (_CF.bCharSet == _bBiDiCharSet);

					// If direction token doesn't correspond to the current charset
					if (fBiDiCharSet ^ pstate->frtlch)
					{
						_CF.bCharSet = fBiDiCharSet ? pstate->ptf->bCharSet:_bBiDiCharSet;
						_CF.dwMask |= CFM_CHARSET;
						pstate->ptf->sCodePage = GetCodePage((BYTE)_CF.bCharSet);
						pstate->SetCodePage(pstate->ptf->sCodePage);
					}
				}
				else
				{
					_CF.bCharSet = (pstate->fltrch) ? ANSI_CHARSET:_bBiDiCharSet;
					_CF.dwMask |= CFM_CHARSET;
				}
			}
			HandleText(_szText, _token == tokenASCIIText ? ALL_ASCII : CONTAINS_NONASCII);
		}
		break;


	// \ltrmark, \rtlmark, \zwj, and \zwnj are translated directly into
	// their Unicode values. \ltrmark and \rtlmark cause no further
	// processing here because we assume that the current font has the
	// CharSet needed to identify the direction.  For the same reason,
	// we ignore \ltrch and \rtlch on import, altho we do export them as
	// indicated by the CharSet.

	case tokenLToRChars:						// \rtlch
	case tokenRToLChars:						// \rtlch
#if 0
		if(!(_CF.dwMask & CFM_CHARSET))			// If _CF.bCharSet isn't defd
		{										//  use current range value
			LONG iCF = _prg->Get_iCF();
			_CF.bCharSet = (_ped->GetCharFormat(iCF))->bCharSet;
			ReleaseFormats(iCF, -1);	
		}

		if(_CF.bCharSet != DEFAULT_CHARSET)
		{
			BOOL	fBiDiCharSet = (_CF.bCharSet == _bBiDiCharSet);

			// If direction token doesn't correspond to the current charset
			if (fBiDiCharSet ^ (_token == tokenRToLChars))
			{
				_CF.bCharSet = fBiDiCharSet ? pstate->ptf->bCharSet:_bBiDiCharSet;
				_CF.dwMask |= CFM_CHARSET;
				pstate->ptf->nCodePage = GetCodePage((BYTE)_CF.bCharSet);
				pstate->SetCodePage(pstate->ptf->nCodePage);
				pstate->fltrch = pstate->frtlch = FALSE;
			}
		}
#endif
		pstate->fltrch = (_token == tokenLToRChars);
		pstate->frtlch = (_token == tokenRToLChars);
		break;

	case tokenLToRDocument:						// \ltrdoc
		PARSERCOVERAGE_CASE();
		_bDocType = DT_LTRDOC;
		break;

	case tokenRToLDocument:						// \rtldoc
		PARSERCOVERAGE_CASE();
		_bDocType = DT_RTLDOC;
		break;



//------------------------- Object Control Words --------------------------------

	case tokenObject:							// \object
		PARSERCOVERAGE_CASE();
		// V-GUYB: PWord Converter requires loss notification.
		#ifdef REPORT_LOSSAGE
       	if(!(_dwFlags & SFF_SELECTION)) // SFF_SELECTION is set if any kind of paste is being done.
       	{
            ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = TRUE;
        }
		#endif // REPORT_LOSSAGE
		
		// Assume that the object failed to load until proven otherwise
		// 	by RTFRead::ObjectReadFromEditStream
	  	// This works for both:
		//	- an empty \objdata tag
		//	- a non-existent \objdata tag
		_fFailedPrevObj = TRUE;

	case tokenPicture:							// \pict
		PARSERCOVERAGE_CASE();

		pstate->sDest = _token==tokenPicture ? destPicture : destObject;

		FreeRtfObject();
		_prtfObject = (RTFOBJECT *) PvAlloc(sizeof(RTFOBJECT), GMEM_ZEROINIT);
		if (!_prtfObject)
			goto OutOfRAM;
		_prtfObject->xScale = _prtfObject->yScale = 100;
		_prtfObject->cBitsPerPixel = 1;
		_prtfObject->cColorPlanes = 1;
		_prtfObject->szClass = NULL;
		_prtfObject->szName = NULL;
		_prtfObject->sType = -1;
		break;

	case tokenObjectEmbedded:					// \objemb
	case tokenObjectLink:						// \objlink
	case tokenObjectAutoLink:					// \objautlink
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = _token - tokenObjectEmbedded + ROT_Embedded;
		break;

	case tokenObjectMacSubscriber:				// \objsub
	case tokenObjectMacPublisher:				// \objpub
	case tokenObjectMacICEmbedder:
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = ROT_MacEdition;
		break;

	case tokenWidth:							// \picw or \objw
		PARSERCOVERAGE_CASE();
		_prtfObject->xExt = iParam;
		break;

	case tokenHeight:							// \pic or \objh
		PARSERCOVERAGE_CASE();
		_prtfObject->yExt = iParam;
		break;

	case tokenObjectSetSize:					// \objsetsize
		PARSERCOVERAGE_CASE();
		_prtfObject->fSetSize = TRUE;
		break;

	case tokenScaleX:							// \picscalex or \objscalex
		PARSERCOVERAGE_CASE();
		_prtfObject->xScale = iParam;
		break;

	case tokenScaleY:							// \picscaley or \objscaley
		PARSERCOVERAGE_CASE();
		_prtfObject->yScale = iParam;
		break;

	case tokenCropLeft:							// \piccropl or \objcropl
 	case tokenCropTop:							// \piccropt or \objcropt
	case tokenCropRight:						// \piccropr or \objcropr
	case tokenCropBottom:						// \piccropb or \objcropb
		PARSERCOVERAGE_CASE();
		*((LONG *)&_prtfObject->rectCrop
			+ (_token - tokenCropLeft)) = iParam;
		break;

	case tokenObjectClass:						// \objclass
		PARSERCOVERAGE_CASE();
		pstate->sDest = destObjectClass;
		break;

	case tokenObjectName:						// \objname
		PARSERCOVERAGE_CASE();
		pstate->sDest = destObjectName;
		break;

	case tokenObjectResult:						// \result
		PARSERCOVERAGE_CASE();
		if (_prtfObject &&						// If it's Mac stuff, we don't
			_prtfObject->sType==ROT_MacEdition)	//  understand the data, but
		{										//  we can try to do something
			pstate->sDest = destRTF;			//  with the results of the
		}										//  data
		else if(!_fFailedPrevObj && !_fNeedPres)// If we failed to retrieve
			goto skip_group;					//  previous object, try to
												//  try to read results
		break;

	case tokenObjectData:						// \objdata
		PARSERCOVERAGE_CASE();
		pstate->sDest = destObjectData;
		if(_prtfObject->sType==ROT_MacEdition)	// It's Mac stuff so just
			goto skip_group;					//  throw away the data
		break;

	case tokenPictureWindowsMetafile:			// wmetafile
#ifdef NOMETAFILES
		goto skip_group;
#endif NOMETAFILES

	case tokenPictureWindowsBitmap:				// wbitmap
	case tokenPictureWindowsDIB:				// dibitmap
		PARSERCOVERAGE_CASE();
		_prtfObject->sType = _token - tokenPictureWindowsBitmap + ROT_Bitmap;
		_prtfObject->sPictureType = (SHORT) iParam;
		break;

	case tokenBitmapBitsPerPixel:				// \wbmbitspixel
		PARSERCOVERAGE_CASE();
		_prtfObject->cBitsPerPixel =(SHORT) iParam;
		break;

	case tokenBitmapNumPlanes:					// \wbmplanes
		PARSERCOVERAGE_CASE();
		_prtfObject->cColorPlanes =(SHORT) iParam;
		break;

	case tokenBitmapWidthBytes:					// \wbmwidthbytes
		PARSERCOVERAGE_CASE();
		_prtfObject->cBytesPerLine =(SHORT) iParam;
		break;

	case tokenDesiredWidth:						// \picwgoal
		PARSERCOVERAGE_CASE();
		_prtfObject->xExtGoal = (SHORT)iParam;
		break;

	case tokenDesiredHeight:					// \pichgoal
		PARSERCOVERAGE_CASE();
		_prtfObject->yExtGoal =(SHORT) iParam;
		break;

	case tokenBinaryData:						// \bin
		PARSERCOVERAGE_CASE();

		if(_cbSkipForUnicode)
		{
			// a \binN and its associated binary data count as a single 
			//	character for the purposes of skipping over characters 
			//	following a \uN
			_cbSkipForUnicode--;
			SkipBinaryData(iParam);
		}
		else
		{
			// update OleGet function
			RTFReadOLEStream.lpstbl->Get = 
					(DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
						   RTFGetBinaryDataFromStream;
			// set data length
			_cbBinLeft = iParam;
		
			switch (pstate->sDest)
			{
				case destObjectData:
					_fFailedPrevObj = !ObjectReadFromEditStream();
					break;
				case destPicture:
					StaticObjectReadFromEditStream(iParam);
					break;

				default:
					AssertSz(FALSE, "Binary data hit but don't know where to put it");
			}

			// restore OleGet function
			RTFReadOLEStream.lpstbl->Get = 
					(DWORD (CALLBACK* )(LPOLESTREAM, void FAR*, DWORD))
						RTFGetFromStream;
		}

		break;

	case tokenObjectDataValue:
		PARSERCOVERAGE_CASE();
		_fFailedPrevObj = !ObjectReadFromEditStream();
		goto EndOfObjectStream;
	
	case tokenPictureDataValue:
		PARSERCOVERAGE_CASE();
		StaticObjectReadFromEditStream();
EndOfObjectStream:
		if (!SkipToEndOfGroup())
			HandleEndGroup();
		break;			

	case tokenObjectPlaceholder:
		PARSERCOVERAGE_CASE();
		if(_ped->GetEventMask() & ENM_OBJECTPOSITIONS) 
		{
			if(!_pcpObPos)
			{
				_pcpObPos = (LONG *)PvAlloc(sizeof(ULONG) * cobPosInitial, GMEM_ZEROINIT);
				if(!_pcpObPos)
				{
					_ecParseError = ecNoMemory;
					break;
				}
				_cobPosFree = cobPosInitial;
				_cobPos = 0;
			}
			if(_cobPosFree-- <= 0)
			{
				const int cobPosNew = _cobPos + cobPosChunk;
				LPVOID pv;

				pv = PvReAlloc(_pcpObPos, sizeof(ULONG) * cobPosNew);
				if(!pv)
				{
					_ecParseError = ecNoMemory;
					break;
				}
				_pcpObPos = (LONG *)pv;
				_cobPosFree = cobPosChunk - 1;
			}
			_pcpObPos[_cobPos++] = _prg->GetCp();
		}
		break;

	default:
		PARSERCOVERAGE_DEFAULT();
		if(pstate->sDest != destFieldInstruction &&	// Values outside token
		   (DWORD)(_token - tokenMin) >				//  range are treated
				(DWORD)(tokenMax - tokenMin))		//  as Unicode chars
		{
			HandleChar(_token);
		}
		#if defined(DEBUG)
		else
		{
			if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGE", 0))
			{
				CHAR *pszKeyword = PszKeywordFromToken(_token);
				CHAR szBuf[256];

				sprintf(szBuf, "CRTFRead::HandleToken():  Token not processed - token = %d, %s%s%s",
							_token,
							"keyword = ", 
							pszKeyword ? "\\" : "<unknown>", 
							pszKeyword ? pszKeyword : "");

				AssertSz(0, szBuf);
			}
		}
		#endif
	}

done:
	TRACEERRSZSC("HandleToken()", - _ecParseError);
	return _ecParseError;
}

/*
 *	CRTFRead::HandleIntblToken()
 *
 *  Added by GUYBARK (8/8/98)
 *
 *	@mfunc
 *
 *      Take all the required action on finding the \intbl token.
 *      Moved to a separate routine as now called from two places.
 */
void CRTFRead::HandleIntblToken()
{
	DWORD  dwT;
	LONG   i;

	// Our simple table model has one para per row, i.e., no paras in
	// cells. Also no tabs in cells (both are converted to blanks). On
	// receipt of \intbl, transfer stored table info into _PF.

	// Delete any tab stops except the default
	if (_PF.cTabCount && (_PF.rgxTabs[0] & PFT_DEFAULT) != PFT_DEFAULT)
	{
		_PF.cTabCount = 0;
		_PF.dwMask |= PFM_TABSTOPS;
	}

	// Set _PF tabs equal to cell right boundaries
	for (i = 0; i < _cCell; i++)
	{
		dwT = _rgCellX[i];
		_PF.AddTab(GetTabPos(dwT), GetTabAlign(dwT), GetTabLdr(dwT), TRUE);
		_PF.dwMask |= PFM_TABSTOPS;
	}
	if(_wBorderWidth)						// Store any border info
	{
		_PF.dwBorderColor = _dwBorderColor;
		_PF.wBorders	  = _wBorders;
		_PF.wBorderSpace  = _wBorderSpace;
		_PF.wBorderWidth  = _wBorderWidth;
		_PF.dwMask |= PFM_BORDER;
	}

	_PF.wAlignment	  = _wAlignment;		// Row alignment (no cell align)

#ifdef PWD_JUPITER
	// GuyBark Jupiter 38532:
	// If we didn't find the expected \trleft and \trgaph, then the text will 
	// be touching the left edge of the cell. This means it looks pretty bad
	// later when opened up again in Word97. So add a default value in this 
	// case. Note: In theory we could be overwriting the user's values of zero
	// if they explicitly set these, but that will be very rare.
	if(!_xRowOffset && !_dxCell)
	{
		_dxCell     = 108;
		_xRowOffset = -_dxCell;
	}
#endif // PWD_JUPITER

	_PF.dxStartIndent = _xRowOffset;		// \trleft N
	_PF.dxOffset	  = _dxCell;			// \trgaph N
	_PF.wEffects |= PFE_TABLE;				
	_PF.dwMask	 |= PFM_TABLE | PFM_OFFSET | PFM_ALIGNMENT;

    return;
}

#pragma warning ( disable : 4390 )

/*
 *	CRTFRead::ReadRtf()
 *
 *	@mfunc
 *		The range _prg is replaced by RTF data resulting from parsing the
 *		input stream _pes.  The CRTFRead object assumes that the range is
 *		already degenerate (caller has to delete the range contents, if
 *		any, before calling this routine).  Currently any info not used
 *		or supported by RICHEDIT is	thrown away.
 *
 *	@rdesc
 *		Number of chars inserted into text.  0 means none were inserted
 *		OR an error occurred.
 */
LONG CRTFRead::ReadRtf()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadRtf");

	CTxtRange *	prg = _prg;
	STATE *		pstate;

	_cpFirst = prg->GetCp();
	if (!InitLex())
		goto Quit;

	TESTPARSERCOVERAGE();

	AssertSz(!prg->GetCch(),
		"CRTFRead::ReadRtf: range must be deleted");

	if(!(_dwFlags & SFF_SELECTION))
	{
		// SFF_SELECTION is set if any kind of paste is being done, i.e.,
		// not just that using the selection.  If it isn't set, data is
		// being streamed in and we allow this to reset the doc params
		_ped->InitDocInfo();
	}

	prg->SetIgnoreFormatUpdate(TRUE);

	_szUnicode = (TCHAR *)PvAlloc(cachTextMax * sizeof(TCHAR), GMEM_ZEROINIT);
	if (!_szUnicode)				// Allocate space for Unicode conversions
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		goto CleanUp;
	}

	//TODO(BradO):  We should examine the member data in the constructor
	//	and determine which data we want initialized on construction and
	//	which at the beginning of every read.

	// Initialize per-read variables
	_nCodePage		= INVALID_CODEPAGE;
	_cbSkipForUnicode = 0;
	_fReadDefFont = FALSE;
	_dwFlags &= ~SFF_UTF8;					// Must have \utf8 to turn it on
	#ifdef DEBUG
	_fSeenFontTable	= FALSE;
	#endif

	// Populate _PF with initial paragraph formatting properties
	prg->GetPF()->Get(&_PF);

	// V-GUYB: PWord Converter requires loss notification.
	#ifdef REPORT_LOSSAGE
    if(!(_dwFlags & SFF_SELECTION)) // SFF_SELECTION is set if any kind of paste is being done.
    {
        ((LOST_COOKIE*)(_pes->dwCookie))->bLoss = FALSE;
    }
	#endif // REPORT_LOSSAGE

	// Valid RTF files start with LBRACE "\rtf" or "\pwd"
	if (TokenGetToken() != tokenStartGroup	||
		HandleToken()						||
		TokenGetToken() != tokenRtf && _token != tokenPocketWord ||
		HandleToken() )
	{
		_ecParseError = ecUnexpectedToken;		// Signal bad file
		goto CleanUp;
	}

	// If initial cp follows EOP, use it for _cpThisPara.  Else
	// search for start of para containing the initial cp.
	_cpThisPara = _cpFirst;
	if (!prg->_rpTX.IsAfterEOP())
	{
		CTxtPtr	tp(prg->_rpTX);
		tp.FindEOP(tomBackward);
		_cpThisPara	= tp.GetCp();
	}

	while ( TokenGetToken() != tokenEOF &&		// Process remaining tokens
			_token != tokenError		&&
			!HandleToken()				&&
			_pstateStackTop )
		;

	AssertSz(!_iCell,
		"CRTFRead::ReadRTF: Inserted cells but no row end");

	prg->SetIgnoreFormatUpdate(FALSE);			// Enable range _iFormat updates
	prg->Update_iFormat(-1); 				    // Update _iFormat to CF 
												//  at current active end
	if(!(_dwFlags & SFF_SELECTION))				// RTF applies to document:
	{											//  update CDocInfo
		// Apply char and para formatting of
		//  final text run to final CR
		if(prg->GetCp() == (LONG)_ped->GetAdjustedTextLength())
		{
			Apply_PF();
			prg->ExtendFormattingCRLF();
		}

		// Update the per-document information from the RTF read
		CDocInfo *pDocInfo = _ped->GetDocInfo();

		if(!pDocInfo)
		{
			_ecParseError = ecNoMemory;
			goto CleanUp;
		}

		pDocInfo->wCpg = _nCodePage == INVALID_CODEPAGE ? 
										tomInvalidCpg : _nCodePage;

		_ped->SetDefaultLCID(_sDefaultLanguage == INVALID_LANGUAGE ?
								tomInvalidLCID : 
								MAKELCID(_sDefaultLanguage, SORT_DEFAULT));

		_ped->SetDefaultLCIDFE(_sDefaultLanguageFE == INVALID_LANGUAGE ?
								tomInvalidLCID :
								MAKELCID(_sDefaultLanguageFE, SORT_DEFAULT));

		_ped->SetDefaultTabStop(TWIPS_TO_FPPTS(_sDefaultTabWidth));
		_ped->SetDocumentType(_bDocType);
	}

CleanUp:
	FreeRtfObject();

	pstate = _pstateStackTop;
	if(pstate)									// Illegal RTF file. Release
	{											//  unreleased format indices
		if (ecNoError == _ecParseError)			// It's only an overflow if no
		{										//  other error has occurred
			_ecParseError = ecStackOverflow;
		}
		while(pstate->pstatePrev)
		{
			pstate = pstate->pstatePrev;
			ReleaseFormats(pstate->iCF, -1);
		}
	}

	pstate = _pstateLast;
	if( pstate )
	{
		while(pstate->pstatePrev)				// Free all but first STATE
		{
			pstate->DeletePF();
			pstate = pstate->pstatePrev;
			FreePv(pstate->pstateNext);
		}
		pstate->DeletePF();
	}
	FreePv(pstate);								// Free first STATE
	FreePv(_szUnicode);
	FreePv( _szHyperlinkFldinst );
	FreePv( _szHyperlinkFldrslt );

Quit:
	DeinitLex();

	if(_pcpObPos)
	{
		if((_ped->GetEventMask() & ENM_OBJECTPOSITIONS) && _cobPos > 0)
		{
			OBJECTPOSITIONS obpos;

			obpos.cObjectCount = _cobPos;
			obpos.pcpPositions = _pcpObPos;

			_ped->TxNotify(EN_OBJECTPOSITIONS, &obpos);
		}

		FreePv(_pcpObPos);
		_pcpObPos = NULL;
	}

#ifdef MACPORT
#if defined(ERROR_HANDLE_EOF) && ERROR_HANDLE_EOF != 38L
#error "ERROR_HANDLE_EOF value incorrect"
#endif
// transcribed from winerror.h
#define ERROR_HANDLE_EOF                 38L
#endif

	// FUTURE(BradO):  We should devise a direct mapping from our error codes
	//					to Win32 error codes.  In particular our clients are
	//					not expecting the error code produced by:
	//						_pes->dwError = (DWORD) -(LONG) _ecParseError;
	if(_ecParseError)
	{
		AssertSz(_ecParseError >= 0,
			"Parse error is negative");

		if (_ecParseError == ecTextMax)
		{
			_ped->GetCallMgr()->SetMaxText();
			_pes->dwError = (DWORD)STG_E_MEDIUMFULL;
		}
		if (_ecParseError == ecUnexpectedEOF)
			_pes->dwError = (DWORD)HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

		if (!_pes->dwError && _ecParseError != ecTruncateAtCRLF)
			_pes->dwError = (DWORD) -(LONG) _ecParseError;

		TRACEERRSZSC("CchParse_", _pes->dwError);
		
		if (ecNoError < _ecParseError && _ecParseError < ecLastError)
			Tracef(TRCSEVERR, "Parse error: %s", rgszParseError[_ecParseError]);
	}
#ifdef PWD_JUPITER
	else if(_fFoundCollapsedText && !(_dwFlags & SFF_SELECTION))
	{
		// GuyBark Jupiter 31960: We found some collapsed text while streaming 
		// in a document. (!SFF_SELECTION menas this isn't a paste). So do PWord
		// a favor and allow it to clear its level combo box.
		_ped->TxNotify(EN_PARAGRAPHEXPANDED, NULL);    
	}
#endif // PWD_JUPITER


	return prg->GetCp() - _cpFirst;
}
#pragma warning ( default : 4390 )

/*
 *	CRTFRead::CpgInfoFromFaceName()
 *
 *	@mfunc
 *		This routine fills in the TEXTFONT::bCharSet and TEXTFONT::nCodePage
 *		members of the TEXTFONT structure by querying the system for the
 *		metrics of the font described by TEXTFONT::szName.
 *
 *	@rdesc
 *		A flag indicating whether the charset and codepage were successfully
 *		determined.
 *
 */
BOOL CRTFRead::CpgInfoFromFaceName(TEXTFONT *ptf)
{
	//FUTURE(BradO):  This code is a condensed version of a more sophisticated
	//	algorithm we use in font.cpp to second-guess the font-mapper.
	//	We should factor out the code from font.cpp for use here as well.

	BOOL fRet = FALSE;
	LOGFONT lf = {0};
	HFONT hfont;
	HDC hdc;
	HFONT hfontOld;
	WCHAR szFaceName[LF_FACESIZE + 1];
	TEXTMETRIC tm;

	if(ptf->fNameIsDBCS)
	{
		// If fNameIsDBCS, we have high-ANSI characters in the facename, and
		// no codepage with which to interpret them.  The facename is gibberish,
		// so don't waste time calling the system to match it.
		goto done;
	}

	hdc = NULL;
	hfontOld = NULL;

	wcscpy_s(lf.lfFaceName, LF_FACESIZE,ptf->szName);
	lf.lfCharSet = DEFAULT_CHARSET;

	if((hfont = CreateFontIndirect(&lf)) &&
		(hdc = _ped->TxGetDC()) &&
		(hfontOld = (HFONT)SelectObject(hdc, hfont)) &&
		GetTextFace(hdc, LF_FACESIZE, szFaceName) &&
		!wcscmp(lf.lfFaceName, szFaceName) &&
		W32->GetTextMetrics(hdc, &tm) &&
		tm.tmCharSet != DEFAULT_CHARSET)
	{
		fRet = TRUE;

		ptf->bCharSet = tm.tmCharSet;
		ptf->sCodePage = GetCodePage(tm.tmCharSet);
    }
    else  //CreateFont failed, but we should still need to init these 
    {
		ptf->bCharSet = DEFAULT_CHARSET;
		ptf->sCodePage = GetCodePage(DEFAULT_CHARSET);
    }		
        

	// Cleanup
	if(hdc)
	{
		if(hfontOld)
		{
			SelectObject(hdc, hfontOld);
		}

		_ped->TxReleaseDC(hdc);
	}

	if(hfont)
	{
		DeleteObject(hfont);
	}

done:
	// Indicates that we've tried to obtain the cpg info from the system,
	// so that after a failure we don't re-call this routine.	
	ptf->fCpgFromSystem = TRUE;

	return fRet;
}

/*
 *	CRTFRead::SetupEquationOffset()
 *
 *  GuyBark JupiterJ: ADDED THIS!
 *
 *  Look for y offset component of an equation string.
 *
 */
void CRTFRead::SetupEquationOffset(LPSTR psz)
{
    LPSTR pszGroupEnd = ")";
    LPSTR pszUp       = "\\up";
    LPSTR pszDown     = "\\do";
    LPSTR pszNow, pszPrev, pszNext;

    int dy = 0;

    // Validate input.
    if(!psz)
    {
        return;
    }

    // Find the last end-of-equation group token in the string if there is one.
    pszPrev = (LPSTR)psz;

    // It's ok to look the characters here in this way, as we know this is an 
    // ASCII string, ie it doesn't contain any DBCS characters.

    while((pszNext = strstr((LPSTR)pszPrev, pszGroupEnd)))
    {
        // We found an end-of-group, so so reset the offset at this point.        
        _CF.yOffset = 0;

        pszPrev = pszNext + 1;
    }                

    // Carry on from this point to find and offset tokens.
    pszNow = pszPrev;

    // Now find the last offset token in the string from this point on.
    while(pszNow)
    {
        if((pszNext = strstr(pszNow, pszUp)))
        {
            pszNext += strlen(pszUp);

            dy = atoi(pszNext);
        }
        else if((pszNext = strstr(pszNow, pszDown)))
        {
            pszNext += strlen(pszDown);

            dy = -atoi(pszNext);
        }

        pszNow = pszNext;
    }

    // Apply an offset if we found it.
    if(dy)
    {
        _CF.yOffset = dy * 10; // Half points->twips
        _CF.dwMask |= CFM_OFFSET;
    }

    return;
}

#if defined(DEBUG)
/*
 *	CRTFRead::TestParserCoverage()
 *
 *	@mfunc
 *		A debug routine used to test the coverage of HandleToken.  The routine
 *		puts the routine into a debug mode and then determines:
 *		
 *			1.  Dead tokens - (T & !S & !P)
 *				Here, token:
 *					a) is defined in tom.h  (T)
 *					b) does not have a corresponding keyword (not scanned)  (!S)
 *					c) is not processed by HandleToken  (!P)
 *			2.  Tokens that are parsed but not scanned - (T & !S & P)
 *				Here, token:
 *					a) is defined in tom.h  (T)
 *					b) does not have a corresponding keyword (not scanned)  (!S}
 *					c) is processed by HandleToken  (P)
 *			3.  Tokens that are scanned but not parsed - (T & S & !P)
 *				Here, token:
 *					a) is defined in tom.h  (T)
 *					b) does have a corresponding keyword (is scanned)  (S)
 *					c) is not processed by HandleToken  (!P)
 *					
 */
void CRTFRead::TestParserCoverage()
{
	int i;
	char *rgpszKeyword[tokenMax - tokenMin];
	BOOL rgfParsed[tokenMax - tokenMin];

	// put HandleToken in debug mode
	_fTestingParserCoverage = TRUE;

	// gather info about tokens/keywords
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		rgpszKeyword[i] = PszKeywordFromToken(i + tokenMin);
		_token = i + tokenMin;
		rgfParsed[i] = HandleToken() == ecNoError ? TRUE : FALSE;
	}

	// reset HandleToken to non-debug mode
	_fTestingParserCoverage = FALSE;

	// Should coverage check include those we know will fail test, but
	// which we've examined and know why they fail?
	BOOL fExcuseCheckedToks = TRUE;

	if(GetProfileIntA("RICHEDIT DEBUG", "RTFCOVERAGESTRICT", 0))
	{
		fExcuseCheckedToks = FALSE;
	}

	// (T & !S & !P)  (1. above)
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
	  	if(rgpszKeyword[i] || rgfParsed[i]) 
		{
			continue;
		}

		TOKEN tok = i + tokenMin;

		// token does not correspond to a keyword, but still may be scanned
		// check list of individual symbols which are scanned
		if(FTokIsSymbol(tok))
		{
			continue;
		}

		// check list of tokens which have been checked and fail
		// the sanity check for some known reason (see FTokFailsCoverageTest def'n)
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
		{
			continue;
		}

		char szBuf[256];

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token neither scanned nor parsed - token = %d", tok);
		AssertSz(0, szBuf);
	}
				
	// (T & !S & P)  (2. above)
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		if(rgpszKeyword[i] || !rgfParsed[i])
		{
			continue;
		}

		TOKEN tok = i + tokenMin;

		// token does not correspond to a keyword, but still may be scanned
		// check list of individual symbols which are scanned
		if(FTokIsSymbol(tok))
		{
			continue;
		}

		// check list of tokens which have been checked and fail
		// the sanity check for some known reason (see FTokFailsCoverageTest def'n)
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
		{
			continue;
		}

		char szBuf[256];

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token parsed but not scanned - token = %d", tok);
		AssertSz(0, szBuf);
	}

	// (T & S & !P)  (3. above)
	for(i = 0; i < tokenMax - tokenMin; i++)
	{
		if(!rgpszKeyword[i] || rgfParsed[i])
		{
			continue;
		}

		TOKEN tok = i + tokenMin;

		// check list of tokens which have been checked and fail
		// the sanity check for some known reason (see FTokFailsCoverageTest def'n)
		if(fExcuseCheckedToks && FTokFailsCoverageTest(tok))
		{
			continue;
		}

		char szBuf[256];

		sprintf(szBuf, "CRTFRead::TestParserCoverage():  Token scanned but not parsed - token = %d, tag = \\%s", tok, rgpszKeyword[i]);
		AssertSz(0, szBuf);
	}
}


/*
 *	CRTFRead::PszKeywordFromToken()
 *
 *	@mfunc
 *		Searches the array of keywords and returns the keyword
 *		string corresponding to the token supplied
 *
 *	@rdesc
 *		returnes a pointer to the keyword string if one exists
 *		and NULL otherwise
 */
CHAR *CRTFRead::PszKeywordFromToken(TOKEN token)
{
	extern KEYWORD rgKeyword[];

	for(int i = 0; i < cKeywords; i++)
	{
		if(rgKeyword[i].token == token) 
		{
			return rgKeyword[i].szKeyword;
		}
	}
	
	return NULL;
}


/*
 *	CRTFRead::FTokIsSymbol(TOKEN tok)
 *
 *	@mfunc
 *		Returns a BOOL indicating whether the token, tok, corresponds to an RTF symbol
 *		(that is, one of a list of single characters that are scanned in the
 *		RTF reader)
 *
 *	@rdesc
 *		BOOL - 	indicates whether the token corresponds to an RTF symbol
 *
 */
BOOL CRTFRead::FTokIsSymbol(TOKEN tok)
{
	const BYTE *pbSymbol;

	extern const BYTE szSymbolKeywords[];
	extern const TOKEN tokenSymbol[];

	// check list of individual symbols which are scanned
	for(pbSymbol = szSymbolKeywords; *pbSymbol; pbSymbol++)
	{
		if(tokenSymbol[pbSymbol - szSymbolKeywords] == tok)
		{
			return TRUE;
		}
	}

	return FALSE;
}


/*
 *	CRTFRead::FTokFailsCoverageTest(TOKEN tok)
 *
 *	@mfunc
 *		Returns a BOOL indicating whether the token, tok, is known to fail the
 *		RTF parser coverage test.  These tokens are those that have been checked 
 *		and either:
 *			1) have been implemented correctly, but just elude the coverage test
 *			2) have yet to be implemented, and have been recognized as such
 *
 *	@rdesc
 *		BOOL - 	indicates whether the token has been checked and fails the
 *				the parser coverage test for some known reason
 *
 */
BOOL CRTFRead::FTokFailsCoverageTest(TOKEN tok)
{
	switch(tok)
	{
	// (T & !S & !P)  (1. in TestParserCoverage)
		// these really aren't tokens per se, but signal ending conditions for the parse
		case tokenError:
		case tokenEOF:

	// (T & !S & P)  (2. in TestParserCoverage)
		// emitted by scanner, but don't correspond to recognized RTF keyword
		case tokenUnknownKeyword:
		case tokenText:
		case tokenASCIIText:

		// recognized directly (before the scanner is called)
		case tokenStartGroup:
		case tokenEndGroup:

		// recognized using context information (before the scanner is called)
		case tokenObjectDataValue:
		case tokenPictureDataValue:

	// (T & S & !P)  (3. in TestParserCoverage)
		// None

			return TRUE;
	}

	return FALSE;
}
#endif // DEBUG


