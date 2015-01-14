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
 *	@doc 	INTERNAL
 *
 *	@module _TEXT.H	-- Declaration for a CTxtRun pointer |
 *	
 *	CTxtRun pointers point at the plain text runs (CTxtArray) of the
 *	backing store and derive from CRunPtrBase via the CRunPtr template.
 *
 */

#ifndef _TEXT_H
#define _TEXT_H

#include "_runptr.h"
#include "_doc.h"
#include "textserv.h"
#include "_m_undo.h"

class CRchTxtPtr;
class CTxtEdit;

/*
 *	CTxtPtr
 *
 *	@class
 *		provides access to the array of characters in the backing store
 *		(i.e. <c CTxtArray>)
 *
 *	@base 	public | CRunPtr<lt>CTxtArray<gt>
 *
 *	@devnote
 *		The state transitions for this object are the same as those for
 *		<c CRunPtrBase>.  <md CTxtPtr::_cp> simply caches the current
 *		cp (even though it can be derived from _iRun and _ich).  _cp is
 *		used frequently enough (and computing may be expensive) that 
 *		caching the value is worthwhile.
 *
 *		CTxtPtr's *may* be put on the stack, but do so with extreme
 *		caution.  These objects do *not* float; if a change is made to 
 *		the backing store while a CTxtPtr is active, it will be out
 *		of sync and may lead to a crash.  If such a situation may 
 *		exist, use a <c CTxtRange> instead (as these float and keep 
 *		their internal text && format run pointers up-to-date).  
 *
 *		Otherwise, a CTxtPtr is a useful, very lightweight plain 
 *		text scanner.
 */

// FindEOP() result flags
#define FEOP_CELL	2
#define FEOP_EOP	1

class CTxtPtr : public CRunPtr<CTxtBlk>
{
	// Only CRchTxtPtr is allowed to call private methods like replace range.  
	friend class CRchTxtPtr;

//@access Public Methods
public:
#ifdef DEBUG
	BOOL Invariant( void ) const;		//@cmember	Invariant checking
	void MoveGapToEndOfBlock () const;
#endif	// DEBUG

	CTxtPtr(CTxtEdit *ped, DWORD cp);	//@cmember	Constructor
	CTxtPtr(const CTxtPtr &tp);			//@cmember	Copy Constructor

	LONG	GetText(LONG cch, TCHAR *pch);	//@cmember 	Fetch <p cch> chars
									//@cmember Fetch <p cch> chars with xlat
	LONG	GetPlainText(LONG cch, TCHAR *pchBuff,
					LONG cpMost, BOOL fTextize);
	TCHAR	NextCharCount(LONG& cch);	//@cmember Advance, GetChar, decrement
	TCHAR	NextChar();				//@cmember Advance to & return next char
	TCHAR	PrevChar();				//@cmember Backup to & return previous char
	TCHAR	GetChar();				//@cmember Fetch char at current cp
	TCHAR	GetPrevChar();			//@cmember Fetch char at previous cp
	DWORD	GetTextLength() const;	//@cmember Get total cch for this document
	const TCHAR* GetPch(LONG& cchValid);//@cmember	Get ptr to block of chars

							//@cmember	Get ptr to a reverse block of chars
	const TCHAR* GetPchReverse(LONG& cchValidReverse, LONG* cchvalid = NULL);

	// The text array has its own versions of these methods (overuling
	// those in runptr base so that <md CTxtPtr::_cp> can be correctly
	// maintained.

	DWORD	BindToCp(DWORD cp);	//@cmember	Rebinds text pointer to cp
	DWORD 	SetCp(DWORD cp);	//@cmember	Sets the cp for the run ptr
	DWORD	GetCp() const 		//@cmember	Gets the current cp
	{ 
		// NB!  we do not do invariant checking here so the floating
		// range mechanism can work OK
		return _cp; 
	};
	void	Zombie();			//@cmember	Turn this tp into a zombie

	LONG	AdvanceCp(LONG cch);	//@cmember	Advance cp by cch chars

	
	// Advance/backup/adjust safe over CRLF and UTF-16 word pairs
	LONG	AdjustCpCRLF();		//@cmember	Backup to start of DWORD char
	LONG	AdvanceCpCRLF();	//@cmember	Advance over DWORD chars 
	LONG	BackupCpCRLF();		//@cmember	Backup over DWORD chars 
	void	CheckMoveGap(DWORD cchLine);//@cmember If line has gap, move gap
	BOOL	IsAfterEOP();		//@cmember	Does current cp follow an EOP?
	BOOL	IsAtEOP();			//@cmember	Is current cp at an EOP marker?
	BOOL	IsAtBOSentence();	//@cmember	At beginning of a sentence?
	BOOL	IsAtBOWord();		//@cmember	At beginning of word?

	
	// Search
								//@cmember	Find indicated text
	LONG	FindText(LONG cpMost, DWORD dwFlags, TCHAR const *,
					 const DWORD cchToFind);
								//@cmember 	Find next EOP
	LONG	FindEOP(LONG cchMax, LONG *pResults = NULL);
								//@cmember	Find next exact	match to <p pch>
	LONG	FindExact(LONG cchMax, TCHAR *pch);
	LONG	FindBOSentence(LONG cch);	//@cmember	Find beginning of sentence

	// Word break support
	LONG	FindWordBreak(INT action, LONG cpMost = -1);//@cmember	Find next word break

//@access	Private methods and data
private:
							//@cmember	Replace <p cchOld> characters with 
							// <p cchNew> characters from <p pch>
	DWORD	ReplaceRange(LONG cchOld, DWORD cchNew, TCHAR const *pch,
									IUndoBuilder *publdr, IAntiEvent *paeCF,
									IAntiEvent *paePF);

							//@cmember  Sub-search routine
	BOOL	MatchAcrossChunks(DWORD dwFlags, DWORD cchMatched, const TCHAR *pch, 
						DWORD cchToMatch, BOOL fWholeWord,  
						EDITWORDBREAKPROC pfnWB) const;

							//@cmember	undo helper
	void 	HandleReplaceRangeUndo(DWORD cchOld, DWORD cchNew, 
						IUndoBuilder *publdr, IAntiEvent *paeCF,
						IAntiEvent *paePF); 

									//@cmember	Insert a range of text helper
									// for ReplaceRange					
	DWORD 	InsertRange(DWORD cch, TCHAR const *pch);
	void 	DeleteRange(DWORD cch);	//@cmember  Delete range of text helper
									// for ReplaceRange
	DWORD		_cp;		//@cmember	character position in text stream

public:
	//FUTURE		(alexgo) see about removing the need to have a _ped member 
	//down here.  This creates a cyclic dependency.  Also, having this as a
	//public method is bogus.  But it's used a lot by derived classes...
	CTxtEdit *	_ped;		//@cmember	pointer to the overall text edit class;
							//needed for things like the word break proc and
							// used a lot by derived classes
};


// =======================   Misc. routines  ====================================================

void	TxCopyText(TCHAR const *pchSrc, TCHAR *pchDst, LONG cch);
//LONG	TxFindEOP(const TCHAR *pchBuff, LONG cch);
INT		CALLBACK TxWordBreakProc(TCHAR const *pch, INT ich, INT cb, INT action);

#endif

