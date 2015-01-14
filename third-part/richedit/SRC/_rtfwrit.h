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
 *	@module _RTFWRIT.H -- RichEdit RTF Writer Class Definition |
 *
 *	Description:
 *		This file contains the type declarations used by the RTF writer
 *		for the RICHEDIT control
 *
 *	Authors: <nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szUnicode.
 */
#ifndef __RTFWRIT_H
#define __RTFWRIT_H

#include "_rtfconv.h"
extern KEYWORD rgKeyword[];

#define PUNCT_MAX	1024


class CRTFWrite ;


class RTFWRITEOLESTREAM : public OLESTREAM
{
	OLESTREAMVTBL OLEStreamVtbl;	// @member - memory for  OLESTREAMVTBL
public:
	 CRTFWrite *Writer;				// @cmember CRTFwriter to use

	RTFWRITEOLESTREAM::RTFWRITEOLESTREAM ()
	{
		lpstbl = & OLEStreamVtbl ;
	}		
};

enum									// Control-Word-Format indices
{
	CWF_STR, CWF_VAL, CWF_GRP, CWF_AST, CWF_GRV
};

#define chEndGroup RBRACE

/*
 *	CRTFWrite
 *
 *	@class	RTF writer class.
 *
 *	@base	public | CRTFConverter
 *
 */
class CRTFWrite : public CRTFConverter
{
private:
	LONG		_cchBufferOut;			// @cmember # chars in output buffer
	LONG		_cchOut;				// @cmember Total # chars put out
	LONG		_cbCharLast;			// @cmember # bytes in char last written
	BYTE		_fBullet;				// @cmember Currently in a bulleted style
	BYTE		_fBulletPending;		// @cmember Set if next output should bull
	BYTE		_fNeedDelimeter;		// @cmember Set if next char must be nonalphanumeric
	BYTE        _fIncludeObjects;       // @cmember Set if objects should be included in stream
	BYTE		_fCheckInTable;			// @cmember If set and in table, output intbl stuff
	BYTE		_fRangeHasEOP;			// @cmember Set if _prg has EOP
	char *		_pchRTFBuffer;			// @cmember Ptr to RTF write buffer
	BYTE *		_pbAnsiBuffer;			// @member Ptr to buffer used for conversion
	char *		_pchRTFEnd;				// @cmember Ptr to RTF-write-buffer end
	LONG		_symbolFont;			// @cmember Font number of Symbol used by Bullet style
	RTFWRITEOLESTREAM RTFWriteOLEStream;// @cmember RTFWRITEOLESTREAM to use
	LONG		_nHeadingStyle;			// @cmember Deepest heading # found
	LONG		_nNumber;				// @cmember Current number in para (1-based)
	LONG		_nFont;					// @cmember Current number font index
	LONG		_cpg;					// @cmember Current number code page
	const CParaFormat *_pPF;			// @cmember Current para format

#ifdef PWD_JUPITER
    // GuyBark: Added this...
    BYTE        _UnicodeBytesPerChar;   // @cmember Current value of \uc, (count of bytes in 
                                        // non-unicode representation of unicode characters).
    int         _pwdCurrentFont;        // Keep a track of the current font selected.
    int         _pwdDefaultJFont;       // Id of a default J font if required.
#endif // PWD_JUPITER
										// @cmember Build font/color tables
	EC			BuildTables		(CFormatRunPtr& rpCF, CFormatRunPtr &rpPF,
								LONG cch);
	inline void		CheckDelimeter()		// @cmember	Put ' ' if need delimeter
	{
		if(_fNeedDelimeter)
		{
			_fNeedDelimeter = FALSE;
			PutChar(' ');
		}
	};

	BOOL		CheckInTable	(BOOL fPutIntbl);
										// @cmember Stream out output buffer
	BOOL		FlushBuffer		();
										// @cmember Get index of <p colorref>
	LONG		LookupColor		(COLORREF colorref);
										// @cmember Get font index for <p pCF>
	LONG		LookupFont		(CCharFormat const * pCF);
										// @cmember "printf" to output buffer
	BOOL _cdecl printF			(CONST CHAR * szFmt, ...);
										// @cmember Put char <p ch> in output buffer
	EC			PutBorders		(BOOL fInTable);
	BOOL		PutChar			(CHAR ch);
										// @cmember Put control word <p iCtrl> with value <p iValue> into output buffer
	BOOL		PutCtrlWord		(LONG iFormat, LONG iCtrl, LONG iValue = 0);
										// @cmember Put string <p sz> in output buffer
	BOOL		Puts			(CHAR const * sz, LONG cb);
										// @cmember Put "sprintf" to arg buffer
	LONG _cdecl sprintF			(LONG cb, CHAR *szBuf, CONST CHAR * szFmt, ...);
										// @cmember Write char format <p pCF>
	EC			WriteCharFormat	(const CCharFormat *pCF);
	EC			WriteColorTable	();		// @cmember Write color table
	EC			WriteFontTable	();		// @cmember Write font table
	EC			WriteInfo		();		// @cmember Write document info
										// @cmember Write para format <p pPF>
	EC			WriteParaFormat	(const CRchTxtPtr * prtp);
										// @cmember Write PC data <p szData>
	EC			WritePcData		(const TCHAR * szData, INT nCodePage = CP_ACP, BOOL fIsDBCS = FALSE );
										// @cmember Write <p cch> chars of text <p pch>
	EC			WriteText(LONG cwch, LPCWSTR lpcwstr, INT nCodePage, BOOL fIsDBCS);
	EC			WriteTextChunk(LONG cwch, LPCWSTR lpcwstr, INT nCodePage, BOOL fIsDBCS);


// OBJECT
	EC			WriteObject		(LONG cp, COleObject *pobj);
	BOOL		GetRtfObjectMetafilePict(HGLOBAL hmfp, RTFOBJECT &rtfobject, SIZEL &sizelGoal);
	BOOL		GetRtfObject(REOBJECT &reobject, RTFOBJECT &rtfobject);
	EC			WriteRtfObject(RTFOBJECT & rtfOb, BOOL fPicture);
	BOOL		ObjectWriteToEditstream(REOBJECT &reObject, RTFOBJECT &rtfobject);
	EC			WritePicture(REOBJECT &reObject,RTFOBJECT  &rtfObject);
	EC			WriteDib(REOBJECT &reObject,RTFOBJECT  &rtfObject);

	enum 		{ MAPTOKWD_ANSI, MAPTOKWD_UNICODE };
	inline BOOL	MapsToRTFKeywordW(WCHAR wch);
	inline BOOL	MapsToRTFKeywordA(char ch);
	int 		MapToRTFKeyword(void *pv, int cch, int iCharEncoding);

public:
										// @cmember Constructor
	CRTFWrite(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags);
	~CRTFWrite()						// @cmember Destructor
	{
		if(_pbAnsiBuffer)
			FreePv(_pbAnsiBuffer);
	};


	LONG		WriteRtf();				// @cmember Main write entry used by
										//  CLiteDTEngine
	LONG		WriteData		(BYTE * pbBuffer, LONG cbBuffer);
	LONG		WriteBinData	(BYTE * pbBuffer, LONG cbBuffer);

};										


#endif // __RTFWRIT_H

