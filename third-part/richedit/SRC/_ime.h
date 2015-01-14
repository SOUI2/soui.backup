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
 *	@module _ime.h -- support for IME APIs |
 *	
 *	Purpose:
 *		Most everything to do with FE composition string editing passes
 *		through here.
 *	
 *	Authors: <nl>
 *		Jon Matousek  <nl>
 *		Justin Voskuhl  <nl>
 *		Hon Wah Chan  <nl>
 * 
 *	History: <nl>
 *		10/18/1995		jonmat	Cleaned up level 2 code and converted it into
 *								a class hierarchy supporting level 3.
 *
 *
 */								 

#ifndef _IME_H
#define _IME_H


class CTxtEdit;
class IUndoBuilder;
class CCharFormat;

// defines for some FE Codepages
#define _JAPAN_CP			932
#define _KOREAN_CP			949
#define _CHINESE_TRAD_CP	950
#define _CHINESE_SIM_CP		936

// special virtual keys copied from Japan MSVC ime.h
#define VK_KANA         0x15
#define VK_KANJI        0x19

// defines for IME Level 2 and 3
#define	IME_LEVEL_2		2
#define IME_LEVEL_3		3
#define IME_PROTECTED	4


extern BOOL forceLevel2;	// Force level 2 composition processing if TRUE.

/*
 *	IME
 *
 *	@class	base class for IME support.
 *
 *	@devnote
 *		For level 2, at caret IMEs, the IME will draw a window directly over the text giving the
 *		impression that the text is being processed by the application--this is called pseudo inline.
 *		All UI is handled by the IME. This mode is currenlty bypassed in favor of level 3 true inline (TI);
 *		however, it would be trivial to allow a user preference to select this mode. Some IMEs may have
 *		a "special" UI, in which case level 3 TI is *NOT* used, necessitating level 2.
 *
 *		For level 2, near caret IMEs, the IME will draw a very small and obvious window near the current
 *		caret position in the document. This currently occurs for PRC(?) and Taiwan.
 *		All UI is handled by the IME.
 *
 *		For level 3, at caret IMEs, the composition string is drawn by the application, which is called
 *		true inline, bypassing the level 2 "composition window".
 *		Currently, we allow the IME to support all remaining UI *except* drawing of the composition string.
 */
class CIme
{
	friend LRESULT OnGetIMECompositionMode ( CTxtEdit &ed );
	friend BOOL	IMECheckGetInvertRange(CTxtEdit *ed, LONG &, LONG &);
	friend HRESULT CompositionStringGlue ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );
	friend HRESULT EndCompositionGlue ( CTxtEdit &ed, IUndoBuilder &undobldr );
	friend void CheckDestroyIME ( CTxtEdit &ed );


	//@access	Protected data
	protected:
	INT		_imeLevel;								//@cmember IME Level 2 or 3
	BOOL	_fKorean;								//@cmember In Hangeul mode?

	LONG	_invertMin, _invertMost;				//@cmember to help renderer.
	LONG	_iFormatSave;							//@cmember	format before we started IME composition mode.

	BOOL	_fHoldNotify;							//@cmember hold notify until we have result string

	INT		_dxCaret;								//@cmember current IME caret width 
	BOOL	_fIgnoreIMEChar;						//@cmember Level 2 IME use to eat WM_IME_CHAR message

	//@access	Public methods
	public:
	BOOL	_fDestroy;								//@cmember set when object wishes to be deleted.
	INT		_compMessageRefCount;					//@cmember so as not to delete if recursed.
	BOOL	_fUnderLineMode;						//@cmember save original Underline mode 

	// GuyBark JupiterJ IME: We may move the candidate window after we've walked the caret.
	POINT   _pt;
	DWORD   _dwIndex;
													//@cmember	Handle WM_IME_STARTCOMPOSITION
	virtual HRESULT StartComposition ( CTxtEdit &ed, IUndoBuilder &undobldr ) = 0;
													//@cmember	Handle WM_IME_COMPOSITION and WM_IME_ENDCOMPOSITION
	virtual HRESULT CompositionString ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr ) = 0;
													//@cmember	Handle post WM_IME_CHAR	to update comp window.
	virtual void PostIMEChar( CTxtEdit &ed ) = 0;

													//@cmember	Handle WM_IME_NOTIFY
	virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CTxtEdit &ed ) = 0;

	// GuyBark JupiterJ IME: Added these...
	virtual BOOL GetUndeterminedInfo(INT *picp, INT *pich) = 0;
	virtual BOOL IMESetCandidateWindowPos(CTxtEdit &ed) = 0;

	enum TerminateMode
	{ 
			TERMINATE_NORMAL = 1,
			TERMINATE_FORCECANCEL = 2
	};

	void	TerminateIMEComposition(CTxtEdit &ed,
				CIme::TerminateMode mode);			//@cmember	Terminate current IME composition session.
	
	BOOL	IsKoreanMode() { return _fKorean; }		//@cmember	check for Korean mode

	BOOL	HoldNotify() { return _fHoldNotify; }	//@cmember	check if we want to hold sending change notification
													//@cmember	check if we need to ignore WM_IME_CHAR messages
	BOOL	IgnoreIMECharMsg() { return _fIgnoreIMEChar; }

	INT		GetIMECaretWidth() { return _dxCaret; } //@cmember	return current caret width
	void	SetIMECaretWidth(INT dxCaretWidth) 
	{ 
		_dxCaret = dxCaretWidth;					//@cmember	setup current caret width
	}
	static	void	CheckKeyboardFontMatching ( UINT cp, CTxtEdit &ed, LONG *iFormat );	//@cmember	Check current font/keyboard matching.	

	INT		GetIMELevel () 							//@cmember	Return the current IME level.
	{
		return _imeLevel;
	}

	//@access	Protected methods
	protected:										//@cmember	Get composition string, convert to unicode.
	
	static INT GetCompositionStringInfo( HIMC hIMC, DWORD dwIndex, WCHAR *uniCompStr, INT cchUniCompStr, BYTE *pattrib, INT cchAttrib, LONG *pcursorCP, LONG *pcchAttrib );
	static HRESULT CheckInsertResultString ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );


	void	SetCompositionFont ( CTxtEdit &ed, BOOL *pbUnderLineMode );	//@cmember	Setup for level 2 and 3 composition and candidate window's font.
	void	SetCompositionForm ( CTxtEdit &ed );	//@cmember	Setup for level 2 IME composition window's position.
};

/*
 *	IME_Lev2
 *
 *	@class	Level 2 IME support.
 *
 */
class CIme_Lev2 : public CIme 
{

	//@access	Public methods
	public:											//@cmember	Handle level 2 WM_IME_STARTCOMPOSITION
	virtual HRESULT StartComposition ( CTxtEdit &ed, IUndoBuilder &undobldr );
													//@cmember	Handle level 2 WM_IME_COMPOSITION
	virtual HRESULT CompositionString ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );
													//@cmember	Handle post WM_IME_CHAR	to update comp window.
	virtual void PostIMEChar( CTxtEdit &ed );
													//@cmember	Handle level 2 WM_IME_NOTIFY
	virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CTxtEdit &ed );

	// GuyBark JupiterJ IME: Added these...
	virtual BOOL GetUndeterminedInfo(INT *picp, INT *pich);
    
	virtual BOOL IMESetCandidateWindowPos(CTxtEdit &ed){return FALSE;}

	CIme_Lev2( CTxtEdit &ed );
	~CIme_Lev2();

};

/*
 *	IME_PROTECTED
 *
 *	@class	IME_PROTECTED
 *
 */
class CIme_Protected : public CIme 
{
	//@access	Public methods
	public:											//@cmember	Handle level 2 WM_IME_STARTCOMPOSITION
	virtual HRESULT StartComposition ( CTxtEdit &ed, IUndoBuilder &undobldr )
		{_imeLevel	= IME_PROTECTED; return S_OK;}
													//@cmember	Handle level 2 WM_IME_COMPOSITION
	virtual HRESULT CompositionString ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );
													//@cmember	Handle post WM_IME_CHAR	to update comp window.
	virtual void PostIMEChar( CTxtEdit &ed )
		{}
													//@cmember	Handle level 2 WM_IME_NOTIFY
	virtual HRESULT IMENotify (const WPARAM wparam, const LPARAM lparam, CTxtEdit &ed )
		{return S_FALSE;}

	// GuyBark JupiterJ IME: Added these...
	virtual BOOL GetUndeterminedInfo(INT *picp, INT *pich);
    
	virtual BOOL IMESetCandidateWindowPos(CTxtEdit &ed){return FALSE;}
};

/*
 *	IME_Lev3
 *
 *	@class	Level 3 IME support.
 *
 */
class CIme_Lev3 : public CIme_Lev2 
{
	//@access	Private data
	private:										

	//@access	Protected data
	protected:
	INT		_ichStart;								//@cmember	maintain starting ich.
	INT		_cchCompStr;							//@cmember	maintain composition string's cch.

	//@access	Public methods
	public:											//@cmember	Handle level 3 WM_IME_STARTCOMPOSITION
	virtual HRESULT StartComposition ( CTxtEdit &ed, IUndoBuilder &undobldr );
													//@cmember	Handle level 3 WM_IME_COMPOSITION
	virtual HRESULT CompositionString ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );
													//@cmember	Handle level 3 WM_IME_NOTIFY
	virtual HRESULT	IMENotify (const WPARAM wparam, const LPARAM lparam, CTxtEdit &ed );

	BOOL			SetCompositionStyle ( 	CTxtEdit &ed, CCharFormat &CF, UINT attribute );

	// GuyBark JupiterJ IME: Added these...
	BOOL GetUndeterminedInfo(INT *picp, INT *pich);
	BOOL IMESetCandidateWindowPos(CTxtEdit &ed);

	CIme_Lev3( CTxtEdit &ed ) : CIme_Lev2 ( ed ) {};

};

/*
 *	Special IME_Lev3 for Korean Hangeul -> Hanja conversion
 *
 *	@class	Hangual IME support.
 *
 */
class CIme_HangeulToHanja : public CIme_Lev3 
{
	//@access	Private data
	private:
	LONG	_xWidth;								//@cmember width of Korean Hangeul char

	public:		
	CIme_HangeulToHanja( CTxtEdit &ed, LONG xWdith );
													//@cmember	Handle Hangeul WM_IME_STARTCOMPOSITION
	virtual HRESULT StartComposition ( CTxtEdit &ed, IUndoBuilder &undobldr );
													//@cmember	Handle Hangeul WM_IME_COMPOSITION
	virtual HRESULT CompositionString ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );
};

// Glue functions to call the respective methods of an IME object stored in the ed.
HRESULT StartCompositionGlue ( CTxtEdit &ed, BOOL IsNotProtected, IUndoBuilder &undobldr );
HRESULT CompositionStringGlue ( const LPARAM lparam, CTxtEdit &ed, IUndoBuilder &undobldr );
HRESULT EndCompositionGlue ( CTxtEdit &ed, IUndoBuilder &undobldr );
void	PostIMECharGlue ( CTxtEdit &ed );
HRESULT IMENotifyGlue ( const WPARAM wparam, const LPARAM lparam, CTxtEdit &ed ); // @parm the containing text edit.

#ifndef TARGET_NT
// GuyBark JupiterJ 49850: Must handle WM_IME_REQUEST if we're going to reconvert.
LRESULT IMEHandleRequest(WPARAM wparam, LPARAM lparam, CTxtEdit *ped);
#endif // !TARGET_NT

// IME helper functions.
void	IMECompositionFull ( CTxtEdit &ed );
LRESULT	OnGetIMECompositionMode ( CTxtEdit &ed ); 
BOOL	IMECheckGetInvertRange(CTxtEdit *ed, LONG &, LONG &);
void	CheckDestroyIME ( CTxtEdit &ed );
BOOL	IMEHangeulToHanja ( CTxtEdit &ed, IUndoBuilder &undobldr );

/*  
 *  IgnoreIMEInput()
 *
 *	@devnote
 *		This is to ignore the IME character.  By translating
 *		message with result from pImmGetVirtualKey, we
 *		will not receive START_COMPOSITION message.  However,
 *		if the host has already called TranslateMessage, then,
 *		we will let START_COMPOSITION message to come in and 
 *		let IME_PROTECTED class to do the work.
 */
HRESULT	IgnoreIMEInput( HWND hwnd, CTxtEdit &ed, DWORD lParam );

#endif // define _IME_H

