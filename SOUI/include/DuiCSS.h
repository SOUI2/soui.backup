#pragma once

#include "DuiSingletonMap.h"
namespace SOUI
{

class SOUI_EXP DuiCSS :public DuiCmnMap<pugi::xml_node,CDuiStringA>
{
public:
    DuiCSS()
    {
    }
    virtual ~DuiCSS()
    {
    }

	BOOL Init(pugi::xml_node xmlNode);

	pugi::xml_node GetDefAttribute(LPCSTR pszClassName);
protected:
	pugi::xml_node _GetDefAttribute(LPCSTR pszClassName);
	void BuildClassAttribute(pugi::xml_node & xmlNode, LPCSTR pszClassName);

	pugi::xml_document m_xmlRoot;
};

}//namespace SOUI

