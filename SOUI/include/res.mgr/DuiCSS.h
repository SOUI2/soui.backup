#pragma once

#include "DuiSingletonMap.h"
namespace SOUI
{

class SOUI_EXP DuiCSS :public DuiCmnMap<pugi::xml_node,SStringW>
{
public:
    DuiCSS()
    {
    }
    virtual ~DuiCSS()
    {
    }

    BOOL Init(pugi::xml_node xmlNode);

    pugi::xml_node GetDefAttribute(LPCWSTR pszClassName);
protected:
    pugi::xml_node _GetDefAttribute(LPCWSTR pszClassName);
    void BuildClassAttribute(pugi::xml_node & xmlNode, LPCWSTR pszClassName);

    pugi::xml_document m_xmlRoot;
};

}//namespace SOUI

