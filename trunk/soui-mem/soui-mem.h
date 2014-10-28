#pragma once

#include <core-def.h>

#ifdef LIB_ALL
#define COLLMEM_API
#else
#ifdef COLLMEM_EXPORTS
#define COLLMEM_API __declspec(dllexport)
#else
#define COLLMEM_API __declspec(dllimport)
#endif
#endif

extern "C"
{
    COLLMEM_API void * CollMalloc(size_t szMem);
    COLLMEM_API void * CollRealloc(void *p,size_t szMem);
    COLLMEM_API void * CollCalloc(size_t count, size_t szEle);
    COLLMEM_API void   CollFree(void *p);
};
