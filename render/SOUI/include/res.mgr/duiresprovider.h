//////////////////////////////////////////////////////////////////////////
//   File Name: duiresprovider.h
// Description: Resource Provider
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duiresprovider-i.h"

namespace SOUI
{


class SOUI_EXP DuiResProviderPE:public IResProvider
{
public:
    DuiResProviderPE(HINSTANCE hInst);
    HBITMAP    LoadBitmap(LPCTSTR pszResName);
    HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);
    HCURSOR   LoadCursor(LPCTSTR pszResName);
    IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);
    BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);

protected:
    HRSRC MyFindResource(LPCTSTR strType, LPCTSTR pszResName );
    HINSTANCE m_hResInst;
};


#define UISKIN_INDEX    _T("uiskin.idx")        //文件夹资源的文件映射表索引表文件名
class SOUI_EXP DuiResProviderFiles:public IResProvider
{
public:

    DuiResProviderFiles();

    BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);
    HBITMAP    LoadBitmap(LPCTSTR pszResName);
    HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);
    HCURSOR LoadCursor(LPCTSTR pszResName);
    IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

    BOOL Init(LPCTSTR pszPath);
protected:
    CDuiStringT GetRes( LPCTSTR strType,LPCTSTR pszResName );

    CDuiStringT m_strPath;
    CDuiMap<DuiResID,CDuiStringT> m_mapFiles;
};

}//namespace SOUI