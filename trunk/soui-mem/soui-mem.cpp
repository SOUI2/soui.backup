#include "soui-mem.h"
#include <malloc.h>

void * CollMalloc(size_t szMem)
{
    return malloc(szMem);
}

void   CollFree(void *p)
{
    free(p);
}

void * CollRealloc( void *p,size_t szMem )
{
    return realloc(p,szMem);
}

void * CollCalloc( size_t count, size_t szEle )
{
    return calloc(count,szEle);
}