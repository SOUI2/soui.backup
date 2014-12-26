#pragma once

#include "interface/sresprovider-i.h"
#include "atl.mini/scomcli.h"
#include "helper/SCriticalSection.h"
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
        /*virtual */BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);

        /*virtual */HCURSOR LoadCursor(LPCTSTR pszResName);

        /*virtual */HBITMAP    LoadBitmap(LPCTSTR pszResName);
        
        /*virtual */IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */IImgX * LoadImgX(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);

        /*virtual */BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

        /*virtual */LPCTSTR FindImageType(LPCTSTR pszImgName);
        
    public://helper
        //使用type:name形式的字符串加载图片，如果没有type,则自动查找type
        IBitmap * LoadImage2(const SStringW & strImgID);
        
        //使用name:size形式的字符串加载图标，如果没有size,则默认系统图标SIZE
        HICON     LoadIcon2(const SStringW & strIconID);
    protected:
        IResProvider * GetMatchResProvider(LPCTSTR pszType,LPCTSTR pszResName);
        LPCTSTR SysCursorName2ID(LPCTSTR pszCursorName);
        
        SList<IResProvider*> m_lstResProvider;
        
        typedef SMap<SStringT,HCURSOR> CURSORMAP;
        CURSORMAP  m_mapCachedCursor;

        SCriticalSection    m_cs;
    };
}
