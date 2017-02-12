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
 *
 *	@doc	INTERNAL
 *
 *	@module	_CALLMGR.H	CCallMgr declaration |
 *
 *	Purpose:  The call manager controls various aspects of
 *		a client call chain, including re-entrancy management,
 *		undo contexts, and change notifications.
 *
 *	Author:	<nl>
 *		alexgo 2/8/96
 *
 */
#ifndef _CALLMGR_H
#define _CALLMGR_H

class CTxtEdit;
class CCallMgr;

#include "textserv.h"

enum CompName
{
	COMP_UNDOBUILDER	= 1,	// currently, these two are only compatible
	COMP_REDOBUILDER	= 2,	// with CGenUndoBuilder undo contexts
	COMP_UNDOGUARD		= 3,
	COMP_SELPHASEADJUSTER = 4
};

/*
 *	IReEntrantComponent
 *
 *	@class	A base class/interface for objects that want to work with the call
 *			manager for special re-entrancy requirements.
 *
 *			This class is similar in spirit to ITxNotify, thus, it contains
 *			private data only accessible to CCallMgr
 */
class IReEntrantComponent
{
	friend class CCallMgr;

//@access	Public Data
public:
	virtual void OnEnterContext() = 0;		//@cmember Called when a 
											// context is entered
//@access	Private Data
private:
	CompName				_idName;		//@cmember the name for this
											// component
	IReEntrantComponent *	_pnext;			//@cmember the next one in the
											// list
};
		
/*
 *
 *	CCallMgr
 *
 *	@class	A stack-based class to handle re-entrancy and notification
 *			management.  CCallMgr's are created on the stack on every
 *			important entry point.  If one already exists (i.e. the
 *			edit control has been re-entered), the Call Manager will 
 *			adjust appropriately.
 */

class CCallMgr
{
	friend class CGenUndoBuilder;

//@access	Public Methods
public:
	// Notification Methods
	void SetChangeEvent(CHANGETYPE fType);	//@cmember something changed
	void ClearChangeEvent();				//@cmember ignore the change
											//@cmember did something change?
	BOOL GetChangeEvent()	{return (_pPrevcallmgr) ? 
								_pPrevcallmgr->GetChangeEvent() : _fChange;}

	void SetNewUndo();						//@cmember new undo action added
	void SetNewRedo();						//@cmember new redo action added
	void SetMaxText();						//@cmember max text length reached
	void SetSelectionChanged();				//@cmember the selection changed
	void SetOutOfMemory();					//@cmember out of memory hit
	void SetInProtected(BOOL f);			//@cmember set if we are in the
											// the protected notification cback
	BOOL GetInProtected();					//@cmember retrieve the InProtected
											// flag

	// SubSystem Management methods
											//@cmember Register a component
											// for this call context
	void RegisterComponent(IReEntrantComponent *pcomp, CompName name);
											//@cmember Revoke a component from
											// this call context
	void RevokeComponent(IReEntrantComponent *pcomp);
	
	IReEntrantComponent *GetComponent(CompName name);//@cmember Retrieves a
											// registered component by name

	// General Methods
	BOOL IsReEntered() { return !!_pPrevcallmgr;} //@cmember Returns TRUE if
											// we are in a re-entrant state.
	BOOL IsZombie() { return !_ped;}		//@cmember Zombie call

	// Constructor/Destructor
	CCallMgr(CTxtEdit *ped);				//@cmember constructor
	~CCallMgr();							//@cmember destructor


//@access	Private Methods and data
private:

	void SendAllNotifications();			//@cmember Flush any cached 
											// notifications 
	void NotifyEnterContext();				//@cmember Notify registered 
											// components of a new context.

	CTxtEdit *		_ped;					//@cmember the current edit context
	CCallMgr *		_pPrevcallmgr;			//@cmember the next highest call
											// manager in
	IReEntrantComponent *_pcomplist;		//@cmember the list of components
											// registered for this call context

	unsigned long	_fTextChanged	:1;		//@cmember the text changed
	unsigned long	_fChange		:1;		//@cmember generic change
	unsigned long	_fNewUndo		:1;		//@cmember new undo action added
	unsigned long	_fNewRedo		:1;		//@cmember new redo action added
	unsigned long	_fMaxText		:1;		//@cmember max text length reached
	unsigned long	_fSelChanged	:1;		//@cmember the selection changed
	unsigned long	_fOutOfMemory	:1;		//@cmember out of memory

	unsigned long	_fInProtected	:1;		//@cmember if we are in the
											// EN_PROTECTED notification.
};

#endif // _CALLMGR_H

