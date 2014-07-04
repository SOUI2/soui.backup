//////////////////////////////////////////////////////////////////////////
//   File Name: duistringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "souistd.h"
#include "SApp.h"
#include "res.mgr/Sstringpool.h"
#include "core/mybuffer.h"

namespace SOUI
{


BOOL SStringPool::BuildString(SStringT &strContainer)
{
    BOOL bRet=FALSE;
    int nSubStringStart=-1;
    int nSubStringEnd=0;
    while ((nSubStringStart = strContainer.Find(_T("%"), nSubStringEnd))!=-1)
    {
        nSubStringEnd = strContainer.Find(_T('%'), nSubStringStart + 1);
        if(nSubStringEnd==-1)
            break;
        SStringT strName=strContainer.Mid(nSubStringStart+1,nSubStringEnd-nSubStringStart-1);

        SStringT strNewSub=GetKeyObject(strName);
        strContainer = strContainer.Left(nSubStringStart)
                       + strNewSub
                       + strContainer.Mid(nSubStringEnd+1);
        nSubStringEnd+=strNewSub.GetLength()-(nSubStringEnd-nSubStringStart);
        bRet=TRUE;
    }
    return bRet;
}

BOOL SStringPool::Init( pugi::xml_node xmlNode )
{
    if (wcscmp(xmlNode.name(), L"string") != 0)
    {
        ASSERT(FALSE);
        return FALSE;
    }
    UINT uStringID = 0;

    for (pugi::xml_node xmlStr=xmlNode.first_child(); xmlStr; xmlStr=xmlStr.next_sibling())
    {
        SStringT strName=DUI_CW2T(xmlStr.name());
        SStringT str=DUI_CW2T(xmlStr.attribute(L"value").value());
        AddKeyObject(strName,str);
    }
    return TRUE;
}

LPCTSTR SStringPool::Get(const SStringT & strName)
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