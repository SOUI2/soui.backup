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
 *  _w32sys.h
 *
 *  Purpose:
 *      Isolate various Win 32 system dependencies.
 *
 */

#ifndef _W32SYS_H

#define _W32SYS_H

#if defined(PEGASUS)
#if !defined(WINNT)
#include "memory.h"                             // for memmove
#endif
#define NOMAGELLAN                              // No Magellan on Win CE
#define NODROPFILES                             // No drop files support on Win CE
#define NOMETAFILES                             // No metafiles on Win CE
#define NOPEDDUMP                               // No support for ped debug dump on CE
#define NOFONTSUBINFO                           // Avoid reading fontsubinfo profile on CE
#define NODUMPFORMATRUNS                        // Don't dump formatting if we run out of memory.
#define CONVERT2BPP

#define dxCaret     2                           // caret width
#define FONTCACHESIZE 8
#define DEFAULT_UNDO_SIZE 20

#else

#define dxCaret     1                           // caret width
#define FONTCACHESIZE 16
#define DEFAULT_UNDO_SIZE 100

#endif  // defined(PEGASUS)

/*
 *  GetCaretDelta ()
 *
 *  @func   Get size of caret to add to current caret position to get the
 *  maximum extent needed to display caret.
 *
 *  @rdesc  Size of caret over 1 pixel
 *
 *  @devnote    This exists solely to abstract this calculation
 *  to handle a variable size caret.
 */
inline int GetCaretDelta()
{
    return dxCaret - 1;
}


// Used in rtfread.cpp to keep track of lossy rtf.
#ifdef PWORD_CONVERTER_V2
#define REPORT_LOSSAGE
#endif

#define SYMBOL_CODEPAGE     2
const SHORT sLanguageEnglishUS = 0x0409;
const SHORT sLanguageMask    =  0x03ff;
const SHORT sLanguageArabic  =  0x0401;
const SHORT sLanguageHebrew  =  0x040d;
// FUTURE: currently this const == sLanguageEnglishUS
//          for no reason except that it was this way
//          in RE1.0 BiDi. Consider changing, or sticking
//          the real language in, and changing the logic
//          of handling wLang a bit.
const SHORT sLanguageNonBiDi =  0x0409;


// Logical unit definition
const int LX_PER_INCH = 1440;
const int LY_PER_INCH = 1440;

// HIMETRIC units per inch (used for conversion)
const int HIMETRIC_PER_INCH = 2540;

#ifdef CopyMemory
#undef CopyMemory
#endif
#ifdef MoveMemory
#undef MoveMemory
#endif
#ifdef FillMemory
#undef FillMemory
#endif
#ifdef ZeroMemory
#undef ZeroMemory
#endif
#ifdef CompareMemory
#undef CompareMemory
#endif

#ifndef KF_ALTDOWN
#define KF_ALTDOWN    0x2000
#endif

// Use for our version of ExtTextOut
enum CONVERTMODE
{
    CM_NULL = CM_NONE,  // Use Unicode (W) CharWidth/TextOut APIs
    CM_WCTMB,           // Convert to MBCS using WCTMB and _wCodePage
    CM_LOWBYTE          // Use low byte of 16-bit chars (for SYMBOL_CHARSET
};                      //  and when code page isn't installed)

// Opaque Type
class CTxtSelection;
class CTxtEdit;
class CCharFormat;
class CCcs;
struct IMESTYLE;
struct IMECOLORSTY;

enum UN_FLAGS
{
    UN_NOOBJECTS                = 1,
    UN_CONVERT_WCH_EMBEDDING    = 2
};

#undef GetStringTypeEx
#undef CharLower
#undef CharLowerBuff
#undef CharUpperBuff
#undef CreateIC
#undef CreateFile
#undef CreateFontIndirect
#undef CompareString
#undef DefWindowProc
//#undef GetKeyboardLayout
#undef GetProfileSection
#undef GetTextExtentPoint32
#undef GetTextFace
#undef GetWindowLong
#undef LoadBitmap
#undef LoadCursor
#undef LoadLibrary
#undef SendMessage
#undef SetWindowLong
#undef PostMessage
#undef lstrcmp
#undef lstrcmpi
#undef PeekMessage
#undef GetModuleFileName

#undef GlobalAlloc
#undef GlobalFree
#undef GlobalReAlloc
#undef GlobalLock
#undef GlobalHandle
#undef GlobalUnlock
#undef GlobalFlags
#undef GlobalSize

class CW32System
{
public :
    static BOOL         _fHaveIMMProcs;
    static DWORD        _dwPlatformId;          // platform GetVersionEx();
    static DWORD        _dwMajorVersion;        // major version from GetVersionEx()
    static INT          _icr3DDarkShadow;       // value to use for COLOR_3DDKSHADOW

    CW32System();

    ~CW32System();

    static LRESULT WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static LRESULT ANSIWndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static HGLOBAL WINAPI GlobalAlloc( UINT uFlags, DWORD dwBytes );
    static HGLOBAL WINAPI GlobalFree( HGLOBAL hMem );
    static UINT WINAPI GlobalFlags( HGLOBAL hMem );
    static HGLOBAL WINAPI GlobalReAlloc( HGLOBAL hMem, DWORD dwBytes, UINT uFlags );
    static DWORD WINAPI GlobalSize( HGLOBAL hMem );
    static LPVOID WINAPI GlobalLock( HGLOBAL hMem );
    static HGLOBAL WINAPI GlobalHandle( LPCVOID pMem );
    static BOOL WINAPI GlobalUnlock( HGLOBAL hMem );
    static BOOL WINAPI REGetCharWidth(
        HDC hdc,
        UINT iChar,
        LPINT pAns,
        UINT uiCodePage);
    static BOOL WINAPI REExtTextOut(
        CONVERTMODE cm,
        UINT uiCodePage,
        HDC hdc,
        int x,
        int y,
        UINT fuOptions,
        CONST RECT *lprc,
        const WCHAR *lpString,
        UINT cbCount,
        CONST INT *lpDx,
        BOOL  FEFontOnNonFEWin95);
    static CONVERTMODE WINAPI DetermineConvertMode( BYTE tmCharSet );
    static void WINAPI CalcUnderlineInfo( CCcs *pcccs, TEXTMETRIC *ptm );
    static BOOL WINAPI EnableScrollBar( HWND hWnd, UINT wSBflags, UINT wArrows );
    static BOOL WINAPI ShowScrollBar( HWND hWnd, int wBar, BOOL bShow, LONG nMax );
    static BOOL WINAPI IsEnhancedMetafileDC( HDC hdc );
    static UINT WINAPI GetTextAlign(HDC);
    static UINT WINAPI SetTextAlign(HDC, UINT);
    static UINT WINAPI InvertRect(HDC hdc, CONST RECT *lprc);
    static BOOL WINAPI GetCursorPos(POINT *ppt);
    static HPALETTE WINAPI ManagePalette(
        HDC hdc,
        CONST LOGPALETTE *plogpal,
        HPALETTE &hpalOld,
        HPALETTE &hpalNew
    );
    static int WINAPI GetMapMode(HDC hdc);
    static BOOL WINAPI WinLPtoDP(HDC hdc, LPPOINT lppoints, int nCount);
    static BOOL WINAPI WinDPtoLP(HDC hdc, LPPOINT lppoints, int nCount);

    static long WINAPI WvsprintfA(LONG cb, LPSTR szBuf, LPCSTR szFmt, va_list arglist);

    static int WINAPI MulDiv(int nNumber, int nNumerator, int nDenominator);

    // Convert Himetric along the X axis to X pixels
    static inline LONG  HimetricXtoDX(LONG xHimetric, LONG xPerInch)
    {
        // This formula is rearranged to get rid of the need for floating point
        // arithmetic. The real way to understand the formula is to use
        // (xHimetric / HIMETRIC_PER_INCH) to get the inches and then multiply
        // the inches by the number of x pixels per inch to get the pixels.
        return (LONG) MulDiv(xHimetric, xPerInch, HIMETRIC_PER_INCH);
    }

    // Convert Himetric along the Y axis to Y pixels
    static inline LONG HimetricYtoDY(LONG yHimetric, LONG yPerInch)
    {
        // This formula is rearranged to get rid of the need for floating point
        // arithmetic. The real way to understand the formula is to use
        // (xHimetric / HIMETRIC_PER_INCH) to get the inches and then multiply
        // the inches by the number of y pixels per inch to get the pixels.
        return (LONG) MulDiv(yHimetric, yPerInch, HIMETRIC_PER_INCH);
    }

    // Convert Pixels on the X axis to Himetric
    static inline LONG DXtoHimetricX(LONG dx, LONG xPerInch)
    {
        // This formula is rearranged to get rid of the need for floating point
        // arithmetic. The real way to understand the formula is to use
        // (dx / x pixels per inch) to get the inches and then multiply
        // the inches by the number of himetric units per inch to get the
        // count of himetric units.
        return (LONG) MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
    }

    // Convert Pixels on the Y axis to Himetric
    static inline LONG DYtoHimetricY(LONG dy, LONG yPerInch)
    {
        // This formula is rearranged to get rid of the need for floating point
        // arithmetic. The real way to understand the formula is to use
        // (dy / y pixels per inch) to get the inches and then multiply
        // the inches by the number of himetric units per inch to get the
        // count of himetric units.
        return (LONG) MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
    }

    //
    // Case insensitive ASCII compare
    //
    static BOOL ASCIICompareI( const BYTE *pstr1, const BYTE *pstr2, int iCount )
    {
        int i;
        for (i = 0; i < iCount && !((pstr1[i] ^ pstr2[i]) & ~0x20); i++)
            ;
        return i == iCount;
    }

    //
    // Allocate and convert a MultiByte string to a wide character string
    // Allocated strings must be freed with delete
    //
    static WCHAR *ConvertToWideChar( const char *pstr )
    {
        int istrlen;
        for (istrlen = 0; pstr[istrlen]; istrlen++);
        WCHAR *pnew = new WCHAR[istrlen + 1];
        if (pnew )
        {
            if  (0 != ::MultiByteToWideChar( CP_ACP, 0, pstr, -1, pnew, istrlen + 1))
            {
                return pnew;
            }
            else
            {
                delete[] pnew;
            }
        }
        return NULL;
    }

    //
    // functions for memory management
    //
    static LPVOID PvAlloc(ULONG cbBuf,  UINT uiMemFlags);
    static LPVOID PvReAlloc(LPVOID pvBuf, DWORD cbBuf);
    static BOOL   FreePv(LPVOID pvBuf);

    static inline void *CopyMemory(void *dst, const void *src, size_t cb)
    {
        // Will work for overlapping regions
        return memmove(dst, src, cb);
    }

    static inline void *MoveMemory(void *dst, const void *src, size_t cb)
    {
        return memmove(dst, src, cb);
    }

    static inline void *FillMemory(void *dst, int fill, size_t cb)
    {
        return memset(dst, fill, cb);
    }

    static inline void *ZeroMemory(void *dst, size_t cb)
    {
        return memset(dst, 0, cb);
    }

    static inline int CompareMemory(const void *s1, const void *s2, size_t cb)
    {
        return memcmp(s1, s2, cb);
    }

    // ----------------------------------
    // IME Support
    // ----------------------------------
    static BOOL ImmInitialize( void );
    static void ImmTerminate( void );

    static LONG ImmGetCompositionStringA ( HIMC, DWORD, LPVOID, DWORD );
    static BOOL ImmSetCompositionFontA ( HIMC, LPLOGFONTA );

// GuyBark: Use the real system functions:
#ifndef PWD_JUPITER

    static HIMC ImmGetContext ( HWND );
    static BOOL ImmSetCompositionWindow ( HIMC, LPCOMPOSITIONFORM );
    static BOOL ImmReleaseContext ( HWND, HIMC );
    static DWORD ImmGetProperty ( HKL, DWORD );
    static BOOL ImmGetCandidateWindow ( HIMC, DWORD, LPCANDIDATEFORM );
    static BOOL ImmSetCandidateWindow ( HIMC, LPCANDIDATEFORM );
    static BOOL ImmNotifyIME ( HIMC, DWORD, DWORD, DWORD );
    static HIMC ImmAssociateContext ( HWND, HIMC );
    static UINT ImmGetVirtualKey ( HWND );
    static HIMC ImmEscape ( HKL, HIMC, UINT, LPVOID );
    static LONG ImmGetOpenStatus ( HIMC );
    static BOOL ImmGetConversionStatus ( HIMC, LPDWORD, LPDWORD );

#endif // !PWD_JUPITER

    static BOOL FSupportSty ( UINT, UINT );
    static const IMESTYLE * PIMEStyleFromAttr ( const UINT );
    static const IMECOLORSTY * PColorStyleTextFromIMEStyle ( const IMESTYLE * );
    static const IMECOLORSTY * PColorStyleBackFromIMEStyle ( const IMESTYLE * );
    static BOOL FBoldIMEStyle ( const IMESTYLE * );
    static BOOL FItalicIMEStyle ( const IMESTYLE * );
    static BOOL FUlIMEStyle ( const IMESTYLE * );
    static UINT IdUlIMEStyle ( const IMESTYLE * );
    static COLORREF RGBFromIMEColorStyle ( const IMECOLORSTY * );

    // ----------------------------------
    // National Language Keyboard support
    // ----------------------------------
    static void CheckChangeKeyboardLayout ( CTxtSelection *psel, BOOL fChangedFont );
    static void CheckChangeFont (
        CTxtSelection *psel,
        CTxtEdit * const ped,
        BOOL fEnableReassign,   // @parm Do we enable CTRL key?
        const WORD lcID,        // @parm LCID from WM_ message
        UINT cpg                // @parm code page to use (could be ANSI for far east with IME off)
    );
    static BOOL FormatMatchesKeyboard( const CCharFormat *pFormat );
//    static HKL GetKeyboardLayout ( DWORD );
//    static int GetKeyboardLayoutList ( int, HKL FAR * );

    // ----------------------------------
    // OLE Support
    // ----------------------------------
    static HRESULT LoadRegTypeLib ( REFGUID, WORD, WORD, LCID, ITypeLib ** );
    static HRESULT LoadTypeLib ( const OLECHAR *, ITypeLib ** );
    static BSTR SysAllocString ( const OLECHAR * );
    static BSTR SysAllocStringLen ( const OLECHAR *, UINT );
    static void SysFreeString ( BSTR );
    static UINT SysStringLen ( BSTR );
    static void VariantInit ( VARIANTARG * );
    static HRESULT OleCreateFromData ( LPDATAOBJECT, REFIID, DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, void ** );
    static void CoTaskMemFree ( LPVOID );
    static HRESULT CreateBindCtx ( DWORD, LPBC * );
    static HANDLE OleDuplicateData ( HANDLE, CLIPFORMAT, UINT );
    static HRESULT CoTreatAsClass ( REFCLSID, REFCLSID );
    static HRESULT ProgIDFromCLSID ( REFCLSID, LPOLESTR * );
    static HRESULT OleConvertIStorageToOLESTREAM ( LPSTORAGE, LPOLESTREAM );
    static HRESULT OleConvertIStorageToOLESTREAMEx ( LPSTORAGE, CLIPFORMAT, LONG, LONG, DWORD, LPSTGMEDIUM, LPOLESTREAM );
    static HRESULT OleSave ( LPPERSISTSTORAGE, LPSTORAGE, BOOL );
    static HRESULT StgCreateDocfileOnILockBytes ( ILockBytes *, DWORD, DWORD, IStorage ** );
    static HRESULT CreateILockBytesOnHGlobal ( HGLOBAL, BOOL, ILockBytes ** );
    static HRESULT OleCreateLinkToFile ( LPCOLESTR, REFIID, DWORD, LPFORMATETC, LPOLECLIENTSITE, LPSTORAGE, void ** );
    static LPVOID CoTaskMemAlloc ( ULONG );
    static LPVOID CoTaskMemRealloc ( LPVOID, ULONG );
    static HRESULT OleInitialize ( LPVOID );
    static void OleUninitialize ( );
    static HRESULT OleSetClipboard ( IDataObject * );
    static HRESULT OleFlushClipboard ( );
    static HRESULT OleIsCurrentClipboard ( IDataObject * );
    static HRESULT DoDragDrop ( IDataObject *, IDropSource *, DWORD, DWORD * );
    static HRESULT OleGetClipboard ( IDataObject ** );
    static HRESULT RegisterDragDrop ( HWND, IDropTarget * );
    static HRESULT OleCreateLinkFromData ( IDataObject *, REFIID, DWORD, LPFORMATETC, IOleClientSite *, IStorage *, void ** );
    static HRESULT OleCreateStaticFromData ( IDataObject *, REFIID, DWORD, LPFORMATETC, IOleClientSite *, IStorage *, void ** );
    static HRESULT OleDraw ( IUnknown *, DWORD, HDC, LPCRECT );
    static HRESULT OleSetContainedObject ( IUnknown *, BOOL );
    static HRESULT CoDisconnectObject ( IUnknown *, DWORD );
    static HRESULT WriteFmtUserTypeStg ( IStorage *, CLIPFORMAT, LPOLESTR );
    static HRESULT WriteClassStg ( IStorage *, REFCLSID );
    static HRESULT SetConvertStg ( IStorage *, BOOL );
    static HRESULT ReadFmtUserTypeStg ( IStorage *, CLIPFORMAT *, LPOLESTR * );
    static HRESULT ReadClassStg ( IStorage *pstg, CLSID * );
    static HRESULT OleRun ( IUnknown * );
    static HRESULT RevokeDragDrop ( HWND );
    static HRESULT CreateStreamOnHGlobal ( HGLOBAL, BOOL, IStream ** );
    static HRESULT GetHGlobalFromStream ( IStream *pstm, HGLOBAL * );
    static HRESULT OleCreateDefaultHandler ( REFCLSID, IUnknown *, REFIID, void ** );
    static HRESULT CLSIDFromProgID ( LPCOLESTR, LPCLSID );
    static HRESULT OleConvertOLESTREAMToIStorage ( LPOLESTREAM, IStorage *, const DVTARGETDEVICE * );
    static HRESULT OleLoad ( IStorage *, REFIID, IOleClientSite *, void ** );
    static HRESULT ReleaseStgMedium ( LPSTGMEDIUM );
    static void FreeOle();

    // ----------------------------------
    // Debugging Support
    //   Weak dbug32.dll replacement
    // ----------------------------------
    #ifdef DEBUG
    void AssertFn(BOOL, LPSTR, LPSTR, int);
    void sprintf (CHAR * buff, char *fmt, ...);
    void TraceOn ( void );
    void TraceOff (void );
    void TraceMsg(WCHAR *ptext);
    #endif

    // ----------------------------------
    // Useful ANSI<-->Unicode conversion
    //          and language id routines
    // ----------------------------------
    static LONG CharSetIndexFromChar(TCHAR ch);
    static BOOL CheckDBCInUnicodeStr(TCHAR *ptext);
    static int  MbcsFromUnicode(LPSTR pstr, int cch, LPCWSTR pwstr,
                    int cwch = -1, UINT codepage = CP_ACP,
                    UN_FLAGS flags = UN_CONVERT_WCH_EMBEDDING);
    static int  UnicodeFromMbcs(LPWSTR pwstr, int cwch, LPCSTR pstr, int cch = -1,
                    UINT uiCodePage = CP_ACP);
    static int  MBTWC(INT CodePage, DWORD dwFlags, LPCSTR pstrMB, int cchMB,
                    LPWSTR pstrWC, int cchWC, LPBOOL pfNoCodePage);
    static int  WCTMB(INT CodePage, DWORD dwFlags, LPCWSTR pstrWC, int cchWC,
                    LPSTR pstrMB, int cchMB, LPCSTR pchDefault, LPBOOL pfUsedDef,
                    LPBOOL pfNoCodePage);
    static HGLOBAL TextHGlobalAtoW( HGLOBAL hglobal, BOOL *pbDBC );
    static HGLOBAL TextHGlobalWtoA( HGLOBAL hglobal );
    static UINT ConvertLanguageIDtoCodePage(WORD lid);
    static BOOL IsFELCID(LCID lcid);
    static BOOL IsFECharset(BYTE bCharSet);
    static BOOL IsFEChar(WCHAR ch); // GuyBark: Added this.
    static INT  In125x(WCHAR ch, BYTE bCharSet);
    static BOOL IsLeadByte(BYTE ach, UINT cpg);
    static BOOL IsTrailByte(BYTE *aszBuff, LONG cb, UINT cpg);
    static BYTE GetCharSet(INT);
    static INT  GetCodePage(BYTE bCharSet);
    static UINT GetKeyboardCodePage();
    static LCID GetKeyboardLCID();
    static HKL GetKeyboardLayout( DWORD dwTId );
    static int GetKeyboardLayoutList ( int, HKL FAR * );
    static UINT GetLocaleCodePage();
    static LCID GetLocaleLCID();
    static BOOL IsCharSetValid(BYTE bCharSet);
    static BOOL ConvertCHARFORMATAtoW( CHARFORMATA *pcfmtA, CHARFORMATW *pcfmtW );
    static BOOL ConvertCHARFORMATWtoA( CHARFORMATW *pcfmtW, CHARFORMATA *pcfmtA );

    // ----------------------------------
    // Unicode Wrapped Functions
    // ----------------------------------

    // We could use inline and a function pointer table to improve efficiency and code size.

    static ATOM WINAPI RegisterREClass(
        const WNDCLASSW *lpWndClass,
        const char *szAnsiClassName,
        WNDPROC AnsiWndProc
    );
    static BOOL GetVersion(
        DWORD *pdwPlatformId,
        DWORD *pdwMajorVersion
    );
    static BOOL WINAPI GetStringTypeEx(
        LCID     Locale,
        DWORD    dwInfoType,
        LPCWSTR lpSrcStr,
        int      cchSrc,
        LPWORD   lpCharType
    );
    static LPWSTR WINAPI CharLower(LPWSTR pwstr);
    static DWORD WINAPI CharLowerBuff(LPWSTR pwstr, DWORD cchLength);
    static DWORD WINAPI CharUpperBuff(LPWSTR pwstr, DWORD cchLength);
    static HDC WINAPI CreateIC(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData
    );
    static HANDLE WINAPI CreateFile(
        LPCWSTR                 lpFileName,
        DWORD                   dwDesiredAccess,
        DWORD                   dwShareMode,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
        DWORD                   dwCreationDisposition,
        DWORD                   dwFlagsAndAttributes,
        HANDLE                  hTemplateFile
    );
    static HFONT WINAPI CreateFontIndirect(CONST LOGFONTW * plfw);
    static int WINAPI CompareString (
        LCID  Locale,           // locale identifier
        DWORD  dwCmpFlags,      // comparison-style options
        LPCWSTR  lpString1,     // pointer to first string
        int  cchCount1,         // size, in bytes or characters, of first string
        LPCWSTR  lpString2,     // pointer to second string
        int  cchCount2          // size, in bytes or characters, of second string
    );
    static LRESULT WINAPI DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static int WINAPI GetObject(HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj);
    static DWORD APIENTRY GetProfileSection(
        LPCWSTR lpAppName,
        LPWSTR lpReturnedString,
        DWORD nSize
    );
    static BOOL APIENTRY GetTextExtentPoint32(
        HDC     hdc,
        LPCWSTR pwsz,
        int     cb,
        LPSIZE  pSize
    );
    static int WINAPI GetTextFace(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName
    );
    static BOOL WINAPI GetTextMetrics(HDC hdc, LPTEXTMETRICW lptm);
    static LONG WINAPI GetWindowLong(HWND hWnd, int nIndex);
    static HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCWSTR lpBitmapName);
    static HCURSOR WINAPI LoadCursor(HINSTANCE hInstance, LPCWSTR lpCursorName);
    static HINSTANCE WINAPI LoadLibrary(LPCWSTR lpLibFileName);
    static LRESULT WINAPI SendMessage(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam
    );
    static LONG WINAPI SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
    static BOOL WINAPI PostMessage(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam
    );
    static BOOL WINAPI UnregisterClass(LPCWSTR lpClassName, HINSTANCE hInstance);
    static int WINAPI lstrcmp(LPCWSTR lpString1, LPCWSTR lpString2);
    static int WINAPI lstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2);
    static BOOL WINAPI PeekMessage(
        LPMSG   lpMsg,
        HWND    hWnd,
        UINT    wMsgFilterMin,
        UINT    wMsgFilterMax,
        UINT    wRemoveMsg
    );
    static DWORD WINAPI GetModuleFileName(
        HMODULE hModule,
        LPWSTR lpFilename,
        DWORD nSize
    );

    // Should also be wrapped but aren't.  Used for debugging.
    // MessageBox
    // OutputDebugString

    // lstrcmpiA should also be wrapped for Win CE's sake but the code
    // that uses it is ifdeffed out for WINCE.
};

extern CW32System *W32;

#if !defined(W32SYS_CPP)

#define PvAlloc             W32->PvAlloc
#define PvReAlloc           W32->PvReAlloc
#define FreePv              W32->FreePv
#define CopyMemory          W32->CopyMemory
#define MoveMemory          W32->MoveMemory
#define FillMemory          W32->FillMemory
#define ZeroMemory          W32->ZeroMemory
#define CompareMemory       W32->CompareMemory
#define GlobalAlloc         W32->GlobalAlloc
#define GlobalFree          W32->GlobalFree
#define GlobalFlags         W32->GlobalFlags
#define GlobalReAlloc       W32->GlobalReAlloc
#define GlobalSize          W32->GlobalSize
#define GlobalLock          W32->GlobalLock
#define GlobalHandle        W32->GlobalHandle
#define GlobalUnlock        W32->GlobalUnlock
#define GetTextAlign        W32->GetTextAlign
#define SetTextAlign        W32->SetTextAlign
#define InvertRect          W32->InvertRect

#define ImmInitialize W32->ImmInitialize
#define ImmTerminate W32->ImmTerminate

// GuyBark: Use the real system functions:
#ifndef PWD_JUPITER

#define pImmGetCompositionString W32->ImmGetCompositionStringA
#define pImmSetCompositionString W32->ImmSetCompositionStringA
#define pImmGetContext W32->ImmGetContext
#define pImmSetCompositionFont W32->ImmSetCompositionFontA
#define pImmSetCompositionWindow W32->ImmSetCompositionWindow
#define pImmReleaseContext W32->ImmReleaseContext
#define pImmGetProperty W32->ImmGetProperty
#define pImmGetCandidateWindow W32->ImmGetCandidateWindow
#define pImmSetCandidateWindow W32->ImmSetCandidateWindow
#define pImmNotifyIME W32->ImmNotifyIME
#define pImmAssociateContext W32->ImmAssociateContext
#define pImmGetVirtualKey W32->ImmGetVirtualKey
#define pImmEscape W32->ImmEscape
#define pImmGetOpenStatus W32->ImmGetOpenStatus
#define pImmGetConversionStatus W32->ImmGetConversionStatus

#else

// On the WinCE device, use the wide version of ImmGetCompositionString() and 
// ImmGetCompositionFont(). Retain the pImmGetCompositionStringA macro to 
// minmize the code changes.
#define pImmGetCompositionString ImmGetCompositionStringW
#define pImmSetCompositionFont ImmSetCompositionFontW
#define pImmGetContext ImmGetContext
#define pImmSetCompositionWindow ImmSetCompositionWindow
#define pImmReleaseContext ImmReleaseContext
#define pImmGetProperty ImmGetProperty
#define pImmGetCandidateWindow ImmGetCandidateWindow
#define pImmSetCandidateWindow ImmSetCandidateWindow
#define pImmNotifyIME ImmNotifyIME
#define pImmAssociateContext ImmAssociateContext
#define pImmGetVirtualKey ImmGetVirtualKey
#define pImmEscape ImmEscape
#define pImmGetOpenStatus ImmGetOpenStatus
#define pImmGetConversionStatus ImmGetConversionStatus

#endif // !PWD_JUPITER

//#define pGetKeyboardLayoutList W32->GetKeyboardLayoutList
#define pLoadRegTypeLib W32->LoadRegTypeLib
#define pLoadTypeLib W32->LoadTypeLib
#define pSysAllocString W32->SysAllocString
#define pSysAllocStringLen W32->SysAllocStringLen
#define pSysFreeString W32->SysFreeString
#define pSysStringLen W32->SysStringLen
#define pVariantInit W32->VariantInit
#define pOleCreateFromData W32->OleCreateFromData
#define pCoTaskMemFree W32->CoTaskMemFree
#define pCreateBindCtx W32->CreateBindCtx
#define pOleDuplicateData W32->OleDuplicateData
#define pCoTreatAsClass W32->CoTreatAsClass
#define pProgIDFromCLSID W32->ProgIDFromCLSID
#define pOleConvertIStorageToOLESTREAM W32->OleConvertIStorageToOLESTREAM
#define pOleConvertIStorageToOLESTREAMEx W32->OleConvertIStorageToOLESTREAMEx
#define pOleSave W32->OleSave
#define pStgCreateDocfileOnILockBytes W32->StgCreateDocfileOnILockBytes
#define pCreateILockBytesOnHGlobal W32->CreateILockBytesOnHGlobal
#define pOleCreateLinkToFile W32->OleCreateLinkToFile
#define pCoTaskMemAlloc W32->CoTaskMemAlloc
#define pCoTaskMemRealloc W32->CoTaskMemRealloc
#define pOleInitialize W32->OleInitialize
#define pOleUninitialize W32->OleUninitialize
#define pOleSetClipboard W32->OleSetClipboard
#define pOleFlushClipboard W32->OleFlushClipboard
#define pOleIsCurrentClipboard W32->OleIsCurrentClipboard
#define pDoDragDrop W32->DoDragDrop
#define pOleGetClipboard W32->OleGetClipboard
#define pRegisterDragDrop W32->RegisterDragDrop
#define pOleCreateLinkFromData W32->OleCreateLinkFromData
#define pOleCreateStaticFromData W32->OleCreateStaticFromData
#define pOleDraw W32->OleDraw
#define pOleSetContainedObject W32->OleSetContainedObject
#define pCoDisconnectObject W32->CoDisconnectObject
#define pWriteFmtUserTypeStg W32->WriteFmtUserTypeStg
#define pWriteClassStg W32->WriteClassStg
#define pSetConvertStg W32->SetConvertStg
#define pReadFmtUserTypeStg W32->ReadFmtUserTypeStg
#define pReadClassStg W32->ReadClassStg
#define pOleRun W32->OleRun
#define pRevokeDragDrop W32->RevokeDragDrop
#define pCreateStreamOnHGlobal W32->CreateStreamOnHGlobal
#define pGetHGlobalFromStream W32->GetHGlobalFromStream
#define pOleCreateDefaultHandler W32->OleCreateDefaultHandler
#define pCLSIDFromProgID W32->CLSIDFromProgID
#define pOleConvertOLESTREAMToIStorage W32->OleConvertOLESTREAMToIStorage
#define pOleLoad W32->OleLoad
#define pReleaseStgMedium W32->ReleaseStgMedium
#define pFSupportSty W32->FSupportSty
#define pPIMEStyleFromAttr W32->PIMEStyleFromAttr
#define pPColorStyleTextFromIMEStyle W32->PColorStyleTextFromIMEStyle
#define pPColorStyleBackFromIMEStyle W32->PColorStyleBackFromIMEStyle
#define pFBoldIMEStyle W32->FBoldIMEStyle
#define pFItalicIMEStyle W32->FItalicIMEStyle
#define pFUlIMEStyle W32->FUlIMEStyle
#define pIdUlIMEStyle W32->IdUlIMEStyle
#define pRGBFromIMEColorStyle W32->RGBFromIMEColorStyle

#define fHaveIMMProcs       W32->_fHaveIMMProcs
#define dwPlatformId        W32->_dwPlatformId
#define dwMajorVersion      W32->_dwMajorVersion
#define icr3DDarkShadow     W32->_icr3DDarkShadow

#define CharSetIndexFromChar        W32->CharSetIndexFromChar
#define CheckDBCInUnicodeStr        W32->CheckDBCInUnicodeStr
#define MbcsFromUnicode             W32->MbcsFromUnicode
#define UnicodeFromMbcs             W32->UnicodeFromMbcs
#define TextHGlobalAtoW             W32->TextHGlobalAtoW
#define TextHGlobalWtoA             W32->TextHGlobalWtoA
#define ConvertLanguageIDtoCodePage W32->ConvertLanguageIDtoCodePage
#define IsFELCID                    W32->IsFELCID
#define IsFECharset                 W32->IsFECharset
#define IsFEChar                    W32->IsFEChar
#define In125x                      W32->In125x
#define IsLeadByte                  W32->IsLeadByte
#define IsTrailByte                 W32->IsTrailByte
#define GetCharSet                  W32->GetCharSet
#define GetCodePage                 W32->GetCodePage
#define GetKeyboardCodePage         W32->GetKeyboardCodePage
#define GetKeyboardLCID             W32->GetKeyboardLCID
#define GetLocaleCodePage           W32->GetLocaleCodePage
#define GetLocaleLCID               W32->GetLocaleLCID
#define IsCharSetValid              W32->IsCharSetValid
#define MBTWC                       W32->MBTWC
#define WCTMB                       W32->WCTMB
#define ConvertCHARFORMATAtoW       W32->ConvertCHARFORMATAtoW
#define ConvertCHARFORMATWtoA       W32->ConvertCHARFORMATWtoA

#define CharLower                   W32->CharLower
#define CharLowerBuff               W32->CharLowerBuff
#define CharUpperBuff               W32->CharUpperBuff
#define CreateIC                    W32->CreateIC
#define CreateFile                  W32->CreateFile
#define CreateFontIndirect          W32->CreateFontIndirect
#define CompareString               W32->CompareString
#define DefWindowProc               W32->DefWindowProc
//#define GetKeyboardLayout           W32->GetKeyboardLayout
#define GetProfileSection           W32->GetProfileSection
#define GetTextExtentPoint32        W32->GetTextExtentPoint32
#define GetTextFace                 W32->GetTextFace
#define GetWindowLong               W32->GetWindowLong
#define LoadBitmap                  W32->LoadBitmap
#define LoadCursor                  W32->LoadCursor
#define LoadLibrary                 W32->LoadLibrary
#define SendMessage                 W32->SendMessage
#define SetWindowLong               W32->SetWindowLong
#define PostMessage                 W32->PostMessage
#define lstrcmp                     W32->lstrcmp
#define lstrcmpi                    W32->lstrcmpi
#define PeekMessage                 W32->PeekMessage
#define GetMapMode                  W32->GetMapMode
#define WinLPtoDP                   W32->WinLPtoDP
#define WinDPtoLP                   W32->WinDPtoLP
#define MulDiv                      W32->MulDiv
#define GetCursorPos                W32->GetCursorPos

#if defined(DEBUG)
#define sprintf                     W32->sprintf
#endif

#endif // !defined(W32SYS_CPP)

#if defined(DEBUG) && defined(PEGASUS)
// dbug32.dll has not been ported to Windows CE but we still want some assert support.
#undef Assert
#undef SideAssert
#undef AssertSz
#ifdef TRACEBEGIN
#undef TRACEBEGIN
#endif
#define AssertSz(f, sz)             W32->AssertFn(f ? TRUE : FALSE, sz, __FILE__, __LINE__)
#define Assert(f)                   W32->AssertFn(f ? TRUE : FALSE, NULL, __FILE__, __LINE__)
#define SideAssert(f)               W32->AssertFn(f ? TRUE : FALSE, NULL, __FILE__, __LINE__)
#define TRACEBEGIN(ss, sc, sz)      W32->TraceMsg(TEXT(sz))
#endif

//#pragma message("Review : Need clean up.  Should be System inline members.")
#define OnWinNTFE() 0
#define OnWin95FE() 0
#define VER_PLATFORM_WIN32_MACINTOSH    0x8001

#if defined PEGASUS && !defined(WINNT)

// The follwing definitions do not exist in the Windows CE environment but we emulate them.
// The values have been copied from the appropriate win32 header files.

#pragma message("Using NT definitions not in Windows CE")

// Memory allocation flag.  Win CE uses Local mem instead of Global mem
#undef GMEM_SHARE
#define GMEM_SHARE          0x2000
#define GMEM_ZEROINIT       LMEM_ZEROINIT
#define GMEM_MOVEABLE       LMEM_MOVEABLE
#define GMEM_FIXED          LMEM_FIXED

// Scroll Bars
#define ESB_ENABLE_BOTH                 0x0000
#define ESB_DISABLE_BOTH                0x0003

// Text alignment values
#define TA_TOP                       0
#define TA_BOTTOM                    8
#define TA_BASELINE                  24
#define TA_CENTER                    6
#define TA_LEFT                      0

// Device Technology.  This one is mostly used for exclusion
#define DT_METAFILE         5   /* Metafile, VDM                    */

// Resources for LoadCursor.
#define IDC_ARROW           MAKEINTRESOURCE(32512)
#define IDC_IBEAM           MAKEINTRESOURCE(32513)

// FInd/Replace options
#define FR_DOWN                         0x00000001
#define FR_WHOLEWORD                    0x00000002
#define FR_MATCHCASE                    0x00000004

// Window messages
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCMBUTTONDBLCLK              0x00A9
#define WM_DROPFILES                    0x0233

// Code Pages
#define CP_UTF8              65001          /* UTF-8 translation */

// Clipboard formats
#define CF_METAFILEPICT     3

// Special cursor shapes
#define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
#define IDC_SIZENESW        MAKEINTRESOURCE(32643)
#define IDC_SIZENS          MAKEINTRESOURCE(32645)
#define IDC_SIZEWE          MAKEINTRESOURCE(32644)

/* Mapping Modes */
#define MM_TEXT             1
#define MM_TWIPS            6
#define MM_ANISOTROPIC      8
#define SetMapMode(hdc, mapmode)
#define SetWindowOrgEx(hdc, xOrg, yOrg, pt)
#define SetViewportExtEx(hdc, nX, nY, lpSize)
#define SetWindowExtEx(hdc, x, y, lpSize)

/* Pen Styles : Windows CE only supports PS_DASH */
#define PS_DOT PS_DASH
#define PS_DASHDOT PS_DASH
#define PS_DASHDOTDOT PS_DASH

/* Missing APIs */
#define GetMessageTime()    0
#define IsIconic(hwnd)      0

#pragma message ("JMO Review : This is temporary to try to get the Pegasus Build untracked" )

#ifdef DEBUG
#define MoveToEx(a, b, c, d) 0
#else
#define MoveToEx(a, b, c, d)
#endif

#ifdef DEBUG
#define LineTo(a, b, c) 0
#else
#define LineTo(a, b, c)
#endif

#define GetProfileIntA(a, b, c) 0

class METARECORD
{
};

#define GetDesktopWindow() NULL

#define WS_EX_TRANSPARENT       0x00000020L
#define WM_MOUSEACTIVATE            0x0021

// GuyBark Jupiter 50100: This DOES exist on the device, and it works 
// just fine! To ifdef it zero here only serves to make RichEdit fail.
#ifndef PWD_JUPITER
#define IsDBCSLeadByte(x) 0
#endif // !PWD_JUPITER

#define WM_SYSCOLORCHANGE               0x0015
#define WM_STYLECHANGING                0x007C
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_SETCURSOR                    0x0020
#define WM_NCPAINT                      0x0085

#define OEM_CHARSET             255
#define SHIFTJIS_CHARSET        128
#define THAI_CHARSET            222
#define WM_IME_CHAR                     0x0286
#define IME_CMODE_NATIVE                0x0001
#define IME_CMODE_HANGEUL               IME_CMODE_NATIVE
#define IME_ESC_HANJA_MODE              0x1008

#define SM_SWAPBUTTON           23

class HDROP
{
};

#define TCI_SRCCODEPAGE 2

#define TPM_RIGHTBUTTON 0x0002L

#define RegisterClipboardFormatA(s)  RegisterClipboardFormatW(TEXT(s))
#define GetThreadLocale() 0

#define EASTEUROPE_CHARSET      238
#define HEBREW_CHARSET          177
#define RUSSIAN_CHARSET         204
#define GB2312_CHARSET          134
#define HANGEUL_CHARSET         129
#define JOHAB_CHARSET           130
#define CHINESEBIG5_CHARSET     136
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define BALTIC_CHARSET          186
#define ARABIC_CHARSET          178
#define MAC_CHARSET             77

#define ENUMLOGFONTA ENUMLOGFONT
#define ENUMLOGFONTW ENUMLOGFONT
#define FONTENUMPROCA FONTENUMPROC
typedef int *LPOPENFILENAMEA;
typedef int *LPOPENFILENAMEW;

#endif

#endif
