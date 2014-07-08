//////////////////////////////////////////////////////////////////////////
//   File Name: sresprovider.h
// Description: Resource Provider
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "interface/SResProvider-i.h"

namespace SOUI
{


class SOUI_EXP SResProviderPE:public IResProvider
{
public:
    SResProviderPE(HINSTANCE hInst);
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
class SOUI_EXP SResProviderFiles:public IResProvider
{
public:

    SResProviderFiles();

    BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);
    HBITMAP    LoadBitmap(LPCTSTR pszResName);
    HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);
    HCURSOR LoadCursor(LPCTSTR pszResName);
    IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

    BOOL Init(LPCTSTR pszPath);
protected:
    SStringT GetRes( LPCTSTR strType,LPCTSTR pszResName );

    SStringT m_strPath;
    SMap<SResID,SStringT> m_mapFiles;
};

}//namespace SOUI