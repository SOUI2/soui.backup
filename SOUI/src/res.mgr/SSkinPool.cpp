#include "souistd.h"
#include "res.mgr/SSkinPool.h"
#include "core/Sskin.h"
#include "SApp.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
// SSkinPool
    template<> SSkinPool* SSingleton<SSkinPool>::ms_Singleton=0;

SSkinPool::SSkinPool()
{
    m_pFunOnKeyRemoved=OnKeyRemoved;
}

SSkinPool::~SSkinPool()
{
}

int SSkinPool::LoadSkins(pugi::xml_node xmlNode,DWORD dwOwnerID)
{
    if(!xmlNode) return 0;
    
    int nLoaded=0;
    SStringW strSkinName, strTypeName;

    pugi::xml_node xmlSkin=xmlNode.first_child();
    while(xmlSkin)
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
            pSkin->SetOwnerID(dwOwnerID);
            AddKeyObject(strSkinName,pSkin);
            nLoaded++;
        }
        else
        {
            ASSERT_FMTW(FALSE,L"load skin error,type=%s,name=%s",strTypeName,strSkinName);
        }
        xmlSkin=xmlSkin.next_sibling();
    }

    return nLoaded;
}


int SSkinPool::FreeSkins(DWORD dwOwnerID)
{
    int nFreed=0;

    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        SMap<SStringW,SSkinPtr>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value->GetOwnerID()==dwOwnerID)
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
        ASSERT_FMTW(FALSE,L"GetSkin[%s] Failed!",strSkinName);
        return NULL;
    }
    return GetKeyObject(strSkinName);
}

void SSkinPool::OnKeyRemoved(const SSkinPtr & obj )
{
    obj->Release();
}

}//namespace SOUI