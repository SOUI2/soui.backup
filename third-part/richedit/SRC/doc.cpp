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
 *	@module DOC.C	CTxtStory and CTxtArray implementation |
 *	
 *	Original Authors: <nl>
 *		Original RichEdit code: David R. Fulmer <nl>
 *		Christian Fortini	<nl>
 *		Murray Sargent <nl>
 *
 *	History: <nl>
 *		6/25/95	alexgo	Cleanup and reorganization
 *	
 */

#include "_common.h"
#include "_doc.h"
#include "_format.h"


ASSERTDATA


// ===========================  Invariant stuff  ======================

#define DEBUG_CLASSNAME CTxtArray
#include "_invar.h"

// ========================  CTxtArray class  =========================

#ifdef DEBUG

/*
 *	CTxtArray::Invariant
 *
 *	@mfunc	Tests CTxtArray's state
 *
 *	@rdesc	returns TRUE always; failures are indicated by Asserts
 */
BOOL CTxtArray::Invariant( void ) const
{
	static LONG	numTests = 0;
	numTests++;				// how many times we've been called.

	if ( Count() > 0 )
	{

		// make sure total characters stored in the blocks match the length
		// that is stored in _cchText.
		DWORD	cch = 0, i, iMax;
		
		iMax = Count();

		for ( i = 0; i < iMax; i++ )
		{
			CTxtBlk *ptb = Elem(i);

			// ptb shouldn't be NULL since we're within Count elements
			Assert(ptb);

			DWORD currCch = ptb->_cch;
			cch += currCch;
			
			Assert ( currCch >= 0 );
			Assert ( currCch <= CchOfCb(ptb->_cbBlock) );

			// while we're here, check the range of the interblock gaps.
			Assert (ptb->_ibGap >= 0);
			Assert (ptb->_ibGap <= ptb->_cbBlock);


			DWORD cchGap = CchOfCb(ptb->_ibGap);
			Assert ( cchGap >= 0 );
			Assert ( cchGap <= currCch );

		}
		Assert ( cch == GetCch() );
	}

	return TRUE;
}

#endif	// DEBUG

/*
 *	CTxtArray::CTxtArray
 *	
 *	@mfunc		Text array constructor
 *	
 */
CTxtArray::CTxtArray() : CArray<CTxtBlk> ()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::CTxtArray()");

	AssertSz(CchOfCb(cbBlockMost) - cchGapInitial >= cchBlkInitmGapI * 2, 
		"cchBlockMax - cchGapInitial must be at least (cchBlockInitial - cchGapInitial) * 2");

	_cchText = 0;
	// make sure we have no data to initialize
	Assert(sizeof(CTxtArray) == (sizeof(CArray<CTxtBlk>)+ sizeof(_cchText)));
}

/*
 *	CTxtArray::~CTxtArray
 *	
 *	@mfunc		Text array destructor
 */
CTxtArray::~CTxtArray()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::~CTxtArray");

	DWORD itb = Count();
	while(itb--)
	{
		Assert(Elem(itb) != NULL);
		Elem(itb)->FreeBlock();
	}
}

/*
 *	CTxtArray::GetCch()
 *	
 *	@mfunc		Computes and return length of text in this text array
 *
 *	@rdesc		The number of character in this text array
 *
 *	@devnote	This call may be computationally expensive; we have to
 *				sum up the character sizes of all of the text blocks in
 *				the array.
 */
DWORD CTxtArray::GetCch() const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::GetCch");

	_TEST_INVARIANT_
		
	DWORD itb = Count();
	DWORD cch = 0;

	while(itb--)
	{
		Assert(Elem(itb) != NULL);
		cch += Elem(itb)->_cch;
	}
	return cch;
}

/*
 *	CTxtArray::AddBlock(itbNew, cb)
 *	
 *	@mfunc		create new text block
 *	
 *	@rdesc
 *		FALSE if block could not be added
 *		non-FALSE otherwise
 *	
 *	@comm 
 *	Side Effects:  
 *		moves text block array
 */
BOOL CTxtArray::AddBlock(
	DWORD	itbNew,		//@parm	index of the new block 
	LONG	cb)			//@parm size of new block; if <lt>= 0, default is used
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::AddBlock");

	_TEST_INVARIANT_

	CTxtBlk *ptb;

	if(cb <= 0)
		cb = cbBlockInitial;

	AssertSz(cb > 0, "CTxtArray::AddBlock() - adding block of size zero");
	AssertSz(cb <= cbBlockMost, "CTxtArray::AddBlock() - block too big");

	ptb = Insert(itbNew, 1);

	if( !ptb || !ptb->InitBlock(cb))
	{	
		TRACEERRSZSC("TXTARRAT::AddBlock() - unable to allocate new block", E_OUTOFMEMORY);

		// GuyBark Jupiter 40329:
		// The call to Insert() above added another entry into the text block array. The subsequent call InitBlock() will
		// update its _cbBlock saying how big the block is, and its _pch pointer to the space for the text itself.
		// However in the event of oom, we end up here with the pointer NULL, but _cbBlock still set. When I
		// forced this to happen a couple of times on the desktop, RichEdit would later crash or hang. So tidy
		// up the text block array by removing the now-useless block we just added.

		RemoveBlocks(itbNew, 1);

		return FALSE;
	}

	return TRUE;
}

/*
 *	CTxtArray::SplitBlock(itb, ichSplit, cchFirst, cchLast, fStreaming)
 *	
 *	@mfunc		split a text block into two
 *	
 *	@rdesc
 *		FALSE if the block could not be split <nl>
 *		non-FALSE otherwise
 *	
 *	@comm
 *	Side Effects: <nl>
 *		moves text block array
 */
BOOL CTxtArray::SplitBlock(
	DWORD itb, 			//@parm	index of the block to split
	DWORD ichSplit,	 	//@parm	character index within block at which to split
	DWORD cchFirst, 	//@parm desired extra space in first block
	DWORD cchLast, 		//@parm desired extra space in new block
	BOOL fStreaming)	//@parm TRUE if streaming in new text
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::SplitBlock");

	_TEST_INVARIANT_

	LPBYTE pbSrc;
	LPBYTE pbDst;
	CTxtBlk *ptb, *ptb1;

	AssertSz(ichSplit > 0 || cchFirst > 0, "CTxtArray::SplitBlock(): splitting at beginning, but not adding anything");

	AssertSz(itb >= 0, "CTxtArray::SplitBlock(): negative itb");
	ptb = Elem(itb);

	// compute size for first half

	AssertSz(cchFirst + ichSplit <= CchOfCb(cbBlockMost),
		"CTxtArray::SplitBlock(): first size too large");
	cchFirst += ichSplit + cchGapInitial;
	cchFirst = min(cchFirst, CchOfCb(cbBlockMost));

	// compute size for second half

	AssertSz(cchLast + ptb->_cch - ichSplit <= CchOfCb(cbBlockMost),
		"CTxtArray::SplitBlock(): second size too large");
	cchLast += ptb->_cch - ichSplit + cchGapInitial;
	cchLast = min(cchLast, CchOfCb(cbBlockMost));

	// allocate second block and move text to it

	// ***** moves rgtb ***** //
	// if streaming in, allocate a block that's as big as possible so that
	// subsequent additions of text are faster
	// we always fall back to smaller allocations so this won't cause
	// unneccesary errors
	// when we're done streaming we compress blocks, so this won't leave
	// a big empty gap
	if(fStreaming)
	{
		DWORD cb = cbBlockMost;
		const DWORD cbMin = CbOfCch(cchLast);

		while(cb >= cbMin && !AddBlock(itb + 1, cb))
			cb -= cbBlockCombine;
		if(cb >= cbMin)
			goto got_block;
	}
	if(!AddBlock(itb + 1, CbOfCch(cchLast)))
	{
		TRACEERRSZSC("CTxtArray::SplitBlock(): unabled to add new block", E_FAIL);
		return FALSE;
	}

got_block:
	ptb1 = Elem(itb+1);	// recompute ptb after rgtb moves
	ptb = Elem(itb);	// recompute ptb after rgtb moves
	ptb1->_cch = ptb->_cch - ichSplit;
	ptb1->_ibGap = 0;
	pbDst = (LPBYTE) (ptb1->_pch - ptb1->_cch) + ptb1->_cbBlock;
	ptb->MoveGap(ptb->_cch); // make sure pch points to a continuous block of all text in ptb.
	pbSrc = (LPBYTE) (ptb->_pch + ichSplit);
	CopyMemory(pbDst, pbSrc, CbOfCch(ptb1->_cch));
	ptb->_cch = ichSplit;
	ptb->_ibGap = CbOfCch(ichSplit);

	// resize the first block
	if(CbOfCch(cchFirst) != ptb->_cbBlock)
	{
//$ FUTURE: don't resize unless growing or shrinking considerably
		if(!ptb->ResizeBlock(CbOfCch(cchFirst)))
		{
			TRACEERRSZSC("TXTARRA::SplitBlock(): unabled to resize block", E_OUTOFMEMORY);
			return FALSE;
		}
	}

	return TRUE;
}


/*
 *	CTxtArray::ShrinkBlocks()
 *	
 *	@mfunc		Shrink all blocks to their minimal size
 *
 *	@rdesc
 *		nothing
 *	
 */
void CTxtArray::ShrinkBlocks()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::ShrinkBlocks");

	_TEST_INVARIANT_

	DWORD itb = Count();
	CTxtBlk *ptb;

	while(itb--)
	{
		ptb = Elem(itb);
		Assert(ptb);
		ptb->ResizeBlock(CbOfCch(ptb->_cch));
	}
}


/*
 *	CTxtArray::RemoveBlocks(itbFirst, ctbDel)
 *	
 *	@mfunc		remove a range of text blocks
 *	
 *	@rdesc
 *		nothing
 *	
 *	@comm Side Effects: <nl>
 *		moves text block array
 */
VOID CTxtArray::RemoveBlocks(
	DWORD itbFirst, 		//@parm index of first block to remove
	DWORD ctbDel)			//@parm	number of blocks to remove
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::RemoveBlocks");

	_TEST_INVARIANT_

	DWORD itb = itbFirst;
	DWORD ctb = ctbDel;

	AssertSz(itb + ctb <= Count(), "CTxtArray::RemoveBlocks(): not enough blocks");

	while(ctb--)
	{
		Assert(Elem(itb) != NULL);
		Elem(itb++)->FreeBlock();
	}
	
	Remove(itbFirst, ctbDel, AF_KEEPMEM);
}


/*
 *	CTxtArray::CombineBlocks(itb)
 *	
 *	@mfunc		combine adjacent text blocks
 *	
 *	@rdesc
 *		nothing
 *	
 *	@comm 
 *	Side Effects: <nl>
 *		moves text block array
 *	
 *	@devnote
 *		scans blocks from itb - 1 through itb + 1 trying to combine
 *		adjacent blocks
 */
void CTxtArray::CombineBlocks(
	DWORD itb)		//@parm	index of the first block modified
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::CombineBlocks");

	_TEST_INVARIANT_

	DWORD ctb;
	DWORD cbT;
	CTxtBlk *ptb, *ptb1;

	if(itb > 0)
		itb--;

	ctb = min(3, Count() - itb);
	if(ctb <= 1)
		return;

	for(; ctb > 1; ctb--)
	{
		ptb  = Elem(itb);							// Can we combine current
		ptb1 = Elem(itb+1);							//  and next blocks ?
		cbT = CbOfCch(ptb->_cch + ptb1->_cch + cchGapInitial);
		if(cbT <= cbBlockInitial)
		{											// Yes
			if(cbT != ptb->_cbBlock && !ptb->ResizeBlock(cbT))
				continue;
			ptb ->MoveGap(ptb->_cch);				// Move gaps at ends of
			ptb1->MoveGap(ptb1->_cch);				//  both blocks
			CopyMemory(ptb->_pch + ptb->_cch,		// Copy next block text
				ptb1->_pch,	CbOfCch(ptb1->_cch));	//  into current block
			ptb->_cch += ptb1->_cch;
			ptb->_ibGap += CbOfCch(ptb1->_cch);
			RemoveBlocks(itb+1, 1);					// Remove next block
		}
		else
			itb++;
	}
}

/*
 *	CTxtArray::GetChunk(ppch, cch, pchChunk, cchCopy)
 *	
 *	@mfunc
 *		Get content of text chunk in this text array into a string	
 *	
 *	@rdesc
 *		remaining count of characters to get
 */
LONG CTxtArray::GetChunk(
	TCHAR **ppch, 			//@parm ptr to ptr to buffer to copy text chunk into
	DWORD cch, 				//@parm length of pch buffer
	TCHAR *pchChunk, 		//@parm ptr to text chunk
	DWORD cchCopy) const	//@parm count of characters in chunk
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtArray::GetChunk");

	_TEST_INVARIANT_

	if(cch > 0 && cchCopy > 0)
	{
		if(cch < cchCopy)
			cchCopy = cch;						// Copy less than full chunk
		CopyMemory(*ppch, pchChunk, cchCopy*sizeof(TCHAR));
		*ppch	+= cchCopy;						// Adjust target buffer ptr
		cch		-= cchCopy;						// Fewer chars to copy
	}
	return cch;									// Remaining count to copy
}


// ========================  CTxtBlk class  =================================


/*
 *	CTxtBlk::InitBlock(cb)
 *	
 *	@mfunc
 *		Initialize this text block
 *
 *	@rdesc
 *		TRUE if success, FALSE if allocation failed
 */
BOOL CTxtBlk::InitBlock(
	DWORD cb)			//@parm	initial size of the text block
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::InitBlock");

	_pch	= NULL;
	_cch	= 0;
	_ibGap	= 0;
	_cbBlock= cb;

	if (cb)
		_pch = (TCHAR*)PvAlloc(cb, GMEM_ZEROINIT);
	return _pch != 0;
}

/*
 *	CTxtBlk::FreeBlock()
 *	
 *	@mfunc
 *		Free this text block
 *	
 *	@rdesc
 *		nothing
 */
VOID CTxtBlk::FreeBlock()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::FreeBlock");

	FreePv(_pch);
	_pch	= NULL;
	_cch	= 0;
	_ibGap	= 0;
	_cbBlock= 0;
}

/*
 *	CTxtBlk::MoveGap(ichGap)
 *	
 *	@mfunc
 *		move gap in this text block
 *	
 *	@rdesc
 *		nothing
 */
void CTxtBlk::MoveGap(
	DWORD ichGap)			//@parm	new position for the gap
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::MoveGap");

	DWORD cbMove;
	DWORD ibGapNew = CbOfCch(ichGap);
	LPBYTE pbFrom = (LPBYTE) _pch;
	LPBYTE pbTo;

	if(ibGapNew == _ibGap)
		return;

	if(ibGapNew < _ibGap)
	{
		cbMove = _ibGap - ibGapNew;
		pbFrom += ibGapNew;
		pbTo = pbFrom + _cbBlock - CbOfCch(_cch);
	}
	else
	{
		cbMove = ibGapNew - _ibGap;
		pbTo = pbFrom + _ibGap;
		pbFrom = pbTo + _cbBlock - CbOfCch(_cch);
	}

	MoveMemory(pbTo, pbFrom, cbMove);
	_ibGap = ibGapNew;
}


/*
 *	CTxtBlk::ResizeBlock(cbNew)
 *	
 *	@mfunc
 *		resize this text block
 *	
 *	@rdesc	
 *		FALSE if block could not be resized <nl>
 *		non-FALSE otherwise
 *	
 *	@comm
 * 	Side Effects: <nl>
 *		moves text block
 */
BOOL CTxtBlk::ResizeBlock(
	DWORD cbNew)		//@parm	the new size
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtBlk::ResizeBlock");

	TCHAR *pch;
	DWORD cbMove;

	AssertSz(cbNew > 0, "resizing block to size <= 0");
	AssertSz(cbNew <= cbBlockMost, "CTxtBlk::ResizeBlock() - block too big");

	if(cbNew < _cbBlock)
	{
		if(_ibGap != CbOfCch(_cch))
		{
			// move text after gap down so that it doesn't get dropped

			cbMove = CbOfCch(_cch) - _ibGap;
			pch = _pch + CchOfCb(_cbBlock - cbMove);
			MoveMemory(pch - CchOfCb(_cbBlock - cbNew), pch, cbMove);
		}
		_cbBlock = cbNew;
	}
	pch = (TCHAR*)PvReAlloc(_pch, cbNew);
	if(!pch)
		return _cbBlock == cbNew;	// FALSE if grow, TRUE if shrink

	_pch = pch;
	if(cbNew > _cbBlock)
	{
		if(_ibGap != CbOfCch(_cch))		// Move text after gap to end so that
		{								// we don't end up with two gaps
			cbMove = CbOfCch(_cch) - _ibGap;
			pch += CchOfCb(_cbBlock - cbMove);
			MoveMemory(pch + CchOfCb(cbNew - _cbBlock), pch, cbMove);
		}
		_cbBlock = cbNew;
	}

	return TRUE;
}


// ========================  CTxtStory class  ============================

/* 
 *	CTxtStory::CTxtStory
 *
 *	@mfunc	Constructor
 *
 *	@devnote	Automatically allocates a text array.  If we want to have a
 *	completely empty edit control, then don't allocate a story.  NB!
 *	
 */
CTxtStory::CTxtStory()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtStory::CTxtStory");

	_pCFRuns = NULL;
	_pPFRuns = NULL;
}

/*
 *	CTxtStory::~CTxtStory
 *
 *	@mfunc	Destructor
 */
CTxtStory::~CTxtStory()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtStory::~CTxtStory");

	// Remove formatting.
	DeleteFormatRuns();
}

/*
 *	DeleteRuns ()
 *
 *	@mfunc
 *		Helper function for DeleteFormatRuns() below.  Releases
 *		formats used by format run collection before deleting the
 *		collection
 */
void DeleteRuns(CFormatRuns *pRuns, IFormatCache *pf)
{
    if(pRuns)									// Format runs exist
	{
		DWORD n = pRuns->Count();				// n of them
		if(n)
		{
			CFormatRun *pRun = pRuns->Elem(0);
			for( ; n--; pRun++)
				pf->ReleaseFormat(pRun->_iFormat);	// Free run's format
		}
        delete pRuns;
	}	
}

/*
 *	CTxtStory::DeleteFormatRuns ()
 *
 *	@mfunc	Convert to plain - remove format runs
 */
void CTxtStory::DeleteFormatRuns()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtStory::ConvertToPlain");

	IFormatCache * pf;

	GetCharFormatCache((ICharFormatCache **)&pf);
	DeleteRuns(_pCFRuns, pf);
	_pCFRuns = NULL;

	GetParaFormatCache((IParaFormatCache **)&pf);
	DeleteRuns(_pPFRuns, pf);
	_pPFRuns = NULL;
}


#ifdef DEBUG
//This dumps the contents of the CTxtStory
//TxtBlk & FormatRun arrays to the debug output.
void CTxtStory::DbgDumpStory(void)
{
	CTxtBlk * pblk;
	CFormatRun * pcfr;
	CFormatRun * ppfr;
	DWORD ctxtr = 0;
	DWORD ccfr = 0;
	DWORD cpfr = 0;
	DWORD i;

	ctxtr = _TxtArray.Count();

	if (_pCFRuns)
		ccfr = _pCFRuns->Count();
	if (_pPFRuns)
		cpfr = _pPFRuns->Count();

	for(i = 0; i < ctxtr; i++)
	{
		pblk = (CTxtBlk*)_TxtArray.Elem(i);
		Tracef(TRCSEVNONE, "TxtBlk #%d: cch = %d.", (i + 1), pblk->_cch);
	}	

	for(i = 0; i < ccfr; i++)
	{
		pcfr = (CFormatRun*)_pCFRuns->Elem(i);
		Tracef(TRCSEVNONE, "CFR #%d: cch = %d, iFormat = %d.",(i + 1), pcfr->_cch, pcfr->_iFormat);
	}	

	for(i = 0; i < cpfr; i++)
	{
		ppfr = (CFormatRun*)_pPFRuns->Elem(i);
		Tracef(TRCSEVNONE, "PFR #%d: cch = %d, iFormat = %d.",(i + 1), ppfr->_cch, ppfr->_iFormat);
			
	}

	return;
}
#endif

