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
 *	@module _ANTIEVT.H |
 *
 *
 *	Purpose:
 *		Class declarations for common anti-event objects
 *
 *	Author:
 *		alexgo  3/25/95
 */

#ifndef __ANTIEVT_H__
#define __ANTIEVT_H__

#include "_frunptr.h"

class CTxtEdit;
class CAntiEventDispenser;
class COleObject;


/*
 *	CBaseAE
 *
 *	@class
 *		Base anti-event that manages a linked list of anti-events
 *
 */

class CBaseAE : public IAntiEvent
{
//@access Public Methods
public:
	virtual void Destroy( void );				//@cmember Destroy
												//@cmember Undo			
	virtual HRESULT Undo( CTxtEdit *ped, IUndoBuilder *publdr );
	virtual HRESULT MergeData( DWORD dwDataType, void *pdata);	//@cmember 
												// Merges undo data into the
												// current context.
	virtual void OnCommit( CTxtEdit *ped );		//@cmember Called when AE is
												// committed to the undo stack
	virtual	void SetNext( IAntiEvent *pNext );	//@cmember	Sets the next AE
	virtual IAntiEvent *GetNext( void );		//@cmember	Gets the next AE

//@access Protected Methods
protected:
	// CBaseAE should only exist as a parent class
	CBaseAE();									//@cmember Constructor
	~CBaseAE(){;}

//@access Private Methods and Data
private:
	IAntiEvent *	_pnext;						//@cmember Pointer to the next
												//AntiEvent
					
};



/*
 *	CReplaceRangeAE
 *
 *	@class
 *		an anti-event object than undoes a CTxtPtr::ReplaceRange
 *		operation
 *
 *	@base	public | CBaseAE
 */
class CReplaceRangeAE: public CBaseAE
{
//@access Public Methods
public:
	//
	// IAntiEvent methods
	//
	virtual void Destroy( void );				//@cmember Destroy
												//@cmember Undo
	virtual HRESULT Undo( CTxtEdit *ped, IUndoBuilder *publdr);		
	virtual HRESULT MergeData( DWORD dwDataType, void *pdata);	//@cmember
												// Merges undo data into the
												// current context

//@access Private methods and data
private:
												//@cmember Constructor
	CReplaceRangeAE(DWORD cpMin, DWORD cpMax, DWORD cchDel, TCHAR *pchDel,
			IAntiEvent *paeCF, IAntiEvent *paePF);
	~CReplaceRangeAE();							//@cmember Destructor

	DWORD		_cpMin;							//@cmember cp delete start
	DWORD		_cpMax;							//@cmember cp delete end
	DWORD		_cchDel;						//@cmember #of chars to insert
	TCHAR *		_pchDel;						//@cmember chars to insert
	IAntiEvent *_paeCF;							//@cmember charformat AE
	IAntiEvent *_paePF;							//@cmember par format AE

	friend class CAntiEventDispenser;
};

/*
 *	CReplaceFormattingAE
 *
 *	@class
 *		an anti-event object than undoes replacing multiple char formats
 *
 *	@base	public |  CBaseAE
 */
class CReplaceFormattingAE: public CBaseAE
{
//@access	Public methods
public:
	//
	// IAntiEvent methods
	//
	virtual void Destroy( void );				//@cmember Destroy
												//@cmember Undo
	virtual HRESULT Undo( CTxtEdit *ped, IUndoBuilder *publdr);

//@access	Private Methods and Data
private:
												//@cmember Constructor
	CReplaceFormattingAE(CFormatRunPtr &rp, DWORD cch, IFormatCache *pf,
		BOOL fPara);

	~CReplaceFormattingAE();					//@cmember Destuctor

	DWORD		_cp;							//@cmember cp where formatting
												// should start
	DWORD		_cRuns;							//@cmember # of format runs
	CFormatRun  *_prgRuns;						//@cmember format runs
	BOOL		_fPara;							//@cmember if TRUE, then 
												// formatting is paragraph fmt

	friend class CAntiEventDispenser;
};

/*
 *	CReplaceObjectAE
 *
 *	@class
 *		an anti-event object that undoes the deletion of an object
 *
 *	@base public | CBaseAE
 */
class CReplaceObjectAE : public CBaseAE
{
//@access	Public methods
public:
	//
	//	IAntiEvent methods
	//
	virtual void Destroy(void);					//@cmember Destroy
												//@cmember Undo
	virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);
	virtual void OnCommit(CTxtEdit *ped);		//@cmember called when
												// committed
private:
	CReplaceObjectAE(COleObject *pobj);			//@cmember Constructor
	~CReplaceObjectAE();						//@cmember Destructor

	COleObject *	_pobj;						//@cmember pointer to the
												// deleted object
	BOOL			_fUndoInvoked;				//@cmember undo was invoked
												// on this object.
	
	friend class CAntiEventDispenser;
};

/*
 *	CResizeObjectAE
 *
 *	@class
 *		an anti-event object that undoes the resizing of an object
 *
 *	@base public | CBaseAE
 */
class CResizeObjectAE : public CBaseAE
{
//@access	Public methods
public:
	//
	//	IAntiEvent methods
	//
	virtual void Destroy(void);					//@cmember Destroy
												//@cmember Undo
	virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);
	virtual void OnCommit(CTxtEdit *ped);		//@cmember called when
												// committed
private:
	CResizeObjectAE(COleObject *pobj,			//@cmember Constructor
		            RECT rcPos);				
	~CResizeObjectAE();							//@cmember Destructor

	COleObject *	_pobj;						//@cmember pointer to the
												// deleted object
	RECT			_rcPos;						//@cmember The old object 
												// position/size rectangle
	BOOL			_fUndoInvoked;				//@cmember undo was invoked
												// on this object.
	
	friend class CAntiEventDispenser;
};

/*
 *  CSelectionAE
 *
 *  @class
 *      an anti-event object to restore a selection
 *
 *  @base public | CBaseAE
 */
class CSelectionAE : public CBaseAE
{
//@access   Public methods
public:
    //
    //  IAntiEvent methods
    //
    virtual void Destroy(void);                 //@cmember Destroy
                                                //@cmember Undo
    virtual HRESULT Undo(CTxtEdit *ped, IUndoBuilder *publdr);
    virtual HRESULT MergeData( DWORD dwDataType, void *pdata);  //@cmember
                                                // Merges undo data into the
                                                // current context

private:
                                                //@cmember Constructor
    CSelectionAE(LONG cp, LONG cch, LONG cpNext, LONG cchNext);
    ~CSelectionAE();                            //@cmember Destructor

    LONG        _cp;                            //@cmember the active end
    LONG        _cch;                           //@cmember signed extension
	LONG		_cpNext;						//@cmember the next active end
	LONG		_cchNext;						//@cmember next extension

    friend class CAntiEventDispenser;
};

/*
 *	CAntiEventDispenser
 *
 *	@class
 *		creates anti events and caches them intelligently to provide
 *		for efficient multi-level undo
 */
class CAntiEventDispenser
{
//@access	Public methods
public:
	// no memory mgmt routines; the dispenser is global

												//@cmember text antievent
	IAntiEvent * CreateReplaceRangeAE( CTxtEdit *ped, DWORD cpMin, 
					DWORD cpMax, DWORD cchDel, TCHAR *pchDel, 
					IAntiEvent *paeCF, IAntiEvent *paePF );
												//@cmember formatting AE
	IAntiEvent * CreateReplaceFormattingAE( CTxtEdit *ped, 
					CFormatRunPtr &rp, DWORD cch,
					IFormatCache *pf, BOOL fPara );
												//@cmember Object AE
	IAntiEvent * CreateReplaceObjectAE(CTxtEdit *ped, COleObject *pobj);
												//@cmember Object AE
	IAntiEvent * CreateResizeObjectAE(CTxtEdit *ped, COleObject *pobj, RECT rcPos);
												//@cmember Selection AE
	IAntiEvent * CreateSelectionAE(CTxtEdit *ped, LONG cp, LONG cch, 
					LONG cpNext, LONG cchNext);

private:

	// FUTURE (alexgo): we'll want to maintain an allocation cache of 
	// anti-events
};

// NB!! Global variable.

extern class CAntiEventDispenser gAEDispenser;

#endif // !__ANTIEVNT_H__




	

