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
 *  @doc EXTERNAL
 *
 *  @module TEXTSERV.CPP    -- Text Services Implementation |
 *
 *  Original Author: <nl>
 *      Rick Sailor
 *
 *  History: <nl>
 *      8/1/95  ricksa  Created and documented
 *      10/95   murrays Further doc and simplifications
 *
 *  Documentation is generated straight from the code.  The following
 *  date/time stamp indicates the version of code from which the
 *  the documentation was generated.
 *
 *  $Header: /richedit/src/textserv.cpp 53    11/15/95 2:39p Ricksa $
 *
 */

#include "_common.h"
#include "_edit.h"
#include "_dispprt.h"
#include "_dispml.h"
#include "_dispsl.h"
#include "_select.h"
#include "_text.h"
#include "_runptr.h"
#include "_font.h"
#include "_measure.h"
#include "_render.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_rtext.h"
#include "_ime.h"
#include "_urlsup.h"
#include "_magelln.h"

// By turning on the PROFILE_TS compiler directive, you tell IceCap2.0
// to turn on profiling for only ITextServices API's.  Typically only
// used during profiling work.
//#define PROFILE_TS
#ifdef PROFILE_TS
#include <icapexp.h>

class CapProfile
{
public:
    CapProfile() { StartCAP(); }
    ~CapProfile() { StopCAP(); }
};

#define START_PROFILING     CapProfile capprf;
#else
#define START_PROFILING
#endif //PROFILE_TS

ASSERTDATA

// Macros to get mouse coordinates out of a message
// need to cast to SHORT first for sign extension
#define MOUSEX  ((INT)(SHORT)LOWORD(lparam))
#define MOUSEY  ((INT)(SHORT)HIWORD(lparam))

LONG ValidateTextRange(TEXTRANGE *pstrg);

#ifdef WIN95_IMEDEBUG
extern BOOL forceLevel2;
#endif

// if there's an active object being dragged around, on WM_PAINT we always
// try to reposition it to there it should be. A not-so-well-behaved object
// my generate another WM_PAINT message in response to that, even if it actually
// did not move. So we limit our number of attempts to reposition it and reset
// this counter every time a mouse moves.
// The corresponding field is declared as :2, so don't try to bump it up
// without allocating more bits!!
#define MAX_ACTIVE_OBJ_POS_TRIES (3)

///////////////////////////// Helper Functions ///////////////////////////////////
#if 0

/*
 *  @doc INTERNAL
 *
 *  IsScreenDC(hdc)
 *
 *  @func
 *      Determine whether DC is for the screen
 *
 *  @rdesc
 *      TRUE - DC is to the screen
 *      FALSE - DC is probably a printer
 *
 *  @devnote
 *      This is currently not used. However, it might sometime prove useful
 *      so the code is left here.
 */
BOOL IsScreenDC(
    HDC hdc )           //@parm DC to test
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "IsScreenDC");

    return (GetDeviceCaps(hdc, LOGPIXELSX) == xPerInchScreenDC)
        && (GetDeviceCaps(hdc, LOGPIXELSY) == yPerInchScreenDC);
}
#endif // 0

/*
 *  ConvertDrawDCMapping(hdcDraw)
 *
 *  @func
 *      Put screen DC in MM_TEXT mapping mode.
 */
void ConvertDrawDCMapping(
    HDC hdcDraw )       //@parm HDC to draw on
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "ConvertDrawDCMapping");

    SaveDC(hdcDraw);
    SetViewportOrgEx(hdcDraw, 0, 0, NULL);
    SetWindowOrgEx(hdcDraw, 0, 0, NULL);
    SetMapMode(hdcDraw, MM_TEXT);
}

/*
 *  CTxtEdit::FormatAndPrint (hdcDraw, hicTargetDev, ptd, lprcBounds,
 *                  fMakeHeightMax, fPrint )
 *  @mfunc
 *      Format and Print data in control
 *
 *  @rdesc
 *      S_OK - everything worked
 *      E_FAIL - unexpected failure occurred
 */
HRESULT CTxtEdit::FormatAndPrint(
    HDC hdcDraw,            //@parm HDC to draw on
    HDC hicTargetDev,       //@parm Input information context if any
    DVTARGETDEVICE *ptd,    //@parm Device target information
    RECT *lprcBounds,       //@parm Rectangle to measure
    RECT *lprcWBounds)      //@parm Metafile information
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::FormatAndPrint");

    // Put client rectangle in format structure
    FORMATRANGE fr;
    fr.rc = *lprcBounds;

    HDC hdcMeasure = NULL;

    // Get number of device units per inch
    LONG xPerInch = 0;
    LONG yPerInch = 0;

    if (NULL == lprcWBounds)
    {
        xPerInch = GetDeviceCaps(hdcDraw, LOGPIXELSX);
        yPerInch = GetDeviceCaps(hdcDraw, LOGPIXELSY);
    }
    else
    {
        hdcMeasure = CreateMeasureDC(hdcDraw, lprcBounds, FALSE, lprcWBounds->left,
            lprcWBounds->top, lprcWBounds->right, lprcWBounds->bottom,
                &xPerInch, &yPerInch);
    }


    // Convert rectangle into TWIPS
    fr.rc.left = MulDiv(fr.rc.left, LX_PER_INCH, xPerInch);
    fr.rc.top = MulDiv(fr.rc.top, LY_PER_INCH, yPerInch);
    fr.rc.right = MulDiv(fr.rc.right, LX_PER_INCH, xPerInch);
    fr.rc.bottom = MulDiv(fr.rc.bottom, LY_PER_INCH, yPerInch);

    // Use message based printing code to do our printing for us
    fr.hdc = hdcDraw;
    fr.hdcTarget = hicTargetDev;
    fr.rcPage = fr.rc;
    fr.chrg.cpMin = _pdp->GetFirstVisibleCp();
    fr.chrg.cpMost = -1;

    // Assume this is all going to work
    HRESULT hr = S_OK;

    SPrintControl prtcon;
    prtcon._fDoPrint = TRUE;
    prtcon._fPrintFromDraw = TRUE;

    // Print control
    if (OnFormatRange(&fr, prtcon, hdcMeasure, xPerInch, yPerInch) == -1)
    {
        // For some reason the control could not be printed
        hr = E_FAIL;
    }

    if (hdcMeasure != NULL)
    {
        TxReleaseMeasureDC(hdcMeasure);
    }

    return hr;
}

/*
 *  CTxtEdit::RectChangeHelper (dwDrawAspect, lindex, pvAspect, ptd, hdcDraw,
 *                              hicTargetDev, lprcClient, prcLocal)
 *  @func
 *      Format and Print data in the control
 *
 *  @rdesc
 *      S_OK - everything worked
 *      E_INVALIDARG - client parameter is invalid
 *
 *  @devnote
 *      Caller must release the DC from the display object.
 */
HRESULT CTxtEdit::RectChangeHelper(
    CDrawInfo *pdi,         //@parm Draw information memory
    DWORD    dwDrawAspect,  //@parm Draw aspect
    LONG     lindex,        //@parm Currently unused
    void *   pvAspect,      //@parm Info for drawing optimizations (OCX 96)
    DVTARGETDEVICE *ptd,    //@parm Info on target device
    HDC      hdcDraw,       //@parm Rendering device context
    HDC      hicTargetDev,  //@parm Target information context
    const RECT *lprcClient, //@parm New client rectangle
    RECT *   prcLocal )     //@parm Rect to use if previous parm is NULL
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEINTERN, "CTxtEdit::RectChangeHelper");

    HRESULT hr = S_OK;
    BOOL fRestore = FALSE;

    // We assume that if client's rectangle is supplied, it has changed.
    if (lprcClient)
    {
        // Set up information context if necessary
        HDC hicLocal = NULL;

        // Did they give us a ptd without a hic?
        if (!hicTargetDev && ptd)
        {
            // Create information context for device information,
            // since it wasn't supplied
            hicLocal = CreateIC(
                (TCHAR *)((BYTE *) ptd + ptd->tdDriverNameOffset),
                (TCHAR *)((BYTE *) ptd + ptd->tdDeviceNameOffset),
                (TCHAR *)((BYTE *) ptd + ptd->tdPortNameOffset),
                (DEVMODE *)((BYTE *)  ptd + ptd->tdExtDevmodeOffset));
            if (NULL == hicLocal)
            {
                // Couldn't create it
                return E_FAIL;
            }

            hicTargetDev = hicLocal;
        }

        // Force DC in MM_TEXT
        if(GetMapMode(hdcDraw) != MM_TEXT &&
            GetDeviceCaps(hdcDraw, TECHNOLOGY) != DT_METAFILE)
        {
            fRestore = TRUE;

            // Store the input data in our local copy so we can update it
            *prcLocal = *lprcClient;

            // Convert input parameters to new mapping
            WinLPtoDP(hdcDraw, (POINT *)prcLocal, 2);

            //
            lprcClient = prcLocal;

            // Convert HDC to new mapping
            ConvertDrawDCMapping(hdcDraw);
        }

        // Set the DC
        _pdp->SetDC(hdcDraw);

        // Set the draw information
        _pdp->SetDrawInfo(
            pdi,
            dwDrawAspect,
            lindex,
            pvAspect,
            ptd,
            hicTargetDev);

        _pdp->ReDrawOnRectChange(hicTargetDev, lprcClient);

        if (fRestore)
        {
            // Put the DC back into the correct mapping mode.
            RestoreDC(hdcDraw, -1);
        }

        if (hicLocal)
        {
            // Clean up information context if we created one.
            DeleteDC(hicLocal);
        }

    }
    else if (_fInPlaceActive)
    {
        // We can figure out what the client rectangle is.
        TxGetClientRect(prcLocal);
        lprcClient = prcLocal;
    }
    else
    {
        // If not inplace active, a rectangle must be supplied.
        hr = E_INVALIDARG;
    }

    return hr;
}

/*
 *  CTxtEdit::SetText (pszText, flags)
 *
 *  @mfunc  sets the text in the document, clearing out any existing
 *          text
 *
 *  @rdesc  HRESULT
 */
HRESULT CTxtEdit::SetText(
    LPCWSTR         pszText,    //@parm Text to set
    SetTextFlags    flags)      //@parm IGNORE_PROTECTION, CHECK_PROTECTION
{
    CCallMgr        callmgr(this);
    CTxtRange       rg(this, 0, 0);// Select whole story
    CGenUndoBuilder undobldr(this, 0);
    LONG            lCleanseResult;

    if( flags & CHECK_PROTECTION &&
        IsProtectedRange(WM_SETTEXT, 0, (LPARAM)pszText, &rg))
    {
        return E_ACCESSDENIED;
    }

    if( _pundo )
    {
        _pundo->ClearAll();
    }

    if( _predo )
    {
        _predo->ClearAll();
    }

    if (IsRich())
    {
        // SetText causing all formatting to return to the default. We use
        // the notification system to remove the formatting. This is
        // particularly important for the final EOP which cannot be deleted.

        // Notify every interested party that they should dump their formatting
        _nm.NotifyPreReplaceRange(NULL, CONVERT_TO_PLAIN, 0, 0, 0, 0);

        // Tell document to dump its format runs
        _story.DeleteFormatRuns();
    }

    rg.Set(0, -(LONG)GetTextLength());

    // if we are re-entered, there may be anti-events higher up the
    // chain.  Grab the undo builder and clear things away if necessary.
    undobldr.Discard();

    lCleanseResult = rg.CleanseAndReplaceRange(-1, pszText, FALSE, NULL);

    if ((0 == lCleanseResult) && (pszText != NULL) && (*pszText != '\0'))
    {
        // There was an input string but for some reason there was no update.
        return E_FAIL;
    }

    // Setting the text means a new document so if there is a selection
    // turn it into an insertion point at the beginning of the document.
    if (_psel)
    {
        _psel->ClearPrevSel();
        _psel->Set(0, 0);

        // Since the text is being completely replaced and all formatting
        // is being lost, let's go back to the default format for the
        // selection.
        _psel->Set_iCF(-1);

        if (_fFocus)
        {
            // Update the caret to reflect new postion
            _psel->UpdateCaret(TRUE);
        }
    }

    // Since we've replaced the entire document, we aren't
    // really "modified" anymore.  This is necessary to match
    // the Windows MLE behavior.  However, since RichEdit1.0
    // did _not_ do this (they left fModified to be TRUE), we
    // only do this for richedit 2.0.

    if( !Get10Mode() )
    {
        _fModified = FALSE;
    }

    _fSaved = FALSE;                        // ITextDocument isn't Saved

    // Adjust text limit if necessary
    TxSetMaxToMaxText();

    return S_OK;
}

/////////////////////////// ITextServices Methods ////////////////////////////////

/*
 *  @doc EXTERNAL
 *
 *  CTxtEdit::TxSendMessage (msg, wparam, lparam, plresult)
 *
 *  @mfunc
 *      Used by window host to forward messages sent to its window to the
 *      text services.
 *
 *  @rdesc
 *      NOERROR Message was processed, and some action taken <nl>
 *      S_FALSE Message was not processed.  Typically indicates that caller
 *              should process message, maybe by calling DefWindowProc <nl>
 *      S_MSG_KEYIGNORED Message processed, but no action was taken <nl>
 *      E_OUTOFMEMORY <nl>
 *
 *  @comm
 *      Note that two return values are passed back from this function.
 *      <p plresult> is the return value that should be passed back from a
 *      window proc.  However, in some cases, the returned LRESULT does not
 *      contain enough information.  For example, to implement cursoring
 *      around controls, it's useful to know if a keystroke (such as right
 *      arrow) was processed, but ignored (e.g. the caret is already at the
 *      rightmost position in the the text).  In these cases, extra
 *      information may be returned via the returned HRESULT.
 *
 *      WM_CHAR and WM_KEYDOWN should return S_MSG_KEYIGNORED when a key or
 *      char has been recognized but had no effect given the current state,
 *      e.g., a VK_RIGHT key when the insertion point is already at the end of
 *      the document). This is used by Forms3 to pass the key up the visual
 *      hierarchy, so that for example, focus moves to the next control in the
 *      TAB order.
 *
 *      This includes the following cases:
 *
 *      1. Any key trying to move the insertion point beyond the end of the
 *      document; or before the begining of the document.
 *
 *      2. Any key trying to move the insertion point beyond the last line or
 *      before the first line.
 *
 *      3. Any insertion of character (WM_CHAR) that would move the insertion
 *      point past the maximum length of the control.
 */
HRESULT CTxtEdit::TxSendMessage (
    UINT    msg,        //@parm Message id
    WPARAM  wparam,     //@parm WPARAM from window's message
    LPARAM  lparam,     //@parm LPARAM from window's message
    LRESULT *plresult ) //@parm Where to put message's return LRESULT
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxSendMessage");

    HRESULT     hr = S_OK;
    LRESULT     lres = 0;
    CObjectMgr *pobjmgr;

    extern UINT gWM_MouseRoller;                    // New Magellan msg ID.

    START_PROFILING

    CCallMgr        callmgr(this);
    CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT );

#ifdef MACPORTREMOVEIT
// PowerMac's cannot set breakpoints on a running application and have
// no way to stop the running appliation.This code allows one to
// stop the executable when the user holds down the option key and
// a break point has been set in the indicated spot below. -jOn

    BOOL fMacBreak = GetAsyncKeyState(VK_OPTION)<0;// Is user holding OPTION?

    if ( fMacBreak )
    {                           // any old statement will due.
        fMacBreak = TRUE;       // Set a break point here for stopping the Mac.
    }

#endif // MACPORTREMOVE

    if ( gWM_MouseRoller == msg )                   // map Magellan msg.
    {
        // convert wparam to correspond to the wparam for the system.
        // Unfortunately, this isn't done automatically for us.
        short zdelta = (short)(long)wparam;

        wparam = MAKELONG(0, zdelta);
        msg = WM_MOUSEWHEEL;
    }

    if (IsIMEComposition())
    {
        // During IME Composition, there are some messages we should
        // not handle.  Also, there are other messages we need to handle by
        // terminating the IME composition first.
        // For WM_KEYDOWN, this is handled inside edit.c OnTxKeyDown().
        switch(msg)
        {
            case WM_COPY:
            case WM_CUT:
#ifndef NODROPFILES
            case WM_DROPFILES:
#endif
            case EM_REDO:
                return S_OK;                    // Just ignore these


            case EM_UNDO:
            case WM_UNDO:
                // just terminate and exist for undo cases
                _ime->TerminateIMEComposition(*this, CIme::TERMINATE_NORMAL);
                return S_OK;

            case EM_REPLACESEL:
            case WM_SETTEXT:
            case WM_CLEAR:
            case EM_STREAMIN:

                // these messages are used to reset our state, so reset
                // IME as well
                _ime->TerminateIMEComposition(*this, CIme::TERMINATE_FORCECANCEL);
                break;

            // GuyBark JupiterJ 49220: Pressing the Alt key with another 
            // key should not result in the string being determined. 
#ifndef PWD_JUPITER 
            case WM_SYSKEYDOWN:
                // Don't terminate IME composition on VK_PROCESSKEY (F10) since Japanese
                // IME will process the F10 key
                if ( wparam == VK_PROCESSKEY )
                    break;
                // otherwise we want to terminate the IME
#endif // ! PWD_JUPITER

            case EM_SETWORDBREAKPROC:
            case WM_PASTE:
            case EM_PASTESPECIAL:

            // GuyBark JupiterJ IME: Ignore dlbclk when undetermined text exists.
            case WM_LBUTTONDBLCLK:
                return hr;

#ifndef PWD_JUPITER // GuyBark: In JupiterJ, a button down does NOT terminate the composition.
            case WM_LBUTTONDOWN:
#ifndef NOMAGELLAN
            case WM_RBUTTONDOWN:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONDBLCLK:
#endif
#endif // ! PWD_JUPITER

            case EM_SCROLL:
            case EM_SCROLLCARET:
            case WM_VSCROLL:
            case WM_HSCROLL:
            case WM_KILLFOCUS:
//            Windows CE Tools #37200
//            TODO: Don't stream out the IME data at all -- right now all the
//            IME data will be streamed out along with the text in the control.
//            case EM_STREAMOUT:
            case EM_SETREADONLY:
            case EM_SETSEL:
            case EM_EXSETSEL:
            case EM_SETPARAFORMAT:
            case EM_SETCHARFORMAT:
#ifndef PEGASUS
            case WM_INPUTLANGCHANGEREQUEST:
#endif
                _ime->TerminateIMEComposition(*this, CIme::TERMINATE_NORMAL);
        }
    }

    switch(msg)
    {
    case EM_CANPASTE:
        // we don't check for protection here, as RichEdit 1.0
        // doesn't
        lres = _ldte.CanPaste(NULL, (CLIPFORMAT) wparam, RECO_PASTE);
        break;

    case EM_CANUNDO:
        if( _pundo )
        {
            lres = _pundo->CanUndo();
        }
        break;

    case EM_CANREDO:
        if( _predo )
        {
            lres = _predo->CanUndo();
        }
        break;

    case EM_GETUNDONAME:
        if( _pundo )
        {
            lres = _pundo->GetNameIDFromAE(wparam);
        }
        break;
    case EM_GETREDONAME:
        if( _predo )
        {
            lres = _predo->GetNameIDFromAE(wparam);
        }
        break;
    case EM_STOPGROUPTYPING:
        if( _pundo )
        {
            // we'll only stop group typing iff wparam
            // is zero (meaning stop regardless) _or_ if
            // wparam matches the merge anti event.
            //
            // This feature allows clients to say that only
            // a specific anti-event should close out it's
            // "fuzzy" state.  Note that currently, only the
            // merge anti-event has this fuzzy state.

            if( !wparam ||
                (IAntiEvent *)wparam == _pundo->GetMergeAntiEvent() )
            {
                _pundo->StopGroupTyping();
            }
        }
        break;

#ifndef TARGET_NT

    case WM_IME_REQUEST:

        // GuyBark JupiterJ 49850: Must handle this if we're going to reconvert.
		// #9560 fix 1999/2/24 by v-tetsus
		{
		CTxtSelection * const psel = GetSel();
		if (NULL == psel)
		{
		    hr = E_OUTOFMEMORY;
		    break;
        }
		
		DWORD	i;
		DWORD	cpMin, cpMax, cch;

		cpMin = psel->GetCpMin();
		cpMax = psel->GetCpMost();
		cch = cpMax - cpMin;

        CTxtPtr tp(this, cpMin);
		LPTSTR pText = new TCHAR[cch+1];

        if(tp.GetText(cch, pText) >= 0){
			for(i=0 ; i < cch ; i++){
				if(pText[i] < 0x20){
					OnSetSel(cpMin, cpMin + i);
					break;
				}
			}
        }
		delete[] pText;
		}
		// #9560 fix 1999/2/24 by v-tetsus
        lres = IMEHandleRequest(wparam, lparam, this);
        break;

#endif // !TARGET_NT

    case WM_IME_STARTCOMPOSITION:   // IME has begun interpreting user input
        // The third parameter to IsntProectedOrReadOnly is the code for
        // regular WM_KEYDOWN lparam because the lparam for
        // WM_IME_STARTCOMPOSITION is not valid.
        hr = StartCompositionGlue (
                *this,
                IsntProtectedOrReadOnly(WM_KEYDOWN, VK_PROCESSKEY, 0x01e001L),
                undobldr );
        break;

    case WM_IME_COMPOSITION:        // State of user's input has changed

        hr = CompositionStringGlue ( lparam, *this, undobldr );
        break;

    case WM_IME_ENDCOMPOSITION:     // User has OK'd IME conversions
        hr = EndCompositionGlue ( *this, undobldr );
        break;

    case WM_IME_NOTIFY:             // Candidate window state change info, etc.

        hr = IMENotifyGlue ( wparam, lparam, *this );
        break;

    case WM_IME_COMPOSITIONFULL:    // Level 2 comp string about to overflow.
        IMECompositionFull ( *this );
        hr = S_FALSE;
        break;

    case WM_IME_CHAR:               // 2 byte character, usually FE.

        if ( IsIMEComposition() && _ime->IgnoreIMECharMsg() )
        {
            hr = NOERROR;
            break;
        }
        else
        {
            TCHAR   unicodeConvert;
            BYTE    bytes[2];
            bytes[0] = wparam >> 8;     // Interchange DBCS bytes in endian
            bytes[1] = wparam;          //  independent fashion (use byte array)
            CTxtSelection *psel = GetSel();

    		if (NULL == psel)
    		{
    		    hr = E_OUTOFMEMORY;
    		    break;
            }


            // need to convert both single-byte KANA and DBC
            if (!bytes[0] || IsDBCSLeadByte(bytes[0]))
            {
                if( UnicodeFromMbcs((LPWSTR)&unicodeConvert, 1,
                    bytes[0] == 0 ? (LPCSTR)&bytes[1] : (LPCSTR)bytes,
                    bytes[0] == 0 ? 1 : 2,
                    GetKeyboardCodePage()) == 1 )
                    wparam = unicodeConvert;
            }
            CIme::CheckKeyboardFontMatching ( GetKeyboardCodePage(), *this, NULL );
            psel->SetDualFontMode(TRUE);
        }                               // **** Fall through to WM_CHAR *****

    case WM_CHAR:

#if defined(WIN95_IMEDEBUG) && !defined(MACPORT)
        if ((WORD)wparam == 0x40 &&             // Shift-2 toggles forceLevel2
            (GetKeyState(VK_SHIFT) & 0x8000))
        {
            forceLevel2 = !forceLevel2;
        }
#endif
        // for Japanese, if Kana mode is on,
        // Kana characters (single byte Japanese char) are coming in via WM_CHAR.
        // Also, for Win95 system and Own host, wparam has been translated already
        // (in remain.cpp)
        if (_fKANAMode && WM_CHAR == msg &&
            !(fInOurHost() && VER_PLATFORM_WIN32_WINDOWS == dwPlatformId))
        {
            // check if this is a single byte character.
            TCHAR   unicodeConvert;
            BYTE    bytes[2];
            bytes[0] = wparam >> 8;     // Interchange DBCS bytes in endian
            bytes[1] = wparam;          //  independent fashion (use byte array)

            if (!bytes[0])
            {
                if( UnicodeFromMbcs((LPWSTR)&unicodeConvert, 1,
                    (LPCSTR)&bytes[1], 1, GetKeyboardCodePage()) == 1 )
                    wparam = unicodeConvert;
            }
        }

        lres = hr = OnTxChar((WORD) wparam, (DWORD) lparam, &undobldr);

#ifdef PWD_JUPITER
        // GuyBark: Try to match what Word97 does.
        // Say the user has just added a CR, on a line with a Heading level style.
        // In Word97 and PWord in Outline View, we stay at the current heading level, 
        // so that's ok. But in Normal view, Word97 will start the next line in the 
        // Normal style. PWord currently maintains the Heading level. So change this, 
        // such that PWord will start the next line in the Normal style too.

        // Has the user added a CR?
        if(wparam == VK_RETURN)
        {
            // Are we in Normal View?
            if(!IsInOutlineView())
            {
                LONG cch; 

                // GuyBark Jupiter 42376:
                // Only do this is the caret is at the end of the line!
                // If the new line added following the insertion of the
                // CR is zero length, then we know the CR was added at 
                // the end of the previous line.
                if((TxLineLength((LONG)-1, &cch) == S_OK) && (cch == 0))
                {
                    // Get the current selection and the range to be changed to 
                    // Normal style. This will always be a zero-length selection,
                    // but we still need to do this to get the caret position.
                    CTxtSelection * psel = GetSelNC();
                    CTxtRange       rg(*psel);
            	    CPFRunPtr       rp(rg);

                    // Don't do anything, if style is already Normal.
                    if(rp.GetOutlineLevel() != 1)
                    {
                        // FUTURE: The undo of the CR addition now takes two steps
                        // due to this extra promotion here. One day this should
                        // me merged into a single undo.

                        // Now change to Normal style.
                        hr = rg.Promote(0, &undobldr);

                        // Make sure the changes to the caret can be seen.
                        psel->Update_iFormat(-1);
                        psel->Update(FALSE);
                    }
                }
            }
        }
#endif // PWD_JUPITER
                                    // Post IME char processing.
        if ( SUCCEEDED(hr) && WM_IME_CHAR == msg )
            PostIMECharGlue( *this );
        break;

    case EM_CHARFROMPOS:
        hr = TxCharFromPos((LPPOINT)lparam, &lres);
        break;

#ifndef PEGASUS
    case WM_INPUTLANGCHANGE:

        if (_fSingleCodePage)
        {
            wparam =_pDocInfo && wparam == GetCharSet(_pDocInfo->wCpg);
        }

        // fall through to WM_INPUTLANGCHANGEREQUEST

    case WM_INPUTLANGCHANGEREQUEST:
        // if the SingleCodePage option is set, then we must
        // have a "good" code page to go to; if not, just
        // eat this message.
        //
        // This will prevent folks from typing French and Greek
        // on the same edit control, which is useful for certain
        // kinds of backward compatibility scenarios.
        //
        // HACK ALERT!  the documentation on WM_INPUTLANGCHANGEREQUEST
        // is wrong.  It turns out that _only_ the low bit of wparam
        // indicates whether or not the new keyboard can be considered
        // as the same code page.
        if( (_fSingleCodePage && (wparam & 0x1) ) ||
            !_fSingleCodePage)
        {
            GetSel()->CheckChangeFont ( this, FALSE, LOWORD(lparam),
                                        ConvertLanguageIDtoCodePage(LOWORD(lparam)) );
            hr = S_FALSE;   // cause default window to allow kb switch.
        }
        break;
#endif

    case WM_CLEAR:
        OnClear(&undobldr);
        break;

    case WM_CONTEXTMENU:
        hr = OnContextMenu(lparam);
        break;

    case WM_COPY:
    case WM_CUT:
        lres = hr = CutOrCopySelection(msg, wparam, lparam, &undobldr);
        break;

    case WM_RENDERFORMAT:
        lres = hr = _ldte.RenderClipboardFormat(wparam);
        break;

    case WM_RENDERALLFORMATS:
        lres = hr = _ldte.RenderAllClipboardFormats();
        break;

    case WM_DESTROYCLIPBOARD:
        lres = hr = _ldte.DestroyClipboard();
        break;

    case EM_DISPLAYBAND:
        if (fInplaceActive())
        {
            OnDisplayBand((const RECT *) lparam, FALSE);
            lres = 1;
        }
        else
        {
            hr = OLE_E_INVALIDRECT;
        }
        break;

#ifndef NODROPFILES
    case WM_DROPFILES:
        OnDropFiles((HANDLE) wparam);
        break;
#endif

    case EM_EMPTYUNDOBUFFER:
        ClearUndo(&undobldr);
        break;

    case WM_ERASEBKGND:
        lres = 1;               // We handle background erase during painting
        break;

    case EM_EXGETSEL:
        OnExGetSel((CHARRANGE *)lparam);
        break;

    case EM_EXLINEFROMCHAR:
        hr = TxLineFromCp((LONG)lparam, &lres);
        break;

    case EM_FINDTEXT:
    case EM_FINDTEXTEX:
    case EM_FINDTEXTW:
    case EM_FINDTEXTEXW:
        lres = OnFindText(msg, (DWORD)wparam, (FINDTEXTEX *)lparam);
        break;

    case EM_FINDWORDBREAK:
        hr = TxFindWordBreak((INT)wparam, (LONG)lparam, &lres);
        break;

    case EM_FORMATRANGE:
        if (fInplaceActive())
        {
            SPrintControl prtcon;
            prtcon._fDoPrint = (wparam) ? TRUE : FALSE;
            lres = OnFormatRange((FORMATRANGE *) lparam, prtcon);
        }
        else
        {
            hr = OLE_E_INVALIDRECT;
        }
        break;

    case EM_GETCHARFORMAT:
    {
        // ensure that all CCharFormat manipulated internally are
        // not CHARFORMAT's or CHARFORMAT2's
        CCharFormat cfInt;
        CHARFORMAT *pcfExt = (CHARFORMAT *)lparam;

        cfInt.cbSize = pcfExt->cbSize;
        lres = OnGetCharFormat(&cfInt, wparam);
        CopyMemory(pcfExt, &cfInt, min(pcfExt->cbSize,sizeof(cfInt)));
        break;
    }
    case EM_GETFIRSTVISIBLELINE:
        if (fInplaceActive())
        {
            lres = _pdp->GetFirstVisibleLine();
        }
        else
        {
            hr = OLE_E_INVALIDRECT;
        }
        break;

    case EM_GETLIMITTEXT:
        lres = TxGetMaxLength();
        break;

    case EM_GETLINE:
        if (fInplaceActive())
        {
            lres = _pdp->GetLineText((LONG)wparam, (TCHAR *)lparam,
                                (LONG) (*(WORD *) lparam));
        }
        else
        {
            hr = OLE_E_INVALIDRECT;
        }
        break;

    case EM_GETLINECOUNT:
        hr = TxGetLineCount(&lres);
        break;

    case EM_GETMODIFY:              // RichEdit 1.0 returned -1 if _fModified
        lres = -(LONG)_fModified;   //  is TRUE (go figure). So for backward
        break;                      //  compatibility, we do too :-(

    case EM_GETOLEINTERFACE:
        if( lparam )
        {
            *(IRichEditOle **)lparam = (IRichEditOle *)this;
            AddRef();
        }
        lres = TRUE;
        break;

    case EM_GETSCROLLPOS:
        {
            POINT *point = (POINT *)lparam;
            point->x = _pdp->GetXScroll();
            point->y = _pdp->GetYScroll();
            point->y = _pdp->ConvertYPosToScrollPos(point->y);
            lres = 1;
        }
        break;

    case EM_SETOLECALLBACK:
        hr = E_FAIL;
        if( lparam )
        {
            pobjmgr = GetObjectMgr();
            if( pobjmgr )
            {
                pobjmgr->SetRECallback((IRichEditOleCallback *)lparam);
                lres = TRUE;
                hr = NOERROR;
            }
        }
        break;

    case EM_GETPARAFORMAT:
    {
        // Use CParaFormat for all internal operations
        CParaFormat PF;
        PARAFORMAT *pPF = (PARAFORMAT *)lparam;

        PF.cbSize = pPF->cbSize;
        lres = OnGetParaFormat(&PF);
        // PREFIX WCEP 31825
        if (IsValidParaFormat(&PF)) 
        {
            CopyMemory(pPF, &PF, pPF->cbSize);
        }
    }
        break;

    case EM_GETSEL:
        lres = OnGetSel((LONG*)wparam, (LONG*)lparam);
        break;

    case EM_GETSELTEXT:
        lres = OnGetSelText((TCHAR *)lparam);
        break;

    case WM_GETTEXT:
        {
            GETTEXTEX gt;

            gt.cb = wparam * 2;
            gt.flags = GT_USECRLF;
            gt.codepage = 1200;
            gt.lpDefaultChar = NULL;
            gt.lpUsedDefChar = NULL;

            lres = GetTextEx(&gt, (TCHAR *)lparam);
        }
        break;

    case WM_GETTEXTLENGTH:
        {
            GETTEXTLENGTHEX gtl;

            gtl.flags = GTL_NUMCHARS | GTL_PRECISE | GTL_USECRLF;
            gtl.codepage = 1200;

            lres = GetTextLengthEx(&gtl);
        }
        break;

    case EM_GETTEXTEX:
        lres = GetTextEx((GETTEXTEX *)wparam, (TCHAR *)lparam);
        break;
    case EM_GETTEXTLENGTHEX:
        lres = GetTextLengthEx((GETTEXTLENGTHEX *)wparam);
        break;

    case EM_GETTEXTRANGE:
    {
        TEXTRANGE * const ptr = (TEXTRANGE *) lparam;
        LONG              cch = ValidateTextRange(ptr);

        if (cch < 0)                    // Get text character count because
            cch = GetTextLength();      //  caller wants it all
        cch++;                          // Bump count by terminating '\0'

        // Only copy if we need to copy anything and destination is valid
        if (cch > 1 &&
            !IsBadWritePtr(ptr->lpstrText, cch * sizeof(TCHAR)))
        {
            lres = GetTextRange(ptr->chrg.cpMin, cch, ptr->lpstrText);
        }
    }
        break;

    case EM_GETWORDBREAKPROC:
        lres = OnGetWordBreakProc();
        break;

    case EM_GETZOOM:
        *(WORD *)wparam = GetZoomNumerator();
        *(WORD *)lparam = GetZoomDenominator();
        lres = 1;
        break;

    case EM_HIDESELECTION:
        lres = OnTxHideSelectionChange((BOOL)wparam);
        break;

    case WM_HSCROLL:
        hr = TxHScroll(LOWORD(wparam), HIWORD(wparam));
        break;

    case WM_KEYDOWN:

        hr = OnTxKeyDown((WORD) wparam, (DWORD) lparam, &undobldr);
        break;

    case WM_KEYUP:

        // GuyBark JupiterJ IME:
        // Have we frozen the display while we force an ime caret move?
        if(_fDisplayFrozen)
        {
            // Do we have undetermined text?
            if(!IsIMEComposition())
            {
                // No! Somehow we determined the text without getting the 
                // key up, so thaw now!

                _pdp->Thaw();
                _fDisplayFrozen = FALSE;
            }
            else
            {
                MSG  msgPeek;
                HWND hWndRE;

                // We have some undetermined text. So check if we need to thaw the display now.

                if((TxGetWindow(&hWndRE) != S_OK) ||
                   !PeekMessage(&msgPeek, hWndRE, WM_KEYUP, WM_KEYUP, PM_NOREMOVE))
                {
                    // Looks like we've got the last of our forced caret moves.

                    // Before thawing the display, make sure any candidate
                    // list is positioned beneath the final caret position.
                    if(_ime->_dwIndex != 0xFFFF)
                    {
                        _ime->IMESetCandidateWindowPos(*this);
                    }

                    // Now thaw!
                    _pdp->Thaw();

                    _fDisplayFrozen = FALSE;
                }
            }
        }

// Start v-holee
// VK_JANJI == VK_HANJA in KOREAN
		if( (WORD)wparam == VK_KANJI )
		{
			if(_KOREAN_CP == GetKeyboardCodePage() && 
			   IsntProtectedOrReadOnly(WM_KEYUP, VK_KANJI, lparam))
			{
				IUndoBuilder *publdr = &undobldr;
				IMEHangeulToHanja ( *this, *publdr );
			}
		}
// Finish v-holee

        if( (WORD)wparam == VK_APPS )
        {
            HandleKbdContextMenu();
        }
        else
        {
            // otherwise, we don't process this message.
            hr = S_FALSE;
        }
        break;

    case WM_KILLFOCUS:
        lres = OnKillFocus();
        break;

    case WM_LBUTTONDBLCLK:
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
        hr = OnTxLButtonDblClk(MOUSEX, MOUSEY, (WORD) wparam, &undobldr);
#else
        hr = OnTxLButtonDblClk(MOUSEX, MOUSEY, (WORD) wparam);
#endif 
        break;

    case WM_LBUTTONDOWN:
        if(_fEatLeftDown)
        {
            TxSetFocus();
            _fEatLeftDown = FALSE;
        }
        else
        {
            hr = OnTxLButtonDown(MOUSEX, MOUSEY, (WORD) wparam);
        }
        break;

    case WM_LBUTTONUP:
        hr = OnTxLButtonUp(MOUSEX, MOUSEY, (WORD) wparam, TRUE);
        break;

#if !defined(NOMAGELLAN)
    case WM_MBUTTONDBLCLK:                      // Magellan zmouse scroll
    case WM_NCMBUTTONDOWN:                      //  support commandeers middle
    case WM_MBUTTONDOWN:                        //  button.
        OnTxMButtonDown(MOUSEX, MOUSEY, (WORD) wparam);
        break;

    case WM_MBUTTONUP:
        OnTxMButtonUp(MOUSEX, MOUSEY, (WORD) wparam);
        break;

    case WM_MOUSEWHEEL:                     // Megellan zmouse scroll n lines.
        lres = HandleMouseWheel(wparam, lparam);
        break;
#endif

    case EM_LINEFROMCHAR:
        hr = TxLineFromCp((LONG) wparam, &lres);
        break;

    case EM_LINEINDEX:
        hr = TxLineIndex((LONG)wparam, &lres);
        break;

    case EM_LINELENGTH:
        hr = TxLineLength((LONG) wparam, &lres);
        break;

    case EM_LINESCROLL:
        hr   = TxLineScroll((LONG) lparam, (LONG) wparam);
        lres = TxGetMultiLine();
        break;

#ifndef PEGASUS
    case WM_MOUSEACTIVATE:
        lres = MA_ACTIVATE;
        // if the window that currently has focus is part of our "application",
        // then don't eat the mouse click.  Otherwise, if it's from another
        // app, the user is probably trying to swap apps, so eat the mosue
        // down message and let our host app get a chance to come to the
        // foreground.
        if(!( IsChild((HWND) wparam, GetFocus()) || ((HWND)wparam != NULL &&
            (HWND)wparam == GetFocus())))
        {
            _fEatLeftDown = TRUE;
        }
        break;
#endif

    case WM_MOUSEMOVE:
        // we reset the "number of tries to put an active object
        //  in place" count here
        _cActiveObjPosTries = MAX_ACTIVE_OBJ_POS_TRIES;

        hr = OnTxMouseMove(MOUSEX, MOUSEY, (WORD) wparam, &undobldr);
        break;

    case EM_OUTLINE:
    {
        if(wparam == EMO_GETVIEWMODE)
        {
            hr = GetViewKind(&lres);
            break;
        }

        CTxtSelection * psel = GetSelNC();

        if(!TxGetMultiLine() || !IsRich() || !psel) // Control must be rich,
            break;                                  //  multiline and active

        if(wparam == EMO_ENTER || wparam == EMO_EXIT)
        {
            hr = SetViewKind(wparam == EMO_ENTER ? VM_OUTLINE : VM_NORMAL);
            lres = SUCCEEDED(hr);
            break;
        }

        if(!IsInOutlineView() || !IsntProtectedOrReadOnly(msg, wparam, lparam))
            break;

        CTxtRange rg(*psel);

        switch(wparam)
        {
            case EMO_PROMOTE:
                hr = rg.Promote(lparam, &undobldr);
                psel->Update_iFormat(-1);
                psel->Update(FALSE);
                break;

            case EMO_EXPAND:
#ifdef PWD_JUPITER // GuyBark 81387: Allow undo of expand/collapse operation
                hr = rg.ExpandOutline((short)LOWORD(lparam),
                                      HIWORD(lparam) == EMO_EXPANDDOCUMENT, &undobldr);
#else
                hr = rg.ExpandOutline((short)LOWORD(lparam),
                                      HIWORD(lparam) == EMO_EXPANDDOCUMENT);
#endif // PWD_JUPITER
                break;

            case EMO_MOVESELECTION:
                hr = MoveSelection(lparam, &undobldr);
                psel->Update(TRUE);
                break;

            default:
//              TraceMessage("Unknown outline function received\r\n");
                break;
        };
        lres = SUCCEEDED(hr);
        _fModified = TRUE;
    }
        break;

    case WM_PASTE:
    case EM_PASTESPECIAL:
        if( IsntProtectedOrReadOnly(msg, wparam, lparam) )
        {
            CTxtSelection *psel = GetSel();

            hr = PasteDataObjectToRange(NULL, psel,
                (CLIPFORMAT) wparam, (REPASTESPECIAL *)lparam, &undobldr,
                PDOR_NONE);

            // to match the Word UI, better go ahead and update the window
            TxUpdateWindow();
        }
        break;

    case EM_POSFROMCHAR:
        // RichEdit 2.x used wparam instead of lparam for the cp and ignored
        // wparam, unlike RE 1.0 and the Win32 documentation (sigh!). We fix
        // this, but are compatible with RE 2.x for cp's < 1MB, since no
        // 32-bit addresses on Win95 or NT are smaller than that.
        if(wparam < 0x10000 || wparam == 0xFFFFFFFF)// Illegal ptr, so assume
        {                                   //  incorrect RE 2.0 params
            // TODO: enable when msgtest gets updated
            //AssertSz(FALSE,
            //  "EM_POSFROMCHAR: wparam is illegal ptr, assuming cp value");
            POINT pt;
            hr = TxPosFromChar((LONG)wparam, &pt);
            lres = SUCCEEDED(hr) ? MAKELONG(pt.x, pt.y) : -1;
        }
        else
            hr = TxPosFromChar((LONG)lparam, (LPPOINT)wparam);
        break;

#ifndef PEGASUS
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        // give the client a chance to handle these messages,
        // if we are over a link
        if( HandleLinkNotification(msg, wparam, lparam) )
        {
            break;
        }

        if( msg == WM_RBUTTONUP )
        {
            hr = OnTxRButtonUp(MOUSEX, MOUSEY, (WORD) wparam);
        }
        else if( msg == WM_RBUTTONDOWN )
        {
            hr = OnTxRButtonDown(MOUSEX, MOUSEY, (WORD) wparam);
        }

        break;
#endif

    case EM_INSERTGRAPHIC:
        if (wparam == IG_TYPE_DIB)
        {
            IUndoBuilder *	publdr;
            CGenUndoBuilder undobldr(this, UB_AUTOCOMMIT);
            publdr = &undobldr;            
      
            lres = hr = OnInsertDIB(lparam, publdr);
        }
        break;

    case EM_REPLACESEL:
        // BUG 511 - check wParam to see if we want to setup undo
        if(!IsProtected(msg, wparam, lparam))
            lres = OnReplaceSel(lparam ? -1 : 0, (TCHAR *) lparam,
                wparam ? &undobldr : NULL);
        break;

    case EM_REQUESTRESIZE:
        hr = _pdp->RequestResize();
        break;

    case EM_SCROLL:
        // TxVScroll returns the number of lines scrolled;
        // this info should be returned in lres
        lres = TxVScroll((WORD)wparam, 0);
        break;

    case EM_SCROLLCARET:
        OnScrollCaret();
        break;

    case EM_SELECTIONTYPE:
    {
        SELCHANGE selchg;

        GetSel()->SetSelectionInfo( &selchg );
        lres = selchg.seltyp;
    }
        break;

    case WM_SETFOCUS:
        hr = OnSetFocus();
        break;

    case EM_SETFONTSIZE:
    {   // TODO: ? Return non-zero if we set a new font size for some text.

        CCharFormat CF;
        CF.dwMask = CFM_SIZE;
        CF.yHeight = 0x80000000 | LOWORD(wparam);

        lres = OnSetCharFormat(SCF_SELECTION, &CF, &undobldr);
    }
        break;

    case EM_SETMODIFY:
        _fModified = wparam != 0;

#ifdef  LATER
        if (!_fModified)
            ObFreezeFrames();
#endif                      // LATER
        break;

    case EM_SETSCROLLPOS:
        {
            POINT *pPoint = (POINT*)lparam;
            _pdp->ScrollView(pPoint->x, pPoint->y, FALSE, TRUE);
            lres = 1;
        }
        break;

    case EM_EXSETSEL:
        // EM_EXSETSEL duplicates the functionality of the 32-bit EM_SETSEL
        // and exists purely for backward compatibility with Win16. We just
        // repackage the params and fall thru to EM_SETSEL
        wparam = (WPARAM)((CHARRANGE *)lparam)->cpMin;
        lparam = (LPARAM)((CHARRANGE *)lparam)->cpMost;

        // FALL-THROUGH to EM_SETSEL!!!

    case EM_SETSEL:
        OnSetSel((LONG)wparam, (LONG)lparam);
        break;

    // Win95's dialog manager doesn't even
    // pretend to be 32 bits despite the best attempts of our marketing dudes.
    // WM_USER + 1 is the old Win3.0 EM_SETSEL in which the selection range
    // was packed into the lparam.
    //
    // Sometimes (like tabbing through a dialog), Win95 will send us the 16
    // bit EM_SETSEL message, so process it here.
    case (WM_USER + 1):
        OnSetSel(LOWORD(lparam), HIWORD(lparam));
        break;

    case EM_SETTARGETDEVICE:
        // Keep width sane so that LXtoDX works OK at least for displays
        // Note that 0x7fffff = 485 feet! This keeps LXtoDX working provided
        // _xPerInch < 257 (it's typically 96 for displays). For more
        // generality, we'd need to use 64-bit arithmetic (see LXtoDX).
        lparam = min(lparam, 0x7fffff);
        lres = _pdp->SetMainTargetDC((HDC) wparam, (LONG) lparam);
        break;

    case WM_SETTEXT:
        hr = TxSetText((LPTSTR) lparam);
        if( SUCCEEDED(hr) )
        {
            lres = 1;
        }
        break;

    case EM_SETWORDBREAKPROC:
        _pfnWB = (EDITWORDBREAKPROC) lparam;
        break;

    case WM_SYSCHAR:

        // If this message is generated by the UNDO keystroke, we eat it
        if(((DWORD) lparam & SYS_ALTERNATE) && ((WORD) wparam == VK_BACK))
            break;
        goto def;

    case WM_SYSKEYDOWN:
        if (OnTxSysKeyDown(wparam, lparam, &undobldr) == S_OK)
        {
            lres = TRUE;
            break;
        }
        goto def;

    case WM_TIMER:
        OnTxTimer((UINT)wparam);
        goto def;

    case EM_UNDO:
    case WM_UNDO:
        if (_pundo && !_fReadOnly )
        {
            hr = PopAndExecuteAntiEvent(_pundo, wparam);
            if( hr == NOERROR )
            {
                lres = TRUE;
            }
        }
        break;

    case EM_REDO:
        if( _predo && !_fReadOnly )
        {
            hr = PopAndExecuteAntiEvent(_predo, wparam);
            if( hr == NOERROR )
            {
                lres = TRUE;
            }
        }
        break;

    case EM_SETUNDOLIMIT:
        lres = HandleSetUndoLimit((DWORD)wparam);
        break;

    case WM_VSCROLL:
        // TxVScroll returns the number of lines scrolled;
        // WM_VSCROLL doesn't care about that info however.
        Assert(lres == 0);
        Assert(hr == NOERROR);
        TxVScroll(LOWORD(wparam), HIWORD(wparam));
        break;

    // Old stuff that's no longer supported
    case EM_FMTLINES:               // Controls returning CRCRLFs for soft
                                    //  line breaks in EM_GETTEXT. Could
                                    //  implement
    case WM_GETFONT:                // Can support but have to hang onto a
                                    //  default HFONT. CCcs has an _hfont, but
                                    //  need to be sure default font is in
                                    //  cache at time of return
    case EM_SETTABSTOPS:            // Can support easily (need to convert
                                    //  dialog-box units to twips and handle
                                    //  ctabs = 1 special case)
    case EM_GETHANDLE:              // Not supported by Win95 32-bit MLE either
    case EM_SETHANDLE:              // Not supported by Win95 32-bit MLE either
#ifdef DEBUG
        TRACEINFOSZ("Homey don't play dat");
#endif
        break;

    case EM_SETCHARFORMAT:
    {
        // Ensure that all CCharFormat's manipulated internally are
        // CCharFormat's and not CHARFORMAT's or CHARFORMAT2's
        CCharFormat CF;
        CHARFORMAT *pCF = (CHARFORMAT *)lparam;

        CF.Set(pCF);
        CF.cbSize = pCF->cbSize;
        CF.dwMask = pCF->dwMask;
        lres = OnSetCharFormat(wparam, &CF, &undobldr);
        break;
    }
    case WM_SETFONT:
        lres = OnSetFont((HFONT) wparam);
        break;

    case EM_SETPARAFORMAT:
    {
        // Ensure that all CParaFormat's manipulated internally are
        // CParaFormat's and not PARAFORMAT's or PARAFORMAT2's
        CParaFormat PF;
        PARAFORMAT *pPF = (PARAFORMAT *)lparam;

        PF.Set(pPF);
        PF.cbSize = pPF->cbSize;
        PF.dwMask = pPF->dwMask;
        lres = OnSetParaFormat(wparam, &PF, &undobldr);
        break;
    }

    case EM_SETZOOM:
        if(wparam < 65536 && lparam < 65536)
        {
            SetZoomNumerator(wparam);
            SetZoomDenominator(lparam);

            // GuyBark: 
            // Don't always return TRUE here. Return the success of UpdateView().
#ifdef PWD_JUPITER
            lres =
#endif // PWD_JUPITER

            _pdp->UpdateView();

#ifndef PWD_JUPITER
            lres = 1;
#endif // !PWD_JUPITER
        }
        break;

    case EM_STREAMIN:
    case EM_STREAMOUT:
    {
        IUndoBuilder *publdr = &undobldr;
        CTxtRange   rg(this, 0, -(LONG)GetTextLength());
        CTxtRange * prg = &rg;                  // Default whole doc

        if( wparam & SFF_SELECTION )            // Save to current selection
        {
            prg = (CTxtRange *)GetSel();
            AssertSz(prg,
                "EM_STREAMIN/OUT: requested selection doesn't exist");
        }
        else
        {
            // if we are not streaming into the selection, then we are
            // "loading" the entire file; this is not an undo-able operation,
            // so set the undo builder to NULL and get rid of the current
            // undo stacks
            publdr = NULL;
            ClearUndo(&undobldr);
            if(msg == EM_STREAMIN)
            {
                wparam |= SFF_ADJUSTENDEOP;
                // clear away the file information if necessary
                if( !(wparam & SFF_KEEPDOCINFO) )
                {
                    CloseFile(FALSE);
                }
            }
        }

        if(msg == EM_STREAMIN)
        {
            // if we are going to be loading an entire file, we only
            // want to check "normal' protection; we can ignore the
            // fIsDBCS protection.  This does mean that somebody
            // can do an "insert file" and break apart a DBCS combo,
            // but we'll have to live with that.  Outlook uses
            // RTF streaming in many different places, so the strong
            // fIsDBCS protection breaks them.
            if( (_dwEventMask & ENM_PROTECTED) &&
                prg->IsProtected(0) == CTxtRange::PROTECTED_ASK &&
                QueryUseProtection(prg, msg, wparam, lparam))
            {
                Sound();
                Assert(lres == 0);
                break;
            }

            // Freeze the display before loading
            CFreezeDisplay fd(_pdp);

            lres = _ldte.LoadFromEs(prg, wparam, (EDITSTREAM *)lparam,
                                    FALSE, publdr);

            if (_fFocus)
            {
                // Update caret but delay till display is thawed and do so only
                // if we have the focus. If we do this all the time we get wierd
                // scrolling effects such as scrolling to the beginning of a
                // document when the focus is set. See bug #1649 for repro of
                // wierd effects.
                _pdp->SaveUpdateCaret(TRUE);
            }
        }
        else
            lres = _ldte.SaveToEs  (prg, wparam, (EDITSTREAM *)lparam);
        break;
    }

#ifndef PEGASUS
    case WM_SYSCOLORCHANGE:
        TxInvalidateRect(NULL, FALSE);
        break;
#endif

    // debug stuff
#ifdef DEBUG
    case EM_DBGPED:
        OnDumpPed();
        break;
#endif                  // DEBUG

    case EM_SETEVENTMASK:
        lres = _dwEventMask;                // Set up to return value before
        _dwEventMask = (DWORD)lparam;       //  the change

        if (lparam & ENM_REQUESTRESIZE)
        {
            // We need to update the display just in case it changes.
            _pdp->UpdateView();
        }
        break;

    case EM_GETEVENTMASK:
        lres = _dwEventMask;
        break;

    case EM_GETTHUMB:
        LONG Pos;
        BOOL IsEnabled;

        if (TxGetVScroll(NULL, NULL, &Pos, NULL, &IsEnabled) == S_OK
            && IsEnabled )
        {
            lres = Pos;
        }
        break;

    case EM_SETLANGOPTIONS:
        _fAutoFont = (lparam & IMF_AUTOFONT) != 0;
        _fAutoKeyboard = (lparam & IMF_AUTOKEYBOARD) != 0;
        _fIMEAlwaysNotify = (lparam & IMF_IMEALWAYSSENDNOTIFY) != 0;
        lres = 1;
        if ( fHaveIMMProcs )
        {
            _fIMECancelComplete = (lparam & IMF_IMECANCELCOMPLETE) != 0;
            lres = 1;
        }
        break;

    case EM_GETLANGOPTIONS:
        if ( _fAutoFont )
            lres |= IMF_AUTOFONT;
        if ( _fAutoKeyboard )
            lres |= IMF_AUTOKEYBOARD;
        if ( _fIMEAlwaysNotify )
            lres |= IMF_IMEALWAYSSENDNOTIFY;
        if ( fHaveIMMProcs )
        {
            if ( _fIMECancelComplete )
                lres |= IMF_IMECANCELCOMPLETE;
        }
        break;

    case EM_GETIMECOMPMODE:
        lres = OnGetIMECompositionMode(*this);
        break;

    case EM_SETTEXTMODE:
        lres = HandleSetTextMode(wparam);
        break;

    case EM_GETTEXTMODE:
        lres = IsRich() ? TM_RICHTEXT : TM_PLAINTEXT;

        if( _pundo && ((CUndoStack *)_pundo)->GetSingleLevelMode() )
        {
            lres |= TM_SINGLELEVELUNDO;
        }
        else
        {
            lres |= TM_MULTILEVELUNDO;
        }

        if( _fSingleCodePage )
        {
            lres |= TM_SINGLECODEPAGE;
        }
        else
        {
            lres |= TM_MULTICODEPAGE;
        }
        break;

    case EM_LIMITTEXT:
        lparam = wparam;

        // Intentionally fall through. These messages are duplicates.

    case EM_EXLIMITTEXT:

        if (lparam == 0)
        {
            // 0 means set the control to the maximum size. However, because
            // 1.0 set this to 64K will keep this the same value so as not to
            // surprise anyone. Apps are free to set the value to be above 64K.
            lparam = (LPARAM) cResetTextMax;
        }

        _cchTextMost = (LONG) lparam;

        break;

    case EM_AUTOURLDETECT:
        if( !_fRich )
        {
            hr = lres = E_FAIL;
        }

        if( lparam != 0 || ((wparam | 1) != 1))
        {
            hr = lres = E_INVALIDARG;
            break;
        }

        if( wparam == TRUE && !_pdetecturl )
        {
            _pdetecturl = new CDetectURL(this);
            if( !_pdetecturl )
            {
                hr = lres = E_OUTOFMEMORY;
            }
        }
        else if( !wparam && _pdetecturl )
        {
            delete _pdetecturl;
            _pdetecturl = NULL;
        }
        break;

    case EM_GETAUTOURLDETECT:
        Assert(lres == 0);
        Assert(hr == NOERROR);
        if( _pdetecturl )
        {
            lres = TRUE;
        }
        break;

    case WM_SIZE:
        // we reset the "number of tries to put an active object
        //  in place" count here
        _cActiveObjPosTries = MAX_ACTIVE_OBJ_POS_TRIES;

        hr = S_FALSE;
        break;

    default:
def:    hr = S_FALSE;
        break;
    }

    if( plresult )
        *plresult = lres;

    return hr;
}

/*
 *  CTxtEdit::TxDraw (dwDrawAspect, lindex, pvAspect, ptd, hdcDraw,
 *                    hicTargetDev, lprcBounds, lprcWBounds, lprcUpdate,
 *                    pfnContinue, dwContinue)
 *
 *  @mfunc  Draws the text services object
 *
 *  @rdesc  HRESULT (typically S_OK).
 *
 *  @comm
 *
 *  This method renders the Text Services. It accepts the same parameters
 *  as the corresponding IViewObject::Draw method in OLE, with the extra
 *  <p lprcUpdate > parameter. It can be used while the host is inactive
 *  or active (in-place).
 *
 *  If dwDrawAspect is DVASPECT_CONTENT, this method should render a screen
 *  image of the text content to the hdcDraw device context. The hicTargetDev
 *  and ptd parameters give information on the target device context if any
 *  (usually a printer).
 *
 *  The lprcClient parameter gives the rectangle to render to, also called
 *  "client rectangle". This rectangle represents the position and extents
 *  of the entire image of the Text Services to be drawn. It is expressed in
 *  the logical coordinate system of hdcDraw. This parameter can only be NULL
 *  if the control is active. In that case, Text Services should render the
 *  in-place active view (which client rectangle can be obtained by calling
 *  TxGetClientRect on the host).
 *
 *  The lprcUpdate parameter, if not NULL, gives the rectangle to update
 *  inside that client rectangle. It is given in the logical coordinate system
 *  of hdcDraw. If NULL, the entire client rectangle should be painted.
 *
 *  Text Services should render with the appropriate zooming factor, which
 *  can be obtained from the client rect and the native size given by
 *  ITextHost::GetExtent.  See ITextHost::GetExtent.
 *
 *  If the drawing aspect is DVASPECT_DOCPRINT, the TxDraw method can assume
 *  that it is rendering to the printer. In that case, hdcDraw is the printer
 *  device context. TxDraw should still render the lprcBounds rectangle,
 *  starting at the current scrolling position. TS can make optimization for
 *  rendering to the printer (like not painting the background color if white)
 *  and certain screen specific elements (such as the selection) should not be
 *  rendered.
 *
 *  General comments on OLE hosts and TxDraw (and TxSetCursor, TxQueryHitPoint):
 *
 *  OLE hosts can call the TxDraw method at any time with any rendering DC or
 *  client rectangle. All an inactive OLE object has on a permanent basis is
 *  a himetric extent. It gets the rectangle in which to render only via the
 *  IViewObject::Draw call and this rectangle is valid only for the scope of
 *  that method. In fact, the same control can be rendered consecutively in
 *  different rectangles and different DCs for example because it is displayed
 *  simultaneously in different views on the screen.
 *
 *  The client rectangle and DC passed to TxDraw should normally not be cached.
 *  However, this would force Text Services to recalc lines for every single
 *  draw, which would lead to terrible performance. So it is likely that Text
 *  Services will actually cache some information computed for a specific
 *  client rectangle and DC (such as the line breaks for example). On the
 *  next call to TxDraw, however, the validity of the cached information
 *  should be checked before it gets used, and updated information should be
 *  regenerated if necessary.
 *
 *  When the control is in-place active, the problem is even more complex
 *  since TxDraw can still be called to render other views than the in-place
 *  active one. In other words, the client rectangle passed to TxDraw may
 *  not be the same as the active view one (passed to OnTxInPlaceActivate
 *  and obtained via TxGetClientRect on the host).The the host specifies
 *  what view they wish to display based on the lViewId parameter. If the
 *  value for lViewId is TXTVIEW_ACTIVE, the view referred to is the inplace
 *  active view. TXTVIEW_INACTIVE means some other view such as a print
 *  preview or even printing itself. It is important to note that
 *  TXTVIEW_INACTIVE views may not have scroll bars.
 *
 *  The same comments apply to TxSetCursor and TxQueryHitPoint, discussed
 *  in the following sections.
 */
HRESULT CTxtEdit::TxDraw (
    DWORD    dwDrawAspect,  //@parm Draw aspect
    LONG     lindex,        //@parm Currently unused
    void *   pvAspect,      //@parm Info for drawing optimizations (OCX 96)
    DVTARGETDEVICE *ptd,    //@parm Info on target device
    HDC      hdcDraw,       //@parm Rendering device context
    HDC      hicTargetDev,  //@parm Target information context
    LPCRECTL lprcBounds,    //@parm Bounding (client) rectangle
    LPCRECTL lprcWBounds,   //@parm Clipping rect for metafiles
    LPRECT   lprcUpdate,    //@parm Dirty rectangle inside lprcBounds
    BOOL (CALLBACK * pfnContinue) (DWORD), //@parm Callback for interupting
                            //      long display (currently unused)
    DWORD    dwContinue,    //@parm Parameter to pass to pfnContinue function
    LONG     lViewId )      //@parm View identifier
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxDraw");

    HRESULT hr;

#if !defined(NOMAGELLAN)
    CMagellanBMPStateWrap bmpOff(*this, hdcDraw);
#endif

    // These are rect's are used if we convert the the input parameters if we
    // convert the HDC's mapping mode. The idea here is we avoid changing our
    // clients memory.
    RECTL rcLocalBounds;
    RECTL rcLocalWBounds;
    RECT rcLocalUpdate;
    CCallMgr callmgr(this);

    START_PROFILING

    // if the display is frozen, don't let ourselves draw.  This is a pretty
    // hoaky re-entrancy check.
    // FUTURE (alexgo/ricksa): be better about this.
    if( TXTVIEW_ACTIVE == lViewId && _pdp->IsFrozen() )
    {
        return E_UNEXPECTED;
    }

    if(dwDrawAspect != DVASPECT_CONTENT && dwDrawAspect != DVASPECT_DOCPRINT)
    {
        // We don't support the aspect requested
        return DV_E_DVASPECT;
    }

    if ((!lprcBounds && !_fInPlaceActive) || (hicTargetDev && !ptd))
    {
        // If we are not inplace active we must have a client rectangle
        return E_INVALIDARG;
    }


    HDC hicLocal = NULL;

    // Did they give us a ptd without a hic?
    if (!hicTargetDev && ptd)
    {
        // Create and information context for the device information
        // since it wasn't supplied.
        hicLocal = CreateIC(
            (TCHAR *)((BYTE *) ptd + ptd->tdDriverNameOffset),
            (TCHAR *)((BYTE *) ptd + ptd->tdDeviceNameOffset),
            (TCHAR *)((BYTE *) ptd + ptd->tdPortNameOffset),
            (DEVMODE *)((BYTE *)  ptd + ptd->tdExtDevmodeOffset));
        if (NULL == hicLocal)
        {
            // Couldn't create it
            return E_FAIL;
        }

        hicTargetDev = hicLocal;
    }

    BOOL fRestore = FALSE;

    // Force DC in MM_TEXT
    if(GetMapMode(hdcDraw) != MM_TEXT &&
       GetDeviceCaps(hdcDraw, TECHNOLOGY) != DT_METAFILE)
    {
        fRestore = TRUE;

        // Convert input parameters to new mapping.
        if (lprcBounds)
        {
            rcLocalBounds = *lprcBounds;
            lprcBounds = &rcLocalBounds;
            WinLPtoDP(hdcDraw, (POINT *)lprcBounds, 2);
        }

        if(lprcWBounds)
        {
            rcLocalWBounds = *lprcWBounds;
            lprcWBounds = &rcLocalWBounds;
            WinLPtoDP(hdcDraw, (POINT *)lprcWBounds, 2);
        }

        if(lprcUpdate)
        {
            rcLocalUpdate = *lprcUpdate;
            lprcUpdate = &rcLocalUpdate;
            WinLPtoDP(hdcDraw, (POINT *)lprcUpdate, 2);
        }

        // Convert HDC to new mapping
        ConvertDrawDCMapping(hdcDraw);
    }

    // Preallocate the memory so the set cannnot fail and we don't
    // have to use the heap. Note that all clean up is handled
    // outside of this object. Also note that we don't assign any
    // information here because we may not use this structure. If
    // recursion is happening we will use the top level structure.
    CDrawInfo di(this);

    _pdp->SetDrawInfo(
        &di,
        dwDrawAspect,
        lindex,
        pvAspect,
        ptd,
        hicTargetDev);

    // We use our main display object if we are the active view (which is
    // indicate by the supplied client rectangle) or if the object is
    // inactive and the ptd is NULL. We assume that the ptd being NULL means
    // that the display request is for the screen and not for a print or
    // print preview.
    if ((TXTVIEW_ACTIVE == lViewId) || (NULL == ptd))
    {
        hr = S_FALSE;

        // The main display object draws active views and tries to draw
        // inactive views if the control is not active.
        if ((lprcWBounds == NULL)
            && ((fInplaceActive() && (TXTVIEW_ACTIVE == lViewId))
                || (!fInplaceActive() && (TXTVIEW_INACTIVE == lViewId))))
        {
            hr = _pdp->Draw(
                hdcDraw,
                hicTargetDev,
                (RECT*)lprcBounds,
                (RECT*)lprcWBounds,
                lprcUpdate,
                NULL,                   // We aren't interruptable drawing to
                0);                     // the screen so why pretend.
        }

        if (S_FALSE == hr)
        {
            // This is an inactive view for which the cached state
            // does not match the input request so we make a special
            // object to do the drawing.
            CDisplay *pdp = _pdp->Clone();

            if (pdp != NULL)
            {
                // Force a recalc - this also tells draw to do so no
                // matter what.
                pdp->InvalidateRecalc();

                // Do the draw.
                hr = pdp->Draw(
                    hdcDraw,
                    hicTargetDev,
                    (RECT*)lprcBounds,
                    (RECT*)lprcWBounds,
                    lprcUpdate,
                    NULL,
                    0);
            }

            delete pdp;
        }
    }
    else
    {
        // Make a copy so that we can update it
        if (lprcBounds)
        {
            RECT rcForPrint = *((RECT *) lprcBounds);

            // We want data both formatted and printed
            hr = FormatAndPrint(hdcDraw, hicTargetDev, ptd, &rcForPrint,
                (RECT*)lprcWBounds);

            struct SPrintControl prtcon;

            // This call to OnFormatRange simply cleans up the printer object
            OnFormatRange(NULL, prtcon);
        }
        else
        {
            Assert(lprcBounds);  //if we hit this, we want to be able to understand the path that got us her
        }
    }

    _pdp->ReleaseDrawInfo();

    if (fRestore)
    {
        // Put the DC back into the correct mapping mode.
        RestoreDC(hdcDraw, -1);
    }

    if (hicLocal != NULL)
    {
        // Clean up information context if we created one.
        DeleteDC(hicLocal);
    }


    // an active OLE object might have been dragged/scrolled away
    // from where it belongs. We need to put it back.
    // The _cActiveObjPosTries guards us from an indefinite looop here
    // (OnReposition may post another paint message, and so on)
    COleObject* poleobjActive;
    if (HasObjects() && _cActiveObjPosTries &&
        (poleobjActive = GetObjectMgr()->GetInPlaceActiveObject()))
        {
            // reduce the number of tries
            _cActiveObjPosTries--;

            // get the new object size (we might have resized it,
            // and we don't want to lose that!!)
            poleobjActive->FetchObjectExtents();

            // get the right coordinates for the object
            poleobjActive->ResetPosRect();

            // and put it there!!
            poleobjActive->OnReposition(0, 0);
        }


    return hr;
}

/*
 *  CTxtEdit::TxGetHScroll (plMin, plMax, plPos, plPage, pfEnabled)
 *
 *  @mfunc
 *      Get horizontal scroll bar state information
 *
 *  @rdesc
 *      HRESULT = S_OK
 */
HRESULT CTxtEdit::TxGetHScroll(
    LONG *plMin,        //@parm Minimum scroll position
    LONG *plMax,        //@parm Maximum scroll position
    LONG *plPos,        //@parm Current scroll position
    LONG *plPage,       //@parm View width in pixels
    BOOL *pfEnabled )   //@parm Whether horizonatl scrolling is enabled.
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetHScroll");

    START_PROFILING

    if (plMin)
        *plMin = 0;

    if (plMax)
        *plMax = _pdp->GetScrollRange( SB_HORZ );

    if (plPos)
        *plPos = _pdp->GetXScroll();

    if (plPage)
        *plPage = _pdp->GetViewWidth();

    // CDisplay::_fHScrollEnabled may be TRUE when not in-place active
    // because it has a dual meaning: 1) need Horiz scroll bars, and 2)
    // CDisplay::_xScroll is allowed for ES_AUTOHSCROLL even with no
    // horizontal scrollbar.  The latter can turn on _fHScrollEnabled when
    // the control is active and when the control goes inactive, it stays
    // on, so we say it's off to keep Forms^3 from displaying a horizontal
    // scroll bar. We probably should have two flags: _fHScrollEnabled and
    // _fHScrollbarEnabled, but for now, we stick with one.  No such problem
    // for vertical case, since vertical scrolling always uses a scrollbar.
    if (pfEnabled)
        *pfEnabled = _fInPlaceActive ? _pdp->IsHScrollEnabled() : 0;

    return S_OK;
}

/*
 *  CTxtEdit::TxGetVScroll (plMin, plMax, plPos, plPage, pfEnabled)
 *
 *  @mfunc
 *      Get vertical scroll bar state information
 *
 *  @rdesc
 *      HRESULT = S_OK
 */
HRESULT CTxtEdit::TxGetVScroll(
    LONG *plMin,        //@parm Minimum scroll position
    LONG *plMax,        //@parm Maximum scroll position
    LONG *plPos,        //@parm Current scroll position
    LONG *plPage,       //@parm Height of view in pixels
    BOOL *pfEnabled )   //@parm Whether vertical scroll bar is enabled.
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetVScroll");

    if (plMin)
        *plMin = 0;

    if (plMax)
        *plMax = _pdp->GetScrollRange( SB_VERT );

    if (plPos)
    {
        *plPos = _pdp->GetYScroll();
        *plPos = _pdp->ConvertYPosToScrollPos(*plPos);
    }

    if (plPage)
    {
        *plPage = _pdp->GetViewHeight();
        *plPage = _pdp->ConvertYPosToScrollPos(*plPage);
    }

    if (pfEnabled)
        *pfEnabled = _pdp->IsVScrollEnabled();


    return S_OK;
}

/*
 *  CTxtEdit::OnTxSetCursor (dwDrawAspect, lindex, pvAspect, ptd, hdcDraw,
 *                           hicTargetDev, lprcClient, x, y)
 *  @mfunc
 *      Notification for text services to set the cursor
 *
 *  @rdesc
 *      HRESULT = FAILED(RectChangeHelper()) ? E_INVALIDARG : S_OK
 *
 *  @comm
 *      Text Services may remeasure as a result of this call in
 *      order to determine the correct cursor.  The correct
 *      cursor will be set via ITextHost::TxSetCursor
 *
 *      More details:
 *
 *      The lprcClient parameter is the client rectangle of the view of the
 *      control over which the mouse cursor is. It is in device coordinates
 *      of the containing window in the same way the WM_SIZE message is. This
 *      may not be the view that was rendered last. Furthermore, if the control
 *      is in-place active, this may not be the view currently being active.
 *      As a consequence, Text Services should check this rectangle against
 *      its current caches values and determine whether recalcing the lines
 *      is necessary or not. The zoom factor should be included in this
 *      computation.
 *
 *      This method should only be called for screen views of the control.
 *      Therefore the DC is not passed in but should be assumed to be a screen
 *      DC.
 *
 *      The x and y parameters hold the cursor position in the same coordinate
 *      system as lprcClient, i.e., the client coordinates of the containing
 *      window.
 */
HRESULT CTxtEdit::OnTxSetCursor (
    DWORD   dwDrawAspect,   //@parm Draw aspect
    LONG    lindex,         //@parm Currently unused
    void *  pvAspect,       //@parm Info for drawing optimizations (OCX 96)
    DVTARGETDEVICE *ptd,    //@parm Info on target device
    HDC     hdcDraw,        //@parm Rendering device context
    HDC     hicTargetDev,   //@parm Target information context
    LPCRECT lprcClient,     //@parm Control's client rectangle
    INT     x,              //@parm x position of cursor
    INT     y )             //@parm y position of cursor
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxSetCursor");

    CCallMgr callmgr(this);
    BOOL     fText = FALSE;
    HITTEST  Hit;
    RECT     rcClient;

    START_PROFILING

    CDrawInfo di(this);

    if (FAILED(RectChangeHelper(&di, dwDrawAspect, lindex, pvAspect, ptd,
        hdcDraw, hicTargetDev, lprcClient, &rcClient)))
    {
        return E_INVALIDARG;
    }

    // Set the cursor
    CTxtSelection * const psel = GetSel();
    HCURSOR hcurNew = _hcurArrow;           // Default using system arrow
    POINT   pt = {x, y};
    LONG    xy = x;
    BOOL    fInLink = FALSE;

    // Find out what the cursor is pointing at
    _pdp->CpFromPoint(pt, lprcClient, NULL, NULL, FALSE, &Hit);

    if(Hit == HT_SelectionBar || Hit == HT_LeftOfText)
        hcurNew = _hcurSelBar;

    // this is a bit strange, but RichEdit1.0 does
    // this--give the client a chance to handle the cursor
    // themselves if we are over a link.
    else if(Hit == HT_Link)
    {
        if(HandleLinkNotification(WM_SETCURSOR, 0,
            MAKELPARAM(pt.x, pt.y), &fInLink) )
        {
            return NOERROR;
        }
        hcurNew = _hcurHand;
    }
    else if(Hit != HT_Nothing && Hit != HT_OutlineSymbol && Hit != HT_BulletArea)
    {
        if(!psel || !psel->PointInSel(pt, lprcClient) || _fDisableDrag)
        {
            hcurNew = (Hit == HT_Italic) ? _hcurItalic : _hcurIBeam;
            fText = TRUE;
        }

        //If we have an object manager and if there is a selected object,
        //check for hits on the frame handles.
        if( _pobjmgr )
        {
            COleObject * pobjselect = _pobjmgr->GetSingleSelect();
            LPTSTR idcur;
            HCURSOR hcurObj;

            if( pobjselect )
            {
                // Handle hits on frame handles.
                idcur = pobjselect->CheckForHandleHit(pt);
                hcurObj = sysparam.GetSizeCursor(idcur);
                if( hcurObj )
                {
                    hcurNew = hcurObj;
                }
            }
        }
    }

    // Tell the host to set the cursor
    _phost->TxSetCursor(hcurNew, fText);

    // Release the DC if we got it
    if (hdcDraw)
    {
        _pdp->ResetDC();
    }

    _pdp->ReleaseDrawInfo();

    return S_OK;
}

/*
 *  CTxtEdit::TxQueryHitPoint (dwDrawAspect, lindex, pvAspect, ptd, hdcDraw,
 *                              hicTargetDev, lprcClient, x, y, pHitResult)
 *  @mfunc
 *      Returns whether point is within text services rectangle
 *
 *  @rdesc
 *      HRESULT
 *
 *  @comm
 *      This method allows the host to implement transparent hit-testing
 *      on text.
 *
 *      The lprcClient parameter is the client rectangle in device coordinates
 *      of the view on which hit testing is performed.
 *
 *      The pt parameter hold the position of the cursor in the same
 *      coordinate system as the lprcClient rectangle (the client
 *      coordinates of the containing window).
 *
 *      Same general comments about client rectangle and DC as for
 *      TxSetCursor apply.
 *
 *      pHitResult returns one of the following values: <nl>
 *
 *      TXTHITRESULT_NOHIT       Hit was outside client rectangle. <nl>
 *      TXTHITRESULT_HIT         Point was inside client rectangle and over
 *                               either text or an opaque background.
 *      TXTHITRESULT_TRANSPARENT Point was inside client rectangle with a
 *                               transparent background and not over text.
 *      TXTHITRESULT_CLOSE       Hit was close to an opaque area.
 *
 *      Refer to the Windowless OLE Control spec for more details on
 *      these return values and how they should be determined.
 */
HRESULT CTxtEdit::TxQueryHitPoint(
    DWORD   dwDrawAspect,   //@parm Draw aspect
    LONG    lindex,         //@parm Currently unused
    void *  pvAspect,       //@parm Info for drawing optimizations (OCX 96)
    DVTARGETDEVICE *ptd,    //@parm Info on target device
    HDC     hdcDraw,        //@parm Rendering device context
    HDC     hicTargetDev,   //@parm Target information context
    LPCRECT lprcClient,     //@parm Control's client rectangle
    INT     x,              //@parm x coordinate to check
    INT     y,              //@parm y coordinate to check
    DWORD * pHitResult )    //@parm Result of hit test see TXTHITRESULT
                            //      enumeration for valid values
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxQueryHitPoint");

    RECT     rcClient;
    CCallMgr callmgr(this);

    START_PROFILING

    CDrawInfo di(this);

    if (FAILED(RectChangeHelper(&di, dwDrawAspect, lindex, pvAspect, ptd,
        hdcDraw, hicTargetDev, lprcClient, &rcClient)))
    {
        return E_INVALIDARG;
    }

    HRESULT hr;
    POINT   pt = {x, y};

    if(!_fTransparent)
    {
        *pHitResult = TXTHITRESULT_HIT;
        hr = S_OK;
    }
    else
    {
        hr = _pdp->TransparentHitTest(hdcDraw, lprcClient, pt, pHitResult);
    }

    // Release the DC if we got it
    if (hdcDraw)
    {
        _pdp->ResetDC();
    }

    _pdp->ReleaseDrawInfo();
    return hr;
}

/*
 *  CTxtEdit::OnTxInPlaceActivate (prcClient)
 *
 *  @mfunc
 *      Notifies text services that this control is inplace active
 *
 *  @rdesc
 *      S_OK - successfully activated object <nl>
 *      E_FAIL - could not activate object due to error. <nl>
 *
 *  @comm
 *      When transitioning directly from a non-active state to the UI-active
 *      state, the host should call OnTxInPlaceActivate first and then
 *      OnTxUIActivate. Similarly, when transitioning from the UI-active
 *      state to a non active state, the host should call OnTxUIDeactivate
 *      first and then OnTxInPlaceDeactivate.
 *
 *      OnTxInPlaceActivate takes the client rectangle of the view being
 *      activated as a parameter. This rectangle is given in client coordinate
 *      of the containing window. It is the same as would be obtained by
 *      calling TxGetClientRect on the host.
 *
 *      UI-activation is different from getting the focus. To let Text
 *      Services know that the control is getting or losing focus, the
 *      host will send WM_SETFOCUS and WM_KILLFOCUS messages. Note that a
 *      windowless host will pass NULL as the wParam (window that lost the
 *      focus) for these messages.
 *
 *      As a reminder, inplace activation typically refers to an embedded
 *      object "running inplace" (for regular controls && embeddings, it
 *      would have a window to draw in, for example).  UI active means that
 *      an object currently has the 'editing focus'.  Specifically, things
 *      like menus and toolbars on the container may also contain elements
 *      from the UI active control/embedding.  There can only be one
 *      UI active control at any given time, while many can be inplace active
 *      at once.
 */
HRESULT CTxtEdit::OnTxInPlaceActivate(
    const RECT *prcClient ) //@parm Control's client rectangle
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxInPlaceActivate");

    RECT rcView;
    HDC hdc;
    BOOL fSucceeded = TRUE;
    CCallMgr callmgr(this);

    START_PROFILING

    // Needs to set here for further TxGetDC() to work
    _fInPlaceActive = TRUE;

    // Set rendering DC to be the screen
    hdc = TxGetDC();

    if( !hdc )
    {
        goto err;
    }

    // Tell display that it is active
    _pdp->SetActiveFlag(TRUE);
    _pdp->SetDC(hdc);

    // Compute view rect from passsed in client rect
    _pdp->GetViewRect(rcView, prcClient);

    // Recalc/update view
    _pdp->RecalcView(rcView);

    // Init selection if there is anything displayed
    if(_pdp->GetViewWidth() != 0)
    {
        // Get the selection if we can

        if (GetSel() != NULL)
        {
            // Set the caret
            _psel->Update(FALSE);
        }
        else
        {
            // We tried but could not create a selection
            // so we fail the activation.
            fSucceeded = FALSE;
        }
    }
    else if (!_psel)
    {
        // Get the selection.  Otherwise, if SetFocus is called later without
        // selection, then the Selection state is not set correctly.
        GetSel();
    }

    // Release the DC
    TxReleaseDC(hdc);
    _pdp->SetDC(NULL);

    // If getting the selection worked we are home free
    if (fSucceeded)
    {
        return S_OK;
    }

err:
    _fInPlaceActive = FALSE;
    return E_FAIL;
}

/*
 *  CTxtEdit::OnTxInPlaceDeactivate()
 *
 *  @mfunc  Notifies text services that this is no longer in place active.
 *
 *  @rdesc  S_OK
 *
 *  @comm   See OnTxInPlaceActivate for a detailed description of
 *  activation/deactivation.
 *
 *  @xref <mf CTxtEdit::OnTxInPlaceActivate>
 *
 */
HRESULT CTxtEdit::OnTxInPlaceDeactivate()
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxInPlaceDeactivate");

    START_PROFILING

    // Get properties that affect whether we will discard the selection
    DWORD dwBits;

    // Tell display that it is not longer active
    _pdp->SetActiveFlag(FALSE);

    // Because we are inactive, this will tell any background recalc going
    // on to stop.
    _pdp->StepBackgroundRecalc();

    _phost->TxGetPropertyBits(TXTBIT_HIDESELECTION | TXTBIT_SAVESELECTION,
        &dwBits);

    // If we don't want to save the selection and we want to hide it while
    // inactive, then we discard our selection
    if (!(dwBits & TXTBIT_SAVESELECTION) && (dwBits & TXTBIT_HIDESELECTION))
    {
        DiscardSelection();
    }

    _fInPlaceActive = FALSE;

    return S_OK;
}

/*
 *  CTxtEdit::OnTxUIActivate()
 *
 *  @mfunc  Informs text services that the control is now UI active.
 *
 *  @rdesc  S_OK
 *
 *  @comm   See OnTxInPlaceActivate for a detailed description of
 *  activation/deactivation.
 *
 *  @xref <mf CTxtEdit::OnTxInPlaceActivate>
 *
 *
 */
HRESULT CTxtEdit::OnTxUIActivate()
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxUIActivate");

    return S_OK;
}

/*
 *  CTxtEdit::OnTxUIDeactivate()
 *
 *  @mfunc  Informs text services that the control is now UI deactive.
 *
 *  @rdesc  S_OK
 *
 *  @comm   See OnTxInPlaceActivate for a detailed description of
 *  activation/deactivation.
 *
 *  @xref <mf CTxtEdit::OnTxInPlaceActivate>
 *
 */
HRESULT CTxtEdit::OnTxUIDeactivate()
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxUIDeactivate");

    return S_OK;
}

/*
 *  CTxtEdit::TxGetText (pbstrText)
 *
 *  @mfunc  Returns all of the UNICODE plain text in the control as an
 *          OLE BSTR.
 *
 *  @rdesc
 *      S_OK - Text successfully returned in the output argument <nl>
 *      E_INVALIDARG - invalid BSTR pointer passed in. <nl>
 *      E_OUTOFMEMORY - could not allocate memory for copy of the text <nl>
 *
 *  @comm   The caller takes ownership of the returned BSTR.  WM_GETTEXT
 *          and TOM ITextRange::GetText are alternate techniques for
 *          retrieving plain text data.
 *
 *          If there is no text in the control, no BSTR will be allocated
 *          and NULL will be returned.
 *
 *          The returned text will NOT necessarily be NULL terminated.
 */
HRESULT CTxtEdit::TxGetText(
    BSTR *pbstrText )   //@parm where to return an allocated BSTR
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetText");

    CTxtPtr     tp(this, 0);
    CCallMgr callmgr(this);

    START_PROFILING

    if (!pbstrText)
        return E_INVALIDARG;

    const LONG cch = GetTextLength();

    if (cch <= 0)
    {
        *pbstrText = 0;
        return S_OK;
    }

    *pbstrText = pSysAllocStringLen(NULL, cch);

    if(!*pbstrText)
    {
        GetCallMgr()->SetOutOfMemory();
        return E_OUTOFMEMORY;
    }

    tp.GetText(cch, *pbstrText);

    return S_OK;
}

/*
 *  CTxtEdit::TxSetText (pszText)
 *
 *  @mfunc  Sets all of the text in the control
 *
 *  @rdesc
 *      S_OK - text was successfully set    <nl>
 *      E_FAIL - text could not be updated. <nl>
 *
 *  @comm
 *      This method should be used with care; it essentially re-initializes
 *      the text engine with some new data; any previous data and formatting
 *      information will be LOST, including undo information.
 *
 *      If previous data has been copied to the clipboard, that data will be
 *      rendered completely to the clipboard (via OleFlushClipboard) before
 *      it is discarded.
 *
 *      This method is NOT undo-able.
 *
 *      Two alternate approaches to setting text are WM_SETTEXT and TOM
 *      ITextRange::SetText.
 *
 *  @xref
 *      <mf CTxtRange::SetText>
 */
HRESULT CTxtEdit::TxSetText(
    LPCTSTR pszText )       //@parm String to replace the current text with
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxSetText");

    START_PROFILING

    return SetText(pszText, CHECK_PROTECTION);
}

/*
 *  CTxtEdit::TxGetCurTargetX (px)
 *
 *  @mfunc
 *      Get the target x position of the caret
 *
 *  @rdesc
 *      HRESULT with possible values:
 *
 *      S_OK         - x position of the caret returned <nl>
 *      E_FAIL       - There is no selection <nl>
 *      E_INVALIDARG - Input argument is invalid <nl>
 *
 *  @comm
 *      This method is useful for implementing up-down cursoring
 *      through a conceptual vertical line.  To illustrate this feature,
 *      consider setting the insertion point at, say, column 20 in a
 *      text editor.  Now cursor up and down--notice that wherever possible,
 *      the editor tries to put the insertion point as close to column 20
 *      as it can for the current line.  Column 20 is thus the "target" column
 *      for the insertion point.
 *
 *      Users would like to have this same capability when cursoring through
 *      Forms; however, as other controls don't necessarily share the same
 *      notion of column position, the target caret position is expressed simply
 *      as an x-coordinate on the display (in *client* coordinates).
 */
HRESULT CTxtEdit::TxGetCurTargetX(
    LONG *px )          //@parm the x location in client coordinates
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetCurTargetX");

    START_PROFILING
    CTxtSelection *psel = GetSel();

    if (!psel)
        return E_FAIL;

    if (!px)
        return E_INVALIDARG;

    *px = psel->GetXCaretReally();

    return S_OK;
}

/*
 *  CTxtEdit::TxGetBaseLinePos(pBaseLinePos)
 *
 *  @mfunc  Get the base line position of the first visible line, in pixels,
 *  relative the TS client rectangle. Needed for aligning controls on their
 *  baselines.
 *
 *  @rdesc  HRESULT = E_NOTIMPL
 */
HRESULT CTxtEdit::TxGetBaseLinePos(
    LONG *pBaseLinePos)     //@parm Where to return baseline position
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetBaseLinePos");

    return E_NOTIMPL;
}

/*
 *  CTxtEdit::TxGetNaturalSize (dwAspect, hdcDraw, hicTargetDev, ptd, dwMode,
 *                              psizelExtent, pwidth, pheight)
 *
 *  @mfunc  Allow control to be resized so it fits content appropriately
 *
 *  @rdesc  S_OK            <nl>
 *          E_INVALIDARG    <nl>
 *          E_FAIL  Unable to determine correct size    <nl>
 *          E_OUTOFMEMORY   <nl>
 *
 *  @comm
 *
 *  The first 4 parameters are similar to equivalent parameters in
 *  TxDraw and give the same information. In the case TS needs to
 *  recalc lines, it should use these values the same ways as in
 *  TxDraw.
 *
 *  <p pWidth> and <p pHeight> are IN/OUT parameters. The host passes the
 *  "tentative" width and height of the client rectangle for the text object
 *  and Text Services will compare these values against its current cached
 *  state, and if different should recalc lines. Then it will compute
 *  and return the natural size. As spec-ed currently, the host can ask for 2
 *  different kinds of natural sizes:
 *
 *  TXTNS_FITTOCONTENT: the entire text should be formatted to the
 *  width that is passed in.Then Text Services return the height of
 *  the entire text and the width of the widest line. Note that this
 *  option ignores any paragraph formatting such as centering and
 *  only returns the raw size for the text.
 *
 *  TXTNS_ROUNDTOLINE: returns the integral height of the number of lines that
 *  will fit in the input height rounded to the next full line boundary.
 *
 *  Note that passed and returned width and height correspond to
 *  the *client* rectangle in client units.
 *
 *
 *  BACKGROUND
 *  Here is a quick description of the features mentioned above:
 *
 *  FITTOCONTEXT: Normally happens when the user double clicks one
 *  of the control grab handles. Sizes the control to the "optimal"
 *  size to fit the entire content. Should accomodate the height of
 *  the entire text and the width of the widest line.
 *
 *  ROUNDTOLINE (Integral height): if this property is set, when the
 *  user resizes the control, it snaps to heights that allow an
 *  integral number of lines to be displayed (no line will be clipped).
 */
HRESULT CTxtEdit::TxGetNaturalSize(
    DWORD   dwAspect,   //@parm Aspect for drawing.  Values taken from OLE's
                        //      DVASPECT enumeration
    HDC     hdcDraw,    //@parm DC into which drawing would occur
    HDC hicTargetDev,   //@parm DC for which text should be formatted, i.e.,
                        //      for WYSIWYG
    DVTARGETDEVICE *ptd,//@parm More info on the target device
    DWORD   dwMode,     //@parm Type of fitting requested.  Either
                        //      TXTNS_FITTOCONTENT or TXTNS_ROUNDTOLINE
    const SIZEL *psizelExtent,//@parm Size of extent to use for zooming
    LONG *  pwidth,     //@parm Width for such a fitting [in,out]
    LONG *  pheight )   //@parm Height for such a fitting [in,out]
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetNaturalSize");

    HRESULT hr;
    CCallMgr callmgr(this);

    START_PROFILING

    if (dwAspect != DVASPECT_CONTENT &&
        dwAspect != DVASPECT_DOCPRINT)
    {
        // We don't support the aspect requested
        return DV_E_DVASPECT;
    }

    if ((hicTargetDev && !ptd) ||
        (dwMode != TXTNS_FITTOCONTENT && dwMode != TXTNS_ROUNDTOLINE) ||
        !pwidth || !pheight)
    {
        // Either and information context is provided without the device
        // target or the mode is not valid or the width was not provided
        // or the height was not provided. In short, the input parameters
        // are not valid so tell the caller.
        return E_INVALIDARG;
    }

    if (0 == psizelExtent->cy)
    {
        // No extent for the control so we just return 0
        *pwidth = 0;
        *pheight = 0;
        return S_OK;
    }


    HDC hicLocal = NULL;

    // Did they give us a ptd without a hic?
    if (!hicTargetDev && ptd)
    {
        // Create and information context for the device information
        // since it wasn't supplied.
        hicLocal = CreateIC(
            (TCHAR *)((BYTE *) ptd + ptd->tdDriverNameOffset),
            (TCHAR *)((BYTE *) ptd + ptd->tdDeviceNameOffset),
            (TCHAR *)((BYTE *) ptd + ptd->tdPortNameOffset),
            (DEVMODE *)((BYTE *)  ptd + ptd->tdExtDevmodeOffset));

        if (NULL == hicLocal)
        {
            // Couldn't create it
            return E_FAIL;
        }

        hicTargetDev = hicLocal;
    }

    // Assume that we need to change mapping modes. Note that we need to be
    // in MM_TEXT because our measuring assumes this is so.
    BOOL fRestore = FALSE;

    // Convenient place to put height & width for converting them to
    // device units.
    POINT pt;
    pt.x = *pwidth;
    pt.y = *pheight;

    // WARNING: From this point on we expect to execute this routine clear
    // through so that the DC state can be reset if we set it and because
    // we reset the extents so that the zoom factor can be calculated
    // appropriately for this call. Therefore, be very careful about
    // any return statements you might want to add.

    // force DC in MM_TEXT
    if(GetMapMode(hdcDraw) != MM_TEXT &&
        GetDeviceCaps(hdcDraw, TECHNOLOGY) != DT_METAFILE)
    {
        WinLPtoDP(hdcDraw, &pt, 1);         // Convert to device units
        fRestore = TRUE;                    // Remember that DC got converted
        ConvertDrawDCMapping(hdcDraw);
    }

    // Set the extent information needed for zooming
    _pdp->SetTempZoomDenominator(psizelExtent->cy);

    if (TXTNS_ROUNDTOLINE == dwMode)
    {
        // Round to fit simply calculates the
        hr = _pdp->RoundToLine(hdcDraw, pt.x, &pt.y);
    }
    else
    {
        // Get natural size for entire presentation

        // Allocate memory for the draw information
        CDrawInfo di(this);

        // Set up the drawing parameters
        _pdp->SetDrawInfo(
            &di,
            dwAspect,
            -1,
            NULL,
            ptd,
            hicTargetDev);

        // Set the Draw DC
        _pdp->SetDC(hdcDraw);

        // Tell the display to figure the size needed for this display
        hr = _pdp->GetNaturalSize(hdcDraw, hicTargetDev, dwMode, &pt.x,
            &pt.y);

        // Restore the state.
        _pdp->ResetDC();

        _pdp->ReleaseDrawInfo();
    }

    if (fRestore)
    {
        // Put the DC back into the correct mapping mode.
        RestoreDC(hdcDraw, -1);
    }

    // Set the return values if this worked
    if (SUCCEEDED(hr))
    {
        if (fRestore)
        {
            // Convert return value back to input logical units if we
            // converted them to use them.
            WinDPtoLP(hdcDraw, &pt, 1);
        }

        // Update return values
        *pwidth = pt.x;
        *pheight = pt.y;
    }

    if (hicLocal)
    {
        // Clean up information context if we created one.
        DeleteDC(hicLocal);
    }

    // Reset the temporary zoom factor
    _pdp->ResetTempZoomDenominator();

    return hr;
}

/*
 *  CTxtEdit::TxGetDropTarget (ppDropTarget)
 *
 *  @mfunc  Get the drop target for the text control
 *
 *  @rdesc
 *      S_OK - Got drop target successfully <nl>
 *      E_OUTOFMEMORY - Could not create drop target <nl>
 *
 *  @comm
 *      The caller (host) is responsible for calling Register/Revoke
 *      DragDrop and for calling IUnknown::Release on the returned
 *      drop target when done.
 */
HRESULT CTxtEdit::TxGetDropTarget(
    IDropTarget **ppDropTarget) //@parm Where to put pointer to drop target
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetDropTarget");

    HRESULT hr;
    CCallMgr callmgr(this);

    START_PROFILING

    hr = _ldte.GetDropTarget(ppDropTarget);
    if( hr == NOERROR )
    {
        (*ppDropTarget)->AddRef();
    }
    return hr;
#else
    return 0;
#endif
}

/*
 *  CTxtEdit::OnTxPropertyBitsChange (dwMask, dwBits)
 *
 *  @mfunc  Set properties that can be represented by bits.
 *
 *  @rdesc  HRESULT
 *
 *  @comm   The following property bits are understood: <nl>
 *
 *      TXTBIT_RICHTEXT             <nl>
 *      TXTBIT_MULTILINE            <nl>
 *      TXTBIT_READONLY             <nl>
 *      TXTBIT_SHOWACCELERATOR      <nl>
 *      TXTBIT_USEPASSWORD          <nl>
 *      TXTBIT_HIDESELECTION        <nl>
 *      TXTBIT_SAVESELECTION        <nl>
 *      TXTBIT_AUTOWORDSEL          <nl>
 *      TXTBIT_AUTOSIZE             <nl>
 *      TXTBIT_VERTICAL             <nl>
 *      TXTBIT_SELECTIONBAR         <nl>
 *      TXTBIT_WORDWRAP             <nl>
 *
 *      TXTBIT_CLIENTRECTCHANGE     <nl>
 *      TXTBIT_VIEWINSETCHANGE      <nl>
 *      TXTBIT_BACKSTYLECHANGE      <nl>
 *      TXTBIT_MAXLENGTHCHANGE      <nl>
 *      TXTBIT_SCROLLBARCHANGE      <nl>
 *      TXTBIT_CHARFORMATCHANGE     <nl>
 *      TXTBIT_PARAFORMATCHANGE     <nl>
 *      TXTBIT_ALLOWBEEP            <nl>
 *      TXTBIT_EXTENTCHANGE         <nl>
 *
 *  A brief description of each property follows:
 *
 *
 *  Client rectangle (TXTBIT_CLIENTRECTCHANGE):
 *
 *  The rectangle the Text Services are responsible for painting
 *  and managing. The host will rely on the Text Services for painting
 *  that area. Text Services must not paint or invalidate areas outside of
 *  that rectangle.
 *
 *  The host will forward mouse messages to the Text Services whenever the
 *  cursor is over this rectangle.
 *
 *  This rectangle is expressed in client coordinates of the containing window.
 *
 *  IMPORTANT: this property cannot be queried from the host when it is
 *  inactive. The TxGetClientRect method will fail if called at inactive time.
 *
 *
 *  View inset (TXTBIT_VIEWINSETCHANGE):
 *
 *  This is the amount of space on each side between the client rectangle and
 *  the view rectangle. The view rectangle (also called Formating rectangle)
 *  is the rectangle the text should be formatted in.
 *
 *  The view insets are is passed in a RECT structure but this is not really
 *  a rectangle. It should be treated as 4 independent values to substract
 *  on each side of the client rectangle to figure the view rectangle.
 *
 *  The view insets are passed in himetrics so that they do not depend on
 *  the client rectangle and the rendering DC.
 *
 *  View insets can be negative on either side of the client rectangle,
 *  leading to a bigger view rectangle than the client rectangle. The text
 *  should then be clipped to the client rectangle. If the view rectangle
 *  is wider than the client rectangle, then the host may add a horizontal
 *  scrollbar to the control.
 *
 *  Single line Text Services ignore the right boundary of the view rectangle
 *  when formatting text.
 *
 *  The view inset is available from the host at all times, active or
 *  inactive.
 *
 *
 *  Backstyle (TXTBIT_BACKSTYLECHANGE):
 *
 *  The style of the background of the client rectangle. Can be either of
 *  the following values: <nl>
 *      #define TXTBACK_TRANSPARENT     0 <nl>
 *      #define TXTBACK_SOLID           1 <nl>
 *
 *  Values for this property are similar to VB4 values for the same property.
 *
 *
 *  MaxLength (TXTBIT_MAXLENGTHCHANGE):
 *
 *  The maximum length the text can have. Text Services should reject
 *  character insertion and  pasted text when this maximum is reached.
 *  TxSetText however should still accept (and set) text longer than the
 *  maximum length. This is because this method is used for binding and
 *  it is critical to maintain the integrity of the data the control
 *  is bound to.
 *
 *
 *  Scrollbar (TXTBIT_SCROLLBARCHANGE):
 *
 *  This property indicates which scollbar is present and whether scollbars
 *  are hidden or disabled when scrolling is impossible. It also controls
 *  auto-scrolling when the insertion point gets off the client rectangle.
 *
 *  This is a DWORD where bits are layed out as in the system window style.
 *  Possible bits are:
 *  WS_HSCROLL              // control has horizontal scrollbar <nl>
 *  WS_VSCROLL              // control has vertical scrollbar <nl>
 *  ES_AUTOVSCROLL          // auto-scroll horizontally <nl>
 *  ES_AUTOVSCROLL          // auto-scroll vertically <nl>
 *  ES_DISABLENOSCROLL      // scrollbar should be disabled when scrolling
 *                             impossible <nl>
 *
 *  Default CHARFORMAT (TXTBIT_CHARFORMATCHANGE):
 *
 *  The CHARFORMAT or CHARFORMAT2 used for default character-format runs,
 *  i.e., those not explicitly formatted via the selection or TOM methods.

 *
 *  Default PARAFORMAT (TXTBIT_PARAFORMATCHANGE):
 *
 *  The PARAFORMAT or PARAFORMAT2 used for default paragraph-format runs,
 *  i.e., those not explicitly formatted via the selection or TOM methods.
 *
 *
 *  TXTBIT_ALLOWBEEP:
 *
 *  TXTBIT_EXTENTCHANGE:
 *
 *
 *  TXTBIT_RICHTEXT:
 *
 *  Whether the Text Services should be in Rich-Text mode or not.  This
 *  principally affects how editing commands are applied.  For example,
 *  applying bold to some text in a plain edit control makes all of the
 *  text bold, rather than just the selected text in a rich text control.
 *
 *  Note that if there is either undo state or the object has any text,
 *  the attempt to change this bit will be ignored.
 *
 *
 *  TXTBIT_MULTILINE:
 *
 *  If this property is FALSE, Text Services should not process the CR
 *  key and truncate any incoming text containing hard line breaks just
 *  before the first line break. It is OK to also truncate text set via
 *  TxSetText (meaning, it is the responsibility of the host to not use a s
 *  single line control when bound to a multi-line field).
 *
 *  If this property is TRUE, Text Services should work in multiline mode.
 *  The TXTBIT_WORDWRAP can be used to know whether to wrap the lines to
 *  the view rectangle or clip them.
 *
 *
 *  TXTBIT_READONLY:
 *
 *  If this property is TRUE, Text Services should not accept any editing
 *  change via the user interface. However, they should still accept
 *  programmatic changes via EM_SETTEXT, EM_REPLACETEXT and TxSetText.
 *
 *  In read only mode, the user should still be able to move the
 *  insertion point, select text and carry out other non content modifying
 *  operations such as Copy.
 *
 *
 *  TXTBIT_SHOWACCELERATOR:
 *
 *  Refer to the "Accelerator" section for details about this property.
 *
 *
 *  TXTBIT_USEPASSWORD:
 *
 *  If this property is TRUE, the Text Services should show the entire
 *  text using the character obtained by TxGetPasswordChar.
 *
 *  The notification on this property may mean two different things:
 *    The password character changed,
 *    The password character was not used before and is used now
 *  (or vice versa).
 *
 *
 *  TXTBIT_HIDESELECTION:
 *
 *  If this property is TRUE, Text Services should hide the selection
 *  when the control is inactive. If it is FALSE, the selection, if any,
 *  should still be displayed when the control is inactive.
 *
 *  If TRUE, this property implies TXTBIT_SAVESELECTION = TRUE.
 *
 *
 *  TXTBIT_SAVESELECTION:
 *
 *  If this property is TRUE, Text Services should remember the
 *  boundaries of the selection when the control goes inactive. If FALSE,
 *  it is not necessary to remember the selection when the control goes
 *  inactive. It can be reset to start = 0, length = 0 when the control
 *  goes active again.
 *
 *  This property is used by hosts for which it is not necessary to
 *  remember the selection when inactive.
 *
 *
 *  TXTBIT_AUTOWORDSEL:
 *
 *  This property turns the AutoWordSelect feature on or off.
 *
 *
 *  TXTBIT_AUTOSIZE:
 *
 *  This property turns the AutoSize feature on or off. Refer to the
 *  "AutoSize" section for more details.
 *
 *
 *  TXTBIT_VERTICAL:
 *
 *  This property turns on vertical writing. Used for FE support.
 *  Details TBD.
 *
 *
 *  TXTBIT_WORDWRAP:
 *
 *  If this property is TRUE and MultiLine is also TRUE, then Text Services
 *  should wrap the line to the view rectangle. If this property is FALSE,
 *  the lines should not be wrapped but clipped. The right side of the
 *  view rectangle should be ignored.
 *
 *  If the MultiLine property is off, this property has no effect.
 *
 *
 *  TXTBIT_LINESELECTION:
 *
 *  This property turns on or off the Line Selection feature. This feature
 *  enable the user to select lines or paragraph by placing the mouse cursor
 *  over a "line selection" area on the left of the control. The cursor is
 *  displayed as a NE arrow in that area. If the Line Selection feature is
 *  off, that area should not be shown.
 *
 */
HRESULT CTxtEdit::OnTxPropertyBitsChange(
    DWORD dwMask,           //@parm Bits representing properties to be changed
    DWORD dwBits )          //@parm New values for bit properties
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::OnTxPropertyBitsChange");

    HRESULT hr = E_FAIL;
    DWORD dwLoopMask = dwMask;
    CCallMgr callmgr(this);

    START_PROFILING

    for (int i = 0; (i < MAX_PROPERTY_BITS) && (dwLoopMask != 0);
        i++, dwLoopMask >>= 1)
    {
        if (dwLoopMask & 1)
        {
            hr = (this->*_fnpPropChg[i])((dwBits & (1 << i)) != 0);

            if (FAILED(hr))
            {
                return hr;
            }
        }
    }
    return S_OK;
}

/*
 *  CTxtEdit::TxGetCachedSize (pdwWidth, pdwHeight)
 *
 *  @mfunc
 *      Returns the cached drawing size (if any) that text services
 *      is using.  Typically, this will be the size of the last client
 *      rect used in TxDraw, TxSetCursor, etc., although it is not
 *      guaranteed to be.
 *
 *  @rdesc
 *      HRESULT
 *
 *  @comm
 *      This information is provided to allow the host to potentially
 *      perform various optimizations, amongst other things.
 */
HRESULT CTxtEdit::TxGetCachedSize(
    DWORD *pdwWidth,    //@parm Where to put width (in client coords)
    DWORD *pdwHeight)   //@parm Where to put height (in client coords)
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CTxtEdit::TxGetCachedSize");

    return _pdp->GetCachedSize(pdwHeight, pdwWidth);
}


/*
 *  CreateTextServices (punkOuter, phost, ppserv)
 *
 *  @func
 *      Create an instance of the RichEdit Engine.  This engine supports the
 *      ITextServices and Microsoft Text Object Model (TOM) interfaces.
 *
 *  @rdesc
 *      S_OK - New text services instance created successfully. <nl>
 *      E_INVALIDARG - An invalid argument was passed in. <nl>
 *      E_OUTOFMEMORY - Memory for text services object could not be allocated. <nl>
 *      E_FAIL - Text services could not be initialized
 *
 *  @comm
 *      Text Services may be created as a standard OLE aggregated object.
 *      Callers should follow standard OLE32 rules for dealing with
 *      aggregated objects and caching interface pointers obtained via
 *      QueryInterface from the private IUnknown.
 */
HRESULT CreateTextServices(
    IUnknown *punkOuter,    //@parm Outer unknown, may be NULL
    ITextHost *phost,       //@parm Client's ITextHost implementation; must be
                            //      valid
    IUnknown **ppUnk )      //@parm Private IUnknown of text services engine
{
    TRACEBEGIN(TRCSUBSYSTS, TRCSCOPEEXTERN, "CreateTextServices");

    if(!ppUnk)
        return E_INVALIDARG;

    CTxtEdit *ped = new CTxtEdit(phost, punkOuter);
    if(!ped)
        return E_OUTOFMEMORY;

    if(ped->Init(NULL))
    {
        *ppUnk = ped->GetPrivateIUnknown();
        return S_OK;
    }

    delete ped;
    return E_FAIL;
}
