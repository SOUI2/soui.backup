// translator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../include/translator.h"
#include <search.h>

using namespace pugi;

namespace SOUI
{
    int StringCmp(const SStringW &str1,const SStringW &str2)
    {
        if(str1 == str2) return 0;
        else return str1<str2?-1:1;
    }
    int SStrMap::Compare( const void * e1, const void * e2)
    {
        SStrMap **p1=(SStrMap**) e1;
        SStrMap **p2=(SStrMap**) e2;
        return StringCmp((*p1)->strSource,(*p2)->strSource);
    }


    int SStrMapEntry::Compare( const void * e1, const void * e2 )
    {
        SStrMapEntry **p1=(SStrMapEntry**) e1;
        SStrMapEntry **p2=(SStrMapEntry**) e2;
        return StringCmp((*p1)->strCtx,(*p2)->strCtx);
    }

    SStrMapEntry::~SStrMapEntry()
    {
        for(UINT i=0;i<m_arrStrMap.GetCount();i++)
            delete m_arrStrMap.GetAt(i);
    }


    //////////////////////////////////////////////////////////////////////////
    // SLang
    SLang::SLang()
    {
        m_arrEntry = new SArray<SStrMapEntry*>;
    }

    SLang::~SLang()
    {
        for(UINT i=0;i<m_arrEntry->GetCount();i++)
            delete m_arrEntry->GetAt(i);
        delete m_arrEntry;
    }
    
    BOOL SLang::Load( LPCTSTR pszFileName )
    {
        return FALSE;
    }

    BOOL SLang::LoadXML( pugi::xml_node xmlLang )
    {
        m_strLang=xmlLang.attribute(L"name").value();
        int ctxCount=0;
        xml_node nodeCtx=xmlLang.child(L"context");
        while(nodeCtx)
        {
            ctxCount++;
            nodeCtx=nodeCtx.next_sibling(L"context");
        }
        m_arrEntry->SetCount(ctxCount);
        nodeCtx=xmlLang.child(L"context");
        for(int i=0;i<ctxCount;i++)
        {
            ASSERT(nodeCtx);
            int strCount=0;
            xml_node nodeStr=nodeCtx.child(L"message");
            while(nodeStr)
            {
                strCount++;
                nodeStr=nodeStr.next_sibling(L"message");
            }
            
            SStrMapEntry * strMapEntry = new SStrMapEntry;
            strMapEntry->strCtx=nodeCtx.attribute(L"name").value();
            strMapEntry->m_arrStrMap.SetCount(strCount);
            nodeStr=nodeCtx.child(L"message");
            for(int j=0;j<strCount;j++)
            {
                ASSERT(nodeStr);
                SStrMap * strMap= new SStrMap;
                strMap->strSource=nodeStr.child(L"source").text().get();
                strMap->strTranslation=nodeStr.child(L"translation").text().get();
                strMapEntry->m_arrStrMap.SetAt(j,strMap);
                nodeStr=nodeStr.next_sibling(L"message");
            }
            qsort(strMapEntry->m_arrStrMap.GetData(),strMapEntry->m_arrStrMap.GetCount(),sizeof(SStrMap*),SStrMap::Compare);
            m_arrEntry->SetAt(i,strMapEntry);
            nodeCtx=nodeCtx.next_sibling(L"context");
        }
        
        qsort(m_arrEntry->GetData(),m_arrEntry->GetCount(),sizeof(SStrMapEntry*),SStrMapEntry::Compare);
        return TRUE;
    }


    //////////////////////////////////////////////////////////////////////////
    //  STranslator
    void STranslator::InstallLang( SLang *pLang )
    {
        m_lstLang->AddHead(pLang);
    }

    void STranslator::UninstallLang( SLang *pLang )
    {
        POSITION pos=m_lstLang->GetHeadPosition();
        while(pos)
        {
            POSITION posBackup=pos;
            SLang *p=m_lstLang->GetNext(pos);
            if(p==pLang)
            {
                m_lstLang->RemoveAt(posBackup);
                break;
            }
        }
    }

    STranslator::STranslator( void )
    {
        m_lstLang=new SList<SLang*>;
        m_ctxStack = new SList<SStringW>;
    }

    STranslator::~STranslator( void )
    {
        delete m_lstLang;
        delete m_ctxStack;
    }

    void STranslator::PushContext( const SStringW &strCtx )
    {
        m_ctxStack->AddHead(strCtx);
    }

    SStringW STranslator::PopContext()
    {
        return m_ctxStack->RemoveHead();
    }

    SStringW STranslator::tr(const SStringW & str )
    {
        if(m_ctxStack->IsEmpty()) return L"";
        SStrMapEntry keyEntry;
        keyEntry.strCtx=m_ctxStack->GetHead();
        SStrMapEntry *pKeyEntry=&keyEntry;
        SStrMap keyMap;
        keyMap.strSource=str;
        SStrMap *pKeyMap=&keyMap;
        POSITION pos=m_lstLang->GetHeadPosition();
        while(pos)
        {
            SLang *pLang=m_lstLang->GetNext(pos);
            SStrMapEntry** pEntry = (SStrMapEntry**)bsearch(&pKeyEntry,pLang->m_arrEntry->GetData(),pLang->m_arrEntry->GetCount(),sizeof(SStrMapEntry*),SStrMapEntry::Compare);
            if(pEntry)
            {
                SStrMap ** pMap=(SStrMap**)bsearch(&pKeyMap,(*pEntry)->m_arrStrMap.GetData(),(*pEntry)->m_arrStrMap.GetCount(),sizeof(SStrMap*),SStrMap::Compare);
                if(pMap) return (*pMap)->strTranslation;
            }
        }
        return L"";
    }

}
