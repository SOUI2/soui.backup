//////////////////////////////////////////////////////////////////////////
//   File Name: bkstringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//                2012.8.28 - 2.0 huang jianxiogn
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "core/SSingletonMap.h"

namespace SOUI
{

class SOUI_EXP SStringPool :public SCmnMap<SStringT,SStringT>
{
public:
    BOOL BuildString(SStringT &strContainer);
    BOOL Init(pugi::xml_node xmlNode);
    LPCTSTR Get(const SStringT & strName);
protected:
    SStringT    m_strTmp;
};

}//namespace SOUI