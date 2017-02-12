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
 *	@module _DOC.H	CTxtStory declaration |
 *	
 *	Purpose:
 *		Encapsulate the plain-text document data (text blocks, cchText)
 *	
 *	Original Authors: <nl>
 *		Christian Fortini <nl>
 *		Murray Sargent <nl>
 *
 *	History: <nl>
 *		6/25/95	alexgo	commented and cleaned up
 *
 */

#ifndef _DOC_H
#define _DOC_H

#include "_array.h"

#define cbBlockCombine 	CbOfCch(3072)
#define cbBlockMost 	CbOfCch(49152)
#define cbBlockInitial 	CbOfCch(4096)
#define cchGapInitial 	128
#define cchBlkCombmGapI	(CchOfCb(cbBlockCombine) - cchGapInitial)
#define cchBlkInitmGapI	(CchOfCb(cbBlockInitial) - cchGapInitial)

#define cchBlkInsertmGapI	(CchOfCb(cbBlockInitial)*5 - cchGapInitial)

class CDisplay;
class CTxtPtr;
class CTxtArray;

/*
 *	CTxtRun
 *
 *	@class	Formalizes a run of text. A range of text with same attribute, 
 * (see CFmtDesc) or within the same line (see CLine), etc. Runs are kept
 * in arrays (see CArray) and are pointed to by CRunPtr's of various kinds.
 * In general the character position of a run is computed by summing the
 * length of all preceding runs, altho it may be possible to start from
 * some other cp, e.g., for CLines, from CDisplay::_cpFirstVisible.
 */

class CTxtRun
{
//@access Public Methods and Data
public:
	CTxtRun()	{_cch = 0;}			//@cmember Constructor
	DWORD _cch;						//@cmember Count of characters in run
};

/*
 *	CTxtBlk
 *
 *	@class	A text block; a chunk of UNICODE text with a buffer gap to allow 
 *	for easy insertions and deletions.
 *
 *	@base	protected | CTxtRun
 *
 *	@devnote	A text block may have four states: <nl>
 *		NULL:	No data allocated for the block <nl>
 *				<md CTxtBlk::_pch> == NULL 	<nl>
 *				<md CTxtRun::_cch> == 0		<nl>
 *				<md CTxtBlk::_ibGap> == 0	<nl>
 *				<md CTxtBlk::_cbBlock> == 0	<nl>
 *
 *		empty:	All of the available space is a buffer gap <nl>
 *				<md CTxtBlk::_pch> != NULL 	<nl>
 *				<md CTxtRun::_cch> == 0		<nl>
 *				<md CTxtBlk::_ibGap> == 0	<nl>
 *				<md CTxtBlk::_cbBlock> <gt>= 0	<nl>
 *
 *		normal:	There is both data and a buffer gap <nl>
 *				<md CTxtBlk::_pch> != NULL 	<nl>
 *				<md CTxtRun::_cch> != 0		<nl>
 *				<md CTxtBlk::_ibGap> != 0	<nl>
 *				<md CTxtBlk::_cbBlock> <gt>= 0	<nl>
 *		
 *		full:	The buffer gap is of zero size <nl>
 *				<md CTxtBlk::_pch> != NULL 	<nl>
 *				<md CTxtRun::_cch> <gt>= 0	<nl>
 *				<md CTxtBlk::_ibGap> <gt> 0	<nl>
 *				<md CTxtBlk::_cbBlock> == _cch * sizeof(WCHAR) <nl>
 *
 *	The position of the buffer gap is given by _ibGap.  With _cch and _cbBlock,
 *	it's possible to figure out the *size* of the gap by simply calculating:
 *	<nl>
 *		size = _cbBlock - (_cch * sizeof(character))
 *
 */

class CTxtBlk : public CTxtRun
{
	friend class CTxtPtr;
	friend class CTxtArray;

//@access Protected Methods
protected:
									//@cmember	Constructor
	CTxtBlk()	{InitBlock(0);}
									//@cmember	Destructor
	~CTxtBlk()	{FreeBlock();}

									//@cmember	Initializes the block to the 
									//# of bytes given by <p cb>	
	BOOL 	InitBlock(DWORD cb);
									//@cmember	Sets a block to the NULL state
	VOID 	FreeBlock();
									//@cmember	Moves the buffer gap in a 
									//block	
	VOID 	MoveGap(DWORD ichGap);
									//@cmember	Resizes a block to <p cbNew> 
									//bytes
	BOOL 	ResizeBlock(DWORD cbNew);

//@access Private Data
private:
									//@cmember	Pointer to the text data
	WCHAR 	*_pch;			
									//@cmember	BYTE offset of the gap
	DWORD 	_ibGap;			
									//@cmember	size of the block in bytes
	DWORD 	_cbBlock;		
};


/* 
 *	CTxtArray
 *
 *	@class	A dynamic array of <c CTxtBlk> classes
 *
 *	@base public | CArray<lt>CTxtBlk<gt>
 */
class CTxtArray : public CArray<CTxtBlk>
{
	friend class CTxtPtr;

//@access 	Public methods
public:
#ifdef DEBUG
									//@cmember	Invariant support
	BOOL Invariant( void ) const;
#endif	// DEBUG
									//@cmember	Constructor
	CTxtArray();
									//@cmember	Destructor
	~CTxtArray();
									//@cmember	Gets the total number of
									//characters in the array.
	DWORD 	GetCch() const;

	friend 	class CTxtPtr;
	friend 	class CTxtStory;

//@access Private methods
private:
	BOOL 	AddBlock(DWORD itbNew, LONG cb);
									//@cmember	Removes the given number of
									//blocks
	VOID 	RemoveBlocks(DWORD itbFirst, DWORD ctbDel);
									//@cmember	Combines blocks adjacent to itb
	VOID 	CombineBlocks(DWORD itb);
									//@cmember	Splits a block
	BOOL 	SplitBlock(DWORD itb, DWORD ichSplit, DWORD cchFirst, 
				DWORD cchLast, BOOL fStreaming);
									//@cmember	Shrinks all blocks to their minimal
									//size
	VOID 	ShrinkBlocks();		
									//@cmember	Copies a chunk of text into the
									//given location
	LONG	GetChunk(TCHAR **ppch, DWORD cch, TCHAR *pchChunk, DWORD cchCopy) const;
									//@cmember	The total number of characters in the
									//this text array.
	DWORD	_cchText;			
};

/*
 *	CFormatRun
 *
 *	@class	A run of like formatted text, where the format is indicated by an
 *	and index into a format table
 *
 *	@base	protected | CTxtRun
 */
class CFormatRun : public CTxtRun
{
//@access 	Public Methods
public:
	friend class CFormatRunPtr;
	friend class CTxtRange;
	friend class CRchTxtPtr;


								//@cmember	Constructor
	CFormatRun () {_iFormat = -1;}

								//@cmember index of CHARFORMAT or PARAFORMAT 
								//struct
	LONG _iFormat;

};

//@type	CFormatRuns | An array of CFormatRun classes
typedef CArray<CFormatRun> CFormatRuns;


/*
 *	CTxtStory
 *
 *	@class
 *		The text "Document".  Maintains the state information related to the
 *		actual data of a document (such as text, formatting information, etc.)
 */

class CTxtStory
{
	friend class CTxtPtr;
	friend class CRchTxtPtr;
	friend class CReplaceFormattingAE;

//@access Public Methods
public:
	CTxtStory();				//@cmember	Constructor
	~CTxtStory();				//@cmember	Destructor

								//@cmember	Get the length  
    LONG GetTextLength() const
        {return _TxtArray._cchText;}

								//@cmember	Get the Paragraph Formatting runs
	CFormatRuns *GetPFRuns()	{return _pPFRuns;}
								//@cmember	Get the Character Formatting runs
	CFormatRuns *GetCFRuns()	{return _pCFRuns;}
								//@cmember	Converts to plain text from rich
	void DeleteFormatRuns();

#ifdef DEBUG
	void DbgDumpStory(void);	// Debug story dump member
#endif

//@access	Private Data
private:
	CTxtArray 		_TxtArray;	//@cmember	Plain-text runs
	CFormatRuns *	_pCFRuns;	//@cmember	Ptr to Character-Formatting runs
	CFormatRuns *	_pPFRuns;	//@cmember	Ptr to Paragraph-Formatting runs
};

#endif		// ifndef _DOC_H

