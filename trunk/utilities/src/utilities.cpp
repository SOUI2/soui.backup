#include "stdafx.h"
#include "utilities.h"

namespace SOUI
{
#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

    void SHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
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

    void SPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
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

    ULONG HexStringToULong(LPCWSTR lpszValue, int nSize)
    {

        LPCWSTR pchValue = lpszValue;
        ULONG ulValue = 0;
        while (*pchValue && nSize != 0)
        {
            ulValue <<= 4;

            if ('a' <= *pchValue && L'f' >= *pchValue)
                ulValue |= (*pchValue - L'a' + 10);
            else if ('A' <= *pchValue && L'F' >= *pchValue)
                ulValue |= (*pchValue - L'A' + 10);
            else if ('0' <= *pchValue && L'9' >= *pchValue)
                ulValue |= (*pchValue - L'0');
            else
                return 0;

            ++ pchValue;
            -- nSize;
        }

        return ulValue;
    }

    COLORREF HexStringToColor(LPCWSTR lpszValue)
    {
       COLORREF cr=RGB(
            HexStringToULong(lpszValue, 2),
            HexStringToULong(lpszValue + 2, 2),
            HexStringToULong(lpszValue + 4, 2)
            );
        if(wcslen(lpszValue)>6)
        {
            cr |= HexStringToULong(lpszValue + 6, 2)<<24;
        }else
        {
            cr |= 0xFF000000;
        }
        return cr;
    }
}//end of namespace SOUI
