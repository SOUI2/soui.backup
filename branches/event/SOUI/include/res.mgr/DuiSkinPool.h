#pragma once
#include "DuiSingletonMap.h"
#include "skinobj-i.h"

namespace SOUI
{

typedef ISkinObj * DuiSkinPtr;
class SOUI_EXP DuiSkinPool :public DuiCmnMap<DuiSkinPtr,SStringW>
{
public:
    DuiSkinPool();

    virtual ~DuiSkinPool();

    BOOL Init(pugi::xml_node xmlNode);

    ISkinObj* GetSkin(LPCWSTR strSkinName);

    int LoadSkins(LPCWSTR  pszOwnerName);

    int FreeSkins(LPCWSTR  pszOwnerName);

protected:
    static void OnKeyRemoved(const DuiSkinPtr & obj);

    pugi::xml_document m_xmlSkinDesc;
};

}//namespace SOUI