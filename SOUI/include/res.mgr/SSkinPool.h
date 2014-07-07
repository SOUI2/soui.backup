#pragma once
#include "core/SSingletonMap.h"
#include "core/Sskinobj-i.h"

#define GETSKIN(p1) SSkinPool::getSingleton().GetSkin(p1)

namespace SOUI
{

typedef ISkinObj * SSkinPtr;
class SOUI_EXP SSkinPool :public SSingletonMap<SSkinPool,SSkinPtr,SStringW>
{
public:
    SSkinPool();

    virtual ~SSkinPool();

    BOOL Init(pugi::xml_node xmlNode);

    ISkinObj* GetSkin(LPCWSTR strSkinName);

    int LoadSkins(LPCWSTR  pszOwnerName);

    int FreeSkins(LPCWSTR  pszOwnerName);

protected:
    static void OnKeyRemoved(const SSkinPtr & obj);

    pugi::xml_document m_xmlSkinDesc;
};

}//namespace SOUI