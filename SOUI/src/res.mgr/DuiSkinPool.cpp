#include "duistd.h"
#include "res.mgr/DuiSkinPool.h"
#include "duiskin.h"
#include "duisystem.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
// DuiSkinPool

DuiSkinPool::DuiSkinPool()
{
    m_pFunOnKeyRemoved=OnKeyRemoved;
}

DuiSkinPool::~DuiSkinPool()
{
}

BOOL DuiSkinPool::Init(pugi::xml_node xmlNode)
{
    if (wcscmp(xmlNode.name(), L"skins") != 0)
    {
        DUIASSERT(FALSE);
        return FALSE;
    }

    m_xmlSkinDesc.append_copy(xmlNode);
    LoadSkins(L"");
    return TRUE;
}

int DuiSkinPool::LoadSkins(LPCWSTR strOwnerName)
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

            DUIASSERT(!HasKey(strSkinName));
            ISkinObj *pSkin=DuiSystem::getSingleton().CreateSkinByName(strTypeName);
            if(pSkin)
            {
                pSkin->Load(xmlSkin);
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


int DuiSkinPool::FreeSkins( LPCWSTR strOwnerName )
{
    if(!strOwnerName || wcslen(strOwnerName)==0) return 0;

    int nFreed=0;

    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        SMap<SStringW,DuiSkinPtr>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value->GetOwner()==strOwnerName)
        {
            OnKeyRemoved(p->m_value);
            m_mapNamedObj->RemoveAtPos((POSITION)p);
            nFreed++;
        }
    }
    return nFreed;
}

ISkinObj* DuiSkinPool::GetSkin(LPCWSTR strSkinName)
{
    if(!HasKey(strSkinName))
    {
        DUIRES_ASSERTA(FALSE,"GetSkin[%s] Failed!",strSkinName);
        return NULL;
    }
    return GetKeyObject(strSkinName);
}

void DuiSkinPool::OnKeyRemoved(const DuiSkinPtr & obj )
{
    obj->Release();
}

}//namespace SOUI