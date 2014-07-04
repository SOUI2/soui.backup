#include "souistd.h"
#include "res.mgr/SSkinPool.h"
#include "core/Sskin.h"
#include "SApp.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
// DuiSkinPool

SSkinPool::SSkinPool()
{
    m_pFunOnKeyRemoved=OnKeyRemoved;
}

SSkinPool::~SSkinPool()
{
}

BOOL SSkinPool::Init(pugi::xml_node xmlNode)
{
    if (wcscmp(xmlNode.name(), L"skins") != 0)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    m_xmlSkinDesc.append_copy(xmlNode);
    LoadSkins(L"");
    return TRUE;
}

int SSkinPool::LoadSkins(LPCWSTR strOwnerName)
{
    int nLoaded=0;
    SStringW strSkinName, strTypeName;

    pugi::xml_node xmlSkin=m_xmlSkinDesc.child(L"skins").first_child();
    while(xmlSkin)
    {
        SStringW strOwner= xmlSkin.attribute(L"owner").value();
        if(strOwner==strOwnerName)
        {
            strTypeName = xmlSkin.name();
            strSkinName = xmlSkin.attribute(L"name").value();

            if (strSkinName.IsEmpty() || strTypeName.IsEmpty())
                continue;

            ASSERT(!HasKey(strSkinName));
            ISkinObj *pSkin=SApplication::getSingleton().CreateSkinByName(strTypeName);
            if(pSkin)
            {
                pSkin->InitFromXml(xmlSkin);
                pSkin->SetOwner(strOwnerName);
                AddKeyObject(strSkinName,pSkin);
                nLoaded++;
            }
            else
            {
                DUIRES_ASSERTW(FALSE,L"load skin error,type=%s,name=%s",strTypeName,strSkinName);
            }
        }
        xmlSkin=xmlSkin.next_sibling();
    }

    return nLoaded;
}


int SSkinPool::FreeSkins( LPCWSTR strOwnerName )
{
    if(!strOwnerName || wcslen(strOwnerName)==0) return 0;

    int nFreed=0;

    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        SMap<SStringW,SSkinPtr>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value->GetOwner()==strOwnerName)
        {
            OnKeyRemoved(p->m_value);
            m_mapNamedObj->RemoveAtPos((POSITION)p);
            nFreed++;
        }
    }
    return nFreed;
}

ISkinObj* SSkinPool::GetSkin(LPCWSTR strSkinName)
{
    if(!HasKey(strSkinName))
    {
        DUIRES_ASSERTA(FALSE,"GetSkin[%s] Failed!",strSkinName);
        return NULL;
    }
    return GetKeyObject(strSkinName);
}

void SSkinPool::OnKeyRemoved(const SSkinPtr & obj )
{
    obj->Release();
}

}//namespace SOUI