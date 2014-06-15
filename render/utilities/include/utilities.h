#pragma once

#include "utilities-def.h"

namespace SOUI
{
    void UTILITIES_API DuiHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix);
    void UTILITIES_API DuiPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric);
}//end of namespace SOUI
