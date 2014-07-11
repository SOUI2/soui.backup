#pragma once

#include <interface/SResProvider-i.h>

#include "ZipArchive.h"

namespace SOUI{


class SResProviderZip : public IResProvider
{
public:
	SResProviderZip();
	~SResProviderZip(void);

    virtual BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);
    virtual HICON   LoadIcon(LPCTSTR pszResName,int cx,int cy);
    virtual HBITMAP    LoadBitmap(LPCTSTR pszResName);
    virtual HCURSOR LoadCursor(LPCTSTR pszResName);
    virtual IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    virtual size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    virtual BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

	HBITMAP	LoadBitmap(LPCTSTR strType,LPCTSTR pszResName);
	HICON   LoadIcon(LPCTSTR strType,LPCTSTR pszResName,int cx,int cy);
	IDuiImage * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
	size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
	BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);
	BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);

	BOOL Init(LPCTSTR pszZipFile);
	BOOL Init(HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType=_T("ZIP"));
protected:
	BOOL LoadSkin();
	CDuiStringT GetFilePath(LPCTSTR pszResName,LPCTSTR pszType);
	CDuiMap<DuiResID,CDuiStringT> m_mapFiles;

	CZipArchive m_zipFile;
};

}//namespace SOUI
