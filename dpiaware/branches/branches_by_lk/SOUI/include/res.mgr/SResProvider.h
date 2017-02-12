//////////////////////////////////////////////////////////////////////////
//   File Name: sresprovider.h
// Description: Resource Provider
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "interface/SResProvider-i.h"
#include <helper/SResID.h>
#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{


class SResProviderPE:public TObjRefImpl<IResProvider>
{
public:
    SResProviderPE();
    BOOL Init(WPARAM wParam,LPARAM lParam);
    HBITMAP    LoadBitmap(LPCTSTR pszResName);
    HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);
    HCURSOR   LoadCursor(LPCTSTR pszResName);
    IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    IImgX   * LoadImgX(LPCTSTR strType,LPCTSTR pszResName);
    BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);
    BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);
    size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    LPVOID GetRawBufferPtr(LPCTSTR strType,LPCTSTR pszResName);
    LPCTSTR FindImageType(LPCTSTR pszImgName){return Helper_FindImageType(this,pszImgName);}
protected:
    HRSRC MyFindResource(LPCTSTR strType, LPCTSTR pszResName );
    HINSTANCE m_hResInst;
};


class SResProviderFiles:public TObjRefImpl<IResProvider>
{
public:

    SResProviderFiles();
    
    BOOL Init(WPARAM wParam,LPARAM lParam);

    BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);
    HBITMAP    LoadBitmap(LPCTSTR pszResName);
    HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0);
    HCURSOR LoadCursor(LPCTSTR pszResName);
    IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    IImgX   * LoadImgX(LPCTSTR strType,LPCTSTR pszResName);
    size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);
    LPCTSTR FindImageType(LPCTSTR pszImgName){return Helper_FindImageType(this,pszImgName);}
protected:
    SStringT GetRes( LPCTSTR strType,LPCTSTR pszResName );

    SStringT m_strPath;
    SMap<SResID,SStringT> m_mapFiles;
};

BOOL SOUI_EXP CreateResProvider(BUILTIN_RESTYPE resType,IObjRef **pObj);

}//namespace SOUI