#include "soui-mem.h"
#include <malloc.h>

void * SouiMalloc(size_t szMem)
{
    return malloc(szMem);
}

void   SouiFree(void *p)
{
    free(p);
}

void * SouiRealloc( void *p,size_t szMem )
{
    return realloc(p,szMem);
}

void * SouiCalloc( size_t count, size_t szEle )
{
    return calloc(count,szEle);
}