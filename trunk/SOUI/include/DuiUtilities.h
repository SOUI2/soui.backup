#pragma once

#include <assert.h>

namespace SOUI
{
	void SOUI_EXP DuiHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix);
	void SOUI_EXP DuiPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric);
	void SOUI_EXP DuiTraceA(LPCSTR pstrFormat, ...);
	void SOUI_EXP DuiTraceW(LPCWSTR pstrFormat, ...);
}//end of namespace SOUI

#ifdef _UNICODE
#define DUITRACE DuiTraceW
#else
#define DUITRACE DuiTraceA
#endif

#define DUIASSERT(x) assert(x)
#define DUIASSERT_NE(a,b) DUIASSERT(a!=b)

#ifdef _DUI_DISABLE_NO_VTABLE
#define DUI_NO_VTABLE
#else
#define DUI_NO_VTABLE __declspec(novtable)
#endif