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
 *	@module - FORMAT.C
 *		CCharFormatArray and CParaFormatArray classes |
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#include "_common.h"
#include "_format.h"


ASSERTDATA

// ===============================  CFixArrayBase  =================================


CFixArrayBase::CFixArrayBase(LONG cbElem)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::CFixArrayBase");
	
	_prgel = NULL; 
	_cel = 0; 
	_ielFirstFree = 0; 
	_cbElem = cbElem;
}


/*
 *	CFixArrayBase::Add()
 *
 *	@mfunc	
 *		Return index of new element, reallocing if necessary
 *
 *	@rdesc
 *		Index of new element.
 *
 *	@comm
 *		Free elements are maintained in place as a linked list indexed
 *		by a chain of cbSize entries with their sign bits set and the
 *		rest of the entry giving the index of the next element on the
 *		free list.  The list is terminated by a 0 entry. This approach
 *		enables element 0 to be on the free list.
 */
LONG CFixArrayBase::Add()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::Add");

	char *pel;
	LONG iel, ielRet;

	if (_ielFirstFree)					// Return first element of free list
	{
		ielRet = _ielFirstFree & ~FLBIT;
		_ielFirstFree = *(INT*)(_prgel + ielRet * _cbElem);
	}
	else								// All lower positions taken: need 
	{									//  to add another celGrow elements
		pel = (char*)PvReAlloc(_prgel, (_cel + celGrow) * _cbElem);
		if(!pel)
			return -1;

		//clear out the *end* of the newly allocated memory
		ZeroMemory((pel + (_cel * _cbElem)), (celGrow * _cbElem));

		_prgel = pel;

		ielRet = _cel;					// Return first one added 
		iel = _cel + 1;
		_cel += celGrow;

		// Add elements _cel+1 thru _cel+celGrow-1 to free list. The last of
		// these retains a cbSize = 0, stored by fZeroFill in Alloc
		_ielFirstFree = iel | FLBIT;

        if (_cbElem > 0)
        {
    		for(pel = _prgel + iel * _cbElem;
    			++iel < _cel;
    			pel += _cbElem)
    		{
    			*(INT*)pel = iel | FLBIT;
    		}
        }
	}		
	return ielRet;
}

void CFixArrayBase::Free(LONG ielFree)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::Free");

	// Simply add it to free list
	*(INT*)(_prgel + ielFree * _cbElem) = _ielFirstFree;
	_ielFirstFree = ielFree | FLBIT;
}

void CFixArrayBase::Free()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::Free");

	FreePv(_prgel);
	_prgel = NULL;
	_cel = 0;
	_ielFirstFree = 0;
}

#ifdef DEBUG

void CFixArrayBase::CheckFreeChainFn(LPSTR szFile, INT nLine) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CFixArrayBase::CheckFreeChainFn");

	LONG cel = 0;
	LONG iel = _ielFirstFree;
	LONG ielT;

	while(iel)
	{
		ielT = *(INT*)(_prgel + (iel & ~FLBIT) * _cbElem);

		if((LONG)(ielT & ~FLBIT) > _cel)
			Tracef(TRCSEVERR, "AttCheckFreeChainCF(): elem %ld points to out of range elem %ld", iel, ielT);

		iel = ielT;
		if(++cel > _cel)
		{
			Dbug32AssertSzFn("CFixArrayBase::CheckFreeChain() - CF free chain seems to contain an infinite loop", szFile, nLine);
			return;
		}
	}
}

#endif


// ===========================  CCharFormatArray  ===========================================

HRESULT CCharFormatArray::Deref(LONG iCF, const CCharFormat **ppcf) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::Deref");

	if(!ppcf)
		return E_INVALIDARG;

	*ppcf = &(*this)[iCF];

	return S_OK;
}

HRESULT CCharFormatArray::ReleaseFormat(LONG iCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::ReleaseFormat");

	CLock	lock;

	CheckFreeChain();

	if(iCF >= 0)								// Ignore default iCF
	{
		AssertSz(((INT) (*this)[iCF].cbSize) > 0, "CCharFormatArray::AttFreeCF(): already free");

		if(((INT) --(*this)[iCF].cbSize) == 0)	// Entry no longer referenced
			Free (iCF);							// Add it to the free chain
	}
	return S_OK;
}

HRESULT CCharFormatArray::AddRefFormat(LONG iCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::AddRefFormat");

	CLock	lock;

	CheckFreeChain();

	if(iCF >= 0)
	{
    	AssertSz(((INT) (*this)[iCF].cbSize) > 0, "CCharFormatArray::AddRefFormat(): add ref to free elem");
		(*this)[iCF].cbSize++;
	}
	return S_OK;
}


LONG CCharFormatArray::Find(const CCharFormat *pcf) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::Find");
	CLock	lock;

	LONG iCF;
	CCharFormat *pcfMatch;

	#define QUICKCRCSEARCHSIZE	15	// Must be 2^n - 1 for quick MOD
									//  operation, it is a simple hash.
 	static struct {
		BYTE	bCRC;
		LONG	iCF;
	} quickCrcSearch[QUICKCRCSEARCHSIZE+1];
 	BYTE	bCRC;
	WORD	hashKey;

	CheckFreeChain();

	// check our cache before going sequential.
	bCRC = pcf->bCRC;
	hashKey = bCRC & QUICKCRCSEARCHSIZE;
	if ( bCRC == quickCrcSearch[hashKey].bCRC )
	{
		iCF = quickCrcSearch[hashKey].iCF - 1;
		if(iCF >= 0 && iCF < Count())
		{
			pcfMatch = &(*this)[iCF];
			if((INT)pcfMatch->cbSize > 0 && pcfMatch->Compare(pcf))
			{
				return iCF;
			}
		}
	}

	for(iCF = 0; iCF < Count(); iCF++)
	{
		pcfMatch = &(*this)[iCF];
        
		// cbSize is used as ref count. cbSize < 0 means entry not in use
		// and cbSize is index of next free entry.
        // Compare the two, ignoring cbSize and dwMask
		if((INT)pcfMatch->cbSize > 0 && pcfMatch->Compare(pcf))
		{
			quickCrcSearch[hashKey].bCRC = bCRC;
			quickCrcSearch[hashKey].iCF = iCF + 1;
			return iCF;
		}
	}
	return -1;
}


HRESULT CCharFormatArray::Cache(const CCharFormat *pcf, LONG* piCF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CCharFormatArray::Cache");
	CLock	lock;

	LONG	iCF = Find(pcf);

	if(iCF >= 0)
		(*this)[iCF].cbSize++;		// cbSize is used as ref count
	else
	{
		iCF = Add();
		if(iCF < 0)
			return E_OUTOFMEMORY;
		(*this)[iCF].Set(pcf);		// Set entry iCF to *pcf
		(*this)[iCF].cbSize = 1;	// cbSize is used as ref count
	}					 

	CheckFreeChain();
	
	if(piCF)
		*piCF = iCF;

	return S_OK;
}


// ===============================  CParaFormatArray  ===========================================

HRESULT CParaFormatArray::Deref(LONG iPF, const CParaFormat **ppPF) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::Deref");
	CLock	lock;

	if(!ppPF)
		return E_INVALIDARG;

	*ppPF = &(*this)[iPF];

	return S_OK;
}

HRESULT CParaFormatArray::ReleaseFormat(LONG iPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::ReleaseFormat");
	CLock	lock;

	CheckFreeChain();

	if(iPF >= 0)
	{
		AssertSz(((INT) (*this)[iPF].cbSize) > 0, "AttFreePF(): already free");

		if(!--(*this)[iPF].cbSize)				// Entry no longer referenced
			Free(iPF);							// Add it to the free chain
	}

	return S_OK;
}

HRESULT CParaFormatArray::AddRefFormat(LONG iPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::AddRefFormat");
	CLock	lock;

	CheckFreeChain();

	if(iPF >= 0)
	{
    	AssertSz(((INT) (*this)[iPF].cbSize) > 0, "CParaFormatArray::AddRefFormat(): add ref to free elem");
		(*this)[iPF].cbSize++;
	}
	return S_OK;
}

LONG CParaFormatArray::Find(const CParaFormat *pPF) const
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::Find");
	CLock	lock;

	LONG iPF;
	CParaFormat *pPFMatch;

	CheckFreeChain();
	
	for(iPF = 0; iPF < Count(); iPF++)
	{
		// cbSize is used as ref count
		// < 0 means entry not in use and cbSize is index of next free entry
		pPFMatch = &(*this)[iPF];
		if(((INT) pPFMatch->cbSize) > 0 && pPFMatch->Compare(pPF))
		{
			return iPF;
		}
	}
	return -1;
}

HRESULT CParaFormatArray::Cache(const CParaFormat *pPF, LONG *piPF)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CParaFormatArray::Cache");
	CLock	lock;

	LONG	iPF = Find(pPF);

	if(iPF >= 0)
		(*this)[iPF].cbSize++;		// cbSize is used as ref count
	else
	{	
		iPF = Add();
		if(iPF < 0)
			return E_OUTOFMEMORY;
		(*this)[iPF].Set(pPF);
		(*this)[iPF].cbSize = 1;	// cbSize is used as ref count
	}

	CheckFreeChain();
	
	if(piPF)
		*piPF = iPF;
	
	return S_OK;
}


// ==================================  Factories  ===========================================

static CCharFormatArray *pCFCache = NULL;		// CCharFormat cache
static CParaFormatArray *pPFCache = NULL;	 	// CParaFormat cache

HRESULT CreateFormatCaches()					// Called by DllMain()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CreateFormatCaches");
	CLock	lock;

	pCFCache = new CCharFormatArray();
	if(!pCFCache)
		return E_OUTOFMEMORY;
     
    pPFCache = new CParaFormatArray();
	if(!pPFCache)
	{
		delete pCFCache;
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

HRESULT DestroyFormatCaches()					// Called by DllMain()
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "DeleteFormatCaches");

	delete pCFCache;
	delete pPFCache;
	return NOERROR;
}

HRESULT	GetCharFormatCache(ICharFormatCache **ppCache)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "GetCharFormatCache");
	// NB!  If this ever does anything but return a global variable,
	// be sure to make the function multi-thread safe.
	if(!ppCache)
		return E_INVALIDARG;

	*ppCache = pCFCache;
	return S_OK;
}

HRESULT	GetParaFormatCache(IParaFormatCache **ppCache)
{
	TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "GetParaFormatCache");
	// NB!  If thes ever does anything but return a global variable,
	// be sure to make the function multi-thread safe.
	if(!ppCache)
		return E_INVALIDARG;

	*ppCache = pPFCache;
	return S_OK;
}

/*
 *	ReleaseFormats(iCF, iPF)
 *
 *	@mfunc
 *		Release char and para formats corresponding to the indices <p iCF>
 *		and <p iPF>, respectively
 */
void ReleaseFormats (
	LONG iCF,			//@parm CCharFormat index for releasing
	LONG iPF)			//@parm CParaFormat index for releasing
{
	AssertSz(pCFCache && pPFCache,
		"ReleaseFormats: uninitialized format caches");
	pCFCache->ReleaseFormat(iCF);
	pPFCache->ReleaseFormat(iPF);
}



