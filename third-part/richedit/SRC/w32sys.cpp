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
 *	@module	w32sys.cpp - thin layer over Win32 services
 *	
 *	History: <nl>
 *		1/22/97 joseogl Created
 *
 */

// This prevents the "W32->" prefix from being prepended to our identifiers.

#define W32SYS_CPP

#include "_common.h"
#include "_host.h"
#include "_font.h"
#include <malloc.h>

// Our interface pointer
CW32System *W32;

CW32System::CW32System( )
{
	if (GetVersion(&_dwPlatformId, &_dwMajorVersion))
	{
        // GuyBark: IMM procs are available under Windows CE.
#ifndef PWD_JUPITER
		_fHaveIMMProcs = FALSE;
#else
        _fHaveIMMProcs = TRUE;
#endif // !PWD_JUPITER

		_icr3DDarkShadow = COLOR_WINDOWFRAME;
		if (_dwMajorVersion >= VERS4)
		{
			_icr3DDarkShadow = COLOR_3DDKSHADOW;
		}
	}
}

CW32System::~CW32System()
{
	FreeOle();
}

///////////////////////////////  Memory mamagement  /////////////////////////////////

/*
 *	PvAllocFn (cbBuf, uiMemFlags)
 *
 *	@mfunc	memory allocation.  Similar to GlobalAlloc.
 *
 *	@comm	The only flag of interest is GMEM_ZEROINIT, which
 *			specifies that memory should be zeroed after allocation.
 */
LPVOID CW32System::PvAlloc(
	ULONG	cbBuf, 			//@parm	Count of bytes to allocate
	UINT	uiMemFlags)		//@parm Flags controlling allocation
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvAlloc");
	
	void * pv = LocalAlloc(LMEM_FIXED, cbBuf);

	if( pv && (uiMemFlags & GMEM_ZEROINIT) )
	{
		ZeroMemory(pv, cbBuf);
	}
	
	return pv;
}

/*
 *	PvReAllocFn	(pvBuf, cbBuf)
 *
 *	@mfunc	memory reallocation.
 *
 *	FUTURE	(alexgo) this should be inline if we don't add any extra
 *			code here (like to zero the memory)
 */
LPVOID CW32System::PvReAlloc(
	LPVOID	pvBuf, 		//@parm Buffer to reallocate
	DWORD	cbBuf)		//@parm New size of buffer
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "PvReAlloc");

	if( pvBuf )
	{
		return LocalReAlloc(pvBuf, cbBuf, LMEM_MOVEABLE);
	}
	else
	{
		return LocalAlloc(LMEM_FIXED, cbBuf);
	}
}

/*
 *	FreePvFn (pvBuf)
 *
 *	@mfunc	free's memory
 *
 *	@rdesc	TRUE if pvBuf is not NULL
 */
BOOL CW32System::FreePv(
	LPVOID pvBuf)		//@parm Buffer to free
{
	TRACEBEGIN(TRCSUBSYSEDIT, TRCSCOPEINTERN, "FreePv");

	if( pvBuf )
	{
		LocalFree(pvBuf);
		return TRUE;
	}
	return FALSE;
}

// Handy defines.
#define ANSI_INDEX		0
#define ARABIC_INDEX	17
#define GREEK_INDEX		13
#define HAN_INDEX		-2
#define HANGUL_INDEX	10
#define HEBREW_INDEX	6
#define RUSSIAN_INDEX	8
#define SHIFTJIS_INDEX	7
#define THAI_INDEX		16
#define UNKNOWN_INDEX	-1

#define	PC437_CHARSET	254

#define IN_RANGE(n1, b, n2)		((unsigned)((b) - n1) <= n2 - n1)

/*
 *	@struct CPGCHAR |
 *		Locally used variable that contains code-page and char-set info
 */
typedef struct _cpgcharset
{
	INT			nCodePage;				// @field Code page
	BYTE		bCharSet;				// @field Character set
} CPGCHAR;

static const CPGCHAR rgCpgChar[] =
{
	{1252,	ANSI_CHARSET},
	{0,		DEFAULT_CHARSET},		// Not reliably implemented...
	{SYMBOL_CODEPAGE, SYMBOL_CHARSET},// No trans, except WORD -> BYTE
	{437,	PC437_CHARSET},			// United States IBM
	{850,	OEM_CHARSET},			// IBM Multilingual
	{1250,	EASTEUROPE_CHARSET},	// Eastern Europe
	{1255,	HEBREW_CHARSET},		// Hebrew
	{932,	SHIFTJIS_CHARSET},		// Japanese
	{1251,	RUSSIAN_CHARSET},		// Russian
	{936,	GB2312_CHARSET},		// PRC
	{949,	HANGEUL_CHARSET},		// Hangeul
	{1361,	JOHAB_CHARSET},			// JOHAB
	{950,	CHINESEBIG5_CHARSET},	// Chinese
	{1253,	GREEK_CHARSET},			// Greek
	{1254,	TURKISH_CHARSET},		// Turkish
	{1257,	BALTIC_CHARSET},		// Estonia, Lithuania, Latvia
	{874,	THAI_CHARSET},			// Thai
	{1256,  ARABIC_CHARSET},		// Arabic
	{10000,	MAC_CHARSET}			// Most popular Mac (English, etc.)
};

#define cCpgChar (sizeof(rgCpgChar) / sizeof(rgCpgChar[0]))
#define	LANG_TAIWAN	 MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)

const WORD CodePageTable[] = {
/* CodePage		  PLID	primary language
   ------------------------------------- */
	   0,		// 00 -	undefined
	1256,		// 01 - Arabic
	1251,		// 02 - Bulgarian
	1252,		// 03 - Catalan
	 950,		// 04 - Taiwan (Hong Kong, PRC, and Singapore are 936)
	1250,		// 05 - Czech
	1252,		// 06 - Danish
	1252,		// 07 - German
	1253,		// 08 - Greek
	1252,		// 09 - English
	1252,		// 0a - Spanish
	1252,		// 0b - Finnish
	1252,		// 0c - French
	1255,		// 0d - Hebrew
	1250,		// 0e - Hungarian
	1252,		// 0f - Icelandic
	1252,		// 10 - Italian
	 932,		// 11 - Japan
	 949,		// 12 - Korea
	1252,		// 13 - Dutch
	1252,		// 14 - Norwegian
	1250,		// 15 - Polish
	1252,		// 16 - Portuguese
	   0,		// 17 -	Rhaeto-Romanic
	1250,		// 18 - Romanian
	1251,		// 19 - Russian
	1250,		// 1a -	Croatian
	1250,		// 1b - Slovak
	1250,		// 1c -	Albanian
	1252,		// 1d - Swedish
	 874,		// 1e - Thai
	1254,		// 1f - Turkish
	   0,		// 20 -	Urdu
	1252,		// 21 - Indonesian
	1251,		// 22 - Ukranian
	1251,		// 23 - Byelorussian
	1250,		// 24 -	Slovenian
	1257,		// 25 - Estonia
	1257,		// 26 - Latvian
	1257,		// 27 - Lithuanian
	   0,		// 28 -	undefined
	1256,		// 29 - Farsi
	   0,		// 2a -	Vietnanese
	   0,		// 2b -	undefined
	   0,		// 2c -	undefined
	1252,		// 2d - Basque
	   0,		// 2e - Sorbian
	1251		// 2f - Macedonian
				// 30 - Sutu	*** use 1252 for the following ***
				// 31 - Tsonga
				// 32 - Tswana
				// 33 - Venda
				// 34 - Xhosa
				// 35 - Zulu
				// 36 - Africaans (uses 1252)
				// 38 - Faerose
				// 39 - Hindi
				// 3a - Maltese
				// 3b - Sami
				// 3c - Gaelic
				// 3e - Malaysian
				// 3f - 
				// 40 -
				// 41 - Swahili
};

#define nCodePageTable	(sizeof(CodePageTable)/sizeof(CodePageTable[0]))
#define lidSerbianCyrillic 0xc1a

// CW32System static members
BOOL CW32System::_fHaveIMMProcs;
DWORD CW32System::_dwPlatformId;
DWORD CW32System::_dwMajorVersion;
INT CW32System::_icr3DDarkShadow;

/*
 *	CW32System::CheckDBCInUnicodeStr (ptext)
 *
 *	@mfunc
 *		returns TRUE if there is a DBC in the Unicode buffer
 *
 *	@rdesc
 *		returns TRUE | FALSE 
 */
BOOL CW32System::CheckDBCInUnicodeStr(TCHAR *ptext)
{
	LONG iCharSetIndex;

	if ( ptext )
	{
		while (*ptext)
		{
			iCharSetIndex = CharSetIndexFromChar( *ptext++ );

			if ( iCharSetIndex == HAN_INDEX ||
				 iCharSetIndex == SHIFTJIS_INDEX ||
				 iCharSetIndex == HANGUL_INDEX )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
 *  CW32System::MbcsFromUnicode(pstr, cch, pwstr, cwch, codepage, flags)
 *
 *  @mfunc
 *		Converts a string to MBCS from Unicode. If cwch equals -1, the string
 *		is assumed to be NULL terminated.  -1 is supplied as a default argument.
 *
 *	@rdesc
 *		If [pstr] is NULL or [cch] is 0, 0 is returned.  Otherwise, the number
 *		of characters converted, including the terminating NULL, is returned
 *		(note that converting the empty string will return 1).  If the
 *		conversion fails, 0 is returned.
 *
 *	@devnote
 *		Modifies pstr
 */
int CW32System::MbcsFromUnicode(
	LPSTR	pstr,		//@parm Buffer for MBCS string
	int		cch,		//@parm Size of MBCS buffer, incl space for NULL terminator
	LPCWSTR pwstr,		//@parm Unicode string to convert
	int		cwch,		//@parm # chars in Unicode string, incl NULL terminator
	UINT	codepage,	//@parm Code page to use (CP_ACP is default)
	UN_FLAGS flags)		//@parm Indicates if WCH_EMBEDDING should be handled specially
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::MbcsFromUnicode");

	LONG			i;
	LPWSTR			pwstrtemp;
	CTempWcharBuf	twcb;

    Assert(cch >= 0 && pwstr && (cwch == -1 || cwch > 0));

    if (!pstr || cch == 0)
        return 0;

	// If we have to convert WCH_EMBEDDINGs, scan through and turn
	// them into spaces.  This is necessary for richedit1.0 compatibity,
	// as WideCharToMultiByte will turn WCH_EMBEDDING into a '?'
	if( flags == UN_CONVERT_WCH_EMBEDDING )
	{
		if( cwch == -1 ) 
			cwch = wcslen(pwstr) + 1;

		pwstrtemp = twcb.GetBuf(cwch);
		if( pwstrtemp )
		{
			for( i = 0; i < cwch; i++ )
			{
				pwstrtemp[i] = pwstr[i];

				if( pwstr[i] == WCH_EMBEDDING )
					pwstrtemp[i] = L' ';
			}
			pwstr = pwstrtemp;
		}
	}
    return WideCharToMultiByte(codepage, 0, pwstr, cwch, pstr, cch, NULL, NULL);
}

/*
 *  CW32System::UnicodeFromMbcs(pwstr, cwch, pstr, cch,	uiCodePage)
 *
 *	@mfunc
 *		Converts a string to Unicode from MBCS.  If cch equals -1, the string
 *		is assumed to be NULL terminated.  -1 is supplied as a default
 *		argument.
 *
 *	@rdesc
 *		If [pwstr] is NULL or [cwch] is 0, 0 is returned.  Otherwise,
 *		the number of characters converted, including the terminating
 *		NULL, is returned (note that converting the empty string will
 *		return 1).  If the conversion fails, 0 is returned.
 *
 *	@devnote
 *		Modifies:   [pwstr]
 */
int CW32System::UnicodeFromMbcs(
	LPWSTR	pwstr,		//@parm Buffer for Unicode string
	int		cwch,		//@parm Size of Unicode buffer, incl space for NULL terminator
	LPCSTR	pstr,		//@parm MBCS string to convert
	int		cch,		//@parm # chars in MBCS string, incl NULL terminator
	UINT	uiCodePage)	//@parm Code page to use (CP_ACP is default)
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::UnicodeFromMbcs");

    Assert(pstr && cwch >= 0 && (cch == -1 || cch >= 0));

    if (!pwstr || cwch == 0)
        return 0;

    return MultiByteToWideChar(uiCodePage, 0, pstr, cch, pwstr, cwch);
}

/*
 *	CW32System::TextHGlobalAtoW (hglobalA, pbDBCInString)
 *
 *	@func
 *		translates a unicode string contained in an hglobal and
 *		wraps the ansi version in another hglobal
 *
 *	@devnote 
 *		does *not* free the incoming hglobal
 */
HGLOBAL	CW32System::TextHGlobalAtoW(
	HGLOBAL hglobalA,
	BOOL *	pbDBCInString )
{
	TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CW32System::TextHGlobalAtoW");

	if( !hglobalA )
		return NULL;

	if ( pbDBCInString )
		*pbDBCInString = FALSE;

	HGLOBAL hnew;
	LPSTR	pstr = (LPSTR)GlobalLock(hglobalA);
	DWORD	dwSize = GlobalSize(hglobalA);
	LONG	cbSize = (dwSize + 1) * sizeof(WCHAR);
	
    hnew = GlobalAlloc(GMEM_FIXED, cbSize );
	if( hnew )
	{
		LPWSTR pwstr = (LPWSTR)GlobalLock(hnew);
		int iLen = UnicodeFromMbcs( pwstr, dwSize + 1, pstr );
		GlobalUnlock(hnew);
		if ( iLen && pbDBCInString )
		{
			int sLen = 0;
			while (pstr[sLen]) sLen++;
			if ( iLen < sLen + 1)				// Allow for NULL terminator
				*pbDBCInString = TRUE;
		}
	}
	GlobalUnlock(hglobalA);
	return hnew;
}

/*
 *	CW32System::TextHGlobalWtoA(hglobalW)
 *
 *	@func
 *		converts a unicode text hglobal into a newly allocated
 *		allocated hglobal with ANSI data
 *
 *	@devnote
 *		does *NOT* free the incoming hglobal 
 */
HGLOBAL CW32System::TextHGlobalWtoA( HGLOBAL hglobalW )
{
	TRACEBEGIN(TRCSUBSYSUTIL, TRCSCOPEINTERN, "CW32System::TextHGlobalWtoA");

	if( !hglobalW )
		return NULL;

	HGLOBAL hnew = NULL;
	LPWSTR 	pwstr = (LPWSTR)GlobalLock(hglobalW);
	DWORD	dwSize = GlobalSize(hglobalW);
	LONG	cbSize = (dwSize * 2) * sizeof(CHAR);
	hnew = GlobalAlloc(GMEM_FIXED, cbSize);

	if( hnew )
	{
		LPSTR pstr = (LPSTR)GlobalLock(hnew);
		int	iLen = MbcsFromUnicode(pstr, cbSize, pwstr );
		GlobalUnlock(hnew);
	}
	GlobalUnlock(hglobalW);
	return hnew;
}	

/*
 *	CW32System::ConvertLanguageIDtoCodePage (lid)
 *
 *	@mfunc		Maps a language ID to a Code Page
 *
 *	@rdesc		returns Code Page
 *
 *	@devnote: 
 *		This routine takes advantage of the fact that except for Chinese,
 *		the code page is determined uniquely by the primary language ID,
 *		which is given by the low-order 10 bits of the lcid.
 *
 *		The WORD CodePageTable could be replaced by a BYTE with the addition
 *		of a couple of if's and the BYTE table replaced by a nibble table
 *		with the addition of a shift and a mask.  Since the table is only
 *		96 bytes long, it seems that the simplicity of using actual code page
 *		values is worth the extra bytes.
 */
UINT CW32System::ConvertLanguageIDtoCodePage(
	WORD lid)				//@parm Language ID to map to code page
{
	UINT j = PRIMARYLANGID(lid);			// j = primary language (PLID)

	if(j >= LANG_CROATIAN)					// PLID = 0x1a
	{
		if(lid == lidSerbianCyrillic)		// Special case for LID = 0xc1a
			return 1251;					// Use Cyrillic code page

		if(j >= nCodePageTable)				// Languages above table currently
			return 1252;					//  all take 1252
	}

	j = CodePageTable[j];					// Translate PLID to code page

	if(j != 950 || lid == LANG_TAIWAN)		// All but China (except for Taiwan)
		return j;

	return 936;								// Hong Kong, Singapore, and PRC
}

/*
 *	GetLocaleLCID ()
 *
 *	@mfunc		Maps an LCID for thread to a Code Page
 *
 *	@rdesc		returns Code Page
 */
LCID CW32System::GetLocaleLCID()
{
	return GetThreadLocale();
}


/*
 *	GetLocaleCodePage ()
 *
 *	@mfunc		Maps an LCID for thread to a Code Page
 *
 *	@rdesc		returns Code Page
 */
UINT CW32System::GetLocaleCodePage()
{
	return W32->ConvertLanguageIDtoCodePage(GetThreadLocale());
}

/*
 *	GetKeyboardLCID ()
 *
 *	@mfunc		Gets LCID for keyboard active on current thread
 *
 *	@rdesc		returns Code Page
 */
LCID CW32System::GetKeyboardLCID()
{
    return (WORD)::GetKeyboardLayout(0 /*idThread*/);
}

/*
 *	GetKeyboardCodePage ()
 *
 *	@mfunc		Gets Code Page for keyboard active on current thread
 *
 *	@rdesc		returns Code Page
 */
UINT CW32System::GetKeyboardCodePage()
{
    return W32->ConvertLanguageIDtoCodePage((WORD)::GetKeyboardLayout(0 /*idThread*/));
}

/*
 *  IsFELCID(lcid)
 *
 *	@mfunc
 *		Returns TRUE iff lcid is for a FE country.
 *
 *	@rdesc
 *		TRUE iff lcid is for a FE country.
 *
 */
BOOL CW32System::IsFELCID(LCID lcid)
{
	switch(PRIMARYLANGID(LANGIDFROMLCID(lcid)))
	{
		case LANG_CHINESE:
		case LANG_JAPANESE:
		case LANG_KOREAN:
			return TRUE;
	}

	return FALSE;
}

/*
 *  IsFECharset(bCHarSet)
 *
 *	@mfunc
 *		Returns TRUE iff charset may be for a FE country.
 *
 *	@rdesc
 *		TRUE iff charset may be for a FE country.
 *
 */
BOOL CW32System::IsFECharset(BYTE bCharSet)
{
	switch(bCharSet)
	{
		case CHINESEBIG5_CHARSET:
		case SHIFTJIS_CHARSET:
		case HANGEUL_CHARSET:
		case JOHAB_CHARSET:
		case GB2312_CHARSET:
			return TRUE;
	}

	return FALSE;
}

/*
 *  IsFEChar(ch)
 *
 *	GUYBARK: ADDED THIS!
 *
 *      Return TRUE, if this input Unicode character lies in a FE range.
 *
 */
BOOL CW32System::IsFEChar(WCHAR ch)
{
    LONG iCharSetIndex;

    iCharSetIndex = CharSetIndexFromChar(ch);

    if(iCharSetIndex == HAN_INDEX ||
       iCharSetIndex == SHIFTJIS_INDEX ||
       iCharSetIndex == HANGUL_INDEX )
    {
        return TRUE;
    }

    return FALSE;
}

typedef struct {
	WCHAR codepoint;
	UCHAR charsets;
	UCHAR runlength;
} Data_125X;

/*
 *  In125x(ch, bCharSet)
 *
 *	@mfunc
 *		Returns non zero iff char is in given 125x charset.
 *      If the charset is any charset other than one corresponding
 *      to a 125x codepage, the return value
 *		is a bit mask identifying a subset of the eight 125x code
 *      pages (bit 1 = 1250, bit 2 = 1251, ... bit 8 = 1257) to which
 *		the input char belongs.
 *
 *	@rdesc
 *		TRUE iff char is in input 125x charset.
 *
 */
INT CW32System::In125x( TCHAR ch, BYTE bCharSet )
{
	static const Data_125X Table_125X[] = {
		{   0x80, 0xfd,   1},
		{   0x81, 0xbd,   1},
		{   0x83, 0x81,   1},
		{   0x88, 0x8b,   1},
		{   0x8a, 0xe8,   1},
		{   0x8c, 0xa8,   1},
		{   0x8d, 0x3c,   2},
		{   0x8f, 0x7c,   1},
		{   0x90, 0xbd,   1},
		{   0x98, 0xcb,   1},
		{   0x9a, 0xe8,   1},
		{   0x9c, 0xa8,   1},
		{   0x9d, 0x3c,   2},
		{   0x9f, 0xe8,   1},
		{   0xa0, 0xff,   1},
		{   0xa1, 0x34,   1},
		{   0xa2, 0xf4,   1},
		{   0xa3, 0xfc,   1},
		{   0xa4, 0xff,   1},
		{   0xa5, 0x7c,   1},
		{   0xa6, 0xff,   2},
		{   0xa8, 0xfd,   1},
		{   0xa9, 0xff,   1},
		{   0xaa, 0x14,   1},
		{   0xab, 0xff,   4},
		{   0xaf, 0xf4,   1},
		{   0xb0, 0xff,   2},
		{   0xb2, 0xfc,   2},
		{   0xb4, 0xf5,   1},
		{   0xb5, 0xff,   3},
		{   0xb8, 0xf5,   1},
		{   0xb9, 0xf4,   1},
		{   0xba, 0x14,   1},
		{   0xbb, 0xff,   1},
		{   0xbc, 0xf4,   1},
		{   0xbd, 0xfc,   1},
		{   0xbe, 0xf4,   1},
		{   0xbf, 0x34,   1},
		{   0xc0, 0x14,   1},
		{   0xc1, 0x15,   2},
		{   0xc3, 0x14,   1},
		{   0xc4, 0x95,   1},
		{   0xc5, 0x94,   2},
		{   0xc7, 0x15,   1},
		{   0xc8, 0x14,   1},
		{   0xc9, 0x95,   1},
		{   0xca, 0x14,   1},
		{   0xcb, 0x15,   1},
		{   0xcc, 0x14,   1},
		{   0xcd, 0x15,   2},
		{   0xcf, 0x14,   1},
		{   0xd0,  0x4,   1},
		{   0xd1, 0x14,   2},
		{   0xd3, 0x95,   1},
		{   0xd4, 0x15,   1},
		{   0xd5, 0x94,   1},
		{   0xd6, 0x95,   1},
		{   0xd7, 0xf5,   1},
		{   0xd8, 0x94,   1},
		{   0xd9, 0x14,   1},
		{   0xda, 0x15,   1},
		{   0xdb, 0x14,   1},
		{   0xdc, 0x95,   1},
		{   0xdd,  0x5,   1},
		{   0xde,  0x4,   1},
		{   0xdf, 0x95,   1},
		{   0xe0, 0x54,   1},
		{   0xe1, 0x15,   1},
		{   0xe2, 0x55,   1},
		{   0xe3, 0x14,   1},
		{   0xe4, 0x95,   1},
		{   0xe5, 0x94,   2},
		{   0xe7, 0x55,   1},
		{   0xe8, 0x54,   1},
		{   0xe9, 0xd5,   1},
		{   0xea, 0x54,   1},
		{   0xeb, 0x55,   1},
		{   0xec, 0x14,   1},
		{   0xed, 0x15,   1},
		{   0xee, 0x55,   1},
		{   0xef, 0x54,   1},
		{   0xf0,  0x4,   1},
		{   0xf1, 0x14,   2},
		{   0xf3, 0x95,   1},
		{   0xf4, 0x55,   1},
		{   0xf5, 0x94,   1},
		{   0xf6, 0x95,   1},
		{   0xf7, 0xf5,   1},
		{   0xf8, 0x94,   1},
		{   0xf9, 0x54,   1},
		{   0xfa, 0x15,   1},
		{   0xfb, 0x54,   1},
		{   0xfc, 0xd5,   1},
		{   0xfd,  0x5,   1},
		{   0xfe,  0x4,   1},
		{   0xff, 0x14,   1},
		{  0x100, 0x80,   2},
		{  0x102,  0x1,   2},
		{  0x104, 0x81,   4},
		{  0x10c, 0x81,   2},
		{  0x10e,  0x1,   4},
		{  0x112, 0x80,   2},
		{  0x116, 0x80,   2},
		{  0x118, 0x81,   2},
		{  0x11a,  0x1,   2},
		{  0x11e, 0x10,   2},
		{  0x122, 0x80,   2},
		{  0x12a, 0x80,   2},
		{  0x12e, 0x80,   2},
		{  0x130, 0x10,   2},
		{  0x136, 0x80,   2},
		{  0x139,  0x1,   2},
		{  0x13b, 0x80,   2},
		{  0x13d,  0x1,   2},
		{  0x141, 0x81,   4},
		{  0x145, 0x80,   2},
		{  0x147,  0x1,   2},
		{  0x14c, 0x80,   2},
		{  0x150,  0x1,   2},
		{  0x152, 0x54,   2},
		{  0x154,  0x1,   2},
		{  0x156, 0x80,   2},
		{  0x158,  0x1,   2},
		{  0x15a, 0x81,   2},
		{  0x15e, 0x11,   2},
		{  0x160, 0x95,   2},
		{  0x162,  0x1,   4},
		{  0x16a, 0x80,   2},
		{  0x16e,  0x1,   4},
		{  0x172, 0x80,   2},
		{  0x178, 0x14,   1},
		{  0x179, 0x81,   6},
		{  0x192, 0x7c,   1},
		{  0x2c6, 0x74,   1},
		{  0x2c7, 0x81,   1},
		{  0x2d8,  0x1,   1},
		{  0x2d9, 0x81,   1},
		{  0x2db, 0x81,   1},
		{  0x2dc, 0x34,   1},
		{  0x2dd,  0x1,   1},
		{  0x384,  0x8,   3},
		{  0x388,  0x8,   3},
		{  0x38c,  0x8,   1},
		{  0x38e,  0x8,  20},
		{  0x3a3,  0x8,  44},
		{  0x401,  0x2,  12},
		{  0x40e,  0x2,  66},
		{  0x451,  0x2,  12},
		{  0x45e,  0x2,   2},
		{  0x490,  0x2,   2},
		{  0x5b0, 0x20,  20},
		{  0x5d0, 0x20,  27},
		{  0x60c, 0x40,   1},
		{  0x61b, 0x40,   1},
		{  0x61f, 0x40,   1},
		{  0x621, 0x40,  26},
		{  0x640, 0x40,  19},
		{  0x67e, 0x40,   1},
		{  0x686, 0x40,   1},
		{  0x698, 0x40,   1},
		{  0x6af, 0x40,   1},
		{ 0x200c, 0x40,   2},
		{ 0x200e, 0x60,   2},
		{ 0x2013, 0xff,   2},
		{ 0x2015,  0x8,   1},
		{ 0x2018, 0xff,   3},
		{ 0x201c, 0xff,   3},
		{ 0x2020, 0xff,   3},
		{ 0x2026, 0xff,   1},
		{ 0x2030, 0xff,   1},
		{ 0x2039, 0xff,   2},
		{ 0x2116,  0x2,   1},
		{ 0x2122, 0xff,   1}
	};

	// Easy check for ASCII
	if (ch <= 0x7f)
	{
		return 0xff;
	}

	// Easy check for missing codes
	if (ch > 0x2122)
	{
		return 0;
	}

	// Perform binary search to find entry in table
	int low = 0;
	int high = sizeof( Table_125X ) / sizeof( Data_125X ) - 1;
	int middle;
	int midval;
	int runlength;
	int result = 0;

	while (low <= high)
	{
		middle = (high + low) / 2;
		midval = Table_125X[middle].codepoint;
		if (midval > ch)
		{
			high = middle - 1;
		}
		else
		{
			low = middle + 1;
		}
		runlength = Table_125X[middle].runlength;
		if (ch >= midval && ch <= midval + runlength - 1)
		{
			result = Table_125X[middle].charsets;
			break;
		}
	}

	switch (bCharSet)
	{
	case EASTEUROPE_CHARSET :
		// Code Page 1250
		result &= 0x1;
		break;
	case RUSSIAN_CHARSET :
		// Code Page 1251
		result &= 0x2;
		break;
	case ANSI_CHARSET :
		// Code Page 1252
		result &= 0x4;
		break;
	case GREEK_CHARSET :
		// Code Page 1253
		result &= 0x8;
		break;
	case TURKISH_CHARSET :
		// Code Page 1254
		result &= 0x10;
		break;
	case HEBREW_CHARSET :
		// Code Page 1255
		result &= 0x20;
		break;
	case ARABIC_CHARSET :
		// Code Page 1255
		result &= 0x40;
		break;
	case BALTIC_CHARSET :
		// Code Page 1255
		result &= 0x80;
		break;
	default :
		break;
	}

	return result;
}

/*
 *	IsLeadByte(ach, cpg)
 *
 *	@mfunc
 *		Returns TRUE iff the byte ach is a lead byte for the code page cpg.
 *
 *	@rdesc
 *		TRUE if ach is lead byte for cpg
 *
 *	@comm
 *		This function potentially doesn't support as many code pages as the
 *		Win32 IsDBCSLeadByte() function (and it might not be as up-to-date).
 *		An AssertSz() is included to compare the results when the code page
 *		is supported by the system.
 *
 *		Reference: \\sparrow\sysnls\cptable\win95. See code-page txt files
 *		in subdirectories windows\txt and others\txt.
 */
BOOL CW32System::IsLeadByte(BYTE ach, UINT cpg)
{
	if(ach < 0x81 || cpg == CP_UTF8)				// Smallest known lead
		return FALSE;								//  byte = 0x81:
													//  early out
	BOOL fRet = FALSE;								// Variable to check
													//  result with system
	if (cpg > 950)									//  ifdef DEBUG
	{
		if (cpg < 1361)								// E.g., the 125x's are
			return FALSE;							//  SBCSs: early out

		else if (cpg == 1361)						// Korean Johab
			fRet = IN_RANGE(0x84, ach, 0xd3) ||		// 0x84 <= ach <= 0xd3
				   IN_RANGE(0xd8, ach, 0xde) ||		// 0xd8 <= ach <= 0xde
				   IN_RANGE(0xe0, ach, 0xf9);		// 0xe0 <= ach <= 0xf9

		else if (cpg == 10001)						// Mac Japanese
			goto JIS;

		else if (cpg == 10002)						// Mac Trad Chinese (Big5)
			fRet = ach <= 0xfe;

		else if (cpg == 10003)						// Mac Korean
			fRet = IN_RANGE(0xa1, ach, 0xac) ||		// 0xa1 <= ach <= 0xac
				   IN_RANGE(0xb0, ach, 0xc8) ||		// 0xb0 <= ach <= 0xc8
				   IN_RANGE(0xca, ach, 0xfd);		// 0xca <= ach <= 0xfd

		else if (cpg == 10008)						// Mac Simplified Chinese
			fRet = IN_RANGE(0xa1, ach, 0xa9) ||		// 0xa1 <= ach <= 0xa9
				   IN_RANGE(0xb0, ach, 0xf7);		// 0xb0 <= ach <= 0xf7
	}
	else if (cpg >= 932)							// cpg <= 950
	{
		if (cpg == 950 || cpg == 949 || cpg == 936)	// Chinese (Taiwan, HK),
		{											//  Korean Ext Wansung,
			fRet = ach <= 0xfe;						//  PRC GBK: 0x81 - 0xfe
		}											  
		else if (cpg == 932)						// Japanese
JIS:		fRet = ach <= 0x9f || IN_RANGE(0xe0, ach, 0xfc);
	}

	#ifdef DEBUG
	TCHAR	ch;
	static	BYTE asz[2] = {0xe0, 0xe0};				// All code pages above

	// if cpg == 0, fRet will FALSE but IsDBCSLeadByteEx may succeed. 
	if ( cpg )
	{
		// If system supports cpg, then fRet should agree with system result
		AssertSz(MultiByteToWideChar(cpg, 0, (char *)asz, 2, &ch, 1) <= 0 ||
			fRet == IsDBCSLeadByteEx(cpg, ach),
			"IsLeadByte() differs from IsDBCSLeadByte()");
	}
	#endif

	return fRet;
}

/*
 *	IsTrailByte(aszBuff, cb, cpg)
 *
 *	@mfunc
 *		Returns TRUE iff the byte aszBuf[cb] is a trail byte for
 *		the code page cpg.
 *
 *	@rdesc
 *		TRUE if aszBuf[cb] is a trail byte for cpg
 *
 *	@comm
 *		The byte is a trail byte if it's preceeded by an odd number of
 *		contiguous lead bytes in aszBuff.
 */
BOOL CW32System::IsTrailByte(
	BYTE *	aszBuff,	//@parm Points to byte buffer
	LONG	cb,			//@parm Count of bytes preceeding possible trail byte
	UINT	cpg)		//@parm Code page to use
{
	LONG i;

	Assert(cb >= 0 && aszBuff);

	for (i = cb; i && IsLeadByte(aszBuff[i - 1], cpg); i--) ;

	return (cb - i) & 1;
}

/*
 *	GetCharSet(nCP)
 *
 *	@func
 *		Get character set for code page <p nCP>
 *
 *	@rdesc
 *		BYTE		character set for code page <p nCP>
 */
BYTE CW32System::GetCharSet(
	INT nCP)			//@parm Code page
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCharSet");

	const CPGCHAR *	pcpgchar = rgCpgChar;

    int i = 0;
	for (i = 0; i < cCpgChar ; i++)
	{
		if (pcpgchar->nCodePage == nCP)
			break;
		++pcpgchar;
	}
	return i < cCpgChar ? pcpgchar->bCharSet : 0;
}

/*
 *	GetCodePage(bCharSet)
 *
 *	@func
 *		Get code page for character set <p bCharSet>
 *
 *	@rdesc
 *		Code page for character set <p bCharSet>
 */
INT CW32System::GetCodePage(
	BYTE bCharSet)		//@parm CharSet
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "GetCodePage");

	const CPGCHAR *	pcpgchar = rgCpgChar;

    int i = 0;
	for (i = 0; i < cCpgChar ; i++)
	{
		if (pcpgchar->bCharSet == bCharSet)
			break;
		++pcpgchar;
	}
	return i < cCpgChar ? pcpgchar->nCodePage : 0;
}

/*
 *	IsCharSetValid(bCharSet)
 *
 *	@func
 *		Return TRUE iff <p bCharSet> is a valid character set index
 *
 *	@rdesc
 *		TRUE iff <p bCharSet> is a valid character set index
 */
BOOL CW32System::IsCharSetValid(
	BYTE bCharSet)		//@parm CharSet
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "IsCharSetValid");

	LONG			i = cCpgChar;
	const CPGCHAR *	pcpgchar = rgCpgChar;

	while(i--)
	{
		if (pcpgchar->bCharSet == bCharSet)
			return TRUE;
		++pcpgchar;
	}
	return FALSE;
}

/*
 *	MBTWC (CodePage, dwFlags, pstrMB, cchMB, pstrWC, cchWC)
 *
 *	@mfunc
 *		Convert MultiByte (MB) string pstrMB of length cchMB to WideChar (WC)
 *		string pstrWC of length cchWC according to the flags dwFlags and code
 *		page CodePage.  If CodePage = SYMBOL_CODEPAGE 
 *		(usually for SYMBOL_CHARSET strings),
 *		convert each byte in pstrMB to a wide char with a zero high byte
 *		and a low byte equal to the MultiByte string byte, i.e., no
 *		translation other than a zero extend into the high byte.  Else call
 *		the Win32 MultiByteToWideChar() function.
 *
 *	@rdesc
 *		Count of characters converted
 */
int CW32System::MBTWC(
	INT		CodePage,	//@parm Code page to use for conversion
	DWORD	dwFlags,	//@parm Flags to guide conversion
	LPCSTR	pstrMB,		//@parm MultiByte string to convert to WideChar
	int		cchMB,		//@parm Count of chars (bytes) in pstrMB or -1
	LPWSTR	pstrWC,		//@parm WideChar string to receive converted chars
	int		cchWC,		//@parm Max count for pstrWC or 0 to get cch needed
	LPBOOL 	pfNoCodePage) //@parm Out parm to receive whether code page is on system
{
	BOOL	fNoCodePage = FALSE;			// Default code page is on OS
	int		cch = -1;

	if(CodePage == CP_UTF8)
	{
		DWORD ch,ch1;

		for(cch = 0; cchMB--; )
		{
			ch = ch1 = *(BYTE *)pstrMB++;
			Assert(ch < 256);
			if(ch > 127 && cchMB && IN_RANGE(0x80, *(BYTE *)pstrMB, 0xBF))
			{
				// Need at least 2 bytes of form 110bbbbb 10bbbbbb
				ch1 = ((ch1 & 0x1F) << 6) + (*pstrMB++ & 0x3F);
				cchMB--;
				if(ch > 0xE0 && cchMB && IN_RANGE(0x80, *(BYTE *)pstrMB, 0xBF))
				{
					// Need at least 3 bytes of form 1110bbbb 10bbbbbb 10bbbbbb
					ch1 = (ch1 << 6) + (*pstrMB++ & 0x3F);
					cchMB--;
					if (ch > 0xF0 && cchMB && IN_RANGE(0x80, *(BYTE *)pstrMB, 0xBF))
					{
						// Handle 4-byte form for 16 UTF-16 planes above the
						// BMP) expect: 11110bbb 10bbbbbb 10bbbbbb 10bbbbbb
						ch1 = ((ch1 & 0x7FFF) << 6) + (*(BYTE *)pstrMB++ & 0x3F)
							- 0x10000;			// Subtract offset for BMP
						if(ch1 <= 0xFFFFF)		// Fits in 20 bits
						{
							cch++;				// Two 16-bit surrogate codes
							if(cch < cchWC)
								*pstrWC++ = UTF16_LEAD + (ch1 >> 10);
							ch1 = (ch1 & 0x3FF) + UTF16_TRAIL; 
							cchMB--;
						}
						else ch1 = '?';
					}
				}
			}
			cch++;
			if(cch < cchWC)
				*pstrWC++ = ch1;
			if(!ch)
				break;
		}
	}
	else if(CodePage != SYMBOL_CODEPAGE)	// Not SYMBOL_CHARSET
	{
		fNoCodePage = TRUE;					// Default codepage isn't on OS
		if(CodePage >= 0)					// Might be..
		{
			cch = MultiByteToWideChar(
				CodePage, dwFlags, pstrMB, cchMB, pstrWC, cchWC);
			if(cch > 0)
				fNoCodePage = FALSE;		// Codepage is on OS
		}
	}
	if(pfNoCodePage)
		*pfNoCodePage = fNoCodePage;

	if(cch <= 0)
	{			
		// SYMBOL_CHARSET or conversion failed: bytes -> words with  
		//  high bytes of 0.  Return count for full conversion

		if(cchWC <= 0)					
			return cchMB >= 0 ? cchMB : (strlen(pstrMB) + 1);

		int cchMBMax = cchMB;

		if(cchMB < 0)					// If negative, use NULL termination
			cchMBMax = tomForward;			//  of pstrMB

		cchMBMax = min(cchMBMax, cchWC);

		for(cch = 0; (cchMB < 0 ? *pstrMB : 1) && cch <= cchMBMax; cch++)
		{
			*pstrWC++ = (unsigned char)*pstrMB++;
		}
		
		// NULL-terminate the WC string if the MB string was NULL-terminated,
		// and if there is room in the WC buffer.
		if(cchMB < 0 && cch < cchWC)
		{
			*pstrWC = 0;
			cch++;
		}
	}
	return cch;
}

/*
 *	WCTMB (CodePage, dwFlags, pstrWC, cchWC, pstrMB, cchMB, pchDefault,
 *			pfUsedDef)
 *
 *	@mfunc
 *		Convert WideChar (WC) string pstrWC of length cchWC to MultiByte (MB) 
 *		string pstrMB of length cchMB according to the flags dwFlags and code
 *		page CodePage.  If CodePage = SYMBOL_CODEPAGE 
 *		(usually for SYMBOL_CHARSET strings),
 *		convert each character in pstrWC to a byte, discarding the high byte.
 *		Else call the Win32 WideCharToMultiByte() function.
 *
 *	@rdesc
 *		Count of bytes stored in target string pstrMB
 */
int CW32System::WCTMB(
	INT		CodePage,	//@parm Code page to use for conversion
	DWORD	dwFlags,	//@parm Flags to guide conversion
	LPCWSTR	pstrWC,		//@parm WideChar string to convert
	int		cchWC,		//@parm Count for pstrWC or -1 to use NULL termination
	LPSTR	pstrMB,		//@parm MultiByte string to receive converted chars
	int		cchMB,		//@parm Count of chars (bytes) in pstrMB or 0
	LPCSTR	pchDefault,	//@parm Default char to use if conversion fails
	LPBOOL	pfUsedDef,	//@parm Out parm to receive whether default char used
	LPBOOL 	pfNoCodePage) //@parm Out parm to receive whether code page is on system
{
	int		cch = -1;						// No chars converted yet
	BOOL	fNoCodePage = FALSE;			// Default code page is on OS

	if(pfUsedDef)							// Default that all chars can be
		*pfUsedDef = FALSE;					//  converted

#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS	0x400
#endif

	if (_dwPlatformId == VER_PLATFORM_WIN32_NT && 
		_dwMajorVersion > 4 && !dwFlags)
	{
		dwFlags = WC_NO_BEST_FIT_CHARS;
	}

	if(CodePage == CP_UTF8)					// Convert to UTF8 since OS
	{										// doesn't (pre NT 5.0)
		WCHAR ch;
		cch = 0;							// No converted bytes yet
		while(cchWC--)
		{
			ch = *pstrWC++;					// Get Unicode char
			if(ch <= 127)					// It's ASCII
			{
				cch++;
				if(cch < cchMB)
					*pstrMB++ = ch;			// One more converted byte
				if(!ch)						// Quit on NULL termination
					break;
				continue;
			}
			if(ch <= 0x7FF)					// Need 2 bytes of form:
			{								//  110bbbbb 10bbbbbb
				cch += 2;
				if(cch < cchMB)				// Store lead byte
					*pstrMB++ = 0xC0 + (ch >> 6);
			}
			else							// Need 3 bytes of form:
			{								//  1110bbbb 10bbbbbb
				cch += 3;					//  10bbbbbb
				if(cch < cchMB)				// Store lead byte followed by
				{							//  first trail byte 
					*pstrMB++ = 0xE0 + (ch >> 12);
					*pstrMB++ = 0x80 + (ch >> 6 & 0x3F);
				}
			}
			if(cch < cchMB)					// Store final UTF-8 byte
				*pstrMB++ = 0x80 + (ch & 0x3F);
		}
	}
	else if(CodePage != SYMBOL_CODEPAGE)
	{
		fNoCodePage = TRUE;					// Default codepage not on OS
		if(CodePage >= 0)					// Might be...
		{
			cch = WideCharToMultiByte(CodePage, dwFlags,
					pstrWC, cchWC, pstrMB, cchMB, pchDefault, pfUsedDef);
			if(cch > 0)
				fNoCodePage = FALSE;		// Found codepage on system
		}
	}
	if(pfNoCodePage)
		*pfNoCodePage = fNoCodePage;

	// SYMBOL_CHARSET, fIsDBCS or conversion failed: low bytes of words ->
	//  bytes
	if(cch <= 0)
	{									
		// Return multibyte count for full conversion. cchWC is correct for
		// single-byte charsets like the 125x's
		if(cchMB <= 0)
		{
			return cchWC >= 0 ? cchWC : wcslen(pstrWC);
		}

		char chDefault = 0;
		BOOL fUseDefaultChar = (pfUsedDef || pchDefault) && CodePage != SYMBOL_CODEPAGE; 

		if(fUseDefaultChar)
		{
			// determine a default char for our home-grown conversion
			if(pchDefault)
			{
				chDefault = *pchDefault;
			}
			else
			{
				static char chSysDef = 0;
				static BOOL fGotSysDef = FALSE;

				// 0x2022 is a math symbol with no conversion to ANSI
				const WCHAR szCantConvert[] = { 0x2022 };
				BOOL fUsedDef;

				if(!fGotSysDef)
				{
					fGotSysDef = TRUE;

					if(!(WideCharToMultiByte
							(CP_ACP, 0, szCantConvert, 1, &chSysDef, 1, NULL, 
										&fUsedDef) == 1 && fUsedDef))
					{
						AssertSz(0, "WCTMB():  Unable to determine what the "
									"system uses as its default replacement "
									"character.");
						chSysDef = '?';
					}
				}
				chDefault = chSysDef;
			}
		}

		int cchWCMax = cchWC;

		// If negative, use NULL termination of pstrMB
		if(cchWC < 0)
		{
			cchWCMax = tomForward;
		}

		cchWCMax = min(cchWCMax, cchMB);

		for(cch = 0; (cchWC < 0 ? *pstrWC : 1) && cch < cchWCMax; cch++)
		{
			// TODO(BradO):  Should this be 0x7F in some conversion cases?
			if(fUseDefaultChar && *pstrWC > 0xFF)
			{
				if(pfUsedDef)
				{
					*pfUsedDef = TRUE;
				}
				*pstrMB = chDefault;
			}
			else
			{
				*pstrMB = (BYTE)*pstrWC;
			}
			pstrMB++;
			pstrWC++;
		}

		if(cchWC < 0 && cch < cchMB)
		{
			*pstrMB = 0;
			cch++;
		}
	}
	return cch;
}

/*
 *	CharSetIndexFromChar (ch)
 *
 *	@mfunc
 *		returns index into CharSet/CodePage table rgCpgChar corresponding
 *		to the Unicode character ch provided such an assignment is
 *		reasonably unambiguous, that is, the currently assigned Unicode
 *		characters in various ranges have Windows code-page equivalents.
 *		Ambiguous or impossible assignments return UNKNOWN_INDEX, which
 *		means that the character can only be represented by Unicode in this
 *		simple model.  Note that both UNKNOWN_INDEX and HAN_INDEX are negative
 *		values, i.e., they imply further processing to figure out what (if
 *		any) charset index to use.  Other indices may also require run
 *		processing, such as the blank in BiDi text.  We need to mark our
 *		right-to-left runs with an Arabic or Hebrew char set, while we mark
 *		left-to-right runs with a left-to-right char set.
 *
 *	@rdesc
 *		returns CharSet index
 */
 LONG CW32System::CharSetIndexFromChar(
	TCHAR ch)		// Unicode character to examine
{
	if(ch < 256)
		return ANSI_INDEX;

	if(ch < 0x700)
	{
		if(ch >= 0x600)
			return ARABIC_INDEX;

		if(ch > 0x590)
			return HEBREW_INDEX;

		if(ch < 0x500)
		{
			if(ch >= 0x400)
				return RUSSIAN_INDEX;

			if(ch >= 0x370)
				return GREEK_INDEX;
		}
	}
	else if (ch < 0xAC00)
	{
		if(ch >= 0x3400)				// CJK Ideographs
			return HAN_INDEX;

		if(ch > 0x3040 && ch < 0x3100)	// Katakana and Hiragana
			return SHIFTJIS_INDEX;

		if(ch < 0xe80 && ch >= 0xe00)	// Thai
			return THAI_INDEX;
	}
	else if (ch < 0xD800)
		return HANGUL_INDEX;

	else if (ch > 0xff00)
	{
		if(ch < 0xff65)					// Fullwidth ASCII and halfwidth
			return HAN_INDEX;			//  CJK punctuation

		if(ch < 0xffA0)					// Halfwidth Katakana
			return SHIFTJIS_INDEX;

		if(ch < 0xffe0)					// Halfwidth Jamo
			return HANGUL_INDEX;

		if(ch < 0xffef)					// Fullwidth punctuation and currency
			return HAN_INDEX;			//  signs; halfwidth forms, arrows
	}									//  and shapes

	return UNKNOWN_INDEX;
}

#ifdef DEBUG
void CW32System::AssertFn(BOOL f, LPSTR, LPSTR, int)
{
	static BOOL fDoit = TRUE;
	if (!f && fDoit)
	{
		int res = MessageBox(NULL, TEXT("Abort, Ignore, Ignore all"), TEXT("Assert"), MB_YESNOCANCEL);
		switch (res)
		{
		case IDYES :
			DebugBreak();
			break;
		case IDNO :
			return;
		case IDCANCEL :
			fDoit = FALSE;
			break;
		}
	}
}

void CW32System::sprintf (CHAR * buff, char *fmt, ...)
{
	#pragma message("Review JMO : Finish this")
}

static BOOL fTracing = FALSE; //FALSE;

void CW32System::TraceOn( void )
{
	fTracing = TRUE;
}

void CW32System::TraceOff( void )
{
	fTracing = FALSE;
}

void CW32System::TraceMsg(WCHAR *ptext)
{
	static int * mark = NULL;
	int local;

	if (mark == NULL)
	{
		mark = &local;
	}
	if (mark - &local > 10000)
	{
		OutputDebugString(TEXT("Stack usage too large"));
	}

	if (fTracing)
	{
		OutputDebugString(ptext);
		OutputDebugString(TEXT("\n"));
	}
}
#endif

