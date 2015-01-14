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
 *	_FORMAT.H
 *	
 *	Purpose:
 *		CCharFormatArray and CParaFormatArray
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#ifndef _FORMAT_H
#define _FORMAT_H

#include "textserv.h"

#define celGrow     8
#define FLBIT		0x80000000

//+-----------------------------------------------------------------------
// 	Interface IFormatCache
// 	Interface ICharFormatCache
// 	Interface IParaFormatCache
//
//	Format caches - Used by the host to manage the cache of CHARFORMAT
// 	and PARAFORMAT structures.  Note that these interfaces DON'T derive from
//  IUnknown
//------------------------------------------------------------------------

interface IFormatCache
{
	virtual HRESULT		AddRefFormat(LONG iFormat) = 0;
	virtual HRESULT 	ReleaseFormat(LONG iFormat) = 0;
};

template <class FORMAT>
interface ITextFormatCache : public IFormatCache
{
	virtual HRESULT 	Cache(const FORMAT *pFormat, LONG *piFormat) = 0;
	virtual HRESULT		Deref(LONG iFormat, const FORMAT **ppFormat) const = 0;
};

interface ICharFormatCache : public ITextFormatCache<CCharFormat>
{
};

interface IParaFormatCache : public ITextFormatCache<CParaFormat>
{
};

// Access to the format caches (these should be moved to _common.h)
HRESULT	GetCharFormatCache(ICharFormatCache **ppCache);
HRESULT	GetParaFormatCache(IParaFormatCache **ppCache);
void	ReleaseFormats(LONG iCF, LONG iPF);

HRESULT	CreateFormatCaches();
HRESULT	DestroyFormatCaches();


// ===========================  CFixArray  =================================

// This array class ensures stability of the indexes. Elements are freed by
// inserting them in a free list, and the array is never shrunk.
// The first UINT of ELEM is used to store the index of next element in the
// free list.

class CFixArrayBase
{
private:
	char*	_prgel;			// array of elements
	LONG 	_cel;			// total count of elements (including free ones)
	LONG	_ielFirstFree; 	// - first free element
	LONG 	_cbElem;		// size of each element

public:
	CFixArrayBase (LONG cbElem);
	~CFixArrayBase ()				{Free();}

	void*	Elem(LONG iel) const	{return _prgel + iel * _cbElem;}
	LONG 	Count() const			{return _cel;}

	LONG 	Add ();
	void 	Free (LONG ielFirst);
	void 	Free ();

#ifdef DEBUG
	VOID CheckFreeChainFn(LPSTR szFile, INT nLine) const; 
#endif
};

template <class ELEM> 
class CFixArray : public CFixArrayBase
{
public:
	CFixArray () : CFixArrayBase (sizeof(ELEM)) 	{}
	ELEM& 	operator[](LONG iel) const 	{return *(ELEM*)Elem(iel);}
};

#ifdef DEBUG
#define CheckFreeChain()\
			CheckFreeChainFn(__FILE__, __LINE__)
#else
#define CheckFreeChain()
#endif // DEBUG


//================================  CCharFormatArray  ==============================


class CCharFormatArray : public CFixArray<CCharFormat>, public ICharFormatCache
{
protected:
	LONG 	Find(const CCharFormat *pcf) const;

public:
	CCharFormatArray() : CFixArray<CCharFormat>()	{}

	// ICharFormatCache
	virtual HRESULT 	Cache(const CCharFormat *pcf, LONG *picf);
	virtual HRESULT		Deref(LONG icf, const CCharFormat **ppcf) const;
	virtual HRESULT 	AddRefFormat(LONG ipf);
	virtual HRESULT 	ReleaseFormat(LONG ipf);
};


//===============================  CParaFormatArray  ==================================


class CParaFormatArray : public CFixArray<CParaFormat>, public IParaFormatCache
{
protected:	
	LONG 	Find(const CParaFormat *ppf) const;

public:
	CParaFormatArray() : CFixArray<CParaFormat>()	{}

	// IParaFormatCache
	virtual HRESULT 	Cache(const CParaFormat *ppf, LONG *pipf);
	virtual HRESULT		Deref(LONG ipf, const CParaFormat **pppf) const;
	virtual HRESULT 	AddRefFormat(LONG ipf);
	virtual HRESULT 	ReleaseFormat(LONG ipf);
};


#endif

