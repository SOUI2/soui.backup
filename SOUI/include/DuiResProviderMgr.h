#pragma once

#include "duiresprovider-i.h"
namespace SOUI
{
    class SOUI_EXP DuiResProviderMgr : public IDuiResProvider
    {
    public:
        DuiResProviderMgr(void);
        ~DuiResProviderMgr(void);
        
        void SetDefResProvider(IDuiResProvider * pDefResProvider)
        {
            m_pDefResProvider=pDefResProvider;
        }

        IDuiResProvider * GetDefResProvider(){return m_pDefResProvider;}

        void SetResProvider(IDuiResProvider * pResProvider)
        {
            m_pResProvider=pResProvider;
        }

        IDuiResProvider * GetResProvider(){return m_pResProvider;}

        //////////////////////////////////////////////////////////////////////////
        // DuiResProviderBase

        virtual BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName)
        {
            BOOL bRet=FALSE;
            if(m_pResProvider) bRet=m_pResProvider->HasResource(strType,pszResName);
            if(!bRet) bRet= m_pDefResProvider->HasResource(strType,pszResName);
            return bRet;
        }

        virtual HICON   LoadIcon(LPCTSTR strType,LPCTSTR pszResName,int cx=0,int cy=0)
        {
            HICON hIcon=0;
            if(m_pResProvider) hIcon=m_pResProvider->LoadIcon(strType,pszResName,cx,cy);
            if(!hIcon) hIcon=m_pDefResProvider->LoadIcon(strType,pszResName,cx,cy);
            return hIcon;
        }
        virtual HBITMAP    LoadBitmap(LPCTSTR strType,LPCTSTR pszResName)
        {
            HBITMAP hBmp=0;
            if(m_pResProvider) hBmp=m_pResProvider->LoadBitmap(strType,pszResName);
            if(!hBmp) hBmp=m_pDefResProvider->LoadBitmap(strType,pszResName);
            return hBmp;
        }
        virtual IDuiImage * LoadImage(LPCTSTR strType,LPCTSTR pszResName)
        {
            IDuiImage *pImg=0;
            if(m_pResProvider) pImg=m_pResProvider->LoadImage(strType,pszResName);
            if(!pImg) pImg=m_pDefResProvider->LoadImage(strType,pszResName);
            return pImg;
        }

        virtual size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName)
        {
            size_t sz=0;
            if(m_pResProvider) sz=m_pResProvider->GetRawBufferSize(strType,pszResName);
            if(!sz) sz=m_pDefResProvider->GetRawBufferSize(strType,pszResName);
            return sz;
        }

        virtual BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size)
        {
            BOOL bRet=FALSE;
            if(m_pResProvider) bRet=m_pResProvider->GetRawBuffer(strType,pszResName,pBuf,size);
            if(!bRet) bRet=m_pDefResProvider->GetRawBuffer(strType,pszResName,pBuf,size);
            return bRet;
        }

    protected:
        IDuiResProvider *m_pDefResProvider;
        IDuiResProvider *m_pResProvider;
    };
}
