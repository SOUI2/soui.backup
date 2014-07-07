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

class SOUI_EXP SStringPool :public SSingletonMap<SStringPool,SStringW,SStringW>
{
public:
    BOOL BuildString(SStringW &strContainer);
    BOOL Init(pugi::xml_node xmlNode);
    SStringW Get(const SStringW & strName);
};

}//namespace SOUI