#include "souistd.h"
#include "res.mgr/SObjDefAttr.h"
#include "SApp.h"
#include "core/mybuffer.h"

namespace SOUI
{

    template<> SObjDefAttr* SSingleton<SObjDefAttr>::ms_Singleton=0;

BOOL SObjDefAttr::Init( pugi::xml_node xmlNode )
{
    if (wcscmp(xmlNode.name(), L"objattr") != 0)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_xmlRoot.append_copy(xmlNode);

    pugi::xml_node xmlObjAttr=m_xmlRoot.child(L"objattr").first_child();
    while(xmlObjAttr)
    {
        AddKeyObject(xmlObjAttr.name(),xmlObjAttr);
        xmlObjAttr=xmlObjAttr.next_sibling();
    }

    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        SMap<SStringW,pugi::xml_node>::CPair *p=m_mapNamedObj->GetNext(pos);
        BuildClassAttribute(p->m_value,p->m_key);
    }

    return TRUE;
}

void SObjDefAttr::BuildClassAttribute( pugi::xml_node & xmlNode, LPCWSTR pszClassName)
{
    LPCWSTR pszBaseClassName=SApplication::getSingleton().BaseClassNameFromClassName(pszClassName);
    if(!pszBaseClassName) return;

    if(HasKey(pszBaseClassName))
    {
        pugi::xml_node xmlNodeAttrs = GetKeyObject(pszBaseClassName);
        pugi::xml_attribute attr=xmlNodeAttrs.first_attribute();
        while(attr)
        {
            if(!xmlNode.attribute(attr.name()))
                xmlNode.append_attribute(attr.name()).set_value(attr.value());
            attr=attr.next_attribute();
        }
    }
    BuildClassAttribute(xmlNode,pszBaseClassName);
}

pugi::xml_node SObjDefAttr::GetDefAttribute(LPCWSTR pszClassName )
{
    ASSERT(pszClassName);
    if(!static_cast<SWindowFactoryMgr*>(SApplication::getSingletonPtr())->HasKey(pszClassName))
        return pugi::xml_node();//节点是窗口时才有默认属性
    return _GetDefAttribute(pszClassName);
}

pugi::xml_node SObjDefAttr::_GetDefAttribute(LPCWSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        LPCWSTR pszBaseClassName=SApplication::getSingleton().BaseClassNameFromClassName(pszClassName);
        if(!pszBaseClassName) return pugi::xml_node();
        return _GetDefAttribute(pszBaseClassName);
    }else
    {
        return GetKeyObject(pszClassName);
    }
}


}//namespace SOUI

