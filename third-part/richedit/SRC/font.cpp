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
 *	@module FONT.C -- font cache |
 *
 *		includes font cache, char width cache;
 *		create logical font if not in cache, look up
 *		character widths on an as needed basis (this
 *		has been abstracted away into a separate class
 *		so that different char width caching algos can
 *		be tried.) <nl>
 *		
 *	Owner: <nl>
 *		David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Jon Matousek <nl>
 *
 *	History: <nl>
 *		7/26/95		jonmat	cleanup and reorganization, factored out
 *					char width caching code into a separate class.
 *
 */								 

#include "_common.h"
#include "_font.h"
#include "_rtfconv.h"	// Needed for GetCodePage
#include <specstrings_strict.h>
#include <intsafe.h>

#define CLIP_DFA_OVERRIDE   0x40	//  Used to disable Korea & Taiwan font association


ASSERTDATA

// corresponds to yHeightCharPtsMost in richedit.h
#define yHeightCharMost 32760

// NOTE: this is global across all instances in the same process.
static CFontCache *__fc;

WCHAR	lfJapaneseFaceName[LF_FACESIZE];
WCHAR	lfHangulFaceName[LF_FACESIZE];
WCHAR	lfBig5FaceName[LF_FACESIZE];
WCHAR	lfGB2312FaceName[LF_FACESIZE];

/*
 *	InitFontCache ()
 *	
 *	@mfunc
 *		Initializes font cache.
 *
 *	@devnote
 *		This is exists so reinit.cpp doesn't have to know all about the 
 *		font cache.
 */
BOOL InitFontCache()
{
	// GuyBark Jupiter: Beware of oom.

	if(!(__fc = new CFontCache))
	{
	    return FALSE;
	}

    __fc->Init();

    return TRUE;
}

/*
 *	FreeFontCachet ()
 *	
 *	@mfunc
 *		Frees font cache.
 *
 *	@devnote
 *		This is exists so reinit.cpp doesn't have to know all about the 
 *		font cache.
 */
void FreeFontCache()
{
	__fc->Free();
	delete __fc;
	__fc = NULL;
}


/*
 *	CFontCache & fc()
 *	
 *	@func
 *		initialize the global __fc.
 *	@comm
 *		current #defined to store 16 logical fonts and
 *		respective character widths.
 *		
 */
CFontCache & fc()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "fc");
    return *__fc;
}

// ===================================  CFontCache  ====================================


/*
 *	CFontCache::Init ()
 *	
 *	@mfunc
 *		Initializes font cache.
 *
 *	@devnote
 *		This is not a constructor because something bad seems to happen
 *		if we try to contruct a global object.
 */
void CFontCache::Init ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::CFontCache");

	_dwAgeNext = 0;
}

/*
 *	CFontCache::Free ()
 *	
 *	@mfunc
 *		Frees resources attached to font cache.
 *
 *	@devnote
 *		This is not a constructor because something bad seems to happen
 *		if we try to call a destructor on a global object.
 */
void CFontCache::Free ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::~CFontCache");
}


/*
 *	CCcs* CFontCache::GetCcs(hdc, pcf, lZoomNumerator, lZoomDenominator, yPixelsPerInch)
 *	
 *	@mfunc
 *		Search the font cache for a matching logical font and return it.
 *		If a match is not found in the cache,  create one.
 *
 *	@rdesc
 *		A logical font matching the given CHARFORMAT info.
 */
CCcs* CFontCache::GetCcs(
	HDC			hdc,				//@parm HDC into which font will be selected
	const CCharFormat *const pcf,	//@parm description of desired logical font
	const LONG lZoomNumerator,		//@parm Zoom numerator for getting display font
	const LONG lZoomDenominator,	//@parm Zoom denominator for getting
	const LONG yPerInch)			//@parm Y pixels per inch
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GetCcs");
	CLock	lock;

									//  display font
	const CCcs * const	pccsMost = &_rgccs[cccsMost - 1];
	CCcs *				pccs;
    LONG				lfHeight;

    BYTE				bCrc;
	SHORT				hashKey;
	// Duplicate the format structure because we might need to change some of the
	// values by the zoom factor 
	// and in the case of sub superscript
	CCharFormat cf = *pcf;


	//FUTURE igorzv
	//Take subscript size, subscript offset, superscript offset, superscript size 
	// from the OUTLINETEXMETRIC


	// change cf.yHeight in the case of sub superscript
	if (cf.dwEffects & (CFE_SUPERSCRIPT | CFE_SUBSCRIPT))
	{  
	     if (cf.dwEffects & CFE_SUBSCRIPT)
		 {  
		 	cf.yOffset-=cf.yHeight/5;
		 }
		 else
		 {
 		 	cf.yOffset += cf.yHeight/2;	 
		 }
	 	 cf.yHeight = 2*cf.yHeight/3;
	}					   

	// We only adjust the size if we need to.
	if (lZoomNumerator != lZoomDenominator)
	{
		cf.yHeight = MulDiv(cf.yHeight, lZoomNumerator, lZoomDenominator);
		cf.yOffset = MulDiv(cf.yOffset, lZoomNumerator, lZoomDenominator);
	}

	// calculate lfHeight used for Compare().
	lfHeight = -MulDiv(cf.yHeight, yPerInch, LY_PER_INCH);

	bCrc = cf.bCRC;

	Assert(0 != bCrc);								// Wasn't computed?

	if(!lfHeight)
		lfHeight--;	// round error, make this a minimum legal height of -1.

	// check our hash before going sequential.
	hashKey = bCrc & QUICKCRCSEARCHSIZE;
	if ( bCrc == quickCrcSearch[hashKey].bCrc )
	{
		pccs = quickCrcSearch[hashKey].pccs;
		if(pccs && pccs->_bCrc == bCrc && pccs->_fValid )
		{
	        if(pccs->Compare( &cf, lfHeight ))
			{
                goto matched;
			}
		}
	}
	quickCrcSearch[hashKey].bCrc = bCrc;

	// squentially search ccs for same character format
	for(pccs = &_rgccs[0]; pccs <= pccsMost; pccs++)
	{
		if( pccs->_bCrc == bCrc && pccs->_fValid )
		{
	        if(!pccs->Compare( &cf, lfHeight ))
                continue;
		matched:
			//$ FUTURE: make this work even with wrap around of dwAgeNext
			// Mark as most recently used if it isn't already in use.
			if(pccs->_dwAge != _dwAgeNext - 1)
				pccs->_dwAge = _dwAgeNext++;
			pccs->_dwRefCount++;		// bump up ref. count

			// setup the font to be used for this hdc.
			pccs->_hdc = hdc;

			// the same font can be used at different offsets.
			pccs->_yOffset = cf.yOffset ? (cf.yOffset * yPerInch / LY_PER_INCH) : 0;

			quickCrcSearch[hashKey].pccs = pccs;

			return pccs;
		}
	}

	// we did not find a match, init a new font cache.
	pccs = GrabInitNewCcs ( hdc, &cf, yPerInch );

	quickCrcSearch[hashKey].pccs = pccs;

	return pccs;
}

/*
 *	CCcs* CFontCache::GrabInitNewCcs(hdc, pcf)
 *	
 *	@mfunc
 *		create a logical font and store it in our cache.
 *		
 */
CCcs* CFontCache::GrabInitNewCcs(
	HDC hdc,						//@parm HDC into which font will be selected
	const CCharFormat * const pcf,	//@parm description of desired logical font
	const LONG yPerInch)			//@parm Y Pixels per Inch
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CFontCache::GrabInitNewCcs");

	DWORD				dwAgeOldest = 0xffffffff;
	CCcs *				pccs;
	const CCcs * const	pccsMost = &_rgccs[cccsMost - 1];
	CCcs *				pccsOldest = NULL;

	// look for unused entry and oldest in use entry
	for(pccs = &_rgccs[0]; pccs <= pccsMost && pccs->_fValid; pccs++)
		if(pccs->_dwRefCount == 0 && pccs->_dwAge < dwAgeOldest)
		{
			dwAgeOldest = pccs->_dwAge;
			pccsOldest = pccs;
		}

	if(pccs > pccsMost)		// Didn't find an unused entry, use oldest entry

	{
		pccs = pccsOldest;
		if(!pccs)
		{
			AssertSz(FALSE, "CFontCache::GrabInitNewCcs oldest entry is NULL");
			return NULL;
		}
	}

	pccs->_pfc = this;
	
	// Initialize new CCcs
	if(!pccs->Init(hdc, pcf, yPerInch) )
	{
		return NULL;
	}

	pccs->_dwRefCount++;

	return pccs;
}

// =============================  CCcs  class  ===================================================


/*
 *	BOOL CCcs::Init()
 *	
 *	@mfunc
 *		Init one font cache object. The global font cache stores
 *		individual CCcs objects. 
 */
BOOL CCcs::Init (
	HDC hdc,						//@parm HDC into which font will be selected
	const CCharFormat * const pcf,	//@parm description of desired logical font
	const LONG yPerInch)			//@parm Y pixels per inch
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Init");

	if ( _fValid ) Free();				// recycle already in-use fonts.


	if( MakeFont(hdc, pcf, yPerInch) )
	{
		_bCrc = pcf->bCRC;
		_yCfHeight = pcf->yHeight;

		Assert(0 != _bCrc);

		if( pcf->yOffset )				// offset for super/sub script.
		{
			_yOffset = pcf->yOffset * yPerInch / LY_PER_INCH;
		}
		else
			_yOffset = 0;
	
		_dwAge = _pfc->_dwAgeNext++;

		_fValid = TRUE;			// successfully created a new font cache.

	}

	return _fValid;
}

/*
 *	void CCcs::Free ()
 *	
 *	@mfunc
 *		Free any dynamic memory allocated by an individual font's cache.
 *		
 */
void CCcs::Free ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Free");

	Assert(_fValid);

	_widths.Free();

	if(_hfont)
		DestroyFont();

	_fValid = FALSE;
	_dwRefCount = 0;
}

/*
 *	BOOL CCcs::CheckFillWidths ()
 *	
 *	@mfunc
 *		Check existence, load nonexistent width information.
 */
BOOL CCcs::CheckFillWidths (
	TCHAR ch, 			//@parm	the TCHAR character in question.
	LONG &rlWidth )	 	//@parm the width to use
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::CheckFillWidths");

	if ( !_widths.CheckWidth(ch, rlWidth) )
	{
		return FillWidths(ch, rlWidth);
	}

	return TRUE;
}

/* 	
 *	CCcs::FillWidths (ch, rlWidth)
 *
 *	@mfunc
 *		Fill in this CCcs with metrics info for given device
 *
 *	@rdesc
 *		TRUE if OK, FALSE if failed
 */
BOOL CCcs::FillWidths(
	TCHAR ch, 		//@parm The TCHAR character we need a width for.
	LONG &rlWidth)	//@parm the width of the character
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::FillWidths");

	BOOL		fRes = FALSE;
	HFONT		hfontOld;

	AssertSz(_hfont, "CCcs::Fill - CCcs has no font");


	//	The mapping mode for the HDC is set before we get here.
	hfontOld = (HFONT)GetCurrentObject( _hdc, OBJ_FONT );
    if ( hfontOld == _hfont || (hfontOld = (HFONT)SelectObject(_hdc, _hfont)) )
    {
		// fill up the width info.
		fRes = _widths.FillWidth ( _hdc, ch, _xOverhangAdjust, rlWidth, 
			_wCodePage, _bConvertMode, _xAveCharWidth, _xDefDBCWidth, _fFixPitchFont );
    }
    
	//	restore the original mapping mode and font.
	//
	if(hfontOld && hfontOld != _hfont)
		SelectFont(_hdc, hfontOld);

	return fRes;
}

/* 	
 *	BOOL CCcs::MakeFont(hdc, pcf)
 *
 *	@mfunc
 *		Wrapper, setup for CreateFontIndirect() to create the font to be
 *		selected into the HDC.
 *
 *	@rdesc
 *		TRUE if OK, FALSE if allocation failure 
 */

#define szFontOfChoice TEXT("Arial")

BOOL CCcs::MakeFont(
	HDC hdc,						//@parm HDC into which  font will be selected
	const CCharFormat * const pcf,	//@parm description of desired logical font
	const LONG yPerInch)
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::MakeFont");

	_bConvertMode = pcf->bInternalEffects & CFEI_RUNISDBCS
				  ? CM_LOWBYTE : CM_NONE;

	_hdc = hdc;

	// Computes font height
	AssertSz(pcf->yHeight <= INT_MAX, "It's too big");

	//	Roundoff can result in a height of zero, which is bad.
	//	If this happens, use the minimum legal height.
	_lf.lfHeight = -(MulDiv(pcf->yHeight, yPerInch, LY_PER_INCH));
	if(_lf.lfHeight > 0)
		_lf.lfHeight = -_lf.lfHeight;		//FUTURE: do something more intelligent...
	if(!_lf.lfHeight)
		_lf.lfHeight--;	// round error, make this a minimum legal height of -1.
	_lf.lfWidth			= 0;

	if( pcf->wWeight != 0 )
	{
		_lf.lfWeight = pcf->wWeight;
	}
	else
	{
		_lf.lfWeight	= (pcf->dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
		#ifdef MACPORTStyle
		_lf.lfWeight	|= (pcf->dwEffects & CFE_OUTLINE) ? FW_OUTLINE : _lf.lfWeight;
		_lf.lfWeight	|= (pcf->dwEffects & CFE_SHADOW) ? FW_SHADOW : _lf.lfWeight; 
		#endif
	}

	_lf.lfItalic		= (pcf->dwEffects & CFE_ITALIC)	!= 0;
	_lf.lfUnderline		= 0;	
	_lf.lfStrikeOut		= 0;
	_lf.lfCharSet		= _bConvertMode == CM_LOWBYTE ? ANSI_CHARSET : pcf->bCharSet;
	_lf.lfEscapement 	= 0;
	_lf.lfOrientation 	= 0;
	_lf.lfOutPrecision 	= OUT_DEFAULT_PRECIS;
	_lf.lfClipPrecision = CLIP_DEFAULT_PRECIS | CLIP_DFA_OVERRIDE;
	_lf.lfQuality 		= DEFAULT_QUALITY;
	_lf.lfPitchAndFamily = pcf->bPitchAndFamily;

	// If the run is DBCS, that means the font's codepage is not available in
	// this system.  Use the English ANSI codepage instead so we will display
	// ANSI characters correctly.  NOTE: _wCodePage is only used for Win95.
	_wCodePage = (WORD)GetCodePage(_lf.lfCharSet);

	wcscpy_s(_lf.lfFaceName, pcf->szFaceName);

	_bUnderlineType = CFU_UNDERLINENONE;

	if ((pcf->dwEffects & CFE_UNDERLINE) != 0)
	{
		_bUnderlineType = CFU_UNDERLINE;

		if (pcf->bUnderlineType)
		{
			_bUnderlineType = pcf->bUnderlineType;
		}
	}

	_fStrikeOut = (pcf->dwEffects & CFE_STRIKEOUT) != 0;

	// Reader! A bundle of spagghetti code lies ahead of you!
	// But go on boldly, for these spagghetti are seasoned with 
	// lots of comments, and ... good luck to you...

	BOOL fTweakedCharSet = FALSE;

	HFONT hfontOriginalCharset = NULL;
	BYTE bOriginalCharset = _lf.lfCharSet;
	WCHAR szNewFaceName[LF_FACESIZE];

	if (NULL == GetFontWithMetrics(szNewFaceName))
	{
	    AssertSz(0, "GetFontWithMetrics failed");
	    return FALSE;
    }
    

	if (0 != wcscmp(szNewFaceName, _lf.lfFaceName))					
	{
		BOOL fCorrectFont = FALSE;

		if (_lf.lfCharSet == SYMBOL_CHARSET)					
		{
			// #1. if the face changed, and the specified charset was SYMBOL,
			//     but the face name exists and suports ANSI, we give preference
			//     to the face name

			_lf.lfCharSet = ANSI_CHARSET;
			fTweakedCharSet = TRUE;			

			hfontOriginalCharset = _hfont;
			GetFontWithMetrics(szNewFaceName);

			if (0 == wcscmp(szNewFaceName, _lf.lfFaceName))
				// that's right, ANSI is the asnwer
				fCorrectFont = TRUE;
			else
				// no, fall back by default
				// the charset we got was right
				_lf.lfCharSet = bOriginalCharset;
		}
		else if (_lf.lfCharSet == DEFAULT_CHARSET && _bCharSet == DEFAULT_CHARSET)
		{
			// #2. If we got the "default" font back, we don't know what it means
			// (could be anything) so we veryfy that this guy's not SYMBOL
			// (symbol is never default, but the OS could be lying to us!!!)
			// we would like to veryfy more like whether it actually gave us
			// Japanese instead of ANSI and labeled it "default"...
			// but SYMBOL is the least we can do

			_lf.lfCharSet = SYMBOL_CHARSET;
			wcscpy_s(_lf.lfFaceName, szNewFaceName);
			fTweakedCharSet = TRUE;			

			hfontOriginalCharset = _hfont;
			GetFontWithMetrics(szNewFaceName);

			if (0 == wcscmp(szNewFaceName, _lf.lfFaceName))
				// that's right, it IS symbol!
				// 'correct' the font to the 'true' one,
				//  and we'll get fMappedToSymbol
				fCorrectFont = TRUE;
				
			// always restore the charset name, we didn't want to
			// question he original choice of charset here
			_lf.lfCharSet = bOriginalCharset;

		}
		else if ( _bConvertMode != CM_LOWBYTE && IsFECharset(_lf.lfCharSet)
			&& (!OnWinNTFE() || !OnWin95FE()))
		{
			if (_bCharSet != _lf.lfCharSet && (VER_PLATFORM_WIN32_WINDOWS == dwPlatformId))
			{
				// on Win95, when rendering to PS driver,
				// it will give us something other than what we asked.
				// We have to try some known font we got from GDI
				switch (_lf.lfCharSet)
				{
					case CHINESEBIG5_CHARSET:
						wcscpy_s(_lf.lfFaceName, lfBig5FaceName);
						break;

					case SHIFTJIS_CHARSET:
						wcscpy_s(_lf.lfFaceName, lfJapaneseFaceName);
						break;

					case HANGEUL_CHARSET:
						wcscpy_s(_lf.lfFaceName, lfHangulFaceName);
						break;

					case GB2312_CHARSET:
						wcscpy_s(_lf.lfFaceName, lfGB2312FaceName);
						break;
				}
			}
			else
			{
				// this is a FE Font (from Lang pack) on a nonFEsystem
				wcscpy_s(_lf.lfFaceName, szNewFaceName);
			}
			hfontOriginalCharset = _hfont;		

			GetFontWithMetrics(szNewFaceName);

			if (0 == wcscmp(szNewFaceName, _lf.lfFaceName))
			{
				// that's right, it IS the FE font we want!
				// 'correct' the font to the 'true' one.
				fCorrectFont = TRUE;
//				if (VER_PLATFORM_WIN32_WINDOWS == dwPlatformId)
//				{
					// save up the GDI font names for later printing use
					switch (_lf.lfCharSet)
					{
						case CHINESEBIG5_CHARSET:
							wcscpy_s(lfBig5FaceName, _lf.lfFaceName);
							break;

						case SHIFTJIS_CHARSET:
							wcscpy_s(lfJapaneseFaceName, _lf.lfFaceName);
							break;

						case HANGEUL_CHARSET:
							wcscpy_s(lfHangulFaceName, _lf.lfFaceName);
							break;

						case GB2312_CHARSET:
							wcscpy_s(lfGB2312FaceName, _lf.lfFaceName);
							break;
					}
//				}
			}
			fTweakedCharSet = TRUE;
		}

		if (hfontOriginalCharset)
		{
		// either keep the old font or the new one		

			if (fCorrectFont)
			{
				DeleteObject(hfontOriginalCharset);
				hfontOriginalCharset = NULL;
			}
			else 
			{
				// fall back to the original font
				DeleteObject(_hfont);

				_hfont = hfontOriginalCharset;
				hfontOriginalCharset = NULL;
				
				GetTextMetrics();
			}
		}
	}

RetryCreateFont:
	{
		// could be that we just plain symply get mapped to symbol. 
		// avoid it
		BOOL fMappedToSymbol =	(_bCharSet == SYMBOL_CHARSET && 
								 _lf.lfCharSet != SYMBOL_CHARSET);

		BOOL fChangedCharset = (_bCharSet != _lf.lfCharSet && 
								_lf.lfCharSet != DEFAULT_CHARSET);

		if (fChangedCharset || fMappedToSymbol)
		{
			// Here, the system did not preserve the font language or mapped 
			// our non-symbol font onto a symbol font,
			// which will look awful when displayed.
			// Giving us a symbol font when we asked for a non-symbol one
			// (default can never be symbol) is very bizzare and means
			// that either the font name is not known or the system
			// has gone complete nuts here.
			// The charset language takes priority over the font name.
			// Hence, I would argue that nothing can be done to save the 
			// situation at this point, and we have to 
			// delete the font name and retry

			// let's tweak it a bit
			fTweakedCharSet = TRUE;

			if (0 == wcscmp(_lf.lfFaceName, szFontOfChoice))
			{
				// we've been here already
				// no font with an appropriate charset is on the system
				// try getting the ANSI one for the original font name
				// next time around, we'll null out the name as well!!
				if (_lf.lfCharSet == ANSI_CHARSET) 
				{
					TRACEINFOSZ("Asking for ANSI ARIAL and not getting it?!");

					// those Win95 guys have definitely outbugged me 
					goto GetOutOfHere;
				}

				DeleteObject(_hfont);
				wcscpy_s(_lf.lfFaceName, pcf->szFaceName);
				_lf.lfCharSet = ANSI_CHARSET;
				
			}
			else
			{
				DeleteObject(_hfont);
				wcscpy_s(_lf.lfFaceName, szFontOfChoice);
			}

			GetFontWithMetrics(szNewFaceName);
			goto RetryCreateFont;
		}

    }

GetOutOfHere:
	if(fTweakedCharSet || _bConvertMode == CM_LOWBYTE)
	{
		// we must preserve the original charset value, since it is used in Compare()
		_lf.lfCharSet = bOriginalCharset;
		wcscpy_s(_lf.lfFaceName, pcf->szFaceName);
	}

	if (hfontOriginalCharset)
		DeleteObject(hfontOriginalCharset);
   
	// if we're really really stuck, just get the system font and hope for the best.
	if( _hfont == 0 )
	{
		_hfont = (HFONT)GetStockObject(SYSTEM_FONT);
	}
	return _hfont != 0;
}

/*
 *	BOOL CCcs::GetFontWithMetrics (szNewFaceName)
 *	
 *	@mfunc
 *		Get metrics used by the measurer and renderer and the new face name.
 */
BOOL CCcs::GetFontWithMetrics (
	WCHAR* szNewFaceName)
{

	// we want to keep _lf untouched as it is used in Compare().
	_hfont = CreateFontIndirect(&_lf);

    if (_hfont)
    {
		// FUTURE (alexgo) if a font was not created then we may want to select 
		//		a default one.
		//		If we do this, then BE SURE not to overwrite the values of _lf as
		//		it is used to match with a pcf in our Compare().
		//
		// get text metrics, in logical units, that are constant for this font,
		// regardless of the hdc in use.
		GetTextMetrics();

		HFONT hfontOld = SelectFont(_hdc, _hfont);
		GetTextFace(_hdc, LF_FACESIZE, szNewFaceName);
		SelectFont(_hdc, hfontOld);
	}

	return (_hfont != NULL);
}

/*
 *	BOOL CCcs::GetTextMetrics ( )
 *	
 *	@mfunc
 *		Get metrics used by the measurer and renderer.
 *
 *	@comm
 *		These are in logical coordinates which are dependent
 *		on the mapping mode and font selected into the hdc.
 */
BOOL CCcs::GetTextMetrics ()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::GetTextMetrics");

	BOOL		fRes = TRUE;
	HFONT		hfontOld;
	TEXTMETRIC	tm;

	AssertSz(_hfont, "No font has been created.");
	AssertSz(_hfont, "CCcs::Fill - CCcs has no font");

    if ( GetCurrentObject( _hdc, OBJ_FONT ) != _hfont )
    {
	    hfontOld = (HFONT)SelectObject(_hdc, _hfont);

        if(!hfontOld)
        {
 			fRes = FALSE;
        	DestroyFont();
            goto cleanup;
        }
    }
    else
        hfontOld = 0;

	if(!W32->GetTextMetrics(_hdc, &tm))
	{
		fRes = FALSE;
    	DestroyFont();
        goto cleanup;
	}

	// the metrics, in logical units, dependent on the map mode and font.
	_yHeight		= (SHORT) tm.tmHeight;
	_yDescent		= (SHORT) tm.tmDescent;
	_xAveCharWidth	= (SHORT) tm.tmAveCharWidth;
	_xOverhangAdjust= (SHORT) tm.tmOverhang;

	_xOverhang = 0;
	_xUnderhang	= 0;
	if ( _lf.lfItalic )
	{
		_xOverhang =  SHORT ( (tm.tmAscent + 1) >> 2 );
		_xUnderhang =  SHORT ( (tm.tmDescent + 1) >> 2 );
	}

	// if fix pitch, the tm bit is clear
	_fFixPitchFont = !(TMPF_FIXED_PITCH & tm.tmPitchAndFamily);
	_xDefDBCWidth = 0;

	_bCharSet = tm.tmCharSet;

	// If SYMBOL_CHARSET is used, use the A APIs with the low bytes of the
	// characters in the run
	if(_bCharSet == SYMBOL_CHARSET)
		_bConvertMode = CM_LOWBYTE;

	else if (_bConvertMode == CM_NONE)
	{
		_bConvertMode = W32->DetermineConvertMode( tm.tmCharSet );
	}

	CalcUnderlineInfo(&tm);

cleanup:

	if(hfontOld)
		SelectFont(_hdc, hfontOld);

	return fRes;
}

/*
 *	BOOL CCcs::CalcUnderlineInfo ( )
 *	
 *	@mfunc
 *		Calculate underline & strike out offsets
 *
 *	@rdesc
 *		None.
 */
void CCcs::CalcUnderlineInfo(
	TEXTMETRIC *ptm)		//@parm text metric for the font
{
	W32->CalcUnderlineInfo (this, ptm);
}


/* 	
 *	CCcs::DestroyFont
 *
 *	@mfunc
 *		Destroy font handle for this CCcs
 *
 */
VOID CCcs::DestroyFont()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::DestroyFont");

	// clear out any old font
	if(_hfont)
	{
		DeleteObject(_hfont);
		_hfont = 0;
	}
}

/*
 *	CCcs::Compare (pcf,	lfHeight, fMetafile)
 *
 *	@mfunc
 *		Compares this font cache with the font properties of a 
 *      given CHARFORMAT
 *
 *	@rdesc
 *		FALSE iff did not match exactly.
 */
BOOL CCcs::Compare (
	const CCharFormat * const pcf,	//@parm Description of desired logical font
	LONG	lfHeight)		//@parm	lfHeight as calculated with the given HDC
{
	BOOL result;
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CCcs::Compare");

	result =
			_yCfHeight      == pcf->yHeight &&	// because different mapping modes
			_lf.lfHeight	== lfHeight &&		//  have diff logical coords
        	_lf.lfWeight	== ((pcf->dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL) &&
	        _lf.lfItalic	== ((pcf->dwEffects & CFE_ITALIC) != 0) &&
	        _fStrikeOut == ((pcf->dwEffects & CFE_STRIKEOUT) != 0) &&
			((((pcf->dwEffects & CFE_UNDERLINE) == 0) 
					&& (_bUnderlineType == CFU_UNDERLINENONE))
				|| (((pcf->dwEffects & CFE_UNDERLINE) != 0) 
					&& (_bUnderlineType == pcf->bUnderlineType))) &&
	        _lf.lfCharSet == pcf->bCharSet &&
        	_lf.lfPitchAndFamily == pcf->bPitchAndFamily &&
			((pcf->bInternalEffects & CFEI_RUNISDBCS)
					? _bConvertMode == CM_LOWBYTE : 1);
	// The following call has been known to cause troubles on Win CE
	result = result && !lstrcmp( _lf.lfFaceName, pcf->szFaceName );
	return result;
}

// =========================  WidthCache by jonmat  =========================
/*
 *	CWidthCache::CheckWidth(ch, rlWidth)
 *	
 *	@mfunc
 *		check to see if we have a width for a TCHAR character.
 *
 *	@comm
 *		Used prior to calling FillWidth(). Since FillWidth
 *		may require selecting the map mode and font in the HDC,
 *		checking here first saves time.
 *
 *	@comm
 *		Statistics are maintained to determine when to
 *		expand the cache. The determination is made after a constant
 *		number of calls in order to make calculations faster.
 *
 *	@rdesc
 *		returns TRUE if we have the width of the given TCHAR.
 */
BOOL CWidthCache::CheckWidth (
	const TCHAR ch,  //@parm char, can be Unicode, to check width for.
	LONG &rlWidth )	//@parm the width of the character
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckWidth");

	BOOL	exist;
	INT		i;

	const	CacheEntry * pWidthData = GetEntry ( ch );

	exist = ( ch == pWidthData->ch		// Have we fetched the width?
				&& pWidthData->width );	//  only because we may have ch == 0.

	if( exist )
	{
		rlWidth = pWidthData->width;
	}
	else
	{
		rlWidth = 0;
	}

	i = CACHE_SWITCH(ch);				// Update statistics
	if ( !_fMaxPerformance[i] )			//  if we have not grown to the max...
	{
		_accesses[i]++;
		if ( !exist )					// Only interesting on collision.
		{
			if ( 0 == pWidthData->width )// Test width not ch, 0 is valid ch.
			{
				_cacheUsed[i]++;		// Used another entry.
				AssertSz( _cacheUsed[i] <= _cacheSize[i]+1, "huh?");
			}
			else
				_collisions[i]++;		// We had a collision.

			if ( _accesses[i] >= PERFCHECKEPOCH )
				CheckPerformance(i);	// After some history, tune cache.
		}
	}
#ifdef DEBUG							// Continue to monitor performance
	else
	{
		_accesses[i]++;
		if ( !exist )					// Only interesting on collision.
		{
			if ( 0 == pWidthData->width )// Test width not ch, 0 is valid ch.
			{
				_cacheUsed[i]++;		// Used another entry.
				AssertSz( _cacheUsed[i] <= _cacheSize[i]+1, "huh?");
			}
			else
				_collisions[i]++;		// We had a collision.
		}

		if ( _accesses[i] > PERFCHECKEPOCH )
		{
			_accesses[i] = 0;
			_collisions[i] = 0;
		}
	}
#endif

	return exist;
}

/*
 *	CWidthCache::CheckPerformance(i)
 *	
 *	@mfunc
 *		check performance and increase cache size if deemed necessary.
 *
 *	@devnote
 *		To calculate 25% collision rate, we make use of the fact that
 *		we are only called once every 64 accesses. The inequality is 
 *		100 * collisions / accesses >= 25. By converting from 100ths to
 *		8ths, the ineqaulity becomes (collisions << 3) / accesses >= 2.
 *		Substituting 64 for accesses, this becomes (collisions >> 3) >= 2.
 */
void CWidthCache::CheckPerformance(
	INT i ) //@parm which cache to check.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CheckPerformance");

	if ( _fMaxPerformance[i] )			// Exit if already grown to our max.
		return;

	if (								// Grow the cache when
										//  cacheSize > 0 && 75% utilized,
			(  _cacheSize[i] > DEFAULTCACHESIZE &&
			( (_cacheSize[i] >> 1) + (_cacheSize[i] >> 2)) < _cacheUsed[i])
										//  or approx 25% collision rate.
		||  ( _collisions[i] >> COLLISION_SHIFT ) >= 2 )
	{
		GrowCache( &_pWidthCache[i], &_cacheSize[i], &_cacheUsed[i] );
	}
	_collisions[i]	= 0;				// This prevents wraps but makes
	_accesses[i]	= 0;				//  calc a local rate, not global.
										
										// Note if we've max'ed out.
	if ( _cacheSize[i] >= maxCacheSize[i] )
		_fMaxPerformance[i] = TRUE;

	AssertSz( _cacheSize[i] <= maxCacheSize[i], "max must be 2^n-1");
	AssertSz( _cacheUsed[i] <= _cacheSize[i]+1, "huh?");
}

/*
 *	CWidthCache::GrowCache(ppWidthCache, pCacheSize, pCacheUsed)
 *	
 *	@mfunc
 *		Exponentially expand the size of the cache.
 *
 *	@comm
 *		The cache size must be of the form 2^n as we use a
 *		logical & to get the hash MOD by storing 2^n-1 as
 *		the size and using this as the modulo.
 *
 *	@rdesc
 *		Returns TRUE if we were able to allocate the new cache.
 *		All in params are also out params.
 *		
 */
BOOL CWidthCache::GrowCache(
	CacheEntry **ppWidthCache,	//@parm cache
	INT *		pCacheSize,		//@parm cache's respective size.
	INT *		pCacheUsed)		//@parm cache's respective utilization.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::GrowCache");

	CacheEntry		*pNewWidthCache, *pOldWidthCache, *pWidthData;
	INT 			j, newCacheSize, newCacheUsed;
	TCHAR			ch;
	UINT uT;
	
	j = *pCacheSize;						// Allocate cache of 2^n.
	newCacheSize = max ( INITIALCACHESIZE, (j << 1) + 1);
	if (newCacheSize + 1 < newCacheSize || FAILED(UIntMult(sizeof(CacheEntry), newCacheSize + 1, &uT)))
		return FALSE;
	pNewWidthCache = (CacheEntry *)
			PvAlloc( uT, GMEM_ZEROINIT);

	if ( pNewWidthCache )
	{
		newCacheUsed = 0;
		*pCacheSize = newCacheSize;			// Update out params.
		pOldWidthCache = *ppWidthCache;
		*ppWidthCache = pNewWidthCache;
		for (; j >= 0; j--)					// Move old cache info to new.
		{
			ch = pOldWidthCache[j].ch;
			if ( ch )
			{
				pWidthData			= &pNewWidthCache [ch & newCacheSize];
				if ( 0 == pWidthData->ch )
					newCacheUsed++;			// Used another entry.
				pWidthData->ch		= ch;
				pWidthData->width	= pOldWidthCache[j].width;
			}
		}
		*pCacheUsed = newCacheUsed;			// Update out param.

											// Free old cache.
              											
		if (   pOldWidthCache <  &_defaultWidthCache[0][0]
			|| pOldWidthCache > &_defaultWidthCache[TOTALCACHES-1][(DEFAULTCACHESIZE+1)-1])
			//-1 on _defaultWidthCache indexes because of 0 based array.
		{
			FreePv(pOldWidthCache);
		}
	}

	return NULL != pNewWidthCache;
}

/*
 *	CWidthCache::FillWidth(hdc, ch, xOverhang, rlWidth)
 *	
 *	@mfunc
 *		Call GetCharWidth() to obtain the width of the given char.
 *
 *	@comm
 *		The HDC must be setup with the mapping mode and proper font
 *		selected *before* calling this routine.
 *
 *	@rdesc
 *		Returns TRUE if we were able to obtain the widths.
 *		
 */
BOOL CWidthCache::FillWidth (
	HDC			hdc,		//@parm Current HDC we want font info for.
	const TCHAR	ch,			//@parm Char to obtain width for.
	const SHORT xOverhang,	//@parm Equivalent to GetTextMetrics() tmOverhang.
	LONG &		rlWidth,	//@parm Width of character
	UINT		uiCodePage,	//@parm code page for text	
	BOOL		fANSI,		//@parm indicates font needs to use ANSI call.
	INT			iDefWidth,	//@parm Default width to use if font calc's zero
							//width. (Handles Win95 problem).
	INT			iDBCDefWidth,	//@parm Default width for DBC to use 
								// Handles Win95 Trad Chinese problem.
	BOOL		fFixPitch)		//@parm fix pitch font for DBC to use 
								// Handles Win95 Trad Chinese problem.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::FillWidth");

	char	ansiChar[2] = {0};
	INT		numOfDBCS = 0;
	CacheEntry * pWidthData = GetEntry ( ch );

	(void) W32->REGetCharWidth( hdc, ch, &pWidthData->width, uiCodePage );

#if 0
	// REVIEW  May not need codepage.
	// TODO

	if (!fANSI && uiCodePage != SYMBOL_CODEPAGE)
		(void) W32->REGetCharWidth( hdc, ch, &pWidthData->width, uiCodePage );
	else
	{
		WORD wDBCS = ch;
		(void) GetCharWidthA( hdc, wDBCS, wDBCS, &pWidthData->width );				
	}

	// Is this needed ????
	// fAnsi case, SYMBOL_CHARSET, or GetCharWidthW failed: try GetCharWidthA 
	if (!fRes || 0 == pWidthData->width)
	{
		if(uiCodePage != SYMBOL_CODEPAGE)
		{
			// Try to convert string
			numOfDBCS = WideCharToMultiByte( uiCodePage, 0, &ch, 1, 
				ansiChar, 2, NULL, NULL);

			if (2 == numOfDBCS)
				wDBCS = (BYTE)ansiChar[0] << 8 | (BYTE)ansiChar[1];

			else if (numOfDBCS)
				wDBCS = (BYTE)ansiChar[0];
		}
		fRes = GetCharWidthA( hdc, wDBCS, wDBCS, &pWidthData->width );				
	}
#endif

	pWidthData->ch = ch;
//	pWidthData->width	-= xOverhang;		// Don't need since we use
											//  GetTextExtentPoint32()
	if (0 >= pWidthData->width)
	{
		// Sometimes GetCharWidth will return a zero length for small
		// characters. When this happens we will use the default width
		// for the font if that is non-zero otherwise we just us 1 because
		// this is the smallest valid value.

		// This code can also be triggered if the overhang is bugger than the
		// width returned by the OS call to get the character width.
		
		// under Win95 Trad. Chinese, there is a bug in the font.
		// It is returning a width of 0 for a few characters (Eg 0x09F8D, 0x81E8)
		// In such case, we need to use 2 * iDefWidth since these are DBCS
		if (0 == iDefWidth)
			pWidthData->width = 1;
		else
			pWidthData->width = (numOfDBCS == 2) ? 
				(iDBCDefWidth ? iDBCDefWidth : 2 * iDefWidth) : iDefWidth;
	}

	rlWidth = pWidthData->width;
	return TRUE;
}

/*
 *	CWidthCache::GetWidth(ch)
 *	
 *	@mfunc
 *		get the width (A+B+C) for the given character.
 *	@comm
 *		we've already called GetCharWidth() at this point.
 *	@rdesc
 *		the width.
 */
INT CWidthCache::GetWidth (
	const TCHAR ch )//@parm char, can be Unicode, to check width for.
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::GetWidth");

	const CacheEntry * pWidthData = GetEntry ( ch );

	AssertSz( pWidthData->ch == ch, "Table not filled in?" );

	return pWidthData->width;
}

/*
 *	CWidthCache::Free()
 *	
 *	@mfunc
 *		Free any dynamic memory allocated by the width cache and prepare
 *		it to be recycled.
 *		
 */
VOID CWidthCache::Free()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::Free");

	INT i;

	for (i = 0; i < TOTALCACHES; i++ )
	{
		_fMaxPerformance[i] = FALSE;
		_cacheSize[i]		= DEFAULTCACHESIZE;
		_cacheUsed[i]		= 0;
		_collisions[i]		= 0;
		_accesses[i]		= 0;
		if ( _pWidthCache[i] != &_defaultWidthCache[i][0] )
		{
			FreePv(_pWidthCache[i]);
			_pWidthCache[i] = &_defaultWidthCache[i][0];
		}	
		ZeroMemory(_pWidthCache[i], sizeof(CacheEntry)*(DEFAULTCACHESIZE + 1));
	}
}

/*
 *	CWidthCache::CWidthCache()
 *	
 *	@mfunc
 *		Point the caches to the defaults.
 *		
 */
CWidthCache::CWidthCache()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::CWidthCache");

	INT i;

	for (i = 0; i < TOTALCACHES; i++ )
	{
		_pWidthCache[i] = &_defaultWidthCache[i][0];
	}
}

/*
 *	CWidthCache::~CWidthCache()
 *	
 *	@mfunc
 *		Free any allocated caches.
 *		
 */
CWidthCache::~CWidthCache()
{
	TRACEBEGIN(TRCSUBSYSFONT, TRCSCOPEINTERN, "CWidthCache::~CWidthCache");


	INT i;

	for (i = 0; i < TOTALCACHES; i++ )
	{
		if (_pWidthCache[i] != &_defaultWidthCache[i][0])
			FreePv(_pWidthCache[i]);
	}
}

#if 0 // unused
/*
 *	INT CheckDBChar ( UINT uiCodePage, WCHAR wChar, char *pAnsiChar)
 *	
 *	@func
 *		Determine if the input wChar is converted to a DBC or SBC
 *
 *	@comm
 *		Called when we need to determine the char width and when we render 
 *		the char.  This is mainly used for determining if the Bullet char (0xb7)
 *		has a DBC equivalent.
 *
 *	@rdesc
 *		return 2 if DBCS, otherwise return 0. Also, pAnsiChar contains the DBCS.
 *		
 */
INT	CheckDBChar ( UINT uiCodePage, WCHAR wChar, char *pAnsiChar)
{
	WORD wDBCS=0;
	
	// Convert string	
	if ( uiCodePage )	
		// check if we have a DBC bullet for this codepage		
		wDBCS = WideCharToMultiByte( uiCodePage, 0, &wChar, 1, 		
			pAnsiChar, 2, NULL, NULL);

	if ( 2 != wDBCS )
		*pAnsiChar = (CHAR)wChar;

	return wDBCS;
}
#endif



