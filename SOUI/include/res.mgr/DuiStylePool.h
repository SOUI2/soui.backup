#pragma once
#include "DuiSingletonMap.h"
#include "duiwndstyle.h"


namespace SOUI
{
    class SOUI_EXP DuiStylePool :public DuiCmnMap<DuiStyle,SStringW>
    {
    public:
        // Get style object from pool by class name
        BOOL GetStyle(LPCWSTR lpszName,DuiStyle& style);

        BOOL Init(pugi::xml_node xmlNode);
    };

}//end of namespace SOUI
