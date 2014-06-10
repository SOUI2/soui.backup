#pragma once

#pragma warning(disable:4005)

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <Windows.h>
#include <tchar.h>

// #include <unknown/obj-ref-i.h>
#include <assert.h>

#define DUIASSERT(x) assert(x)
#define DUIASSERT_NE(a,b) DUIASSERT(a!=b)