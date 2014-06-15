#include "duistd.h"
#include "res.mgr/DuiResProviderMgr.h"

namespace SOUI
{
    DuiResProviderMgr::DuiResProviderMgr()
    {
    }

    DuiResProviderMgr::~DuiResProviderMgr(void)
    {
    }

    IDuiResProvider * DuiResProviderMgr::GetMatchResProvider( LPCTSTR pszType,LPCTSTR pszResName )
    {
        POSITION pos=m_lstResProvider.GetHeadPosition();
        while(pos)
        {
            IDuiResProvider * pResProvider=m_lstResProvider.GetNext(pos);
            if(pResProvider->HasResource(pszType,pszResName)) return pResProvider;
        }
        return NULL;
    }

    void DuiResProviderMgr::AddResProvider( IDuiResProvider * pResProvider )
    {
        m_lstResProvider.AddHead(pResProvider);
    }

    void DuiResProviderMgr::RemoveResProvider( IDuiResProvider * pResProvider )
    {
        POSITION pos=m_lstResProvider.Find(pResProvider);
        if(pos)  m_lstResProvider.RemoveAt(pos);
    }


}
