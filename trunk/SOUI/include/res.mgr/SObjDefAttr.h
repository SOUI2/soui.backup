#pragma once

#include "core/SSingletonMap.h"

#define GETCSS(p1) SObjDefAttr::getSingleton().GetDefAttribute(p1)

namespace SOUI
{

class SOUI_EXP SObjDefAttr :public SSingletonMap<SObjDefAttr,pugi::xml_node,SStringW>
{
public:
    SObjDefAttr()
    {
    }
    virtual ~SObjDefAttr()
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

