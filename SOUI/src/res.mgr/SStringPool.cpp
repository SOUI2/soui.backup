//////////////////////////////////////////////////////////////////////////
//   File Name: sstringpool.h
// Description: String Pool
//     Creator: Zhang Xiaoxuan
//     Version: 2009.5.13 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "souistd.h"
#include "res.mgr/Sstringpool.h"

namespace SOUI
{

    template<> SStringPool * SSingleton<SStringPool>::ms_Singleton =0;

BOOL SStringPool::BuildString(SStringT &strContainer)
{
    BOOL bRet=FALSE;
    int nSubStringStart=-1;
    int nSubStringEnd=0;
    while ((nSubStringStart = strContainer.Find(_T("%"), nSubStringEnd))!=-1)
    {
        nSubStringEnd = strContainer.Find(_T("%"), nSubStringStart + 1);
        if(nSubStringEnd==-1)
            break;
        SStringT strName=strContainer.Mid(nSubStringStart+1,nSubStringEnd-nSubStringStart-1);

        if(HasKey(strName))
        {
            SStringT strNewSub=GetKeyObject(strName);
            strContainer = strContainer.Left(nSubStringStart)
                + strNewSub
                + strContainer.Mid(nSubStringEnd+1);
            nSubStringEnd+=strNewSub.GetLength()-(nSubStringEnd-nSubStringStart);
        }else
        {//不是有效的内部变量，不做转义
            nSubStringEnd++;
        }
        bRet=TRUE;
    }
    return bRet;
}

BOOL SStringPool::Init( pugi::xml_node xmlNode )
{
    if(!xmlNode) return FALSE;
    if (wcsicmp(xmlNode.name(), L"string") != 0)
    {
        SASSERT(FALSE);
        return FALSE;
    }

    for (pugi::xml_node xmlStr=xmlNode.first_child(); xmlStr; xmlStr=xmlStr.next_sibling())
    {
        SStringT strName=S_CW2T(xmlStr.name());
        SStringT str=S_CW2T(xmlStr.attribute(L"value").value());
        AddKeyObject(strName,str);
    }
    return TRUE;
}

SStringT SStringPool::Get(const SStringT & strName)
{
    SStringT strRet;
    if(HasKey(strName))
    {
        strRet=GetKeyObject(strName);
        BuildString(strRet);
    }
    return strRet;
}

//////////////////////////////////////////////////////////////////////////
template<> SNamedID * SSingleton<SNamedID>::ms_Singleton =0;

// SNamedID
int SNamedID::funCompare(const  void * p1, const void * p2 )
{
    const NAMEDID * namedid1 = (const NAMEDID*)p1;
    const NAMEDID * namedid2 = (const NAMEDID*)p2;
    return namedid2->strName.Compare(namedid1->strName);
}

BOOL SNamedID::Init( pugi::xml_node xmlNode )
{
    if(!xmlNode) return FALSE;
    if (wcsicmp(xmlNode.name(), L"id") != 0)
    {
        SASSERT(FALSE);
        return FALSE;
    }

    for (pugi::xml_node xmlStr=xmlNode.first_child(); xmlStr; xmlStr=xmlStr.next_sibling())
    {
        NAMEDID namedID;
        namedID.strName=S_CW2T(xmlStr.name());
        namedID.nID=xmlStr.attribute(L"value").as_int(0);
        m_lstNamedID.Add(namedID);
    }
    qsort(m_lstNamedID.GetData(),m_lstNamedID.GetCount(),sizeof(NAMEDID),funCompare);
    return TRUE;
}


int SNamedID::String2ID( const SStringW &strName )
{
    NAMEDID target = {strName,0};
    NAMEDID *pFind = (NAMEDID *)bsearch(&target,m_lstNamedID.GetData(),m_lstNamedID.GetCount(),sizeof(NAMEDID),funCompare);
    if(!pFind) return 0;
    return pFind->nID;
}

}//namespace SOUI