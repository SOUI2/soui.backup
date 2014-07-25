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

        BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);

        HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);

        HCURSOR LoadCursor(LPCTSTR pszResName);

        HBITMAP    LoadBitmap(LPCTSTR pszResName);
        
        IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);

        IImgX * LoadImgX(LPCTSTR strType,LPCTSTR pszResName);

        size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);

        BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

        LPCTSTR FindImageType(LPCTSTR pszImgName);
        
    protected:
        IResProvider * GetMatchResProvider(LPCTSTR pszType,LPCTSTR pszResName);
        LPCTSTR SysCursorName2ID(LPCTSTR pszCursorName);
        
        SList<IResProvider*> m_lstResProvider;
        
        typedef SMap<SStringT,HCURSOR> CURSORMAP;
        CURSORMAP  m_mapCachedCursor;
    };
}
