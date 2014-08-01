#include "souistd.h"
#include "res.mgr/SStylePool.h"

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    //SStylePool
    
    // Get style object from pool by class name
    BOOL SStylePool::GetStyle(LPCWSTR lpszName, SwndStyle& style)
    {
        if(!HasKey(lpszName)) return FALSE;
        style=GetKeyObject(lpszName);
        return TRUE;
    }

    // Load style-pool from xml tree
    BOOL SStylePool::Init(pugi::xml_node xmlStyleRoot)
    {
        if(!xmlStyleRoot) return FALSE;

        if (wcscmp(xmlStyleRoot.name(), L"style") != 0)
        {
            ASSERT(FALSE);
            return FALSE;
        }

        LPCWSTR lpszClassName = NULL;

        for (pugi::xml_node xmlChild=xmlStyleRoot.child(L"class"); xmlChild; xmlChild=xmlChild.next_sibling(L"class"))
        {
            lpszClassName = xmlChild.attribute(L"name").value();
            if (!lpszClassName)
                continue;

            GetKeyObject(lpszClassName).InitFromXml(xmlChild);
        }
        return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
    // SStylePoolMgr
    template<> SStylePoolMgr * SSingleton<SStylePoolMgr>::ms_Singleton=0;

    BOOL SStylePoolMgr::GetStyle( LPCWSTR lpszName,SwndStyle& style )
    {
        POSITION pos=m_lstStylePools.GetTailPosition();
        while(pos)
        {
            SStylePool *pStylePool=m_lstStylePools.GetPrev(pos);
            if(pStylePool->GetStyle(lpszName,style)) return TRUE;
        }
        return FALSE;
    }

    void SStylePoolMgr::PushStylePool( SStylePool *pStylePool )
    {
        m_lstStylePools.AddTail(pStylePool);
        pStylePool->AddRef();
    }

    SStylePool * SStylePoolMgr::PopStylePool(SStylePool *pStylePool)
    {
        SStylePool * pRet=NULL;
        if(pStylePool)
        {
            POSITION pos=m_lstStylePools.Find(pStylePool);
            if(pos)
            {
                pRet=m_lstStylePools.GetAt(pos);
                m_lstStylePools.RemoveAt(pos);
            }
        }else
        {
            pRet = m_lstStylePools.RemoveTail();
        }
        if(pRet) pRet->Release();
        return pRet;
    }

    SStylePoolMgr::~SStylePoolMgr()
    {
        POSITION pos=m_lstStylePools.GetHeadPosition();
        while(pos)
        {
            SStylePool *p = m_lstStylePools.GetNext(pos);
            p->Release();
        }
        m_lstStylePools.RemoveAll();
    }

}//end of namespace SOUI
