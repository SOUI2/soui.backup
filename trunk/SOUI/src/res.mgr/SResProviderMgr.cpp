#include "souistd.h"
#include "res.mgr/SResProviderMgr.h"

namespace SOUI
{
    SResProviderMgr::SResProviderMgr()
    {
    }

    SResProviderMgr::~SResProviderMgr(void)
    {
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
    }

    void SResProviderMgr::RemoveResProvider( IResProvider * pResProvider )
    {
        POSITION pos=m_lstResProvider.Find(pResProvider);
        if(pos)  m_lstResProvider.RemoveAt(pos);
    }



}
