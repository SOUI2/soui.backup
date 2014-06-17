#pragma once
#include "DuiSingletonMap.h"
#include "skinobj-i.h"

namespace SOUI
{

typedef ISkinObj * DuiSkinPtr;
class SOUI_EXP DuiSkinPool :public DuiCmnMap<DuiSkinPtr,CDuiStringA>
{
public:
    DuiSkinPool();

    virtual ~DuiSkinPool();

    BOOL Init(pugi::xml_node xmlNode);

    ISkinObj* GetSkin(LPCSTR strSkinName);

    int LoadSkins(LPCSTR  pszOwnerName);

    int FreeSkins(LPCSTR  pszOwnerName);

protected:
    static void OnKeyRemoved(const DuiSkinPtr & obj);

    pugi::xml_document m_xmlSkinDesc;
};

}//namespace SOUI