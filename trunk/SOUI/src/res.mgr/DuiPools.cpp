#include "duistd.h"
#include "mybuffer.h"
#include "res.mgr/DuiPools.h"

namespace SOUI
{
    DuiPools::DuiPools()
    {
    }

    DuiPools::~DuiPools(void)
    {
    }

    void DuiPools::Init( LPCTSTR pszInitXml ,LPCTSTR pszResType)
    {
        pugi::xml_document xmlDoc;
        if(LOADXML(xmlDoc,pszInitXml,pszResType))
        {
            Init(xmlDoc.first_child());
        }
    }

    void DuiPools::Init( pugi::xml_node xmlNode )
    {
        //load string table
        pugi::xml_node xmlStr=xmlNode.child("string");
        if(xmlStr)
        {
            DuiStringPool::Init(xmlStr);
        }
        //load style table
        pugi::xml_node xmlStyle=xmlNode.child("style");
        if(xmlStyle)
        {
            DuiStylePool::Init(xmlStyle);
        }
        //load skin
        pugi::xml_node xmlSkin=xmlNode.child("skins");
        if(xmlSkin)
        {
            DuiSkinPool::Init(xmlSkin);
        }
        pugi::xml_node xmlObjAttr=xmlNode.child("objattr");
        //load objattr
        if(xmlObjAttr)
        {
            DuiCSS::Init(xmlObjAttr);
        }
    }

    void DuiPools::Clear()
    {
        DuiStringPool::RemoveAll();
        DuiStylePool::RemoveAll();
        DuiSkinPool::RemoveAll();
        DuiCSS::RemoveAll();
    }
}//end of namespace
