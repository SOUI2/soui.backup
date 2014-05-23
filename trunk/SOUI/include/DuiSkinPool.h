#pragma once
#include "DuiSingletonMap.h"
#include "DuiSkinBase.h"

namespace SOUI
{

typedef CDuiSkinBase * DuiSkinPtr;
class SOUI_EXP DuiSkinPool :public DuiCmnMap<DuiSkinPtr,CDuiStringA>
{
public:
    DuiSkinPool();

    virtual ~DuiSkinPool();

    BOOL Init(pugi::xml_node xmlNode);

    CDuiSkinBase* GetSkin(LPCSTR strSkinName);

    int LoadSkins(LPCSTR  pszOwnerName);

    int FreeSkins(LPCSTR  pszOwnerName);

protected:
    static void OnKeyRemoved(const DuiSkinPtr & obj);

    pugi::xml_document m_xmlSkinDesc;
};

}//namespace SOUI