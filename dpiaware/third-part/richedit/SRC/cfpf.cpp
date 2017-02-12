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
 *	@module	CFPF.C -- -- RichEdit CCharFormat and CParaFormat Classes |
 *
 *	Created: <nl>
 *		9/1995 -- Murray Sargent <nl>
 *
 *	@devnote
 *		The this ptr for all methods points to an internal format class, i.e.,
 *		either a CCharFormat or a CParaFormat, which uses the cbSize field as
 *		a reference count.  The pCF or pPF argument points at an external
 *		CCharFormat or CParaFormat class, that is, pCF->cbSize and pPF->cbSize
 *		give the size of their structure.  The code still assumes that both
 *		internal and external forms are derived from the CHARFORMAT(2) and
 *		PARAFORMAT(2) API structures, so some redesign would be necessary to
 *		obtain a more space-efficient internal form.
 *
 */

#include "_common.h"
#include "_array.h"					// for fumemmov()
#include "_rtfconv.h"				// for IsCharSetValid()
#include "_w32sys.h"
#include "_cfpf.h"


ASSERTDATA


// Table of formatting info for Normal and Heading styles
const STYLEFORMAT g_Style[] =		// {dwEffects; yHeight}
{							// Measurements in points
	{CFE_BOLD,				14},	// Heading 1
	{CFE_BOLD + CFE_ITALIC,	12},	// Heading 2
	{0,						12},	// Heading 3
	{CFE_BOLD,				12},	// Heading 4
	{0,						11},	// Heading 5
	{CFE_ITALIC,			11},	// Heading 6
	{0,						 0},	// Heading 7
	{CFE_ITALIC,			 0},	// Heading 8
	{CFE_BOLD + CFE_ITALIC,	 9}		// Heading 9
};

// GuyBark JupiterJ: Offsets for the characters that appear in ordered Katakana lists.
const BYTE rgKatakanaCharsAIUEO[] =
{
     0,  2,  4,  6,  8,
     9, 11, 13, 15, 17,
    19, 21, 23, 25, 27,
    29, 31, 34, 36, 38, 
    40, 41, 42, 43, 44,
    45, 48, 51, 54, 57,
    60, 61, 62, 63, 64,
    66,     68,     70, 
    71, 72, 73, 74, 75,
    77,             80,
    81
};

#define KATAKANACHARSAIUEOCNT (sizeof(rgKatakanaCharsAIUEO) / sizeof(BYTE))

// Offsets for the characters that appear in poem-based Katakana lists.
const BYTE rgKatakanaCharsIROHA[] =
{
     2, 75, 45, 41, 57,
    54, 38, 31, 72, 42,
    73, 80, 77,  9, 70,
    29, 74, 27, 34, 43,
    40, 71, 62,  4, 78, 
    44,  8, 13, 66, 60, 
    15, 51, 17,  6, 36,  
     0, 19, 11, 68, 63, 
    61, 21, 79, 48, 64,
    25, 23, 81
};

#define KATAKANACHARSIROHACNT (sizeof(rgKatakanaCharsIROHA) / sizeof(BYTE))

//------------------------- CCharFormat Class -----------------------------------

CCharFormat::CCharFormat()
{
	cbSize = sizeof(CHARFORMAT2);
	wInternalFlags = 0;
}

/*
 *	CCharFormat::Apply(pCF)
 *
 *	@mfunc
 *		Apply *<p pCF> to this CCharFormat as specified by nonzero bits in
 *		<p pCF>->dwMask
 *
 *	@devnote
 *		Autocolor is dealt with through a neat little hack made possible
 *		by the choice CFE_AUTOCOLOR = CFM_COLOR (see richedit.h).  Hence
 *		if <p pCF>->dwMask specifies color, it automatically resets autocolor
 *		provided (<p pCF>->dwEffects & CFE_AUTOCOLOR) is zero.
 *
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. But
 *		this CCharFormat is internal and cbSize is used as a reference count.
 */
HRESULT CCharFormat::Apply (
	const CCharFormat *pCF,		//@parm	CCharFormat to apply to this CF
	BOOL bInOurHost)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Apply");

	DWORD dwEffectMask;
	const DWORD dwMaskApply	 = pCF->dwMask;
	BOOL  fCF;

	if(pCF->cbSize == sizeof(CHARFORMAT))
	{
		fCF = TRUE;	 
		dwEffectMask = dwMaskApply & CFM_EFFECTS;
		// or'ing in CFM_DISABLED in this manner is incredibly lame
		// and prone to compatibility problems.  However, the Forms^3
		// team has decided _not_ to use CharFormat2's even though
		// they explicitly asked for the feature in the first place.
		// go figure.
		if (!bInOurHost)
			dwEffectMask |= CFM_DISABLED;		
	}
	else
	{
		fCF = FALSE;
		dwEffectMask = dwMaskApply & CFM_EFFECTS2;
	}

	// Reset effect bits to be modified and OR in supplied values
	dwEffects &= ~dwEffectMask;
	dwEffects |= pCF->dwEffects & dwEffectMask;

	// wWeight is always used; so if a 1.0 CHARFORMAT tries to 
	// set BOLD/NOT BOLD, reset weight to be the appropriate value.
	// Also, if CFM_BOLD is specified, it completely overrides
	// any existing weight.
	if(dwMaskApply & CFM_BOLD)
		wWeight = (pCF->dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;

	if(dwMaskApply & CFM_COLOR)
		crTextColor = pCF->crTextColor;

	if(dwMaskApply & ~CFM_EFFECTS)				// Early out if only dwEffects
	{											//  is modified. Note that
		if(dwMaskApply & CFM_SIZE)				//  CFM_EFFECTS includes CFM_COLOR
		{
			// If high word of input height is 0x8000, low word is signed
			// increment in points
			yHeight = HIWORD(pCF->yHeight) == 0x8000
					? GetUsableFontHeight(yHeight, (SHORT)pCF->yHeight)
					: pCF->yHeight;
		}

		if(dwMaskApply & CFM_OFFSET)
			yOffset = pCF->yOffset;

		if(dwMaskApply & CFM_CHARSET)
		{
			bCharSet = pCF->bCharSet;
			if(!IsCharSetValid(bCharSet))		// Char set not valid, so set 
			{									//  it to something sensible
				bCharSet = DEFAULT_CHARSET;
			}
		}

		if(dwMaskApply & CFM_FACE)
		{
			bPitchAndFamily = pCF->bPitchAndFamily;
			wcscpy_s(szFaceName, pCF->szFaceName);
		}

		if( fCF )									// CHARFORMAT
			dwEffects |= CFE_AUTOBACKCOLOR;			// Be sure autobackcolor

		else										// CHARFORMAT2 extensions
		{
			if((dwMaskApply & (CFM_WEIGHT | CFM_BOLD)) == CFM_WEIGHT) 
			{			
				wWeight			= pCF->wWeight;

				// Set bold to be the appropriate value. The choice here 
				// comes from VB4.0 help.  Basically, only high weight
				// values are bold
				dwEffects |= CFE_BOLD;
				if(wWeight < 551)
					dwEffects &= ~CFE_BOLD;
			}

			if(dwMaskApply & CFM_BACKCOLOR)
				crBackColor		= pCF->crBackColor;

			if(dwMaskApply & CFM_LCID)
				lcid			= pCF->lcid;

			if(dwMaskApply & CFM_SPACING)
				sSpacing		= pCF->sSpacing;

			if(dwMaskApply & CFM_KERNING)
				wKerning		= pCF->wKerning;

			if(dwMaskApply & CFM_STYLE)
				sStyle			= pCF->sStyle;

			if(dwMaskApply & CFM_UNDERLINETYPE)
			{
				bUnderlineType	= pCF->bUnderlineType;
				if(!(dwMaskApply & CFM_UNDERLINE))	// If CFE_UNDERLINE
				{									//  isn't defined,
					dwEffects	&= ~CFE_UNDERLINE;	//  set it according to
					if(bUnderlineType)				//  bUnderlineType
						dwEffects |= CFE_UNDERLINE;
				}
			}

			if((dwMaskApply & CFM_ANIMATION) && pCF->bAnimation <= 18)
				bAnimation		= pCF->bAnimation;

			if(dwMaskApply & CFM_REVAUTHOR)
				bRevAuthor		= pCF->bRevAuthor;
		}
	}

	SetCRC();

	bInternalEffects &= ~pCF->bInternalMask;
	bInternalEffects |= pCF->bInternalEffects & pCF->bInternalMask;

	return NOERROR;
}

/*
 *	CCharFormat::ApplyDefaultStyle(pCF)
 *
 *	@mfunc	
 *		Set default style properties in this CCharFormat
 */
void CCharFormat::ApplyDefaultStyle (
	LONG Style)		//@parm Style to use
{
	Assert(IsKnownStyle(Style));

	if(IsHeadingStyle(Style))
	{
		LONG i = -Style + STYLE_HEADING_1;
              LONG count = sizeof(g_Style) / sizeof(g_Style[0]);

              if ((i >= count) || (i < 0))
                    return;
              
		dwEffects = (dwEffects & 0xFFFFFF00) | g_Style[i].bEffects;
		wWeight = (dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;

		if(g_Style[i].bHeight)
			yHeight = g_Style[i].bHeight * 20;
//Bug 917
		if(!bCharSet){
			bCharSet = ANSI_CHARSET;
			bPitchAndFamily = FF_SWISS;
			wcscpy_s(szFaceName, L"Arial");
		}
//
	}
}

/*
 *	CCharFormat::Compare(pCF)
 *
 *	@mfunc
 *		Compare this CCharFormat to *<p pCF>
 *
 *	@rdesc
 *		TRUE if they are the same
 *
 *	@devnote
 *		First compare 6 DWORDs of CCharFormat (dwEffects, yHeight, yOffset
 *		crTextColor, bCharSet, bPitchAndFamily, and first WORD of szFaceName).
 *		If they are identical, then compare the full szFaceName's.  If they
 *		too are identical, compare the CHARFORMAT2 extensions. For
 *		CHARFORMAT, the extension values are taken to equal 0.  Return
 *		TRUE only if all comparisons succeed.  See also Delta(), which NINCH's
 *		the pCF->dwMask bits for parameters that differ between the two
 *		CCharFormats.
 *
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. "This"
 *		CCharFormat is internal and cbSize is used as a reference count.
 */
BOOL CCharFormat::Compare (
	const CCharFormat *pCF) const	//@parm	CCharFormat to compare this
{									//  CCharFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Compare");

	BOOL	fCF2 = pCF->cbSize == sizeof(CHARFORMAT2);
	DWORD	i;
	DWORD *	p1 = (DWORD *)this + 2;			// Bypass cbSize & dwMask fields
	DWORD *	p2 = (DWORD *)pCF  + 2;

	for (i = 0; i < 6; i++)					// Compare first six DWORDs
	{										//  (which includes most often
		if(*p1++ != *p2++)					//  changed attributes, like
			return FALSE;					//  dwEffects)
	}
	if(wcscmp(szFaceName, pCF->szFaceName))	// Compare font facename
		return FALSE;

	/* Compare CHARFORMAT2 extras:
	 *		1. (wWeight, sSpacing)
	 *		2. crBackColor
	 *		3. lcid
	 *		4. dwReserved
	 *		5. (wKerning, sStyle)
	 *		6. (bUnderlineType, bAnimation, bRevAuthor, bRes)
	 *		7. dwRes2 to add (msgtest needs to recompile)
	 *
	 *		i.e., 7 DWORDs.  Leave it 6 for now...
	 */
#define	NCF_EXTRAS	6

	DWORD j;
	p1 = (DWORD *)&this->wWeight;
	p2 = (DWORD *)&pCF->wWeight;

	AssertSz(offsetof(CCharFormat, wWeight) == 4*7 + sizeof(TCHAR)*LF_FACESIZE,
		"CCharFormat::Compare: unpacked CCharFormat struct");
	AssertSz(sizeof(CHARFORMAT2) == 4*(7 + NCF_EXTRAS) + sizeof(TCHAR)*LF_FACESIZE,
		"CCharFormat::Compare: unexpected CCharFormat size");
	AssertSz(dwReserved == 0,
		"CCharFormat::Compare: nonzero dwReserved");

	for (i = j = 0; i < NCF_EXTRAS; i++)	// CHARFORMAT2 extensions
	{
		if(*p1++ != *p2++)
			return FALSE;
	}

	return bInternalEffects == pCF->bInternalEffects;
}

/*
 *	CCharFormat::Delta(pCF)
 *
 *	@mfunc
 *		Adjust pCF->dwMask for differences between this CCharformat and
 *		*<p pCF>
 *
 *	@devnote
 *		*<p pCF> is an external CCharFormat, i.e., it's either a CHARFORMAT
 *		or a CHARFORMAT2 with the appropriate size given by cbSize. But
 *		this CCharFormat is internal and cbSize is used as a reference count.
 */
void CCharFormat::Delta (
	CCharFormat *pCF) const			//@parm	CCharFormat to compare this
{									//  CCharFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Delta");
												// Collect bits for properties
	LONG	dwT = dwEffects ^ pCF->dwEffects;	//  that change. Note: auto
												//  color is handled since
	if(yHeight		!= pCF->yHeight)			//  CFM_COLOR = CFE_AUTOCOLOR
		dwT |= CFM_SIZE;

	if(yOffset		!= pCF->yOffset)
		dwT |= CFM_OFFSET;

	if(crTextColor	!= pCF->crTextColor)
		dwT |= CFM_COLOR;

	if(bCharSet		!= pCF->bCharSet)
		dwT |= CFM_CHARSET;

	if((pCF->dwMask & CFM_FACE) && wcscmp(szFaceName, pCF->szFaceName))
		dwT |= CFM_FACE;

	if(pCF->cbSize > sizeof(CHARFORMAT))
	{
		if(crBackColor	!= pCF->crBackColor)	// CHARFORMAT2 stuff
			dwT |= CFM_BACKCOLOR;

		if(wKerning		!= pCF->wKerning)
			dwT |= CFM_KERNING;

		if(lcid			!= pCF->lcid)
			dwT |= CFM_LCID;

		if(wWeight		!= pCF->wWeight)
			dwT |= CFM_WEIGHT;

		if(sSpacing		!= pCF->sSpacing)
			dwT |= CFM_SPACING;
	
		if(sStyle		!= pCF->sStyle)
			dwT |= CFM_STYLE;

		if(bUnderlineType != pCF->bUnderlineType)
			dwT |= CFM_UNDERLINETYPE;

		if(bAnimation	!= pCF->bAnimation)
			dwT |= CFM_ANIMATION;

		if(bRevAuthor	!= pCF->bRevAuthor)
			dwT |= CFM_REVAUTHOR;
	}
	pCF->dwMask &= ~dwT;						// Reset mask bits for
												//  properties that differ

	// now handle internal properties
	dwT = bInternalEffects ^ pCF->bInternalEffects;
	pCF->bInternalMask &= ~dwT;
}

/*
 *	CCharFormat::fSetStyle(pCF)
 *
 *	@mfunc
 *		return TRUE iff pCF specifies that the style should be set. See
 *		code for list of conditions for this to be true
 *
 *	@rdesc
 *		TRUE iff pCF specifies that the style sStyle should be set
 */
BOOL CCharFormat::fSetStyle() const
{
	return	dwMask != CFM_ALL2				&&
			dwMask &  CFM_STYLE				&&
			cbSize == sizeof(CHARFORMAT2)	&&
			IsKnownStyle(sStyle);
}

/*
 *	CCharFormat::Get(pCF)
 *
 *	@mfunc
 *		Copy this CCharFormat to *<p pCF>
 *
 *	@devnote
 *		*<p pCF> is an external CCharFormat, i.e., it represents either a
 *		CHARFORMAT or a CHARFORMAT2 with the appropriate size given by
 *		cbSize. But this CCharFormat is internal and cbSize is used as
 *		a reference count.
 */
void CCharFormat::Get (
	CCharFormat *pCF) const	 //@parm CCharFormat to copy this CCharFormat to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Get");

	UINT cb = pCF->cbSize;

	pCF->dwMask = CFM_ALL2;						// Default CHARFORMAT2
	if(cb != sizeof(CHARFORMAT2))				// It isn't
	{
		pCF->dwMask = CFM_ALL;					// Use CHARFORMAT
		ASSERT(cb == sizeof(CHARFORMAT));		// It better be a CHARFORMAT
	}

    // Bound the copy
    cb = min(cb, sizeof(CCharFormat));

	CopyFormat(pCF, this, cb);					// Copy this to pCF
	pCF->wInternalFlags = wInternalFlags;
}

/*
 *	CCharFormat::GetA(pCFA)
 *
 *	@mfunc
 *		Copy this UNICODE character format (including its dwMask) to an ANSI
 *		CHARFORMAT *pCFA with size given by pCFA->cbSize.
 *
 *	@rdesc
 *		TRUE if successful; else FALSE
 */
BOOL CCharFormat::GetA(CHARFORMATA *pCFA) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::GetA");

	if(!IsValidCharFormatA(pCFA))
		return FALSE;

	// Copy from dwMask up to szFaceName
	CopyMemory((BYTE *)pCFA + sizeof(DWORD), (BYTE *)this + sizeof(DWORD),
				(BYTE *)&szFaceName[0] - (BYTE *)&dwMask);

	if(bInternalEffects & CFEI_FACENAMEISDBCS)
	{
		// HACK:  The face name is actually DBCS stuffed into the unicode
		//			buffer, so simply un-stuff this DBCS into the ANSI string
		TCHAR *pchSrc = const_cast<TCHAR *>(szFaceName);
		char *pachDst = pCFA->szFaceName;
              UINT i=0;
              
		while(i<(LF_FACESIZE-1) && pchSrc[i])
		{
			pachDst[i] = pchSrc[i];
                    i++;
		}
             
              pachDst[i] = 0;
	}
	else 
	{
		// We have to use CP_ACP to convert font name.  We used to 
		// use GetLocaleCodePage() which would cause problem in FE system using
		// non-FE locale.  e.g. Win95Trad.Chinese with US English regional setting.
		MbcsFromUnicode( pCFA->szFaceName, LF_FACESIZE, szFaceName, -1, CP_ACP,
						UN_NOOBJECTS);
	}

	if(pCFA->cbSize	== sizeof(CHARFORMATA))
	{
		pCFA->dwEffects &= CFM_EFFECTS;		// We're done. Don't return more
		pCFA->dwMask	&= CFM_ALL;			//  info than requested (for
	}										//  backward compatibility)
	else
		CopyMemory(&((CHARFORMAT2A *)pCFA)->wWeight, &wWeight, CHARFORMATDELTA);

	return TRUE;
}

/*
 *	CCharFormat::InitDefault(hfont)
 *
 *	@mfunc	
 *		Initialize this CCharFormat with information coming from the font
 *		<p hfont>
 *	
 *	@rdesc
 *		HRESULT = (if success) ? NOERROR : E_FAIL
 */
HRESULT CCharFormat::InitDefault (
	HFONT hfont)		//@parm Handle to font info to use
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::InitDefault");

	LOGFONT lf;

	if(!IsValidCharFormat(this))
		return E_FAIL;

	ZeroMemory((LPBYTE)this + cbSkipFormat, cbSize - cbSkipFormat);
	bCRC	= 0;

	// If hfont isn't defined, get LOGFONT for default font
	if (!hfont)
		hfont = (HFONT)GetStockObject(SYSTEM_FONT);

	// Get LOGFONT for passed hfont
	if (!W32->GetObject(hfont, sizeof(LOGFONT), &lf))
		return E_FAIL;

	/* COMPATIBILITY ISSUE:
	 * RichEdit 1.0 selects hfont into a screen DC, gets the TEXTMETRIC,
	 * and uses tm.tmHeight - tm.tmInternalLeading instead of lf.lfHeight
	 * in the following. The following is simpler and since we have broken
	 * backward compatibility on line/page breaks, I've left it (murrays).
	 */

	yHeight = (lf.lfHeight * LY_PER_INCH) / sysparam.GetYPerInchScreenDC();
	if(yHeight < 0)
		yHeight = -yHeight;
#ifdef MACPORTStyle
	dwEffects = (CFM_EFFECTS | CFE_AUTOBACKCOLOR | CFE_OUTLINE | CFE_SHADOW) & ~(CFE_PROTECTED | CFE_LINK);
#else
	dwEffects = (CFM_EFFECTS | CFE_AUTOBACKCOLOR) & ~(CFE_PROTECTED | CFE_LINK);
#endif
	dwMask = CFM_ALL2;							// In case default gets
												//  Apply()'d
	if(lf.lfWeight < FW_BOLD)
		dwEffects &= ~CFE_BOLD;

#ifdef MACPORTStyle
	if(!(lf.lfWeight & FW_OUTLINE))
		dwEffects &= ~CFE_OUTLINE;
	if (!(lf.lfWeight & FW_SHADOW))
		dwEffects &= ~CFE_SHADOW;
#endif

	if(!lf.lfItalic)
		dwEffects &= ~CFE_ITALIC;

	if(!lf.lfUnderline)
		dwEffects &= ~CFE_UNDERLINE;

	if(!lf.lfStrikeOut)
		dwEffects &= ~CFE_STRIKEOUT;

	wWeight	= (WORD)lf.lfWeight;

	lcid = GetSystemDefaultLCID();
	bCharSet = lf.lfCharSet;
	bPitchAndFamily = lf.lfPitchAndFamily;
	wcscpy_s(szFaceName, lf.lfFaceName);
	bUnderlineType = CFU_UNDERLINE;				// Default solid underlines
												// Are gated by CFE_UNDERLINE
	SetCRC();
	wInternalFlags = 0;

	return NOERROR;
}

/*
 *	CCharFormat::Set(pCF)
 *
 *	@mfunc
 *		Copy *<p pCF> to this CCharFormat 
 *
 *	@devnote
 *		*<p pCF> is an external CHARFORMAT or CHARFORMAT2 with the
 *		appropriate size given by cbSize. But this CCharFormat is internal
 *		and cbSize is used as a reference count.  Note: this differs from
 *		Apply() in that it copies data without checking the dwMask bits.
 */
void CCharFormat::Set (
	const CHARFORMAT *pCF) 	//@parm	CHARFORMAT to copy to this CCharFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::Set");

	UINT cb = pCF->cbSize;

    // Protect against overflow
    ASSERT(cb <= sizeof(CCharFormat));
    if(cb > sizeof(CCharFormat))
    {
        return;
    }

	CopyFormat(this, pCF, cb);
	ZeroMemory((LPBYTE)this + cb, sizeof(CCharFormat) - cb);
	if(cb < sizeof(CHARFORMAT2))		// Use defaults for CHARFORMAT
	{
		dwEffects |= CFE_AUTOBACKCOLOR;
		bUnderlineType = CFU_UNDERLINE;
	}
	SetCRC(); 							// Set the CRC for rapid Finds
}

/*
 *	CCharFormat::Set(pCF)
 *
 *	@mfunc
 *		Copy *<p pCF> to this CCharFormat 
 *
 *	@devnote
 *		This function also copies the data members of CCharFormat (in addition
 *		to those of CHARFORMAT and CHARFORMAT2
 */
void CCharFormat::Set(const CCharFormat *pCF)
{
	*this = *pCF; 
	SetCRC();
}

/*
 *	CCharFormat::SetA(pCFA)
 *
 *	@mfunc
 *		Copy the ANSI CHARFORMATA *<p pCFA> (including pCFA->dwMask) to this
 *		UNICODE character format structure
 *
 *	@rdesc
 *		TRUE if successful; else FALSE
 */
BOOL CCharFormat::SetA(
	CHARFORMATA *pCFA)		//@parm CHARFORMATA to apply to this CF
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::SetA");

	if(!IsValidCharFormatA(pCFA))
		return FALSE;

	// Copy from dwMask up to szFaceName
	CopyMemory((BYTE *)this + sizeof(DWORD), (BYTE *)pCFA + sizeof(DWORD),
				offsetof(CHARFORMAT, szFaceName) - sizeof(DWORD));

	// should this code change such that we pass a code page to UnicodeFromMbcs
	// which isn't guaranteed to be on the system, we should change this code so 
	// that we stuff the DBCS char's into the wchar buffer (see rtfread.cpp)

	// We have to use CP_ACP to convert font name.  We used to 
	// use GetLocaleCodePage() which would cause problem in FE system using
	// non-FE locale.  e.g. Win95Trad.Chinese with US English regional setting.
	if(pCFA->dwMask & CFM_FACE)
		UnicodeFromMbcs(szFaceName, LF_FACESIZE, pCFA->szFaceName, LF_FACESIZE,
			CP_ACP);

	if(pCFA->cbSize	== sizeof(CHARFORMATA))
	{
		// ignore CHARFORMAT2 adds, but set our size to the UNICODE version
		cbSize = sizeof(CHARFORMATW);
		dwMask &= CFM_ALL;	
	}									
	else if (pCFA->dwMask & ~CFM_ALL)
	{
		// better have been an ansi 2.0 structure
		Assert(pCFA->cbSize == sizeof(CHARFORMAT2A)); 
		CopyMemory(&wWeight, &((CHARFORMAT2A *)pCFA)->wWeight, CHARFORMATDELTA);
	}

	SetCRC(); 
	wInternalFlags = 0;

	return TRUE;
}

/*
 *	CCharFormat::SetCRC()
 *
 *	@mfunc
 *		For fast font cache lookup, calculate CRC.
 *
 *	@devnote
 *		The font cache stores anything that has to
 *		do with measurement of metrics. Any font attribute
 *		that does not affect this should NOT be counted
 *		in the CRC; things like underline and color are not counted.
 */
void CCharFormat::SetCRC()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormat::SetCRC");

    BYTE    bCrc = 0;
    BYTE*   pb;
    BYTE *  pend = (BYTE*)&yOffset;
	TCHAR	*pFaceName;

    for (pb = (BYTE*)&dwEffects; pb < pend; pb++)
        bCrc ^= *pb;

    pend = (BYTE*)&szFaceName;
    
    for (pb = (BYTE*)&bCharSet; pb < pend; pb++)
        bCrc ^= *pb;

    pend = (BYTE*)&szFaceName + sizeof(szFaceName);
    
    for (pb = (BYTE*)&szFaceName, pFaceName = (TCHAR *)&szFaceName; 
		*pFaceName && pb < pend; pFaceName++)
	{
        bCrc ^= *pb++;
		bCrc ^= *pb++;
	}

	if (!bCrc ) bCrc++;				// 0 is reserved for not set.

	this->bCRC = bCrc;
}

//------------------------- CParaFormat Class -----------------------------------

CParaFormat::CParaFormat()
{
	cbSize = sizeof(PARAFORMAT2);
	dwBorderColor = 0;
	//dwMask2 = 0;							// For further extensions...
}

/*
 *	CParaFormat::AddTab(tbPos, tbAln, tbLdr)
 *
 *	@mfunc
 *		Add tabstop at position <p tbPos>, alignment type <p tbAln>, and
 *		leader style <p tbLdr>
 *
 *	@rdesc
 *		(success) ? NOERROR : S_FALSE
 *
 *	@devnote
 *		Tab struct that overlays LONG in internal rgxTabs is
 *
 *			DWORD	tabPos : 24;
 *			DWORD	tabType : 4;
 *			DWORD	tabLeader : 4;
 */
HRESULT CParaFormat::AddTab (
	LONG	tbPos,		//@parm New tab position
	LONG	tbAln,		//@parm New tab alignment type
	LONG	tbLdr,		//@parm New tab leader style
	BOOL	fInTable)	//@parm True if simulating cells
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::AddTab");

	if (!fInTable &&
		((DWORD)tbAln > tomAlignBar ||				// Validate arguments
		 (DWORD)tbLdr > tomLines ||					// Comparing DWORDs causes
		 (DWORD)tbPos > 0xffffff || !tbPos))		//  negative values to be
	{												//  treated as invalid
		return E_INVALIDARG;
	}

	LONG iTab;
	LONG tbPosCurrent;
	LONG tbValue = tbPos + (tbAln << 24) + (tbLdr << 28);

	if((rgxTabs[0] & PFT_DEFAULT) == PFT_DEFAULT)	// If 1st tab is default
		cTabCount = 0;								//  tab, delete it

	for(iTab = 0; iTab < cTabCount &&				// Determine where to
		tbPos > GetTabPos(rgxTabs[iTab]); 			//  insert new tabstop
		iTab++) ;

	if(iTab < MAX_TAB_STOPS)
	{
		tbPosCurrent = GetTabPos(rgxTabs[iTab]);
		if(iTab == cTabCount || tbPosCurrent != tbPos)
		{
			MoveMemory(&rgxTabs[iTab + 1],			// Shift array up
				&rgxTabs[iTab],						//  (unless iTab = Count)
				(cTabCount - iTab)*sizeof(LONG));

			if(cTabCount < MAX_TAB_STOPS)			// If there's room,
			{
				rgxTabs[iTab] = tbValue;			//  store new tab stop,
				cTabCount++;						//  increment tab count,
				return NOERROR;						//  signal no error
			}
		}
		else if(tbPos == tbPosCurrent)				// Update tab since leader
		{											//  style or alignment may
			rgxTabs[iTab] = tbValue;				//  have changed
			return NOERROR;
		}
	}
	return S_FALSE;
}

/*
 *	CParaFormat::Apply(pPF)
 *
 *	@mfunc
 *		Apply *<p pPF> to this CParaFormat as specified by nonzero bits in
 *		<p pPF>->dwMask
 */
HRESULT CParaFormat::Apply (
	const CParaFormat *pPF)		//@parm CParaFormat to apply to this PF
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Apply");

	const DWORD dwMaskApply	= pPF->dwMask;
	BOOL		fPF = pPF->cbSize == sizeof(PARAFORMAT);
	WORD		wEffectMask;

	if(dwMaskApply & PFM_NUMBERING)
		wNumbering = pPF->wNumbering;

	if(dwMaskApply & PFM_STARTINDENT)
		dxStartIndent = pPF->dxStartIndent;

	else if(dwMaskApply & PFM_OFFSETINDENT)
		dxStartIndent += pPF->dxStartIndent;

	if(dwMaskApply & PFM_OFFSET)
		dxOffset = pPF->dxOffset;

	if(dwMaskApply & PFM_RIGHTINDENT)
		dxRightIndent = pPF->dxRightIndent;

	if(dwMaskApply & PFM_ALIGNMENT)
	{
		wAlignment = pPF->wAlignment;
		if(!fPF && (wAlignment < PFA_LEFT || wAlignment > PFA_JUSTIFY))
		{
			TRACEERRORSZ("CParaFormat::Apply: invalid Alignment");
			return E_INVALIDARG;
		}
	}

	if(dwMaskApply & PFM_TABSTOPS)
	{
		cTabCount = min(pPF->cTabCount, MAX_TAB_STOPS);
		cTabCount = max(cTabCount, 0);
		CopyMemory(rgxTabs, pPF->rgxTabs, sizeof(LONG)*cTabCount);
		ZeroMemory(rgxTabs + cTabCount, sizeof(LONG)*(MAX_TAB_STOPS - cTabCount));
	}

	// AymanA: 11/7/96 Moved the wEffects set before the possible return NOERROR.
	wEffectMask	= (WORD)(dwMaskApply >> 16);	// Reset effect bits to be
	wEffects &= ~wEffectMask;					//  modified and OR in
	wEffects |= pPF->wEffects & wEffectMask;	//  supplied values

	if ((dwMaskApply & PFM_RTLPARA) && !(dwMaskApply & PFM_ALIGNMENT) &&
		wAlignment != PFA_CENTER)
	{
		wAlignment = (wEffects & PFE_RTLPARA) ? PFA_RIGHT : PFA_LEFT;
	}

	// PARAFORMAT check
	if(fPF && dwMaskApply & (PFM_STARTINDENT | PFM_OFFSET))
	{
		if(dxStartIndent < 0)					// Don't let indent go
			dxStartIndent = 0;					//  negative

		if(dxStartIndent + dxOffset < 0)		// Don't let indent +
			dxOffset = -dxStartIndent;			//  offset go negative

		return NOERROR;
	}

	// PARAFORMAT2 extensions
	if(dwMaskApply & PFM_SPACEBEFORE)
		dySpaceBefore	= pPF->dySpaceBefore;

	if(dwMaskApply & PFM_SPACEAFTER)
		dySpaceAfter	= pPF->dySpaceAfter;

	if(dwMaskApply & PFM_LINESPACING)
	{
		dyLineSpacing	= pPF->dyLineSpacing;
		bLineSpacingRule = pPF->bLineSpacingRule;
	}

	if(dwMaskApply & PFM_OUTLINELEVEL)
		bOutlineLevel	= pPF->bOutlineLevel;

	if(dwMaskApply & PFM_STYLE)
		HandleStyle(pPF->sStyle);

	Assert((bOutlineLevel & 1) ^ IsHeadingStyle(sStyle));

	if(dwMaskApply & PFM_SHADING)
	{
		wShadingWeight	= pPF->wShadingWeight;
		wShadingStyle	= pPF->wShadingStyle;
	}

	if(dwMaskApply & PFM_NUMBERINGSTART)
		wNumberingStart	= pPF->wNumberingStart;

	if(dwMaskApply & PFM_NUMBERINGSTYLE)
		wNumberingStyle	= pPF->wNumberingStyle;

	if(dwMaskApply & PFM_NUMBERINGTAB)
		wNumberingTab	= pPF->wNumberingTab;

	if(dwMaskApply & PFM_BORDER)
	{
		dwBorderColor	= pPF->dwBorderColor;
		wBorders		= pPF->wBorders;
		wBorderSpace	= pPF->wBorderSpace;
		wBorderWidth	= pPF->wBorderWidth;
	}

	return NOERROR;
}

/*
 *	CParaFormat::ApplyDefaultStyle(Style)
 *
 *	@mfunc	
 *		Copy default properties for Style
 */
void CParaFormat::ApplyDefaultStyle (
	LONG Style)		//@parm Style to apply
{
	Assert(IsKnownStyle(Style));

	if(IsHeadingStyle(Style))				// Set Style's dySpaceBefore,
	{										//  dySpaceAfter (in twips)
		dySpaceBefore = 12*20;				//  (same for all headings)
		dySpaceAfter  =  3*20;
		wNumbering	  = 0;					// No numbering
	}
}

/*
 *	CParaFormat::Compare(pPF)
 *
 *	@mfunc
 *		Compare this CParaFormat to *<p pPF>
 *
 *	@rdesc
 *		TRUE if they are the same
 *
 *	@devnote
 *		First compare 5 DWORDs of PARAFORMAT (wNumbering, wReserved),
 *		dxStartIndent, dxRightIndent, dxOffset, (wAlignment, cTabCount).
 *		If they are identical, compare the remaining cTabCount - 1
 *		elements of the tab array.  If they, too, are identical, compare the
 *		PARAFORMAT2 extensions. For PARAFORMAT structures, the extension values
 *		are taken to equal 0.  Return TRUE only if all comparisons succeed.
 */
BOOL CParaFormat::Compare (
	const CParaFormat *pPF) const		//@parm	CParaFormat to compare this
{										//  CParaFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Compare");

	DWORD	Count = 5;
	BOOL	fPF2 = pPF->cbSize == sizeof(PARAFORMAT2);
	DWORD	i;
	DWORD *	p1 = (DWORD *)this + 2;			// Bypass cbSize & dwMask fields
	DWORD *	p2 = (DWORD *)pPF  + 2;

	if(cTabCount)
		Count += cTabCount;

	for (i = 0; i < Count; i++)				// Compare first 5 DWORDs plus
	{										//  any tabs that are defined
		if(*p1++ != *p2++)
			return FALSE;
	}

	/* Compare PARAFORMAT2 extras:
	 *		1. dySpaceBefore
	 *		2. dySpaceAfter
	 *		3. dyLineSpacing
	 *		4. sStyle, bLineSpacingRule, bCRC
	 *		5. wShadingWeight,	wShadingStyle,
	 *		6. wNumberingStart, wNumberingStyle
	 *		7. wNumberingTab, wBorderSpace
	 *		8. wBorderWidth, wBorders
	 *
	 *		i.e., 8 extra DWORDs
	 *
	 * Currently internal:
	 *		1. dwBorderColors
	 *
	 *		i.e., 1 extra DWORD
	 */
	DWORD j;
	p1 = (DWORD *)&this->dySpaceBefore;
	p2 = (DWORD *)&pPF->dySpaceBefore;

	AssertSz(offsetof(CParaFormat, dySpaceBefore) == 4*(7 + MAX_TAB_STOPS),
		"CParaFormat::Compare: unpacked PARAFORMAT struct");
	AssertSz(sizeof(CParaFormat) == 4*(7 + MAX_TAB_STOPS + 8 + 1),
		"CParaFormat::Compare: unexpected CParaFormat size");

	for (i = j = 0; i < 9; i++)				// PARAFORMAT2 extensions
	{
		if(*p1++ != *p2++)
			return FALSE;
	}

	return TRUE;
}

/*
 *	CParaFormat::DeleteTab(tbPos)
 *
 *	@mfunc
 *		Delete tabstop at position <p tbPos>
 *
 *	@rdesc
 *		(success) ? NOERROR : S_FALSE
 */
HRESULT CParaFormat::DeleteTab (
	LONG	tbPos)			//@parm Tab position to delete
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::DeleteTab");

	LONG	Count	= cTabCount;
	LONG	iTab;

	if(tbPos <= 0)
		return E_INVALIDARG;

	for(iTab = 0; iTab < Count; iTab++)			// Find tabstop for position
	{
		if (GetTabPos(rgxTabs[iTab]) == tbPos)
		{
			MoveMemory(&rgxTabs[iTab],			// Shift array down
				&rgxTabs[iTab + 1],				//  (unless iTab is last tab)
				(Count - iTab - 1)*sizeof(LONG));
			cTabCount--;						// Decrement tab count and
			return NOERROR;						//  signal no error
		}
	}
	return S_FALSE;
}

/*
 *	CParaFormat::Delta(pPF)
 *
 *	@mfunc
 *		Adjust dwMask for differences between this CParaFormat and *<p pPF>
 *
 *	@devnote
 *		*<p pPF> is an external CParaFormat, i.e., it's either a PARAFORMAT
 *		or a PARAFORMAT2 with the appropriate size given by cbSize. But
 *		this CParaFormat is internal and cbSize is used as a reference count.
 */
void CParaFormat::Delta (
	CParaFormat *pPF) const 		//@parm	CParaFormat to compare this
{									//  CParaFormat to
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Delta");

	BOOL	fPF2 = pPF->cbSize == sizeof(PARAFORMAT2);
	LONG	dwT = 0;								// Collect mask bits for
													//  properties that change
	if(wNumbering	 != pPF->wNumbering)
		dwT |= PFM_NUMBERING;

	if(dxStartIndent != pPF->dxStartIndent)
		dwT |= PFM_STARTINDENT;

	if(dxRightIndent != pPF->dxRightIndent)
		dwT |= PFM_RIGHTINDENT;

	if(dxOffset		 != pPF->dxOffset)
		dwT |= PFM_OFFSET;

	if(wAlignment	 != pPF->wAlignment)
		dwT |= PFM_ALIGNMENT;

	AssertSz(pPF->cTabCount >= 0 && pPF->cTabCount <= MAX_TAB_STOPS,
		"RTR::GetParaFormat(): illegal tab count");

	if (pPF->dwMask & PFM_TABSTOPS &&
		(cTabCount != pPF->cTabCount ||
			(cTabCount > 0 && CompareMemory(rgxTabs, pPF->rgxTabs,
										cTabCount * sizeof(LONG)))))
	{
		dwT |= PFM_TABSTOPS;
	}

	dwT |= (wEffects ^ pPF->wEffects) << 16;


	if(fPF2)
	{
		if(dySpaceBefore	!= pPF->dySpaceBefore)
			dwT |= PFM_SPACEBEFORE;

		if(dySpaceAfter	 	!= pPF->dySpaceAfter)
			dwT |= PFM_SPACEAFTER;

		if(dyLineSpacing	!= pPF->dyLineSpacing ||
		   bLineSpacingRule	!= pPF->bLineSpacingRule)
		{
			dwT |= PFM_LINESPACING;
		}

		if(sStyle			!= pPF->sStyle)
			dwT |= PFM_STYLE;

		if (wShadingWeight	!= pPF->wShadingWeight ||
			wShadingStyle	!= pPF->wShadingStyle)
		{
			dwT |= PFM_SHADING;
		}

		if(wNumberingStart	!= pPF->wNumberingStart)
			dwT |= PFM_NUMBERINGSTART;

		if(wNumberingStyle	!= pPF->wNumberingStyle)
			dwT |= PFM_NUMBERINGSTYLE;

		if(wNumberingTab	!= pPF->wNumberingTab)
			dwT |= PFM_NUMBERINGTAB;

		if (wBorders		!= pPF->wBorders	 ||
			wBorderWidth	!= pPF->wBorderWidth ||
			wBorderSpace	!= pPF->wBorderSpace ||
			dwBorderColor	!= pPF->dwBorderColor)
		{
			dwT |= PFM_BORDER;
		}
	}

	pPF->dwMask &= ~dwT;						// Reset mask bits for
}												//  properties that differ

/*
 *	CParaFormat::fSetStyle()
 *
 *	@mfunc
 *		Return TRUE iff this PF specifies that the style should be set.
 *		See code for list of conditions for this to be true
 *
 *	@rdesc
 *		TRUE iff pCF specifies that the style sStyle should be set
 */
BOOL CParaFormat::fSetStyle() const
{
	return	(dwMask & ~(PFM_OUTLINELEVEL | PFM_COLLAPSED)) != PFM_ALL2 &&
			dwMask &  PFM_STYLE			  &&
			cbSize == sizeof(PARAFORMAT2) &&
			IsKnownStyle(sStyle);
}

/*
 *	CParaFormat::Get(pPF)
 *
 *	@mfunc
 *		Copy this CParaFormat to *<p pPF>
 */
void CParaFormat::Get (
	CParaFormat *pPF) const		//@parm	CParaFormat to copy this CParaFormat to
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Get");

	UINT cb = pPF->cbSize;

	pPF->dwMask = PFM_ALL2;						// Default PARAFORMAT2
	if(cb != sizeof(PARAFORMAT2))				// It isn't
	{
		pPF->dwMask = PFM_ALL;					// Make it PARAFORMAT
		ASSERT(cb == sizeof(PARAFORMAT));		// It better be a PARAFORMAT
	}

    // Bound the copy
    cb = min(cb, sizeof(CParaFormat));

	CopyFormat(pPF, this, cb);					// Copy this to pPF
	pPF->dwBorderColor = dwBorderColor;
}

/*
 *	CParaFormat::GetTab (iTab, ptbPos, ptbAln, ptbLdr)
 *
 *	@mfunc
 *		Get tab parameters for the <p iTab> th tab, that is, set *<p ptbPos>,
 *		*<p ptbAln>, and *<p ptbLdr> equal to the <p iTab> th tab's
 *		displacement, alignment type, and leader style, respectively.  The
 *		displacement is given in twips.
 *
 *	@rdesc
 *		HRESULT = (no <p iTab> tab)	? E_INVALIDARG : NOERROR
 */
HRESULT CParaFormat::GetTab (
	long	iTab,			//@parm Index of tab to retrieve info for
	long *	ptbPos,			//@parm Out parm to receive tab displacement
	long *	ptbAln,			//@parm Out parm to receive tab alignment type
	long *	ptbLdr) const	//@parm Out parm to receive tab leader style
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEEXTERN, "CParaFormat::GetTab");

	AssertSz(ptbPos && ptbAln && ptbLdr,
		"CParaFormat::GetTab: illegal arguments");

	if(iTab < 0)									// Get tab previous to, at,
	{												//  or subsequent to the
		if(iTab < tomTabBack)						//  position *ptbPos
			return E_INVALIDARG;

		LONG i;
		LONG tbPos = *ptbPos;
		LONG tbPosi;

		*ptbPos = 0;								// Default tab not found
		for(i = 0; i < cTabCount &&					// Find *ptbPos
			tbPos > GetTabPos(rgxTabs[i]); 
			i++) ;

		tbPosi = GetTabPos(rgxTabs[i]);				// tbPos <= tbPosi
		if(iTab == tomTabBack)						// Get tab info for tab
			i--;									//  previous to tbPos
		else if(iTab == tomTabNext)					// Get tab info for tab
		{											//  following tbPos
			if(tbPos == tbPosi)
				i++;
		}
		else if(tbPos != tbPosi)					// tomTabHere
			return S_FALSE;

		iTab = i;		
	}
	if((DWORD)iTab >= (DWORD)cTabCount)				// DWORD cast also
		return E_INVALIDARG;						//  catches values < 0

	iTab = rgxTabs[iTab];
	*ptbPos = GetTabPos(iTab);
	if((iTab & PFT_DEFAULT) == PFT_DEFAULT)			// Default tab is left
		iTab = 0;									//  aligned, no leader
	*ptbAln = GetTabAlign(iTab);
	*ptbLdr = GetTabLdr(iTab);
	return NOERROR;
}

/*
 *	CParaFormat::HandleStyle(Style)
 *
 *	@func
 *		If Style is a promote/demote command, i.e., if abs((char)Style)
 *			<= # heading styles - 1, add (char)Style to	sStyle (if heading)
 *			and to bOutlineLevel (subject to defined max and min values);
 *		else sStyle = Style.
 *
 *	@rdesc
 *		return TRUE iff sStyle or bOutlineLevel changed
 *
 *	@devnote
 *		Heading styles are -2 (heading 1) through -10 (heading 9), which
 *		with TOM and WOM. Heading outline levels are 0, 2,..., 16,
 *		corresponding to headings 1 through 9 (NHSTYLES), respectively,
 *		while text that follows has outline levels 1, 3,..., 17.  This value
 *		is used for indentation. Collapsed text has the PFE_COLLAPSED bit set.
 */
BOOL CParaFormat::HandleStyle(
	LONG Style)		//@parm Style, promote/demote code, or collapse-level code
{
	if(IsStyleCommand(Style))					// Set collapse level
	{											
		WORD wEffectsSave = wEffects;			

		Style = (char)Style;					// Sign extend low byte
		if(IN_RANGE(1, Style, NHSTYLES))
		{
			wEffects &= ~PFE_COLLAPSED;
			if((bOutlineLevel & 1) || bOutlineLevel > 2*(Style - 1))
				wEffects |= PFE_COLLAPSED;		// Collapse nonheadings and
		}										//  higher numbered headings
		else if(Style == -1)
			wEffects &= ~PFE_COLLAPSED;			// Expand all

		return wEffects != wEffectsSave;		// Return whether something
	}											//  changed

	// Ordinary Style specification
	BYTE bLevel = bOutlineLevel;
	bOutlineLevel |= 1;							// Default not a heading
	if(IsHeadingStyle(Style))					// Headings have levels
	{											//  0, 2,..., 16, while the
		bOutlineLevel = -2*(Style				//  text that follows has
						 - STYLE_HEADING_1);	//  1, 3,..., 17.
	}
	if(sStyle == Style && bLevel == bOutlineLevel)
		return FALSE;							// No change

	sStyle = (SHORT)Style;						
	return TRUE;
}

/*
 *	CParaFormat::InitDefault(lDefTab, wDefEffects)
 *
 *	@mfunc
 *		Initialize this CParaFormat with default paragraph formatting
 *
 *	@rdesc
 *		HRESULT = (if success) ? NOERROR : E_FAIL
 */
HRESULT CParaFormat::InitDefault(LONG lDefTab, WORD wDefEffects)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::InitDefault");
	
	if(!IsValidParaFormat(this))
		return E_FAIL;

	AssertSz(cbSize == sizeof(PARAFORMAT2),
		"CParaFormat::InitDefault: invalid CParaFormat");

	ZeroMemory((LPBYTE)this + cbSkipFormat, sizeof(CParaFormat) - cbSkipFormat);
	dwMask			= PFM_ALL2;
	wAlignment		= PFA_LEFT;
	sStyle			= STYLE_NORMAL;			// Default style
	wEffects		= wDefEffects;
	bOutlineLevel	= 1;					// Default highest text outline
											//  level
#if lDefaultTab <= 0
#error "default tab (lDefaultTab) must be > 0"
#endif

	cTabCount = 1;

	if (lDefTab <= 0)
		lDefTab = lDefaultTab;

	rgxTabs[0] = lDefTab + PFT_DEFAULT;

	return NOERROR;
}

#pragma optimize ( "", off )
/*
 *	CParaFormat::NumToStr(pch, n)
 *
 *	@mfunc	
 *		Convert the list number n to a string taking into consideration
 *		CParaFormat::wNumbering, wNumberingStart, and wNumberingStyle
 *	
 *	@rdesc
 *		cch of string converted
 */
LONG CParaFormat::NumToStr(
	TCHAR *	pch,		//@parm Target string
	LONG	n) const	//@parm Number + 1 to convert
{
	if((wNumberingStyle & 0xF00) == PFNS_NONUMBER)
	{
		*pch = 0;
		return 0;								// Number/bullet suppressed
	}

	if(!n)										// Bullet of some kind
	{											// CParaFormat::wNumbering
		*pch++ = (wNumbering > ' ')				//  values > ' ' are Unicode
			 ? wNumbering : L'\xB7';			//  bullets. Else use bullet
		return 1;								//  in symbol font
	}

	// Numbering of some kind
	//							 i  ii  iii  iv v  vi  vii  viii   ix
	static BYTE RomanCode[]	  = {1, 5, 0x15, 9, 2, 6, 0x16, 0x56, 0xd};
	static char RomanLetter[] = "ivxlcdmno";
	LONG		RomanOffset = 0;
	LONG		cch	= 0;						// No chars yet
	WCHAR		ch	= '0';						// Default char code offset
	LONG		d	= 1;						// Divisor
	LONG		r	= 10;						// Default radix 
	LONG   		quot, rem;						// ldiv result
	LONG		Style = (wNumberingStyle << 8) & 0xF0000;

	n--;										// Convert to number offset
	if(Style == tomListParentheses)				// Numbering like: (x)
	{					
		cch = 1;								// Store leading lparen
		*pch++ = '(';
	}

	if(wNumbering == tomListNumberAsSequence)
		ch = wNumberingStart;					// Needs generalizations, e.g.,
												//  appropriate radix
	else
	{
		n += wNumberingStart;
		if(IN_RANGE(tomListNumberAsLCLetter, wNumbering, tomListNumberAsUCLetter))
		{
			ch = (wNumbering == tomListNumberAsLCLetter) ? 'a' : 'A';
			if(wNumberingStart >= 1)
				n--;
			r = 26;								// LC or UC alphabetic number
		}										// Radix 26
	}

    // GuyBark JupiterJ: Only ever display a single character for katakana lists.
    if((wNumbering == tomListNumberAsKatakanaAIUEOdbl) || (wNumbering == tomListNumberAsKatakanaIROHAdbl))
    {
        // Start with 'A' in katakana.
        ch = 0x30A2;

        // The use of n here is zero based.
        if(wNumberingStart >= 1)
        {
            n--;
        }

        // Just cycle round at the end of the katakana.
        n %= (wNumbering == tomListNumberAsKatakanaAIUEOdbl ? KATAKANACHARSAIUEOCNT : KATAKANACHARSIROHACNT);

        // Display the required katanana character.
        *pch++ = (WORD)(ch + (wNumbering == tomListNumberAsKatakanaAIUEOdbl ? 
                                rgKatakanaCharsAIUEO[n] : rgKatakanaCharsIROHA[n]));

        cch++;
    }										
    else
    {
	    while(d < n)
	    {
		    d *= r;									// d = smallest power of r > n
		    RomanOffset += 2;
	    }
	    if(n && d > n)
	    {
		    d /= r;
		    RomanOffset -= 2;
	    }

	    while(d)
	    {
		    quot = n / d;
		    rem = n % d;
		    if(IN_RANGE(tomListNumberAsLCRoman, wNumbering, tomListNumberAsUCRoman))
		    {
			    if(quot)
			    {
				    n = RomanCode[quot - 1];
				    while(n)
				    {
					    ch = RomanLetter[(n & 3) + RomanOffset - 1];
					    if(wNumbering == tomListNumberAsUCRoman)
						    ch &= 0x5F;
					    *pch++ = ch;
					    n >>= 2;
					    cch++;
				    }
			    }
			    RomanOffset -= 2;
		    }
		    else
		    {
			    n = quot + ch;
			    if(r == 26 && d > 1)				// If alphabetic higher-order
				    n--;							//  digit, base it on 'a' or 'A'
			    *pch++ = (WORD)n;					// Store digit
			    cch++;
		    }
		    n = rem;								// Setup remainder
		    d /= r;
	    }
    }

	if(Style != tomListPlain)					// Trailing text
	{											// We only do rparen or period
		*pch++ = (Style == tomListPeriod) ? '.' : ')';
		cch++;
	}
	*pch = 0;									// Null terminate for RTF writer

	return cch;
}
#pragma optimize ( "", on)

/*
 *	CParaFormat::Set(pPF)
 *
 *	@mfunc
 *		Copy *<p pPF> to this CParaFormat 
 *
 *	@devnote
 *		*<p pPF> is an external PARAFORMAT or PARAFORMAT2 with the
 *		appropriate size given by cbSize. But this CParaFormat is internal
 *		and cbSize is used as a reference count.
 */
void CParaFormat::Set (
	const PARAFORMAT *pPF) 	//@parm	PARAFORMAT to copy to this CParaFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::Set");

	UINT cb = pPF->cbSize;

    // Protect against overflow
    ASSERT(cb <= sizeof(CParaFormat));
    if(cb > sizeof(CParaFormat))
    {
        return;
    }
    
	CopyFormat(this, pPF, cb);
	if(cb < sizeof(CParaFormat))
		ZeroMemory((LPBYTE)this + cb, sizeof(CParaFormat) - cb);
}

/*
 *	CParaFormat::UpdateNumber(n, pPF)
 *
 *	@mfunc
 *		Return new value of number for paragraph described by this PF
 *		following a paragraph described by pPF
 *
 *	@rdesc
 *		New number for paragraph described by this PF
 */
LONG CParaFormat::UpdateNumber (
	LONG  n,						//@parm Current value of number
	const CParaFormat *pPF) const	//@parm Previous CParaFormat
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormat::UpdateNumber");

	if(!IsListNumbered())			
		return 0;						// No numbering

	if(IsNumberSuppressed())
		return n;						// Number is suppressed, so no change

	if (!pPF || wNumbering != pPF->wNumbering ||
		(wNumberingStyle != pPF->wNumberingStyle && !pPF->IsNumberSuppressed()) ||
		wNumberingStart != pPF->wNumberingStart)
	{									// Numbering type or style
		return 1;						//  changed, so start over
	}
	return n + 1;						// Same kind of numbering,
}


//------------------------- Helper Functions -----------------------------------

// Defines and fixed font size details for increasing/decreasing font size
#define PWD_FONTSIZEPOINTMIN    1
// The following corresponds to the max signed 2-byte TWIP value, (32760)
#define PWD_FONTSIZEPOINTMAX    1638    

typedef struct tagfsFixup
{
    BYTE EndValue;
    BYTE Delta;
}
FSFIXUP;

const FSFIXUP fsFixups[] =
{
    12, 1,
    28, 2,
    36, 0,
    48, 0,
    72, 0,
    80, 0,
  	 0, 10			// EndValue = 0 case is treated as "infinite"
};

#define PWD_FONTSIZEMAXFIXUPS   (sizeof(fsFixups)/sizeof(fsFixups[0]))

/*
 *	GetUsableFontHeight(ySrcHeight, lPointChange)
 *
 *	@func
 *		Return a font size for setting text or insertion point attributes
 *
 *	@rdesc
 *		New TWIPS height
 *
 *	@devnote
 *		Copied from WinCE RichEdit code (written by V-GUYB)
 */
LONG GetUsableFontHeight(
	LONG ySrcHeight,		//@parm Current font size in twips
	LONG lPointChange)		//@parm Increase in pt size, (-ve if shrinking)
{
	LONG	EndValue;
	LONG	Delta;
    int		i;
    LONG	yRetHeight;

    // Input height in twips here, (TWentIeths of a Point).
    // Note, a Point is a 1/72th of an inch. To make these
    // calculations clearer, use point sizes here. Input height
    // in twips is always divisible by 20 (NOTE (MS3): maybe with
	// a truncation, since RTF uses half-point units).
    yRetHeight = (ySrcHeight / 20) + lPointChange;

    // Fix new font size to match sizes used by Word95
    for(i = 0; i < PWD_FONTSIZEMAXFIXUPS; ++i)
    {
		EndValue = fsFixups[i].EndValue;
		Delta	 = fsFixups[i].Delta;

        // Does new height lie in this range of point sizes?
        if(yRetHeight <= EndValue || !EndValue)
        {
            // If new height = EndValue, then it doesn't need adjusting
            if(yRetHeight != EndValue)
            {
                // Adjust new height to fit this range of point sizes. If
                // Delta = 1, all point sizes in this range stay as they are.
                if(!Delta)
                {
                    // Everything in this range is rounded to the EndValue
                    yRetHeight = fsFixups[(lPointChange > 0 ?
                                    i : max(i - 1, 0))].EndValue;
                }
                else if(Delta != 1)
                {
                    // Round new height to next delta in this range
                    yRetHeight = ((yRetHeight +
                        (lPointChange > 0 ? Delta - 1 : 0))
                                / Delta) * Delta;
                }
            }
            break;
        }
    }

    // Limit the new text size. Note, if we fix the text size
    // now, then we won't take any special action if we change
    // the text size later in the other direction. For example,
    // we shrink chars with size 1 and 2. They both change to
    // size 1. Then we grow them both to 2. So they are the
    // same size now, even though they weren't before. This
    // matches Word95 behavior.
    yRetHeight = max(yRetHeight, PWD_FONTSIZEPOINTMIN);
    yRetHeight = min(yRetHeight, PWD_FONTSIZEPOINTMAX);

    return yRetHeight*20;			// Return value in twips
}

/*
 *	IsValidCharFormat(pCF)
 *
 *	@func
 *		Return TRUE iff the structure *<p pCF> has the correct size to be
 *		a CHARFORMAT or a CHARFORMAT2
 *
 *	@rdesc
 *		Return TRUE if *<p pCF> is a valid CHARFORMAT(2)
 */
BOOL IsValidCharFormat (
	const CHARFORMAT * pCF) 		//@parm CHARFORMAT to validate
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidCharFormat");

	if (pCF && (pCF->cbSize == sizeof(CHARFORMAT) ||
				pCF->cbSize == sizeof(CHARFORMAT2)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus CHARFORMAT from client !!!!!!!!!!!!!");
	return FALSE;
}

/*
 *	IsValidCharFormatA(pCFA)
 *
 *	@func
 *		Return TRUE iff the structure *<p pCF> has the correct size to be
 *		a CHARFORMATA or a CHARFORMAT2A
 *
 *	@rdesc
 *		Return TRUE if *<p pCF> is a valid CHARFORMAT(2)A
 */
BOOL IsValidCharFormatA (
	const CHARFORMATA * pCFA) 	//@parm CHARFORMATA to validate
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidCharFormatA");

	if (pCFA && (pCFA->cbSize == sizeof(CHARFORMATA) ||
				 pCFA->cbSize == sizeof(CHARFORMAT2A)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus CHARFORMATA from client !!!!!!!!!!!!!");
	return FALSE;
}

/*
 *	IsValidParaFormat(pPF)
 *
 *	@func
 *		Return TRUE iff the structure *<p pPF> has the correct size to be
 *		a PARAFORMAT or a PARAFORMAT2
 *
 *	@rdesc
 *		Return TRUE if *<p pPF> is a valid PARAFORMAT(2)
 */
BOOL IsValidParaFormat (
	const PARAFORMAT * pPF)		//@parm PARAFORMAT to validate
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "IsValidParaFormat");

	if (pPF && (pPF->cbSize == sizeof(PARAFORMAT) ||
				pPF->cbSize == sizeof(PARAFORMAT2)))
	{
		return TRUE;
	}
	TRACEERRORSZ("!!!!!!!!!!! bogus PARAFORMAT from client !!!!!!!!!!!!!");
	return FALSE;
}


