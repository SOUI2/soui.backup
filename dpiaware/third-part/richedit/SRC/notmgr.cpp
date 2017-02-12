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
 *	NOTMGR.C
 *
 *	Purpose:
 *		Notification Manager implemenation
 *
 *	Author:
 *		AlexGo	6/5/95
 */

#include "_common.h"
#include "_notmgr.h"

ASSERTDATA

/*
 *	CNotifyMgr::CNotifyMgr ()
 */
CNotifyMgr::CNotifyMgr()
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::CNotifyMgr");

	_pitnlist = NULL;
}

/*
 *	CNotifyMgr::~CNotifyMgr ()
 *
 */
CNotifyMgr::~CNotifyMgr()
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::~CNotifyMgr");

	ITxNotify *plist;

	for( plist = _pitnlist; plist != NULL; plist = plist->_pnext )
	{
		plist->Zombie();
	}

	TRACEERRSZSC("CNotifyMgr::~CNotifyMgr(): zombie(s) exist", _pitnlist != 0);
}

/*
 *	CNotifyMgr::Add (pITN)
 *
 *	Purpose:
 *		Adds a notification sink to the list
 *
 *	Algorithm:
 *		puts the entry at the *front* of the notification list, so
 *		that high frequency entries (like ranges and text pointers
 *		existing on the stack) can be added and removed efficiently
 *
 */
void CNotifyMgr::Add( ITxNotify *pITN )
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::Add");

		pITN->_pnext = _pitnlist;
		_pitnlist = pITN;
}

/*
 *	CNotifyMgr::Remove (pITN)
 *
 *	Purpose:
 *		removes a notification sink from the list
 */
void CNotifyMgr::Remove( ITxNotify *pITN )
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::Remove");

	ITxNotify *plist, **ppprev;
	plist = _pitnlist;
	ppprev = &_pitnlist;

	while( plist != NULL )
	{
		if( plist == pITN )
		{
			*ppprev = plist->_pnext;
			break;
		}
		ppprev = &(plist->_pnext);
		plist = plist->_pnext;
	}
}

/*
 *	CNotifyMgr::NotifyPreReplaceRange (pITNignore, cp, cchDel, cchNew)
 *
 *	Purpose:
 *		send an OnReplaceRange notification to all sinks (except pITNignore)
 *
 *	Arguments:
 *		pITNignore		-- the notification sink to ignore.  Typically, this
 *						   is the TxtPtr/etc that is actually making the
 *						   ReplaceRange modification
 *		cp				-- the cp at which the replace range operation starts
 *						   (conceptually, it's a cpMin)
 *		cchDel			-- the number of characters after cp that are deleted
 *		cchNew			-- the number of characters inserted after cp
 *		cpFormatMin		-- the minimum cp for a formatting change
 *		cpFormatMax		-- the max cp for a format change.
 *
 */
void CNotifyMgr::NotifyPreReplaceRange( ITxNotify *pITNignore, DWORD cp, 
		DWORD cchDel, DWORD cchNew, DWORD cpFormatMin, DWORD cpFormatMax)
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::NotifyPreReplaceRange");

	ITxNotify *plist;

	for( plist = _pitnlist; plist != NULL; plist = plist->_pnext )
	{
		if( plist != pITNignore )
		{
			plist->OnPreReplaceRange( cp, cchDel, cchNew, cpFormatMin, 
				cpFormatMax );
		}
	}
}

/*
 *	CNotifyMgr::NotifyPostReplaceRange (pITNignore, cp, cchDel, cchNew)
 *
 *	Purpose:
 *		send an OnReplaceRange notification to all sinks (except pITNignore)
 *
 *	Arguments:
 *		pITNignore		-- the notification sink to ignore.  Typically, this
 *						   is the TxtPtr/etc that is actually making the
 *						   ReplaceRange modification
 *		cp				-- the cp at which the replace range operation starts
 *						   (conceptually, it's a cpMin)
 *		cchDel			-- the number of characters after cp that are deleted
 *		cchNew			-- the number of characters inserted after cp
 *		cpFormatMin		-- the minimum cp for a formatting change
 *		cpFormatMax		-- the max cp for a format change.
 *
 */
void CNotifyMgr::NotifyPostReplaceRange( ITxNotify *pITNignore, DWORD cp, 
		DWORD cchDel, DWORD cchNew, DWORD cpFormatMin, DWORD cpFormatMax )
{
	TRACEBEGIN(TRCSUBSYSNOTM, TRCSCOPEINTERN, "CNotifyMgr::NotifyPostReplaceRange");

	ITxNotify *plist;

	for( plist = _pitnlist; plist != NULL; plist = plist->_pnext )
	{
		if( plist != pITNignore )
		{
			plist->OnPostReplaceRange( cp, cchDel, cchNew, cpFormatMin,
				cpFormatMax );
		}
	}
}



