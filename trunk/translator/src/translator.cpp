// translator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../include/translator.h"
#include <search.h>
#include <ObjBase.h>

using namespace pugi;

namespace SOUI
{
    int StringCmp(const SStringW &str1,const SStringW &str2)
    {
        if(str1 == str2) return 0;
        else return str1<str2?-1:1;
    }

    class SStrMap
    {
        friend class STranslator;
    public:
        SStringW strSource;
        SStringW strTranslation;

        static int  Compare(const void * e1, const void * e2);
        static int  CompareInSearch(const void * e1, const void * e2);
    };

    class SStrMapEntry
    {
        friend class STranslator;
    public:
        ~SStrMapEntry();
        SStringW strCtx;
        SArray<SStrMap*> m_arrStrMap;
        static int  Compare(const void * e1, const void * e2);
        static int  CompareInSearch(const void * e1, const void * e2);
    };


    int SStrMap::Compare( const void * e1, const void * e2)
    {
        SStrMap **p1=(SStrMap**) e1;
        SStrMap **p2=(SStrMap**) e2;
        return StringCmp((*p1)->strSource,(*p2)->strSource);
    }

    int SStrMap::CompareInSearch( const void * e1, const void * e2 )
    {
        SStringW * pKey=(SStringW *)e1;
        SStrMap **p2=(SStrMap**) e2;
        return StringCmp(*pKey,(*p2)->strSource);     
    }


    int SStrMapEntry::Compare( const void * e1, const void * e2 )
    {
        SStrMapEntry **p1=(SStrMapEntry**) e1;
        SStrMapEntry **p2=(SStrMapEntry**) e2;
        return StringCmp((*p1)->strCtx,(*p2)->strCtx);
    }
    
    int SStrMapEntry::CompareInSearch( const void * e1, const void * e2 )
    {
        SStringW *pKey=(SStringW*) e1;
        SStrMapEntry **p2=(SStrMapEntry**) e2;
        return StringCmp(*pKey,(*p2)->strCtx);
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

    SStringW SLang::name()
    {
        return m_strLang;
    }

    GUID SLang::guid()
    {
        return m_guid;
    }

    BOOL SLang::Load( LPVOID pData,UINT uType )
    {
        switch(uType)
        {
        case LD_XML:
            return LoadFromXml((*(pugi::xml_node*)pData));
        }
        return FALSE;
    }
    
    BOOL SLang::LoadFromXml( pugi::xml_node xmlLang )
    {
        m_strLang=xmlLang.attribute(L"name").value();
        
        OLECHAR szIID[100] = { 0 };
        wcscpy(szIID,xmlLang.attribute(L"guid").value());

        IIDFromString(szIID,&m_guid);
        
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

    BOOL SLang::tr( const SStringW & strSrc,const SStringW & strCtx,SStringW & strRet )
    {
        SStrMapEntry** pEntry = (SStrMapEntry**)bsearch(&strCtx,m_arrEntry->GetData(),m_arrEntry->GetCount(),sizeof(SStrMapEntry*),SStrMapEntry::CompareInSearch);
        if(pEntry)
        {
            SStrMap ** pMap=(SStrMap**)bsearch(&strSrc,(*pEntry)->m_arrStrMap.GetData(),(*pEntry)->m_arrStrMap.GetCount(),sizeof(SStrMap*),SStrMap::CompareInSearch);
            if(pMap)
            {
                strRet=(*pMap)->strTranslation;
                return TRUE;
            }
        }
        return FALSE;
    }


    //////////////////////////////////////////////////////////////////////////
    //  STranslator
    BOOL STranslator::InstallLang(ILang *pLang)
    {
        POSITION pos=m_lstLang->GetHeadPosition();
        while(pos)
        {
            ILang *p=m_lstLang->GetNext(pos);
            if(IsEqualGUID(pLang->guid(),p->guid()))
            {
                return FALSE;
            }
        }
        m_lstLang->AddHead(pLang);
        pLang->AddRef();
        return TRUE;
    }

    BOOL STranslator::UninstallLang(REFGUID id)
    {
        POSITION pos=m_lstLang->GetHeadPosition();
        while(pos)
        {
            POSITION posBackup=pos;
            ILang *p=m_lstLang->GetNext(pos);
            if(IsEqualGUID(id,p->guid()))
            {
                m_lstLang->RemoveAt(posBackup);
                p->Release();
                return TRUE;
            }
        }
        return FALSE;
    }

    STranslator::STranslator( void )
    {
        m_lstLang=new SList<ILang*>;
    }

    STranslator::~STranslator( void )
    {
        POSITION pos=m_lstLang->GetHeadPosition();
        while(pos)
        {
            ILang *pLang=m_lstLang->GetNext(pos);
            pLang->Release();
        }
        delete m_lstLang;
    }

    SStringW STranslator::tr(const SStringW & strSrc,const SStringW & strCtx)
    {
        if(strSrc.IsEmpty()) return strSrc;
        SStringW strRet;
        POSITION pos=m_lstLang->GetHeadPosition();
        while(pos)
        {
            ILang *pLang=m_lstLang->GetNext(pos);
            if(pLang->tr(strSrc,strCtx,strRet)) return strRet;
        }
        return strSrc;
    }

    BOOL STranslator::CreateLang( ILang ** ppLang )
    {
        *ppLang = new SLang;
        return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
    //  
    BOOL CreateTranslator( ITranslator **ppTrans )
    {
        *ppTrans = new STranslator;
        return TRUE;
    }

}
