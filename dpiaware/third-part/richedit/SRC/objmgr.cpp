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
 *  @doc    INTERNAL
 *
 *  @module objmgr.cpp.  Object manager implementation | manages a
 *          collection of OLE embedded objects 
 *
 *  Author: alexgo 11/5/95
 *
 */

#include "_common.h"
#include "_objmgr.h"
#include "_edit.h"
#include "_disp.h"
#include "_select.h"

ASSERTDATA

//
//	PUBLIC methods
//

/*
 *	CObjectMgr::GetObjectCount
 *
 *	@mfunc	returns the number of embedded objects currently in
 *			the document.
 *
 *	@rdesc	LONG, the count.  A LONG is used instead of a DWORD for
 *			compatibility with Richedit1.0's IRicheditOle interface.
 */
LONG CObjectMgr::GetObjectCount(void)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetObjectCount");

	return (LONG)_objarray.Count();
}

/*
 *	CObjectMgr::GetLinkCount
 *
 *	@mfunc	returns the number of embedded objects which are links
 *
 *	@rdesc	LONG, the count.  A LONG is used instead of a DWORD for
 *			compatibility with Richedit1.0's IRichEditOle interface.
 *
 */
LONG CObjectMgr::GetLinkCount(void)
{
	LONG count = 0;
	COleObject *pobj;
	DWORD i;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetLinkCount");
		
	for( i = 0; i < _objarray.Count(); i++ )
	{
		pobj = *_objarray.Elem(i);
		if( pobj && pobj->IsLink() )
		{
			count++;
		}
	}
	
	return count;
}

/*
 *	CObjectMgr::GetObjectFromCp()
 *
 *	@mfunc	fetches an object corresponding to the given cp
 *
 *	@rdesc	the object @ a cp; NULL if nothing found
 *
 *	@comm	the algorithm is a modified binary search.  Since the
 *			"typical" access pattern will be to linearly access the
 *			objects, we used the cached index to guess first.  If
 *			that doesn't work, we resort to a binary search.
 */
COleObject *CObjectMgr::GetObjectFromCp(
	DWORD cp)		//@parm the cp for the object
{
	COleObject *pobj = NULL;
	DWORD i = 0;
	
	// no tracing on this method as it's too noisy.
		
	if( _objarray.Count() > 0 )
	{
		if( _lastindex < _objarray.Count() )
		{
			pobj = *_objarray.Elem(_lastindex);
			
			if( pobj && pobj->GetCp() == cp )
			{
				return pobj;
			}
		}
		
		// the quick lookup failed; try a binary search.

		i = FindIndexForCp(cp);

		// because of the insert at end case, i may be equal 
		// to the count of objects().
		if( i < _objarray.Count() )
		{
			pobj = *_objarray.Elem(i);
		}
		else
		{
			pobj = NULL;
		}
	}

	// FindIndex will return a matching or _near_ index.
	// In this case, we only want a matching index
	if( pobj )
	{
		if( pobj->GetCp() != cp )
		{
			pobj = NULL;
		}
		else
		{
			// set the cached index to be the next one,
			// so that somebody walking through objects in
			// cp order will always get immediate hits.
			_lastindex = i + 1;
		}
	}
	
#ifdef DEBUG
	// make sure the binary search found the right thing

	for( i = 0 ; i < _objarray.Count();  i++ )
	{
		COleObject *pobj2 = *_objarray.Elem(i);
	
		if( pobj2 )
		{
			if( *_objarray.Elem(i) == pobj )
			{
				Assert((*_objarray.Elem(i))->GetCp() == cp);
			}
			else
			{
				Assert((*_objarray.Elem(i))->GetCp() != cp);
			}
		}
	}
#endif //DEBUG

	return pobj;
}

/*
 *	CObjectMgr::CountObjects (rcObject, cp)
 *
 *	@mfunc	Count char counts upto <p rcObject> objects away The direction of
 *			counting is determined by the sign of <p rcObject>. 
 *
 *	@rdesc	Return the signed cch counted and set <p rcObject> to count of
 *			objects actually counted.  If <p cobject> <gt> 0 and cp is at
 *			the last object, no change is made and 0 is returned.
 *
 *	@devnote This is called from TOM, which uses LONGs for cp's (because VB
 *			can't use unsigned quantities)
 */
LONG CObjectMgr::CountObjects (
	LONG&	rcObject,		//@parm Count of objects to get cch for
	LONG	cp)				//@parm cp to start counting from
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::CountObjects");

	LONG		iStart, iEnd;
	LONG		iMaxEnd = (LONG)_objarray.Count() - 1;

	if( !rcObject || !_objarray.Count() )
	{
		rcObject = 0;
		return 0;
	}

	iStart = (LONG)FindIndexForCp(cp);

	// if we are looking past either end, return 0

	if( iStart > iMaxEnd && rcObject > 0 )
	{
		rcObject = 0;
		return 0;
	}
	else if( iStart == 0 && rcObject < 0 )
	{
		rcObject = 0;
		return 0;
	}

	// If the index that we found is on an object and
	// we are looking forward, it should be skipped.

	if( iStart < (LONG)_objarray.Count() && 
		(LONG)(*_objarray.Elem(iStart))->GetCp() == cp &&
		rcObject > 0)
	{
		iStart++;
	}

	// Calculate where the end would be.  Note that we use
	// explicit if's to avoid 32 integer arithmetic round
	// off errors in boundary cases.

	if( rcObject < 0 )
	{
		if( (-rcObject) > iStart )
		{
			// subtracting would make us negative, set us to
			// the floor (0).
			iEnd = 0;
			rcObject = -iStart;
		}
		else
		{
			iEnd = iStart + rcObject + 1;
		}
	}
	else
	{
		if( rcObject > iMaxEnd - iStart )
		{
			// adding could make us overflow, set us to the
			// ceiling.
			iEnd = iMaxEnd;
			rcObject = iMaxEnd - iStart + 1;
		}
		else
		{
			iEnd = iStart + rcObject - 1;
		}
	}

	Assert(iEnd >= 0 && iEnd < (LONG)_objarray.Count() );

	return (*_objarray.Elem(iEnd))->GetCp() - cp;
}

			
/*
 *	CObjectMgr::CountObjectsInRange (cpMin, cpMost)
 *
 *	@mfunc	Count the number of objects in the given range.
 *
 *	@rdesc	Return the number of objects.
 *
 */
DWORD CObjectMgr::CountObjectsInRange (
	DWORD	cpMin,	//@parm Beginning of range
	DWORD	cpMost)	//@parm End of range
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::CountObjectsInRange");

	DWORD	iMin, iMost;

	//Get the indexs for the objects at or after cpMin and cpMost
	//respectively.
	iMin = FindIndexForCp(cpMin);
	iMost = FindIndexForCp(cpMost);
	return (LONG)(iMost - iMin);
}


/*
 *	CObjectMgr::GetFirstObjectInRange (cpMin, cpMost)
 *
 *	@mfunc	Get the first object in the given range. 
 *
 *	@rdesc	Pointer to first object in range, or NULL if none.
 *
 */
COleObject * CObjectMgr::GetFirstObjectInRange (
	DWORD	cpMin,	//@parm Beginning of range
	DWORD	cpMost)	//@parm End of range
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetFirstObjectInRange");

	COleObject * pObj = NULL;
	LONG	iObj;
	LONG	iLast = (LONG)_objarray.Count() - 1;

	//Get the index for next object at or after cpMin.
	iObj = FindIndexForCp(cpMin);

	//Make sure this is an existing object.
	if( iObj <= iLast )
	{
		//Make sure it is within the range.
		pObj = *_objarray.Elem(iObj);

		if( pObj && pObj->GetCp() <= cpMost )
		{
			return pObj;
		}
	}

	return NULL;
}


/*
 *	CObjectMgr::GetObjectFromIndex
 *
 *	@mfunc	retrieves the object at the indicated index
 *
 *	@rdesc	a pointer to the object, if found, NULL otherwise
 */
COleObject *CObjectMgr::GetObjectFromIndex(
	DWORD index)		//@parm	the index to use
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::GetObjectFromIndex");

	if( index < _objarray.Count() )
	{
		return *_objarray.Elem(index);
	}
	return NULL;
}

/*
 *	CObjectMgr::InsertObject
 *
 *	@mfunc	inserts an object at the indicated index.  It is the
 *			caller's responsibility to handle inserting any data
 *			(such as WCH_EMBEDDING) into the text stream.
 *
 *	@rdesc	HRESULT
 */
HRESULT CObjectMgr::InsertObject(
	DWORD cp,				//@parm the cp to use
	REOBJECT *preobj,		//@parm the object to insert
	IUndoBuilder *publdr)	//@parm the undo context
{
	COleObject *pobj;
	HRESULT hr;
	pobj = (COleObject *)(preobj->polesite);


 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::InsertObject");

	// Let the client know what we're up to.
	if (_precall)
	{
		hr = _precall->QueryInsertObject(&preobj->clsid, preobj->pstg,
			REO_CP_SELECTION);

		if( hr != NOERROR )
		{
			return hr;
		}
	}

	// set some stuff up first; since we may make outgoing calls, don't
	// change our internal state yet.
	hr = pobj->InitFromREOBJECT(cp, preobj);
	if( hr != NOERROR )
	{
		return hr;
	}

	return RestoreObject(pobj);
}

/*
 *	CObjectMgr::RestoreObject
 *
 *	@mfunc	[re-]inserts the given object into the list of objects
 *			in the backing store
 *
 *	@rdesc	HRESULT
 */
HRESULT CObjectMgr::RestoreObject(
	COleObject *pobj)		//@parm the object to insert
{
	DWORD i;
	COleObject **ppobj;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::RestoreObject");

	i = FindIndexForCp(pobj->GetCp());

	ppobj = _objarray.Insert(i, 1);

	if( ppobj == NULL )
	{
		return E_OUTOFMEMORY;
	}

	*ppobj = pobj;
	pobj->AddRef();

	return NOERROR;
}

/*
 *	CObjectMgr::SetRECallback
 *
 *	@mfunc	sets the callback interface
 *
 *	@rdesc	void
 */
void CObjectMgr::SetRECallback(
	IRichEditOleCallback *precall) //@parm the callback interface pointer
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::SetRECallback");

	if( _precall )
	{
        SafeReleaseAndNULL((IUnknown**)&_precall);
	}

	_precall = precall;

	if( _precall )
	{
		_precall->AddRef();
	}
}

/*
 *	CObjectMgr::SetHostNames
 *
 *	@mfunc	set the host names for this edit instance
 *
 *	@rdesc	NOERROR or E_OUTOFMEMORY
 */
HRESULT CObjectMgr::SetHostNames(
	LPWSTR	pszApp,	//@parm the app name
	LPWSTR  pszDoc)	//@parm the doc name
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::SetHostNames");
	HRESULT hr = NOERROR;

	if( _pszApp )
	{
		delete _pszApp;
		_pszApp = NULL;
	}

	if( _pszDoc )
	{
		delete _pszDoc;
		_pszDoc = NULL;
	}

	if( pszApp )
	{
        size_t cszApp = wcslen(pszApp) + 1;
		_pszApp = new WCHAR[cszApp];
		if( _pszApp )
		{
			wcscpy_s(_pszApp, cszApp, pszApp);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}

	if( pszDoc )
	{
        size_t cszDoc = wcslen(pszDoc) + 1;
		_pszDoc = new WCHAR[cszDoc];
		if( _pszDoc )
		{
			wcscpy_s(_pszDoc, cszDoc, pszDoc);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	
	return hr;
}



/*
 *	CObjectMgr::CObjectMgr
 *
 *	@mfunc constructor
 */
CObjectMgr::CObjectMgr()
{
	_pobjselect = NULL;
	_pobjactive = NULL;
}

/*
 *	CObjectMgr::~CObjectMgr
 *
 *	@mfunc	destructor
 */
CObjectMgr::~CObjectMgr()
{
	DWORD i, count;
	COleObject *pobj;

 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::~CObjectMgr");

	count = _objarray.Count();

	for( i = 0; i < count; i++ )
	{
		pobj = *_objarray.Elem(i);
		// we NULL stuff here to try to protect ourselves
		// better in re-entrancy cases.
		*_objarray.Elem(i) = NULL;
		if( pobj )
		{
			pobj->Close(OLECLOSE_NOSAVE);
  			pobj->MakeZombie();
    		SafeReleaseAndNULL((IUnknown**)&pobj);
		}
	}

	if( _precall )
	{
        SafeReleaseAndNULL((IUnknown**)&_precall);
	}
		
	if( _pszApp )
	{
		delete _pszApp;
	}
	if( _pszDoc )
	{
		delete _pszDoc;
	}
}

/*
 *	CObjectMgr::ReplaceRange
 *
 *	@mfunc	handles the deletion of objects from a given range.  This
 *			method _must_ be called before any floating range notifications
 *			are sent.
 *
 *	@rdesc	void
 */
void CObjectMgr::ReplaceRange(
	DWORD cp,				//@parm the cp starting the deletion
	DWORD cchDel,			//@parm the number of characters deleted
	IUndoBuilder *publdr)	//@parm the undo builder for this actions
{
	DWORD	i;
	DWORD	iDel = (DWORD)-1, 
			cDel = 0;	// index at which to delete && number of objects
						// to delete.
	COleObject *pobj;

	// nothing deleted, don't bother doing anything.
	if( !cchDel )
	{
		return;
	}


	// basically, we loop through all of the objects within the
	// range of deleted text and ask them to delete themselves.
	// We remember the range of objects deleted (the starting index
	// and # of objects deleted) so that we can remove them from
	// the array all at once.

	i = FindIndexForCp(cp);

	while( i < _objarray.Count() )
	{
		pobj = *_objarray.Elem(i);

		if( pobj && pobj->GetCp() >= cp)
		{
			if( pobj->GetCp() < (cp + cchDel) )
			{
				if( _pobjactive == pobj )
				{
					// Deactivate the object just to be on the safe side.
					_pobjactive->DeActivateObj();
					_pobjactive = NULL;
				}

				if( iDel == (DWORD)-1 )
				{
					iDel = i;
				}
				cDel++;

				if (_precall)
				{
					IOleObject *poo;
					if (pobj->GetIUnknown()->QueryInterface(IID_IOleObject,
						(void **)&poo) == NOERROR)
					{
						_precall->DeleteObject(poo);
						poo->Release();
					}
				}

				// if the object was selected, then it obviously
				// can't be anymore!
				if( _pobjselect == pobj )
				{
					_pobjselect = NULL;
				}

				pobj->Delete(publdr);
				*_objarray.Elem(i) = NULL;
				pobj->Release();
			}
			else
			{
				break;
			}
		}
		i++;
	}

	if( cDel )
	{
		_objarray.Remove(iDel, cDel, AF_DELETEMEM);
	}

	return;
}

/*
 *	CObjectMgr::ScrollObjects
 *
 *	@mfunc	informs all objects that scrolling has occured so they can
 *			update if necessary
 *
 *	@rdesc	void
 */
void CObjectMgr::ScrollObjects(
	LONG dx,			//@parm change in the x direction
	LONG dy,			//@parm change in the y direction
	LPCRECT prcScroll)	//@parm the rect that is being scrolled
{
	DWORD i, count;
	COleObject *pobj;
	
	count = _objarray.Count();
	
	for( i = 0; i < count; i++ )
	{
		pobj = *_objarray.Elem(i);
		if( pobj )
		{
			pobj->ScrollObject(dx, dy, prcScroll);
		}
	}
} 	

//
//	PRIVATE methods
//

/*
 *	CObjectMgr::FindIndexForCp
 *
 *	@mfunc	does a binary search to find the index at which an object
 *			at the given cp exists or should be inserted.
 *
 *	@rdesc	DWORD, an index
 */
DWORD CObjectMgr::FindIndexForCp(DWORD cp)
{
	LONG l, r;
	COleObject *pobj = NULL;
	DWORD i = 0;
		
	l = 0; 
	r = _objarray.Count() - 1;
	
	while( r >= l )
	{
		i = (l + r)/2;
		
		pobj = *_objarray.Elem(i);
		
		if( !pobj )
		{
			TRACEWARNSZ("null entry in object table.  Recovering...");
			for( i = 0 ; i < _objarray.Count() -1; i++ )
			{
				pobj = *_objarray.Elem(i);
				if( pobj && pobj->GetCp() >= cp )
				{
					return i;
				}
			}
			return i;
		}
			
		if( pobj->GetCp() == cp )
		{
			return i;
		}
		else if( pobj->GetCp() < cp )
		{
			l = i + 1;
		}
		else
		{
			r = i - 1;
		}
	}

	// yikes! nothing was found.  Fixup i so that
	// it points to the correct index for insertion.

	Assert(pobj || (!pobj && i == 0));

	if( pobj  )
	{
		Assert(pobj->GetCp() != cp);
		if( pobj->GetCp() < cp )
		{
			i++;
		}
	}
					
	return i;
}
						
/*
 *	CObjectMgr::HandleDoubleClick
 *	
 *	@mfunc		Handles a double click message, potentially activating
 *				an object.
 *
 *	@rdesc		BOOL-- TRUE if double click-processing is completely
 *				finished.
 */
BOOL CObjectMgr::HandleDoubleClick(
	CTxtEdit *ped,	//@parm edit context
	const POINT &pt,//@parm the point of the click (WM_LBUTTONDBLCLK wparam)
	DWORD flags)	//@parm flags (lparam)
{
	DWORD cp;
	COleObject *pobj;

	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, 
						"CObjectMgr::HandleDoubleClick");

	cp = ped->_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE);


	pobj = FindObjectAtPointNearCp(pt, cp);

	if (!pobj)
	{
		return FALSE;
	}

	if (_pobjactive != pobj)
	{
		//Deactivate currently active object if any.
		if (_pobjactive)
		{
			_pobjactive->DeActivateObj();
		}

		return pobj->ActivateObj(WM_LBUTTONDBLCLK, MAKELONG(pt.x, pt.y), 
					flags);
	}
	
	return TRUE;
}

/*
 *	CObjectMgr::HandleClick
 *	
 *	@mfunc
 *		The position of the caret is changing.  We need to
 *		Deactivate the active object, if any.  If the change is
 *		because of a mouse click and there is an object at this
 *		cp, we set a new individually selected object. Otherwise
 *		we set the individually selected object to NULL.
 *
 *	@rdesc	returns TRUE if this method set the selection.  Otherwise,
 *		returns FALSE;
 */
ClickStatus CObjectMgr::HandleClick(
	CTxtEdit *ped,	//@parm the edit context
	const POINT &pt)//@parm the point of the mouse click 
{
	COleObject *	pobjnew;//, * pobjold;
	CTxtSelection * psel;
	DWORD			cp;
	LONG			cpMin, cpMost;

 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::HandleClick");

	
	if( _pobjactive )
	{
		_pobjactive->DeActivateObj();
		return CLICK_OBJDEACTIVATED;
	}

	cp = ped->_pdp->CpFromPoint(pt, NULL, NULL, NULL, FALSE);
	pobjnew = FindObjectAtPointNearCp(pt, cp);

	//If we clicked on an object, set the selection to this object.
	//CTxtSelection::UpdateSelection will be called as a result of this
	//and will determine the highlighting.
	if( pobjnew )
	{
		cp = pobjnew->GetCp();
		psel = ped->GetSel();
		if (psel->GetRange(cpMin, cpMost) > 1 &&
			cpMin <= (LONG) cp &&
			(LONG) cp <= cpMost)
		{
			// There is more than one character in the selection
			// And the object is part of the selection.
			// Do not change the selection
			return CLICK_SHOULDDRAG;
		}
		
		// don't reset the selection if the object is already selected
		if( pobjnew != _pobjselect )
		{
			// Freeze the Display while we handle this click
			CFreezeDisplay fd(ped->_pdp);

			psel->SetSelection(cp, cp+1);
			if (GetSingleSelect())
			{
				// Note thate the call to SetSelection may have set selected object to NULL !!!!
				// This can happen in some strange scenarios where our state is out of whack
				AssertSz(GetSingleSelect() == pobjnew, "Object NOT Selected!!");
				return CLICK_OBJSELECTED;
			}
			return CLICK_IGNORED;
		}
		return CLICK_OBJSELECTED;
	}

	return CLICK_IGNORED;
}

/*
 *	CObjectMgr::HandleSingleSelect
 *	
 *	@mfunc
 *		When an object is selected and it is the only thing selected, we do
 *		not highlight it by inverting it.  We Draw a frame and handles around
 *		it.  This function is called either because an object has been
 *		selected and it is the only thing selected, or because we need to
 *		check for an object that used to be in this state but may no longer be.
 *
 */
void CObjectMgr::HandleSingleSelect(
	CTxtEdit *ped,	//@parm the edit context
	DWORD cp,		//@parm cp of object
	BOOL fHiLite)	//@parm is this a call for hding the selection
{
 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, "CObjectMgr::HandleSingleSelect");

	COleObject* pobjnew;

	pobjnew = GetObjectFromCp(cp);

	//This should only be called when we know we have a singley selected
	//object.  However, there are boundary cases (such as inserting an object)
	//where WCH_EMBEDDING is the backing store yet no object exists.  These
	//cases are OK; thus, we check for NULL on pobjnew.
	
	if (pobjnew)
	{
		//The object is the same as the currently selected object (if any)
		//we are deselecting it.  This works like a toggle unless state is messed up.
		//If the object is different, we are replacing the current selected
		//object (if any).
		if (!fHiLite && _pobjselect)
		{
			// This covers _pobjselct == pobjnew  Normal case
			//  and _pobjselect != pobjnew  Degenerate case.
			_pobjselect->SetREOSELECTED(FALSE);
			_pobjselect = NULL;

			//Remove frame/handles from currently selected object.
			ped->_pdp->OnPostReplaceRange(INFINITE, 0, 0, cp, cp + 1);
		}
		else if (fHiLite && pobjnew != _pobjselect)
		{
			// Only do this if we are setting a new selection.
			_pobjselect = pobjnew;
			_pobjselect->SetREOSELECTED(TRUE);

			//Draw frame/handles on newly selected object.
			ped->_pdp->OnPostReplaceRange(INFINITE, 0, 0, cp, cp + 1);
		}
		else
		{
			// We want to hilite the selection but the object is already selected.
			// Or we want to undo hilite on the selection but the selected object is NULL.
			// Do nothing.
		}
	}

	return;
}

/*
 *	CObjectMgr::FindObjectAtPointNearCp
 *
 *	@mfunc		if an object exists at the given point, return
 *				it.  <p cp> is used to help narrow the search
 *				of which objects to look for.
 *
 *	@rdesc		the object, if found.  NULL otherwise
 */
COleObject *CObjectMgr::FindObjectAtPointNearCp(
	const POINT &pt,//@parm the point in question
	DWORD cp)		//@parm hinting cp
{
	LONG i, stop;
	COleObject *pobj;

 	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEINTERN, 
				"CObjectMgr::FindObjectAtPointNearCp");

	// the cp tells us roughly where the object should be in the
	// stream; however, it may be one to the left or one to the right.

	i = FindIndexForCp(cp);

	stop = i + 1;
	i--;

	for( ; i <= stop ; i++ )
	{
		if( i >= 0 && i < (LONG)_objarray.Count() )
		{
			pobj = *_objarray.Elem(i);
			if( pobj && PtInRect(pobj->GetPosRect(), pt) && 
				(pobj->GetCp() == cp || pobj->GetCp() == cp - 1) )
			{
				return pobj;
			}
		}
	}

	return NULL;
}

/*
 *	CObjectMgr::ActivateObjectsAs
 *
 *	@mfunc	Handles a request by the user to activate all objects of a particular
 *		class as objects of another class.
 *
 *	@rdesc
 *		HRESULT				Success code.
 */
HRESULT CObjectMgr::ActivateObjectsAs(REFCLSID rclsid, REFCLSID rclsidAs)
{
	TRACEBEGIN(TRCSUBSYSOLE, TRCSCOPEEXTERN, "CObjectMgr::ActivateObjectsAs");

	COleObject * pobj;
	DWORD cobj, iobj;
	HRESULT hr, hrLatest;

	// Tell the system to treat all rclsid objects as rclsidAs
	hr = pCoTreatAsClass(rclsid, rclsidAs);

	if( hr != NOERROR )
	{
		return hr;
	}

	cobj = GetObjectCount();

	//Go through the objects, letting them decide if
	//they have anything to do for this.
	for (iobj = 0; iobj < cobj; iobj++)
	{
		pobj = GetObjectFromIndex(iobj);
		hrLatest = pobj->ActivateAs(rclsid, rclsidAs);
		//Make hr the latest hresult unless we have previously
		//had an error.
		if( hr == NOERROR )
		{
			hr = hrLatest;
		}
	}
	
	return hr;
}

#ifdef DEBUG
void CObjectMgr::DbgDump(void)
{
	Tracef(TRCSEVNONE, "Object Manager %d objects", _objarray.Count());

	for( DWORD i = 0 ; i < _objarray.Count();  i++ )
	{
		COleObject *pobj = *_objarray.Elem(i);
		if( pobj )
		{
			pobj->DbgDump( i );
		}
	}
}
#endif