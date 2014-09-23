#pragma once

#include "utilities-def.h"
#include "string/tstring.h"

namespace SOUI
{
    void UTILITIES_API SHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix);
    void UTILITIES_API SPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric);
    ULONG UTILITIES_API HexStringToULong(LPCWSTR lpszValue, int nSize = -1);
    COLORREF UTILITIES_API HexStringToColor(LPCWSTR lpszValue);
    COLORREF UTILITIES_API StringToColor(const SStringW & str);
}//end of namespace SOUI
