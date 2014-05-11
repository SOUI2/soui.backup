//////////////////////////////////////////////////////////////////////////
//   File Name: bkstringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//				2012.8.28 - 2.0 huang jianxiogn
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "DuiSingletonMap.h"

namespace SOUI
{

class SOUI_EXP DuiStringPool :public DuiCmnMap<CDuiStringT,UINT>
{
public:
    BOOL BuildString(CDuiStringT &strContainer);
	BOOL Init(pugi::xml_node xmlNode);
    LPCTSTR Get(UINT uID);
protected:
    CDuiStringT	m_strTmp;
};

}//namespace SOUI