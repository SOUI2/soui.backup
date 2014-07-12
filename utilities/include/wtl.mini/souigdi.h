// Windows Template Library - WTL version 8.1
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// This file is a part of the Windows Template Library.
// The use and distribution terms for this software are covered by the
// Common Public License 1.0 (http://opensource.org/licenses/cpl1.0.php)
// which can be found in the file CPL.TXT at the root of this distribution.
// By using this software in any fashion, you are agreeing to be bound by
// the terms of this license. You must not remove this notice, or
// any other, from this software.

#ifndef __SOUIGDI_H__
#define __SOUIGDI_H__

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
// CPenT<t_bManaged>
// CBrushT<t_bManaged>
// CLogFont
// CFontT<t_bManaged>
// CRgnT<t_bManaged>
// CDCT<t_bManaged>
namespace SOUI
{

///////////////////////////////////////////////////////////////////////////////
// CPen

template <bool t_bManaged>
class CPenT
{
public:
// Data members
    HPEN m_hPen;

// Constructor/destructor/operators
    CPenT(HPEN hPen = NULL) : m_hPen(hPen)
    { }

    ~CPenT()
    {
        if(t_bManaged && m_hPen != NULL)
            DeleteObject();
    }

    CPenT<t_bManaged>& operator =(HPEN hPen)
    {
        Attach(hPen);
        return *this;
    }

    void Attach(HPEN hPen)
    {
        if(t_bManaged && m_hPen != NULL && m_hPen != hPen)
            ::DeleteObject(m_hPen);
        m_hPen = hPen;
    }

    HPEN Detach()
    {
        HPEN hPen = m_hPen;
        m_hPen = NULL;
        return hPen;
    }

    operator HPEN() const
    {
        return m_hPen;
    }

    bool IsNull() const
    {
        return (m_hPen == NULL);
    }

// Create methods
    HPEN CreatePen(int nPenStyle, int nWidth, COLORREF crColor)
    {
        ASSERT(m_hPen == NULL);
        m_hPen = ::CreatePen(nPenStyle, nWidth, crColor);
        return m_hPen;
    }

#ifndef _WIN32_WCE
    HPEN CreatePen(int nPenStyle, int nWidth, const LOGBRUSH* pLogBrush, int nStyleCount = 0, const DWORD* lpStyle = NULL)
    {
        ASSERT(m_hPen == NULL);
        m_hPen = ::ExtCreatePen(nPenStyle, nWidth, pLogBrush, nStyleCount, lpStyle);
        return m_hPen;
    }
#endif // !_WIN32_WCE

    HPEN CreatePenIndirect(LPLOGPEN lpLogPen)
    {
        ASSERT(m_hPen == NULL);
        m_hPen = ::CreatePenIndirect(lpLogPen);
        return m_hPen;
    }

    BOOL DeleteObject()
    {
        ASSERT(m_hPen != NULL);
        BOOL bRet = ::DeleteObject(m_hPen);
        if(bRet)
            m_hPen = NULL;
        return bRet;
    }

// Attributes
    int GetLogPen(LOGPEN* pLogPen) const
    {
        ASSERT(m_hPen != NULL);
        return ::GetObject(m_hPen, sizeof(LOGPEN), pLogPen);
    }

    bool GetLogPen(LOGPEN& LogPen) const
    {
        ASSERT(m_hPen != NULL);
        return (::GetObject(m_hPen, sizeof(LOGPEN), &LogPen) == sizeof(LOGPEN));
    }

#ifndef _WIN32_WCE
    int GetExtLogPen(EXTLOGPEN* pLogPen) const
    {
        ASSERT(m_hPen != NULL);
        return ::GetObject(m_hPen, sizeof(EXTLOGPEN), pLogPen);
    }

    bool GetExtLogPen(EXTLOGPEN& ExtLogPen) const
    {
        ASSERT(m_hPen != NULL);
        return (::GetObject(m_hPen, sizeof(EXTLOGPEN), &ExtLogPen) == sizeof(EXTLOGPEN));
    }
#endif // !_WIN32_WCE
};

typedef CPenT<false>   CPenHandle;
typedef CPenT<true>    CPen;


///////////////////////////////////////////////////////////////////////////////
// CBrush

template <bool t_bManaged>
class CBrushT
{
public:
// Data members
    HBRUSH m_hBrush;

// Constructor/destructor/operators
    CBrushT(HBRUSH hBrush = NULL) : m_hBrush(hBrush)
    { }

    CBrushT(COLORREF cr): m_hBrush(NULL)
    {
        CreateSolidBrush(cr);
    }

    ~CBrushT()
    {
        if(t_bManaged && m_hBrush != NULL)
            DeleteObject();
    }

    CBrushT<t_bManaged>& operator =(HBRUSH hBrush)
    {
        Attach(hBrush);
        return *this;
    }

    void Attach(HBRUSH hBrush)
    {
        if(t_bManaged && m_hBrush != NULL && m_hBrush != hBrush)
            ::DeleteObject(m_hBrush);
        m_hBrush = hBrush;
    }

    HBRUSH Detach()
    {
        HBRUSH hBrush = m_hBrush;
        m_hBrush = NULL;
        return hBrush;
    }

    operator HBRUSH() const
    {
        return m_hBrush;
    }

    bool IsNull() const
    {
        return (m_hBrush == NULL);
    }

// Create methods
    HBRUSH CreateSolidBrush(COLORREF crColor)
    {
        ASSERT(m_hBrush == NULL);
        m_hBrush = ::CreateSolidBrush(crColor);
        return m_hBrush;
    }

#ifndef _WIN32_WCE
    HBRUSH CreateHatchBrush(int nIndex, COLORREF crColor)
    {
        ASSERT(m_hBrush == NULL);
        m_hBrush = ::CreateHatchBrush(nIndex, crColor);
        return m_hBrush;
    }
#endif // !_WIN32_WCE

#if !defined(_WIN32_WCE) || (_ATL_VER >= 0x0800)
    HBRUSH CreateBrushIndirect(const LOGBRUSH* lpLogBrush)
    {
        ASSERT(m_hBrush == NULL);
#ifndef _WIN32_WCE
        m_hBrush = ::CreateBrushIndirect(lpLogBrush);
#else // CE specific
        m_hBrush = ATL::CreateBrushIndirect(lpLogBrush);
#endif // _WIN32_WCE
        return m_hBrush;
    }
#endif // !defined(_WIN32_WCE) || (_ATL_VER >= 0x0800)

    HBRUSH CreatePatternBrush(HBITMAP hBitmap)
    {
        ASSERT(m_hBrush == NULL);
        m_hBrush = ::CreatePatternBrush(hBitmap);
        return m_hBrush;
    }

    HBRUSH CreateDIBPatternBrush(HGLOBAL hPackedDIB, UINT nUsage)
    {
        ASSERT(hPackedDIB != NULL);
        const void* lpPackedDIB = GlobalLock(hPackedDIB);
        ASSERT(lpPackedDIB != NULL);
        m_hBrush = ::CreateDIBPatternBrushPt(lpPackedDIB, nUsage);
        GlobalUnlock(hPackedDIB);
        return m_hBrush;
    }

    HBRUSH CreateDIBPatternBrush(const void* lpPackedDIB, UINT nUsage)
    {
        ASSERT(m_hBrush == NULL);
        m_hBrush = ::CreateDIBPatternBrushPt(lpPackedDIB, nUsage);
        return m_hBrush;
    }

    HBRUSH CreateSysColorBrush(int nIndex)
    {
        ASSERT(m_hBrush == NULL);
        m_hBrush = ::GetSysColorBrush(nIndex);
        return m_hBrush;
    }

    BOOL DeleteObject()
    {
        ASSERT(m_hBrush != NULL);
        BOOL bRet = ::DeleteObject(m_hBrush);
        if(bRet)
            m_hBrush = NULL;
        return bRet;
    }

// Attributes
    int GetLogBrush(LOGBRUSH* pLogBrush) const
    {
        ASSERT(m_hBrush != NULL);
        return ::GetObject(m_hBrush, sizeof(LOGBRUSH), pLogBrush);
    }

    bool GetLogBrush(LOGBRUSH& LogBrush) const
    {
        ASSERT(m_hBrush != NULL);
        return (::GetObject(m_hBrush, sizeof(LOGBRUSH), &LogBrush) == sizeof(LOGBRUSH));
    }
};

typedef CBrushT<false>   CBrushHandle;
typedef CBrushT<true>    CBrush;


///////////////////////////////////////////////////////////////////////////////
// CFont

class CLogFont : public LOGFONT
{
public:
    CLogFont()
    {
        memset(this, 0, sizeof(LOGFONT));
    }

    CLogFont(const LOGFONT& lf)
    {
        Copy(&lf);
    }

    CLogFont(HFONT hFont)
    {
        ASSERT(::GetObjectType(hFont) == OBJ_FONT);
        ::GetObject(hFont, sizeof(LOGFONT), (LOGFONT*)this);
    }

    HFONT CreateFontIndirect()
    {
        return ::CreateFontIndirect(this);
    }

    void SetBold()
    {
        lfWeight = FW_BOLD;
    }

    bool IsBold() const
    {
        return (lfWeight >= FW_BOLD);
    }

    void MakeBolder(int iScale = 1)
    {
        lfWeight += FW_BOLD * iScale;
    }

    void MakeLarger(int iScale)
    {
        if(lfHeight > 0)
            lfHeight += iScale;
        else
            lfHeight -= iScale;
    }

    void SetHeight(LONG nPointSize, HDC hDC = NULL)
    {
        HDC hDC1 = (hDC != NULL) ? hDC : ::GetDC(NULL);
        // For MM_TEXT mapping mode
        lfHeight = -::MulDiv(nPointSize, ::GetDeviceCaps(hDC1, LOGPIXELSY), 72);
        if(hDC == NULL)
            ::ReleaseDC(NULL, hDC1);
    }

    LONG GetHeight(HDC hDC = NULL) const
    {
        HDC hDC1 = (hDC != NULL) ? hDC : ::GetDC(NULL);
        // For MM_TEXT mapping mode
        LONG nPointSize = ::MulDiv(-lfHeight, 72, ::GetDeviceCaps(hDC1, LOGPIXELSY));
        if(hDC == NULL)
            ::ReleaseDC(NULL, hDC1);

        return nPointSize;
    }

    LONG GetDeciPointHeight(HDC hDC = NULL) const
    {
        HDC hDC1 = (hDC != NULL) ? hDC : ::GetDC(NULL);
#ifndef _WIN32_WCE
        POINT ptOrg = { 0, 0 };
        ::DPtoLP(hDC1, &ptOrg, 1);
        POINT pt = { 0, 0 };
        pt.y = abs(lfHeight) + ptOrg.y;
        ::LPtoDP(hDC1, &pt,1);
        LONG nDeciPoint = ::MulDiv(pt.y, 720, ::GetDeviceCaps(hDC1, LOGPIXELSY));   // 72 points/inch, 10 decipoints/point
#else // CE specific
        // DP and LP are always the same on CE
        LONG nDeciPoint = ::MulDiv(abs(lfHeight), 720, ::GetDeviceCaps(hDC1, LOGPIXELSY));   // 72 points/inch, 10 decipoints/point
#endif // _WIN32_WCE
        if(hDC == NULL)
            ::ReleaseDC(NULL, hDC1);

        return nDeciPoint;
    }

    void SetHeightFromDeciPoint(LONG nDeciPtHeight, HDC hDC = NULL)
    {
        HDC hDC1 = (hDC != NULL) ? hDC : ::GetDC(NULL);
#ifndef _WIN32_WCE
        POINT pt = { 0, 0 };
        pt.y = ::MulDiv(::GetDeviceCaps(hDC1, LOGPIXELSY), nDeciPtHeight, 720);   // 72 points/inch, 10 decipoints/point
        ::DPtoLP(hDC1, &pt, 1);
        POINT ptOrg = { 0, 0 };
        ::DPtoLP(hDC1, &ptOrg, 1);
        lfHeight = -abs(pt.y - ptOrg.y);
#else // CE specific
        // DP and LP are always the same on CE
        lfHeight = -abs(::MulDiv(::GetDeviceCaps(hDC1, LOGPIXELSY), nDeciPtHeight, 720));   // 72 points/inch, 10 decipoints/point
#endif // _WIN32_WCE
        if(hDC == NULL)
            ::ReleaseDC(NULL, hDC1);
    }

    void Copy(const LOGFONT* pLogFont)
    {
        ASSERT(pLogFont != NULL);
        *(LOGFONT*)this = *pLogFont;
    }

    CLogFont& operator =(const CLogFont& src)
    {
        Copy(&src);
        return *this;
    }

    CLogFont& operator =(const LOGFONT& src)
    {
        Copy(&src);
        return *this;
    }

    CLogFont& operator =(HFONT hFont)
    {
        ASSERT(::GetObjectType(hFont) == OBJ_FONT);
        ::GetObject(hFont, sizeof(LOGFONT), (LOGFONT*)this);
        return *this;
    }

    bool operator ==(const LOGFONT& logfont) const
    {
        return(logfont.lfHeight == lfHeight &&
               logfont.lfWidth == lfWidth &&
               logfont.lfEscapement == lfEscapement &&
               logfont.lfOrientation == lfOrientation &&
               logfont.lfWeight == lfWeight &&
               logfont.lfItalic == lfItalic &&
               logfont.lfUnderline == lfUnderline &&
               logfont.lfStrikeOut == lfStrikeOut &&
               logfont.lfCharSet == lfCharSet &&
               logfont.lfOutPrecision == lfOutPrecision &&
               logfont.lfClipPrecision == lfClipPrecision &&
               logfont.lfQuality == lfQuality &&
               logfont.lfPitchAndFamily == lfPitchAndFamily &&
               lstrcmp(logfont.lfFaceName, lfFaceName) == 0);
    }
};


template <bool t_bManaged>
class CFontT
{
public:
// Data members
    HFONT m_hFont;

// Constructor/destructor/operators
    CFontT(HFONT hFont = NULL) : m_hFont(hFont)
    { }

    ~CFontT()
    {
        if(t_bManaged && m_hFont != NULL)
            DeleteObject();
    }

    CFontT<t_bManaged>& operator =(HFONT hFont)
    {
        Attach(hFont);
        return *this;
    }

    void Attach(HFONT hFont)
    {
        if(t_bManaged && m_hFont != NULL && m_hFont != hFont)
            ::DeleteObject(m_hFont);
        m_hFont = hFont;
    }

    HFONT Detach()
    {
        HFONT hFont = m_hFont;
        m_hFont = NULL;
        return hFont;
    }

    operator HFONT() const
    {
        return m_hFont;
    }

    bool IsNull() const
    {
        return (m_hFont == NULL);
    }

// Create methods
    HFONT CreateFontIndirect(const LOGFONT* lpLogFont)
    {
        ASSERT(m_hFont == NULL);
        m_hFont = ::CreateFontIndirect(lpLogFont);
        return m_hFont;
    }

#if !defined(_WIN32_WCE) && (_WIN32_WINNT >= 0x0500)
    HFONT CreateFontIndirectEx(CONST ENUMLOGFONTEXDV* penumlfex)
    {
        ASSERT(m_hFont == NULL);
        m_hFont = ::CreateFontIndirectEx(penumlfex);
        return m_hFont;
    }
#endif // !defined(_WIN32_WCE) && (_WIN32_WINNT >= 0x0500)

#if !defined(_WIN32_WCE) || (_ATL_VER >= 0x0800)
    HFONT CreateFont(int nHeight, int nWidth, int nEscapement,
                     int nOrientation, int nWeight, BYTE bItalic, BYTE bUnderline,
                     BYTE cStrikeOut, BYTE nCharSet, BYTE nOutPrecision,
                     BYTE nClipPrecision, BYTE nQuality, BYTE nPitchAndFamily,
                     LPCTSTR lpszFacename)
    {
        ASSERT(m_hFont == NULL);
#ifndef _WIN32_WCE
        m_hFont = ::CreateFont(nHeight, nWidth, nEscapement,
                               nOrientation, nWeight, bItalic, bUnderline, cStrikeOut,
                               nCharSet, nOutPrecision, nClipPrecision, nQuality,
                               nPitchAndFamily, lpszFacename);
#else // CE specific
        m_hFont = ATL::CreateFont(nHeight, nWidth, nEscapement,
                                  nOrientation, nWeight, bItalic, bUnderline, cStrikeOut,
                                  nCharSet, nOutPrecision, nClipPrecision, nQuality,
                                  nPitchAndFamily, lpszFacename);
#endif // _WIN32_WCE
        return m_hFont;
    }
#endif // !defined(_WIN32_WCE) || (_ATL_VER >= 0x0800)

    HFONT CreatePointFont(int nPointSize, LPCTSTR lpszFaceName, HDC hDC = NULL, bool bBold = false, bool bItalic = false)
    {
        LOGFONT logFont = { 0 };
        logFont.lfCharSet = DEFAULT_CHARSET;
        logFont.lfHeight = nPointSize;
        _tcscpy(logFont.lfFaceName, lpszFaceName);

        if(bBold)
            logFont.lfWeight = FW_BOLD;
        if(bItalic)
            logFont.lfItalic = (BYTE)TRUE;

        return CreatePointFontIndirect(&logFont, hDC);
    }

    HFONT CreatePointFontIndirect(const LOGFONT* lpLogFont, HDC hDC = NULL)
    {
        HDC hDC1 = (hDC != NULL) ? hDC : ::GetDC(NULL);

        // convert nPointSize to logical units based on hDC
        LOGFONT logFont = *lpLogFont;
#ifndef _WIN32_WCE
        POINT pt = { 0, 0 };
        pt.y = ::MulDiv(::GetDeviceCaps(hDC1, LOGPIXELSY), logFont.lfHeight, 720);   // 72 points/inch, 10 decipoints/point
        ::DPtoLP(hDC1, &pt, 1);
        POINT ptOrg = { 0, 0 };
        ::DPtoLP(hDC1, &ptOrg, 1);
        logFont.lfHeight = -abs(pt.y - ptOrg.y);
#else // CE specific
        // DP and LP are always the same on CE
        logFont.lfHeight = -abs(::MulDiv(::GetDeviceCaps(hDC1, LOGPIXELSY), logFont.lfHeight, 720));   // 72 points/inch, 10 decipoints/point
#endif // _WIN32_WCE

        if(hDC == NULL)
            ::ReleaseDC(NULL, hDC1);

        return CreateFontIndirect(&logFont);
    }

    BOOL DeleteObject()
    {
        ASSERT(m_hFont != NULL);
        BOOL bRet = ::DeleteObject(m_hFont);
        if(bRet)
            m_hFont = NULL;
        return bRet;
    }

// Attributes
    int GetLogFont(LOGFONT* pLogFont) const
    {
        ASSERT(m_hFont != NULL);
        return ::GetObject(m_hFont, sizeof(LOGFONT), pLogFont);
    }

    bool GetLogFont(LOGFONT& LogFont) const
    {
        ASSERT(m_hFont != NULL);
        return (::GetObject(m_hFont, sizeof(LOGFONT), &LogFont) == sizeof(LOGFONT));
    }
};

typedef CFontT<false>   CFontHandle;
typedef CFontT<true>    CFont;


///////////////////////////////////////////////////////////////////////////////
// CBitmap

template <bool t_bManaged>
class CBitmapT
{
public:
// Data members
    HBITMAP m_hBitmap;

// Constructor/destructor/operators
    CBitmapT(HBITMAP hBitmap = NULL) : m_hBitmap(hBitmap)
    { }

    ~CBitmapT()
    {
        if(t_bManaged && m_hBitmap != NULL)
            DeleteObject();
    }

    CBitmapT<t_bManaged>& operator =(HBITMAP hBitmap)
    {
        Attach(hBitmap);
        return *this;
    }

    void Attach(HBITMAP hBitmap)
    {
        if(t_bManaged && m_hBitmap != NULL&& m_hBitmap != hBitmap)
            ::DeleteObject(m_hBitmap);
        m_hBitmap = hBitmap;
    }

    HBITMAP Detach()
    {
        HBITMAP hBitmap = m_hBitmap;
        m_hBitmap = NULL;
        return hBitmap;
    }

    operator HBITMAP() const
    {
        return m_hBitmap;
    }

    bool IsNull() const
    {
        return (m_hBitmap == NULL);
    }


    HBITMAP LoadOEMBitmap(UINT nIDBitmap) // for OBM_/OCR_/OIC_
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(nIDBitmap));
        return m_hBitmap;
    }

    HBITMAP CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitsPerPixel, const void* lpBits)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateBitmap(nWidth, nHeight, nPlanes, nBitsPerPixel, lpBits);
        return m_hBitmap;
    }

#ifndef _WIN32_WCE
    HBITMAP CreateBitmapIndirect(LPBITMAP lpBitmap)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateBitmapIndirect(lpBitmap);
        return m_hBitmap;
    }
#endif // !_WIN32_WCE

    HBITMAP CreateCompatibleBitmap(HDC hDC, int nWidth, int nHeight)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
        return m_hBitmap;
    }

#ifndef _WIN32_WCE
    HBITMAP CreateDiscardableBitmap(HDC hDC, int nWidth, int nHeight)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateDiscardableBitmap(hDC, nWidth, nHeight);
        return m_hBitmap;
    }
#endif // !_WIN32_WCE

    BOOL DeleteObject()
    {
        ASSERT(m_hBitmap != NULL);
        BOOL bRet = ::DeleteObject(m_hBitmap);
        if(bRet)
            m_hBitmap = NULL;
        return bRet;
    }

// Attributes
    int GetBitmap(BITMAP* pBitMap) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetObject(m_hBitmap, sizeof(BITMAP), pBitMap);
    }

    bool GetBitmap(BITMAP& bm) const
    {
        ASSERT(m_hBitmap != NULL);
        return (::GetObject(m_hBitmap, sizeof(BITMAP), &bm) == sizeof(BITMAP));
    }

    bool GetSize(SIZE& size) const
    {
        ASSERT(m_hBitmap != NULL);
        BITMAP bm = { 0 };
        if(!GetBitmap(&bm))
            return false;
        size.cx = bm.bmWidth;
        size.cy = bm.bmHeight;
        return true;
    }

#ifndef _WIN32_WCE
    DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetBitmapBits(m_hBitmap, dwCount, lpBits);
    }
#endif // !_WIN32_WCE

#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 410)
    DWORD SetBitmapBits(DWORD dwCount, const void* lpBits)
    {
        ASSERT(m_hBitmap != NULL);
        return ::SetBitmapBits(m_hBitmap, dwCount, lpBits);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 410)

#ifndef _WIN32_WCE
    BOOL GetBitmapDimension(LPSIZE lpSize) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetBitmapDimensionEx(m_hBitmap, lpSize);
    }

    BOOL SetBitmapDimension(int nWidth, int nHeight, LPSIZE lpSize = NULL)
    {
        ASSERT(m_hBitmap != NULL);
        return ::SetBitmapDimensionEx(m_hBitmap, nWidth, nHeight, lpSize);
    }

// DIB support
    HBITMAP CreateDIBitmap(HDC hDC, CONST BITMAPINFOHEADER* lpbmih, DWORD dwInit, CONST VOID* lpbInit, CONST BITMAPINFO* lpbmi, UINT uColorUse)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateDIBitmap(hDC, lpbmih, dwInit, lpbInit, lpbmi, uColorUse);
        return m_hBitmap;
    }
#endif // !_WIN32_WCE

    HBITMAP CreateDIBSection(HDC hDC, CONST BITMAPINFO* lpbmi, UINT uColorUse, VOID** ppvBits, HANDLE hSection, DWORD dwOffset)
    {
        ASSERT(m_hBitmap == NULL);
        m_hBitmap = ::CreateDIBSection(hDC, lpbmi, uColorUse, ppvBits, hSection, dwOffset);
        return m_hBitmap;
    }

#ifndef _WIN32_WCE
    int GetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines,  LPVOID lpvBits, LPBITMAPINFO lpbmi, UINT uColorUse) const
    {
        ASSERT(m_hBitmap != NULL);
        return ::GetDIBits(hDC, m_hBitmap, uStartScan, cScanLines,  lpvBits, lpbmi, uColorUse);
    }

    int SetDIBits(HDC hDC, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
    {
        ASSERT(m_hBitmap != NULL);
        return ::SetDIBits(hDC, m_hBitmap, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
    }
#endif // !_WIN32_WCE
};

typedef CBitmapT<false>   CBitmapHandle;
typedef CBitmapT<true>    CBitmap;


///////////////////////////////////////////////////////////////////////////////
// CPalette

template <bool t_bManaged>
class CPaletteT
{
public:
// Data members
    HPALETTE m_hPalette;

// Constructor/destructor/operators
    CPaletteT(HPALETTE hPalette = NULL) : m_hPalette(hPalette)
    { }

    ~CPaletteT()
    {
        if(t_bManaged && m_hPalette != NULL)
            DeleteObject();
    }

    CPaletteT<t_bManaged>& operator =(HPALETTE hPalette)
    {
        Attach(hPalette);
        return *this;
    }

    void Attach(HPALETTE hPalette)
    {
        if(t_bManaged && m_hPalette != NULL && m_hPalette != hPalette)
            ::DeleteObject(m_hPalette);
        m_hPalette = hPalette;
    }

    HPALETTE Detach()
    {
        HPALETTE hPalette = m_hPalette;
        m_hPalette = NULL;
        return hPalette;
    }

    operator HPALETTE() const
    {
        return m_hPalette;
    }

    bool IsNull() const
    {
        return (m_hPalette == NULL);
    }

// Create methods
    HPALETTE CreatePalette(LPLOGPALETTE lpLogPalette)
    {
        ASSERT(m_hPalette == NULL);
        m_hPalette = ::CreatePalette(lpLogPalette);
        return m_hPalette;
    }

#ifndef _WIN32_WCE
    HPALETTE CreateHalftonePalette(HDC hDC)
    {
        ASSERT(m_hPalette == NULL);
        ASSERT(hDC != NULL);
        m_hPalette = ::CreateHalftonePalette(hDC);
        return m_hPalette;
    }
#endif // !_WIN32_WCE

    BOOL DeleteObject()
    {
        ASSERT(m_hPalette != NULL);
        BOOL bRet = ::DeleteObject(m_hPalette);
        if(bRet)
            m_hPalette = NULL;
        return bRet;
    }

// Attributes
    int GetEntryCount() const
    {
        ASSERT(m_hPalette != NULL);
        WORD nEntries = 0;
        ::GetObject(m_hPalette, sizeof(WORD), &nEntries);
        return (int)nEntries;
    }

    UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const
    {
        ASSERT(m_hPalette != NULL);
        return ::GetPaletteEntries(m_hPalette, nStartIndex, nNumEntries, lpPaletteColors);
    }

    UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors)
    {
        ASSERT(m_hPalette != NULL);
        return ::SetPaletteEntries(m_hPalette, nStartIndex, nNumEntries, lpPaletteColors);
    }

// Operations
#ifndef _WIN32_WCE
    void AnimatePalette(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors)
    {
        ASSERT(m_hPalette != NULL);
        ::AnimatePalette(m_hPalette, nStartIndex, nNumEntries, lpPaletteColors);
    }

    BOOL ResizePalette(UINT nNumEntries)
    {
        ASSERT(m_hPalette != NULL);
        return ::ResizePalette(m_hPalette, nNumEntries);
    }
#endif // !_WIN32_WCE

    UINT GetNearestPaletteIndex(COLORREF crColor) const
    {
        ASSERT(m_hPalette != NULL);
        return ::GetNearestPaletteIndex(m_hPalette, crColor);
    }
};

typedef CPaletteT<false>   CPaletteHandle;
typedef CPaletteT<true>    CPalette;


///////////////////////////////////////////////////////////////////////////////
// CRgn

template <bool t_bManaged>
class CRgnT
{
public:
// Data members
    HRGN m_hRgn;

// Constructor/destructor/operators
    CRgnT(HRGN hRgn = NULL) : m_hRgn(hRgn)
    { }

    ~CRgnT()
    {
        if(t_bManaged && m_hRgn != NULL)
            DeleteObject();
    }

    CRgnT<t_bManaged>& operator =(HRGN hRgn)
    {
        Attach(hRgn);
        return *this;
    }

    void Attach(HRGN hRgn)
    {
        if(t_bManaged && m_hRgn != NULL && m_hRgn != hRgn)
            ::DeleteObject(m_hRgn);
        m_hRgn = hRgn;
    }

    HRGN Detach()
    {
        HRGN hRgn = m_hRgn;
        m_hRgn = NULL;
        return hRgn;
    }

    operator HRGN() const
    {
        return m_hRgn;
    }

    bool IsNull() const
    {
        return (m_hRgn == NULL);
    }

// Create methods
    HRGN CreateRectRgn(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreateRectRgn(x1, y1, x2, y2);
        return m_hRgn;
    }

    HRGN CreateRectRgnIndirect(LPCRECT lpRect)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreateRectRgnIndirect(lpRect);
        return m_hRgn;
    }

#ifndef _WIN32_WCE
    HRGN CreateEllipticRgn(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreateEllipticRgn(x1, y1, x2, y2);
        return m_hRgn;
    }

    HRGN CreateEllipticRgnIndirect(LPCRECT lpRect)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreateEllipticRgnIndirect(lpRect);
        return m_hRgn;
    }

    HRGN CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreatePolygonRgn(lpPoints, nCount, nMode);
        return m_hRgn;
    }

    HRGN CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount, int nPolyFillMode)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreatePolyPolygonRgn(lpPoints, lpPolyCounts, nCount, nPolyFillMode);
        return m_hRgn;
    }

    HRGN CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::CreateRoundRectRgn(x1, y1, x2, y2, x3, y3);
        return m_hRgn;
    }

    HRGN CreateFromPath(HDC hDC)
    {
        ASSERT(m_hRgn == NULL);
        ASSERT(hDC != NULL);
        m_hRgn = ::PathToRegion(hDC);
        return m_hRgn;
    }

    HRGN CreateFromData(const XFORM* lpXForm, int nCount, const RGNDATA* pRgnData)
    {
        ASSERT(m_hRgn == NULL);
        m_hRgn = ::ExtCreateRegion(lpXForm, nCount, pRgnData);
        return m_hRgn;
    }
#endif // !_WIN32_WCE

    BOOL DeleteObject()
    {
        ASSERT(m_hRgn != NULL);
        BOOL bRet = ::DeleteObject(m_hRgn);
        if(bRet)
            m_hRgn = NULL;
        return bRet;
    }

// Operations
    void SetRectRgn(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hRgn != NULL);
        ::SetRectRgn(m_hRgn, x1, y1, x2, y2);
    }

    void SetRectRgn(LPCRECT lpRect)
    {
        ASSERT(m_hRgn != NULL);
        ::SetRectRgn(m_hRgn, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }

    int CombineRgn(HRGN hRgnSrc1, HRGN hRgnSrc2, int nCombineMode)
    {
        ASSERT(m_hRgn != NULL);
        return ::CombineRgn(m_hRgn, hRgnSrc1, hRgnSrc2, nCombineMode);
    }

    int CombineRgn(HRGN hRgnSrc, int nCombineMode)
    {
        ASSERT(m_hRgn != NULL);
        return ::CombineRgn(m_hRgn, m_hRgn, hRgnSrc, nCombineMode);
    }

    int CopyRgn(HRGN hRgnSrc)
    {
        ASSERT(m_hRgn != NULL);
        return ::CombineRgn(m_hRgn, hRgnSrc, NULL, RGN_COPY);
    }

    BOOL EqualRgn(HRGN hRgn) const
    {
        ASSERT(m_hRgn != NULL);
        return ::EqualRgn(m_hRgn, hRgn);
    }

    int OffsetRgn(int x, int y)
    {
        ASSERT(m_hRgn != NULL);
        return ::OffsetRgn(m_hRgn, x, y);
    }

    int OffsetRgn(POINT point)
    {
        ASSERT(m_hRgn != NULL);
        return ::OffsetRgn(m_hRgn, point.x, point.y);
    }

    int GetRgnBox(LPRECT lpRect) const
    {
        ASSERT(m_hRgn != NULL);
        return ::GetRgnBox(m_hRgn, lpRect);
    }

    BOOL PtInRegion(int x, int y) const
    {
        ASSERT(m_hRgn != NULL);
        return ::PtInRegion(m_hRgn, x, y);
    }

    BOOL PtInRegion(POINT point) const
    {
        ASSERT(m_hRgn != NULL);
        return ::PtInRegion(m_hRgn, point.x, point.y);
    }

    BOOL RectInRegion(LPCRECT lpRect) const
    {
        ASSERT(m_hRgn != NULL);
        return ::RectInRegion(m_hRgn, lpRect);
    }

    int GetRegionData(LPRGNDATA lpRgnData, int nDataSize) const
    {
        ASSERT(m_hRgn != NULL);
        return (int)::GetRegionData(m_hRgn, nDataSize, lpRgnData);
    }
};

typedef CRgnT<false>   CRgnHandle;
typedef CRgnT<true>    CRgn;


///////////////////////////////////////////////////////////////////////////////
// CDC - The device context class

template <bool t_bManaged>
class CDCT
{
public:
// Data members
    HDC m_hDC;

// Constructor/destructor/operators
    CDCT(HDC hDC = NULL) : m_hDC(hDC)
    {
    }

    ~CDCT()
    {
        if(t_bManaged && m_hDC != NULL)
            ::DeleteDC(Detach());
    }

    CDCT<t_bManaged>& operator =(HDC hDC)
    {
        Attach(hDC);
        return *this;
    }

    void Attach(HDC hDC)
    {
        if(t_bManaged && m_hDC != NULL && m_hDC != hDC)
            ::DeleteDC(m_hDC);
        m_hDC = hDC;
    }

    HDC Detach()
    {
        HDC hDC = m_hDC;
        m_hDC = NULL;
        return hDC;
    }

    operator HDC() const
    {
        return m_hDC;
    }

    bool IsNull() const
    {
        return (m_hDC == NULL);
    }

// Operations
#ifndef _WIN32_WCE
    HWND WindowFromDC() const
    {
        ASSERT(m_hDC != NULL);
        return ::WindowFromDC(m_hDC);
    }
#endif // !_WIN32_WCE

    CPenHandle GetCurrentPen() const
    {
        ASSERT(m_hDC != NULL);
        return CPenHandle((HPEN)::GetCurrentObject(m_hDC, OBJ_PEN));
    }

    CBrushHandle GetCurrentBrush() const
    {
        ASSERT(m_hDC != NULL);
        return CBrushHandle((HBRUSH)::GetCurrentObject(m_hDC, OBJ_BRUSH));
    }

    CPaletteHandle GetCurrentPalette() const
    {
        ASSERT(m_hDC != NULL);
        return CPaletteHandle((HPALETTE)::GetCurrentObject(m_hDC, OBJ_PAL));
    }

    CFontHandle GetCurrentFont() const
    {
        ASSERT(m_hDC != NULL);
        return CFontHandle((HFONT)::GetCurrentObject(m_hDC, OBJ_FONT));
    }

    CBitmapHandle GetCurrentBitmap() const
    {
        ASSERT(m_hDC != NULL);
        return CBitmapHandle((HBITMAP)::GetCurrentObject(m_hDC, OBJ_BITMAP));
    }

    HDC CreateDC(LPCTSTR lpszDriverName, LPCTSTR lpszDeviceName, LPCTSTR lpszOutput, const DEVMODE* lpInitData)
    {
        ASSERT(m_hDC == NULL);
        m_hDC = ::CreateDC(lpszDriverName, lpszDeviceName, lpszOutput, lpInitData);
        return m_hDC;
    }

    HDC CreateCompatibleDC(HDC hDC = NULL)
    {
        ASSERT(m_hDC == NULL);
        m_hDC = ::CreateCompatibleDC(hDC);
        return m_hDC;
    }

    BOOL DeleteDC()
    {
        if(m_hDC == NULL)
            return FALSE;
        BOOL bRet = ::DeleteDC(m_hDC);
        if(bRet)
            m_hDC = NULL;
        return bRet;
    }

// Device-Context Functions
    int SaveDC()
    {
        ASSERT(m_hDC != NULL);
        return ::SaveDC(m_hDC);
    }

    BOOL RestoreDC(int nSavedDC)
    {
        ASSERT(m_hDC != NULL);
        return ::RestoreDC(m_hDC, nSavedDC);
    }

    int GetDeviceCaps(int nIndex) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetDeviceCaps(m_hDC, nIndex);
    }

#ifndef _WIN32_WCE
    UINT SetBoundsRect(LPCRECT lpRectBounds, UINT flags)
    {
        ASSERT(m_hDC != NULL);
        return ::SetBoundsRect(m_hDC, lpRectBounds, flags);
    }

    UINT GetBoundsRect(LPRECT lpRectBounds, UINT flags) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetBoundsRect(m_hDC, lpRectBounds, flags);
    }

    BOOL ResetDC(const DEVMODE* lpDevMode)
    {
        ASSERT(m_hDC != NULL);
        return ::ResetDC(m_hDC, lpDevMode) != NULL;
    }

// Drawing-Tool Functions
    BOOL GetBrushOrg(LPPOINT lpPoint) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetBrushOrgEx(m_hDC, lpPoint);
    }
#endif // !_WIN32_WCE

    BOOL SetBrushOrg(int x, int y, LPPOINT lpPoint = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::SetBrushOrgEx(m_hDC, x, y, lpPoint);
    }

    BOOL SetBrushOrg(POINT point, LPPOINT lpPointRet = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::SetBrushOrgEx(m_hDC, point.x, point.y, lpPointRet);
    }

#ifndef _WIN32_WCE
    int EnumObjects(int nObjectType, int (CALLBACK* lpfn)(LPVOID, LPARAM), LPARAM lpData)
    {
        ASSERT(m_hDC != NULL);
#ifdef STRICT
        return ::EnumObjects(m_hDC, nObjectType, (GOBJENUMPROC)lpfn, lpData);
#else
        return ::EnumObjects(m_hDC, nObjectType, (GOBJENUMPROC)lpfn, (LPVOID)lpData);
#endif
    }
#endif // !_WIN32_WCE

// Type-safe selection helpers
    HPEN SelectPen(HPEN hPen)
    {
        ASSERT(m_hDC != NULL);
#ifndef _WIN32_WCE
        ASSERT(hPen == NULL || ::GetObjectType(hPen) == OBJ_PEN || ::GetObjectType(hPen) == OBJ_EXTPEN);
#else // CE specific
        ASSERT(hPen == NULL || ::GetObjectType(hPen) == OBJ_PEN);
#endif // _WIN32_WCE
        return (HPEN)::SelectObject(m_hDC, hPen);
    }

    HBRUSH SelectBrush(HBRUSH hBrush)
    {
        ASSERT(m_hDC != NULL);
        ASSERT(hBrush == NULL || ::GetObjectType(hBrush) == OBJ_BRUSH);
        return (HBRUSH)::SelectObject(m_hDC, hBrush);
    }

    HFONT SelectFont(HFONT hFont)
    {
        ASSERT(m_hDC != NULL);
        ASSERT(hFont == NULL || ::GetObjectType(hFont) == OBJ_FONT);
        return (HFONT)::SelectObject(m_hDC, hFont);
    }

    HBITMAP SelectBitmap(HBITMAP hBitmap)
    {
        ASSERT(m_hDC != NULL);
        ASSERT(hBitmap == NULL || ::GetObjectType(hBitmap) == OBJ_BITMAP);
        return (HBITMAP)::SelectObject(m_hDC, hBitmap);
    }

    int SelectRgn(HRGN hRgn)       // special return for regions
    {
        ASSERT(m_hDC != NULL);
        ASSERT(hRgn == NULL || ::GetObjectType(hRgn) == OBJ_REGION);
        return PtrToInt(::SelectObject(m_hDC, hRgn));
    }

// Type-safe selection helpers for stock objects
    HPEN SelectStockPen(int nPen)
    {
        ASSERT(m_hDC != NULL);
#if (_WIN32_WINNT >= 0x0500)
        ASSERT(nPen == WHITE_PEN || nPen == BLACK_PEN || nPen == NULL_PEN || nPen == DC_PEN);
#else
        ASSERT(nPen == WHITE_PEN || nPen == BLACK_PEN || nPen == NULL_PEN);
#endif // !(_WIN32_WINNT >= 0x0500)
        return SelectPen((HPEN)::GetStockObject(nPen));
    }

    HBRUSH SelectStockBrush(int nBrush)
    {
#if (_WIN32_WINNT >= 0x0500)
        ASSERT((nBrush >= WHITE_BRUSH && nBrush <= HOLLOW_BRUSH) || nBrush == DC_BRUSH);
#else
        ASSERT(nBrush >= WHITE_BRUSH && nBrush <= HOLLOW_BRUSH);
#endif // !(_WIN32_WINNT >= 0x0500)
        return SelectBrush((HBRUSH)::GetStockObject(nBrush));
    }

    HFONT SelectStockFont(int nFont)
    {
#ifndef _WIN32_WCE
        ASSERT((nFont >= OEM_FIXED_FONT && nFont <= SYSTEM_FIXED_FONT) || nFont == DEFAULT_GUI_FONT);
#else // CE specific
        ASSERT(nFont == SYSTEM_FONT);
#endif // _WIN32_WCE
        return SelectFont((HFONT)::GetStockObject(nFont));
    }

    HPALETTE SelectStockPalette(int nPalette, BOOL bForceBackground)
    {
        ASSERT(nPalette == DEFAULT_PALETTE); // the only one supported
        return SelectPalette((HPALETTE)::GetStockObject(nPalette), bForceBackground);
    }

// Color and Color Palette Functions
    COLORREF GetNearestColor(COLORREF crColor) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetNearestColor(m_hDC, crColor);
    }

    HPALETTE SelectPalette(HPALETTE hPalette, BOOL bForceBackground)
    {
        ASSERT(m_hDC != NULL);

        return ::SelectPalette(m_hDC, hPalette, bForceBackground);
    }

    UINT RealizePalette()
    {
        ASSERT(m_hDC != NULL);
        return ::RealizePalette(m_hDC);
    }

#ifndef _WIN32_WCE
    void UpdateColors()
    {
        ASSERT(m_hDC != NULL);
        ::UpdateColors(m_hDC);
    }
#endif // !_WIN32_WCE

// Drawing-Attribute Functions
    COLORREF GetBkColor() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetBkColor(m_hDC);
    }

    int GetBkMode() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetBkMode(m_hDC);
    }

#ifndef _WIN32_WCE
    int GetPolyFillMode() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetPolyFillMode(m_hDC);
    }

    int GetROP2() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetROP2(m_hDC);
    }

    int GetStretchBltMode() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetStretchBltMode(m_hDC);
    }
#endif // !_WIN32_WCE

    COLORREF GetTextColor() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextColor(m_hDC);
    }

    COLORREF SetBkColor(COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::SetBkColor(m_hDC, crColor);
    }

    int SetBkMode(int nBkMode)
    {
        ASSERT(m_hDC != NULL);
        return ::SetBkMode(m_hDC, nBkMode);
    }

#ifndef _WIN32_WCE
    int SetPolyFillMode(int nPolyFillMode)
    {
        ASSERT(m_hDC != NULL);
        return ::SetPolyFillMode(m_hDC, nPolyFillMode);
    }
#endif // !_WIN32_WCE

    int SetROP2(int nDrawMode)
    {
        ASSERT(m_hDC != NULL);
        return ::SetROP2(m_hDC, nDrawMode);
    }

#ifndef _WIN32_WCE
    int SetStretchBltMode(int nStretchMode)
    {
        ASSERT(m_hDC != NULL);
        return ::SetStretchBltMode(m_hDC, nStretchMode);
    }
#endif // !_WIN32_WCE

    COLORREF SetTextColor(COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::SetTextColor(m_hDC, crColor);
    }

#ifndef _WIN32_WCE
    BOOL GetColorAdjustment(LPCOLORADJUSTMENT lpColorAdjust) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetColorAdjustment(m_hDC, lpColorAdjust);
    }

    BOOL SetColorAdjustment(const COLORADJUSTMENT* lpColorAdjust)
    {
        ASSERT(m_hDC != NULL);
        return ::SetColorAdjustment(m_hDC, lpColorAdjust);
    }

// Mapping Functions
    int GetMapMode() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetMapMode(m_hDC);
    }

    BOOL GetViewportOrg(LPPOINT lpPoint) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetViewportOrgEx(m_hDC, lpPoint);
    }

    int SetMapMode(int nMapMode)
    {
        ASSERT(m_hDC != NULL);
        return ::SetMapMode(m_hDC, nMapMode);
    }
#endif // !_WIN32_WCE

    // Viewport Origin
    BOOL SetViewportOrg(int x, int y, LPPOINT lpPoint = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::SetViewportOrgEx(m_hDC, x, y, lpPoint);
    }

    BOOL SetViewportOrg(POINT point, LPPOINT lpPointRet = NULL)
    {
        ASSERT(m_hDC != NULL);
        return SetViewportOrg(point.x, point.y, lpPointRet);
    }

#ifndef _WIN32_WCE
    BOOL OffsetViewportOrg(int nWidth, int nHeight, LPPOINT lpPoint = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::OffsetViewportOrgEx(m_hDC, nWidth, nHeight, lpPoint);
    }

    // Viewport Extent
    BOOL GetViewportExt(LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetViewportExtEx(m_hDC, lpSize);
    }

    BOOL SetViewportExt(int x, int y, LPSIZE lpSize = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::SetViewportExtEx(m_hDC, x, y, lpSize);
    }

    BOOL SetViewportExt(SIZE size, LPSIZE lpSizeRet = NULL)
    {
        ASSERT(m_hDC != NULL);
        return SetViewportExt(size.cx, size.cy, lpSizeRet);
    }

    BOOL ScaleViewportExt(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::ScaleViewportExtEx(m_hDC, xNum, xDenom, yNum, yDenom, lpSize);
    }
#endif // !_WIN32_WCE

    // Window Origin
#ifndef _WIN32_WCE
    BOOL GetWindowOrg(LPPOINT lpPoint) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetWindowOrgEx(m_hDC, lpPoint);
    }

    BOOL SetWindowOrg(int x, int y, LPPOINT lpPoint = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::SetWindowOrgEx(m_hDC, x, y, lpPoint);
    }

    BOOL SetWindowOrg(POINT point, LPPOINT lpPointRet = NULL)
    {
        ASSERT(m_hDC != NULL);
        return SetWindowOrg(point.x, point.y, lpPointRet);
    }

    BOOL OffsetWindowOrg(int nWidth, int nHeight, LPPOINT lpPoint = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::OffsetWindowOrgEx(m_hDC, nWidth, nHeight, lpPoint);
    }

    // Window extent
    BOOL GetWindowExt(LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetWindowExtEx(m_hDC, lpSize);
    }

    BOOL SetWindowExt(int x, int y, LPSIZE lpSize = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::SetWindowExtEx(m_hDC, x, y, lpSize);
    }

    BOOL SetWindowExt(SIZE size, LPSIZE lpSizeRet = NULL)
    {
        ASSERT(m_hDC != NULL);
        return SetWindowExt(size.cx, size.cy, lpSizeRet);
    }

    BOOL ScaleWindowExt(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::ScaleWindowExtEx(m_hDC, xNum, xDenom, yNum, yDenom, lpSize);
    }

// Coordinate Functions
    BOOL DPtoLP(LPPOINT lpPoints, int nCount = 1) const
    {
        ASSERT(m_hDC != NULL);
        return ::DPtoLP(m_hDC, lpPoints, nCount);
    }

    BOOL DPtoLP(LPRECT lpRect) const
    {
        ASSERT(m_hDC != NULL);
        return ::DPtoLP(m_hDC, (LPPOINT)lpRect, 2);
    }

    BOOL DPtoLP(LPSIZE lpSize) const
    {
        SIZE sizeWinExt = { 0, 0 };
        if(!GetWindowExt(&sizeWinExt))
            return FALSE;
        SIZE sizeVpExt = { 0, 0 };
        if(!GetViewportExt(&sizeVpExt))
            return FALSE;
        lpSize->cx = ::MulDiv(lpSize->cx, abs(sizeWinExt.cx), abs(sizeVpExt.cx));
        lpSize->cy = ::MulDiv(lpSize->cy, abs(sizeWinExt.cy), abs(sizeVpExt.cy));
        return TRUE;
    }

    BOOL LPtoDP(LPPOINT lpPoints, int nCount = 1) const
    {
        ASSERT(m_hDC != NULL);
        return ::LPtoDP(m_hDC, lpPoints, nCount);
    }

    BOOL LPtoDP(LPRECT lpRect) const
    {
        ASSERT(m_hDC != NULL);
        return ::LPtoDP(m_hDC, (LPPOINT)lpRect, 2);
    }

    BOOL LPtoDP(LPSIZE lpSize) const
    {
        SIZE sizeWinExt = { 0, 0 };
        if(!GetWindowExt(&sizeWinExt))
            return FALSE;
        SIZE sizeVpExt = { 0, 0 };
        if(!GetViewportExt(&sizeVpExt))
            return FALSE;
        lpSize->cx = ::MulDiv(lpSize->cx, abs(sizeVpExt.cx), abs(sizeWinExt.cx));
        lpSize->cy = ::MulDiv(lpSize->cy, abs(sizeVpExt.cy), abs(sizeWinExt.cy));
        return TRUE;
    }

// Special Coordinate Functions (useful for dealing with metafiles and OLE)
#define HIMETRIC_INCH   2540    // HIMETRIC units per inch

    void DPtoHIMETRIC(LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        int nMapMode;
        if((nMapMode = GetMapMode()) < MM_ISOTROPIC && nMapMode != MM_TEXT)
        {
            // when using a constrained map mode, map against physical inch
            ((CDCHandle*)this)->SetMapMode(MM_HIMETRIC);
            DPtoLP(lpSize);
            ((CDCHandle*)this)->SetMapMode(nMapMode);
        }
        else
        {
            // map against logical inch for non-constrained mapping modes
            int cxPerInch = GetDeviceCaps(LOGPIXELSX);
            int cyPerInch = GetDeviceCaps(LOGPIXELSY);
            ASSERT(cxPerInch != 0 && cyPerInch != 0);
            lpSize->cx = ::MulDiv(lpSize->cx, HIMETRIC_INCH, cxPerInch);
            lpSize->cy = ::MulDiv(lpSize->cy, HIMETRIC_INCH, cyPerInch);
        }
    }

    void HIMETRICtoDP(LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        int nMapMode;
        if((nMapMode = GetMapMode()) < MM_ISOTROPIC && nMapMode != MM_TEXT)
        {
            // when using a constrained map mode, map against physical inch
            ((CDCHandle*)this)->SetMapMode(MM_HIMETRIC);
            LPtoDP(lpSize);
            ((CDCHandle*)this)->SetMapMode(nMapMode);
        }
        else
        {
            // map against logical inch for non-constrained mapping modes
            int cxPerInch = GetDeviceCaps(LOGPIXELSX);
            int cyPerInch = GetDeviceCaps(LOGPIXELSY);
            ASSERT(cxPerInch != 0 && cyPerInch != 0);
            lpSize->cx = ::MulDiv(lpSize->cx, cxPerInch, HIMETRIC_INCH);
            lpSize->cy = ::MulDiv(lpSize->cy, cyPerInch, HIMETRIC_INCH);
        }
    }

    void LPtoHIMETRIC(LPSIZE lpSize) const
    {
        LPtoDP(lpSize);
        DPtoHIMETRIC(lpSize);
    }

    void HIMETRICtoLP(LPSIZE lpSize) const
    {
        HIMETRICtoDP(lpSize);
        DPtoLP(lpSize);
    }
#endif // !_WIN32_WCE

// Region Functions
    BOOL FillRgn(HRGN hRgn, HBRUSH hBrush)
    {
        ASSERT(m_hDC != NULL);
        return ::FillRgn(m_hDC, hRgn, hBrush);
    }

#ifndef _WIN32_WCE
    BOOL FrameRgn(HRGN hRgn, HBRUSH hBrush, int nWidth, int nHeight)
    {
        ASSERT(m_hDC != NULL);
        return ::FrameRgn(m_hDC, hRgn, hBrush, nWidth, nHeight);
    }

    BOOL InvertRgn(HRGN hRgn)
    {
        ASSERT(m_hDC != NULL);
        return ::InvertRgn(m_hDC, hRgn);
    }

    BOOL PaintRgn(HRGN hRgn)
    {
        ASSERT(m_hDC != NULL);
        return ::PaintRgn(m_hDC, hRgn);
    }
#endif // !_WIN32_WCE

// Clipping Functions
    int GetClipBox(LPRECT lpRect) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetClipBox(m_hDC, lpRect);
    }

    int GetClipRgn(CRgn& region) const
    {
        ASSERT(m_hDC != NULL);
        if(region.IsNull())
            region.CreateRectRgn(0, 0, 0, 0);

        int nRet = ::GetClipRgn(m_hDC, region);
        if(nRet != 1)
            region.DeleteObject();

        return nRet;
    }

#ifndef _WIN32_WCE
    BOOL PtVisible(int x, int y) const
    {
        ASSERT(m_hDC != NULL);
        return ::PtVisible(m_hDC, x, y);
    }

    BOOL PtVisible(POINT point) const
    {
        ASSERT(m_hDC != NULL);
        return ::PtVisible(m_hDC, point.x, point.y);
    }
#endif // !_WIN32_WCE

    BOOL RectVisible(LPCRECT lpRect) const
    {
        ASSERT(m_hDC != NULL);
        return ::RectVisible(m_hDC, lpRect);
    }

    int SelectClipRgn(HRGN hRgn)
    {
        ASSERT(m_hDC != NULL);
        return ::SelectClipRgn(m_hDC, (HRGN)hRgn);
    }

    int ExcludeClipRect(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hDC != NULL);
        return ::ExcludeClipRect(m_hDC, x1, y1, x2, y2);
    }

    int ExcludeClipRect(LPCRECT lpRect)
    {
        ASSERT(m_hDC != NULL);
        return ::ExcludeClipRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }

#ifndef _WIN32_WCE
    int ExcludeUpdateRgn(HWND hWnd)
    {
        ASSERT(m_hDC != NULL);
        return ::ExcludeUpdateRgn(m_hDC, hWnd);
    }
#endif // !_WIN32_WCE

    int IntersectClipRect(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hDC != NULL);
        return ::IntersectClipRect(m_hDC, x1, y1, x2, y2);
    }

    int IntersectClipRect(LPCRECT lpRect)
    {
        ASSERT(m_hDC != NULL);
        return ::IntersectClipRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }

#ifndef _WIN32_WCE
    int OffsetClipRgn(int x, int y)
    {
        ASSERT(m_hDC != NULL);
        return ::OffsetClipRgn(m_hDC, x, y);
    }

    int OffsetClipRgn(SIZE size)
    {
        ASSERT(m_hDC != NULL);
        return ::OffsetClipRgn(m_hDC, size.cx, size.cy);
    }

    int SelectClipRgn(HRGN hRgn, int nMode)
    {
        ASSERT(m_hDC != NULL);
        return ::ExtSelectClipRgn(m_hDC, hRgn, nMode);
    }
#endif // !_WIN32_WCE

// Line-Output Functions
#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)
    BOOL GetCurrentPosition(LPPOINT lpPoint) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCurrentPositionEx(m_hDC, lpPoint);
    }

    BOOL MoveTo(int x, int y, LPPOINT lpPoint = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::MoveToEx(m_hDC, x, y, lpPoint);
    }

    BOOL MoveTo(POINT point, LPPOINT lpPointRet = NULL)
    {
        ASSERT(m_hDC != NULL);
        return MoveTo(point.x, point.y, lpPointRet);
    }

    BOOL LineTo(int x, int y)
    {
        ASSERT(m_hDC != NULL);
        return ::LineTo(m_hDC, x, y);
    }

    BOOL LineTo(POINT point)
    {
        ASSERT(m_hDC != NULL);
        return LineTo(point.x, point.y);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)

#ifndef _WIN32_WCE
    BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
    {
        ASSERT(m_hDC != NULL);
        return ::Arc(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    BOOL Arc(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
    {
        ASSERT(m_hDC != NULL);
        return ::Arc(m_hDC, lpRect->left, lpRect->top,
                     lpRect->right, lpRect->bottom, ptStart.x, ptStart.y,
                     ptEnd.x, ptEnd.y);
    }
#endif // !_WIN32_WCE

    BOOL Polyline(LPPOINT lpPoints, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::Polyline(m_hDC, lpPoints, nCount);
    }

#ifndef _WIN32_WCE
    BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle)
    {
        ASSERT(m_hDC != NULL);
        return ::AngleArc(m_hDC, x, y, nRadius, fStartAngle, fSweepAngle);
    }

    BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
    {
        ASSERT(m_hDC != NULL);
        return ::ArcTo(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    BOOL ArcTo(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
    {
        ASSERT(m_hDC != NULL);
        return ArcTo(lpRect->left, lpRect->top, lpRect->right,
                     lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
    }

    int GetArcDirection() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetArcDirection(m_hDC);
    }

    int SetArcDirection(int nArcDirection)
    {
        ASSERT(m_hDC != NULL);
        return ::SetArcDirection(m_hDC, nArcDirection);
    }

    BOOL PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::PolyDraw(m_hDC, lpPoints, lpTypes, nCount);
    }

    BOOL PolylineTo(const POINT* lpPoints, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::PolylineTo(m_hDC, lpPoints, nCount);
    }

    BOOL PolyPolyline(const POINT* lpPoints,
                      const DWORD* lpPolyPoints, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::PolyPolyline(m_hDC, lpPoints, lpPolyPoints, nCount);
    }

    BOOL PolyBezier(const POINT* lpPoints, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::PolyBezier(m_hDC, lpPoints, nCount);
    }

    BOOL PolyBezierTo(const POINT* lpPoints, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::PolyBezierTo(m_hDC, lpPoints, nCount);
    }
#endif // !_WIN32_WCE

// Simple Drawing Functions
    BOOL FillRect(LPCRECT lpRect, HBRUSH hBrush)
    {
        ASSERT(m_hDC != NULL);
        return ::FillRect(m_hDC, lpRect, hBrush);
    }

    BOOL FillRect(LPCRECT lpRect, int nColorIndex)
    {
        ASSERT(m_hDC != NULL);
#ifndef _WIN32_WCE
        return ::FillRect(m_hDC, lpRect, (HBRUSH)LongToPtr(nColorIndex + 1));
#else // CE specific
        return ::FillRect(m_hDC, lpRect, ::GetSysColorBrush(nColorIndex));
#endif // _WIN32_WCE
    }

#ifndef _WIN32_WCE
    BOOL FrameRect(LPCRECT lpRect, HBRUSH hBrush)
    {
        ASSERT(m_hDC != NULL);
        return ::FrameRect(m_hDC, lpRect, hBrush);
    }
#endif // !_WIN32_WCE

#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 420)
    BOOL InvertRect(LPCRECT lpRect)
    {
        ASSERT(m_hDC != NULL);
        return ::InvertRect(m_hDC, lpRect);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 420)

    BOOL DrawIcon(int x, int y, HICON hIcon)
    {
        ASSERT(m_hDC != NULL);
#ifndef _WIN32_WCE
        return ::DrawIcon(m_hDC, x, y, hIcon);
#else // CE specific
        return ::DrawIconEx(m_hDC, x, y, hIcon, 0, 0, 0, NULL, DI_NORMAL);
#endif // _WIN32_WCE
    }

    BOOL DrawIcon(POINT point, HICON hIcon)
    {
        ASSERT(m_hDC != NULL);
#ifndef _WIN32_WCE
        return ::DrawIcon(m_hDC, point.x, point.y, hIcon);
#else // CE specific
        return ::DrawIconEx(m_hDC, point.x, point.y, hIcon, 0, 0, 0, NULL, DI_NORMAL);
#endif // _WIN32_WCE
    }

    BOOL DrawIconEx(int x, int y, HICON hIcon, int cxWidth, int cyWidth, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawIconEx(m_hDC, x, y, hIcon, cxWidth, cyWidth, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
    }

    BOOL DrawIconEx(POINT point, HICON hIcon, SIZE size, UINT uStepIfAniCur = 0, HBRUSH hbrFlickerFreeDraw = NULL, UINT uFlags = DI_NORMAL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawIconEx(m_hDC, point.x, point.y, hIcon, size.cx, size.cy, uStepIfAniCur, hbrFlickerFreeDraw, uFlags);
    }

#ifndef _WIN32_WCE
    BOOL DrawState(POINT pt, SIZE size, HBITMAP hBitmap, UINT nFlags, HBRUSH hBrush = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)hBitmap, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_BITMAP);
    }

    BOOL DrawState(POINT pt, SIZE size, HICON hIcon, UINT nFlags, HBRUSH hBrush = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)hIcon, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_ICON);
    }

    BOOL DrawState(POINT pt, SIZE size, LPCTSTR lpszText, UINT nFlags, BOOL bPrefixText = TRUE, int nTextLen = 0, HBRUSH hBrush = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawState(m_hDC, hBrush, NULL, (LPARAM)lpszText, (WPARAM)nTextLen, pt.x, pt.y, size.cx, size.cy, nFlags | (bPrefixText ? DST_PREFIXTEXT : DST_TEXT));
    }

    BOOL DrawState(POINT pt, SIZE size, DRAWSTATEPROC lpDrawProc, LPARAM lData, UINT nFlags, HBRUSH hBrush = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawState(m_hDC, hBrush, lpDrawProc, lData, 0, pt.x, pt.y, size.cx, size.cy, nFlags | DST_COMPLEX);
    }
#endif // !_WIN32_WCE

// Ellipse and Polygon Functions
#ifndef _WIN32_WCE
    BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
    {
        ASSERT(m_hDC != NULL);
        return ::Chord(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    BOOL Chord(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
    {
        ASSERT(m_hDC != NULL);
        return ::Chord(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
    }
#endif // !_WIN32_WCE

    void DrawFocusRect(LPCRECT lpRect)
    {
        ASSERT(m_hDC != NULL);
        ::DrawFocusRect(m_hDC, lpRect);
    }

    BOOL Ellipse(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hDC != NULL);
        return ::Ellipse(m_hDC, x1, y1, x2, y2);
    }

    BOOL Ellipse(LPCRECT lpRect)
    {
        ASSERT(m_hDC != NULL);
        return ::Ellipse(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }

#ifndef _WIN32_WCE
    BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
    {
        ASSERT(m_hDC != NULL);
        return ::Pie(m_hDC, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    BOOL Pie(LPCRECT lpRect, POINT ptStart, POINT ptEnd)
    {
        ASSERT(m_hDC != NULL);
        return ::Pie(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
    }
#endif // !_WIN32_WCE

    BOOL Polygon(LPPOINT lpPoints, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::Polygon(m_hDC, lpPoints, nCount);
    }

#ifndef _WIN32_WCE
    BOOL PolyPolygon(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount)
    {
        ASSERT(m_hDC != NULL);
        return ::PolyPolygon(m_hDC, lpPoints, lpPolyCounts, nCount);
    }
#endif // !_WIN32_WCE

    BOOL Rectangle(int x1, int y1, int x2, int y2)
    {
        ASSERT(m_hDC != NULL);
        return ::Rectangle(m_hDC, x1, y1, x2, y2);
    }

    BOOL Rectangle(LPCRECT lpRect)
    {
        ASSERT(m_hDC != NULL);
        return ::Rectangle(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
    }

    BOOL RoundRect(int x1, int y1, int x2, int y2, int x3, int y3)
    {
        ASSERT(m_hDC != NULL);
        return ::RoundRect(m_hDC, x1, y1, x2, y2, x3, y3);
    }

    BOOL RoundRect(LPCRECT lpRect, POINT point)
    {
        ASSERT(m_hDC != NULL);
        return ::RoundRect(m_hDC, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, point.x, point.y);
    }

// Bitmap Functions
    BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop)
    {
        ASSERT(m_hDC != NULL);
        return ::PatBlt(m_hDC, x, y, nWidth, nHeight, dwRop);
    }

    BOOL BitBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC,
                int xSrc, int ySrc, DWORD dwRop)
    {
        ASSERT(m_hDC != NULL);
        return ::BitBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, dwRop);
    }

    BOOL StretchBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop)
    {
        ASSERT(m_hDC != NULL);
        return ::StretchBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
    }

    COLORREF GetPixel(int x, int y) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetPixel(m_hDC, x, y);
    }

    COLORREF GetPixel(POINT point) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetPixel(m_hDC, point.x, point.y);
    }

    COLORREF SetPixel(int x, int y, COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::SetPixel(m_hDC, x, y, crColor);
    }

    COLORREF SetPixel(POINT point, COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::SetPixel(m_hDC, point.x, point.y, crColor);
    }

#ifndef _WIN32_WCE
    BOOL FloodFill(int x, int y, COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::FloodFill(m_hDC, x, y, crColor);
    }

    BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType)
    {
        ASSERT(m_hDC != NULL);
        return ::ExtFloodFill(m_hDC, x, y, crColor, nFillType);
    }
#endif // !_WIN32_WCE

    BOOL MaskBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, HBITMAP hMaskBitmap, int xMask, int yMask, DWORD dwRop)
    {
        ASSERT(m_hDC != NULL);
        return ::MaskBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, hMaskBitmap, xMask, yMask, dwRop);
    }

#ifndef _WIN32_WCE
    BOOL PlgBlt(LPPOINT lpPoint, HDC hSrcDC, int xSrc, int ySrc, int nWidth, int nHeight, HBITMAP hMaskBitmap, int xMask, int yMask)
    {
        ASSERT(m_hDC != NULL);
        return ::PlgBlt(m_hDC, lpPoint, hSrcDC, xSrc, ySrc, nWidth, nHeight, hMaskBitmap, xMask, yMask);
    }

    BOOL SetPixelV(int x, int y, COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::SetPixelV(m_hDC, x, y, crColor);
    }

    BOOL SetPixelV(POINT point, COLORREF crColor)
    {
        ASSERT(m_hDC != NULL);
        return ::SetPixelV(m_hDC, point.x, point.y, crColor);
    }
#endif // !_WIN32_WCE

#if !defined(_ATL_NO_MSIMG) || defined(_WIN32_WCE)
#ifndef _WIN32_WCE
    BOOL TransparentBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, UINT crTransparent)
    {
        ASSERT(m_hDC != NULL);
        return ::TransparentBlt(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, crTransparent);
    }
#else // CE specific
    BOOL TransparentImage(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, UINT crTransparent)
    {
        ASSERT(m_hDC != NULL);
        return ::TransparentImage(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, crTransparent);
    }
#endif // _WIN32_WCE

#if (!defined(_WIN32_WCE) || (_WIN32_WCE >= 420))
    BOOL GradientFill(const PTRIVERTEX pVertices, DWORD nVertices, void* pMeshElements, DWORD nMeshElements, DWORD dwMode)
    {
        ASSERT(m_hDC != NULL);
        return ::GradientFill(m_hDC, pVertices, nVertices, pMeshElements, nMeshElements, dwMode);
    }

    BOOL GradientFillRect(RECT& rect, COLORREF clr1, COLORREF clr2, bool bHorizontal)
    {
        ASSERT(m_hDC != NULL);

        TRIVERTEX arrTvx[2] = { { 0 }, { 0 } };

        arrTvx[0].x = rect.left;
        arrTvx[0].y = rect.top;
        arrTvx[0].Red = MAKEWORD(0, GetRValue(clr1));
        arrTvx[0].Green = MAKEWORD(0, GetGValue(clr1));
        arrTvx[0].Blue = MAKEWORD(0, GetBValue(clr1));
        arrTvx[0].Alpha = 0;

        arrTvx[1].x = rect.right;
        arrTvx[1].y = rect.bottom;
        arrTvx[1].Red = MAKEWORD(0, GetRValue(clr2));
        arrTvx[1].Green = MAKEWORD(0, GetGValue(clr2));
        arrTvx[1].Blue = MAKEWORD(0, GetBValue(clr2));
        arrTvx[1].Alpha = 0;

        GRADIENT_RECT gr = { 0, 1 };

        return ::GradientFill(m_hDC, arrTvx, 2, &gr, 1, bHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 420)

#if !defined(_WIN32_WCE) || (_WIN32_WCE > 0x500)
    BOOL AlphaBlend(int x, int y, int nWidth, int nHeight, HDC hSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, BLENDFUNCTION bf)
    {
        ASSERT(m_hDC != NULL);
        return ::AlphaBlend(m_hDC, x, y, nWidth, nHeight, hSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, bf);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE > 0x500)
#endif //  !defined(_ATL_NO_MSIMG) || defined(_WIN32_WCE)

// Extra bitmap functions
    // Helper function for painting a disabled toolbar or menu bitmap
    // This function can take either an HBITMAP (for SS) or a DC with
    //           the bitmap already painted (for cmdbar)
    BOOL DitherBlt(int x, int y, int nWidth, int nHeight, HDC hSrcDC, HBITMAP hBitmap, int xSrc, int ySrc,
                   HBRUSH hBrushBackground = ::GetSysColorBrush(COLOR_3DFACE),
                   HBRUSH hBrush3DEffect = ::GetSysColorBrush(COLOR_3DHILIGHT),
                   HBRUSH hBrushDisabledImage = ::GetSysColorBrush(COLOR_3DSHADOW))
    {
        ASSERT(m_hDC != NULL || hBitmap != NULL);
        ASSERT(nWidth > 0 && nHeight > 0);

        // Create a generic DC for all BitBlts
        CDCHandle dc = (hSrcDC != NULL) ? hSrcDC : ::CreateCompatibleDC(m_hDC);
        ASSERT(dc.m_hDC != NULL);
        if(dc.m_hDC == NULL)
            return FALSE;

        // Create a DC for the monochrome DIB section
        CDC dcBW = ::CreateCompatibleDC(m_hDC);
        ASSERT(dcBW.m_hDC != NULL);
        if(dcBW.m_hDC == NULL)
        {
            if(hSrcDC == NULL)
                dc.DeleteDC();
            return FALSE;
        }

        // Create the monochrome DIB section with a black and white palette
        struct RGBBWBITMAPINFO
        {
            BITMAPINFOHEADER bmiHeader;
            RGBQUAD bmiColors[2];
        };

        RGBBWBITMAPINFO rgbBWBitmapInfo =
        {
            { sizeof(BITMAPINFOHEADER), nWidth, nHeight, 1, 1, BI_RGB, 0, 0, 0, 0, 0 },
            { { 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 } }
        };

        VOID* pbitsBW;
        CBitmap bmpBW = ::CreateDIBSection(dcBW, (LPBITMAPINFO)&rgbBWBitmapInfo, DIB_RGB_COLORS, &pbitsBW, NULL, 0);
        ASSERT(bmpBW.m_hBitmap != NULL);
        if(bmpBW.m_hBitmap == NULL)
        {
            if(hSrcDC == NULL)
                dc.DeleteDC();
            return FALSE;
        }

        // Attach the monochrome DIB section and the bitmap to the DCs
        HBITMAP hbmOldBW = dcBW.SelectBitmap(bmpBW);
        HBITMAP hbmOldDC = NULL;
        if(hBitmap != NULL)
            hbmOldDC = dc.SelectBitmap(hBitmap);

        // Block: Dark gray removal: we want (128, 128, 128) pixels to become black and not white
        {
            CDC dcTemp1 = ::CreateCompatibleDC(m_hDC);
            CDC dcTemp2 = ::CreateCompatibleDC(m_hDC);
            CBitmap bmpTemp1;
            bmpTemp1.CreateCompatibleBitmap(dc, nWidth, nHeight);
            CBitmap bmpTemp2;
            bmpTemp2.CreateBitmap(nWidth, nHeight, 1, 1, NULL);
            HBITMAP hOldBmp1 = dcTemp1.SelectBitmap(bmpTemp1);
            HBITMAP hOldBmp2 = dcTemp2.SelectBitmap(bmpTemp2);
            // Let's copy our image, it will be altered
            dcTemp1.BitBlt(0, 0, nWidth, nHeight, dc, xSrc, ySrc, SRCCOPY);

            // All dark gray pixels will become white, the others black
            dcTemp1.SetBkColor(RGB(128, 128, 128));
            dcTemp2.BitBlt(0, 0, nWidth, nHeight, dcTemp1, 0, 0, SRCCOPY);
            // Do an XOR to set to black these white pixels
            dcTemp1.BitBlt(0, 0, nWidth, nHeight, dcTemp2, 0, 0, SRCINVERT);

            // BitBlt the bitmap into the monochrome DIB section
            // The DIB section will do a true monochrome conversion
            // The magenta background being closer to white will become white
            dcBW.BitBlt(0, 0, nWidth, nHeight, dcTemp1, 0, 0, SRCCOPY);

            // Cleanup
            dcTemp1.SelectBitmap(hOldBmp1);
            dcTemp2.SelectBitmap(hOldBmp2);
        }

        // Paint the destination rectangle using hBrushBackground
        if(hBrushBackground != NULL)
        {
            RECT rc = { x, y, x + nWidth, y + nHeight };
            FillRect(&rc, hBrushBackground);
        }

        // BitBlt the black bits in the monochrome bitmap into hBrush3DEffect color in the destination DC
        // The magic ROP comes from the Charles Petzold's book
        HBRUSH hOldBrush = SelectBrush(hBrush3DEffect);
        BitBlt(x + 1, y + 1, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

        // BitBlt the black bits in the monochrome bitmap into hBrushDisabledImage color in the destination DC
        SelectBrush(hBrushDisabledImage);
        BitBlt(x, y, nWidth, nHeight, dcBW, 0, 0, 0xB8074A);

        SelectBrush(hOldBrush);
        dcBW.SelectBitmap(hbmOldBW);
        dc.SelectBitmap(hbmOldDC);

        if(hSrcDC == NULL)
            dc.DeleteDC();

        return TRUE;
    }

// Text Functions
#ifndef _WIN32_WCE
    BOOL TextOut(int x, int y, LPCTSTR lpszString, int nCount = -1)
    {
        ASSERT(m_hDC != NULL);
        if(nCount == -1)
            nCount = lstrlen(lpszString);
        return ::TextOut(m_hDC, x, y, lpszString, nCount);
    }
#endif // !_WIN32_WCE

    BOOL ExtTextOut(int x, int y, UINT nOptions, LPCRECT lpRect, LPCTSTR lpszString, UINT nCount = -1, LPINT lpDxWidths = NULL)
    {
        ASSERT(m_hDC != NULL);
        if(nCount == -1)
            nCount = lstrlen(lpszString);
        return ::ExtTextOut(m_hDC, x, y, nOptions, lpRect, lpszString, nCount, lpDxWidths);
    }

#ifndef _WIN32_WCE
    SIZE TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount = -1, int nTabPositions = 0, LPINT lpnTabStopPositions = NULL, int nTabOrigin = 0)
    {
        ASSERT(m_hDC != NULL);
        if(nCount == -1)
            nCount = lstrlen(lpszString);
        LONG lRes = ::TabbedTextOut(m_hDC, x, y, lpszString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin);
        SIZE size = { GET_X_LPARAM(lRes), GET_Y_LPARAM(lRes) };
        return size;
    }
#endif // !_WIN32_WCE

    int DrawText(LPCTSTR lpstrText, int cchText, LPRECT lpRect, UINT uFormat)
    {
        ASSERT(m_hDC != NULL);
#ifndef _WIN32_WCE
        ASSERT((uFormat & DT_MODIFYSTRING) == 0);
#endif // !_WIN32_WCE
        return ::DrawText(m_hDC, lpstrText, cchText, lpRect, uFormat);
    }

    int DrawText(LPTSTR lpstrText, int cchText, LPRECT lpRect, UINT uFormat)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawText(m_hDC, lpstrText, cchText, lpRect, uFormat);
    }

#ifndef _WIN32_WCE
    int DrawTextEx(LPTSTR lpstrText, int cchText, LPRECT lpRect, UINT uFormat, LPDRAWTEXTPARAMS lpDTParams = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawTextEx(m_hDC, lpstrText, cchText, lpRect, uFormat, lpDTParams);
    }
#endif // !_WIN32_WCE

#if (_WIN32_WINNT >= 0x0501)
    int DrawShadowText(LPCWSTR lpstrText, int cchText, LPRECT lpRect, DWORD dwFlags, COLORREF clrText, COLORREF clrShadow, int xOffset, int yOffset)
    {
        ASSERT(m_hDC != NULL);
        // This function is present only if comctl32.dll version 6 is loaded;
        // we use LoadLibrary/GetProcAddress to allow apps compiled with
        // _WIN32_WINNT >= 0x0501 to run on older Windows/CommCtrl
        int nRet = 0;
        HMODULE hCommCtrlDLL = ::LoadLibrary(_T("comctl32.dll"));
        ASSERT(hCommCtrlDLL != NULL);
        if(hCommCtrlDLL != NULL)
        {
            typedef int (WINAPI *PFN_DrawShadowText)(HDC hDC, LPCWSTR lpstrText, UINT cchText, LPRECT lpRect, DWORD dwFlags, COLORREF clrText, COLORREF clrShadow, int xOffset, int yOffset);
            PFN_DrawShadowText pfnDrawShadowText = (PFN_DrawShadowText)::GetProcAddress(hCommCtrlDLL, "DrawShadowText");
            ASSERT(pfnDrawShadowText != NULL);   // this function requires CommCtrl6
            if(pfnDrawShadowText != NULL)
                nRet = pfnDrawShadowText(m_hDC, lpstrText, cchText, lpRect, dwFlags, clrText, clrShadow, xOffset, yOffset);
            ::FreeLibrary(hCommCtrlDLL);
        }
        return nRet;
    }
#endif // (_WIN32_WINNT >= 0x0501)

    BOOL GetTextExtent(LPCTSTR lpszString, int nCount, LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        if(nCount == -1)
            nCount = lstrlen(lpszString);
        return ::GetTextExtentPoint32(m_hDC, lpszString, nCount, lpSize);
    }

    BOOL GetTextExtentExPoint(LPCTSTR lpszString, int cchString, LPSIZE lpSize, int nMaxExtent, LPINT lpnFit = NULL, LPINT alpDx = NULL)
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextExtentExPoint(m_hDC, lpszString, cchString, nMaxExtent, lpnFit, alpDx, lpSize);
    }

#ifndef _WIN32_WCE
    DWORD GetTabbedTextExtent(LPCTSTR lpszString, int nCount = -1, int nTabPositions = 0, LPINT lpnTabStopPositions = NULL) const
    {
        ASSERT(m_hDC != NULL);
        if(nCount == -1)
            nCount = lstrlen(lpszString);
        return ::GetTabbedTextExtent(m_hDC, lpszString, nCount, nTabPositions, lpnTabStopPositions);
    }

    BOOL GrayString(HBRUSH hBrush, BOOL (CALLBACK* lpfnOutput)(HDC, LPARAM, int), LPARAM lpData, int nCount, int x, int y, int nWidth, int nHeight)
    {
        ASSERT(m_hDC != NULL);
        return ::GrayString(m_hDC, hBrush, (GRAYSTRINGPROC)lpfnOutput, lpData, nCount, x, y, nWidth, nHeight);
    }
#endif // !_WIN32_WCE

#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)
    UINT GetTextAlign() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextAlign(m_hDC);
    }

    UINT SetTextAlign(UINT nFlags)
    {
        ASSERT(m_hDC != NULL);
        return ::SetTextAlign(m_hDC, nFlags);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)

    int GetTextFace(LPTSTR lpszFacename, int nCount) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextFace(m_hDC, nCount, lpszFacename);
    }

    int GetTextFaceLen() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextFace(m_hDC, 0, NULL);
    }


    BOOL GetTextMetrics(LPTEXTMETRIC lpMetrics) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextMetrics(m_hDC, lpMetrics);
    }

#ifndef _WIN32_WCE
    int SetTextJustification(int nBreakExtra, int nBreakCount)
    {
        ASSERT(m_hDC != NULL);
        return ::SetTextJustification(m_hDC, nBreakExtra, nBreakCount);
    }

    int GetTextCharacterExtra() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextCharacterExtra(m_hDC);
    }

    int SetTextCharacterExtra(int nCharExtra)
    {
        ASSERT(m_hDC != NULL);
        return ::SetTextCharacterExtra(m_hDC, nCharExtra);
    }
#endif // !_WIN32_WCE

// Advanced Drawing
    BOOL DrawEdge(LPRECT lpRect, UINT nEdge, UINT nFlags)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawEdge(m_hDC, lpRect, nEdge, nFlags);
    }

    BOOL DrawFrameControl(LPRECT lpRect, UINT nType, UINT nState)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawFrameControl(m_hDC, lpRect, nType, nState);
    }

// Scrolling Functions
    BOOL ScrollDC(int dx, int dy, LPCRECT lpRectScroll, LPCRECT lpRectClip, HRGN hRgnUpdate, LPRECT lpRectUpdate)
    {
        ASSERT(m_hDC != NULL);
        return ::ScrollDC(m_hDC, dx, dy, lpRectScroll, lpRectClip, hRgnUpdate, lpRectUpdate);
    }

// Font Functions
#ifndef _WIN32_WCE
    BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharWidth(m_hDC, nFirstChar, nLastChar, lpBuffer);
    }

    // GetCharWidth32 is not supported under Win9x
    BOOL GetCharWidth32(UINT nFirstChar, UINT nLastChar, LPINT lpBuffer) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharWidth32(m_hDC, nFirstChar, nLastChar, lpBuffer);
    }

    DWORD SetMapperFlags(DWORD dwFlag)
    {
        ASSERT(m_hDC != NULL);
        return ::SetMapperFlags(m_hDC, dwFlag);
    }

    BOOL GetAspectRatioFilter(LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetAspectRatioFilterEx(m_hDC, lpSize);
    }

    BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABC lpabc) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharABCWidths(m_hDC, nFirstChar, nLastChar, lpabc);
    }

    DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID lpData, DWORD cbData) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetFontData(m_hDC, dwTable, dwOffset, lpData, cbData);
    }

    int GetKerningPairs(int nPairs, LPKERNINGPAIR lpkrnpair) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetKerningPairs(m_hDC, nPairs, lpkrnpair);
    }

    UINT GetOutlineTextMetrics(UINT cbData, LPOUTLINETEXTMETRIC lpotm) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetOutlineTextMetrics(m_hDC, cbData, lpotm);
    }

    DWORD GetGlyphOutline(UINT nChar, UINT nFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpBuffer, const MAT2* lpmat2) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetGlyphOutline(m_hDC, nChar, nFormat, lpgm, cbBuffer, lpBuffer, lpmat2);
    }

    BOOL GetCharABCWidths(UINT nFirstChar, UINT nLastChar, LPABCFLOAT lpABCF) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharABCWidthsFloat(m_hDC, nFirstChar, nLastChar, lpABCF);
    }

    BOOL GetCharWidth(UINT nFirstChar, UINT nLastChar, float* lpFloatBuffer) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharWidthFloat(m_hDC, nFirstChar, nLastChar, lpFloatBuffer);
    }
#endif // !_WIN32_WCE

// Printer/Device Escape Functions
#ifndef _WIN32_WCE
    int Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData)
    {
        ASSERT(m_hDC != NULL);
        return ::Escape(m_hDC, nEscape, nCount, lpszInData, lpOutData);
    }
#endif // !_WIN32_WCE

    int Escape(int nEscape, int nInputSize, LPCSTR lpszInputData,
               int nOutputSize, LPSTR lpszOutputData)
    {
        ASSERT(m_hDC != NULL);
        return ::ExtEscape(m_hDC, nEscape, nInputSize, lpszInputData, nOutputSize, lpszOutputData);
    }

#ifndef _WIN32_WCE
    int DrawEscape(int nEscape, int nInputSize, LPCSTR lpszInputData)
    {
        ASSERT(m_hDC != NULL);
        return ::DrawEscape(m_hDC, nEscape, nInputSize, lpszInputData);
    }
#endif // !_WIN32_WCE

    // Escape helpers
#if !defined(_WIN32_WCE) || ((_WIN32_WCE >= 200) && defined(StartDoc))
    int StartDoc(LPCTSTR lpszDocName)  // old Win3.0 version
    {
        DOCINFO di = { 0 };
        di.cbSize = sizeof(DOCINFO);
        di.lpszDocName = lpszDocName;
        return StartDoc(&di);
    }

    int StartDoc(LPDOCINFO lpDocInfo)
    {
        ASSERT(m_hDC != NULL);
        return ::StartDoc(m_hDC, lpDocInfo);
    }

    int StartPage()
    {
        ASSERT(m_hDC != NULL);
        return ::StartPage(m_hDC);
    }

    int EndPage()
    {
        ASSERT(m_hDC != NULL);
        return ::EndPage(m_hDC);
    }

    int SetAbortProc(BOOL (CALLBACK* lpfn)(HDC, int))
    {
        ASSERT(m_hDC != NULL);
        return ::SetAbortProc(m_hDC, (ABORTPROC)lpfn);
    }

    int AbortDoc()
    {
        ASSERT(m_hDC != NULL);
        return ::AbortDoc(m_hDC);
    }

    int EndDoc()
    {
        ASSERT(m_hDC != NULL);
        return ::EndDoc(m_hDC);
    }
#endif // !defined(_WIN32_WCE) || ((_WIN32_WCE >= 200) && defined(StartDoc))

// MetaFile Functions
#ifndef _WIN32_WCE
    BOOL PlayMetaFile(HMETAFILE hMF)
    {
        ASSERT(m_hDC != NULL);
        if(::GetDeviceCaps(m_hDC, TECHNOLOGY) == DT_METAFILE)
        {
            // playing metafile in metafile, just use core windows API
            return ::PlayMetaFile(m_hDC, hMF);
        }

        // for special playback, lParam == pDC
        return ::EnumMetaFile(m_hDC, hMF, EnumMetaFileProc, (LPARAM)this);
    }

    BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, LPCRECT lpBounds)
    {
        ASSERT(m_hDC != NULL);
        return ::PlayEnhMetaFile(m_hDC, hEnhMetaFile, lpBounds);
    }

    BOOL AddMetaFileComment(UINT nDataSize, const BYTE* pCommentData) // can be used for enhanced metafiles only
    {
        ASSERT(m_hDC != NULL);
        return ::GdiComment(m_hDC, nDataSize, pCommentData);
    }

    // Special handling for metafile playback
    static int CALLBACK EnumMetaFileProc(HDC hDC, HANDLETABLE* pHandleTable, METARECORD* pMetaRec, int nHandles, LPARAM lParam)
    {
        CDCHandle* pDC = (CDCHandle*)lParam;

        switch (pMetaRec->rdFunction)
        {
        case META_SETMAPMODE:
            pDC->SetMapMode((int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SETWINDOWEXT:
            pDC->SetWindowExt((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SETWINDOWORG:
            pDC->SetWindowOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SETVIEWPORTEXT:
            pDC->SetViewportExt((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SETVIEWPORTORG:
            pDC->SetViewportOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SCALEWINDOWEXT:
            pDC->ScaleWindowExt((int)(short)pMetaRec->rdParm[3], (int)(short)pMetaRec->rdParm[2],
                                (int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SCALEVIEWPORTEXT:
            pDC->ScaleViewportExt((int)(short)pMetaRec->rdParm[3], (int)(short)pMetaRec->rdParm[2],
                                  (int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_OFFSETVIEWPORTORG:
            pDC->OffsetViewportOrg((int)(short)pMetaRec->rdParm[1], (int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SAVEDC:
            pDC->SaveDC();
            break;
        case META_RESTOREDC:
            pDC->RestoreDC((int)(short)pMetaRec->rdParm[0]);
            break;
        case META_SETBKCOLOR:
            pDC->SetBkColor(*(UNALIGNED COLORREF*)&pMetaRec->rdParm[0]);
            break;
        case META_SETTEXTCOLOR:
            pDC->SetTextColor(*(UNALIGNED COLORREF*)&pMetaRec->rdParm[0]);
            break;

            // need to watch out for SelectObject(HFONT), for custom font mapping
        case META_SELECTOBJECT:
        {
            HGDIOBJ hObject = pHandleTable->objectHandle[pMetaRec->rdParm[0]];
            UINT nObjType = ::GetObjectType(hObject);
            if(nObjType == 0)
            {
                // object type is unknown, determine if it is a font
                HFONT hStockFont = (HFONT)::GetStockObject(SYSTEM_FONT);
                HFONT hFontOld = (HFONT)::SelectObject(pDC->m_hDC, hStockFont);
                HGDIOBJ hObjOld = ::SelectObject(pDC->m_hDC, hObject);
                if(hObjOld == hStockFont)
                {
                    // got the stock object back, so must be selecting a font
                    pDC->SelectFont((HFONT)hObject);
                    break;  // don't play the default record
                }
                else
                {
                    // didn't get the stock object back, so restore everything
                    ::SelectObject(pDC->m_hDC, hFontOld);
                    ::SelectObject(pDC->m_hDC, hObjOld);
                }
                // and fall through to PlayMetaFileRecord...
            }
            else if(nObjType == OBJ_FONT)
            {
                // play back as CDCHandle::SelectFont(HFONT)
                pDC->SelectFont((HFONT)hObject);
                break;  // don't play the default record
            }
        }
        // fall through...

        default:
            ::PlayMetaFileRecord(hDC, pHandleTable, pMetaRec, nHandles);
            break;
        }

        return 1;
    }
#endif // !_WIN32_WCE

// Path Functions
#ifndef _WIN32_WCE
    BOOL AbortPath()
    {
        ASSERT(m_hDC != NULL);
        return ::AbortPath(m_hDC);
    }

    BOOL BeginPath()
    {
        ASSERT(m_hDC != NULL);
        return ::BeginPath(m_hDC);
    }

    BOOL CloseFigure()
    {
        ASSERT(m_hDC != NULL);
        return ::CloseFigure(m_hDC);
    }

    BOOL EndPath()
    {
        ASSERT(m_hDC != NULL);
        return ::EndPath(m_hDC);
    }

    BOOL FillPath()
    {
        ASSERT(m_hDC != NULL);
        return ::FillPath(m_hDC);
    }

    BOOL FlattenPath()
    {
        ASSERT(m_hDC != NULL);
        return ::FlattenPath(m_hDC);
    }

    BOOL StrokeAndFillPath()
    {
        ASSERT(m_hDC != NULL);
        return ::StrokeAndFillPath(m_hDC);
    }

    BOOL StrokePath()
    {
        ASSERT(m_hDC != NULL);
        return ::StrokePath(m_hDC);
    }

    BOOL WidenPath()
    {
        ASSERT(m_hDC != NULL);
        return ::WidenPath(m_hDC);
    }

    BOOL GetMiterLimit(PFLOAT pfMiterLimit) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetMiterLimit(m_hDC, pfMiterLimit);
    }

    BOOL SetMiterLimit(float fMiterLimit)
    {
        ASSERT(m_hDC != NULL);
        return ::SetMiterLimit(m_hDC, fMiterLimit, NULL);
    }

    int GetPath(LPPOINT lpPoints, LPBYTE lpTypes, int nCount) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetPath(m_hDC, lpPoints, lpTypes, nCount);
    }

    BOOL SelectClipPath(int nMode)
    {
        ASSERT(m_hDC != NULL);
        return ::SelectClipPath(m_hDC, nMode);
    }
#endif // !_WIN32_WCE

// Misc Helper Functions
    static CBrushHandle PASCAL GetHalftoneBrush()
    {
        HBRUSH halftoneBrush = NULL;
        WORD grayPattern[8];
        for(int i = 0; i < 8; i++)
            grayPattern[i] = (WORD)(0x5555 << (i & 1));
        HBITMAP grayBitmap = CreateBitmap(8, 8, 1, 1, &grayPattern);
        if(grayBitmap != NULL)
        {
            halftoneBrush = ::CreatePatternBrush(grayBitmap);
            DeleteObject(grayBitmap);
        }
        return CBrushHandle(halftoneBrush);
    }

    void DrawDragRect(LPCRECT lpRect, SIZE size, LPCRECT lpRectLast, SIZE sizeLast, HBRUSH hBrush = NULL, HBRUSH hBrushLast = NULL)
    {
        // first, determine the update region and select it
        CRgn rgnOutside;
        rgnOutside.CreateRectRgnIndirect(lpRect);
        RECT rect = *lpRect;
        ::InflateRect(&rect, -size.cx, -size.cy);
        ::IntersectRect(&rect, &rect, lpRect);
        CRgn rgnInside;
        rgnInside.CreateRectRgnIndirect(&rect);
        CRgn rgnNew;
        rgnNew.CreateRectRgn(0, 0, 0, 0);
        rgnNew.CombineRgn(rgnOutside, rgnInside, RGN_XOR);

        HBRUSH hBrushOld = NULL;
        CBrush brushHalftone;
        if(hBrush == NULL)
            brushHalftone = hBrush = CDCHandle::GetHalftoneBrush();
        if(hBrushLast == NULL)
            hBrushLast = hBrush;

        CRgn rgnLast;
        CRgn rgnUpdate;
        if(lpRectLast != NULL)
        {
            // find difference between new region and old region
            rgnLast.CreateRectRgn(0, 0, 0, 0);
            rgnOutside.SetRectRgn(lpRectLast->left, lpRectLast->top, lpRectLast->right, lpRectLast->bottom);
            rect = *lpRectLast;
            ::InflateRect(&rect, -sizeLast.cx, -sizeLast.cy);
            ::IntersectRect(&rect, &rect, lpRectLast);
            rgnInside.SetRectRgn(rect.left, rect.top, rect.right, rect.bottom);
            rgnLast.CombineRgn(rgnOutside, rgnInside, RGN_XOR);

            // only diff them if brushes are the same
            if(hBrush == hBrushLast)
            {
                rgnUpdate.CreateRectRgn(0, 0, 0, 0);
                rgnUpdate.CombineRgn(rgnLast, rgnNew, RGN_XOR);
            }
        }
        if(hBrush != hBrushLast && lpRectLast != NULL)
        {
            // brushes are different -- erase old region first
            SelectClipRgn(rgnLast);
            GetClipBox(&rect);
            hBrushOld = SelectBrush(hBrushLast);
            PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);
            SelectBrush(hBrushOld);
            hBrushOld = NULL;
        }

        // draw into the update/new region
        SelectClipRgn(rgnUpdate.IsNull() ? rgnNew : rgnUpdate);
        GetClipBox(&rect);
        hBrushOld = SelectBrush(hBrush);
        PatBlt(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, PATINVERT);

        // cleanup DC
        if(hBrushOld != NULL)
            SelectBrush(hBrushOld);
        SelectClipRgn(NULL);
    }

    void FillSolidRect(LPCRECT lpRect, COLORREF clr)
    {
        ASSERT(m_hDC != NULL);

        COLORREF clrOld = ::SetBkColor(m_hDC, clr);
        ASSERT(clrOld != CR_INVALID);
        if(clrOld != CR_INVALID)
        {
            ::ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
            ::SetBkColor(m_hDC, clrOld);
        }
    }

    void FillSolidRect(int x, int y, int cx, int cy, COLORREF clr)
    {
        ASSERT(m_hDC != NULL);

        RECT rect = { x, y, x + cx, y + cy };
        FillSolidRect(&rect, clr);
    }

    void Draw3dRect(LPCRECT lpRect, COLORREF clrTopLeft, COLORREF clrBottomRight)
    {
        Draw3dRect(lpRect->left, lpRect->top, lpRect->right - lpRect->left,
                   lpRect->bottom - lpRect->top, clrTopLeft, clrBottomRight);
    }

    void Draw3dRect(int x, int y, int cx, int cy, COLORREF clrTopLeft, COLORREF clrBottomRight)
    {
        FillSolidRect(x, y, cx - 1, 1, clrTopLeft);
        FillSolidRect(x, y, 1, cy - 1, clrTopLeft);
        FillSolidRect(x + cx, y, -1, cy, clrBottomRight);
        FillSolidRect(x, y + cy, cx, -1, clrBottomRight);
    }

// DIB support
#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 410)
    int SetDIBitsToDevice(int x, int y, DWORD dwWidth, DWORD dwHeight, int xSrc, int ySrc, UINT uStartScan, UINT cScanLines, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse)
    {
        ASSERT(m_hDC != NULL);
        return ::SetDIBitsToDevice(m_hDC, x, y, dwWidth, dwHeight, xSrc, ySrc, uStartScan, cScanLines, lpvBits, lpbmi, uColorUse);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 410)

#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)
    int StretchDIBits(int x, int y, int nWidth, int nHeight, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, CONST VOID* lpvBits, CONST BITMAPINFO* lpbmi, UINT uColorUse, DWORD dwRop)
    {
        ASSERT(m_hDC != NULL);
        return ::StretchDIBits(m_hDC, x, y, nWidth, nHeight, xSrc, ySrc, nSrcWidth, nSrcHeight, lpvBits, lpbmi, uColorUse, dwRop);
    }

    UINT GetDIBColorTable(UINT uStartIndex, UINT cEntries, RGBQUAD* pColors) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetDIBColorTable(m_hDC, uStartIndex, cEntries, pColors);
    }

    UINT SetDIBColorTable(UINT uStartIndex, UINT cEntries, CONST RGBQUAD* pColors)
    {
        ASSERT(m_hDC != NULL);
        return ::SetDIBColorTable(m_hDC, uStartIndex, cEntries, pColors);
    }
#endif // !defined(_WIN32_WCE) || (_WIN32_WCE >= 400)


// New for Windows 2000 only
#if (_WIN32_WINNT >= 0x0500)
    COLORREF GetDCPenColor() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetDCPenColor(m_hDC);
    }

    COLORREF SetDCPenColor(COLORREF clr)
    {
        ASSERT(m_hDC != NULL);
        return ::SetDCPenColor(m_hDC, clr);
    }

    COLORREF GetDCBrushColor() const
    {
        ASSERT(m_hDC != NULL);
        return ::GetDCBrushColor(m_hDC);
    }

    COLORREF SetDCBrushColor(COLORREF clr)
    {
        ASSERT(m_hDC != NULL);
        return ::SetDCBrushColor(m_hDC, clr);
    }

#ifndef _WIN32_WCE
    DWORD GetFontUnicodeRanges(LPGLYPHSET lpgs) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetFontUnicodeRanges(m_hDC, lpgs);
    }
#endif // !_WIN32_WCE

    DWORD GetGlyphIndices(LPCTSTR lpstr, int cch, LPWORD pgi, DWORD dwFlags) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetGlyphIndices(m_hDC, lpstr, cch, pgi, dwFlags);
    }

    BOOL GetTextExtentPointI(LPWORD pgiIn, int cgi, LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextExtentPointI(m_hDC, pgiIn, cgi, lpSize);
    }

    BOOL GetTextExtentExPointI(LPWORD pgiIn, int cgi, int nMaxExtent, LPINT lpnFit, LPINT alpDx, LPSIZE lpSize) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetTextExtentExPointI(m_hDC, pgiIn, cgi, nMaxExtent, lpnFit, alpDx, lpSize);
    }

    BOOL GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pgi, LPINT lpBuffer) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharWidthI(m_hDC, giFirst, cgi, pgi, lpBuffer);
    }

    BOOL GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pgi, LPABC lpabc) const
    {
        ASSERT(m_hDC != NULL);
        return ::GetCharABCWidthsI(m_hDC, giFirst, cgi, pgi, lpabc);
    }
#endif // (_WIN32_WINNT >= 0x0500)

// New for Windows 2000 and Windows 98
#if (WINVER >= 0x0500) && !defined(_WIN32_WCE)
    BOOL ColorCorrectPalette(HPALETTE hPalette, DWORD dwFirstEntry, DWORD dwNumOfEntries)
    {
        ASSERT(m_hDC != NULL);
        return ::ColorCorrectPalette(m_hDC, hPalette, dwFirstEntry, dwNumOfEntries);
    }
#endif // (WINVER >= 0x0500) && !defined(_WIN32_WCE)
};

typedef CDCT<false>   CDCHandle;
typedef CDCT<true>    CDC;


///////////////////////////////////////////////////////////////////////////////
// CDC Helpers

class CPaintDC : public CDC
{
public:
// Data members
    HWND m_hWnd;
    PAINTSTRUCT m_ps;

// Constructor/destructor
    CPaintDC(HWND hWnd)
    {
        ASSERT(::IsWindow(hWnd));
        m_hWnd = hWnd;
        m_hDC = ::BeginPaint(hWnd, &m_ps);
    }

    ~CPaintDC()
    {
        ASSERT(m_hDC != NULL);
        ASSERT(::IsWindow(m_hWnd));
        ::EndPaint(m_hWnd, &m_ps);
        Detach();
    }
};

class CClientDC : public CDC
{
public:
// Data members
    HWND m_hWnd;

// Constructor/destructor
    CClientDC(HWND hWnd)
    {
        ASSERT(hWnd == NULL || ::IsWindow(hWnd));
        m_hWnd = hWnd;
        m_hDC = ::GetDC(hWnd);
    }

    ~CClientDC()
    {
        ASSERT(m_hDC != NULL);
        ::ReleaseDC(m_hWnd, Detach());
    }
};

class CWindowDC : public CDC
{
public:
// Data members
    HWND m_hWnd;

// Constructor/destructor
    CWindowDC(HWND hWnd)
    {
        ASSERT(hWnd == NULL || ::IsWindow(hWnd));
        m_hWnd = hWnd;
        m_hDC = ::GetWindowDC(hWnd);
    }

    ~CWindowDC()
    {
        ASSERT(m_hDC != NULL);
        ::ReleaseDC(m_hWnd, Detach());
    }
};


}; // namespace SOUI

#endif // __SOUIGDI_H__
