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
 *	@module RTFLEX.CPP - RichEdit RTF reader lexical analyzer |
 *
 *		This file contains the implementation of the lexical analyzer part of
 *		the RTF reader.
 *
 *	Authors: <nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent <nl>
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szUnicode.
 */

#include "_common.h"
#include "_rtfread.h"
#include "hash.h"

ASSERTDATA

#include "tokens.h"

#pragma BEGIN_CODESPACE_DATA

// Array used by character classification macros to speed classification
// of chars residing in two or more discontiguous ranges, e.g., alphanumeric
// or hex.  The alphabetics used in RTF control words are lower-case ASCII.
// *** DO NOT DBCS rgbCharClass[] ***

#define	fCS		fCT + fSP
#define fSB		fBL + fSP
#define fHD		fHX + fDG
#define	fHU		fHX + fUC
#define	fHL		fHX + fLC

const BYTE rgbCharClass[256] =
{
	fCT,fCT,fCT,fCT,fCT,fCT,fCT,fCT, fCT,fCS,fCS,fCS,fCS,fCS,fCT,fCT,
	fCT,fCT,fCT,fCT,fCT,fCT,fCT,fCT, fCT,fCT,fCT,fCT,fCT,fCT,fCT,fCT,
	fSB,fPN,fPN,fPN,fPN,fPN,fPN,fPN, fPN,fPN,fPN,fPN,fPN,fPN,fPN,fPN,
	fHD,fHD,fHD,fHD,fHD,fHD,fHD,fHD, fHD,fHD,fPN,fPN,fPN,fPN,fPN,fPN,

	fPN,fHU,fHU,fHU,fHU,fHU,fHU,fUC, fUC,fUC,fUC,fUC,fUC,fUC,fUC,fUC,
	fUC,fUC,fUC,fUC,fUC,fUC,fUC,fUC, fUC,fUC,fUC,fPN,fPN,fPN,fPN,fPN,
	fPN,fHL,fHL,fHL,fHL,fHL,fHL,fLC, fLC,fLC,fLC,fLC,fLC,fLC,fLC,fLC,
	fLC,fLC,fLC,fLC,fLC,fLC,fLC,fLC, fLC,fLC,fLC,fPN,fPN,fPN,fPN,fPN,

	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
};

const char szRTFSig[] = "{\\rtf";

// Specifies the number of bytes we can safely "UngetChar"
// before possibly underflowing the buffer.
const int cbBackupMax = 4;

#pragma END_CODESPACE_DATA

// Bug2298 - I found an RTF writer which emits uppercase RTF keywords,
// 			so I had to change IsLCAscii to IsAlphaChar for use in scanning
//			for RTF keywords.
inline BOOL IsAlphaChar(BYTE b)
{
	return ((DWORD)((b) - 'a') <= (DWORD)('z' - 'a')) ||
			((DWORD)((b) - 'A') <= (DWORD)('Z' - 'A'));
}

// Quick and dirty tolower(b)
inline BYTE REToLower(BYTE b)
{
	Assert(!b || IsAlphaChar(b));
	return b ? (b | 0x20) : b;
}


/*
 *	CRTFRead::InitLex()
 *
 *	@mfunc
 *		Initialize the lexical analyzer. Reset the variables. if reading in
 *		from resource file, sort the keyword list (). Uses global hinstRE
 *		from the RichEdit to find out where its resources are.  Note: in
 *		RichEdit 2.0, currently the resource option is not supported.
 *
 *	@rdesc
 *		TRUE				If lexical analyzer was initialized
 */
BOOL CRTFRead::InitLex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::InitLex");

	AssertSz(cKeywords == i_TokenIndexMax,
		"Keyword index enumeration is incompatible with rgKeyword[]");
	Assert(!_szText && !_pchRTFBuffer);

	// Allocate our buffers with an extra byte for szText so that hex
	// conversion doesn't have to worry about running off the end if the
	// first char is NULL
	if ((_szText	   = (BYTE *)PvAlloc(cachTextMax + 1, GMEM_ZEROINIT)) &&
		(_pchRTFBuffer = (BYTE *)PvAlloc(cachBufferMost, GMEM_ZEROINIT)))
	{
		return TRUE;					// Signal that lexer is initialized
	}

	_ped->GetCallMgr()->SetOutOfMemory();
	_ecParseError = ecLexInitFailed;
	return FALSE;
}

/*
 *	CRTFRead::DeinitLex()
 *
 *	@mfunc
 *		Shut down lexical analyzer
 */
void CRTFRead::DeinitLex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::DeinitLex");

#ifdef KEYWORD_RESOURCE
	if (hglbKeywords)
	{
		FreeResource(hglbKeywords);
		hglbKeywords = NULL;
		rgKeyword = NULL;
	}
#endif

	FreePv(_szText);
	FreePv(_pchRTFBuffer);
}

/*
 *	CRTFRead::GetChar()
 *	
 *	@mfunc
 *		Get next char, filling buffer as needed
 *	
 *	@rdesc
 *		BYTE			nonzero char value if success; else 0
 */
BYTE CRTFRead::GetChar()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetChar");

	if (_pchRTFCurrent == _pchRTFEnd && !FillBuffer())
	{
		_ecParseError = ecUnexpectedEOF;
		return 0;
	}
	return *_pchRTFCurrent++;
}

/*
 *	CRTFRead::FillBuffer()
 *
 *	@mfunc
 *		Fill RTF buffer & return != 0 if successful
 *
 *	@rdesc
 *		LONG			# chars read
 *
 *	@comm
 *		This routine doesn't bother copying anything down if
 *		pchRTFCurrent <lt> pchRTFEnd so anything not read yet is lost.
 *		The only exception to this is that it always copies down the
 *		last two bytes read so that UngetChar() will work. ReadData()
 *		actually counts on this behavior, so if you change it, change
 *		ReadData() accordingly.
 */
LONG CRTFRead::FillBuffer()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FillBuffer");

	LONG cchRead;

	if (!_pchRTFCurrent)				
	{									
		// No data yet, nothing for backup
		// Leave cbBackupMax NULL chars so backup
		// area of buffer doesn't contain garbage.

		for(int i = 0; i < cbBackupMax; i++)
		{
			_pchRTFBuffer[i] = 0;
		}
	}
	else
	{
		Assert(_pchRTFCurrent == _pchRTFEnd);

		// Copy most recently read chars in case
		//  we need to back up

		int cbBackup = min(cbBackupMax, 
				DiffPtrs(_pchRTFCurrent, &_pchRTFBuffer[cbBackupMax], BYTE)); 
		int i;

		for(i = -1; i >= -cbBackup; i--)
		{
			_pchRTFBuffer[cbBackupMax + i] = _pchRTFCurrent[i];
		}

		if(cbBackup < cbBackupMax)
		{
			// NULL the before the first valid character in the backup buffer
			_pchRTFBuffer[cbBackupMax + i] = 0;
		}
	}
	_pchRTFCurrent = &_pchRTFBuffer[cbBackupMax];

	// Fill buffer with as much as we can take given our starting offset
	_pes->dwError = _pes->pfnCallback(_pes->dwCookie,
									  _pchRTFCurrent,
									  cachBufferMost - cbBackupMax,
									  &cchRead);
	if (_pes->dwError)
	{
		TRACEERRSZSC("RTFLEX: GetChar()", _pes->dwError);
		_ecParseError = ecGeneralFailure;
		return 0;
	}

	_pchRTFEnd = &_pchRTFBuffer[cbBackupMax + cchRead];		// Point the end

#if defined(DEBUG) && !defined(MACPORT)
	if(_hfileCapture)
	{
		DWORD cbLeftToWrite = cchRead;
		DWORD cbWritten = 0;
		BYTE *pbToWrite = (BYTE *)_pchRTFCurrent;
		
		while(WriteFile(_hfileCapture,
						pbToWrite,
						cbLeftToWrite,
						&cbWritten,
						NULL) && 
						(pbToWrite += cbWritten,
						(cbLeftToWrite -= cbWritten)));
	}
#endif

	return cchRead;
}

/*
 *	CRTFRead::UngetChar()
 *
 *	@mfunc
 *		Bump our file pointer back one char
 *
 *	@rdesc
 *		BOOL				TRUE on success
 *
 *	@comm
 *		You can safely UngetChar _at most_ cbBackupMax times without
 *		error.
 */
BOOL CRTFRead::UngetChar()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::UngetChar");

	if (_pchRTFCurrent == _pchRTFBuffer || !_pchRTFCurrent)
	{
		Assert(0);
		_ecParseError = ecUnGetCharFailed;
		return FALSE;
	}

	--_pchRTFCurrent;
	return TRUE;
}

/*
 *	CRTFRead::UngetChar(cch)
 *
 *	@mfunc
 *		Bump our file pointer back 'cch' chars
 *
 *	@rdesc
 *		BOOL				TRUE on success
 *
 *	@comm
 *		You can safely UngetChar _at most_ cbBackupMax times without
 *		error.
 */
BOOL CRTFRead::UngetChar(UINT cch)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::UngetChar");

	AssertSz(cch <= cbBackupMax, "CRTFRead::UngetChar():  Number of UngetChar's "
								"exceeds size of backup buffer.");

	while(cch-- > 0)
	{
		if(!UngetChar())
			return FALSE;
	}

	return TRUE;
}

/*
 *	CRTFRead::GetHex()
 *
 *	@mfunc
 *		Get next char if hex and return hex value
 *		If not hex, leave char in buffer and return 255
 *
 *	@rdesc
 *		BYTE			hex value of GetChar() if hex; else 255
 */
BYTE CRTFRead::GetHex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetHex");

	BYTE ch = GetChar();

	if (IsXDigit(ch))
		return (ch <= '9') ? ch - '0' : (ch & 0x4f) - 'A' + 10;
	if (ch)
		UngetChar();
	return 255;
}

/*
 *	CRTFRead::GetHexSkipCRLF()
 *
 *	@mfunc
 *		Get next char if hex and return hex value
 *		If not hex, leave char in buffer and return 255
 *
 *	@rdesc
 *		BYTE			hex value of GetChar() if hex; else 255
 *
 *	@devnote
 *		Keep this in sync with GetHex above.
 */
BYTE CRTFRead::GetHexSkipCRLF()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::GetHexSkipCRLF");

	BYTE ch = GetChar();

	// skip \r \n
	while(ch == CR || ch == LF)
	{
		ch = GetChar(); 
	}

	if (IsXDigit(ch))
		return (ch <= '9') ? ch - '0' : (ch & 0x4f) - 'A' + 10;
	if (ch)
		UngetChar();
	return 255;
}

/*
 *	CRTFRead::TokenGetHex()
 *
 *	@mfunc
 *		Get an 8 bit character saved as a 2 hex digit value
 *
 *	@rdesc
 *		TOKEN			value of hex number read in
 */
TOKEN CRTFRead::TokenGetHex()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetHex");

	BYTE bChar0 = GetHex();
	BYTE bChar1;

	if(bChar0 < 16 && (bChar1 = GetHex()) < 16)
		_token = bChar0 << 4 | bChar1;
	else
		_token = tokenError;

	return _token;
}

/*
 *	CRTFRead::SkipToEndOfGroup()
 *
 *	@mfunc
 *		Skip to end of current group
 *
 *	@rdesc
 *		EC				An error code
 */
EC CRTFRead::SkipToEndOfGroup()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::SkipToEndOfGroup");

	INT		nDepth = 1;
	BYTE	ach;

	while(TRUE)
	{
		ach = GetChar();
		switch(ach)
		{
			case BSLASH:
			{
				BYTE achNext = GetChar();

				// EOF: goto done; else ignore NULLs
				if(!achNext && _ecParseError == ecUnexpectedEOF)
					goto done;

				if(achNext == 'b' && UngetChar() && 
					TokenGetKeyword() == tokenBinaryData)
				{
					// We've encountered the \binN tag in the RTF we want
					//	to skip.  _iParam contains N from \binN once the 
					// 	tag is parsed by TokenGetKeyword()
					SkipBinaryData(_iParam);
				}
				break;
			}

			case LBRACE:
				nDepth++;
				break;

			case RBRACE:
				if (--nDepth <= 0)
					goto done;
				break;

			case 0:
				if(_ecParseError == ecUnexpectedEOF)
					goto done;

#ifdef PWD_JUPITER
			default:

				// GuyBark JupiterJ 50034: Detect Lead bytes here.
				if(IsLeadByte(ach, _nCodePage))
				{
					// Get the trailing byte and ignore it.
					ach = GetChar();

					// Extra checkjust in case we hit the end of the file.
					if(ach == 0)
					{
					    if(_ecParseError == ecUnexpectedEOF)
					        goto done;
					}

				}

				break;
#endif // PWD_JUPITER                
		}
	} 

	Assert(!_ecParseError);
	_ecParseError = ecUnexpectedEOF;

done:
	return _ecParseError;
}

/*
 *	CRTFRead::TokenFindKeyword(szKeyword)
 *
 *	@mfunc
 *		Find keyword <p szKeyword> and return its token value
 *
 *	@rdesc
 *		TOKEN			token number of keyword
 */
TOKEN CRTFRead::TokenFindKeyword(
	BYTE *	szKeyword)			// @parm Keyword to find
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenFindKeyword");

	INT				iMin;
	INT				iMax;
	INT				iMid;
	INT				nComp;
	BYTE *			pchCandidate;
	BYTE *			pchKeyword;
	const KEYWORD *	pk;

	AssertSz(szKeyword[0],
		"CRTFRead::TokenFindKeyword: null keyword");

#ifdef RTF_HASHCACHE
	if ( _rtfHashInited )
	{
		// Hash is 23% faster than the following binary search on finds
		//  and 55% faster on misses: For 97 words stored in a 257 cache.
		//  Performance numbers will change when the total stored goes up.
		pk = HashKeyword_Fetch ( (CHAR *) szKeyword );
	}
	else
#endif
	{
		iMin = 0;
		iMax = cKeywords - 1;
		pk = NULL;
		do				// Note (MS3): Hash would be quicker than binary search
		{
			iMid		 = (iMin + iMax) / 2;
			pchCandidate = (BYTE *)rgKeyword[iMid].szKeyword;
			pchKeyword	 = szKeyword;
			while (!(nComp = REToLower(*pchKeyword) - *pchCandidate)	// Be sure to match
				&& *pchKeyword)											//  terminating 0's
			{
				pchKeyword++;
				pchCandidate++;
			}
			if (nComp < 0)
				iMax = iMid - 1;
			else if (nComp)
				iMin = iMid + 1;
			else
			{
				pk = &rgKeyword[iMid];
				break;
			}
		} while (iMin <= iMax);
	}


	if( pk )
	{
		_token = pk->token;
		
		// here, we log the RTF keyword scan to aid in tracking RTF tag ocverage
// TODO: Implement RTF tag logging for the Mac and WinCE
#if defined(DEBUG) && !defined(MACPORT) && !defined(PEGASUS)
		if(_prtflg) 
		{
#ifdef RTF_HASCACHE
			_prtflg->AddAt(szKeyword); 
#else
			_prtflg->AddAt((size_t)iMid);
#endif
		}
#endif
	}
	else
		_token = tokenUnknownKeyword;		// No match: TODO: place to take

	return _token;				 			//  care of unrecognized RTF
}

/*
 *	CRTFRead::TokenGetKeyword()
 *
 *	@mfunc
 *		Collect a keyword and its parameter. Return token's keyword
 *
 *	@rdesc
 *		TOKEN				token number of keyword
 *
 *	@comm
 *		Most RTF control words (keywords) consist of a span of lower-case
 *		ASCII letters possibly followed by a span of decimal digits. Other
 *		control words consist of a single character that isn't LC ASCII. No
 *		control words contain upper-case characters.
 */
TOKEN CRTFRead::TokenGetKeyword()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetKeyword");

	BYTE		ach = GetChar();
	BYTE *		pach;
	const BYTE * pachSymbol;
	SHORT		cachKeyword = 1;
	BYTE		szKeyword[cachKeywordMax];

	_szParam[0] = '\0';							// Clear parameter
	_iParam = 0;

	if(!IsAlphaChar(ach))							// Not alpha, i.e.,
	{											//  single char
		if (ach == '\'')						// Most common case needs
		{										//  special treatment
			// Convert hex to char and store result in _token
			if(TokenGetHex() == tokenError)
			{							
				_ecParseError = ecUnexpectedChar;
				goto TokenError;
			}
			if((_token == CR || _token == LF) && FInDocTextDest())
			{
				// Add raw CR or LF in the byte stream as a \par
				return tokenEndParagraph;
			}
		}
		else
		{
			pachSymbol = szSymbolKeywords;			// Check for other known
			while (ach != *pachSymbol && *pachSymbol)		//  symbols
				pachSymbol++;
			if (*pachSymbol)							// Found one
			{
				_token = tokenSymbol[pachSymbol - szSymbolKeywords];
				if(_token > 0x7F)				// Token or larger Unicode
					return _token;				//  value
			}
			else if (!ach)						// No more input chars
				goto TokenError;
			else								// Code for unrecognized RTF
				_token = ach;					// We'll just insert it for now 
		}
		_token = TokenGetText((BYTE)_token);
		return _token; 
	}

	szKeyword[0] = ach;							// Collect keyword that starts
	pach = szKeyword + 1;						// 	with lower-case ASCII
	while (cachKeyword < cachKeywordMax &&
		   IsAlphaChar(ach = GetChar()))
	{
		*pach++ = ach;
	}
	*pach = '\0';								// Terminate keyword

	if (cachKeyword == cachKeywordMax)
	{
		_ecParseError = ecKeywordTooLong;
		goto TokenError;
	}

	if (IsDigit(ach) || ach == '-')				// Collect parameter
	{
              int cch = 0;  
              pach = _szParam;
		*pach++ = ach;
              cch++;
		if(ach != '-')
			_iParam = ach - '0';				// Get parameter value

		while ((IsDigit(ach = GetChar())) && (cch < cachParamMax))
		{
			_iParam = _iParam*10 + ach - '0';
			*pach++ = ach;
                     cch++;
		}
              *pach = '\0';							// Terminate parameter string
		if (_szParam[0] == '-')
			_iParam = -_iParam;
	}

	if (!_ecParseError &&						// We overshot:
		(ach == ' ' || UngetChar()))			//  if not ' ', unget char
			return TokenFindKeyword(szKeyword);	// Find and return keyword

TokenError:
	TRACEERRSZSC("TokenGetKeyword()", _ecParseError);
	return _token = tokenError;
}

/*
 *	CRTFRead::TokenGetText(ach)
 *
 *	@mfunc
 *		Collect a string of text starting with the char <p ach> and treat as a
 *		single token. The string ends when a LBRACE, RBRACE, or single '\\' is found.
 *
 *	@devnote
 *		We peek past the '\\' for \\'xx, which we decode and keep on going;
 *		else we return in a state where the next character is the '\\'.
 *
 *	@rdesc
 *		TOKEN			Token number of next token (tokenText or tokenError)
 */
TOKEN CRTFRead::TokenGetText(
	BYTE ach)				// @parm First char of 8-bit text string
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetText");

	BYTE *	pach = _szText;
	SHORT	cachText = 0;
	LONG	CodePage = _pstateStackTop->nCodePage;
	BOOL	fAllASCII = TRUE;
	int     cTrailBytesNeeded = 0;

	// GuyBark JupiterJ 11/16/98
	// This routine's old use of "fLeadByte" has been replaced with the current
	// RichEdit 3.0 use of "cTrailBytesNeeded". This came to light when I 
	// investigated 50477, which happened when an attempt to paste UTF8 failed.
	// It turns out the old routine couldn't handle UTF8 as that's not DBCS.
	// Instead it can have multiple trail bytes.

	_token = tokenError;						// Default error

	// FUTURE(BradO):  This 'goto' into a while loop is pretty weak.
	//	Restructure this 'while' loop such that the 'goto' is removed.

	// add the character passed into the routine
	goto add;

	// If cTrailBytesNeeded is non-zero, we need to get all the trail bytes.  Otherwise,
	// a string end in the middle of a DBC or UTF-8 will cause bad display/print problem
	// - 5 to allow extra space for up to 4 bytes for UTF-8 and Null char
	while (cachText < cachTextMax - 5 || cTrailBytesNeeded)
	{
		ach = GetChar();
		switch (ach)
		{
			case BSLASH:
			{
				// FUTURE(BradO):  This code looks ALOT like TokenGetKeyword.
				//	We should combine the two into a common routine.

				BYTE achNext;

				// Get char after BSLASH
				if(!(achNext = GetChar()))
				{
					goto error;
				}
	
				if(achNext == '\'')					// Handle most frequent
				{									//  case here
					if(TokenGetHex() == tokenError)
					{
						if(cTrailBytesNeeded)
						{
							// The trail-byte must be a raw BSLASH.
							// Unget the single-quote.

							if(!UngetChar())
							{
								goto error;
							}
							// fall through to add BSLASH
						}
						else
						{
							_ecParseError = ecUnexpectedChar;
							goto error;
						}
					}
					else
					{
						ach = (BYTE)_token;

						if((cTrailBytesNeeded == 0) && (ach == CR || ach == LF) &&
							FInDocTextDest())
						{
							// Here, we have a raw CR or LF in document text.  
							// Unget the whole lot of characters and bail out.  
							// TokenGetKeyword will convert this CR or LF into
							// a \par.

							if(!UngetChar(4))
							{
								goto error;
							}
							goto done;
						}
					}
					goto add;
				}

				// If we are not expecting a trail byte, then
				// check next byte against list of RTF symbol

				// GuyBark JupiterJ 50478:
				// Forget about only doing this if we're not expecting a trail byte.
				// Say we have a "\{" as the trail byte of a DBCS character. Before,
				// if we were trying to find the trail byte, we'd simply pick up the
				// backslash and use that. The subsequent '{' would then start the 
				// next group, and everything fails. This RTF really means "the trail 
				// byte has the value of the '{' character". By always executing the
				// following code we interpret the RTF as it's meant to be.

				const BYTE *pachSymbol;

				// Check for other known symbols
				pachSymbol = szSymbolKeywords;			

				while(achNext != *pachSymbol && *pachSymbol)	
				{
					pachSymbol++;
				}

				TOKEN tokenTmp;

				if(*pachSymbol && 
					(tokenTmp = tokenSymbol[pachSymbol - szSymbolKeywords])
						 <= 0x7F)
				{
					ach = tokenTmp;
					goto add;
				}

				// We will want to unget the byte following the BSLASH
				if(!UngetChar())
				{
					goto error;
				}
	
				// GuyBark JupiterJ 50203:
				// The following test has been made tighter with the checking of
				// IsAlphaChar() here, (lifted from RichEdit3). This is to trap
				// hitting RTF tokens when we were expecting the trail byte of a 
				// DBCS character. For example, we can get RTF that contains this
				// in the middle of a FE text run...
				//
				// 0x81\plain
				//
				// 0x81 is a lead byte so we're expecting a trail byte to follow.
				// Previously we'd pick up the backslash as the trail byte, and then
				// add "plain" to the main text run. In fact, this RTF is bad, but
				// we can get it anyway. What we now do is consider 0x81 to be an
				// SBCS extended character, and then the "\plain" is treated as the
				// RTF token it is.

				if(cTrailBytesNeeded && !IsAlphaChar(achNext))
				{
					// this BSLASH is a raw BSLASH which is the trail 
					// byte for a DBCS character.
					// add the raw BSLASH
					goto add;					
				}

				// Here, my guess is that the BSLASH begins the next RTF 
				// keyword, so unget the BSLASH
			    if(!UngetChar())
				{
					goto error;					
				}
				goto done;
			}

			case LBRACE:						// End of text string
			case RBRACE:
				if(cTrailBytesNeeded)
				{
					// Previous char was a lead-byte of a DBCS pair, which makes
					// this char a raw trail-byte.
					goto add;
				}

				if(!UngetChar())				// Unget delimeter
				{
					goto error;
				}
				goto done;

			case LF:							// Throw away noise chars
			case CR:
				break;

			case 0:
				if(_ecParseError == ecUnexpectedEOF)
					goto done;
				ach = ' ';						// Replace NULL by blank

			default:							// Collect chars
add:
				// outstanding chars to be skipped after \uN tag
				if(_cbSkipForUnicode)
				{
					_cbSkipForUnicode--;
					continue;
				}

				*pach++ = ach;

				++cachText;

				if(ach > 0x7F)
				{
					fAllASCII = FALSE;
				}
	
				// GuyBark JupiterJ 50477:
				// RichEdit 3.0 now calls a new routine here to determine if
				// this is a lead byte. But with the exception of UTF8, we
				// already determine this ok with the call to IsLeadByte()
				// below. But for UTF8 we must take special action here.

				// Check if we are expecting more trail bytes
				if (cTrailBytesNeeded)
				{
				    cTrailBytesNeeded--;
				}
				else
				{
				    // Not already processing trail bytes. Is this UTF8?
				    if(CodePage == CP_UTF8)
				    {
				        // Yes. Take exactly the same steps here for UTF8 as the new 
				        // RichEdit 3.0 does. The maximum trail byte count we use here 
				        // is 3, even though in theory it could go up to 5. I queried 
				        // this with Murray Sargent who said that the number is 3 for 
				        // the foreseeable future (covers all the Unicode surrogates).

				        if (ach >= 0x0F0)
				        {
				            cTrailBytesNeeded = 3;
				        }
				        else if (ach >= 0x0E0)
				        {
				            cTrailBytesNeeded = 2;
				        }
				        else if (ach >= 0x0C0)
				        {
				            cTrailBytesNeeded = 1;
				        }
				        else
				        {
				            cTrailBytesNeeded = 0;
				        }
				    }
				    else
				    {
				        // This is not UTF8, so IsLeadByte() will do nicely.
				        cTrailBytesNeeded = (IsLeadByte(ach, CodePage) ? 1 : 0);
				    }
				}

				Assert(cTrailBytesNeeded >= 0);
    		}
	}

done:
	_token = fAllASCII ? tokenASCIIText : tokenText;
	*pach = '\0';								// Terminate token string

error:
	return _token;
}
 
/*
 *	CRTFRead::TokenGetToken()
 *
 *	@mfunc
 *		This function reads in next token from input stream
 *
 *	@rdesc
 *		TOKEN				token number of next token
 */
TOKEN CRTFRead::TokenGetToken()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::TokenGetToken");

	BYTE		ach;

	_tokenLast	= _token;					// Used by \* destinations and FE
	_token = tokenEOF;

SkipNoise:
	ach = GetChar();
	switch (ach)
	{
	case CR:
	case LF:
		goto SkipNoise;

	case LBRACE:
		_token = tokenStartGroup;
		break;

	case RBRACE:
		_token = tokenEndGroup;
		break;

	case BSLASH:
		_token = TokenGetKeyword();
		break;

	case 0:									
		if(_ecParseError == ecUnexpectedEOF)
			break;
		ach = ' ';							// Replace NULL by blank
											// Fall thru to default
	default:
		if( !_pstateStackTop )
		{
			TRACEWARNSZ("Unexpected token in rtf file");
			Assert(_token == tokenEOF);
		}
		else if (_pstateStackTop->sDest == destObjectData || 
				 _pstateStackTop->sDest == destPicture )
		// not text but data
		{
			_token = tokenObjectDataValue + _pstateStackTop->sDest - destObjectData;
			UngetChar();
		}
		else
			_token = TokenGetText(ach);
	}
	return _token;
}


/*
 *	CRTFRead::FInDocTextDest()
 *
 *	@mfunc
 *		Returns a BOOL indicating if the current destination is one in which
 *		we would encounter document text.
 *
 *	@rdesc
 *		BOOL	indicates the current destination may contain document text.
 */
BOOL CRTFRead::FInDocTextDest() const
{
	switch(_pstateStackTop->sDest)
	{
		case destRTF:
		case destField:
		case destFieldResult:
		case destFieldInstruction:
		case destParaNumbering:
		case destParaNumText:
		case destNULL:
			return TRUE;

		case destFontTable:
		case destRealFontName:
		case destObjectClass:
		case destObjectName:
		case destFollowingPunct:
		case destLeadingPunct:
		case destColorTable:
		case destBinary:
		case destObject:
		case destObjectData:
		case destPicture:
		case destDocumentArea:
			return FALSE;
	
		default:
			AssertSz(0, "CRTFRead::FInDocTextDest():  New destination "
							"encountered - update enum in _rtfread.h");
			return TRUE;
	}
}

