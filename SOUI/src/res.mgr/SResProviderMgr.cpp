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

    LPCTSTR SResProviderMgr::SysCursorName2ID( LPCTSTR pszCursorName )
    {
        if(!_wcsicmp(pszCursorName,L"arrow"))
            return IDC_ARROW;
        if(!_wcsicmp(pszCursorName,L"ibeam"))
            return IDC_IBEAM;
        if(!_wcsicmp(pszCursorName,L"wait"))
            return IDC_WAIT;
        if(!_wcsicmp(pszCursorName,L"cross"))
            return IDC_CROSS;
        if(!_wcsicmp(pszCursorName,L"uparrow"))
            return IDC_UPARROW;
        if(!_wcsicmp(pszCursorName,L"size"))
            return IDC_SIZE;
        if(!_wcsicmp(pszCursorName,L"sizenwse"))
            return IDC_SIZENWSE;
        if(!_wcsicmp(pszCursorName,L"sizenesw"))
            return IDC_SIZENESW;
        if(!_wcsicmp(pszCursorName,L"sizewe"))
            return IDC_SIZEWE;
        if(!_wcsicmp(pszCursorName,L"sizens"))
            return IDC_SIZENS;
        if(!_wcsicmp(pszCursorName,L"sizeall"))
            return IDC_SIZEALL;
        if(!_wcsicmp(pszCursorName,L"no"))
            return IDC_NO;
        if(!_wcsicmp(pszCursorName,L"hand"))
            return IDC_HAND;
        if(!_wcsicmp(pszCursorName,L"help"))
            return IDC_HELP;
        return NULL;
   }

}
