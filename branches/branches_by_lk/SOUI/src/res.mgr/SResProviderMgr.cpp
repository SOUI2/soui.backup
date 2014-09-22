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

        pos = m_mapCachedCursor.GetStartPosition();
        while(pos)
        {
            CURSORMAP::CPair *pPair=m_mapCachedCursor.GetNext(pos);
            DeleteObject(pPair->m_value);
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
        if(!_tcsicmp(pszCursorName,_T("arrow")))
            return IDC_ARROW;
        if(!_tcsicmp(pszCursorName,_T("ibeam")))
            return IDC_IBEAM;
        if(!_tcsicmp(pszCursorName,_T("wait")))
            return IDC_WAIT;
        if(!_tcsicmp(pszCursorName,_T("cross")))
            return IDC_CROSS;
        if(!_tcsicmp(pszCursorName,_T("uparrow")))
            return IDC_UPARROW;
        if(!_tcsicmp(pszCursorName,_T("size")))
            return IDC_SIZE;
        if(!_tcsicmp(pszCursorName,_T("sizenwse")))
            return IDC_SIZENWSE;
        if(!_tcsicmp(pszCursorName,_T("sizenesw")))
            return IDC_SIZENESW;
        if(!_tcsicmp(pszCursorName,_T("sizewe")))
            return IDC_SIZEWE;
        if(!_tcsicmp(pszCursorName,_T("sizens")))
            return IDC_SIZENS;
        if(!_tcsicmp(pszCursorName,_T("sizeal_T(")))
            return IDC_SIZEALL;
        if(!_tcsicmp(pszCursorName,_T("no")))
            return IDC_NO;
        if(!_tcsicmp(pszCursorName,_T("hand")))
            return IDC_HAND;
        if(!_tcsicmp(pszCursorName,_T("help")))
            return IDC_HELP;
        return NULL;
   }

    LPCTSTR SResProviderMgr::FindImageType( LPCTSTR pszImgName )
    {
        POSITION pos=m_lstResProvider.GetHeadPosition();
        while(pos)
        {
            IResProvider* pResProvider=m_lstResProvider.GetNext(pos);
            LPCTSTR pszType=pResProvider->FindImageType(pszImgName);
            if(pszType) return pszType;
        }
        return NULL;
    }

    BOOL SResProviderMgr::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
    {
        IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
        if(!pResProvider) return FALSE;
        return pResProvider->GetRawBuffer(strType,pszResName,pBuf,size);
    }

    size_t SResProviderMgr::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
    {
        IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
        if(!pResProvider) return 0;
        return pResProvider->GetRawBufferSize(strType,pszResName);
    }

    IImgX * SResProviderMgr::LoadImgX( LPCTSTR strType,LPCTSTR pszResName )
    {
        if(!strType) strType = FindImageType(pszResName);
        if(!strType) return NULL;
        IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
        if(!pResProvider) return NULL;
        return pResProvider->LoadImgX(strType,pszResName);
    }

    IBitmap * SResProviderMgr::LoadImage( LPCTSTR strType,LPCTSTR pszResName )
    {
        if(!strType) strType = FindImageType(pszResName);
        if(!strType) return NULL;
        IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
        if(!pResProvider) return NULL;
        return pResProvider->LoadImage(strType,pszResName);
    }

    HBITMAP SResProviderMgr::LoadBitmap( LPCTSTR pszResName )
    {
        IResProvider *pResProvider=GetMatchResProvider(_T("BITMAP"),pszResName);
        if(!pResProvider) return NULL;
        return pResProvider->LoadBitmap(pszResName);
    }

    HCURSOR SResProviderMgr::LoadCursor( LPCTSTR pszResName )
    {
        if(IS_INTRESOURCE(pszResName))
            return ::LoadCursor(NULL, pszResName);
        else 
        {
            LPCTSTR pszCursorID=SysCursorName2ID(pszResName);
            if(pszCursorID)
                return ::LoadCursor(NULL, pszCursorID);
        }
        const CURSORMAP::CPair * pPair  = m_mapCachedCursor.Lookup(pszResName);
        if(pPair) return pPair->m_value;
        
        IResProvider *pResProvider=GetMatchResProvider(_T("CURSOR"),pszResName);
        if(!pResProvider) return NULL;
        HCURSOR hRet =pResProvider->LoadCursor(pszResName);
        if(hRet)
        {
            m_mapCachedCursor[pszResName]=hRet;
        }
        return hRet;
    }

    HICON SResProviderMgr::LoadIcon( LPCTSTR pszResName,int cx/*=0*/,int cy/*=0*/ )
    {
        IResProvider *pResProvider=GetMatchResProvider(_T("ICON"),pszResName);
        if(!pResProvider) return NULL;
        return pResProvider->LoadIcon(pszResName,cx,cy);
    }

    BOOL SResProviderMgr::HasResource( LPCTSTR strType,LPCTSTR pszResName )
    {
        IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
        if(!pResProvider) return FALSE;
        return TRUE;
    }

}
