#pragma once

#include "duiresprovider-i.h"
#include "atl.mini/duicomcli.h"

namespace SOUI
{
    class SOUI_EXP DuiResProviderMgr : public IDuiResProvider
    {
    public:
        DuiResProviderMgr(void);
        ~DuiResProviderMgr(void);
        
        void AddResProvider(IDuiResProvider * pResProvider);
        
        void RemoveResProvider(IDuiResProvider * pResProvider);
        
        //////////////////////////////////////////////////////////////////////////
        // DuiResProviderBase

        virtual BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName)
        {
            IDuiResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return FALSE;
            return TRUE;
        }

        virtual HICON   LoadIcon(LPCTSTR strType,LPCTSTR pszResName,int cx=0,int cy=0)
        {
            IDuiResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadIcon(strType,pszResName,cx,cy);
        }
        virtual HBITMAP    LoadBitmap(LPCTSTR strType,LPCTSTR pszResName)
        {
            IDuiResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadBitmap(strType,pszResName);
        }
        virtual IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName)
        {
            IDuiResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return NULL;
            return pResProvider->LoadImage(strType,pszResName);
        }

        virtual size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName)
        {
            IDuiResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return 0;
            return pResProvider->GetRawBufferSize(strType,pszResName);
        }

        virtual BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size)
        {
            IDuiResProvider *pResProvider=GetMatchResProvider(strType,pszResName);
            if(!pResProvider) return FALSE;
            return pResProvider->GetRawBuffer(strType,pszResName,pBuf,size);
        }

    protected:
        IDuiResProvider * GetMatchResProvider(LPCTSTR pszType,LPCTSTR pszResName);
        
        CDuiList<IDuiResProvider*> m_lstResProvider;
    };
}
