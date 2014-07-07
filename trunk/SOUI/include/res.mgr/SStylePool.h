#pragma once
#include "core/SSingletonMap.h"
#include "core/swndstyle.h"

#define GETSTYLE(p1,p2) SStylePool::getSingleton().GetStyle(p1,p2)

namespace SOUI
{
    class SOUI_EXP SStylePool :public SSingletonMap<SStylePool,SwndStyle,SStringW>
    {
    public:
        // Get style object from pool by class name
        BOOL GetStyle(LPCWSTR lpszName,SwndStyle& style);

        BOOL Init(pugi::xml_node xmlNode);
    };

}//end of namespace SOUI
