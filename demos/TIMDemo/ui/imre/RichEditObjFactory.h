//////////////////////////////////////////////////////////////////////////
// RichEditObj的简单工厂

#pragma once
#include "souicoll.h"
#include <map>

namespace SOUI
{

    class RichEditObj;
    class RichEditObjFactory
    {
    public:
        RichEditObjFactory();
        ~RichEditObjFactory();

        typedef RichEditObj * (*pfnCreateObj)();

        RichEditObj * CreateObjectByName(LPCWSTR pszName)
        {
            CreaterMap::iterator it = _creaters.find(pszName);
            if (it == _creaters.end())
            {
                return NULL;
            }

            return it->second();
        }

        void Register(LPCWSTR pszName, pfnCreateObj pfn)
        {
            _creaters[pszName] = pfn;
        }

        void UnRegister(LPCWSTR pszName)
        {
            CreaterMap::iterator it = _creaters.find(pszName);
            if (it == _creaters.end())
            {
                _creaters.erase(it);
            }
        }

        static RichEditObjFactory& GetInstance()
        {
            static RichEditObjFactory factory;
            return factory;
        }

    private:

        typedef std::map<SStringW, pfnCreateObj> CreaterMap;
        CreaterMap _creaters;
    };

}; // namespace SOUI
