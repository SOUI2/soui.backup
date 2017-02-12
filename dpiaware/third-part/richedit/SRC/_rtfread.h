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
 *	@module _RTFREAD.H -- RichEdit RTF Reader Class Definition |
 *
 *		This file contains the type declarations used by the RTF reader
 *		for the RICHEDIT control
 *
 *	Authors:<nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szUnicode.
 */
#ifndef __RTFREAD_H
#define __RTFREAD_H

#include "_rtfconv.h"

// TODO: Implement RTF tag logging for the Mac
#include "_rtflog.h"

typedef SHORT	ALIGN;

/*
 *		Destinations of the stuff we may read in while parsing
 */
enum
{
	destRTF,
	destColorTable,
	destFontTable,
	destBinary,
	destObject,
	destObjectClass,
	destObjectName,
	destObjectData,		  // keep next two together
	destPicture,
	destField,
	destFieldResult,
	destFieldInstruction,
	destParaNumbering,
	destParaNumText,
	destRealFontName,
	destFollowingPunct,
	destLeadingPunct,
	destDocumentArea,
	destNULL,
	destStyleSheet
};


/*
 *		Super or subscripting state
 */
enum
{
	sSub = -1,
	sNoSuperSub,
	sSuper
};

/*
 *	@struct STATE | 
 *		Structure to save current reader state
 */
struct STATE
{
	SHORT		sLanguage;				//@field Font information
	SHORT		sDefaultTabWidth;		//@field Default tab info in twips
	LONG		iCF;					//@field CF index at LBRACE

	// Miscellaneous flags
	unsigned	fInTable		 : 1;	//@field Are we parsing a table cell ?
	unsigned	fBullet			 : 1;	//@field group is a \\pn bullet group
	unsigned	fRealFontName	 : 1;	//@field found a real font name when parsing
	unsigned	fExplicitLang	 : 1;	//@field language explicitly declared

	// BiDi flags
	unsigned	fModDirection	 : 1;	//@field direction was modified
	unsigned	fRightToLeft	 : 1;	//@field Are we going right to left ?
	unsigned	fRightToLeftPara : 1;	//@field Para text going right to left ?
	unsigned	fModJoiner		 : 1;	//@field joiner bit was modified
	unsigned	fZeroWidthJoiner : 1;	//@field Zero Width Joiner ?

	unsigned	frtlch			 : 1;	//@field Are we going right to left ?
	unsigned	fltrch			 : 1;	//@field Are we going right to left ?

	

	// xchg 12370: keep numbering indent separate
	SHORT		sIndentNumbering;		//@field numbering indent
	SHORT		sDest;					//@field Current destination
	int			nCodePage;				//@field Current code page

	WORD		cbSkipForUnicodeMax;	//@field Bytes to skip after \uN is read

	// Scratch pad variables
	TEXTFONT *	ptf;					//@field Ptr to font table entry to fill
	BYTE		bRed;					//@field Color table red entry
	BYTE		bGreen;					//@field Color table green entry
	BYTE		bBlue;					//@field Color table blue entry
	STATE * 	pstateNext;				//@field Next state on stack
	STATE * 	pstatePrev;				//@field Previous state on stack

	CParaFormat *pPF;					//@field PF for the state to which 
										//	delta's are applied
										//@cmember Adds or applies PF to state's PF

	STATE(LONG CodePage) {nCodePage = CodePage;}

	BOOL AddPF(const CParaFormat &pf,
				LONG lDefTab, LONG lDocType);
	void DeletePF();					//@cmember Deletes PF for state
	void SetCodePage(LONG CodePage);
};

class CRTFRead ;
class COleObject;


class RTFREADOLESTREAM : public OLESTREAM
{
	OLESTREAMVTBL OLEStreamVtbl;	// @member - memory for  OLESTREAMVTBL
public:
	 CRTFRead *Reader;				// @cmember EDITSTREAM to use

	RTFREADOLESTREAM::RTFREADOLESTREAM ()
	{
		lpstbl = & OLEStreamVtbl ;
	}		
};

#define	NSTYLES		(NHSTYLES + 1)
/*
 *	CRTFRead
 *
 *	@class	RichEdit RTF reader class.
 *
 *	@base	public | CRTFConverter
 *
 */
class CRTFRead : public CRTFConverter
{

//@access Private Methods and Data
	// Lexical analyzer outputs
	LONG		_iParam;				//@cmember Control-word parameter
	TOKEN		_token;					//@cmember Current control-word token
	TOKEN		_tokenLast;				//@cmember Previous token
	BYTE *		_szText;				//@cmember Current BYTE text string
	BYTE		_szParam[cachParamMax];	//@cmember Current parameter string

	// Used for reading in
	BYTE		_rgStyles[NSTYLES];		//@cmember Style handle table
	LONG		_Style;					//@cmember Current style handle
	LONG		_cbBinLeft;				//@cmember cb of bin data left to read
	BYTE *		_pchRTFBuffer;			//@cmember Buffer for GetChar()
	BYTE *		_pchRTFCurrent;			//@cmember Current position in buffer
	BYTE *		_pchRTFEnd;				//@cmember End of buffer
	BYTE *		_pchHexCurr;			//@cmember Current position within
										//  _szText when reading object data
	INT			_nStackDepth;			//@cmember Stack depth
	STATE *		_pstateStackTop;		//@cmember Stack top
	STATE *		_pstateLast;			//@cmember Last STATE allocated
	LONG		_cpThisPara;			//@cmember Start of current paragraph
#ifdef BIDI
	LONG		_cpThisDirection;		//@cmember Start of current Direction run
	LONG		_cpThisJoiner;			//@cmember Start of current Joiner run
#endif	// BIDI

	CParaFormat	_PF;					//@cmember Paragraph format changes

	LONG		_dxCell;				//@cmember Half space betw table cells
	LONG		_cCell;					//@cmember Count of cells in table row
	LONG		_iCell;					//@cmember Current cell in table row 
	LONG		_rgCellX[MAX_TAB_STOPS];//@cmember Cell right boundaries
	LONG		_xRowOffset;			//@cmember Row offset to ensure rows fall along left margin
	DWORD		_dwBorderColor;			//@cmember Border colors
	WORD		_wBorders;				//@cmember Border styles
	WORD		_wBorderSpace;			//@cmember Border/text spaces
	WORD		_wBorderWidth;			//@cmember Border widths
	WORD		_wAlignment;			//@cmember Cache alignment for tables
	
	COleObject *_pobj;					//@cmember Pointer to our object

	union
	{
	  DWORD		_dwFlagsUnion;			// All together now
	  struct
	  {
		WORD	_fFailedPrevObj	 : 1;	//@cmember Fail to get prev object ?
		WORD	_fNeedIcon		 : 1;	//@cmember Objects needs an icon pres
		WORD	_fNeedPres		 : 1;	//@cmember Use stored presenation.
		WORD	_fGetColorYet	 : 1;	//@cmember used for AutoColor detect
		WORD	_fRightToLeftDoc : 1;	//@cmember Document is R to L ?
		WORD	_fReadDefFont	 : 1;	//@cmember Indicates whether we've read a default
										// 	font from the RTF input
		WORD	_fHyperlinkField : 1;	//@cmember TRUE if handling HYPERLINK field

#ifdef PWD_JUPITER
		// GuyBark JupiterJ 49674: Know when we're in an equation field instruction.
		WORD	_fEquationField   : 1;
#endif // PWD_JUPITER

#ifdef DEBUG
		WORD	_fSeenFontTable	 : 1;	//@cmember Indicates we've processed
										//	the RTF file's font table
#endif
		BYTE	_bDocType;				//@cmember Document Type
	  };
	};

#ifdef PWD_JUPITER
	// GuyBark JupiterJ 49674: Know the cp at the start of an equation field instruction.
	LONG		_cpFieldInstruction;
#endif // PWD_JUPITER

	SHORT		_sDefaultFont;			//@cmember Default font to use
	SHORT		_sDefaultLanguage;		//@cmember Default language to use
	SHORT		_sDefaultLanguageFE;	//@cmember Default FE language to use
	SHORT		_sDefaultTabWidth;		//@cmember Default tabwidth to use
	BYTE *		_szSymbolFieldResult;   //@cmember Buffer for SYMBOL field result
	CCharFormat _FieldCF;				//@cmember Formatting for field result
	TEXTFONT *	_ptfField;
	BOOL		_fRestoreFieldFormat;	//@cmember Indicates if the caced format was restored
	TCHAR		_szNumText[cchMaxNumText];	//@cmember Scratch pad for numbered lists
	BYTE		_bTabType;				//@cmember left/right/center/deciml/bar tab
	BYTE		_bTabLeader;			//@cmember none/dotted/dashed/underline 
	BYTE		_bInstFieldCharSet;		//@cmember charset of for HYPERLINK inst field
	int			_nFieldCodePage;
	int			_nCodePage;				//@cmember default codepage (RTF-read-level)
	int			_cchUsedNumText;		//@cmember space used in szNumText

	BYTE *		_szHyperlinkFldinst;	//@cmember Hyperlink Fldinst string	buffer
	int			_cchHyperlinkFldinst;	//@cmember size of Hyperlink Fldinst string	buffer
	int			_cchHyperlinkFldinstUsed;	//@cmember space used in Hyperlink Fldinst string buffer

	BYTE *		_szHyperlinkFldrslt;	//@cmember Hyperlink Fldrslt string	buffer	
	int			_cchHyperlinkFldrslt;	//@cmember size of Hyperlink Fldrslt string	buffer
	int			_cchHyperlinkFldrsltUsed;	//@cmember space used in Hyperlink Fldrslt string buffer

	RTFOBJECT *	_prtfObject;			//@cmember Ptr to RTF Object
	RTFREADOLESTREAM RTFReadOLEStream;	//@cmember RTFREADOLESTREAM to use

	TCHAR *		_szUnicode;				//@cmember String to hold Unicoded chars
	DWORD		_cchMax;				//@cmember Max cch that can still be inserted
	LONG		_cpFirst;				//@cmember Starting cp for insertion

	// object attachment placeholder list
	LONG *		_pcpObPos;
	int			_cobPosFree;
	int 		_cobPos;

	WORD		_wNumberingStyle;		//@cmember Numbering style to use
	WORD		_cbSkipForUnicode; 		//@cmember Bytes to skip over when a Unicode char is read
	BYTE		_bBiDiCharSet;			//@cmember Default system's charset
	BYTE		_bBorder;				//@cmember Current border segment
	BYTE		_bCellBrdrWdths;		//@cmember Current cell border widths

#ifdef PWD_JUPITER
	// GuyBark Jupiter 31960: Have we found any collapsed text while reading a document?
	BOOL		_fFoundCollapsedText;
#endif // PWD_JUPITER

	// Lexical Analyzer Functions
	void	DeinitLex();				//@cmember Release lexer storage
	BOOL	InitLex();					//@cmember Alloc lexer storage
	EC		SkipToEndOfGroup();			//@cmember Skip to matching }
	TOKEN	TokenFindKeyword(BYTE *szKeyword);	//@cmember Find _token for szKeyword
	TOKEN	TokenGetHex();				//@cmember Get next byte from hex input
	TOKEN	TokenGetKeyword();			//@cmember Get next control word
	TOKEN	TokenGetText(BYTE ch);		//@cmember Get text in between ctrl words
	TOKEN	TokenGetToken();			//@cmember Get next {, }, \\, or text
	BOOL 	FInDocTextDest() const;		//@cmember Is reader in document text destination

	// Input Functions
	LONG	FillBuffer();				//@cmember Fill input buffer
	BYTE	GetChar();					//@cmember Return char from input buffer
	BYTE	GetHex();					//@cmember Get next hex value from input
	BYTE	GetHexSkipCRLF();			//@cmember Get next hex value from input
	void	ReadFontName(STATE *pstate, int iAllASCII);//@cmember Copy font name into state
	BOOL	UngetChar();				//@cmember Decrement input buffer ptr
	BOOL	UngetChar(UINT cch);		//@cmember Decrement input buffer ptr 'cch' times

	// Reader Functions
	EC		AddTab();					//@cmember Add tab to current state
	EC		AddText(TCHAR *pch, LONG cch, BOOL fNumber);//@cmember Insert text into range
	void	Apply_CF();					//@cmember Apply _CF changes
	void	Apply_PF();					//@cmember Apply _PF changes
	COLORREF GetColor(DWORD dwMask);	//@cmember Get color _iParam for mask
	LONG	GetStandardColorIndex();	//@cmember Get std index <-> _iparam
	EC		HandleChar(WORD ch);		//@cmember Insert single Unicode
	EC		HandleEndGroup();			//@cmember Handle }
	EC		HandleEndOfPara();			//@cmember Insert EOP into range
	EC		HandleStartGroup();			//@cmember Handle {
	enum { CONTAINS_NONASCII, ALL_ASCII };
	EC		HandleText(BYTE * szText, int iAllASCII);	//@cmember Insert szText into range
	EC		HandleToken();				//@cmember Grand _token switchboard
	void	SelectCurrentFont(INT iFont);//@cmember Select font <p iFont>
	void	SetPlain(STATE *pstate);	//@cmember Setup _CF for \plain
	EC		HandleFieldInstruction();	//@cmember	Handle field instruction
	EC		HandleFieldSymbolInstruction(BYTE *pch);	//@cmember	Handle specific SYMBOL field instruction
	EC		HandleFieldHyperlink(BYTE *pch); //@cmember	Handle hyperlink field instruction
	EC		HandleFieldSymbolFont(BYTE *pch); //@cmember Handle \\f "Facename" in symbol

	// Object functions
	EC		HexToByte(BYTE *rgchHex, BYTE *pb);
	void	FreeRtfObject();
	EC		StrAlloc(TCHAR ** ppsz, BYTE * sz);
	BOOL	ObjectReadFromEditStream(void);
	BOOL	StaticObjectReadFromEditStream(int cb = 0);
	BOOL	ObjectReadSiteFlags( REOBJECT * preobj);
	
	void	SetBorderParm(WORD& Parm, LONG Value);
	BOOL 	CpgInfoFromFaceName(TEXTFONT *ptf);	//@cmember Determines 
										// charset/cpg based on TEXTFONT::szName

	void	SetupEquationOffset(LPSTR psz); // GuyBark JupiterJ: Added this.

	// GuyBark Jupiter 36591: Account for unexpected table RTF.
	void 	HandleIntblToken();

//@access Public Methods
public:
		//@cmember RTF reader constructor
	CRTFRead(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags);
	inline ~CRTFRead();					//@cmember CRTFRead destructor

	LONG	ReadRtf();					//@cmember Main Entry to RTF reader

	LONG	ReadData(BYTE *pbBuffer, LONG cbBuffer); // todo friend
	LONG	ReadBinaryData(BYTE *pbBuffer, LONG cbBuffer);
	LONG	SkipBinaryData(LONG cbSkip);

// member functions/data to test coverage of RTF reader
public:
	void TestParserCoverage();
private:
	CHAR *PszKeywordFromToken(TOKEN token);
	BOOL FTokIsSymbol(TOKEN tok);
	BOOL FTokFailsCoverageTest(TOKEN tok);

	BOOL _fTestingParserCoverage;

#if defined(DEBUG)
private:
	// member data for RTF tag logging
	CRTFLog *_prtflg;
#endif //DEBUG
};

/*
 *	PointsToFontHeight(cHalfPoints)
 *
 *	@func
 *		Convert half points to font heights
 *
 *	@parm int |
 *		sPointSize |		Font height in half points
 *
 *	@rdesc
 *		LONG				The corresponding CCharFormat.yHeight value
 */
#define PointsToFontHeight(cHalfPoints) (((LONG) cHalfPoints) * 10)


/*
 *	CRTFRead::~CRTFRead
 *
 *	@mdesc
 *		Destructor 
 *
 */
inline CRTFRead::~CRTFRead()
{
// TODO: Implement RTF tag logging for the Mac and WinCE
#if defined(DEBUG) && !defined(MACPORT)	&& !defined(PEGASUS)
	if(_prtflg)
	{
		delete _prtflg;
		_prtflg = NULL;
	}
#endif
}
#endif // __RTFREAD_H

