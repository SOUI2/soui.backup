//////////////////////////////////////////////////////////////////////////
// RichEditObj的简单工厂

#pragma once
#include "souicoll.h"

class RichEditObj;
class RichEditObjFactory
{
public:
    RichEditObjFactory();
    ~RichEditObjFactory();

    typedef RichEditObj * (*pfnCreateObj)(); 

    RichEditObj * CreateObjectByName(LPCWSTR pszName)
    {
        MapCreater::CPair * p = m_mapCreater.Lookup(pszName);
        if (!p)return NULL;
        return p->m_value();
    }

    void Register(LPCWSTR pszName, pfnCreateObj pfn)
    {
        m_mapCreater[pszName] = pfn;
    }

    void UnRegister(LPCWSTR pszName)
    {
        m_mapCreater.RemoveKey(pszName);
    }

    static RichEditObjFactory& GetInstance()
    {
        static RichEditObjFactory factory;
        return factory;
    }

private:
    typedef SMap<SStringW,pfnCreateObj> MapCreater;
    MapCreater m_mapCreater;
};
