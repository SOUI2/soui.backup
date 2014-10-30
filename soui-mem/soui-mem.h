#pragma once

#include <core-def.h>

#ifdef LIB_ALL
#define SOUIMEM_API
#else
#ifdef SOUIMEM_EXPORTS
#define SOUIMEM_API __declspec(dllexport)
#else
#define SOUIMEM_API __declspec(dllimport)
#endif
#endif

extern "C"
{
    SOUIMEM_API void * SouiMalloc(size_t szMem);
    SOUIMEM_API void * SouiRealloc(void *p,size_t szMem);
    SOUIMEM_API void * SouiCalloc(size_t count, size_t szEle);
    SOUIMEM_API void   SouiFree(void *p);
};
