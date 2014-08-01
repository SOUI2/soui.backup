#pragma once
#include "core/SSingletonMap.h"
#include "core/swndstyle.h"
#include <unknown/obj-ref-i.h>
#include <unknown/obj-ref-impl.hpp>

#define GETSTYLEPOOLMGR SStylePoolMgr::getSingletonPtr()
#define GETSTYLE(p1,p2) SStylePoolMgr::getSingleton().GetStyle(p1,p2)

namespace SOUI
{
    class SOUI_EXP SStylePool :public SCmnMap<SwndStyle,SStringW> , public TObjRefImpl2<IObjRef,SStylePool>
    {
    public:
        // Get style object from pool by class name
        BOOL GetStyle(LPCWSTR lpszName,SwndStyle& style);

        BOOL Init(pugi::xml_node xmlNode);
    };

    class SOUI_EXP SStylePoolMgr : public SSingleton<SStylePoolMgr>
    {
    public:
        ~SStylePoolMgr();
        
        BOOL GetStyle(LPCWSTR lpszName,SwndStyle& style);
        
        void PushStylePool(SStylePool *pStylePool);
        
        SStylePool * PopStylePool();
    protected:
        SList<SStylePool *> m_lstStylePools;
    };
}//end of namespace SOUI
