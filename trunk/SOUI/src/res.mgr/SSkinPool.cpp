#include "souistd.h"
#include "res.mgr/SSkinPool.h"
#include "core/Sskin.h"
#include "SApp.h"
#include "core/mybuffer.h"

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

const wchar_t * BUILDIN_SKIN_NAMES[]=
{
    L"_skin.sys.checkbox",
    L"_skin.sys.radio",
    L"_skin.sys.focuscheckbox",
    L"_skin.sys.focusradio",
    L"_skin.sys.btn.normal",
    L"_skin.sys.scrollbar",
    L"_skin.sys.border",
    L"_skin.sys.dropbtn",
    L"_skin.sys.tree.toggle",
    L"_skin.sys.tree.checkbox",
    L"_skin.sys.tab.page",
    L"_skin.sys.header",
    L"_skin.sys.split.vert",
    L"_skin.sys.split.horz",
    L"_skin.prog.bkgnd",
    L"_skin.prog.bar",
    L"_skin.slider.thumb",
    L"_skin.sys.btn.close",
    L"_skin.sys.btn.minimize",
    L"_skin.sys.btn.maxmize",
    L"_skin.sys.btn.restore"
};

BOOL SSkinPool::LoadBuildinSkins( IResProvider *pSysSkinProvider ,LPCTSTR pszSkinXmlName,LPCTSTR pszXmlType)
{
    size_t szXml= pSysSkinProvider->GetRawBufferSize(pszXmlType,pszSkinXmlName);
    if(szXml == 0) return FALSE;
    CMyBuffer<char> xmlBuf;
    xmlBuf.Allocate(szXml);
    if(!pSysSkinProvider->GetRawBuffer(pszXmlType,pszSkinXmlName,xmlBuf,szXml)) return FALSE;

    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_buffer_inplace(xmlBuf,szXml)) return FALSE;

    SApplication::getSingleton().AddResProvider(pSysSkinProvider);
    int nSkins=LoadSkins(xmlDoc.child(L"skins"));
    SApplication::getSingleton().RemoveResProvider(pSysSkinProvider);
    return nSkins == ARRAYSIZE(BUILDIN_SKIN_NAMES);
}

ISkinObj * SSkinPool::GetBuildinSkin( SYS_SKIN uID )
{
    return GetSkin(BUILDIN_SKIN_NAMES[uID]);
}

}//namespace SOUI