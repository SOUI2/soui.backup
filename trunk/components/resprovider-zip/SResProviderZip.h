#pragma once

#ifdef RESPROVIDERZIP_EXPORTS
#define RESPROVIDERZIP_API __declspec(dllexport)
#else
#define RESPROVIDERZIP_API __declspec(dllimport)
#endif

#ifndef SOUI_EXP
#define SOUI_EXP RESPROVIDERZIP_API
#endif

#include <interface/SResProvider-i.h>
#include <unknown/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <souicoll.h>
#include <SResID.h>
#include <interface/render-i.h>

#include "ZipArchive.h"

namespace SOUI{

class RESPROVIDERZIP_API SResProviderZip : public TObjRefImpl<IResProvider>
{
public:
	SResProviderZip(IRenderFactory *pRenderFac);
	~SResProviderZip(void);

    virtual BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName);
    virtual HICON   LoadIcon(LPCTSTR pszResName,int cx,int cy);
    virtual HBITMAP    LoadBitmap(LPCTSTR pszResName);
    virtual HCURSOR LoadCursor(LPCTSTR pszResName);
    virtual IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName);
    virtual size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName);
    virtual BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size);

	BOOL Init(LPCTSTR pszZipFile);
	BOOL Init(HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType=_T("ZIP"));
protected:
	BOOL LoadSkin();
	SStringT GetFilePath(LPCTSTR pszResName,LPCTSTR pszType);
	SMap<SResID,SStringT> m_mapFiles;
    CAutoRefPtr<IRenderFactory> m_renderFactory;
	CZipArchive m_zipFile;
};

}//namespace SOUI
