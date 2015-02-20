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
        BOOL HasResource(LPCTSTR pszType,LPCTSTR pszResName);

        HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0,BOOL bFromFile = FALSE);

        HCURSOR LoadCursor(LPCTSTR pszResName,BOOL bFromFile = FALSE);

        HBITMAP LoadBitmap(LPCTSTR pszResName,BOOL bFromFile = FALSE);
        
        IBitmap * LoadImage(LPCTSTR pszType,LPCTSTR pszResName);

        IImgX * LoadImgX(LPCTSTR pszType,LPCTSTR pszResName);

        size_t GetRawBufferSize(LPCTSTR pszType,LPCTSTR pszResName);

        BOOL GetRawBuffer(LPCTSTR pszType,LPCTSTR pszResName,LPVOID pBuf,size_t size);
        
    public://helper
        //使用type:name形式的字符串加载图片，如果没有type,则自动查找type
        IBitmap * LoadImage2(const SStringW & strImgID);
        
        //使用name:size形式的字符串加载图标，如果没有size,则默认系统图标SIZE
        HICON     LoadIcon2(const SStringW & strIconID);
    protected:
        IResProvider * GetMatchResProvider(LPCTSTR pszType,LPCTSTR pszResName);
        LPCTSTR SysCursorName2ID(LPCTSTR pszCursorName);

        //检查资源类型是否为文件
        BOOL    IsFileType(LPCTSTR pszType);

        SList<IResProvider*> m_lstResProvider;
        
        typedef SMap<SStringT,HCURSOR> CURSORMAP;
        CURSORMAP  m_mapCachedCursor;

        SCriticalSection    m_cs;
    };
}
