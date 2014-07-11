#include "souistd.h"
#include "res.mgr/SResProviderMgr.h"
#include "res.mgr/SResProvider.h"

namespace SOUI
{
    SResProviderMgr::SResProviderMgr()
    {
    }

    SResProviderMgr::~SResProviderMgr(void)
    {
        POSITION pos=m_lstResProvider.GetHeadPosition();
        while(pos)
        {
            IResProvider *pResProvider=m_lstResProvider.GetNext(pos);
            pResProvider->Release();
        }
    }

    IResProvider * SResProviderMgr::GetMatchResProvider( LPCTSTR pszType,LPCTSTR pszResName )
    {
        POSITION pos=m_lstResProvider.GetHeadPosition();
        while(pos)
        {
            IResProvider * pResProvider=m_lstResProvider.GetNext(pos);
            if(pResProvider->HasResource(pszType,pszResName)) return pResProvider;
        }
        return NULL;
    }
   
    void SResProviderMgr::AddResProvider( IResProvider * pResProvider )
    {
        m_lstResProvider.AddHead(pResProvider);
        pResProvider->AddRef();
    }

    void SResProviderMgr::RemoveResProvider( IResProvider * pResProvider )
    {
        POSITION pos=m_lstResProvider.Find(pResProvider);
        if(pos)
        {
            m_lstResProvider.RemoveAt(pos);
            pResProvider->Release();
        }
    }

}
