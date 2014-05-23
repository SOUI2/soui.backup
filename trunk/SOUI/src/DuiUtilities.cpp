#include "duistd.h"
#include "DuiUtilities.h"

namespace SOUI
{
    #define HIMETRIC_PER_INCH   2540
    #define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
    #define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

    void DuiHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
    {
        int nPixelsPerInchX;    // Pixels per logical inch along width
        int nPixelsPerInchY;    // Pixels per logical inch along height

        HDC hDCScreen = GetDC(NULL);
        nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
        nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
        ReleaseDC(NULL, hDCScreen);

        lpSizeInPix->cx = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cx, nPixelsPerInchX);
        lpSizeInPix->cy = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cy, nPixelsPerInchY);
    }

    void DuiPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
    {
        int nPixelsPerInchX;    // Pixels per logical inch along width
        int nPixelsPerInchY;    // Pixels per logical inch along height

        HDC hDCScreen = GetDC(NULL);
        nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
        nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
        ReleaseDC(NULL, hDCScreen);

        lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
        lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
    }

    void SOUI_EXP DuiTrace(LPCTSTR pstrFormat, ...)
    {
#ifdef _DEBUG
        TCHAR szBuffer[300] = { 0 };
        va_list args;
        va_start(args, pstrFormat);
        ::wvnsprintf(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
        _tcscat(szBuffer, _T("\n"));
        va_end(args);
        ::OutputDebugString(szBuffer);
#endif
    }

    void SOUI_EXP DuiTraceA(LPCSTR pstrFormat, ...)
    {
#ifdef _DEBUG
        char szBuffer[300] = { 0 };
        va_list args;
        va_start(args, pstrFormat);
        ::wvnsprintfA(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
        strcat(szBuffer, "\n");
        va_end(args);
        ::OutputDebugStringA(szBuffer);
#endif
    }

    void SOUI_EXP DuiTraceW(LPCWSTR pstrFormat, ...)
    {
#ifdef _DEBUG
        wchar_t szBuffer[300] = { 0 };
        va_list args;
        va_start(args, pstrFormat);
        ::wvnsprintfW(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
        wcscat(szBuffer, L"\n");
        va_end(args);
        ::OutputDebugStringW(szBuffer);
#endif
    }
}//end of namespace SOUI
