//////////////////////////////////////////////////////////////////////////
//   File Name: bkstringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//                2012.8.28 - 2.0 huang jianxiogn
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "core/SSingletonMap.h"

#define BUILDSTRING(p1) SStringPool::getSingleton().BuildString(p1)

namespace SOUI
{

    class SOUI_EXP SStringPool :public SSingletonMap<SStringPool,SStringT,SStringT>
    {
    public:
        BOOL BuildString(SStringT &strContainer);
        BOOL Init(pugi::xml_node xmlNode);
        SStringT Get(const SStringT & strName);
    };

    class SOUI_EXP SNamedID : public  SSingleton<SNamedID>
    {
        struct NAMEDID
        {
            SStringW strName;
            int      nID;
        };
    public:
        BOOL Init(pugi::xml_node xmlNode);
        int  String2ID(const SStringW &strName);
    protected:
        static int funCompare(const void * p1,const void * p2);
        SArray<NAMEDID> m_lstNamedID;
    };
}//namespace SOUI