#include "stdafx.h"
#include "soui_mem_wrapper.h"
#include <malloc.h>
#include "utilities-def.h"

namespace SOUI
{
    soui_mem_wrapper::soui_mem_wrapper(void)
    {
        m_hSouiMem = LoadLibrary(TEXT("soui-mem.dll"));
        if(m_hSouiMem)
        {
            m_funMalloc = (FunMalloc)GetProcAddress(m_hSouiMem,"SouiMalloc");SASSERT(m_funMalloc);
            m_funCalloc = (FunCalloc)GetProcAddress(m_hSouiMem,"SouiCalloc");SASSERT(m_funCalloc);
            m_funRealloc = (FunRealloc)GetProcAddress(m_hSouiMem,"SouiRealloc");SASSERT(m_funRealloc);
            m_funFree = (FunFree)GetProcAddress(m_hSouiMem,"SouiFree");SASSERT(m_funFree);
        }else
        {
            m_funMalloc = malloc;
            m_funCalloc =calloc;
            m_funRealloc =realloc;
            m_funFree   =free;
        }
    }

    soui_mem_wrapper::~soui_mem_wrapper(void)
    {
        if(m_hSouiMem) FreeLibrary(m_hSouiMem);
    }

    soui_mem_wrapper * soui_mem_wrapper::GetInstance()
    {
        static soui_mem_wrapper s_mem;
        return &s_mem;
    }
    
    void * soui_mem_wrapper::SouiMalloc( size_t szMem )
    {
        return GetInstance()->m_funMalloc(szMem);
    }

    void * soui_mem_wrapper::SouiRealloc( void *p,size_t szMem )
    {
        return GetInstance()->m_funRealloc(p,szMem);
    }

    void * soui_mem_wrapper::SouiCalloc( size_t count, size_t szEle )
    {
        return GetInstance()->m_funCalloc(count,szEle);
    }

    void soui_mem_wrapper::SouiFree( void *p )
    {
        GetInstance()->m_funFree(p);
    }


}
