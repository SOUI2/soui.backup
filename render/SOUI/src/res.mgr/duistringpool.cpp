//////////////////////////////////////////////////////////////////////////
//   File Name: duistringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "res.mgr/duistringpool.h"
#include "DuiSystem.h"
#include "mybuffer.h"

namespace SOUI
{


BOOL DuiStringPool::BuildString(CDuiStringT &strContainer)
{
    BOOL bRet=FALSE;
    int nSubStringStart=-1;
    int nSubStringEnd=0;
    while ((nSubStringStart = strContainer.Find(_T("%"), nSubStringEnd))!=-1)
    {
        nSubStringEnd = strContainer.Find(_T('%'), nSubStringStart + 1);
        if(nSubStringEnd==-1)
            break;
        CDuiStringT strName=strContainer.Mid(nSubStringStart+1,nSubStringEnd-nSubStringStart-1);

        CDuiStringT strNewSub=GetKeyObject(strName);
        strContainer = strContainer.Left(nSubStringStart)
                       + strNewSub
                       + strContainer.Mid(nSubStringEnd+1);
        nSubStringEnd+=strNewSub.GetLength()-(nSubStringEnd-nSubStringStart);
        bRet=TRUE;
    }
    return bRet;
}

BOOL DuiStringPool::Init( pugi::xml_node xmlNode )
{
    if (strcmp(xmlNode.name(), "string") != 0)
    {
        DUIASSERT(FALSE);
        return FALSE;
    }
    UINT uStringID = 0;

    for (pugi::xml_node xmlStr=xmlNode.first_child(); xmlStr; xmlStr=xmlStr.next_sibling())
    {
        CDuiStringT strName=DUI_CA2T(xmlStr.name(),CP_UTF8);
        CDuiStringT str=DUI_CA2T(xmlStr.attribute("value").value(),CP_UTF8);
        AddKeyObject(strName,str);
    }
    return TRUE;
}

LPCTSTR DuiStringPool::Get(const CDuiStringT & strName)
{
    m_strTmp=_T("");
    if(HasKey(strName))
    {
        m_strTmp=GetKeyObject(strName);
        BuildString(m_strTmp);
    }
    return m_strTmp;
}

}//namespace SOUI