#pragma once
#include "DuiSingletonMap.h"
#include "duiwndstyle.h"


namespace SOUI
{
	class SOUI_EXP DuiStylePool :public DuiCmnMap<DuiStyle,CDuiStringA>
	{
	public:
		// Get style object from pool by class name
		BOOL GetStyle(LPCSTR lpszName,DuiStyle& style);

		BOOL Init(pugi::xml_node xmlNode);
	};

}//end of namespace SOUI
