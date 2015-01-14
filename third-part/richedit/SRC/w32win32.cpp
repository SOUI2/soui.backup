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
 *  Unicode <--> MultiByte conversions, OLE, and other system functions
 *
 */

#define W32SYS_CPP

#include "_common.h"
#include "_host.h"
#include "_font.h"
#include <malloc.h>

class CConvertStr
{
public:
    operator char *();

protected:
    CConvertStr();
    ~CConvertStr();
    void Free();

    LPSTR   _pstr;
    char    _ach[MAX_PATH * 2];
};

inline CConvertStr::operator char *()
{
    return _pstr;
}

inline CConvertStr::CConvertStr()
{
    _pstr = NULL;
}

inline CConvertStr::~CConvertStr()
{
    Free();
}

class CStrIn : public CConvertStr
{
public:
    CStrIn(LPCWSTR pwstr);
    CStrIn(LPCWSTR pwstr, int cwch);
    int strlen();

protected:
    CStrIn();
    void Init(LPCWSTR pwstr, int cwch);

    int _cchLen;
};

inline CStrIn::CStrIn()
{
}

inline int CStrIn::strlen()
{
    return _cchLen;
}

class CStrOut : public CConvertStr
{
public:
    CStrOut(LPWSTR pwstr, int cwchBuf);
    ~CStrOut();

    int     BufSize();
    int     Convert();

private:
    LPWSTR  _pwstr;
    int     _cwchBuf;
};

inline int CStrOut::BufSize()
{
    return _cwchBuf * 2;
}

//
//  Multi-Byte ---> Unicode conversion
//

class CConvertStrW
{
public:
    operator WCHAR *();

protected:
    CConvertStrW();
    ~CConvertStrW();
    void Free();

    LPWSTR   _pwstr;
    WCHAR    _awch[MAX_PATH * 2];
};

inline CConvertStrW::CConvertStrW()
{
    _pwstr = NULL;
}

inline CConvertStrW::~CConvertStrW()
{
    Free();
}

inline CConvertStrW::operator WCHAR *()
{
    return _pwstr;
}

class CStrInW : public CConvertStrW
{
public:
    CStrInW(LPCSTR pstr);
    CStrInW(LPCSTR pstr, UINT uiCodePage);
    CStrInW(LPCSTR pstr, int cch, UINT uiCodePage);
    int strlen();

protected:
    CStrInW();
    void Init(LPCSTR pstr, int cch, UINT uiCodePage);

    int _cwchLen;
    UINT _uiCodePage;
};

inline CStrInW::CStrInW()
{
}

inline int CStrInW::strlen()
{
    return _cwchLen;
}

class CStrOutW : public CConvertStrW
{
public:
    CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage);
    ~CStrOutW();

    int     BufSize();
    int     Convert();

private:

    LPSTR   _pstr;
    int     _cchBuf;
    UINT    _uiCodePage;
};

inline int CStrOutW::BufSize()
{
    return _cchBuf;
}

ATOM WINAPI CW32System::RegisterREClass(
    const WNDCLASSW *lpWndClass,
    const char *szAnsiClassName,
    WNDPROC AnsiWndProc
)
{
    WNDCLASSA wc;

    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "RegisterREClass");
    // First register the normal window class.
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
    {
        if (!::RegisterClass(lpWndClass))
            return NULL;
    }
    else
    {
        // On WIndows 95 we need to convert the window class name.
        CStrIn strMenuName(lpWndClass->lpszMenuName);
        CStrIn strClassName(lpWndClass->lpszClassName);
        ASSERT(sizeof(wc) == sizeof(*lpWndClass));
        memcpy(&wc, lpWndClass, sizeof(wc));
        wc.lpszMenuName = strMenuName;
        wc.lpszClassName = strClassName;
        if (!::RegisterClassA(&wc))
            return NULL;
    }

    // Now REgister the ANSI window class name i.e. RICHEDIT20A
    wc.style = lpWndClass->style;
    wc.cbClsExtra = lpWndClass->cbClsExtra;
    wc.cbWndExtra = lpWndClass->cbWndExtra;
    wc.hInstance = lpWndClass->hInstance;
    wc.hIcon = lpWndClass->hIcon;
    wc.hCursor = lpWndClass->hIcon;
    wc.hbrBackground = lpWndClass->hbrBackground;
    wc.lpszMenuName = NULL;
    wc.lpfnWndProc = AnsiWndProc;
    wc.lpszClassName = szAnsiClassName;

    return ::RegisterClassA(&wc);
}


LRESULT CW32System::WndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    CTxtWinHost *ped = (CTxtWinHost *) GetWindowLong(hwnd, ibPed);

    #ifdef DEBUG
    Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
    #endif  // DEBUG

    switch(msg)
    {
    case WM_NCCREATE:
        return CTxtWinHost::OnNCCreate(hwnd, (CREATESTRUCT *) lparam);
        break;

    case WM_NCDESTROY:
        if( ped )
        {
            CTxtWinHost::OnNCDestroy(ped);
        }
        return 0;
    }

    return ped ? ped->TxWindowProc(hwnd, msg, wparam, lparam)
               : DefWindowProc(hwnd, msg, wparam, lparam);
}

LONG ValidateTextRange(TEXTRANGE *pstrg);

LRESULT CW32System::ANSIWndProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

    TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditANSIWndProc");

    #ifdef DEBUG
    Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
    #endif  // DEBUG

    CTxtWinHost *   ped = (CTxtWinHost *) GetWindowLong(hwnd, ibPed);
    LPARAM          lparamNew = 0;
    WPARAM          wparamNew = 0;
    CCharFormat     cf;
    DWORD           cpSelMin, cpSelMost;

    LRESULT         lres;

    switch( msg )
    {
    case WM_SETTEXT:
        {
            CStrInW strinw((char *)lparam, CP_ACP);

            return RichEditWndProc(hwnd, msg, wparam, (LPARAM)(WCHAR *)strinw);
        }
    case WM_CHAR:
        if( UnicodeFromMbcs((LPWSTR)&wparamNew,
                                 1,
                                 (char *)&wparam,
                                 1,
                                 GetKeyboardCodePage()) == 1 )
        {
            wparam = wparamNew;
            goto def;
        }
        break;

    case EM_SETCHARFORMAT:
        if( cf.SetA((CHARFORMATA *)lparam) )
        {
            lparam = (LPARAM)&cf;
            goto def;
        }
        break;

    case EM_GETCHARFORMAT:
        RichEditWndProc(hwnd, msg, wparam, (LPARAM)&cf);
        // Convert CCharFormat to CHARFORMAT(2)A
        if (cf.GetA((CHARFORMATA *)lparam))
            return ((CHARFORMATA *)lparam)->dwMask;
        return 0;

    case EM_FINDTEXT:
    case EM_FINDTEXTEX:
        {
            // we cheat a little here because FINDTEXT and FINDTEXTEX overlap
            // with the exception of the extra out param chrgText in FINDTEXTEX

            FINDTEXTEXW ftexw;
            FINDTEXTA *pfta = (FINDTEXTA *)lparam;
            CStrInW strinw(pfta->lpstrText, GetKeyboardCodePage());

            ftexw.chrg = pfta->chrg;
            ftexw.lpstrText = (WCHAR *)strinw;

            lres = WndProc(hwnd, msg, wparam, (LPARAM)&ftexw);

            if( msg == EM_FINDTEXTEX )
            {
                // in the FINDTEXTEX case, the extra field in the
                // FINDTEXTEX data structure is an out parameter indicating
                // the range where the text was found.  Update the 'real'
                // [in, out] parameter accordingly.
                ((FINDTEXTEXA *)lparam)->chrgText = ftexw.chrgText;
            }

            return lres;
        }
        break;

    case EM_GETSELTEXT:
        {
            // We aren't told how big the incoming buffer is; only that it's
            // "big enough".  Since we know we are grabbing the selection,
            // we'll assume that the buffer is the size of the selection's
            // Unicode data (plus 1 for NULL terminator) in bytes.
            WndProc(hwnd, EM_GETSEL, (WPARAM)&cpSelMin,
                (LPARAM)&cpSelMost);

            CStrOutW stroutw((LPSTR)lparam,
                             (cpSelMost - cpSelMin + 1)*sizeof(WCHAR),
                             GetKeyboardCodePage());
            return WndProc(hwnd, msg, wparam,
                        (LPARAM)(WCHAR *)stroutw);
        }
        break;

    case WM_GETTEXT:
        {
            // comvert WM_GETTEXT to ANSI using EM_GTETEXTEX
            GETTEXTEX gt;

            gt.cb = wparam;
            gt.flags = GT_USECRLF;
            gt.codepage = CP_ACP;
            gt.lpDefaultChar = NULL;
            gt.lpUsedDefChar = NULL;

            return WndProc(hwnd, EM_GETTEXTEX, (WPARAM)&gt, lparam);
        }
        break;

    case WM_GETTEXTLENGTH:
        {
            // convert WM_GETTEXTLENGTH to ANSI using EM_GETTEXTLENGTHEX
            GETTEXTLENGTHEX gtl;

            gtl.flags = GTL_NUMBYTES | GTL_PRECISE | GTL_USECRLF;
            gtl.codepage = CP_ACP;

            return WndProc(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
        }
        break;

    case EM_GETTEXTRANGE:
        {
            TEXTRANGEA *ptrg = (TEXTRANGEA *)lparam;

            LONG clInBuffer = ValidateTextRange((TEXTRANGEW *) ptrg);

            // If size is -1, this means that the size required is the total
            // size of the the text.
            if (-1 == clInBuffer)
            {
                // We can get this length either by digging the data out of the
                // various structures below us or we can take advantage of the
                // WM_GETTEXTLENGTH message. The first might be slightly
                // faster but the second definitely save code size. So we
                // will go with the second.
                clInBuffer = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
            }

            if (0 == clInBuffer)
            {
                // The buffer was invalid for some reason or there was not data
                // to copy. In any case, we are done.
                return 0;
            }

            // Verify that the output buffer is big enough.
            if (IsBadWritePtr(ptrg->lpstrText, clInBuffer + 1))
            {
                // Not enough space so don't copy any
                return 0;
            }

            // For EM_GETTEXTRANGE case, we again don't know how big the incoming buffer is, only that
            // it should be *at least* as great as cpMax - cpMin in the
            // text range structure.  We also know that anything *bigger*
            // than (cpMax - cpMin)*2 bytes is uncessary.  So we'll just assume
            // that's it's "big enough" and let WideCharToMultiByte scribble
            // as much as it needs.  Memory shortages are the caller's
            // responsibility (courtesy of the RichEdit1.0 design).
            CStrOutW stroutw( ptrg->lpstrText, (clInBuffer + 1) * sizeof(WCHAR),
                    CP_ACP );
            TEXTRANGEW trgw;
            trgw.chrg = ptrg->chrg;
            trgw.lpstrText = (WCHAR *)stroutw;

            if (WndProc(hwnd, EM_GETTEXTRANGE, wparam, (LPARAM)&trgw))
            {
                // need to return the number of BYTE converted.
                return stroutw.Convert();
            }
        }

    case EM_REPLACESEL:
        {
            CStrInW strinw((LPSTR)lparam, CP_ACP);
            return WndProc(hwnd, msg, wparam, (LPARAM)(WCHAR *)strinw);
        }

    case EM_GETLINE:
        {
            // the size is indicated by the first word of the memory pointed
            // to by lparam
            WORD size = *(WORD *)lparam;
            CStrOutW stroutw((char *)lparam, (DWORD)size, CP_ACP);
            WCHAR *pwsz = (WCHAR *)stroutw;
            *(WORD *)pwsz = size;

            return WndProc(hwnd, msg, wparam, (LPARAM)pwsz);
        }

    case WM_NCCREATE:
    case WM_CREATE:
        {
            // the only thing we need to convert are the strings,
            // so just do a structure copy and replace the
            // strings.
            CREATESTRUCTW csw = *(CREATESTRUCTW *)lparam;
            CREATESTRUCTA *pcsa = (CREATESTRUCTA *)lparam;
            CStrInW strinwName(pcsa->lpszName, GetKeyboardCodePage());
            CStrInW strinwClass(pcsa->lpszClass, CP_ACP);

            csw.lpszName = (WCHAR *)strinwName;
            csw.lpszClass = (WCHAR *)strinwClass;

            return WndProc(hwnd, msg, wparam, (LPARAM)&csw);
        }

    default:
def:    return WndProc(hwnd, msg, wparam, lparam);
    }
    return 0;       // Something went wrong.
}

HGLOBAL WINAPI CW32System::GlobalAlloc( UINT uFlags, DWORD dwBytes )
{
    return ::GlobalAlloc( uFlags, dwBytes );
}

HGLOBAL WINAPI CW32System::GlobalFree( HGLOBAL hMem )
{
    return hMem ? ::GlobalFree( hMem ) : NULL;
}

UINT WINAPI CW32System::GlobalFlags( HGLOBAL hMem )
{
    return ::GlobalFlags( hMem );
}

HGLOBAL WINAPI CW32System::GlobalReAlloc( HGLOBAL hMem, DWORD dwBytes, UINT uFlags )
{
    return ::GlobalReAlloc( hMem, dwBytes, uFlags );
}

DWORD WINAPI CW32System::GlobalSize( HGLOBAL hMem )
{
    return ::GlobalSize( hMem );
}

LPVOID WINAPI CW32System::GlobalLock( HGLOBAL hMem )
{
    return ::GlobalLock( hMem );
}

HGLOBAL WINAPI CW32System::GlobalHandle( LPCVOID pMem )
{
    return ::GlobalHandle( pMem );
}

BOOL WINAPI CW32System::GlobalUnlock( HGLOBAL hMem )
{
    return ::GlobalUnlock( hMem );
}

void CW32System::CheckChangeKeyboardLayout ( CTxtSelection *psel, BOOL fChangedFont )
{
    #pragma message ("Review : Incomplete")
    return;
}

void CW32System::CheckChangeFont (
    CTxtSelection *psel,
    CTxtEdit * const ped,
    BOOL fEnableReassign,   // @parm Do we enable CTRL key?
    const WORD lcID,        // @parm LCID from WM_ message
    UINT cpg                // @parm code page to use (could be ANSI for far east with IME off)
)
{
    #pragma message ("Review : Incomplete")
    return;
}

BOOL CW32System::FormatMatchesKeyboard( const CCharFormat *pFormat )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

HKL CW32System::GetKeyboardLayout( DWORD dwTId )
{
    return ::GetKeyboardLayout( dwTId );
}

int CW32System::GetKeyboardLayoutList ( int, HKL FAR * )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

enum DLL_ENUM{
    DLL_OLEAUT32,
    DLL_OLE32
};

static HINSTANCE hOleAut32 = NULL;
static HINSTANCE hOle32 = NULL;

static void SetProcAddr( void * &pfunc, DLL_ENUM which, char * fname )
{
    HINSTANCE hdll = NULL;
    EnterCriticalSection(&g_CriticalSection);
    if (pfunc == NULL)
    {
        switch (which) {
        case DLL_OLEAUT32 :
            if (hOleAut32 == NULL)
                hOleAut32 = W32->LoadLibrary(L"oleaut32.dll" );
            Assert( hOleAut32 != NULL );
            hdll = hOleAut32;
            break;
        case DLL_OLE32 :
            if (hOle32 == NULL)
                hOle32 = W32->LoadLibrary(L"ole32.dll" );
            Assert( hOle32 != NULL );
            hdll = hOle32;
            break;
        }
        Assert(hdll != NULL);
        pfunc = GetProcAddress( hdll, fname );
    }
    Assert(pfunc != NULL );
    LeaveCriticalSection(&g_CriticalSection);
}

void CW32System::FreeOle()
{
    if (hOleAut32 || hOle32) {
        EnterCriticalSection(&g_CriticalSection);
        if (hOleAut32 != NULL && FreeLibrary(hOleAut32)) {
            hOleAut32 = NULL;
        }
        if (hOle32 != NULL && FreeLibrary(hOle32)) {
            hOle32 = NULL;
        }
        LeaveCriticalSection(&g_CriticalSection);
    }
}

#define RE_OLEAUTAPI(name)      DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *name)
#define RE_OLEAUTAPI_(type, name)   DECLSPEC_IMPORT type (STDAPICALLTYPE *name)

typedef RE_OLEAUTAPI(LRTL_CAST)(REFGUID, WORD, WORD, LCID, ITypeLib **);
HRESULT CW32System::LoadRegTypeLib (
    REFGUID rguid,
    WORD wmajor,
    WORD wminor,
    LCID lcid,
    ITypeLib ** pptlib
)
{
    static void *pLoadRegTypeLib = NULL;
    if (pLoadRegTypeLib == NULL)
        SetProcAddr( pLoadRegTypeLib, DLL_OLEAUT32, "LoadRegTypeLib" );
    return ((LRTL_CAST)pLoadRegTypeLib)(rguid, wmajor, wminor, lcid, pptlib);
}

typedef RE_OLEAUTAPI(LTL_CAST)(const OLECHAR *, ITypeLib **);
HRESULT CW32System::LoadTypeLib ( const OLECHAR *szfile, ITypeLib **pptlib )
{
    static void *pLoadTypeLib = NULL;
    if (pLoadTypeLib == NULL)
        SetProcAddr( pLoadTypeLib, DLL_OLEAUT32, "LoadTypeLib" );
    return ((LTL_CAST)pLoadTypeLib)(szfile, pptlib);
}

typedef RE_OLEAUTAPI_(BSTR, SAS_CAST)(const OLECHAR *);
BSTR CW32System::SysAllocString ( const OLECHAR * sz )
{
    static void *pSysAllocString = NULL;
    if (pSysAllocString == NULL)
        SetProcAddr( pSysAllocString, DLL_OLEAUT32, "SysAllocString" );
    return ((SAS_CAST)pSysAllocString)(sz);
}

typedef RE_OLEAUTAPI_(BSTR, SASL_CAST)(const OLECHAR *, UINT);
BSTR CW32System::SysAllocStringLen ( const OLECHAR *pch, UINT cch )
{
    static void *pSysAllocStringLen = NULL;
    if (pSysAllocStringLen == NULL)
        SetProcAddr( pSysAllocStringLen, DLL_OLEAUT32, "SysAllocStringLen" );
    return ((SASL_CAST)pSysAllocStringLen)(pch, cch);
}

typedef RE_OLEAUTAPI_(void, SFS_CAST)(BSTR);
void CW32System::SysFreeString ( BSTR bstr )
{
    static void *pSysFreeString = NULL;
    if (pSysFreeString == NULL)
        SetProcAddr( pSysFreeString, DLL_OLEAUT32, "SysFreeString" );
    ((SFS_CAST)pSysFreeString)(bstr);
}

typedef RE_OLEAUTAPI_(UINT, SSL_CAST)(BSTR);
UINT CW32System::SysStringLen ( BSTR bstr )
{
    static void *pSysStringLen = NULL;
    if (pSysStringLen == NULL)
        SetProcAddr( pSysStringLen, DLL_OLEAUT32, "SysStringLen" );
    return ((SSL_CAST)pSysStringLen)(bstr);
}

typedef RE_OLEAUTAPI_(void, VI_CAST)(VARIANTARG *);
void CW32System::VariantInit ( VARIANTARG * pvarg )
{
    static void *pVariantInit = NULL;
    if (pVariantInit == NULL)
        SetProcAddr( pVariantInit, DLL_OLEAUT32, "VariantInit" );
    ((VI_CAST)pVariantInit)(pvarg);
}

#define RE_OLE32API(name)       DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *name)
#define RE_OLE32API_(type, name) DECLSPEC_IMPORT type (STDAPICALLTYPE *name)

typedef RE_OLE32API(OCFD_CAST)(LPDATAOBJECT, REFIID, DWORD,
                               LPFORMATETC, LPOLECLIENTSITE,
                               LPSTORAGE, void **);
HRESULT CW32System::OleCreateFromData (
    LPDATAOBJECT pDataObj,
    REFIID riid,
    DWORD renderopt,
    LPFORMATETC pfetc,
    LPOLECLIENTSITE pClientSite,
    LPSTORAGE pStg,
    void **ppvObj
)
{
    static void *pOleCreateFromData = NULL;
    if (pOleCreateFromData == NULL)
        SetProcAddr( pOleCreateFromData, DLL_OLE32, "OleCreateFromData" );
    return ((OCFD_CAST)pOleCreateFromData)(pDataObj, riid, renderopt, pfetc, pClientSite, pStg, ppvObj);
}

typedef RE_OLE32API_(void, CTMF_CAST)(LPVOID);
void CW32System::CoTaskMemFree ( LPVOID pv )
{
    static void *pCoTaskMemFree = NULL;
    if (pCoTaskMemFree == NULL)
        SetProcAddr( pCoTaskMemFree, DLL_OLE32, "CoTaskMemFree" );
    ((CTMF_CAST)pCoTaskMemFree)(pv);
}

typedef RE_OLE32API(CBC_CAST)(DWORD, LPBC *);
HRESULT CW32System::CreateBindCtx ( DWORD reserved, LPBC * ppbc )
{
    static void *pCreateBindCtx = NULL;
    if (pCreateBindCtx == NULL)
        SetProcAddr( pCreateBindCtx, DLL_OLE32, "CreateBindCtx" );
    return ((CBC_CAST)pCreateBindCtx)(reserved, ppbc);
}

typedef RE_OLE32API_(HANDLE, ODD_CAST)(HANDLE, CLIPFORMAT, UINT);
HANDLE CW32System::OleDuplicateData ( HANDLE hSrc, CLIPFORMAT cfFormat, UINT uFlags )
{
    static void *pOleDuplicateData = NULL;
    if (pOleDuplicateData == NULL)
        SetProcAddr( pOleDuplicateData, DLL_OLE32, "OleDuplicateData" );
    return ((ODD_CAST)pOleDuplicateData)(hSrc, cfFormat, uFlags);
}

typedef RE_OLE32API(CTAC_CAST)(REFCLSID, REFCLSID);
HRESULT CW32System::CoTreatAsClass ( REFCLSID clsidold, REFCLSID clsidnew )
{
    static void *pCoTreatAsClass = NULL;
    if (pCoTreatAsClass == NULL)
        SetProcAddr( pCoTreatAsClass, DLL_OLE32, "CoTreatAsClass" );
    return ((CTAC_CAST)pCoTreatAsClass)(clsidold, clsidnew);
}

typedef RE_OLE32API(PIFC_CAST)(REFCLSID, LPOLESTR *);
HRESULT CW32System::ProgIDFromCLSID ( REFCLSID clsid, LPOLESTR * lplpszProgId )
{
    static void *pProgIDFromCLSID = NULL;
    if (pProgIDFromCLSID == NULL)
        SetProcAddr( pProgIDFromCLSID, DLL_OLE32, "ProgIDFromCLSID" );
    return ((PIFC_CAST)pProgIDFromCLSID)(clsid, lplpszProgId);
}

typedef RE_OLE32API(OCITO_CAST)(LPSTORAGE, LPOLESTREAM);
HRESULT CW32System::OleConvertIStorageToOLESTREAM ( LPSTORAGE pstg, LPOLESTREAM lpolestream)
{
    static void *pOleConvertIStorageToOLESTREAM = NULL;
    if (pOleConvertIStorageToOLESTREAM == NULL)
        SetProcAddr( pOleConvertIStorageToOLESTREAM, DLL_OLE32, "OleConvertIStorageToOLESTREAM" );
    return ((OCITO_CAST)pOleConvertIStorageToOLESTREAM)(pstg, lpolestream);
}

typedef RE_OLE32API(OCITOX_CAST)(LPSTORAGE, CLIPFORMAT, LONG, LONG, DWORD, LPSTGMEDIUM, LPOLESTREAM);
HRESULT CW32System::OleConvertIStorageToOLESTREAMEx (
    LPSTORAGE pstg,
    CLIPFORMAT cf,
    LONG lwidth,
    LONG lheight,
    DWORD dwsize,
    LPSTGMEDIUM pmedium,
    LPOLESTREAM lpolestream
)
{
    static void *pOleConvertIStorageToOLESTREAMEx = NULL;
    if (pOleConvertIStorageToOLESTREAMEx == NULL)
        SetProcAddr( pOleConvertIStorageToOLESTREAMEx, DLL_OLE32, "OleConvertIStorageToOLESTREAMEx" );
    return ((OCITOX_CAST)pOleConvertIStorageToOLESTREAMEx)
        (pstg,cf, lwidth, lheight, dwsize, pmedium, lpolestream);
}

typedef RE_OLE32API(OS_CAST)(LPPERSISTSTORAGE, LPSTORAGE, BOOL);
HRESULT CW32System::OleSave ( LPPERSISTSTORAGE pPS, LPSTORAGE pstg, BOOL fSameAsLoad )
{
    static void *pOleSave = NULL;
    if (pOleSave == NULL)
        SetProcAddr( pOleSave, DLL_OLE32, "OleSave" );
    return ((OS_CAST)pOleSave)(pPS, pstg, fSameAsLoad);
}

typedef RE_OLE32API(SCDOI_CAST)(ILockBytes *, DWORD, DWORD, IStorage **);
HRESULT CW32System::StgCreateDocfileOnILockBytes (
    ILockBytes *plkbyt,
    DWORD grfmode,
    DWORD res,
    IStorage **ppstg
)
{
    static void *pStgCreateDocfileOnILockBytes = NULL;
    if (pStgCreateDocfileOnILockBytes == NULL)
        SetProcAddr( pStgCreateDocfileOnILockBytes, DLL_OLE32, "StgCreateDocfileOnILockBytes" );
    return ((SCDOI_CAST)pStgCreateDocfileOnILockBytes)(plkbyt, grfmode, res, ppstg);
}

typedef RE_OLE32API(CIOH_CAST)(HGLOBAL, BOOL, ILockBytes **);
HRESULT CW32System::CreateILockBytesOnHGlobal ( HGLOBAL hGlobal, BOOL fDel, ILockBytes **pplkbyt )
{
    static void *pCreateILockBytesOnHGlobal = NULL;
    if (pCreateILockBytesOnHGlobal == NULL)
        SetProcAddr( pCreateILockBytesOnHGlobal, DLL_OLE32, "CreateILockBytesOnHGlobal" );
    return ((CIOH_CAST)pCreateILockBytesOnHGlobal)(hGlobal, fDel, pplkbyt);
}

typedef RE_OLE32API(OCLTF_CAST)(LPCOLESTR, REFIID, DWORD, LPFORMATETC,
                                LPOLECLIENTSITE, LPSTORAGE, void **);
HRESULT CW32System::OleCreateLinkToFile(
    LPCOLESTR pstr,
    REFIID rid,
    DWORD renderopt,
    LPFORMATETC pfetc,
    LPOLECLIENTSITE psite,
    LPSTORAGE pstg,
    void **ppstg
)
{
    static void *pOleCreateLinkToFile = NULL;
    if (pOleCreateLinkToFile == NULL)
        SetProcAddr( pOleCreateLinkToFile, DLL_OLE32, "OleCreateLinkToFile" );
    return ((OCLTF_CAST)pOleCreateLinkToFile)(pstr, rid, renderopt, pfetc, psite, pstg, ppstg);
}

typedef RE_OLE32API_(LPVOID, CTMA_CAST)(ULONG);
LPVOID CW32System::CoTaskMemAlloc ( ULONG cb )
{
    static void *pCoTaskMemAlloc = NULL;
    if (pCoTaskMemAlloc == NULL)
        SetProcAddr( pCoTaskMemAlloc, DLL_OLE32, "CoTaskMemAlloc" );
    return ((CTMA_CAST)pCoTaskMemAlloc)(cb);
}

typedef RE_OLE32API_(LPVOID, CTMR_CAST)(LPVOID, ULONG);
LPVOID CW32System::CoTaskMemRealloc ( LPVOID pv, ULONG cv)
{
    static void *pCoTaskMemRealloc = NULL;
    if (pCoTaskMemRealloc == NULL)
        SetProcAddr( pCoTaskMemRealloc, DLL_OLE32, "CoTaskMemRealloc" );
    return ((CTMR_CAST)pCoTaskMemRealloc)(pv, cv);
}

typedef RE_OLE32API(OI_CAST)(LPVOID);
HRESULT CW32System::OleInitialize ( LPVOID pvres )
{
    static void *pOleInitialize = NULL;
    if (pOleInitialize == NULL)
        SetProcAddr( pOleInitialize, DLL_OLE32, "OleInitialize" );
    return ((OI_CAST)pOleInitialize)(pvres);
}

typedef RE_OLE32API_(void, OUI_CAST)( void );
void CW32System::OleUninitialize ( void )
{
    static void *pOleUninitialize = NULL;
    if (pOleUninitialize == NULL)
        SetProcAddr( pOleUninitialize, DLL_OLE32, "OleUninitialize" );
    ((OUI_CAST)pOleUninitialize)();
}

typedef RE_OLE32API(OSC_CAST)(IDataObject *);
HRESULT CW32System::OleSetClipboard ( IDataObject *pdo )
{
    static void *pOleSetClipboard = NULL;
    if (pOleSetClipboard == NULL)
        SetProcAddr( pOleSetClipboard, DLL_OLE32, "OleSetClipboard" );
    return ((OSC_CAST)pOleSetClipboard)(pdo);
}

typedef RE_OLE32API(OFC_CAST)(void);
HRESULT CW32System::OleFlushClipboard ( void )
{
    static void *pOleFlushClipboard = NULL;
    if (pOleFlushClipboard == NULL)
        SetProcAddr( pOleFlushClipboard, DLL_OLE32, "OleFlushClipboard" );
    return ((OFC_CAST)pOleFlushClipboard)();
}

typedef RE_OLE32API(OICC_CAST)(IDataObject *);
HRESULT CW32System::OleIsCurrentClipboard ( IDataObject *pdo )
{
    static void *pOleIsCurrentClipboard = NULL;
    if (pOleIsCurrentClipboard == NULL)
        SetProcAddr( pOleIsCurrentClipboard, DLL_OLE32, "OleIsCurrentClipboard" );
    return ((OICC_CAST)pOleIsCurrentClipboard)(pdo);
}

typedef RE_OLE32API(DDD_CAST)(IDataObject *, IDropSource *,
            DWORD, DWORD *);
HRESULT CW32System::DoDragDrop ( IDataObject *pdo, IDropSource *pds, DWORD dweffect, DWORD *pdweffect )
{
    static void *pDoDragDrop = NULL;
    if (pDoDragDrop == NULL)
        SetProcAddr( pDoDragDrop, DLL_OLE32, "DoDragDrop" );
    return ((DDD_CAST)pDoDragDrop)(pdo, pds, dweffect, pdweffect);
}

typedef RE_OLE32API(OGC_CAST)(IDataObject **);
HRESULT CW32System::OleGetClipboard ( IDataObject **ppdo )
{
    static void *pOleGetClipboard = NULL;
    if (pOleGetClipboard == NULL)
        SetProcAddr( pOleGetClipboard, DLL_OLE32, "OleGetClipboard" );
    return ((OGC_CAST)pOleGetClipboard)(ppdo);
}

typedef RE_OLE32API(RDD_CAST)(HWND, IDropTarget *);
HRESULT CW32System::RegisterDragDrop ( HWND hwnd, IDropTarget *pdt )
{
    static void *pRegisterDragDrop = NULL;
    if (pRegisterDragDrop == NULL)
        SetProcAddr( pRegisterDragDrop, DLL_OLE32, "RegisterDragDrop" );
    return ((RDD_CAST)pRegisterDragDrop)(hwnd, pdt);
}

typedef RE_OLE32API(OCLFD_CAST)(IDataObject *, REFIID, DWORD,
                                LPFORMATETC, IOleClientSite *,
                                IStorage *, void **);
HRESULT CW32System::OleCreateLinkFromData (
    IDataObject *pdo,
    REFIID rid,
    DWORD renderopt,
    LPFORMATETC pfetc,
    IOleClientSite *psite,
    IStorage *pstg,
    void **ppv
)
{
    static void *pOleCreateLinkFromData = NULL;
    if (pOleCreateLinkFromData == NULL)
        SetProcAddr( pOleCreateLinkFromData, DLL_OLE32, "OleCreateLinkFromData" );
    return ((OCLFD_CAST)pOleCreateLinkFromData)
        (pdo, rid, renderopt, pfetc, psite, pstg, ppv);
}

typedef RE_OLE32API(OCSFD_CAST)(IDataObject *, REFIID, DWORD,
                                LPFORMATETC, IOleClientSite *,
                                IStorage *, void **);
HRESULT CW32System::OleCreateStaticFromData (
    IDataObject *pdo,
    REFIID rid,
    DWORD renderopt,
    LPFORMATETC pfetc,
    IOleClientSite *psite,
    IStorage *pstg,
    void **ppv
)
{
    static void *pOleCreateStaticFromData = NULL;
    if (pOleCreateStaticFromData == NULL)
        SetProcAddr( pOleCreateStaticFromData, DLL_OLE32, "OleCreateStaticFromData" );
    return ((OCSFD_CAST)pOleCreateStaticFromData)
        (pdo, rid, renderopt, pfetc, psite, pstg, ppv);
}

typedef RE_OLE32API(OD_CAST)(IUnknown *, DWORD, HDC, LPCRECT);
HRESULT CW32System::OleDraw ( IUnknown *punk, DWORD dwAspect, HDC hdc, LPCRECT prect )
{
    static void *pOleDraw = NULL;
    if (pOleDraw == NULL)
        SetProcAddr( pOleDraw, DLL_OLE32, "OleDraw" );
    return ((OD_CAST)pOleDraw)(punk, dwAspect, hdc, prect);
}

typedef RE_OLE32API(OSCO_CAST)(IUnknown *, BOOL);
HRESULT CW32System::OleSetContainedObject ( IUnknown *punk, BOOL fContained )
{
    static void *pOleSetContainedObject = NULL;
    if (pOleSetContainedObject == NULL)
        SetProcAddr( pOleSetContainedObject, DLL_OLE32, "OleSetContainedObject" );
    return ((OSCO_CAST)pOleSetContainedObject)(punk, fContained);
}

typedef RE_OLE32API(CDO_CAST)(IUnknown *, DWORD);
HRESULT CW32System::CoDisconnectObject ( IUnknown *punk, DWORD dwres )
{
    static void *pCoDisconnectObject = NULL;
    if (pCoDisconnectObject == NULL)
        SetProcAddr( pCoDisconnectObject, DLL_OLE32, "CoDisconnectObject" );
    return ((CDO_CAST)pCoDisconnectObject)(punk, dwres);
}

typedef RE_OLE32API(WFUTS_CAST)(IStorage *, CLIPFORMAT, LPOLESTR);
HRESULT CW32System::WriteFmtUserTypeStg ( IStorage *pstg, CLIPFORMAT cf, LPOLESTR pstr)
{
    static void *pWriteFmtUserTypeStg = NULL;
    if (pWriteFmtUserTypeStg == NULL)
        SetProcAddr( pWriteFmtUserTypeStg, DLL_OLE32, "WriteFmtUserTypeStg" );
    return ((WFUTS_CAST)pWriteFmtUserTypeStg)(pstg, cf, pstr);
}

typedef RE_OLE32API(WCS_CAST)(IStorage *, REFCLSID);
HRESULT CW32System::WriteClassStg ( IStorage *pstg, REFCLSID rid )
{
    static void *pWriteClassStg = NULL;
    if (pWriteClassStg == NULL)
        SetProcAddr( pWriteClassStg, DLL_OLE32, "WriteClassStg" );
    return ((WCS_CAST)pWriteClassStg)(pstg, rid);
}

typedef RE_OLE32API(SCS_CAST)(IStorage *, BOOL);
HRESULT CW32System::SetConvertStg ( IStorage *pstg, BOOL fConv )
{
    static void *pSetConvertStg = NULL;
    if (pSetConvertStg == NULL)
        SetProcAddr( pSetConvertStg, DLL_OLE32, "SetConvertStg" );
    return ((SCS_CAST)pSetConvertStg)(pstg, fConv);
}

typedef RE_OLE32API(RFUTS_CAST)(IStorage *, CLIPFORMAT *, LPOLESTR *);
HRESULT CW32System::ReadFmtUserTypeStg ( IStorage *pstg, CLIPFORMAT *pcf, LPOLESTR *pstr )
{
    static void *pReadFmtUserTypeStg = NULL;
    if (pReadFmtUserTypeStg == NULL)
        SetProcAddr( pReadFmtUserTypeStg, DLL_OLE32, "ReadFmtUserTypeStg" );
    return ((RFUTS_CAST)pReadFmtUserTypeStg)(pstg, pcf, pstr);
}

typedef RE_OLE32API(RCS_CAST)(IStorage *, CLSID *);
HRESULT CW32System::ReadClassStg ( IStorage *pstg, CLSID *pclsid )
{
    static void *pReadClassStg = NULL;
    if (pReadClassStg == NULL)
        SetProcAddr( pReadClassStg, DLL_OLE32, "ReadClassStg" );
    return ((RCS_CAST)pReadClassStg)(pstg, pclsid);
}

typedef RE_OLE32API(OR_CAST)(IUnknown *);
HRESULT CW32System::OleRun ( IUnknown *punk )
{
    static void *pOleRun = NULL;
    if (pOleRun == NULL)
        SetProcAddr( pOleRun, DLL_OLE32, "OleRun" );
    return ((OR_CAST)pOleRun)(punk);
}

typedef RE_OLE32API(RevDD_CAST)(HWND);
HRESULT CW32System::RevokeDragDrop ( HWND hwnd )
{
    static void *pRevokeDragDrop = NULL;
    if (pRevokeDragDrop == NULL)
        SetProcAddr( pRevokeDragDrop, DLL_OLE32, "RevokeDragDrop" );
    return ((RevDD_CAST)pRevokeDragDrop)(hwnd);
}

typedef RE_OLE32API(CSOH_CAST)(HGLOBAL, BOOL, IStream **);
HRESULT CW32System::CreateStreamOnHGlobal ( HGLOBAL hglobal, BOOL fDel, IStream **ppstrm )
{
    static void *pCreateStreamOnHGlobal = NULL;
    if (pCreateStreamOnHGlobal == NULL)
        SetProcAddr( pCreateStreamOnHGlobal, DLL_OLE32, "CreateStreamOnHGlobal" );
    return ((CSOH_CAST)pCreateStreamOnHGlobal)(hglobal, fDel, ppstrm);
}

typedef RE_OLE32API(GHFS_CAST)(IStream *, HGLOBAL *);
HRESULT CW32System::GetHGlobalFromStream ( IStream *pstrm, HGLOBAL *phglobal )
{
    static void *pGetHGlobalFromStream = NULL;
    if (pGetHGlobalFromStream == NULL)
        SetProcAddr( pGetHGlobalFromStream, DLL_OLE32, "GetHGlobalFromStream" );
    return ((GHFS_CAST)pGetHGlobalFromStream)(pstrm, phglobal);
}

typedef RE_OLE32API(OCDH_CAST)(REFCLSID, IUnknown *, REFIID, void **);
HRESULT CW32System::OleCreateDefaultHandler (
    REFCLSID clsid,
    IUnknown *punk,
    REFIID riid,
    void **ppv
)
{
    static void *pOleCreateDefaultHandler = NULL;
    if (pOleCreateDefaultHandler == NULL)
        SetProcAddr( pOleCreateDefaultHandler, DLL_OLE32, "OleCreateDefaultHandler" );
    return ((OCDH_CAST)pOleCreateDefaultHandler)(clsid, punk, riid, ppv);
}

typedef RE_OLE32API(CFPI_CAST)(LPCOLESTR, LPCLSID);
HRESULT CW32System::CLSIDFromProgID ( LPCOLESTR pstr, LPCLSID pclsid )
{
    static void *pCLSIDFromProgID = NULL;
    if (pCLSIDFromProgID == NULL)
        SetProcAddr( pCLSIDFromProgID, DLL_OLE32, "CLSIDFromProgID" );
    return ((CFPI_CAST)pCLSIDFromProgID)(pstr, pclsid);
}

typedef RE_OLE32API(OCOTI_CAST)(LPOLESTREAM, IStorage *,
                                const DVTARGETDEVICE *);
HRESULT CW32System::OleConvertOLESTREAMToIStorage (
    LPOLESTREAM pstrm,
    IStorage *pstg,
    const DVTARGETDEVICE *ptd
)
{
    static void *pOleConvertOLESTREAMToIStorage = NULL;
    if (pOleConvertOLESTREAMToIStorage == NULL)
        SetProcAddr( pOleConvertOLESTREAMToIStorage, DLL_OLE32, "OleConvertOLESTREAMToIStorage" );
    return ((OCOTI_CAST)pOleConvertOLESTREAMToIStorage)(pstrm, pstg, ptd);
}

typedef RE_OLE32API(OL_CAST)(IStorage *, REFIID, IOleClientSite *, void **);
HRESULT CW32System::OleLoad (
    IStorage *pstg,
    REFIID riid,
    IOleClientSite *psite,
    void **ppv
)
{
    static void *pOleLoad = NULL;
    if (pOleLoad == NULL)
        SetProcAddr( pOleLoad, DLL_OLE32, "OleLoad" );
    return ((OL_CAST)pOleLoad)(pstg, riid, psite, ppv);
}

typedef RE_OLE32API(RSM_CAST)(LPSTGMEDIUM);
HRESULT CW32System::ReleaseStgMedium ( LPSTGMEDIUM pmedium )
{
    static void *pReleaseStgMedium = NULL;
    if (pReleaseStgMedium == NULL)
        SetProcAddr( pReleaseStgMedium, DLL_OLE32, "ReleaseStgMedium" );
    return ((RSM_CAST)pReleaseStgMedium)(pmedium);
}

BOOL CW32System::ImmInitialize( void )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

void CW32System::ImmTerminate( void )
{
    #pragma message ("Review : Incomplete")
    return;
}

// GuyBark: Use the system calls on Windows CE. So emulate that here for NT.
#ifndef PWD_JUPITER

LONG CW32System::ImmGetCompositionStringA ( HIMC, DWORD, LPVOID, DWORD )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

HIMC CW32System::ImmGetContext ( HWND )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

BOOL CW32System::ImmSetCompositionFontA ( HIMC, LPLOGFONTA )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

BOOL CW32System::ImmSetCompositionWindow ( HIMC, LPCOMPOSITIONFORM )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

BOOL CW32System::ImmReleaseContext ( HWND, HIMC )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

DWORD CW32System::ImmGetProperty ( HKL, DWORD )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

BOOL CW32System::ImmGetCandidateWindow ( HIMC, DWORD, LPCANDIDATEFORM )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

BOOL CW32System::ImmSetCandidateWindow ( HIMC, LPCANDIDATEFORM )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

BOOL CW32System::ImmNotifyIME ( HIMC, DWORD, DWORD, DWORD )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

HIMC CW32System::ImmAssociateContext ( HWND, HIMC )
{
    #pragma message ("Review : Incomplete")
    return NULL;
}

UINT CW32System::ImmGetVirtualKey ( HWND )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

HIMC CW32System::ImmEscape ( HKL, HIMC, UINT, LPVOID )
{
    #pragma message ("Review : Incomplete")
    return NULL;
}

LONG CW32System::ImmGetOpenStatus ( HIMC )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

BOOL CW32System::ImmGetConversionStatus ( HIMC, LPDWORD, LPDWORD )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

#endif // !PWD_JUPITER

BOOL CW32System::FSupportSty ( UINT, UINT )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

const IMESTYLE * CW32System::PIMEStyleFromAttr ( const UINT )
{
    #pragma message ("Review : Incomplete")
    return NULL;
}

const IMECOLORSTY * CW32System::PColorStyleTextFromIMEStyle ( const IMESTYLE * )
{
    #pragma message ("Review : Incomplete")
    return NULL;
}

const IMECOLORSTY * CW32System::PColorStyleBackFromIMEStyle ( const IMESTYLE * )
{
    #pragma message ("Review : Incomplete")
    return NULL;
}

BOOL CW32System::FBoldIMEStyle ( const IMESTYLE * )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

BOOL CW32System::FItalicIMEStyle ( const IMESTYLE * )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

BOOL CW32System::FUlIMEStyle ( const IMESTYLE * )
{
    #pragma message ("Review : Incomplete")
    return FALSE;
}

UINT CW32System::IdUlIMEStyle ( const IMESTYLE * )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

COLORREF CW32System::RGBFromIMEColorStyle ( const IMECOLORSTY * )
{
    #pragma message ("Review : Incomplete")
    return 0;
}

CONVERTMODE WINAPI CW32System::DetermineConvertMode( BYTE tmCharSet )
{
    #pragma message ("Review : Incomplete")
    return CM_NULL;
}

void WINAPI CW32System::CalcUnderlineInfo( CCcs *pcccs, TEXTMETRIC *ptm )
{
    OUTLINETEXTMETRICA *potm;
    unsigned cb;
    CTempBuf tb;

    if (ptm->tmPitchAndFamily & TMPF_TRUETYPE)
    {
        cb = GetOutlineTextMetricsA(pcccs->_hdc, 0, NULL);

        if ((cb != 0)
            && ((potm = (OUTLINETEXTMETRICA *) tb.GetBuf(cb)) != NULL)
            && GetOutlineTextMetricsA(pcccs->_hdc, cb, potm))
        {
            pcccs->_dyULOffset = -potm->otmsUnderscorePosition;
            pcccs->_dyULWidth = max(1, potm->otmsUnderscoreSize);
            pcccs->_dySOOffset = -potm->otmsStrikeoutPosition;
            pcccs->_dySOWidth = max(1, potm->otmsStrikeoutSize);
            return;
        }
    }

    // Default calculation of size of underline
    SHORT dyDescent = pcccs->_yDescent;

    if (0 == dyDescent)
    {
        dyDescent = pcccs->_yHeight >> 3;
    }

    pcccs->_dyULWidth = max(1, dyDescent / 4);
    pcccs->_dyULOffset = (dyDescent - 3 * pcccs->_dyULWidth + 1) / 2;

    if ((0 == pcccs->_dyULOffset) && (dyDescent > 1))
    {
        pcccs->_dyULOffset = 1;
    }

    pcccs->_dySOOffset = -ptm->tmAscent / 3;
    pcccs->_dySOWidth = pcccs->_dyULWidth;

    return;
}

BOOL WINAPI CW32System::ShowScrollBar( HWND hWnd, int wBar, BOOL bShow, LONG )
{
    return ::ShowScrollBar( hWnd, wBar, bShow );
}

BOOL WINAPI CW32System::EnableScrollBar( HWND hWnd, UINT wSBflags, UINT wArrows )
{
    return ::EnableScrollBar( hWnd, wSBflags, wArrows );
}

BOOL WINAPI CW32System::REExtTextOut(
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
    BOOL  FEFontOnNonFEWin95
)
{
    #pragma message ("Review : Incomplete")
    return ExtTextOut(hdc, x, y, fuOptions, lprc, lpString, cbCount, lpDx);
}

BOOL WINAPI CW32System::REGetCharWidth(
    HDC hdc,
    UINT iChar,
    LPINT pAns,
    UINT        // For Windows CE the code page is not used
                //  as the A version is not called.
)
{
    #pragma message ("Review : Incomplete")

   TCHAR buff[2];
   SIZE  sz;

    buff[0] = iChar;
    buff[1] = 0;

    if (GetTextExtentPoint32(hdc, buff, 1, &sz))
    {
        *pAns = sz.cx;

        Assert(*pAns > 0);
        return TRUE;
    }

    return FALSE;
}

BOOL WINAPI CW32System::IsEnhancedMetafileDC( HDC hDC )
{
    BOOL    fEMFDC = FALSE;
    DWORD   dwObjectType;

    dwObjectType = ::GetObjectType( hDC );

    if ( OBJ_ENHMETADC == dwObjectType || OBJ_ENHMETAFILE == dwObjectType )
        fEMFDC = TRUE;
    else if ( OBJ_DC == dwObjectType )
    {
        // HACK Alert,  Enhanced Metafile DC does not support any Escape function
        // and shoudl return 0.
        int iEscapeFuction = QUERYESCSUPPORT;

        if ( Escape( hDC, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&iEscapeFuction, NULL) == 0 )
            fEMFDC = TRUE;
    }

    return fEMFDC;
}

UINT WINAPI CW32System::GetTextAlign(HDC hdc)
{
    return ::GetTextAlign(hdc);
}

UINT WINAPI CW32System::SetTextAlign(HDC hdc, UINT uAlign)
{
    return ::SetTextAlign(hdc, uAlign);
}

UINT WINAPI CW32System::InvertRect(HDC hdc, CONST RECT *lprc)
{
    return ::InvertRect(hdc, lprc);
}

HPALETTE WINAPI CW32System::ManagePalette(
    HDC hdc,
    CONST LOGPALETTE *plogpal,
    HPALETTE &hpalOld,
    HPALETTE &hpalNew
)
{
    if (hpalNew == NULL)
    {
        hpalNew = ::CreatePalette(plogpal);
        if (hpalNew != NULL)
        {
            hpalOld = ::SelectPalette(hdc, hpalNew, TRUE);
            ::RealizePalette(hdc);
        }
    }
    else
    {
        // A new palette was created previously and we are restoring the old one
        ::SelectPalette(hdc, hpalOld, TRUE);
        ::RealizePalette(hdc);
        DeleteObject(hpalNew);
        hpalNew = NULL;
    }
    return hpalNew;
}

int WINAPI CW32System::GetMapMode(HDC hdc)
{
    return ::GetMapMode(hdc);
}

BOOL WINAPI CW32System::WinLPtoDP(HDC hdc, LPPOINT lppoints, int nCount)
{
    return ::LPtoDP(hdc, lppoints, nCount);
}

BOOL WINAPI CW32System::WinDPtoLP(HDC hdc, LPPOINT lppoints, int nCount)
{
    return ::DPtoLP(hdc, lppoints, nCount);
}

long WINAPI CW32System::WvsprintfA( LONG cbBuf, LPSTR szBuf, LPCSTR szFmt, va_list arglist )
{
    LONG cb;
    cb = ::wvsprintfA( szBuf, szFmt, arglist );
    Assert(cb < cbBuf);
    return cb;
}

int WINAPI CW32System::MulDiv(int nNumber, int nNumerator, int nDenominator)
{
    return ::MulDiv(nNumber, nNumerator, nDenominator);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::CStrIn
//
//  Synopsis:   Inits the class.
//
//  NOTE:       Don't inline these functions or you'll increase code size
//              by pushing -1 on the stack for each call.
//
//----------------------------------------------------------------------------

CStrIn::CStrIn(LPCWSTR pwstr)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, -1);
}

CStrIn::CStrIn(LPCWSTR pwstr, int cwch)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::CStrIn");

    Init(pwstr, cwch);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrIn::Init
//
//  Synopsis:   Converts a LPWSTR function argument to a LPSTR.
//
//  Arguments:  [pwstr] -- The function argument.  May be NULL or an atom
//                              (HIWORD(pwstr) == 0).
//
//              [cwch]  -- The number of characters in the string to
//                          convert.  If -1, the string is assumed to be
//                          NULL terminated and its length is calculated.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

void
CStrIn::Init(LPCWSTR pwstr, int cwch)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrIn::Init");

    int cchBufReq;

    _cchLen = 0;

    // Check if string is NULL or an atom.
    if (HIWORD(pwstr) == 0)
    {
        _pstr = (LPSTR) pwstr;
        return;
    }

    Assert(cwch == -1 || cwch > 0);

    //
    // Convert string to preallocated buffer, and return if successful.
    //

    _cchLen = W32->MbcsFromUnicode(_ach, ARRAY_SIZE(_ach), pwstr, cwch);

    if (_cchLen > 0)
    {
        if(_ach[_cchLen-1] == 0)
            _cchLen--;                // account for terminator
        _pstr = _ach;
        return;
    }

    //
    // Alloc space on heap for buffer.
    //

    TRACEINFOSZ("CStrIn: Allocating buffer for wrapped function argument.");

    cchBufReq = WideCharToMultiByte(
            CP_ACP, 0, pwstr, cwch, NULL, 0,  NULL, NULL);

    Assert(cchBufReq > 0);
    _pstr = new char[cchBufReq];
    if (!_pstr)
    {
        // On failure, the argument will point to the empty string.
        TRACEINFOSZ("CStrIn: No heap space for wrapped function argument.");
        _ach[0] = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD(_pstr));
    _cchLen = -1 + W32->MbcsFromUnicode(_pstr, cchBufReq, pwstr, cwch);

    Assert(_cchLen >= 0);
}



//+---------------------------------------------------------------------------
//
//  Class:      CStrInMulti (CStrIM)
//
//  Purpose:    Converts multiple strings which are terminated by two NULLs,
//              e.g. "Foo\0Bar\0\0"
//
//----------------------------------------------------------------------------

class CStrInMulti : public CStrIn
{
public:
    CStrInMulti(LPCWSTR pwstr);
};



//+---------------------------------------------------------------------------
//
//  Member:     CStrInMulti::CStrInMulti
//
//  Synopsis:   Converts mulitple LPWSTRs to a multiple LPSTRs.
//
//  Arguments:  [pwstr] -- The strings to convert.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

CStrInMulti::CStrInMulti(LPCWSTR pwstr)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInMulti::CStrInMulti");

    LPCWSTR pwstrT;

    // We don't handle atoms because we don't need to.
    Assert(HIWORD(pwstr));

    //
    // Count number of characters to convert.
    //

    pwstrT = pwstr;
    if (pwstr)
    {
        do {
            while (*pwstrT++)
                ;

        } while (*pwstrT++);
    }

    Init(pwstr, pwstrT - pwstr);
}

//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::CStrOut
//
//  Synopsis:   Allocates enough space for an out buffer.
//
//  Arguments:  [pwstr]   -- The Unicode buffer to convert to when destroyed.
//                              May be NULL.
//
//              [cwchBuf] -- The size of the buffer in characters.
//
//  Modifies:   [this].
//
//----------------------------------------------------------------------------

CStrOut::CStrOut(LPWSTR pwstr, int cwchBuf)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::CStrOut");

    Assert(cwchBuf >= 0);

    _pwstr = pwstr;
    _cwchBuf = cwchBuf;

    if (!pwstr)
    {
        Assert(cwchBuf == 0);
        _pstr = NULL;
        return;
    }

    Assert(HIWORD(pwstr));

    // Initialize buffer in case Windows API returns an error.
    _ach[0] = 0;

    // Use preallocated buffer if big enough.
    if (cwchBuf * 2 <= ARRAY_SIZE(_ach))
    {
        _pstr = _ach;
        return;
    }

    // Allocate buffer.
    TRACEINFOSZ("CStrOut: Allocating buffer for wrapped function argument.");
    _pstr = new char[cwchBuf * 2];
    if (!_pstr)
    {
        //
        // On failure, the argument will point to a zero-sized buffer initialized
        // to the empty string.  This should cause the Windows API to fail.
        //

        TRACEINFOSZ("CStrOut: No heap space for wrapped function argument.");
        Assert(cwchBuf > 0);
        _pwstr[0] = 0;
        _cwchBuf = 0;
        _pstr = _ach;
        return;
    }

    Assert(HIWORD(_pstr));
    _pstr[0] = 0;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::Convert
//
//  Synopsis:   Converts the buffer from MBCS to Unicode.
//
//----------------------------------------------------------------------------

int
CStrOut::Convert()
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::Convert");

    int cch;

    if (!_pstr)
        return 0;

    cch = MultiByteToWideChar(CP_ACP, 0, _pstr, -1, _pwstr, _cwchBuf);
    Assert(cch > 0 || _cwchBuf == 0);

    Free();
    return cch - 1;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOut::~CStrOut
//
//  Synopsis:   Converts the buffer from MBCS to Unicode.
//
//  Note:       Don't inline this function, or you'll increase code size as
//              both Convert() and CConvertStr::~CConvertStr will be called
//              inline.
//
//----------------------------------------------------------------------------

CStrOut::~CStrOut()
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOut::~CStrOut");

    Convert();
}


//
//  MultiByte --> UNICODE routins
//

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStr::Free
//
//  Synopsis:   Frees string if alloc'd and initializes to NULL.
//
//----------------------------------------------------------------------------

void
CConvertStr::Free()
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStr::Free");

    if (_pstr != _ach && HIWORD(_pstr) != 0)
    {
        delete [] _pstr;
    }

    _pstr = NULL;
}

//+---------------------------------------------------------------------------
//
//  Member:     CConvertStrW::Free
//
//  Synopsis:   Frees string if alloc'd and initializes to NULL.
//
//----------------------------------------------------------------------------

void
CConvertStrW::Free()
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CConvertStrW::Free");

    if (_pwstr != _awch && HIWORD(_pwstr) != 0 )
    {
        delete [] _pwstr;
    }

    _pwstr = NULL;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::CStrInW
//
//  Synopsis:   Inits the class.
//
//  NOTE:       Don't inline these functions or you'll increase code size
//              by pushing -1 on the stack for each call.
//
//----------------------------------------------------------------------------

CStrInW::CStrInW(LPCSTR pstr)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, CP_ACP);
}

CStrInW::CStrInW(LPCSTR pstr, UINT uiCodePage)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, -1, uiCodePage);
}

CStrInW::CStrInW(LPCSTR pstr, int cch, UINT uiCodePage)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::CStrInW");

    Init(pstr, cch, uiCodePage);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrInW::Init
//
//  Synopsis:   Converts a LPSTR function argument to a LPWSTR.
//
//  Arguments:  [pstr] -- The function argument.  May be NULL or an atom
//                              (HIWORD(pwstr) == 0).
//
//              [cch]  -- The number of characters in the string to
//                          convert.  If -1, the string is assumed to be
//                          NULL terminated and its length is calculated.
//
//  Modifies:   [this]
//
//----------------------------------------------------------------------------

void
CStrInW::Init(LPCSTR pstr, int cch, UINT uiCodePage)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrInW::Init");

    int cchBufReq;

    _cwchLen = 0;

    // Check if string is NULL or an atom.
    if (HIWORD(pstr) == 0)
    {
        _pwstr = (LPWSTR) pstr;
        return;
    }

    Assert(cch == -1 || cch > 0);

    //
    // Convert string to preallocated buffer, and return if successful.
    //

    _cwchLen = MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _awch, ARRAY_SIZE(_awch));

    if (_cwchLen > 0)
    {
        if(_awch[_cwchLen-1] == 0)
            _cwchLen--;                // account for terminator
        _pwstr = _awch;
        return;
    }

    //
    // Alloc space on heap for buffer.
    //

    TRACEINFOSZ("CStrInW: Allocating buffer for wrapped function argument.");

    cchBufReq = MultiByteToWideChar(
            CP_ACP, 0, pstr, cch, NULL, 0);

    Assert(cchBufReq > 0);
    _pwstr = new WCHAR[cchBufReq];
    if (!_pwstr)
    {
        // On failure, the argument will point to the empty string.
        TRACEINFOSZ("CStrInW: No heap space for wrapped function argument.");
        _awch[0] = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD(_pwstr));
    _cwchLen = -1 + MultiByteToWideChar(
            uiCodePage, 0, pstr, cch, _pwstr, cchBufReq);
    Assert(_cwchLen >= 0);
}


//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::CStrOutW
//
//  Synopsis:   Allocates enough space for an out buffer.
//
//  Arguments:  [pstr]   -- The ansi buffer to convert to when destroyed.
//                              May be NULL.
//
//              [cchBuf] -- The size of the buffer in characters.
//
//  Modifies:   [this].
//
//----------------------------------------------------------------------------

CStrOutW::CStrOutW(LPSTR pstr, int cchBuf, UINT uiCodePage)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::CStrOutW");

    Assert(cchBuf >= 0);

    _pstr = pstr;
    _cchBuf = cchBuf;
    _uiCodePage = uiCodePage;

    if (!pstr)
    {
        Assert(cchBuf == 0);
        _pwstr = NULL;
        return;
    }

    Assert(HIWORD(pstr));

    // Initialize buffer in case Windows API returns an error.
    _awch[0] = 0;

    // Use preallocated buffer if big enough.
    if (cchBuf <= ARRAY_SIZE(_awch))
    {
        _pwstr = _awch;
        return;
    }

    // Allocate buffer.
    TRACEINFOSZ("CStrOutW: Allocating buffer for wrapped function argument.");
    _pwstr = new WCHAR[cchBuf * 2];
    if (!_pwstr)
    {
        //
        // On failure, the argument will point to a zero-sized buffer initialized
        // to the empty string.  This should cause the Windows API to fail.
        //

        TRACEINFOSZ("CStrOutW: No heap space for wrapped function argument.");
        Assert(cchBuf > 0);
        _pstr[0] = 0;
        _cchBuf = 0;
        _pwstr = _awch;
        return;
    }

    Assert(HIWORD(_pwstr));
    _pwstr[0] = 0;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::Convert
//
//  Synopsis:   Converts the buffer from Unicode to MBCS
//
//----------------------------------------------------------------------------

int
CStrOutW::Convert()
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::Convert");

    int cch;

    if (!_pwstr)
        return 0;

    WCHAR *pwstr = _pwstr;
    int cchBuf = _cchBuf;

    cch = W32->MbcsFromUnicode(_pstr, cchBuf, _pwstr, -1, _uiCodePage);

    Free();
    return cch - 1;
}



//+---------------------------------------------------------------------------
//
//  Member:     CStrOutW::~CStrOutW
//
//  Synopsis:   Converts the buffer from Unicode to MBCS.
//
//  Note:       Don't inline this function, or you'll increase code size as
//              both Convert() and CConvertStr::~CConvertStr will be called
//              inline.
//
//----------------------------------------------------------------------------

CStrOutW::~CStrOutW()
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CStrOutW::~CStrOutW");

    Convert();
}

BOOL CW32System::GetVersion(
    DWORD *pdwPlatformId,
    DWORD *pdwMajorVersion
)
{
    OSVERSIONINFOA osv;
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    *pdwPlatformId = 0;
    *pdwMajorVersion = 0;
    if (::GetVersionExA(&osv))
    {
        *pdwPlatformId = osv.dwPlatformId;
        *pdwMajorVersion = osv.dwMajorVersion;
        return TRUE;
    }
    return FALSE;
}

BOOL WINAPI CW32System::GetStringTypeEx(
    LCID lcid,
    DWORD dwInfoType,
    LPCTSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetStringTypeEx");

    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetStringTypeExW(lcid, dwInfoType, lpSrcStr, cchSrc, lpCharType);

    CStrIn  str(lpSrcStr, cchSrc);
    return GetStringTypeExA(lcid, dwInfoType, str, str.strlen(), lpCharType);
}

typedef LPSTR (CALLBACK *FnCharChangeCase)(LPSTR);

static LPWSTR CharChangeCase(LPWSTR pwstr, FnCharChangeCase pfn)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharChangeCaseWrap");

    if (HIWORD(pwstr) == 0)
    {
        LPSTR   pstr=0;
        int     retCode;
        char    DBChar[3];

        retCode = W32->MbcsFromUnicode((LPSTR) &pstr, sizeof(pstr), (LPWSTR) &pwstr, 1);
        Assert(HIWORD(pstr) == 0);
        if (retCode == 2)
        {
            // This is a DBC, use string
            DWORD   iTemp = (int)pstr;
            DBChar[0] = iTemp & 0x0ff;
            DBChar[1] = iTemp >> 8;
            DBChar[2] = 0;
            pstr = (*pfn)(DBChar);
            W32->UnicodeFromMbcs((LPWSTR) &pwstr, sizeof(pwstr) / sizeof(WCHAR), (LPSTR)DBChar, 2);
        }
        else
        {
            pstr = (*pfn)(pstr);
            W32->UnicodeFromMbcs((LPWSTR) &pwstr, sizeof(pwstr) / sizeof(WCHAR), (LPSTR) &pstr);
        }
        Assert(HIWORD(pwstr) == 0);
    }
    else
    {
        CStrOut strOut(pwstr, wcslen(pwstr));
        W32->MbcsFromUnicode(strOut, strOut.BufSize(), pwstr);
        (*pfn)(strOut);
    }
    return pwstr;
}

LPWSTR WINAPI CW32System::CharLower(LPWSTR pwstr)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLower");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CharLowerW(pwstr);
    return CharChangeCase(pwstr, CharLowerA);
}

DWORD WINAPI CW32System::CharLowerBuff(LPWSTR pwstr, DWORD cchLength)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharLowerBuff");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CharLowerBuffW(pwstr, cchLength);
    LPWSTR lpBuffer = pwstr;
    DWORD pos = 0;
    for (pos = 0; pos < cchLength; pos++, lpBuffer++)
        *lpBuffer =  (WCHAR)CharChangeCase((LPWSTR)*lpBuffer, CharLowerA);
    return pos;
}

DWORD WINAPI CW32System::CharUpperBuff(LPWSTR pwstr, DWORD cchLength)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CharUpperBuff");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CharUpperBuffW(pwstr, cchLength);
    LPWSTR lpBuffer = pwstr;
    DWORD pos = 0;
    for (pos = 0; pos < cchLength; pos++, lpBuffer++)
        *lpBuffer =  (WCHAR)CharChangeCase((LPWSTR)*lpBuffer, CharUpperA);
    return pos;
}

typedef HDC (CALLBACK *FnCreateHDCA)(LPCSTR, LPCSTR, LPCSTR, CONST DEVMODEA *);

static HDC WINAPI CreateHDCAux(
    LPCWSTR             lpszDriver,
    LPCWSTR             lpszDevice,
    LPCWSTR             lpszOutput,
    CONST DEVMODEW *    lpInitData,
    FnCreateHDCA        pfn
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateHDCWrap");

    DEVMODEA    devmode;
    CStrIn      strDriver(lpszDriver);
    CStrIn      strDevice(lpszDevice);
    CStrIn      strOutput(lpszOutput);

    if ( lpInitData )
    {
        // converting DEVMODEW to DEVMODEA

        int byteCount;

        // copying the data between the two strings members
        byteCount = (char *)&(devmode.dmFormName)
            - (char *)&(devmode.dmSpecVersion);
        memcpy(&(devmode.dmSpecVersion),
            &(lpInitData->dmSpecVersion),
            byteCount);

        // copying the data after the second string member
        byteCount = (char *)((char *)&devmode + sizeof(DEVMODEA))
            - (char *)&(devmode.dmLogPixels);
        memcpy(&(devmode.dmLogPixels),
            &(lpInitData->dmLogPixels),
            byteCount);

        // converting the two strings members
        W32->MbcsFromUnicode((CHAR *)devmode.dmDeviceName, CCHDEVICENAME, lpInitData->dmDeviceName);
        W32->MbcsFromUnicode((CHAR *)devmode.dmFormName, CCHFORMNAME, lpInitData->dmFormName);
    }

    return (*pfn)(strDriver, strDevice, strOutput,
        lpInitData ? &devmode : NULL);
}

HDC WINAPI CW32System::CreateIC(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateIC");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CreateICW( lpszDriver, lpszDevice, lpszOutput, lpInitData );
    return CreateHDCAux(lpszDriver, lpszDevice, lpszOutput, lpInitData, CreateICA);
}

HANDLE WINAPI CW32System::CreateFile(
    LPCWSTR                 lpFileName,
    DWORD                   dwDesiredAccess,
    DWORD                   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   dwCreationDisposition,
    DWORD                   dwFlagsAndAttributes,
    HANDLE                  hTemplateFile
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFile");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CreateFileW(lpFileName,
                            dwDesiredAccess,
                            dwShareMode,
                            lpSecurityAttributes,
                            dwCreationDisposition,
                            dwFlagsAndAttributes,
                            hTemplateFile);

    CStrIn  str(lpFileName);
    return ::CreateFileA(
            str,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDisposition,
            dwFlagsAndAttributes,
            hTemplateFile);
}

HFONT WINAPI CW32System::CreateFontIndirect(CONST LOGFONTW * plfw)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CreateFontIndirect");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CreateFontIndirectW(plfw);
    LOGFONTA  lfa;
    HFONT     hFont;

    memcpy(&lfa, plfw, offsetof(LOGFONTA, lfFaceName));
    MbcsFromUnicode(lfa.lfFaceName, ARRAY_SIZE(lfa.lfFaceName), plfw->lfFaceName,
        -1, CP_ACP, UN_NOOBJECTS);
    hFont = ::CreateFontIndirectA(&lfa);
    return hFont;
}

int WINAPI CW32System::CompareString (
    LCID  Locale,           // locale identifier
    DWORD  dwCmpFlags,      // comparison-style options
    LPCWSTR  lpString1,     // pointer to first string
    int  cchCount1,         // size, in bytes or characters, of first string
    LPCWSTR  lpString2,     // pointer to second string
    int  cchCount2          // size, in bytes or characters, of second string
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "CompareString");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);

    CStrIn      str1(lpString1, cchCount1);
    CStrIn      str2(lpString2, cchCount2);

    return CompareStringA(
        Locale,
        dwCmpFlags,
        str1,
        str1.strlen(),
        str2,
        str2.strlen()
        );
}

LRESULT WINAPI CW32System::DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "DefWindowProcWrap");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    return ::DefWindowProcA(hWnd, msg, wParam, lParam);
}

int WINAPI CW32System::GetObject(HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetObject");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetObjectW( hgdiObj, cbBuffer, lpvObj);

    int nRet;

    if(cbBuffer != sizeof(LOGFONTW) || !lpvObj)
    {
        nRet = ::GetObjectA(hgdiObj, cbBuffer, lpvObj);
        if(nRet == sizeof(LOGFONTA))
        {
            nRet = sizeof(LOGFONTW);
        }
    }
    else
    {
        LOGFONTA lfa;

        nRet = ::GetObjectA(hgdiObj, sizeof(lfa), &lfa);

        if(nRet > 0)
        {
            memcpy(lpvObj, &lfa, offsetof(LOGFONTW, lfFaceName));
            UnicodeFromMbcs(((LOGFONTW*)lpvObj)->lfFaceName, ARRAY_SIZE(((LOGFONTW*)lpvObj)->lfFaceName),
                            lfa.lfFaceName, -1);
            nRet = sizeof(LOGFONTW);
        }
    }

    return nRet;
}

DWORD APIENTRY CW32System::GetProfileSection(
    LPCWSTR lpAppName,
    LPWSTR lpReturnedString,
    DWORD nSize
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetProfileSection");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetProfileSectionW( lpAppName, lpReturnedString, nSize );

    CStrIn  strAppName(lpAppName);

    // we can't use CStrOut here, since the returned string contains a set of
    // strings delimited by single-NULL's and terminated by a double-NULL
    char *pszReturnedString;

    pszReturnedString = new char[nSize];
    Assert(pszReturnedString);

    DWORD cch = ::GetProfileSectionA(strAppName, pszReturnedString, nSize);

    if(cch)
    {
        MultiByteToWideChar(CP_ACP, 0, pszReturnedString, cch,
                                lpReturnedString, nSize);
    }

    return cch;
}

BOOL APIENTRY CW32System::GetTextExtentPoint32(
    HDC     hdc,
    LPCWSTR pwsz,
    int     cb,
    LPSIZE  pSize
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextExtentPoint32");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetTextExtentPoint32W( hdc, pwsz, cb, pSize );
     CStrIn str(pwsz);
     return ::GetTextExtentPoint32A(hdc, str, cb, pSize);
}

int WINAPI CW32System::GetTextFace(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextFace");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetTextFaceW( hdc, cch, lpFaceName );
    CStrOut str(lpFaceName, cch);
    ::GetTextFaceA(hdc, str.BufSize(), str);
    return str.Convert();
}

BOOL WINAPI CW32System::GetTextMetrics(HDC hdc, LPTEXTMETRICW lptm)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetTextMetrics");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetTextMetricsW( hdc, lptm);

   BOOL         ret;
   TEXTMETRICA  tm;

   ret = ::GetTextMetricsA(hdc, &tm);

    if (ret)
    {
        lptm->tmHeight              = tm.tmHeight;
        lptm->tmAscent              = tm.tmAscent;
        lptm->tmDescent             = tm.tmDescent;
        lptm->tmInternalLeading     = tm.tmInternalLeading;
        lptm->tmExternalLeading     = tm.tmExternalLeading;
        lptm->tmAveCharWidth        = tm.tmAveCharWidth;
        lptm->tmMaxCharWidth        = tm.tmMaxCharWidth;
        lptm->tmWeight              = tm.tmWeight;
        lptm->tmOverhang            = tm.tmOverhang;
        lptm->tmDigitizedAspectX    = tm.tmDigitizedAspectX;
        lptm->tmDigitizedAspectY    = tm.tmDigitizedAspectY;
        lptm->tmItalic              = tm.tmItalic;
        lptm->tmUnderlined          = tm.tmUnderlined;
        lptm->tmStruckOut           = tm.tmStruckOut;
        lptm->tmPitchAndFamily      = tm.tmPitchAndFamily;
        lptm->tmCharSet             = tm.tmCharSet;

        UnicodeFromMbcs(&lptm->tmFirstChar, 1, (LPSTR) &tm.tmFirstChar, 1);
        UnicodeFromMbcs(&lptm->tmLastChar, 1, (LPSTR) &tm.tmLastChar, 1);
        UnicodeFromMbcs(&lptm->tmDefaultChar, 1, (LPSTR) &tm.tmDefaultChar, 1);
        UnicodeFromMbcs(&lptm->tmBreakChar, 1, (LPSTR) &tm.tmBreakChar, 1);
    }

    return ret;
}

LONG WINAPI CW32System::GetWindowLong(HWND hWnd, int nIndex)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "GetWindowLong");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetWindowLongW(hWnd, nIndex);
    return ::GetWindowLongA(hWnd, nIndex);
}

HBITMAP WINAPI CW32System::LoadBitmap(HINSTANCE hInstance, LPCWSTR lpBitmapName)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadBitmap");
    Assert(HIWORD(lpBitmapName) == 0);
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::LoadBitmapW(hInstance, lpBitmapName);
    return ::LoadBitmapA(hInstance, (LPCSTR) lpBitmapName);
}

HCURSOR WINAPI CW32System::LoadCursor(HINSTANCE hInstance, LPCWSTR lpCursorName)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadCursor");
    Assert(HIWORD(lpCursorName) == 0);
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::LoadCursorW(hInstance, lpCursorName);
    return ::LoadCursorA(hInstance, (LPCSTR) lpCursorName);
}

HINSTANCE WINAPI CW32System::LoadLibrary(LPCWSTR lpLibFileName)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "LoadLibrary");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::LoadLibraryW(lpLibFileName);
    CStrIn  str(lpLibFileName);
    return ::LoadLibraryA(str);
}

LRESULT WINAPI CW32System::SendMessage(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SendMessage");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::SendMessageW(hWnd, Msg, wParam, lParam);

    switch (Msg)
    {
    case WM_GETTEXT:
        {
            CStrOut str((LPWSTR)lParam, (int) wParam);
            return ::SendMessageA(hWnd, Msg, (WPARAM) str.BufSize(), (LPARAM) (LPSTR) str);
        }

    case WM_SETTEXT:
    case LB_ADDSTRING:
    case LB_INSERTSTRING:
    case CB_ADDSTRING:
    case CB_SELECTSTRING:
    case CB_INSERTSTRING:
    case EM_REPLACESEL:
        {
            CStrIn  str((LPWSTR) lParam);
            return ::SendMessageA(hWnd, Msg, (WPARAM) str.strlen(), (LPARAM) (LPSTR) str);
        }

    case LB_GETTEXT:
    case CB_GETLBTEXT:
        {
            CStrOut str((LPWSTR)lParam, 255);
            return ::SendMessageA(hWnd, Msg, wParam, (LPARAM) (LPSTR) str);
        }

    case EM_SETPASSWORDCHAR:
        {
            WPARAM  wp;

            Assert(HIWORD(wParam) == 0);
            MbcsFromUnicode((LPSTR) &wp, sizeof(wp), (LPWSTR) &wParam);
            Assert(HIWORD(wp) == 0);

            return ::SendMessageA(hWnd, Msg, wp, lParam);
        }

    default:
        return ::SendMessageA(hWnd, Msg, wParam, lParam);
    }
}

LONG WINAPI CW32System::SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "SetWindowLong");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::SetWindowLongW(hWnd, nIndex, dwNewLong);
    return ::SetWindowLongA(hWnd, nIndex, dwNewLong);
}

BOOL WINAPI CW32System::PostMessage(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PostMessage");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::PostMessageW(hWnd, Msg, wParam, lParam);
    return ::PostMessageA(hWnd, Msg, wParam, lParam);
}

BOOL WINAPI CW32System::UnregisterClass(LPCWSTR lpClassName, HINSTANCE hInstance)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "UnregisterClass");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::UnregisterClassW( lpClassName, hInstance);
    CStrIn  str(lpClassName);
    return ::UnregisterClassA(str, hInstance);
}

int WINAPI CW32System::lstrcmp(LPCWSTR lpString1, LPCWSTR lpString2)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmp");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::lstrcmpW(lpString1, lpString2);
    return ::wcscmp(lpString1, lpString2);
}

int WINAPI CW32System::lstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "lstrcmpi");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::lstrcmpiW(lpString1, lpString2);
    return ::_wcsicmp(lpString1, lpString2);
}

BOOL WINAPI CW32System::PeekMessage(
    LPMSG   lpMsg,
    HWND    hWnd,
    UINT    wMsgFilterMin,
    UINT    wMsgFilterMax,
    UINT    wRemoveMsg
)
{
    TRACEBEGIN(TRCSUBSYSWRAP, TRCSCOPEINTERN, "PeekMessage");
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return ::PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

DWORD WINAPI CW32System::GetModuleFileName(
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
)
{
    if (VER_PLATFORM_WIN32_WINDOWS != _dwPlatformId)
        return ::GetModuleFileNameW(hModule, lpFilename, nSize);
    CStrOut  strout(lpFilename, nSize);
    DWORD res = ::GetModuleFileNameA(hModule, strout, nSize);
    strout.Convert();
    return res;
}

BOOL WINAPI CW32System::GetCursorPos(
    POINT *ppt
)
{
    return ::GetCursorPos(ppt);
}
#if 0

    // flags
#pragma message ("JMO Review : Should be same as has nlsprocs, initialize in constructor")
    unsigned _fIntlKeyboard : 1;

// From nlsprocs.h

// TranslateCharsetInfo
// typedef WINGDIAPI BOOL (WINAPI*TCI_CAST)( DWORD FAR *, LPCHARSETINFO, DWORD);
// #define  pTranslateCharsetInfo(a,b,c) (( TCI_CAST) nlsProcTable[iTranslateCharsetInfo])(a,b,c)

//#define   pTranslateCharsetInfo() (*()nlsProcTable[iTranslateCharsetInfo])()


/*
 *  CW32System::CheckChangeKeyboardLayout ( BOOL fChangedFont )
 *
 *  @mfunc
 *      Change keyboard for new font, or font at new character position.
 *  @comm
 *      Using only the currently loaded KBs, locate one that will support
 *      the insertion points font. This is called anytime a character format
 *      change occurs, or the insert font (caret position) changes.
 *  @devnote
 *      The current KB is preferred. If a previous association
 *      was made, see if the KB is still loaded in the system and if so use
 *      it. Otherwise, locate a suitable KB, preferring KB's that have
 *      the same charset ID as their default, preferred charset. If no match
 *      can be made then nothing changes.
 *
 *      This routine is only useful on Windows 95.
 */
#ifndef MACPORT
#ifndef PEGASUS


#define MAX_HKLS 256                                // It will be awhile
                                                    //  before we have more KBs
    CTxtEdit * const ped = GetPed();                // Document context.

    INT         i, totalLayouts,                    // For matching KBs.
                iBestLayout = -1;

    WORD        preferredKB;                        // LCID of preferred KB.

    HKL         hklList[MAX_HKLS];                  // Currently loaded KBs.

    const CCharFormat *pcf;                         // Current font.
    CHARSETINFO csi;                                // Font's CodePage bits.

    AssertSz(ped, "no ped?");                       // hey now!

    if (!ped || !ped->IsRich() || !ped->_fFocus ||  // EXIT if no ped or focus or
        !ped->IsAutoKeyboard())                     // auto keyboard is turn off
        return;

    pcf = ped->GetCharFormat(_iFormat);             // Get insert font's data

    hklList[0]      = pGetKeyboardLayout(0);        // Current hkl preferred?
    preferredKB     = fc().GetPreferredKB( pcf->bCharSet );
    if ( preferredKB != LOWORD(hklList[0]) )        // Not correct KB?
    {
                                                    // Get loaded HKLs.
        totalLayouts    = 1 + pGetKeyboardLayoutList(MAX_HKLS, &hklList[1]);
                                                    // Know which KB?
        if ( preferredKB )                          //  then locate it.
        {                                           // Sequential match because
            for ( i = 1; i < totalLayouts; i++ )    //  HKL may be unloaded.
            {                                       // Match LCIDs.
                if ( preferredKB == LOWORD( hklList[i]) )
                {
                    iBestLayout = i;
                    break;                          // Matched it.
                }
            }
            if ( i >= totalLayouts )                // Old KB is missing.
            {                                       // Reset to locate new KB.
                fc().SetPreferredKB ( pcf->bCharSet, 0 );
            }
        }
        if ( iBestLayout < 0 )                          // Attempt to find new KB.
        {
            for ( i = 0; i < totalLayouts; i++ )
            {
                pTranslateCharsetInfo(              // Get KB's charset.
                        (DWORD *) ConvertLanguageIDtoCodePage(LOWORD(hklList[iBestLayout])),
                        &csi, TCI_SRCCODEPAGE);

                if( csi.ciCharset == pcf->bCharSet) // If charset IDs match?
                {
                    iBestLayout = i;
                    break;                          //  then this KB is best.
                }
            }
            if ( iBestLayout >= 0)                  // Bind new KB.
            {
                fChangedFont = TRUE;
                fc().SetPreferredKB(pcf->bCharSet, LOWORD(hklList[iBestLayout]));
            }
        }
        if ( fChangedFont && iBestLayout >= 0)          // Bind font.
        {
            ICharFormatCache *  pCF;

            if(SUCCEEDED(GetCharFormatCache(&pCF)))
            {
                pCF->AddRefFormat(_iFormat);
                fc().SetPreferredFont(
                        LOWORD(hklList[iBestLayout]), _iFormat );
            }
        }
        if( iBestLayout > 0 )                           // If == 0 then
        {                                               //  it's already active.
                                                        // Activate KB.
            ActivateKeyboardLayout( hklList[iBestLayout], 0);
        }
    }
#endif // PEGASUS needs its own code
#endif // MACPORT -- the mac needs its own code.

/*
 *  CTxtSelection::CheckChangeFont ( CTxtEdit * const ped, const WORD lcID )
 *
 *  @mfunc
 *      Change font for new keyboard layout.
 *  @comm
 *      If no previous preferred font has been associated with this KB, then
 *      locate a font in the document suitable for this KB.
 *  @devnote
 *      This routine is called via WM_INPUTLANGCHANGEREQUEST message
 *      (a keyboard layout switch). This routine can also be called
 *      from WM_INPUTLANGCHANGE, but we are called more, and so this
 *      is less efficient.
 *
 *      Exact match is done via charset ID bitmask. If a match was previously
 *      made, use it. A user can force the insertion font to be associated
 *      to a keyboard if the control key is held through the KB changing
 *      process. The association is broken when another KB associates to
 *      the font. If no match can be made then nothing changes.
 *
 *      This routine is only useful on Windows 95.
 *
 */
#ifndef MACPORT
#ifndef PEGASUS

    LOCALESIGNATURE ls, curr_ls;                    // KB's code page bits.

    CCharFormat     cf,                             // For creating new font.
                    currCF;                         // For searching
    const CCharFormat   *pPreferredCF;
    CHARSETINFO     csi;                            //  with code page bits.

    LONG            iFormat, iBestFormat = -1;      // Loop support.
    INT             i;

    BOOL            fLastTime;                      // Loop support.
    BOOL            fSetPreferred = FALSE;

    HKL             currHKL;                        // current KB;

    BOOL            fLookUpFaceName = FALSE;        // when picking a new font.

    ICharFormatCache *  pCF;

    LPTSTR          lpszFaceName = NULL;
    BYTE            bCharSet;
    BYTE            bPitchAndFamily;
    BOOL            fFaceNameIsDBCS;

    AssertSz (ped, "No ped?");

    if (!ped->IsRich() || !ped->IsAutoFont())       // EXIT if not running W95.
        return;                                     // EXIT if auto font is turn off

    if(FAILED(GetCharFormatCache(&pCF)))            // Get CharFormat Cache.
        return;

    cf.InitDefault(0);

    // An alternate approach would be to get the key state from the corresponding
    // message; FUTURE (alexgo): we can consider doing this, but we need to make sure
    // everything works with windowless controls..
    BOOL fReassign = fEnableReassign
                  && (GetAsyncKeyState(VK_CONTROL)<0);// Is user holding CTRL?

    currHKL = pGetKeyboardLayout(0);

    ped->GetCharFormat(_iFormat)->Get(&currCF);
    GetLocaleInfoA( lcID, LOCALE_FONTSIGNATURE, (char *) &ls, sizeof(ls));

    if ( fReassign )                                // Force font/KB assoc.
    {                                               // If font supports KB
                                                    //  in any way,
                                                    // Note: test Unicode bits.
        GetLocaleInfoA( fc().GetPreferredKB (currCF.bCharSet),
                LOCALE_FONTSIGNATURE, (char *) &curr_ls, sizeof(ls));
        if ( CountMatchingBits(curr_ls.lsUsb, ls.lsUsb, 4) )
        {                                           // Break old font/KB assoc.
            fc().SetPreferredFont( fc().GetPreferredKB (currCF.bCharSet), -1 );
                                                    // Bind KB and font.
            fc().SetPreferredKB( currCF.bCharSet, lcID );

            pCF->AddRefFormat(_iFormat);
            fc().SetPreferredFont( lcID, _iFormat );
        }
    }
    else                                            // Lookup preferred font.
    {
                                                    // Attempt to Find new
        {                                           //  preferred font.
            CFormatRunPtr rp(_rpCF);                // Nondegenerate range

            fLastTime = TRUE;
            if ( _rpCF.IsValid() )                  // If doc has cf runs.
            {
                fLastTime = FALSE;
                rp.AdjustBackward();
            }
            pTranslateCharsetInfo(                  //  charset.
                        (DWORD *)cpg, &csi, TCI_SRCCODEPAGE);

            iFormat = _iFormat;                     // Search _iFormat,
                                                    //  then backwards.
            i = MAX_RUNTOSEARCH;                    // Don't be searching for
            while ( 1 )                             //  years...
            {                                       // Get code page bits.
                pPreferredCF = ped->GetCharFormat(iFormat);

                if (csi.ciCharset == pPreferredCF->bCharSet)    // Equal charset ids?
                {
                    fSetPreferred = TRUE;
                    break;
                }
                if ( fLastTime )                    // Done searching?
                    break;
                iFormat = rp.GetFormat();           // Keep searching backward.
                fLastTime = !rp.PrevRun() && i--;
            }
            if ( !fSetPreferred && _rpCF.IsValid()) // Try searching forward.
            {
                rp = _rpCF;
                rp.AdjustBackward();
                i = MAX_RUNTOSEARCH;                // Don't be searching for
                while (i-- && rp.NextRun() )        //  years...
                {
                    iFormat = rp.GetFormat();       // Get code page bits.
                    pPreferredCF = ped->GetCharFormat(iFormat);
                                                    // Equal charset ids?
                    if (csi.ciCharset == pPreferredCF->bCharSet)
                    {
                        fSetPreferred = TRUE;
                        break;
                    }
                }
            }
        }

        if ( !fSetPreferred )
        {
            iFormat = fc().GetPreferredFont( lcID );

                                                        // Set preferred if usable.
            if (iFormat >= 0 &&
                csi.ciCharset == ped->GetCharFormat(iFormat)->bCharSet)
            {
                fSetPreferred = TRUE;
                pPreferredCF = ped->GetCharFormat(iFormat);
            }
        }

        // setup cf needed for creating a new format run
        cf = currCF;

        // We know that the facename is not tagged IsDBCS in all cases
        //  unless explicitly set below.
        fFaceNameIsDBCS = FALSE;

        if ( fSetPreferred )
        {
            // pick face name from the previous preferred format
            bPitchAndFamily = pPreferredCF->bPitchAndFamily;
            bCharSet = pPreferredCF->bCharSet;
            lpszFaceName = (LPTSTR)pPreferredCF->szFaceName;
            fFaceNameIsDBCS = pPreferredCF->bInternalEffects & CFMI_FACENAMEISDBCS;
        }
        else                                            // Still don't have a font?
        {                                               //  For FE, use hard coded defaults.
                                                        //  else get charset right.
            WORD CurrentCodePage = cpg;

            switch (CurrentCodePage)
            {                                           // FE hard codes from Word.
            case _JAPAN_CP:
                bCharSet = SHIFTJIS_CHARSET;
                lpszFaceName = lpJapanFontName;
                bPitchAndFamily = 17;
                break;

            case _KOREAN_CP:
                bCharSet = HANGEUL_CHARSET;
                lpszFaceName = lpKoreanFontName;
                bPitchAndFamily = 49;
                break;

            case _CHINESE_TRAD_CP:
                bCharSet = CHINESEBIG5_CHARSET;
                lpszFaceName = lpTChineseFontName;
                bPitchAndFamily = 54;
                break;

            case _CHINESE_SIM_CP:
                bCharSet = GB2312_CHARSET;
                lpszFaceName = lpSChineseFontName;
                bPitchAndFamily = 54;
                break;

            default:                                    // Use translate to get
                pTranslateCharsetInfo(                  //  charset.
                            (DWORD *) CurrentCodePage, &csi, TCI_SRCCODEPAGE);
                bCharSet = csi.ciCharset;

                if (IsFECharset(currCF.bCharSet) && !IsFECharset(bCharSet))
                {
                    // fall back to default
                    lpszFaceName = L"Arial";
                    bPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
                }
                else
                {
                    bPitchAndFamily = currCF.bPitchAndFamily;
                    lpszFaceName = currCF.szFaceName;
                    fFaceNameIsDBCS = currCF.bInternalEffects & CFEI_FACENAMEISDBCS;
                }
                fLookUpFaceName = TRUE;                 // Get Font's real name.

                break;
            }
        }

        // setup the rest of cf
        cf.bPitchAndFamily = bPitchAndFamily;
        cf.bCharSet = (BYTE) bCharSet;
        _tcscpy ( cf.szFaceName, lpszFaceName );
        if(fFaceNameIsDBCS)
        {
            cf.bInternalEffects |= CFEI_FACENAMEISDBCS;
        }
        else
        {
            cf.bInternalEffects &= ~CFEI_FACENAMEISDBCS;
        }
        cf.lcid = lcID;

        // If we relied on GDI to match a font, get the font's real name...
        if ( fLookUpFaceName )
        {
            const CDevDesc      *pdd = _pdp->GetDdRender();
            HDC                 hdc;
            CCcs                *pccs;
            HFONT               hfontOld;
            OUTLINETEXTMETRICA  *potm;
            CTempBuf            mem;
            UINT                otmSize;

            hdc = pdd->GetDC();
                                                // Select logfont into DC,
            if( hdc)                            //  for OutlineTextMetrics.
            {
                cf.SetCRC();
                pccs = fc().GetCcs(hdc, &cf, _pdp->GetZoomNumerator(),
                    _pdp->GetZoomDenominator(),
                    GetDeviceCaps(hdc, LOGPIXELSY));

                if( pccs )
                {
                    hfontOld = SelectFont(hdc, pccs->_hfont);

                    if( otmSize = ::GetOutlineTextMetricsA(hdc, 0, NULL) )
                    {
                        potm = (OUTLINETEXTMETRICA *) mem.GetBuf(otmSize);
                        if ( NULL != potm )
                        {
                            ::GetOutlineTextMetricsA(hdc, otmSize, potm);

                            CStrInW  strinw( &((CHAR *)(potm))[ BYTE(potm->otmpFaceName)] );

                            cf.bPitchAndFamily
                                = potm->otmTextMetrics.tmPitchAndFamily;
                            cf.bCharSet
                                = (BYTE) potm->otmTextMetrics.tmCharSet;

                            _tcscpy ( cf.szFaceName, (WCHAR *)strinw );
                            cf.bInternalEffects &= ~CFEI_FACENAMEISDBCS;
                        }
                    }

                    SelectFont( hdc, hfontOld );

                    pccs->Release();
                }

                pdd->ReleaseDC(hdc);
            }
        }

        if ( SUCCEEDED(pCF->Cache(&cf, &iFormat)) )
        {
            // This is redundent if ed.IsAutoKeyboard() == TRUE.
            pCF->AddRefFormat(_iFormat);
            fc().SetPreferredFont ( LOWORD (currHKL), _iFormat );

            fc().SetPreferredKB( cf.bCharSet, lcID );
            pCF->AddRefFormat(iFormat);
            fc().SetPreferredFont ( lcID, iFormat );

            pCF->ReleaseFormat(_iFormat);
            _iFormat = iFormat;
            ped->GetCallMgr()->SetSelectionChanged();
        }
    }
#endif
#endif // MACPORT -- the mac needs its own code.

                if (FormatMatchesKeyboard(pcfForward))
                LOCALESIGNATURE ls;                             // Per HKL, CodePage bits.
                CHARSETINFO csi;                                // Font's CodePage bits.

                // Font's code page bits.
                pTranslateCharsetInfo((DWORD *) pcfForward->bCharSet, &csi, TCI_SRCCHARSET);
                // Current KB's code page bits.
                GetLocaleInfoA( LOWORD(pGetKeyboardLayout(0)), LOCALE_FONTSIGNATURE, (CHAR *) &ls, sizeof(ls));
                if ( (csi.fs.fsCsb[0] & ls.lsCsbDefault[0]) ||
                            (csi.fs.fsCsb[1] & ls.lsCsbDefault[1]) )
#endif

#if 0
/*
 *  ReExtTextOutW(uiCodePage, hdc, x, y, fuOptions, lprc, lpString, cbCount,lpDx)
 *
 *  @mfunc
 *      Patch around the Win95 FE bug.
 *
 *  @rdesc
 *      Returns whatever ExtTextOut returns
 */
BOOL ReExtTextOutW(
    HDC hdc,                    //@parm handle to device context
    int xp,                     //@parm x-coordinate of reference point
    int yp,                     //@parm y-coordinate of reference point
    UINT fuOptions,             //@parm text-output options
    CONST RECT *lprect,         //@parm optional clipping and/or opaquing rectangle
    const WCHAR *lpwchString,   //@parm points to string
    UINT cchCount,              //@parm number of characters in string
    CONST INT *lpDx)            //@parm Ptr to array of intercharacter spacing values
{
    // This is a protion of Word code adapted for our needs.
    // This is a work around for Win95FE bugs that cause GPFs in GDI if multiple
    // characters above Unicode 0x7F are passed to ExtTextOutW.

    int     cch;
    const WCHAR *lpwchT = lpwchString;
    const WCHAR *lpwchStart = lpwchT;
    const WCHAR *lpwchEnd = lpwchString + cchCount;

    CONST int *lpdxpCur;

    BOOL    fRet = 0;

    while (lpwchT < lpwchEnd)
    {
        // characters less than 0x007F do not need special treatment
        // we output then in contiguous runs
        if (*lpwchT > 0x007F)
        {
            if ((cch = lpwchT - lpwchStart) > 0)
            {
                lpdxpCur = lpDx ? lpDx + (lpwchStart - lpwchString) : NULL;

                // Output the run of chars less than 0x7F
                fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, cch, lpdxpCur);
                if (!fRet)
                    return fRet;

                fuOptions &= ~ETO_OPAQUE; // Don't erase mutliple times!!!

                // Advance
                if (lpdxpCur)
                {
                    while (cch--)
                    {
                        xp += *lpdxpCur++;
                    }
                }
                else
                {
                    SIZE size;
                    GetTextExtentPointW(hdc, lpwchStart, cch, &size);
                    xp += size.cx;
                }

                lpwchStart = lpwchT;
            }

            // Output chars above 0x7F one at a time to prevent Win95 FE GPF
            lpdxpCur = lpDx ? lpDx + (lpwchStart - lpwchString) : NULL;
            fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, 1, lpdxpCur);
            if (!fRet)
                return fRet;

            fuOptions &= ~ETO_OPAQUE; // Don't erase mutliple times!!!

            // Advance
            if (lpdxpCur)
                xp += *lpdxpCur;
            else
            {
                // GetTextExtentPointW GPFs on characters above 0x00FF as well
                // So instead of calling that, we call GetCharWidth, and then we add
                // the difference between GetCharWidth of 'X' and GetTextExtentPoint32W of 'X'
                // While this is wrong, that's what Word does, and if it's
                // good enough for them...
                WCHAR chX = L'X';
                int dxp, dxpX;
                SIZE size;

                fRet = GetTextExtentPoint(hdc, &chX, 1, &size);

                GetCharWidthA(hdc, chX, chX, &dxpX);
                GetCharWidth(hdc, *lpwchStart, *lpwchStart, &dxp);

                xp += size.cx - dxpX + dxp;
            }

            lpwchStart++;
        }

        lpwchT++;
    }

    // output the final run; also, if we were called with cchCount == 0,
    // make a call here to erase the rectangle
    if ((cch = lpwchT - lpwchStart) > 0 || !cchCount)
    {
        fRet = ExtTextOutW(hdc, xp, yp, fuOptions, lprect, lpwchStart, cch,
                            lpDx ? lpDx + (lpwchStart - lpwchString) : NULL);
    }

    return fRet;

}

/*
 *  ReExtTextOut(uiCodePage, hdc, x, y, fuOptions, lprc, lpString, cbCount,lpDx)
 *
 *  @mfunc
 *      Dispatch to ExtTextOut appropriate for the O/S.
 *
 *  @rdesc
 *      Returns whatever ExtTextOut returns
 */
BOOL ReExtTextOut(
    CONVERTMODE cm,     //@parm CM_NONE, CM_WCTMB, CM_LOWBYTE
    UINT uiCodePage,    //@parm code page for text
    HDC hdc,            //@parm handle to device context
    int x,              //@parm x-coordinate of reference point
    int y,              //@parm y-coordinate of reference point
    UINT fuOptions,     //@parm text-output options
    CONST RECT *lprc,   //@parm optional clipping and/or opaquing rectangle
    const WCHAR *lpString,  //@parm points to string
    UINT cbCount,       //@parm number of characters in string
    CONST INT *lpDx,    //@parm Ptr to array of intercharacter spacing values
    BOOL  FEFontOnNonFEWin95)   //@parm TRUE --> use ExtTextOutW
{
    if (!FEFontOnNonFEWin95 && cm != CM_NONE)   // Need to convert and use
    {                                           //  ExtTextOutA
        CTempCharBuf tcb;

        // Double the buffer size
        int cbString = (cm == CM_LOWBYTE) ? cbCount : cbCount * 2;

        // String buffer for converted string - allocate on the stack
        char *psz = tcb.GetBuf(cbString);

        if (NULL == psz)
        {
            // Could not allocate buffer
            return FALSE;
        }

        int cbConv = 0;

        if(cm == CM_WCTMB)
        {
            cbConv = WideCharToMultiByte(uiCodePage, 0, lpString, cbCount,
                psz, cbString, NULL, NULL);

            if(!cbConv)
            {
                // The conversion failed for one reason or another.  We should
                // make every effort to use WCTMB before we fall back to
                // taking the low-byte of every wchar (below), otherwise we
                // risk dropping the high-bytes and displaying garbage.

                // Use the cpg from the font, since the uiCodePage passed is
                //  the requested codepage and the font-mapper may very well
                //  have mapped to a different one.
                TEXTMETRIC tm;

                uiCodePage = (GetTextMetrics(hdc, &tm) &&
                                tm.tmCharSet != DEFAULT_CHARSET &&
                                (UINT)GetCodePage(tm.tmCharSet) != uiCodePage) ?
                                GetCodePage(tm.tmCharSet) : 1252;

                cbConv = WideCharToMultiByte(uiCodePage, 0, lpString, cbCount,
                    psz, cbString, NULL, NULL);
            }
        }
        else
        {
            Assert(cm == CM_LOWBYTE);
            // drop through and convert using only low-bytes of WCHAR's
        }

        // WCTMB failed OR cm == CM_LOWBYTE
        if(!cbConv)                         // Convert WCHARs to CHARs
        {
            // FUTURE:  We come here for both SYMBOL_CHARSET fonts and for
            // DBCS bytes stuffed into wchar's (one byte per wchar) when
            // the requested code page is not installed on the machine and
            // the MBTWC fails. Instead, we could have another conversion
            // mode that collects each DBCS char as a single wchar and then
            // remaps to a DBCS string for ExtTextOutA. This would allow us
            // to display text if the system has the right font even tho it
            // doesn't have the right cpg.

            // If we are converting this WCHAR buffer in this manner
            // (by taking only the low-byte's of the WCHAR's), it is
            // because:
            //  1) cm == CM_LOWBYTE
            //  2) WCTMB above failed for some reason or another.  It may
            //      be the case that the string is entirely ASCII in which
            //      case dropping the high-bytes is not a big deal (otherwise
            //      we assert).

            cbConv = cbCount;

            while(cbCount--)
            {
                AssertSz(lpString[cbCount] <= 0xFF, "ReExtTextOut():  Found "
                            "a WCHAR with non-zero high-byte when using "
                            "CM_LOWBYTE conversion mode.");
                psz[cbCount] = lpString[cbCount];
            }
        }

#ifndef PEGASUS
        return ExtTextOutA(hdc, x, y, fuOptions, lprc, psz, cbConv, lpDx);
#else
        return 0;
#endif
    }

#ifndef MACPORT
#pragma message("Review : Move this to w32sys")
#if 0
    // do we need the Win95 FE bug workaround??
    if (OnWin95FE() || FEFontOnNonFEWin95 )
        return ReExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cbCount, lpDx);
    else
#endif
#endif
        return ExtTextOutW(hdc, x, y, fuOptions, lprc, lpString, cbCount, lpDx);

}

        _bConvertMode = DetermineConvertMode( tm.tmCharSet );

    // Some fonts have problems under Win95 with the GetCharWidthW call; this
    // is a simple heuristic to determine if this problem exists.
        INT     widthA, widthW;
        BOOL    fResA, fResW;

        // Future(BradO):  We should add the expression
        //  "&& IsFELCID(GetSystemDefaultLCID())" to the 'if' below to use
        //  Unicode GetCharWidth and ExtTextOut for FE fonts on non-FE
        //  systems (see postponed bug #3337).

        // Yet another hack - FE font on Non-FE Win95 cannot use
        // GetCharWidthW and ExtTextOutW
        if(IsFECharset (tm.tmCharSet) && OnWin95FE())
        {
            // always use ANSI call for DBC fonts.
            _bConvertMode = CM_WCTMB;

            // setup _xDefDBWidth to by-pass some Trad. Chinese character
            // width problem.
            if (CHINESEBIG5_CHARSET == tm.tmCharSet)
            {
                BYTE    ansiChar[2] = {0xD8, 0xB5 };
                if (fResA = GetCharWidthA( _hdc, *((USHORT *) ansiChar), *((USHORT *) ansiChar), &widthA ) && widthA)
                    _xDefDBCWidth = widthA;
            }
        }
        else
        {
            fResA = GetCharWidthA( _hdc, ' ', ' ', &widthA );
            fResW = GetCharWidthW( _hdc, L' ', L' ', &widthW );
            if ( fResA && fResW && widthA != widthW )
            {
                _bConvertMode = CM_WCTMB;
            }
            else
            {
                fResA = GetCharWidthA( _hdc, 'a', 'a', &widthA );
                fResW = GetCharWidthW( _hdc, L'a', L'a', &widthW );
                if ( fResA && fResW && widthA != widthW )
                {
                    _bConvertMode = CM_WCTMB;
                }
            }
        }

    W32-CalcUnderlineInfo (this);
    ////////// new

    (void) GetCharWidth( hdc, ch, ch, &pWidthData->width, uiCodePage );

    // fAnsi case, SYMBOL_CHARSET, or GetCharWidthW failed: try GetCharWidthA
    if (!fRes || 0 == pWidthData->width)
    {
        WORD wDBCS = ch;
        if(uiCodePage != SYMBOL_CODEPAGE)
        {
            // Try to convert string
            numOfDBCS = WideCharToMultiByte( uiCodePage, 0, &ch, 1,
                ansiChar, 2, NULL, NULL);

            if (2 == numOfDBCS)
                wDBCS = (BYTE)ansiChar[0] << 8 | (BYTE)ansiChar[1];

            else if (numOfDBCS)
                wDBCS = (BYTE)ansiChar[0];
        }
        fRes = GetCharWidthA( hdc, wDBCS, wDBCS, &pWidthData->width );
    }

#endif // MACPORT
