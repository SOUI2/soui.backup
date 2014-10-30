/********************************************************************
	created:	2014/10/30
	created:	30:10:2014   10:24
	filename: 	soui_mem_wrapper.h
	author:		soui group
	
	purpose:	包装内存动态分配
*********************************************************************/
#pragma once
#include "utilities-def.h"

namespace SOUI
{
    class UTILITIES_API soui_mem_wrapper
    {
    public:
        static void * SouiMalloc(size_t szMem);
        static void * SouiRealloc(void *p,size_t szMem);
        static void * SouiCalloc(size_t count, size_t szEle);
        static void   SouiFree(void *p);
    private:
        static soui_mem_wrapper * GetInstance();
        
        soui_mem_wrapper(void);
        ~soui_mem_wrapper(void);
        
        typedef void * (*FunMalloc)(size_t szMem);
        typedef void * (*FunRealloc)(void *p,size_t szMem);
        typedef void * (*FunCalloc)(size_t count, size_t szEle);
        typedef void   (*FunFree)(void *p);

        FunMalloc   m_funMalloc;
        FunRealloc  m_funRealloc;
        FunCalloc   m_funCalloc;
        FunFree     m_funFree;
        
        HMODULE m_hSouiMem;      
    };
}
