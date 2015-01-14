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
 *	@module	RTFWRIT.CPP - RichEdit RTF Writer (w/o objects) |
 *
 *		This file contains the implementation of the RTF writer
 *		for the RichEdit control, except for embedded objects,
 *		which are handled mostly in rtfwrit2.cpp
 *
 *	Authors: <nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent <nl>
 *		Lots of enhancements: Brad Olenick <nl>
 *
 */

#include "_common.h"
#include "_rtfwrit.h"
#include "_objmgr.h"
#include "_coleobj.h"

ASSERTDATA

extern KEYWORD rgKeyword[];

//========================= Global String Constants ==================================

BYTE bCharSetANSI = ANSI_CHARSET;				// ToDo: make more general

#ifdef DEBUG
// Quick way to find out what went wrong: rgszParseError[ecParseError]
//
CHAR *	rgszParseError[] =
{
	"No error",
	"Can't convert to Unicode",				// FF
	"Color table overflow",					// FE
	"Expecting '\\rtf'",					// FD
	"Expecting '{'",						// FC
	"Font table overflow",					// FB
	"General failure",						// FA
	"Keyword too long",						// F9
	"Lexical analyzer initialize failed",	// F8
	"No memory",							// F7
	"Parser is busy",						// F6
	"PutChar() function failed",			// F5
	"Stack overflow",						// F4
	"Stack underflow",						// F3
	"Unexpected character",					// F2
	"Unexpected end of file",				// F1
	"Unexpected token",						// F0
	"UnGetChar() function failed",			// EF
	"Maximum text length reached",			// EE
	"Streaming out object failed",			// ED
	"Streaming in object failed",			// EC
	"Truncated at CR or LF",				// EB
	"Format-cache failure",					// EA
	NULL									// End of list marker
};

CHAR * szDest[] =
{
	"RTF",
	"Color Table",
	"Font Table",
	"Binary",
	"Object",
	"Object Class",
	"Object Name",
	"Object Data",
	"Field",
	"Field Result",
	"Field Instruction",
	"Symbol",
	"Paragraph Numbering",
	"Picture"
};

#endif

// Most control-word output is done with the following printf formats
static const CHAR * rgszCtrlWordFormat[] =
{
	"\\%s", "\\%s%d", "{\\%s", "{\\*\\%s", "{\\%s%d"
};

// Special control-word formats
static const CHAR szBeginFontEntryFmt[]	= "{\\f%d\\%s";
static const CHAR szBulletGroup[]		= "{\\pntext\\f%d\\'B7\\tab}";
static const CHAR szBulletFmt[]			= "{\\*\\pn\\pnlvlblt\\pnf%d\\pnindent%d{\\pntxtb\\'B7}}";
static const CHAR szBeginNumberGroup[]	= "{\\pntext\\f%d ";
static const CHAR szEndNumberGroup[]	= "\\tab}";
static const CHAR szBeginNumberFmt[]	= "{\\*\\pn\\pnlvl%s\\pnf%d\\pnindent%d\\pnstart%d";
static const CHAR szpntxtb[]			= "{\\pntxtb(}";
static const CHAR szpntxta[]			= "{\\pntxta%c}";
static const CHAR szColorEntryFmt[]		= "\\red%d\\green%d\\blue%d;";
static const CHAR szEndFontEntry[]		= ";}";
       const CHAR szEndGroupCRLF[]		= "}\r\n";
static const CHAR szEscape2CharFmt[]	= "\\'%02x\\'%02x";
static const CHAR szLiteralCharFmt[]	= "\\%c";
static const CHAR szPar[]				= "\\par\r\n";
static const CHAR szObjPosHolder[] 		= "\\objattph\\'20";
static const CHAR szDefaultFont[]		= "\\deff0";
static const CHAR szHorzdocGroup[]		= "{\\horzdoc}";
static const CHAR szNormalStyle[]		= "{ Normal;}";
static const CHAR szHeadingStyle[]		= "{\\s%d heading %d;}";
static const CHAR szEndRow[]			= "\\row\r\n";
static const CHAR szPwdComment[]		= "{\\*\\pwdcomment ";

#define szEscapeCharFmt		&szEscape2CharFmt[6]

const WORD rgiszTerminators[] =
{	i_cell, 0, i_tab, 0, i_line, i_page};

// Keep these indices in sync with the special character values in _common.h
const WORD rgiszSpecial[] =
{
	i_enspace,
	i_emspace,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	i_endash,		
	i_emdash,
	0,
	0,
	0,
	i_lquote, 
	i_rquote,
	0,
	0,
	i_ldblquote, 
	i_rdblquote,
	0,
	0,
	0,
	0,
	i_bullet
};

const WORD rgiszEffects[] =							
{													// Effects keywords
	i_revised, i_disabled, i_impr, i_embo,			// Ordered max CFE_xx to
	i_shad, i_outl, i_v, i_caps, i_scaps,		 	//  min CFE_xx
	i_disabled, i_protect, i_strike, i_ul, i_i,	i_b	// (see WriteCharFormat())
};													

#define CEFFECTS	ARRAY_SIZE(rgiszEffects)

const WORD rgiszPFEffects[] =						// PF effects keywords
{													// Ordered max PFE_xx to
	i_collapsed, i_sbys, i_hyphpar, i_nowidctlpar,	//  min PFE_xx
	i_noline, i_pagebb, i_keepn, i_keep, i_rtlpar
};													// (see WriteParaFormat())

#define CPFEFFECTS	ARRAY_SIZE(rgiszPFEffects)

const WORD rgiszUnderlines[] =
{
	i_ul, i_ulw, i_uldb, i_uld,						// Std Word underlines
	i_uldash, i_uldashd, i_uldashdd, i_ulwave, i_ulth, i_ulhair
};

#define CUNDERLINES	ARRAY_SIZE(rgiszUnderlines)

const WORD rgiszFamily[] =							// Font family RTF name
{													//  keywords in order of
	i_fnil, i_froman, i_fswiss, i_fmodern,			//  bPitchAndFamily
	i_fscript, i_fdecor
//  , i_ftech, i_fbidi								// TODO
};

const WORD rgiszAlignment[] =						// Alignment keywords
{													// Keep in sync with
	i_ql, i_qr,	i_qc, i_qj							//  alignment constants
};

const WORD rgiszTabAlign[] =						// Tab alignment keywords
{													// Keep in sync with tab
	i_tqc, i_tqr, i_tqdec							//  alignment constants
};

const WORD rgiszTabLead[] =							// Tab leader keywords
{													// Keep in sync with tab
	i_tldot, i_tlhyph, i_tlul, i_tlth, i_tleq		//  leader constants
};

const WORD rgiszNumberStyle[] =						// Numbering style keywords
{													// Keep in sync with TOM
	i_pndec, i_pnlcltr, i_pnucltr,					//  values 
	i_pnlcrm, i_pnucrm, 
    i_pnaiueod, i_pnirohad    // GuyBark JupiterJ: added these
};

const WORD rgiszBorders[] =							// Border combination keywords
{													
	i_box,
	i_brdrt, i_brdrl, i_brdrb, i_brdrr,
	i_trbrdrt, i_trbrdrl, i_trbrdrb, i_trbrdrr,
	i_clbrdrt, i_clbrdrl, i_clbrdrb, i_clbrdrr
};

const WORD rgiszBorderStyles[] =					// Border style keywords
{													
	i_brdrdash, i_brdrdashsm, i_brdrdb, i_brdrdot,
	i_brdrhair, i_brdrs, i_brdrth, i_brdrtriple
};
#define CBORDERSTYLES ARRAY_SIZE(rgiszBorderStyles)

const WORD rgiszBorderEffects[] =					// Border effect keywords
{													
	i_brdrbar, i_brdrbtw, i_brdrsh					// Reverse order from bits
};

const WORD rgiszShadingStyles[] =					// Shading style keywords
{													
	i_bgbdiag, i_bgcross, i_bgdcross, i_bgdkbdiag,
	i_bgdkcross, i_bgdkdcross, i_bgdkfdiag, i_bgdkhoriz,
	i_bgdkvert, i_bgfdiag, i_bghoriz, i_bgvert 
};
#define CSHADINGSTYLES ARRAY_SIZE(rgiszShadingStyles)

// RGB with 2 bits per color type (in BGR order)
const COLORREF g_Colors[] =
{
	RGB(  0,   0,   0),	// \red0\green0\blue0
	RGB(  0,   0, 255),	// \red0\green0\blue255
	RGB(  0, 255, 255),	// \red0\green255\blue255
	RGB(  0, 255,   0),	// \red0\green255\blue0
	RGB(255,   0, 255),	// \red255\green0\blue255
	RGB(255,   0,   0),	// \red255\green0\blue0
	RGB(255, 255,   0),	// \red255\green255\blue0
	RGB(255, 255, 255),	// \red255\green255\blue255
	RGB(  0,   0, 128),	// \red0\green0\blue128
	RGB(  0, 128, 128),	// \red0\green128\blue128
	RGB(  0, 128,   0),	// \red0\green128\blue0
	RGB(128,   0, 128),	// \red128\green0\blue128
	RGB(128,   0,   0),	// \red128\green0\blue0
	RGB(128, 128,   0),	// \red128\green128\blue0
	RGB(128, 128, 128),	// \red128\green128\blue128
	RGB(192, 192, 192),	// \red192\green192\blue192
};

/*
 *	CRTFWrite::MapsToRTFKeywordW(wch)
 *
 *	@mfunc
 *		Returns a flag indicating whether the character maps to an RTF keyword
 *
 *	@rdesc
 *		BOOL			TRUE if char maps to RTF keyword
 */
inline BOOL CRTFWrite::MapsToRTFKeywordW(WCHAR wch)
{
	return
		IN_RANGE(TAB, wch, CR) ||
#ifdef PWD_JUPITER
        // GuyBark Jupiter: Handle the special character for CRLF in table cells.
        wch == PWD_CRLFINCELL ||
#endif // PWD_JUPITER
		wch == CELL ||
		wch == CELL ||
		wch == BSLASH ||
		wch == LBRACE || 
		wch == RBRACE ||
		IN_RANGE(ENSPACE, wch, EMSPACE) ||
		IN_RANGE(ENDASH, wch, EMDASH) ||
		IN_RANGE(LQUOTE, wch, RQUOTE) ||
		IN_RANGE(LDBLQUOTE, wch, RDBLQUOTE) ||
		wch == BULLET ||
		wch == chOptionalHyphen ||
		wch == chNonBreakingSpace;
}

/*
 *	CRTFWrite::MapsToRTFKeywordA(ch)
 *
 *	@mfunc
 *		Returns a flag indicating whether the character maps to an RTF keyword
 *
 *	@rdesc
 *		BOOL			TRUE if char maps to RTF keyword
 */
inline BOOL CRTFWrite::MapsToRTFKeywordA(char ch)
{
	return IN_RANGE(TAB, ch, CR) ||
		ch == CELL ||
		ch == BSLASH ||
		ch == LBRACE || 
		ch == RBRACE;
}

/*
 *	CRTFWrite::MapToRTFKeywordW(pv, cch, iCharEncoding)
 *
 *	@mfunc
 *		Examines the first character in the string pointed to by pv and
 *		writes out the corresponding RTF keyword.  In situations where
 *		the first and subsequent characters map to a single keyword, we
 *		return the number of additional characters used in the mapping.
 *
 *	@rdesc
 *		int		indicates the number of additional characters used when
 *				the mapping to an RTF keyword involves > 1 characters.
 */
int CRTFWrite::MapToRTFKeyword(
	void *	pv,				//@parm ptr to ansi or Unicode string
	int		cch,
	int		iCharEncoding)
{
	Assert(iCharEncoding == MAPTOKWD_ANSI || iCharEncoding == MAPTOKWD_UNICODE);

	WCHAR ch = ((iCharEncoding == MAPTOKWD_ANSI) ? *(char *)pv : *(WCHAR *)pv);
	int cchRet = 0;

	Assert((iCharEncoding == MAPTOKWD_ANSI) ? MapsToRTFKeywordA(ch) : MapsToRTFKeywordW(ch));

	switch(ch)
	{
#ifdef PWD_JUPITER
        // GuyBark Jupiter: We've hit the special character inserting when 
        // reading the file, to represent a CRLF in a table cell.
        case PWD_CRLFINCELL:
        {
            // Mimic the rtf output by Word97 in this situation.
            char szParInTable[] = "\r\n\\par";
            
            Puts(szParInTable, strlen(szParInTable));

            break;
        }
#endif // PWD_JUPITER
		case BULLET:
		case EMDASH:
		case EMSPACE:
		case ENDASH:
		case ENSPACE:
		case LDBLQUOTE:
		case LQUOTE:
		case RDBLQUOTE:
		case RQUOTE:
			Assert(ch > 0xFF);

			if(iCharEncoding != MAPTOKWD_ANSI)
			{
				AssertSz(rgiszSpecial[ch - ENSPACE] != 0,
					"CRTFWrite::WriteText(): rgiszSpecial out-of-sync");
				PutCtrlWord(CWF_STR, rgiszSpecial[ch - ENSPACE]);
			}
			break;

		case FF:
		case VT:
		case TAB:
		case CELL:

			PutCtrlWord(CWF_STR, rgiszTerminators[ch - CELL]);
			break;

		case CR:
		{
			WCHAR ch1;
			WCHAR ch2;

			if(iCharEncoding == MAPTOKWD_ANSI)
			{
				char *pch = (char *)pv;
				ch1 = pch[1];
				ch2 = pch[2];
			}
			else
			{
				WCHAR *pch = (WCHAR *)pv;
				ch1 = pch[1];
				ch2 = pch[2];
			}

			if(cch > 1 && ch1 == CR && ch2 == LF)
			{
				// Translate CRCRLF	to a blank (represents soft line break)
				PutChar(' ');
				cchRet = 2;
				break;
			}
			if(cch && ch1 == LF)		// Ignore LF after CR
			{
				cchRet = 1;
			}							
			if(_pPF->InTable())			// CR terminates a row in our simple
			{							//  table model, so output \row
				Puts(szEndRow, sizeof(szEndRow) - 1);
				_fCheckInTable = TRUE;

				break;
			}
		}								// Fall thru to LF (EOP) case

		case LF:
			Puts(szPar, sizeof(szPar) - 1);
			if(_fBullet)
			{
				if(cch > 0)
				{
					if(!_nNumber) 
						printF(szBulletGroup, _symbolFont);

					else if(!_pPF->IsNumberSuppressed())
					{
						WCHAR szNumber[CCHMAXNUMTOSTR];
						_pPF->NumToStr(szNumber, ++_nNumber);
						printF(szBeginNumberGroup, _nFont);
						WritePcData(szNumber, _cpg, FALSE);
						printF(szEndNumberGroup);
					}
				}
				else
					_fBulletPending = TRUE;
			}
			break;

		case chOptionalHyphen:
			ch = '-';					// Fall thru to printFLiteral

printFLiteral:
		case BSLASH:
		case LBRACE:
		case RBRACE:
			printF(szLiteralCharFmt, ch);
			break;

		case chNonBreakingSpace:
			ch = '~';
			goto printFLiteral;
	}
	
	return cchRet;
}


//======================== CRTFConverter Base Class ==================================

/*
 *	CRTFConverter::CRTFConverter()
 *
 *	@mfunc
 *		RTF Converter constructor
 */
CRTFConverter::CRTFConverter(
	CTxtRange *		prg,			// @parm CTxtRange for transfer
	EDITSTREAM *	pes,			// @parm Edit stream for transfer
	DWORD			dwFlags,		// @parm Converter flags
	BOOL 			fRead)			// @parm Initialization for a reader or writer
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFConverter::CRTFConverter");

	AssertSz(prg && pes && pes->pfnCallback,
		"CRTFWrite::CRTFWrite: Bad RichEdit");

	_prg			= prg;
	_pes			= pes;
	_ped			= prg->GetPed();
	_dwFlags		= dwFlags;
	_ecParseError	= ecNoError;

	if(!_ctfi)
	{
		ReadFontSubInfo();
	}

#if defined(DEBUG) && !defined(MACPORT)
	_hfileCapture = NULL;

#if !defined(PEGASUS)
	if(GetProfileIntA("RICHEDIT DEBUG", "RTFCAPTURE", 0))
	{
		char szTempPath[MAX_PATH] = "\0";
		const char cszRTFReadCaptureFile[] = "CaptureRead.rtf";
		const char cszRTFWriteCaptureFile[] = "CaptureWrite.rtf";
		DWORD cchLength;
		
		SideAssert(cchLength = GetTempPathA(MAX_PATH, szTempPath));

		// append trailing backslash if neccessary
		if(szTempPath[cchLength - 1] != '\\')
		{
			szTempPath[cchLength] = '\\';
			szTempPath[cchLength + 1] = 0;
		}

		strcat(szTempPath, fRead ? cszRTFReadCaptureFile : 
									cszRTFWriteCaptureFile);
		
		SideAssert(_hfileCapture = CreateFileA(szTempPath,
											GENERIC_WRITE,
											FILE_SHARE_READ,
											NULL,
											CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL,
											NULL));
	}
#endif // !defined(PEGASUS)

#endif // defined(DEBUG) && !defined(MACPORT)
}



//======================== OLESTREAM functions =======================================

DWORD CALLBACK RTFPutToStream (
	RTFWRITEOLESTREAM *	OLEStream,	//@parm OLESTREAM
	const void *		pvBuffer,	//@parm Buffer to  write
	DWORD				cb)			//@parm Bytes to write
{
	return OLEStream->Writer->WriteData ((BYTE *)pvBuffer, cb);
}



//============================ CRTFWrite Class ==================================

/*
 *	CRTFWrite::CRTFWrite()
 *
 *	@mfunc
 *		RTF writer constructor
 */
CRTFWrite::CRTFWrite(
	CTxtRange *		prg,			// @parm CTxtRange to write
	EDITSTREAM *	pes,			// @parm Edit stream to write to
	DWORD			dwFlags)		// @parm Write flags
	: CRTFConverter(prg, pes, dwFlags, FALSE)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::CRTFWrite");

	ZeroMemory(&_CF, sizeof(CCharFormat));	// Setup "previous" CF with RTF
	_CF.cbSize		= sizeof(CHARFORMAT2); 	//  defaults. 12 Pt in twips
	_CF.dwEffects	= CFE_AUTOCOLOR | CFE_AUTOBACKCOLOR;//  Font info is given 
	_CF.yHeight		= 12*20;				//  by first font in range
											//  [see end of LookupFont()]
    if (NULL == _ped)
    {
	    Assert(_ped);
	    _CF.lcid = (LCID)0;
    }
    else
    {
	    _ped->GetDefaultLCID(&_CF.lcid);
    }
	// init OleStream
	RTFWriteOLEStream.Writer = this;
	RTFWriteOLEStream.lpstbl->Put = (DWORD (CALLBACK* )(LPOLESTREAM, const void FAR*, DWORD))
							   RTFPutToStream;
	RTFWriteOLEStream.lpstbl->Get = NULL;

	_fIncludeObjects = TRUE;
	if (dwFlags == SF_RTFNOOBJS)
		_fIncludeObjects = FALSE;

	_fNeedDelimeter = FALSE;
	_nHeadingStyle = 0;					// No headings found
	_nNumber = 0;						// No paragraph numbering yet
	_fCheckInTable = FALSE;
	_pPF = NULL;
	_pbAnsiBuffer = NULL;
}											

/*
 *	CRTFWrite::FlushBuffer()
 *
 *	@mfunc
 *		Flushes output buffer
 *
 *	@rdesc
 *		BOOL			TRUE if successful
 */
BOOL CRTFWrite::FlushBuffer()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::FlushBuffer");

	LONG cchWritten;

	if (!_cchBufferOut)
		return TRUE;

#ifdef DEBUG_PASTE
	if (FromTag(tagRTFAsText))
	{
		CHAR *	pchEnd	= &_pchRTFBuffer[_cchBufferOut];
		CHAR	chT		= *pchEnd;

		*pchEnd = 0;
		TraceString(_pchRTFBuffer);
		*pchEnd = chT;
	}
#endif

	_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
									  (unsigned char *)_pchRTFBuffer,
									  _cchBufferOut,	&cchWritten);

#if defined(DEBUG) && !defined(MACPORT) && !defined(PEGASUS)
	if(_hfileCapture)
	{
		DWORD cbLeftToWrite = _cchBufferOut;
		DWORD cbWritten2 = 0;
		BYTE *pbToWrite = (BYTE *)_pchRTFBuffer;
		
		while(WriteFile(_hfileCapture,
						pbToWrite,
						cbLeftToWrite,
						&cbWritten2,
						NULL) && 
						(pbToWrite += cbWritten2,
						(cbLeftToWrite -= cbWritten2)));
	}
#endif

	if (_pes->dwError)
	{
		_ecParseError = ecPutCharFailed; 
		return FALSE;
	}
	AssertSz(cchWritten == _cchBufferOut,
		"CRTFW::FlushBuffer: incomplete write");

	_cchOut		  += _cchBufferOut;
	_pchRTFEnd	  = _pchRTFBuffer;					// Reset buffer
	_cchBufferOut = 0;

	return TRUE;
}

/*
 *	CRTFWrite::PutChar(ch)
 *
 *	@mfunc
 *		Put out the character <p ch>
 *
 *	@rdesc
 *		BOOL	TRUE if successful
 */
BOOL CRTFWrite::PutChar(
	CHAR ch)				// @parm char to be put
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::PutChar");

	CheckDelimeter();					// If _fNeedDelimeter, may need to
										//  PutChar(' ')
	// Flush buffer if char won't fit
	if (_cchBufferOut + 1 >= cachBufferMost && !FlushBuffer())
		return FALSE;

	*_pchRTFEnd++ = ch;						// Store character in buffer
	++_cchBufferOut;	
	return TRUE;
}

/*
 *	CRTFWrite::CheckInTable(fPutIntbl)
 *
 *	@mfunc
 *		If _fCheckInTable or !fPutIntbl, output row header RTF. If fPutIntbl 
 *		and _fCheckInTable, output \intbl as well. Note that fPutIntbl is
 *		FALSE when a PF is being output, since this control word needs to
 *		be output after the \pard, but the other row RTF needs to be output
 *		before the \pard.
 *
 *	@rdesc
 *		BOOL	TRUE if in table and outputted all relevant \trowd stuff
 */
BOOL CRTFWrite::CheckInTable(
	BOOL fPutIntbl)		//@parm TRUE if \intbl should be output
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::CheckInTable");

	_fCheckInTable = FALSE;
	if(_pPF->InTable())
	{
		if(!_fRangeHasEOP)
			return TRUE;

		LONG  cTab = _pPF->cTabCount;
		LONG  h	   = _pPF->dxOffset;
		LONG  i, j = _pPF->dxStartIndent;
		LONG  k	   = _pPF->wAlignment;
		DWORD Tab, Widths;

		if (!PutCtrlWord(CWF_STR, i_trowd) || // Reset table properties
			h && !PutCtrlWord(CWF_VAL, i_trgaph, h) ||
			j && !PutCtrlWord(CWF_VAL, i_trleft, j) ||
			IN_RANGE(PFA_RIGHT, k, PFA_CENTER) &&
			!PutCtrlWord(CWF_STR, k == PFA_RIGHT ? i_trqr : i_trqc))
		{
			return FALSE;
		}
		PutBorders(TRUE);
		for(i = 0; i < cTab; i++)
		{
			Tab = _pPF->rgxTabs[i];
			Widths = Tab >> 24;
			if(Widths)
			{
				for(j = 0; j < 4; j++, Widths >>= 2)
				{
					LONG w = Widths & 3;
					if(w && (!PutCtrlWord(CWF_STR, rgiszBorders[j + 9]) ||
						!PutCtrlWord(CWF_VAL, i_brdrw, 15*w) ||
						!PutCtrlWord(CWF_STR, i_brdrs)))
					{
						return FALSE;
					}
				}
				CheckDelimeter();
			}
			if(!PutCtrlWord(CWF_VAL, i_cellx, GetTabPos(Tab)))
				return FALSE;
		}
		if(!fPutIntbl || PutCtrlWord(CWF_STR, i_intbl))
			return TRUE;
	}
	return FALSE;
}

/*
 *	CRTFWrite::PutBorders(fInTable)
 *
 *	@mfunc
 *		If any borders are defined, output their control words
 *
 *	@rdesc
 *		error code
 */
EC CRTFWrite::PutBorders(
	BOOL fInTable)
{
	if(_pPF->wBorderWidth)
	{
		DWORD Colors = _pPF->dwBorderColor;
		DWORD dwEffects = Colors >> 20;
		LONG  i = 1, iMax = 4;					// NonBox for loop limits
		LONG  j, k;
		DWORD Spaces = _pPF->wBorderSpace;
		DWORD Styles = _pPF->wBorders;
		DWORD Widths = _pPF->wBorderWidth;

		if(_pPF->wEffects & PFE_BOX)
			i = iMax = 0;						// For box, only write one set

		for( ; i <= iMax; i++, Spaces >>= 4, Styles >>= 4, Widths >>= 4, Colors >>= 5)
		{
			if(!(Widths & 0xF))						// No width, so no border
				continue;

			j = TWIPS_PER_POINT*(Spaces & 0xF);
			k = Colors & 0x1F;
			if (!PutCtrlWord(CWF_STR, rgiszBorders[i + 4*fInTable])		||
				!PutCtrlWord(CWF_STR, rgiszBorderStyles[Styles & 0xF])	||
				!PutCtrlWord(CWF_VAL, i_brdrw, 10*(Widths & 0xF))		||
				k &&
				!PutCtrlWord(CWF_VAL, i_brdrcf, LookupColor(g_Colors[k-1]) + 1) ||
				j && !PutCtrlWord(CWF_VAL, i_brsp, j))
			{
				break;
			}
			for(j = 3; j--; dwEffects >>= 1)		// Output border effects
			{
				if (dwEffects & 1 &&
					!PutCtrlWord(CWF_STR, rgiszBorderEffects[j]))
				{
					break;
				}				
			}
			CheckDelimeter();						// Output a ' '
		}
	}
	return _ecParseError;
}

/*
 *	CRTFWrite::Puts(sz, cb)
 *
 *	@mfunc
 *		Put out the string <p sz>
 *	
 *	@rdesc
 *		BOOL				TRUE if successful
 */
BOOL CRTFWrite::Puts(
	CHAR const * sz,
	LONG cb)		// @parm String to be put
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::Puts");

	if(*sz == '\\' || *sz == '{' || *sz == ' ')
		_fNeedDelimeter = FALSE;

	CheckDelimeter();					// If _fNeedDelimeter, may need to
										//  PutChar(' ')
	// Flush buffer if string won't fit
	if (cb < 0)
		return FALSE;
	if (_cchBufferOut + cb < _cchBufferOut || (_cchBufferOut + cb >= cachBufferMost && !FlushBuffer()))
		return FALSE;

	if (cb >= cachBufferMost)			// If buffer still can't handle string,
	{									//   we have to write string directly
		LONG	cbWritten;

#ifdef DEBUG_PASTE
		if (FromTag(tagRTFAsText))
			TraceString(sz);
#endif
		_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
										(LPBYTE) sz, cb, &cbWritten);
		_cchOut += cbWritten;

#if defined(DEBUG) && !defined(MACPORT) && !defined(PEGASUS)
		if(_hfileCapture)
		{
			DWORD cbLeftToWrite = cb;
			DWORD cbWritten2 = 0;
			BYTE *pbToWrite = (BYTE *)sz;
		
			while(WriteFile(_hfileCapture,
							pbToWrite,
							cbLeftToWrite,
							&cbWritten2,
							NULL) && 
							(pbToWrite += cbWritten2,
							(cbLeftToWrite -= cbWritten2)));
		}
#endif

		if (_pes->dwError)
		{
			_ecParseError = ecPutCharFailed;
			return FALSE;
		}
		AssertSz(cbWritten == cb,
			"CRTFW::Puts: incomplete write");
	}
	else
	{
		CopyMemory(_pchRTFEnd, sz, cb);		// Put string into buffer for later
		_pchRTFEnd += cb;							//  output
		_cchBufferOut += cb;
	}

	return TRUE;
}

/*
 *	CRTFWrite::PutCtrlWord(iFormat, iCtrl, iValue)
 *
 *	@mfunc
 *		Put control word with rgKeyword[] index <p iCtrl> and value <p iValue>
 *		using format rgszCtrlWordFormat[<p iFormat>]
 *
 *	@rdesc
 *		TRUE if successful
 *
 *	@devnote
 *		Sets _fNeedDelimeter to flag that next char output must be a control
 *		word delimeter, i.e., not alphanumeric (see PutChar()).
 */
BOOL CRTFWrite::PutCtrlWord(

	LONG iFormat,			// @parm Format index into rgszCtrlWordFormat
	LONG iCtrl,				// @parm Index into Keyword array
	LONG iValue)			// @parm Control-word parameter value. If missing,
{							//		 0 is assumed
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::PutCtrlWord");

	BOOL	bRet;

    const INT c_chT = 60;
	CHAR	szT[c_chT];
	LONG    cb;

	cb = sprintF(c_chT,
			  szT,
			  rgszCtrlWordFormat[iFormat],
			  rgKeyword[iCtrl].szKeyword,
			  iValue);
	_fNeedDelimeter = FALSE;

    // Truncate if needed
    cb = min(cb, c_chT);
    
	bRet = Puts(szT, cb);
	_fNeedDelimeter = TRUE;					// Ensure next char isn't
											//  alphanumeric
	return bRet;
}

/*
 *	CRTFWrite::printF(szFmt, ...)
 *
 *	@mfunc
 *		Provide formatted output
 *
 *	@rdesc
 *		TRUE if successful
 */
BOOL _cdecl CRTFWrite::printF(
	CONST CHAR * szFmt,		// @parm Format string for printf()
	...)					// @parmvar Parameter list
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::printF");
	va_list	marker;

    const INT c_chT = 60;
    
	CHAR	szT[c_chT];
	int cb;
	va_start(marker, szFmt);
	cb = W32->WvsprintfA(c_chT, szT, szFmt, marker);
	va_end(marker);

    // Truncate if needed
    cb = min(cb, c_chT);
    
	return Puts(szT, cb);
}

/*
 *	CRTFWrite::sprintF(szBuf, szFmt, ...)
 *
 *	@mfunc
 *		Provide formatted output to a string buffer
 *
 *	@rdesc
 *		number of bytes written
 */
LONG _cdecl CRTFWrite::sprintF(
	LONG cbBuf,
	CHAR *szBuf,
	CONST CHAR * szFmt,		// @parm Format string for printf()
	...)					// @parmvar Parameter list
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::printF");
	va_list	marker;
	int cb;
	va_start(marker, szFmt);
	cb = W32->WvsprintfA(cbBuf, szBuf, szFmt, marker);
	va_end(marker);
	return cb;
}
/*
 *	CRTFWrite::WritePcData(szData, nCodePage, fIsDBCS)
 *
 *	@mfunc
 *		Write out the string <p szData> as #PCDATA where any special chars
 *		are protected by leading '\\'.
 *
 *	@rdesc
 *		EC (_ecParseError)
 */
EC CRTFWrite::WritePcData(
	const TCHAR * szData,	// @parm #PCDATA string to write
	INT  nCodePage,			// @parm code page  default value CP_ACP
	BOOL fIsDBCS)			// @parm szData is a DBCS string stuffed into Unicode buffer
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WritePcData");

	BYTE		ch;
	BOOL		fMissingCodePage;
	BOOL		fMultiByte;
	const BYTE *pch;
	const char *pchToDBCSDefault = NULL;
	BOOL *		pfUsedDefault = NULL;

	if(_dwFlags & SFF_UTF8)				// Use UTF-8 for all conversions
		nCodePage = CP_UTF8;			// (doesn't work for unknown cpg's
										//  i.e., if fIsDBCS is TRUE...)
	if(!*szData)
	{
		return _ecParseError;
	}

	int	DataSize = wcslen(szData) + 1;
	int BufferSize = DataSize * 2;

#ifdef PWD_JUPITER
	// GuyBark JupiterJ 51164:
	// Yes, well, "* 2" may be ok for Unicode to DBCS. but it's too small to
	// cope with conversion to UTF8. So bump it up to always be big enough.
	BufferSize *= 3;
#endif // PWD_JUPITER

	char *pBuffer = (char *)PvAlloc(BufferSize * 2, GMEM_ZEROINIT);
	if(!pBuffer)
	{
		return ecNoMemory;
	}

#ifdef DEBUG
	// When WCTMB fails to convert a char, the following default
	// char is used as a placeholder in the string being converted
	const char	chToDBCSDefault = 0;
	BOOL		fUsedDefault;

	pchToDBCSDefault = &chToDBCSDefault;
	pfUsedDefault	 = &fUsedDefault;
#endif

	int cchRet = WCTMB(fIsDBCS ? INVALID_CODEPAGE : nCodePage, 0, 
						szData, -1, pBuffer, BufferSize,
						pchToDBCSDefault, pfUsedDefault,
						&fMissingCodePage);
	Assert(cchRet > 0);

	if(!fIsDBCS && fMissingCodePage && nCodePage != CP_ACP)
	{
		// Here, the system could not convert the Unicode string because the
		// code page is not installed on the system.  Fallback to CP_ACP.

		cchRet = WCTMB(CP_ACP, 0, 
						szData, -1, pBuffer, BufferSize,
						pchToDBCSDefault, pfUsedDefault,
						&fMissingCodePage);
		Assert(cchRet > 0);

		nCodePage = CP_ACP;
	}

	AssertSz(!fUsedDefault, "CRTFWrite::WritePcData():  Found character in "
							"control text which cannot be converted from "
							"Unicode");
	if(cchRet <= 0)
	{
		_ecParseError = ecCantUnicode;
		goto CleanUp;
	}

	BufferSize = cchRet;

	fMultiByte = (BufferSize > DataSize) || fIsDBCS || fMissingCodePage;
	pch = (BYTE *)pBuffer;
	ch = *pch;
	
	// If _fNeedDelimeter, may need	to PutChar(' ')
	CheckDelimeter();
									
	while (!_ecParseError && (ch = *pch++))
	{
		if(fMultiByte && *pch && IsLeadByte(ch, nCodePage))
		{
			printF(szEscape2CharFmt, ch, *pch++);
		}
		else
		{
			if(ch == LBRACE || ch == RBRACE || ch == BSLASH)
			{
				printF(szLiteralCharFmt, ch);
			}
			else if(ch < 32 || ch == ';' || ch > 127)
			{
				printF(szEscapeCharFmt, ch);
			}
			else
			{
				PutChar(ch);
			}
		}
	}

CleanUp:
	FreePv(pBuffer); 
	return _ecParseError;
}

/*
 *	CRTFWrite::LookupColor(colorref)
 *
 *	@mfunc
 *		Return color-table index for color referred to by <p colorref>.
 *		If a match isn't found, an entry is added.
 *
 *	@rdesc
 *		LONG			Index into colortable
 *		<lt> 0			on error
 */
LONG CRTFWrite::LookupColor(
	COLORREF colorref)		// @parm colorref to look for
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::LookupColor");

	LONG		Count = _colors.Count();
	LONG		iclrf;
	COLORREF *	pclrf;

	for(iclrf = 0; iclrf < Count; iclrf++)		// Look for color
		if(_colors.GetAt(iclrf) == colorref)
		 	return iclrf;

	pclrf = _colors.Add(1, NULL);				// If we couldn't find it,
	if(!pclrf)									//  add it to color table
		return -1;
	*pclrf = colorref;

	return iclrf;
}

/*
 *	CRTFWrite::LookupFont(pCF)
 *
 *	@mfunc
 *		Returns index into font table for font referred to by
 *		CCharFormat *<p pCF>. If a match isn't found, an entry is added.
 *
 *	@rdesc
 *		SHORT		Index into fonttable
 *		<lt> 0		on error
 */
LONG CRTFWrite::LookupFont(
	CCharFormat const * pCF)	// @parm CCharFormat holding font name
{								//		 to look up
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::LookupFont");

	LONG		Count = _fonts.Count();
	LONG		itf;
	TEXTFONT *	ptf;
	
	for(itf = 0; itf < Count; itf++)
	{														// Look for font
		ptf = _fonts.Elem(itf);
		if (ptf->bPitchAndFamily == pCF->bPitchAndFamily &&	//  of same pitch,
			ptf->bCharSet		 == pCF->bCharSet &&		//  char set, and
			!wcscmp(ptf->szName, pCF->szFaceName))			//  name
		{
			return itf;										// Found it
		}
	}
	ptf = _fonts.Add(1, NULL);								// Didn't find it:
	if(!ptf)												//  add to table
		return -1;

	ptf->bPitchAndFamily = pCF->bPitchAndFamily;
	ptf->bCharSet		 = pCF->bCharSet;
	ptf->sCodePage		 = GetCodePage (ptf->bCharSet);
	wcscpy_s(ptf->szName, pCF->szFaceName);
	ptf->fNameIsDBCS = (pCF->bInternalEffects & CFEI_FACENAMEISDBCS);

#if 0
	// Bug1523 - (BradO) I removed this section of code so that a /fN tag is always
	// emitted for the first run of text.  In theory, we should be able to
	// assume that the first run of text would carry the default font.
	// It turns out that when reading RTF, Word doesn't use anything predictable
	// for the font of the first run of text in the absence of an explicit /fN, 
	// thus, we have to explicitly emit a /fN tag for the first run of text.
	if(!Count)												// 0th font is
	{														//  default \deff0
		_CF.bPitchAndFamily	= pCF->bPitchAndFamily;			// Set "previous"
		_CF.bCharSet		= pCF->bCharSet;				//  CF accordingly
		wcscpy(_CF.szFaceName, pCF->szFaceName);
	}
#endif

	return itf;
}

/*
 *	CRTFWrite::BuildTables(prp, cch)
 *
 *	@mfunc
 *		Build font and color tables for write range of length <p cch> and
 *		charformat run ptr <p prp>
 *
 *	@rdesc
 *		EC			The error code
 */
EC CRTFWrite::BuildTables(
	CFormatRunPtr& rpCF,	// @parm CF run ptr for start of write range
	CFormatRunPtr& rpPF,	// @parm PF run ptr for start of write range
	LONG cch)				// @parm # chars in write range
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::BuildTables");

	LONG				i;
	LONG				ifmt = 0;
	const CCharFormat *	pCF = NULL;
	const CParaFormat * pPF = NULL;
	CFormatRunPtr		rp(rpCF);
	CFormatRunPtr		rpPFtemp(rpPF);
	LONG				cchTotal = cch;

	while(cch > 0)
	{
		ifmt = rp.GetFormat();					// _iFormat for next CF run
		pCF = _ped->GetCharFormat(ifmt);

		if( !pCF )
			goto CacheError;

		// Look up character-format *pCF's font and color. If either isn't
		// found, it is added to appropriate table.  Don't lookup color
		// for CCharFormats with auto-color

		if (LookupFont(pCF) < 0 ||
			(!(pCF->dwEffects & CFE_AUTOCOLOR) &&
				LookupColor(pCF->crTextColor) < 0) ||
			(!(pCF->dwEffects & CFE_AUTOBACKCOLOR) &&
				LookupColor(pCF->crBackColor) < 0))
		{
			break;
		}
		if(!rp.IsValid())
			break;
		cch -= rp.GetCchLeft();
		rp.NextRun();
	}

	// now look for bullets; if found, then we need to include
	// the "Symbol" font

	cch = cchTotal;
	_symbolFont = 0;

	while( cch > 0 )
	{
		ifmt = rpPFtemp.GetFormat();
		pPF = _ped->GetParaFormat(ifmt);

		if( !pPF )
		{
			goto CacheError;
		}
		
		if( pPF->wNumbering == PFN_BULLET )
		{
			// Save the Font index for Symbol.
			// Reset it to 0 if LookupFont return error.
			if ( (_symbolFont = LookupFont((CCharFormat *)&cfBullet)) < 0 )
				_symbolFont = 0;

			// We don't need to bother looking for more bullets, since
			// in RichEdit 2.0, all bullets either have the same font or
			// have their formatting information in the character format
			// for the EOP mark.

            // GuyBark: That may be true, but we still need to keep looping 
            // through the paragraghs for things like the stylesheet below.
//            break;
		}
		
		WORD  Widths = pPF->wBorderWidth;
		DWORD Colors = pPF->dwBorderColor & 0xFFFFF;

		while(Widths && Colors)
		{
			i = Colors & 0x1F;
			if(i && (Widths & 0xF))
				LookupColor(g_Colors[i - 1]);

			Widths >>= 4;
			Colors >>= 5;
		}
		
		i = (pPF->wShadingStyle >> 6) & 31;			// Shading forecolor
		if(i)
			LookupColor(g_Colors[i - 1]);
		i = pPF->wShadingStyle >> 11;				// Shading backcolor
		if(i)
			LookupColor(g_Colors[i - 1]);

		if(IsHeadingStyle(pPF->sStyle) && pPF->sStyle < _nHeadingStyle)
			_nHeadingStyle = pPF->sStyle;

		if(!rpPFtemp.IsValid())
			break;
		
		cch -= rpPFtemp.GetCchLeft();
		rpPFtemp.NextRun();
	}	

	return _ecParseError;

CacheError:
	_ecParseError = ecFormatCache;
	return ecFormatCache;					// Access to CF/PF cache failed
}

/*
 *	CRTFWrite::WriteFontTable()
 *
 *	@mfunc
 *		Write out font table
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteFontTable()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteFontTable");

	LONG			Count = _fonts.Count();
	int				itf;
	int				m;
	int				pitch;
	TEXTFONT *ptf;
	char *			szFamily;
	TCHAR *			szTaggedName;

	if(!Count || !PutCtrlWord(CWF_GRP, i_fonttbl))	// Start font table group
		goto CleanUp;

	for (itf = 0; itf < Count; itf++)
	{
		ptf = _fonts.Elem(itf);

//		if (ptf->sCodePage)
//			if (! PutCtrlWord(CWF_VAL, i_cpg, ptf->sCodePage ) )
//				goto CleanUp;

		// Define font family
		m			 = ptf->bPitchAndFamily >> 4;
		szFamily	 = rgKeyword[rgiszFamily[m < 6 ? m : 0]].szKeyword;
		szTaggedName = NULL;

		// check to see if this is a tagged font
		if (!ptf->bCharSet ||
			!FindTaggedFont(ptf->szName, ptf->bCharSet, &szTaggedName))
		{
			szTaggedName = NULL;
		}

		pitch = ptf->bPitchAndFamily & 0xF;					// Write font
		if (!printF(szBeginFontEntryFmt, itf, szFamily))	//  entry, family,
			goto CleanUp;
		_fNeedDelimeter = TRUE;
		if (pitch && !PutCtrlWord(CWF_VAL, i_fprq, pitch))	//  and pitch
			goto CleanUp;

		if (!ptf->sCodePage && ptf->bCharSet)
		{
			ptf->sCodePage = GetCodePage(ptf->bCharSet);
		}

#ifdef PWD_JUPITER
        // GuyBark: If this is a J font, then store the id here so we can 
        // select it later if necessary when outputting J text chunks.
        if((_pwdDefaultJFont == -1) && (ptf->sCodePage == 932))
        {
            _pwdDefaultJFont = itf;
        }
#endif // PWD_JUPITER

		// Write charset. Win32 uses ANSI_CHARSET to mean the default Windows
		// character set, so find out what it really is

		extern BYTE bCharSetANSI;

		if(ptf->bCharSet != DEFAULT_CHARSET)
		{
			if(!PutCtrlWord(CWF_VAL, i_fcharset, ptf->bCharSet))
			{
				goto CleanUp;
			}

			// We want to skip the \cpgN output if we've already output a \fcharsetN
			// tag.  This is to accomodate RE1.0, which can't handle some \cpgN tags
			// properly.  Specifically, when RE1.0 parses the \cpgN tag it does a 
			// table lookup to obtain a charset value corresponding to the codepage.
			// Turns out the codepage/charset table for RE1.0 is incomplete and RE1.0
			// maps some codepages to charset 0, trouncing the previously read \fcharsetN
			// value.
			goto WroteCharSet;
		}

		if (ptf->sCodePage && !PutCtrlWord (CWF_VAL, i_cpg, ptf->sCodePage))
		{
			goto CleanUp;
		}

WroteCharSet:

#ifdef PWD_JUPITER

        // GuyBark:
        // Check if the font has a charset that means a font decoration should be
        // output with the name. Word 95 uses the decoration. Word 97 will just
        // ignore it.

        LPSTR pszDecoration = NULL;

        // No decoration needed for plain ol' Western ANSI fonts. Don't mess with 
        // the name if we're taking any special action with it either.

        if(ptf->bCharSet && !szTaggedName && !ptf->fNameIsDBCS)
        {
            int i = 0;

            // Word97 uses a decoration of "Turkish, rather than "Tur". So we'll 
            // use that too. We hit "Tur" first in the decoration array. 
            // Note: the Office converters check for "Turkish" so that may mean, 
            // but we're better off emulating Word 97.

            // For all the decorations we handle...
            while(fontDec[i].pszName)
            {
                // Is the charset of the font we're outputting the same as the decoration's?
                if(ptf->bCharSet == fontDec[i].charset)
                {
                    // Yes! So output the decoration after the name.
                    pszDecoration = fontDec[i].pszName;

                    break;
                }

                // Oh well, try the next decoration.
                ++i;
            }
        }
#endif // PWD_JUPITER

		if (szTaggedName)							
		{											
			// Have a tagged font:  write out group with real name followed by tagged name
			if(!PutCtrlWord(CWF_AST, i_fname) ||	
				WritePcData(ptf->szName, ptf->sCodePage, ptf->fNameIsDBCS) ||			
				!Puts(szEndFontEntry, sizeof(szEndFontEntry) - 1) ||
				WritePcData(szTaggedName, ptf->sCodePage, ptf->fNameIsDBCS) ||
				!Puts(szEndFontEntry, sizeof(szEndFontEntry) - 1))
			{
				goto CleanUp;
			}
		}
		else if(WritePcData(ptf->szName, ptf->sCodePage, ptf->fNameIsDBCS) ||
#ifdef PWD_JUPITER
               // GuyBark: We may have a decoration to output with the font name.
               (pszDecoration && !Puts(pszDecoration, strlen(pszDecoration))) ||
#endif // PWD_JUPITER
					!Puts(szEndFontEntry, sizeof(szEndFontEntry) - 1))
		// If non-tagged font just write name out
		{
			goto CleanUp;
		}
	}

	Puts(szEndGroupCRLF, sizeof(szEndGroupCRLF) - 1);							// End font table group

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteColorTable()
 *
 *	@mfunc
 *		Write out color table
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteColorTable()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteColorTable");

	LONG		Count = _colors.Count();
	COLORREF	clrf;
	LONG		iclrf;

    // GuyBark Jupiter 35396: Now that we allow autocolor, we must ALWAYS
    // output a color table, even if it only has the one ';' entry.
#ifdef PWD_JUPITER
	if (!PutCtrlWord(CWF_GRP, i_colortbl)	// Start color table group
#else
	if (!Count || !PutCtrlWord(CWF_GRP, i_colortbl)	// Start color table group
#endif // PWD_JUPITER
		|| !PutChar(';'))							//  with null first entry
	{
		goto CleanUp;
	}

	for(iclrf = 0; iclrf < Count; iclrf++)
	{
		clrf = _colors.GetAt(iclrf);
		if (!printF(szColorEntryFmt,
					GetRValue(clrf), GetGValue(clrf), GetBValue(clrf)))
			goto CleanUp;
	}

	Puts(szEndGroupCRLF,sizeof(szEndGroupCRLF) -1);		// End color table group

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteCharFormat(pCF)
 *
 *	@mfunc
 *		Write deltas between CCharFormat <p pCF> and the previous CCharFormat
 *		given by _CF, and then set _CF = *<p pCF>.
 *
 *	@rdesc
 *		EC			The error code
 *
 *	@devnote
 *		For optimal output, could write \\plain and use deltas relative to
 *		\\plain if this results in less output (typically only one change
 *		is made when CF changes, so less output results when compared to
 *		previous CF than when compared to \\plain).
 */
EC CRTFWrite::WriteCharFormat(
	const CCharFormat * pCF)		// @parm Ptr to CCharFormat
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteCharFormat");

	DWORD	dwEffects = pCF->dwEffects;
	DWORD	dwChanges = dwEffects ^ _CF.dwEffects;
	LONG	i;										// Counter
	LONG	iFormat;
	LONG	iValue;									// Control-word value
	LONG	i_sz;									// Temp ctrl string index
	DWORD	UType;									// Underline type
	LONG	yOffset = pCF->yOffset;

    // GuyBark Jupiter J
    // For JupiterJ I've added two new numbered list types. This mean 
    // I had to change all the i_xx array to Word arrays not bytes.
//	AssertSz(cKeywords < 256,
//		"CRTFWrite::WriteCharFormat: change BYTE i_xx to WORD");

	if (dwChanges & CFE_AUTOCOLOR ||				// Change in autocolor
		pCF->crTextColor != _CF.crTextColor)		//  or text color
	{
		iValue = 0;									// Default autocolor
		if(!(dwEffects & CFE_AUTOCOLOR))			// Make that text color
			iValue = LookupColor(pCF->crTextColor) + 1;
		if(!PutCtrlWord(CWF_VAL, i_cf, iValue))
			goto CleanUp;
	}

	if (dwChanges & CFE_AUTOBACKCOLOR ||			// Change in autobackcolor
		pCF->crBackColor != _CF.crBackColor)		//  or backcolor
	{
		iValue = 0;									// Default autobackcolor
		if(!(dwEffects & CFE_AUTOBACKCOLOR))		// Make that back color
			iValue = LookupColor(pCF->crBackColor) + 1;
		if(!PutCtrlWord(CWF_VAL, i_highlight, iValue))
			goto CleanUp;
	}

	if (pCF->lcid		!= _CF.lcid &&
		!PutCtrlWord(CWF_VAL, i_lang, LANGIDFROMLCID((WORD)pCF->lcid)) ||
		pCF->sSpacing	!= _CF.sSpacing &&
		!PutCtrlWord(CWF_VAL, i_expndtw, pCF->sSpacing)		||
		/* FUTURE (alexgo): This code is incorrect and we don't
		yet handle the Style table.  We may want to support this
		better in a future version.
		pCF->sStyle		!= _CF.sStyle && pCF->sStyle > 0    &&
		!PutCtrlWord(CWF_VAL, i_cs, pCF->sStyle)			|| */
		pCF->bAnimation	!= _CF.bAnimation &&
		!PutCtrlWord(CWF_VAL, i_animtext, pCF->bAnimation)	||
		/* FUTURE (alexgo): this code doesn't work yet, as we don't
		output the revision table.  We may want to support this 
		better in a future version
		pCF->bRevAuthor	!= _CF.bRevAuthor &&
		!PutCtrlWord(CWF_VAL, i_revauth, pCF->bRevAuthor)	|| */
		pCF->wKerning	!= _CF.wKerning &&
		!PutCtrlWord(CWF_VAL, i_kerning, pCF->wKerning/10) )
	{
		goto CleanUp;
	}

	UType = _CF.bUnderlineType;						// Handle all underline
	if (UType <= CUNDERLINES &&						//  known and
		dwEffects & CFM_UNDERLINE &&				//  active changes
		(UType != pCF->bUnderlineType ||			// Type change while on
		 dwChanges & CFM_UNDERLINE))				// Turn on
	{
		dwChanges &= ~CFE_UNDERLINE;				// Suppress underline
		i = pCF->bUnderlineType;
		if(i)
			i--;
		if(!PutCtrlWord(CWF_STR,
			rgiszUnderlines[i]))					// action in next for()
				goto CleanUp;						// Note: \ul0 turns off
	}												//  all underlining

													// This must be before next stuff
	if(dwChanges & (CFM_SUBSCRIPT | CFM_SUPERSCRIPT))//  change in sub/sup
	{												// status	
	 	i_sz = dwEffects & CFE_SUPERSCRIPT ? i_super
	    	 : dwEffects & CFE_SUBSCRIPT   ? i_sub
	       	 : i_nosupersub;
     	if(!PutCtrlWord(CWF_STR, i_sz))
			goto CleanUp;
	}

	dwChanges &= ((1 << CEFFECTS) - 1) & ~CFE_LINK;	// Output keywords for
	for(i = CEFFECTS;								//  effects that changed
		dwChanges && i--;							// rgszEffects[] contains
		dwChanges >>= 1, dwEffects >>= 1)			//  effect keywords in
	{												//  order max CFE_xx to
		if(dwChanges & 1)							//  min CFE-xx
		{											// Change from last call
			iValue = dwEffects & 1;					// If effect is off, write
			iFormat = iValue ? CWF_STR : CWF_VAL;	//  a 0; else no value
			if(!PutCtrlWord(iFormat,
				rgiszEffects[i], iValue))
					goto CleanUp;
		}
	}

	if(yOffset != _CF.yOffset)						// Change in base line 
	{												// position 
		yOffset /= 10;								// Default going to up
		i_sz = i_up;
		iFormat = CWF_VAL;
		if(yOffset < 0)								// Make that down
		{
			i_sz = i_dn;
			yOffset = -yOffset;
		}
		if(!PutCtrlWord(iFormat, i_sz, yOffset))
			goto CleanUp;
	}

	if (pCF->bPitchAndFamily != _CF.bPitchAndFamily ||	// Change in font
		pCF->bCharSet		 != _CF.bCharSet		||
		lstrcmp(pCF->szFaceName, _CF.szFaceName))
	{
		iValue = LookupFont(pCF);
		if(iValue < 0 || !PutCtrlWord(CWF_VAL, i_f, iValue))
			goto CleanUp;

#ifdef PWD_JUPITER
        // GuyBark: Keep a track of the current font selected.
        _pwdCurrentFont = iValue;
#endif // PWD_JUPITER
    }

	if (pCF->yHeight != _CF.yHeight)					// Change in font size
	{
		iValue = (pCF->yHeight + (pCF->yHeight > 0 ? 5 : -5))/10;
		if(!PutCtrlWord(CWF_VAL, i_fs, iValue))
			goto CleanUp;
	}

	_CF = *pCF;									// Update previous CCharFormat

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteParaFormat(prtp)
 *
 *	@mfunc
 *		Write out attributes specified by the CParaFormat <p pPF> relative
 *		to para defaults (probably produces smaller output than relative to
 *		previous para format and let's you redefine tabs -- no RTF kill
 *		tab command	except \\pard)
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteParaFormat(
	const CRchTxtPtr * prtp)	// @parm Ptr to rich-text ptr at current cp
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteParaFormat");

	Assert(_ped);

	//if(!_fRangeHasEOP)							// Don't write para info if
	//	return _ecParseError;					//  range has no EOPs

	const CParaFormat * pPFPrev = _pPF;
	const CParaFormat * pPF = _pPF = prtp->GetPF();

    if (NULL == pPF)
        return _ecParseError;
        
	BOOL	fInTable = pPF->InTable();
	LONG	c;					  				// Temporary count
	LONG	cTab = pPF->cTabCount;
	DWORD	dwEffects;
	DWORD	dwRule	= pPF->bLineSpacingRule;
	LONG	dy		= pPF->dyLineSpacing;
	LONG	i_t, i, j, k;
	LONG	tabAlign, tabLead, tabPos;
	LONG	lDocDefaultTab = _ped->GetDefaultTab();

	if(!lDocDefaultTab)
		lDocDefaultTab = lDefaultTab;

	if (cTab == 1 && pPF->rgxTabs[0] == lDocDefaultTab + (LONG)PFT_DEFAULT ||
		CheckInTable(FALSE))
	{
		cTab = 0;								// Suppress \tab output
	}

	AssertSz(cTab >= 0 && cTab <= MAX_TAB_STOPS,
		"CRTFW::WriteParaFormat: illegal cTabCount");

	// Exchange's IMC keys on the \protect tag when it does
	//	its reply-ticking for mail being sent to Internet recipients.  
	//	Paragraphs following a \pard and containing a \protect tag are 
	//	reply-ticked, so we must ensure that each \pard in a protected range
	//	is followed by a \protect tag.

	if (_CF.dwEffects & CFE_PROTECTED && !PutCtrlWord(CWF_VAL, i_protect, 0) ||
		!PutCtrlWord(CWF_STR, i_pard) ||			// Reset para attributes
		_CF.dwEffects & CFE_PROTECTED && !PutCtrlWord(CWF_STR, i_protect))
	{
		goto CleanUp;
	}

	if(fInTable)
	{
		if(_fRangeHasEOP && !PutCtrlWord(CWF_STR, i_intbl))
			goto CleanUp;
	}
	else if(PutBorders(FALSE))
		goto CleanUp;

	if(pPF->wShadingStyle)
	{
		i = pPF->wShadingStyle & 15;				// Shading patterns
		j = (pPF->wShadingStyle >> 6) & 31;			// Shading forecolor
		k = pPF->wShadingStyle >> 11;				// Shading backcolor
		if (i && i <= CSHADINGSTYLES &&
			!PutCtrlWord(CWF_STR, rgiszShadingStyles[i - 1]) ||
			j && !PutCtrlWord(CWF_VAL, i_cfpat, LookupColor(g_Colors[j-1]) + 1) ||
			k && !PutCtrlWord(CWF_VAL, i_cbpat, LookupColor(g_Colors[k-1]) + 1))
		{
			goto CleanUp;
		}
	}
	if(pPF->wShadingWeight && !PutCtrlWord(CWF_VAL, i_shading, pPF->wShadingWeight))
		goto CleanUp;

	// Paragraph numbering
	_fBullet = _fBulletPending = FALSE;
	_nNumber = pPF->UpdateNumber(_nNumber, pPFPrev);

	if(pPF->wNumbering)							// Write numbering info
	{
		LONG iFont = _symbolFont;
		if(pPF->IsListNumbered())
		{
			const CCharFormat *pCF;
			WCHAR szNumber[CCHMAXNUMTOSTR];

			CTxtPtr		  rpTX(prtp->_rpTX);
			CFormatRunPtr rpCF(prtp->_rpCF);

			rpCF.AdvanceCp(rpTX.FindEOP(tomForward));
			rpCF.AdjustBackward();
			pCF = _ped->GetCharFormat(rpCF.GetFormat());
			iFont = LookupFont(pCF);
			if(iFont < 0)
			{
				iFont = 0;
				TRACEERRORSZ("CWRTFW::WriteParaFormat: illegal bullet font");
			}
			_nFont = iFont;
			// TODO: make the following smarter, i.e., may need to increment
			// _nNumber instead of resetting it to 1.
			_cpg = GetCodePage(pCF->bCharSet);

			i = 0;

            // GuyBark JupiterJ: Assume we want everything before sequence value
//			if(pPF->wNumbering <= tomListNumberAsUCRoman)
            if(pPF->wNumbering < tomListNumberAsSequence)
				i = pPF->wNumbering - tomListNumberAsArabic;

			WORD  wStyle = pPF->wNumberingStyle & 0xF00;

			WCHAR ch = (wStyle == PFNS_PARENS || wStyle == PFNS_PAREN) ? ')'
					 : (wStyle == PFNS_PERIOD) ? '.' : 0;
			if(wStyle != PFNS_NONUMBER)			  // Unless number suppressed
			{									  //  write \pntext group
				pPF->NumToStr(szNumber, _nNumber);
				if (!printF(szBeginNumberGroup, iFont) ||
					WritePcData(szNumber, _cpg, FALSE) ||	
					!printF(szEndNumberGroup))
				{
					goto CleanUp;
				}
			}

			if (!printF(szBeginNumberFmt,
						wStyle == PFNS_NONUMBER ? "cont" : "body",
						iFont, pPF->wNumberingTab,
						pPF->wNumberingStart)				||
				!PutCtrlWord(CWF_STR, rgiszNumberStyle[i])	||
				wStyle == PFNS_PARENS && !printF(szpntxtb)	||
				ch && !printF(szpntxta, ch)					||
				!printF(szEndGroupCRLF))
			{
				goto CleanUp;
			}
		}
		else
		{
			if (!printF(szBulletGroup, iFont) ||
				!printF(szBulletFmt,   iFont, pPF->wNumberingTab))
			{
				goto CleanUp;
			}
		}
		_fBullet = TRUE;
	}

	// Put out para indents. RTF first indent = -PF.dxOffset
	// RTF left indent = PF.dxStartIndent + PF.dxOffset

	if(IsHeadingStyle(pPF->sStyle) && !PutCtrlWord(CWF_VAL, i_s, -pPF->sStyle-1))
		goto CleanUp;
		
	if(!fInTable && (pPF->dxOffset &&
		!PutCtrlWord(CWF_VAL, i_fi, -pPF->dxOffset)	||
		pPF->dxStartIndent + pPF->dxOffset &&
		!PutCtrlWord(CWF_VAL, i_li, pPF->dxStartIndent + pPF->dxOffset) ||
		pPF->dxRightIndent	  &&
		!PutCtrlWord(CWF_VAL, i_ri, pPF->dxRightIndent)))
	{
		goto CleanUp;
	}
	if (pPF->dySpaceBefore	  &&
		!PutCtrlWord(CWF_VAL, i_sb, pPF->dySpaceBefore) ||
		pPF->dySpaceAfter	  &&
		!PutCtrlWord(CWF_VAL, i_sa, pPF->dySpaceAfter))
	{
		goto CleanUp;
	}

	if (dwRule)									// Special line spacing active
	{
		i = 0;									// Default "At Least" or
		if (dwRule == tomLineSpaceExactly)		//  "Exactly" line spacing
			dy = -abs(dy);						// Use negative for "Exactly"

		else if(dwRule == tomLineSpaceMultiple)	// RichEdit uses 20 units/line
		{										// RTF uses 240 units/line
			i++;
			dy *= 12;							
		}

		else if (dwRule != tomLineSpaceAtLeast && dy > 0)
		{
			i++;								// Multiple line spacing
			if (dwRule <= tomLineSpaceDouble)	// 240 units per line
				dy = 120 * (dwRule + 2);
		}
		if (!PutCtrlWord(CWF_VAL, i_sl, dy) ||
			!PutCtrlWord(CWF_VAL, i_slmult, i))
		{
			goto CleanUp;
		}
	}

	dwEffects = pPF->wEffects & ((1 << CPFEFFECTS) - 1);
	for(c = CPFEFFECTS; dwEffects && c--;		// Output PARAFORMAT2 effects
		dwEffects >>= 1)	
	{
		// rgiszPFEffects[] contains PF effect keywords in the
		//  order max PFE_xx to min PFE-xx

		AssertSz(rgiszPFEffects[2] == i_hyphpar,
			"CRTFWrite::WriteParaFormat(): rgiszPFEffects is out-of-sync with PFE_XXX");
		// \hyphpar has opposite logic to our PFE_DONOTHYPHEN so we emit
		// \hyphpar0 to toggle the property off

		if (dwEffects & 1 &&
			!PutCtrlWord((c == 2) ? CWF_VAL : CWF_STR, rgiszPFEffects[c], 0))
		{
			goto CleanUp;
		}				
	}
	
	if (!fInTable && IN_RANGE(PFA_RIGHT, pPF->wAlignment, PFA_JUSTIFY) &&
		!PutCtrlWord(CWF_STR, rgiszAlignment[pPF->wAlignment - 1]))
	{
		goto CleanUp;
	}

	for (i = 0; i < cTab; i++)
	{
		pPF->GetTab(i, &tabPos, &tabAlign, &tabLead);
		AssertSz (tabAlign <= tomAlignBar && tabLead <= 5,
			"CRTFWrite::WriteParaFormat: illegal tab leader/alignment");

		i_t = i_tb;								// Default \tb (bar tab)
		if (tabAlign != tomAlignBar)			// It isn't a bar tab
		{
			i_t = i_tx;							// Use \tx for tabPos
			if (tabAlign &&						// Put nonleft alignment
				!PutCtrlWord(CWF_STR, rgiszTabAlign[tabAlign-1]))
			{
				goto CleanUp;
			}
		}
		if (tabLead &&							// Put nonzero tab leader
			!PutCtrlWord(CWF_STR, rgiszTabLead[tabLead-1]) ||
			!PutCtrlWord(CWF_VAL, i_t, tabPos))
		{
			goto CleanUp;
		}
	}

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteText(cwch, lpcwstr, nCodePage, fIsDBCS)
 *
 *	@mfunc
 *		Write out <p cwch> chars from the Unicode text string <p lpcwstr> taking care to
 *		escape any special chars.  The Unicode text string is scanned for characters which
 *		map directly to RTF strings, and the surrounding chunks of Unicode are written
 *		by calling WriteTextChunk.
 *
 *	@rdesc
 *		EC	The error code
 */
EC CRTFWrite::WriteText(
	LONG		cwch,					// @parm # chars in buffer
	LPCWSTR 	lpcwstr,				// @parm Pointer to text
	INT			nCodePage,				// @parm code page to use to convert to DBCS
	BOOL		fIsDBCS)				// @parm indicates whether lpcwstr is a Unicode string
										//		or a DBCS string stuffed into a WSTR
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteText");

	WCHAR *pwchScan;
	WCHAR *pwchStart;

	if (_fBulletPending)
	{
		_fBulletPending = FALSE;
		if(!_nNumber)
		{
			if(!printF(szBulletGroup, _symbolFont))
				goto CleanUp;
		}
		else if(!_pPF->IsNumberSuppressed())
		{
			WCHAR szNumber[CCHMAXNUMTOSTR];
			_pPF->NumToStr(szNumber, ++_nNumber);
			if (!printF(szBeginNumberGroup, _nFont) ||
				WritePcData(szNumber, _cpg, FALSE)	||
				!printF(szEndNumberGroup))
			{
				goto CleanUp;
			}
		}
	}
	if(_fCheckInTable)
	{
		CheckInTable(TRUE);
		if(_ecParseError)
			goto CleanUp;
	}

	pwchScan = const_cast<LPWSTR>(lpcwstr);
	pwchStart = pwchScan;
	if(_CF.bCharSet == SYMBOL_CHARSET)
	{
		pwchScan += cwch;
		cwch = 0;
	}

	// Step through the Unicode buffer, weeding out characters that have  
	// known translations to RTF strings
	while(cwch-- > 0)
	{
		WCHAR	wch = *pwchScan;

		// If this is a string for which the MultiByteToUnicode conversion
		// failed, the buffer will be filled with ANSI bytes stuffed into
		// wchar's (one per).  In this case, we don't want to map trail bytes
		// to RTF strings.
		if(fIsDBCS && IsLeadByte(wch, nCodePage))
		{
#ifdef PWD_JUPITER
		   // GuyBark JupiterJ:
		   // If there is no trailing byte then that wasn't really a
		   // lead byte. It was probably an extended ansi character 
		   // which incorrectly had a FE font applied to it.
		   if(cwch > 0)
		   {
#endif // PWD_JUPITER
			    Assert(cwch);
			    cwch--;
			    pwchScan += 2;
			    continue;
#ifdef PWD_JUPITER
		   }
#endif // PWD_JUPITER
		}

		// if the char is one for which there is an appropriate RTF string
		// write the preceding chars and output the RTF string

		if(!IN_RANGE(' ', wch, 'Z') &&
		   !IN_RANGE('a', wch, 'z') &&
		   !IN_RANGE(chOptionalHyphen + 1, wch, ENSPACE - 1) &&
#ifdef PWD_JUPITER
        // GuyBark Jupiter: Handle the special character for CRLF in table cells.
		   (wch <= BULLET || wch == PWD_CRLFINCELL) &&
#else
		   wch <= BULLET &&
#endif // PWD_JUPITER
		   MapsToRTFKeywordW(wch))
		{
			if (pwchScan != pwchStart &&
				WriteTextChunk(pwchScan - pwchStart, pwchStart, nCodePage, 
									fIsDBCS))
			{
				goto CleanUp;
			}

			// map the char(s) to the RTF string
			int cwchUsed = MapToRTFKeyword(pwchScan, cwch, MAPTOKWD_UNICODE);

			cwch -= cwchUsed;
			pwchScan += cwchUsed;

			// start of next run of unprocessed chars is one past current char
			pwchStart = pwchScan + 1;

#ifdef TARGET_NT // V-GUYB: Don;t change device code at this late stage.
    		if(cwch)
            {
	    		_fCheckInTable = FALSE;
            }
#endif // TARGET_NT

		}
		pwchScan++;
	}

	// write the last chunk
	if (pwchScan != pwchStart &&
		WriteTextChunk(pwchScan - pwchStart, pwchStart, nCodePage, fIsDBCS))
	{
		goto CleanUp;
	}

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteTextChunk(cwch, lpcwstr, nCodePage, fIsDBCS)
 *
 *	@mfunc
 *		Write out <p cwch> chars from the Unicode text string <p lpcwstr> taking care to
 *		escape any special chars.  Unicode chars which cannot be converted to
 *		DBCS chars using the supplied codepage, <p nCodePage>, are written using the
 *		\u RTF tag.
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteTextChunk(
	LONG		cwch,					// @parm # chars in buffer
	LPCWSTR 	lpcwstr,				// @parm Pointer to text
	INT			nCodePage,				// @parm code page to use to convert to DBCS
	BOOL		fIsDBCS)				// @parm indicates whether lpcwstr is a Unicode string
										//		or a DBCS string stuffed into a WSTR
{
	// FUTURE(BradO):  There is alot of commonality b/t this routine and
	//	WritePcData.  We should re-examine these routines and consider 
	//	combining them into a common routine.

	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteTextChunk");

	BYTE 	b;
	LONG	cbAnsi;
	LONG	cbAnsiBufferSize;
	BYTE *	pbAnsiBuffer;
	BYTE *	pbAnsi;
	BOOL 	fUsedDefault = FALSE;
	BOOL	fMultiByte;
	BOOL 	fMissingCodePage = FALSE;

#ifdef PWD_JUPITER
    // GuyBark: If we're going to output J text below, then we must make
    // sure a J font is current for the text chunk. RichEdit and Word97 
    // don;t care, as they look at the unicode, but Word95 relies on the
    // MBCS equivalent of the unicode. While Word95 and Word97 always have
    // a J font selected for the run, the Word converters may not.
    BOOL    bSetJFont = FALSE;
#endif // PWD_JUPITER

	// When WideCharToMultiByte fails to convert a char, the following default
	// char is used as a placeholder in the string being converted

	// GuyBark JupiterJ 50783:
	// Originally we were passing a default character os '\0' through to 
	// WideCharToMultiByte(). But the OS code says, if the codepage is FE,
	// then the MBCS we may end up getting is either one or two bytes
	// depending on what the second byte if the default character is. But
	// we shouldn't have to care what the second byte is. If the first 
	// byte is zero (ie the low byte) then we can't have supplied a DBCS
	// character. Anyway, pass in two zero bytes here.
	const char szDBCSDefault[2] = {0, 0};

	// Allocate temp buffer for ANSI text we convert to
	cbAnsiBufferSize = cachBufferMost * (nCodePage == CP_UTF8 ? 3 : MB_LEN_MAX);
	if (!_pbAnsiBuffer)
	{
		// If the code page was CP_UTF8, it will always be CP_UTF8 for this instance
		_pbAnsiBuffer = (BYTE *)PvAlloc(cbAnsiBufferSize, GMEM_FIXED);
		if (!_pbAnsiBuffer)
			goto RAMError;
	}
	pbAnsiBuffer = _pbAnsiBuffer;

	// Convert Unicode (or fIsDBCS) buffer to ANSI 
	if(fIsDBCS)
	{
		// Supply some bogus code page which will force direct conversion
		// from wchar to bytes (losing high byte of wchar).
		// Also, don't want to use default char replacement in this case.
		cbAnsi = WCTMB(INVALID_CODEPAGE, 0, lpcwstr, cwch, 
						(char *)pbAnsiBuffer, cbAnsiBufferSize,
						NULL, NULL, NULL);
	}
	else
	{
		cbAnsi = WCTMB(nCodePage, 0, lpcwstr, cwch, 
						(char *)pbAnsiBuffer, cbAnsiBufferSize,
						szDBCSDefault, &fUsedDefault,
						&fMissingCodePage);
	}
	Assert(cbAnsi > 0);

	pbAnsi = pbAnsiBuffer;
	fMultiByte = (cbAnsi > cwch) || fIsDBCS || fMissingCodePage;

	while (!_ecParseError && cbAnsi-- > 0)
	{
		b = *pbAnsi;

		// Compare ASCII chars to their Unicode counterparts to check
		// that we're in sync
		AssertSz(cwch <= 0 || *lpcwstr > 127 || b == *lpcwstr, 
			"CRTFWrite::WriteText: Unicode and DBCS strings out of sync");

		// FUTURE(BradO):  MurrayS made a clever change to this code which
		//	caused the RTF writer to output the \uN tag for all ASCII characters
		//	greater than 0x7F.  This change was necessitated by the behaviour
		// 	of WCTMB whereby Unicode chars which should fail the conversion to ANSI
		//	are converted to some "best-match" for the codepage
		//	(ex.  alpha's convert to 'a' with cpg==1252).
		//
		//	This change was pulled out at the request of Outlook, but should
		// 	be considered for RE2.1.  This change can be found in version 75 of
		//	this file.  Note: NT 5.0 plans to introduce the flag
		//  WC_NO_BEST_FIT_CHARS, which should make our current algorithm output
		//  \uN values whenever the system cannot convert a character correctly.

		if (!IN_RANGE(' ', b, 'z') && !IN_RANGE('A', b, 'Z') &&
			MapsToRTFKeywordA(b))
		{
			int cchUsed = MapToRTFKeyword(pbAnsi, cbAnsi, MAPTOKWD_ANSI);
			cbAnsi -= cchUsed;
			pbAnsi += cchUsed;
		}
		else if(nCodePage == CP_UTF8)
		{
			PutChar(b);								// Output 1st byte in any
			if(b >= 0xC0)							//  case. At least 2-byte
			{										// At least 2-byte lead
				pbAnsi++;							//  byte, so output a
				Assert(cbAnsi && IN_RANGE(0x80, *pbAnsi, 0xBF));
				cbAnsi--;							//  trail byte
				PutChar(*pbAnsi);
				if(b >= 0xE0)						// 3-byte lead byte, so
				{									//  output another trail
					pbAnsi++;						//  byte
					Assert(cbAnsi && IN_RANGE(0x80, *pbAnsi, 0xBF));
					cbAnsi--;
					PutChar(*pbAnsi);
				}
			}
		}
		else if(fMultiByte && cbAnsi && IsLeadByte(b, nCodePage))
		{
#ifdef PWD_JUPITER
            // GuyBark: If we have the UNICODE value for the character, output it here.
            // Then follow it by the MBCS value. This allows the output RTF file to
            // be more portable later, if it's read on a system that doesn't have the
            // required code page for the MBCS.
            if(!fIsDBCS)
            {
                // If this is a lead byte, then we will try to output a DBCS value.
                // This means the UNICODE will ALWAYS be followed by 2 MBCS values.
                int cb = 2;

                // Is the current \ucN value good to use?
                if(cb != _UnicodeBytesPerChar)
                {
                    // No, so set up \uc2
            	    if(!PutCtrlWord(CWF_VAL, i_uc, cb))
                    {
                        goto CleanUp;
                    }

                    _UnicodeBytesPerChar = cb;
                }

                // Now output the unicode value.
                if(!PutCtrlWord(CWF_VAL, i_u, (int)*lpcwstr))
                {
                    goto CleanUp;
                }
            }

            // If the code page is missing, then the pbAnsi buffer was artifically
            // created to hold one byte per character. In which case we never want
            // to jump two bytes per character.
            if(!fMissingCodePage)
            {
#endif // PWD_JUPITER
		        pbAnsi++;								// Output DBCS pair
			    cbAnsi--;
#ifdef PWD_JUPITER
            }
#endif // PWD_JUPITER

			if(fIsDBCS)
			{
				lpcwstr++;
				cwch--;
			}
			printF(szEscape2CharFmt, b, *pbAnsi);
		}
		else 
		{
            // GuyBark 3/10/98 17827:
            // If the character being output is not a character from the lower character set
            // of an ansi font, then output it with the matching unicode token. Otherwise we 
            // have problems preserving characters from non-existing fonts. The
            // following test means that we will write out the unicode more often than we 
            // need too, but that just means the output rtf is pretty darn portable.

#ifdef PWD_JUPITER
            if(*lpcwstr != (WCHAR)b)
            {
                // The unicode char value is not the same as the equivalent ansi value.

                // GuyBark: 3/27/97 This is what we'd do if we're were happy always writing
                // the non-unicode representation as one byte, (ie with the \uc1 token).
                // But some unicode values we output here can only be represented as double
                // byte, so we will need to \uc2 token in this case.
/*
                if(!PutCtrlWord(CWF_VAL, i_u, ((cwch > 0) ? (int)*lpcwstr : TEXT('?'))) ||
                    !printF(szEscapeCharFmt, b))
                {
                    goto CleanUp;
                }
*/
                int  cb, nCodePageUse = nCodePage;
                BYTE sza[2] = {0};

                // If we're outputting J text here, we must make sure a J font is selected.

                // IMPORTANT: This check was originally added to make sure J text is output 
                // with a font with the J code page. This worked around a problem with the
                // Office converters, whereby J text may still the 1252 code page selected.
                // This workaround is never needed here now, as we trap this problem during
                // the stream in process instead. Keep this code here anyway in case the 
                // stream in failed to recognize the problem.

                if((_pwdDefaultJFont != -1) &&  // We have a J font to use if necessary.
                   !bSetJFont &&                // We haven't yet selected a J font ourselves.
                   (nCodePage == 1252))         // The current font for this chunk is set to be ansi.
                {
                    // Is the character we're outputting part of the FE unicode range?
                	if(IN_RANGE(0x3000, *lpcwstr, 0x33FF) ||   // "CJK Symbols and Punctuation", "Hiragana", "Katakana", "Bopomofo", "Hangul Compatibility Jamo", "Kanbun", "Enclosed CJK Letters and Months", "CJK Compatibility"
                	   IN_RANGE(0x4E00, *lpcwstr, 0x9FFF) ||   // "CJK Unified Ideographs"
                	   IN_RANGE(0xF900, *lpcwstr, 0xFAFF) ||   // "CJK Compatibility Ideographs"
                	   IN_RANGE(0xFE30, *lpcwstr, 0xFE4F) ||   // "CJK Compatibility Forms"
                	   IN_RANGE(0xFF00, *lpcwstr, 0xFFEF))     // "Halfwidth and Fullwidth Forms"
                    {
                        // Yes! So we will explicitly set the current font to be J in the output file.
                        bSetJFont = TRUE;                    

                        // Assume that this entire chunk for have the J font. We will restore
                        // the original font after processing this text chunk.
                        if(!PutCtrlWord(CWF_VAL, i_f, _pwdDefaultJFont))
                        {
                            goto CleanUp;
                        }

                        // Use the J code page (not ansi) for finding the MBCS char below.
                        nCodePageUse = 932;
                    }
                }

                // Map the unicode value to MBCS.
                if(!(cb = WCTMB(nCodePageUse, 0, lpcwstr, 1, (char*)sza, 2, NULL, NULL, NULL)))
                {
                    // No map possible. So output a single escaped question mark.
                    sza[0] = '?';
                    cb = 1;
                }

                // Output the count of bytes that comprise the MBCS character
                // if it's not the current value.
                if(cb != _UnicodeBytesPerChar)
                {
            	    if(!PutCtrlWord(CWF_VAL, i_uc, cb))
                    {
                        goto CleanUp;
                    }

                    _UnicodeBytesPerChar = cb;
                }

                // Now output the unicode token first.
                if(!PutCtrlWord(CWF_VAL, i_u, ((cwch > 0) ? (int)*lpcwstr : TEXT('?'))))
                {
                    goto CleanUp;
                }

                // Now output the non-unicode equivalent. 
                if(!printF(szEscapeCharFmt, sza[0]))
                {
                    goto CleanUp;
                }

                // If this character has a two byte non-unicode representation, 
                // then output the second byte, also adjust the cbAnsi and pbAnsi
                if(cb > 1) 
                {
                    cbAnsi--;
                    pbAnsi++;
                    if (!printF(szEscapeCharFmt, sza[1]))
                    {
                        goto CleanUp;
                    }
                }
            }
            else // *** DROP THROUGH TO EXISTING RICHEDIT CODE. *** 
#endif // PWD_JUPITER

            if(b == szDBCSDefault[0] && fUsedDefault)
			{
				// Here, the WideCharToMultiByte couldn't complete a conversion
				// so the routine used as a placeholder the default char we provided.
				// In this case we want to output the original Unicode character.

				if(!PutCtrlWord(CWF_VAL, i_u, 
								((cwch > 0) ? (int)*lpcwstr : TEXT('?'))) ||
#ifndef PWD_JUPITER
					!printF(szEscapeCharFmt, '?'))
#else
                    // We may currently be outputting 2 MBCS bytes per UNICODE character.
					!printF(szEscapeCharFmt, '?') ||
                    (_UnicodeBytesPerChar == 2 && !printF(szEscapeCharFmt, '?')))
#endif // PWD_JUPITER
 				{
					goto CleanUp;
				}
			}
			else if(!IN_RANGE(32, b, 127))
				printF(szEscapeCharFmt, b);
      else if(b == '\\')
        printF(szLiteralCharFmt, b);
			else
				PutChar(b);
 		}
		pbAnsi++;
		lpcwstr++;
		cwch--;
	}
	goto CleanUp;

RAMError:
	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;

CleanUp:

#ifdef PWD_JUPITER
    // GuyBark: If we selected the J font above, restore the previous font now.
    if(bSetJFont &&
       !PutCtrlWord(CWF_VAL, i_f, _pwdCurrentFont))
    {
        goto CleanUp;
    }
#endif // PWD_JUPITER

	return _ecParseError;
}

/*
 *	CRTFWrite::WriteInfo()
 *
 *	@mfunc
 *		Write out Far East specific data.
 *
 *	@rdesc
 *		EC				The error code
 */
EC CRTFWrite::WriteInfo()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteInfo");

	// TODO(BradO):  Ultimately it would be nice to set some kind of
	//	fRTFFE bit to determine whether to write \info stuff.  For now,
	//	we rely on the fact that lchars and fchars info actually exists
	//	to determine whether to write out the \info group.

#ifdef UNDER_WORK
	if (!(_dwFlags & fRTFFE)	||					// Start doc area
		!PutCtrlWord(CWF_GRP, i_info)	||
		!printF("{\\horzdoc}"))
			goto CleanUp;

	// Write out punctuation character info

	CHAR	sz[PUNCT_MAX];
	if(UsVGetPunct(_ped->lpPunctObj, PC_FOLLOWING, sz, sizeof(sz))
					> PUNCT_MAX - 2)
		goto CleanUp;

	if(!Puts("{\\*\\fchars") || WritePcData(sz) || !PutChar(chEndGroup))
		goto CleanUp;
	
	if(UsVGetPunct(ped->lpPunctObj, PC_LEADING, sz, sizeof(sz)) > PUNCT_MAX+2)
		goto CleanUp;

	if(!Puts("{\\*\\lchars") || WritePcData(sz) || !PutChar(chEndGroup))
		goto CleanUp;

	Puts(szEndGroupCRLF);							// End info group

#endif

	LPTSTR lpstrLeading = NULL;
	LPTSTR lpstrFollowing = NULL;

	// if either succeeds (but evaluate both)
	if(((_ped->GetLeadingPunct(&lpstrLeading) == NOERROR) +
		(_ped->GetFollowingPunct(&lpstrFollowing) == NOERROR)) &&
		(lpstrLeading || lpstrFollowing))
	{
		if (!PutCtrlWord(CWF_GRP, i_info) ||
			!Puts(szHorzdocGroup,sizeof(szHorzdocGroup) - 1))
		{
			goto CleanUp;
		}

		if(lpstrLeading)
		{
			if(!PutCtrlWord(CWF_AST, i_lchars) || 
				WritePcData(lpstrLeading, INVALID_CODEPAGE, TRUE) ||
				!PutChar(chEndGroup))
			{
				goto CleanUp;
			}
		}

		if(lpstrFollowing)
		{
			if(!PutCtrlWord(CWF_AST, i_fchars) || 
				WritePcData(lpstrFollowing, INVALID_CODEPAGE, TRUE) ||
				!PutChar(chEndGroup))
			{
				goto CleanUp;
			}
		}

		Puts(szEndGroupCRLF,sizeof(szEndGroupCRLF) - 1);			// End info group
	}

CleanUp:
	return _ecParseError;
}

/*
 *	CRTFWrite::WriteRtf()
 *
 *	@mfunc
 *		Write range _prg to output stream _pes.
 *
 *	@rdesc
 *		LONG	Number of chars inserted into text; 0 means none were
 *				inserted, OR an error occurred.
 */
LONG CRTFWrite::WriteRtf()
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteRtf");

	LONG			cch, cchBuffer;
	LONG			cchCF, cchPF;
	LONG			cchT;
	LONG			cpMin, cpMost;
	BOOL 			fOutputEndGroup;
	LONG			i, j;
	LONG			lDocDefaultTab;
	TCHAR *			pch;
	TCHAR *			pchBuffer;
	CTxtEdit *		ped = _ped;
	CDocInfo *		pDocInfo = ped->GetDocInfo();
	CRchTxtPtr		rtp(*_prg);
	WORD			wCodePage = CP_ACP;
	CTxtPtr			tp(rtp._rpTX);

	AssertSz(_prg && _pes, "CRTFW::WriteRtf: improper initialization");
	// Allocate buffers for text we pick up and for RTF output
	pchBuffer = (TCHAR *) PvAlloc(cachBufferMost * (sizeof(TCHAR) + 1) + 1,
								 GMEM_FIXED); // Final 1 is for debug
	if(!pchBuffer)
	{
		fOutputEndGroup = FALSE;
		goto RAMError;
	}
	_pchRTFBuffer = (CHAR *)(pchBuffer + cachBufferMost);

	_pchRTFEnd = _pchRTFBuffer;				// Initialize RTF buffer ptr
	_cchBufferOut = 0;						//  and character count
	_cchOut = 0;							//  and character output

	cch = _prg->GetRange(cpMin, cpMost);	// Get rtp = cpMin and cch > 0
	rtp.SetCp(cpMin);
	_fRangeHasEOP = tp.IsAtEOP() || tp.FindEOP(cch);

#ifdef PWD_JUPITER
    // GuyBark: We need to keep track of the current font selected .
    _pwdCurrentFont  = 0;
    _pwdDefaultJFont = -1;
#endif // PWD_JUPITER

	// Start \rtfN or \pwdN group
	i =	(_dwFlags & SFF_RTFVAL) >> 16;
	if (!PutCtrlWord(CWF_GRV, (_dwFlags & SFF_PWD) ? i_pwd : i_rtf, i + 1) ||
		!PutCtrlWord(CWF_STR, i_ansi)) 
	{
		goto CleanUpNoEndGroup;
	}

#ifdef PWD_REHEADERCOMMENT
    // GuyBark: Add a comment at the top of a WordPad file to get the attention
    // of users who trying opening this file in something other than WordPad.
    if(_dwFlags & SFF_PWD) 
    {
        int        cch2, cbRequired;
        LPSTR      pszaComment = NULL;
        LPTSTR     pszwComment = NULL;
        HWND       hWndControl;

        // Get the RichEdit control window.
        if(_ped->TxGetWindow(&hWndControl) == S_OK)
        {
            // Can we get a Header comment from PWord?
            if((cch2 = SendMessage(GetParent(hWndControl), WM_PWORD_GETHEADERCOMMENT, 0, (LPARAM)&pszwComment)) && 
               pszwComment)
		    {
                // Try to output a multibyte comment in the file.
                BOOL bErrorWritingComment = TRUE;

                // RichEdit ALWAYS built unicode.
                if((cbRequired = WideCharToMultiByte(CP_ACP, 0, pszwComment, cch2, NULL, 0, NULL, NULL)))
                {
                    // Allocate space for a multibyte version.
                    if((pszaComment = (LPSTR)PvAlloc(cbRequired + 1, GMEM_ZEROINIT)))
                    {
                        // Use the same code page as that associated with this document.

                        // No! Don't use that. The code pafe associated with the document
                        // could be the unicode codepage, (1200). Calling WideCharToMultiByte()
                        // fails here if we supply the unicode code page. So use the default
                        // code page for whatever system the user is running on.
                        if(WideCharToMultiByte(CP_ACP, 0, pszwComment, cch2, pszaComment, cbRequired, NULL, NULL))
                        {
                            // Make sure it's null terminated.
                            pszaComment[cbRequired] = '\0';

                            // There is a comment, so output the comment group start.
	                        if(printF(szPwdComment))
	                        {
                                // Now output the comment itself.
        		                if(Puts(pszaComment, cbRequired))
                                {
                                    // End PWord Comment group.
	                                if(PutChar(chEndGroup)) 
                                    {
                                        // It all worked ok!
                                        bErrorWritingComment = FALSE;
                                    }
                                }
                            }
                        }

                        FreePv(pszaComment); 
                    }
                }

                // WordPad needs us to free the supplied memory too.
                LocalFree(pszwComment);

                // Did we have any problems?
                if(bErrorWritingComment)
                {
        		    goto CleanUp;
                }
            }
        }
    }
#endif // PWD_REHEADERCOMMENT


	// Determine the \ansicpgN value
	if(!pDocInfo)
	{
		fOutputEndGroup = TRUE;
		goto RAMError;
	}

	wCodePage = pDocInfo->wCpg;
	if (_dwFlags & SFF_UTF8 && !PutCtrlWord(CWF_VAL, i_utf, 8) ||
		wCodePage != tomInvalidCpg && wCodePage != CP_ACP &&
		!PutCtrlWord(CWF_VAL, i_ansicpg, wCodePage))
	{
		goto CleanUp;
	}

	if(!printF(szDefaultFont))
		goto CleanUp;

	LCID	lcid;
	LANGID	langid;

	if (_ped->GetDefaultLCID(&lcid) == NOERROR && 
		lcid != tomInvalidLCID && (langid = LANGIDFROMLCID(lcid)) &&
		!PutCtrlWord(CWF_VAL, i_deflang, langid))
	{
		goto CleanUp;
	}

	if (_ped->GetDefaultLCIDFE(&lcid) == NOERROR && 
		lcid != tomInvalidLCID && (langid = LANGIDFROMLCID(lcid)) &&
		!PutCtrlWord(CWF_VAL, i_deflangfe, langid))
	{
		goto CleanUp;
	}

	lDocDefaultTab = pDocInfo->dwDefaultTabStop;
	if(!lDocDefaultTab)
		lDocDefaultTab = lDefaultTab;

	if (lDocDefaultTab != 720 && !PutCtrlWord(CWF_VAL, i_deftab, lDocDefaultTab) ||
		BuildTables(rtp._rpCF, rtp._rpPF, cch) ||
		WriteFontTable() || WriteColorTable())
	{
		goto CleanUp;
	}

	if(_nHeadingStyle)
	{
		if(!PutCtrlWord(CWF_GRP, i_stylesheet) || !printF(szNormalStyle))
			goto CleanUp;
		
		for(i = 1; i < -_nHeadingStyle; i++)
		{
			if(!printF(szHeadingStyle, i, i))
				goto CleanUp;
		}
		Puts(szEndGroupCRLF, sizeof(szEndGroupCRLF) - 1); // End font table group
	}
	
	_ped->GetViewKind(&i);
	_ped->GetViewScale(&j);
	if (WriteInfo() ||
		_fRangeHasEOP && !PutCtrlWord(CWF_VAL, i_viewkind, i) ||
		(_dwFlags & SFF_PERSISTVIEWSCALE) && j != 100 &&
		!PutCtrlWord(CWF_VAL, i_viewscale, j))
	{
		goto CleanUp;
	}

	// Write Unicode character byte count for use by entire document (since
	// we don't use \plain's and since \ucN behaves as a char formatting tag,
	// we're safe outputting it only once).

	if(!PutCtrlWord(CWF_VAL, i_uc, iUnicodeCChDefault))
		goto CleanUp;

#ifdef PWD_JUPITER
    // GuyBark: We now output \uc1 and \uc2
    _UnicodeBytesPerChar = iUnicodeCChDefault;
#endif // PWD_JUPITER

	while (cch > 0)
	{
		// Get next run of chars with same para formatting
		cchPF = rtp.GetCchLeftRunPF();
		cchPF = min(cchPF, cch);

		AssertSz(cchPF, "CRTFW::WriteRtf: Empty para format run!");

		if(WriteParaFormat(&rtp))			// Write paragraph formatting
			goto CleanUp;

		while (cchPF > 0)
		{
			// Get next run of characters with same char formatting
			cchCF = rtp.GetCchLeftRunCF();
			cchCF = min(cchCF, cchPF);
			AssertSz(cchCF, "CRTFW::WriteRtf: Empty char format run!");

			const CCharFormat *	pCF = rtp.GetCF();

			if (WriteCharFormat(pCF))		// Write char attributes
				goto CleanUp;

			INT nCodePage =	(_dwFlags & SFF_UTF8)
						  ? CP_UTF8 : GetCodePage(pCF->bCharSet);

			while (cchCF > 0)
			{
				cchBuffer = min(cachBufferMost, cchCF);
				cchBuffer = rtp._rpTX.GetText(cchBuffer, pchBuffer);
				pch  = pchBuffer;
				cchT = cchBuffer;  
				if(cchT > 0)					
				{								
					TCHAR * pchWork = pch;
					LONG    cchWork = cchT;
					LONG	cchTWork;
					CRchTxtPtr rtpObjectSearch(rtp);

					while (cchWork >0)
					{
						cchT = cchWork ;
						pch = pchWork;
						while (cchWork > 0 )			// search for objects
						{
							if(*pchWork++ == WCH_EMBEDDING) 
							{
								break;				// Will write out object
							}
							cchWork--;
						}

						cchTWork = cchT - cchWork;
						if (cchTWork)   // write text before object
						{							
							if(WriteText(cchTWork, pch, nCodePage, 
									(pCF->bInternalEffects & CFEI_RUNISDBCS)))
							{
								goto CleanUp;
							}
						}
						rtpObjectSearch.Advance(cchTWork);
						if(cchWork > 0)		// there is object
						{
							DWORD cp = rtpObjectSearch.GetCp();
							COleObject *pobj;

							Assert(_ped->GetObjectMgr());

							pobj = _ped->GetObjectMgr()->GetObjectFromCp(cp);

							if( !pobj )
							{
								goto CleanUp;
							}

							// first, commit the object to make sure the pres. 
							// caches, etc. are up-to-date.  Don't worry 
							// about errors here.

							pobj->SaveObject();

							if(_fIncludeObjects) 
							{
								WriteObject(cp, pobj);
							}
							else if(!Puts(szObjPosHolder,sizeof(szObjPosHolder) - 1))
							{
								goto CleanUp;
							}
							rtpObjectSearch.Advance(1);
							cchWork--;
						}
					}
				}
				rtp.Advance(cchBuffer);
				cchCF	-= cchBuffer;
				cchPF	-= cchBuffer;
				cch		-= cchBuffer;
			}
		}
	}

CleanUp:
	// End RTF group
	Puts(szEndGroupCRLF,sizeof(szEndGroupCRLF) - 1);
	PutChar(0);
	FlushBuffer();

CleanUpNoEndGroup:
	FreePv(pchBuffer);

	if (_ecParseError != ecNoError)
	{
		TRACEERRSZSC("CRTFW::WriteRtf()", _ecParseError);
		Tracef(TRCSEVERR, "Writing error: %s", rgszParseError[_ecParseError]);
		
		if(!_pes->dwError)						// Make error code OLE-like
			_pes->dwError = -abs(_ecParseError);
		_cchOut = 0;
	}
	return _cchOut;

RAMError:
	ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecNoMemory;

	if(fOutputEndGroup)
		goto CleanUp;

	goto CleanUpNoEndGroup;
}

