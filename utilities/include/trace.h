#pragma once

#include "utilities-def.h"

namespace SOUI
{
    void UTILITIES_API DuiTraceA(LPCSTR pstrFormat, ...);
    void UTILITIES_API DuiTraceW(LPCWSTR pstrFormat, ...);
}//end of namespace SOUI

#ifdef _UNICODE
#define STRACE SOUI::DuiTraceW
#else
#define STRACE SOUI::DuiTraceA
#endif
