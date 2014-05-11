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
#define GET_X_LPARAM(lParam)	((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	((int)(short)HIWORD(lParam))
#endif


#ifdef _DEBUG
#include <crtdbg.h>
#   define DUIRES_ASSERTW(expr, format, ...) \
	(void) ((!!(expr)) || \
	(1 != _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__, NULL, format, __VA_ARGS__)) || \
	(_CrtDbgBreak(), 0))

#   define DUIRES_ASSERTA(expr, format, ...) \
	(void) ((!!(expr)) || \
	(1 != _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, format, __VA_ARGS__)) || \
	(_CrtDbgBreak(), 0))

#else
#   define DUIRES_ASSERTW
#	define DUIRES_ASSERTA
#endif

#ifdef _UNICODE 
#   define DUIRES_ASSERT	DUIRES_ASSERTW
#else
#   define DUIRES_ASSERT	DUIRES_ASSERTA
#endif//_UNICODE

// DuiWindow Handle
typedef DWORD HDUIWND;
typedef ULONG_PTR HSTREEITEM;

typedef struct _DUINMHDR
{
	HDUIWND	hDuiWnd;		//消息源窗口
	UINT	code;			//消息类型
	LPCSTR	pszNameFrom;	//消息源的name
	UINT	idFrom;			//消息源的id
}DUINMHDR, *LPDUINMHDR;
