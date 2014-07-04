#pragma once
#include "core/SSingletonMap.h"
#include "core/swndstyle.h"


namespace SOUI
{
    class SOUI_EXP SStylePool :public SCmnMap<SwndStyle,SStringW>
    {
    public:
        // Get style object from pool by class name
        BOOL GetStyle(LPCWSTR lpszName,SwndStyle& style);

        BOOL Init(pugi::xml_node xmlNode);
    };

}//end of namespace SOUI
