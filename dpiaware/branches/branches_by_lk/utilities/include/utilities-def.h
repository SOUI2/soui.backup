#ifdef UTILITIES_EXPORTS
#define UTILITIES_API __declspec(dllexport)
#else
#define UTILITIES_API __declspec(dllimport)
#endif

#ifndef SASSERT
#include <assert.h>
#define SASSERT(x) assert(x)
#define ASSERT_NE(a,b) SASSERT(a!=b)
#endif