#pragma once

#include "interface/sresprovider-i.h"
#include "atl.mini/scomcli.h"

namespace SOUI
{
    class SOUI_EXP SResProviderMgr
    {
    public:
        SResProviderMgr(void);
        ~SResProviderMgr(void);
        
        void AddResProvider(IResProvider * pResProvider);
        
        void RemoveResProvider(IResProvider * pResProvider);
        
        //////////////////////////////////////////////////////////////////////////
        // IResProvider

        BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName)
        {
            IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return FALSE;
            return TRUE;
        }

        HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0)
        {
            IResProvider *pResProvider=GetMatchResProvider(_T("ICON"),pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadIcon(pszResName,cx,cy);
        }

        HCURSOR LoadCursor(LPCTSTR pszResName)
        {
            if(pszResName>=IDC_ARROW && pszResName<=IDC_HELP)
                return ::LoadCursor(NULL,pszResName);
            IResProvider *pResProvider=GetMatchResProvider(_T("CURSOR"),pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadCursor(pszResName);
        }

        HBITMAP    LoadBitmap(LPCTSTR pszResName)
        {
            IResProvider *pResProvider=GetMatchResProvider(_T("BITMAP"),pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadBitmap(pszResName);
        }
        
        IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName)
        {
            if(!strType) strType = FindImageType(pszResName);
            if(!strType) return NULL;
            IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadImage(strType,pszResName);
        }

        IImgX * LoadImgX(LPCTSTR strType,LPCTSTR pszResName)
        {
            if(!strType) strType = FindImageType(pszResName);
            if(!strType) return NULL;
            IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadImgX(strType,pszResName);
        }

        size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName)
        {
            IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return 0;
            return pResProvider->GetRawBufferSize(strType,pszResName);
        }

        BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size)
        {
            IResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return FALSE;
            return pResProvider->GetRawBuffer(strType,pszResName,pBuf,size);
        }

        LPCTSTR FindImageType(LPCTSTR pszImgName)
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
        
    protected:
        IResProvider * GetMatchResProvider(LPCTSTR pszType,LPCTSTR pszResName);
        
        SList<IResProvider*> m_lstResProvider;
    };
}
