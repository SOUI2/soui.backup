#pragma once

#include "DuiPools.h"

namespace SOUI
{
    class SOUI_EXP DuiPoolsStack
    {
    public:
        DuiPoolsStack(void);
        ~DuiPoolsStack(void);

        CDuiPools * GetCurResMgr();

        void Push(CDuiPools * pResMgr);

        CDuiPools * Pop();

        CDuiPools * CreateResMgr();

        void DestroyResMgr(CDuiPools *pResMgr);

        CDuiSkinBase * GetSkin(LPCSTR pszName);
        BOOL GetStyle(LPCSTR pszName,DuiStyle &style);
        pugi::xml_node GetObjDefAttr(LPCSTR pszName);
        void BuildString(CDuiStringT & str);

    protected:
        CDuiList<CDuiPools*>    m_lstResMgr;
    };
}//end of namespace
