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
 *	@doc 	INTERNAL
 *
 *	@module _FONT.H -- Declaration of classes comprising font caching |
 *	
 *	Purpose:
 *		Font cache
 *	
 *	Owner: <nl>
 *		David R. Fulmer <nl>
 *		Christian Fortini <nl>
 *		Jon Matousek <nl>
 *
 *	History: <nl>
 *		8/6/95		jonmat Devised dynamic expanding cache for widths.
 *
 */

#ifndef _FONT_H
#define _FONT_H

// Forwards
class CFontCache;
class CDevDesc;


// =============================  CCcs  ========================
// CCcs - caches font metrics and character size for one font

#define DEFAULTCACHESIZE	0			// size - 1
#define INITIALCACHESIZE	7			// size - 1 = 7; 2^n-1; size = 8
#define PERFCHECKEPOCH		64			// If changed, you must recalc
										//  and change COLLISION_SHIFT below.

#define COLLISION_SHIFT		3			// log(PERFCHECKEPOCH) / log(2) - 3

										// Where this character is cached.
#define CACHE_SWITCH(ch)	((ch < 0x4E00) ? 0 : ((ch < 0xAC00) ? 1 : 2))

// U+0000 -> U+4DFF		All Latin and other phonetics.
// U+4E00 -> U+ABFF		CJK Ideographics
// U+AC00 -> U+FFFF		Korean+, as Korean ends at U+D7A3
#define TOTALCACHES			3			

// For 2 caches at CJK Ideo split, max cache sizes {256,512} that give us a
// respective collision rate of <4% and <22%, and overall rate of <8%.
// Stats based on a 300K char Japanese text file.
static const INT maxCacheSize[TOTALCACHES] = {255, 511, 511};

/*
 *	CWidthCache
 *
 *	@class	Lightweight Unicode width cache.
 *
 *	@devnote Initial size is 52 bytes, 1st step is 100, and exponentially
 *			growing (currently) to 4660 bytes; NOTE, this can be reduced
 *			to 28, 60 and 3100 bytes if shorts are used and appropriate
 *			guarantees are placed on the range of width values.
 *
 *	Owner: <nl>
 *		Jon Matousek (jonmat) <nl>
 */
class CWidthCache 
{
	//@access	Private methods and data
	private:

							//@cmember	size is total cache slots - 1.
	INT		_cacheSize[TOTALCACHES];

							//@cmember	for statistics, num slots in use.
	INT		_cacheUsed[TOTALCACHES];
							//@cmember	for statistics, num fetches required.
	INT		_collisions[TOTALCACHES];
							//@cmember	for statistics, total num accesses.
	INT		_accesses[TOTALCACHES];
							//@cmember	for statistics, TRUE if grown to max.
	BOOL	_fMaxPerformance[TOTALCACHES];

	typedef struct {
		TCHAR	ch;
		INT		width;
	} CacheEntry;

							//@cmember	default storage for widths.
	CacheEntry	_defaultWidthCache[TOTALCACHES][DEFAULTCACHESIZE+1];

							//@cmember	pointers to storage for widths.
	CacheEntry *(_pWidthCache[TOTALCACHES]);

							//@cmember	Get location where width is stored.
	inline CacheEntry * GetEntry( const TCHAR ch )
				{	// logical & is really a MOD, as all of the bits
					// of cacheSize are turned on; the value of cacheSize is
					// required to be of the form 2^n-1.
					INT i = CACHE_SWITCH(ch);
					return &_pWidthCache[i][ ch & _cacheSize[i] ];
				}

							//@cmember	See if cache is performing within spec.
	void	CheckPerformance( INT iCache );
							//@cmember	Increase width cache size.
	BOOL	GrowCache( CacheEntry **widthCache, INT *cacheSize, INT *cacheUsed);

	//@access Public Methods
	public:
							//@cmember	Called before GetWidth
	BOOL	CheckWidth ( const TCHAR ch, LONG &rlWidth );
							//@cmember	Fetch width if CheckWidth ret FALSE.
	BOOL	FillWidth ( 
				HDC hdc, 
				const TCHAR ch, 
				const SHORT xOverhang, 
				LONG &rlWidth, 
				UINT uiCodePage, 
				BOOL fANSI,
				INT iDefWidth,
				INT	iDBCDefWidth,
				BOOL fFixPitch);

							//@cmember	Fetch the width.
	INT		GetWidth ( const TCHAR ch );
	
							//@cmember	Recycle width cache.
	void	Free();

							//@cmember	Construct width cache.
	CWidthCache();
							//@cmember	Free dynamic mem.
	~CWidthCache();
};


class CCcs
{
	friend class CFontCache;

private:
	CFontCache *	  _pfc;	// font cache this entry belongs to

	DWORD	_dwRefCount;	// ref. count
	DWORD 	_dwAge;			// for LRU algorithm

    BYTE    _bCrc;          // check sum for quick comparison with charformats

	class CWidthCache _widths;

public:
	HDC		_hdc;			// device context this entry is for
	LONG	_yCfHeight;		// Height of font in TWIPs.
	LONG 	_yOffset;		// baseline offset for super/subscript
	SHORT	_yHeight;		// total height of the character cell in logical units.
	SHORT 	_yDescent;		// distance from baseline to bottom of character cell in logical units.
	SHORT	_xAveCharWidth;	// average character width in logical units.
	USHORT	_wCodePage;		// code page for font.
	SHORT 	_xOverhangAdjust;// overhang for synthesized fonts in logical units.
	SHORT	_xOverhang;		// font's overhang.
	SHORT	_xUnderhang;	// font's underhang.
	SHORT	_xDefDBCWidth;	// default width for DB Character
	SHORT	_dyULOffset;	// Offset of the underline
	SHORT	_dyULWidth;		// Width of the underline.
	SHORT	_dySOOffset;	// Offset of the underline
	SHORT	_dySOWidth;		// Width of the underline.
	BYTE	_bCharSet;
	BYTE	_bConvertMode;	// See CONVERTMODE: CM_NONE, CM_WCTMB, CM_LOWBYTE
	BYTE	_bUnderlineType;// From char format 2.
	BYTE 	_fValid:1;		// CCcs is valid
	BYTE	_fFixPitchFont:1;	// font with fix character width
	BYTE	_fStrikeOut:1;	// Use strikeout for this font.

	HFONT 	_hfont;			// Windows font handle

    LOGFONT _lf;			// the log font as returned from GetObject().

private:

    BOOL    Compare (const CCharFormat * const pcf, LONG lfHeight);

    BOOL    MakeFont(
				HDC hdc, 
				const CCharFormat * const pcf, 
				const LONG yPerInch);

	void 	DestroyFont();

	BOOL	GetTextMetrics();

	BOOL	GetFontWithMetrics(WCHAR* szNewFaceName);

	void	CalcUnderlineInfo(TEXTMETRIC *ptm);

	BOOL	CheckFillWidths ( TCHAR ch, LONG &rlWidth );// exported for better 
														// error handling by 
														// client;
	BOOL	FillWidths ( TCHAR ch, LONG &rlWidth );

public:
	CCcs ()
		{
			_pfc = NULL;
			_fValid = FALSE;
		}
	~CCcs ()
		{
			if(_fValid) Free();
		}

	BOOL 	Init(HDC hdc, const CCharFormat * const pcf, const LONG yPerInch);
	void 	Free();
	void 	AddRef() 				{_dwRefCount++;}
	void 	Release() 				{if(_dwRefCount) _dwRefCount--;}

	INT		operator [](TCHAR ch)	{ return _widths.GetWidth (ch);}

	BOOL	Include ( TCHAR ch, LONG &rlWidth )
	{
		return CheckFillWidths ( ch, rlWidth );
	}
};

// =============================  CFontCache  =====================================================
// CFontCache - maintains up to ccsMost font caches

#define cccsMost FONTCACHESIZE
#define QUICKCRCSEARCHSIZE	31	// Must be 2^n - 1 for quick MOD
								//  operation, it is a simple hash.

class CFontCache
{
	friend class CCcs;

private:
	CCcs	_rgccs[cccsMost];
	DWORD 	_dwAgeNext;
	struct {
		BYTE	bCrc;
		CCcs	*pccs;
	} quickCrcSearch[QUICKCRCSEARCHSIZE+1];

private:

	CCcs* 	GrabInitNewCcs(
				HDC hdc, 
				const CCharFormat * const pcf, 
				const LONG yPerInch);

public:
	void Init();
	void Free();

	CCcs*	GetCcs(
				HDC hdc, 
				const CCharFormat * const pcf, 
				const LONG lZoomNumerator, 
				const LONG lZoomDenominator,
				const LONG yPerInch);

	void 	FreeCcs(const CCharFormat * const pcf);
};


extern CFontCache & fc();			// font cache manager

// helper function
INT	CheckDBChar ( UINT uiCodePage, WCHAR wChar, char *pAnsiChar); 

#endif

