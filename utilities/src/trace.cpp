#include "StdAfx.h"
#include "trace.h"
#include <stdio.h>

namespace SOUI
{
void  STraceA(LPCSTR pstrFormat, ...)
{
#ifdef _DEBUG
    char szBuffer[300] = { 0 };
    va_list args;
    va_start(args, pstrFormat);
    vsprintf_s(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
    strcat(szBuffer, "\n");
    va_end(args);
    ::OutputDebugStringA(szBuffer);
#endif
}

void  STraceW(LPCWSTR pstrFormat, ...)
{
#ifdef _DEBUG
    wchar_t szBuffer[300] = { 0 };
    va_list args;
    va_start(args, pstrFormat);
    vswprintf_s(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
    wcscat(szBuffer, L"\n");
    va_end(args);
    ::OutputDebugStringW(szBuffer);
#endif
}

}