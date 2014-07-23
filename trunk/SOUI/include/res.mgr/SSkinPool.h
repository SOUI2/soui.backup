#pragma once
#include "core/SSingletonMap.h"
#include "interface/Sskinobj-i.h"

#define GETSKIN(p1) SSkinPool::getSingleton().GetSkin(p1)

namespace SOUI
{

typedef ISkinObj * SSkinPtr;
class SOUI_EXP SSkinPool :public SSingletonMap<SSkinPool,SSkinPtr,SStringW>
{
public:
    SSkinPool();

    virtual ~SSkinPool();

    ISkinObj* GetSkin(LPCWSTR strSkinName);

    int LoadSkins(pugi::xml_node xmlNode,DWORD dwOwnerID=0);

    int FreeSkins(DWORD dwOwnerID);

protected:
    static void OnKeyRemoved(const SSkinPtr & obj);
};

}//namespace SOUI