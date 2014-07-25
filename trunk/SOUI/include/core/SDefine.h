#pragma once

// DISALLOW_COPY_AND_ASSIGN禁用拷贝和赋值构造函数.
// 需要在类的private:访问控制域中使用.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&); \
    void operator=(const TypeName&)


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)    ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)    ((int)(short)HIWORD(lParam))
#endif


#ifdef _DEBUG
#include <crtdbg.h>
#   define ASSERT_FMTW(expr, format, ...) \
    (void) ((!!(expr)) || \
    (1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, format, __VA_ARGS__)) || \
    (_CrtDbgBreak(), 0))

#   define ASSERT_FMTA(expr, format, ...) \
    (void) ((!!(expr)) || \
    (1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, format, __VA_ARGS__)) || \
    (_CrtDbgBreak(), 0))
#else
#   define ASSERT_FMTW
#   define ASSERT_FMTA
#endif

#ifdef _UNICODE 
#   define ASSERT_FMT    ASSERT_FMTW
#else
#   define ASSERT_FMT    ASSERT_FMTA
#endif//_UNICODE

#include <assert.h>
#define ASSERT(x) assert(x)
#define ASSERT_NE(a,b) ASSERT(a!=b)

#ifdef _DISABLE_NO_VTABLE
#define S_NO_VTABLE
#else
#define S_NO_VTABLE __declspec(novtable)
#endif


// SWindow Handle
typedef DWORD SWND;
typedef ULONG_PTR HSTREEITEM;
