#pragma once

#include "interface/sresprovider-i.h"
#include "atl.mini/scomcli.h"

namespace SOUI
{
    class SOUI_EXP SResProviderMgr : public IResProvider
    {
    public:
        SResProviderMgr(void);
        ~SResProviderMgr(void);
        
        void AddResProvider(IResProvider * pResProvider);
        
        void RemoveResProvider(IResProvider * pResProvider);
        
        //////////////////////////////////////////////////////////////////////////
        // IResProvider

        /*virtual */BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);

        /*virtual */HCURSOR LoadCursor(LPCTSTR pszResName);

        /*virtual */HBITMAP    LoadBitmap(LPCTSTR pszResName);
        
        /*virtual */IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */IImgX * LoadImgX(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

        /*virtual */LPCTSTR FindImageType(LPCTSTR pszImgName);
        
        //////////////////////////////////////////////////////////////////////////
        //IObjRef
        /*virtual */void AddRef(){}
        /*virtual */void Release(){};
        /*virtual */void OnFinalRelease(){}
    protected:
        IResProvider * GetMatchResProvider(LPCTSTR pszType,LPCTSTR pszResName);
        LPCTSTR SysCursorName2ID(LPCTSTR pszCursorName);
        
        SList<IResProvider*> m_lstResProvider;
        
        typedef SMap<SStringT,HCURSOR> CURSORMAP;
        CURSORMAP  m_mapCachedCursor;
    };
}
