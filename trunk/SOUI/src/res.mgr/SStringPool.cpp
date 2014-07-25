//////////////////////////////////////////////////////////////////////////
//   File Name: sstringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "souistd.h"
#include "res.mgr/Sstringpool.h"
#include "core/mybuffer.h"

namespace SOUI
{

    template<> SStringPool * SSingleton<SStringPool>::ms_Singleton =0;

BOOL SStringPool::BuildString(SStringW &strContainer)
{
    BOOL bRet=FALSE;
    int nSubStringStart=-1;
    int nSubStringEnd=0;
    while ((nSubStringStart = strContainer.Find(L"%", nSubStringEnd))!=-1)
    {
        nSubStringEnd = strContainer.Find(L"%", nSubStringStart + 1);
        if(nSubStringEnd==-1)
            break;
        SStringW strName=strContainer.Mid(nSubStringStart+1,nSubStringEnd-nSubStringStart-1);

        SStringW strNewSub=GetKeyObject(strName);
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

    for (pugi::xml_node xmlStr=xmlNode.first_child(); xmlStr; xmlStr=xmlStr.next_sibling())
    {
        SStringW strName=xmlStr.name();
        SStringW str=xmlStr.attribute(L"value").value();
        AddKeyObject(strName,str);
    }
    return TRUE;
}

SStringW SStringPool::Get(const SStringW & strName)
{
    SStringW strRet;
    if(HasKey(strName))
    {
        strRet=GetKeyObject(strName);
        BuildString(strRet);
    }
    return strRet;
}

}//namespace SOUI