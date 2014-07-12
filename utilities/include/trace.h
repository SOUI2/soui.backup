#pragma once

#include "utilities-def.h"

namespace SOUI
{
    void UTILITIES_API STraceA(LPCSTR pstrFormat, ...);
    void UTILITIES_API STraceW(LPCWSTR pstrFormat, ...);
}//end of namespace SOUI

#ifdef _UNICODE
#define STRACE SOUI::STraceW
#else
#define STRACE SOUI::STraceA
#endif
