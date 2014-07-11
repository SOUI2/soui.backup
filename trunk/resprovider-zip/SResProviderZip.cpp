#include "stdafx.h"
#pragma warning(disable:4251)

#include "SResProviderZip.h"
#include <pugixml/pugixml.hpp>

extern HICON CURSORICON_LoadFromBuf(const BYTE * bits,DWORD filesize,INT width, INT height,BOOL fCursor, UINT loadflags);
extern HICON CURSORICON_LoadFromFile( LPCWSTR filename,
                              INT width, INT height,
                              BOOL fCursor, UINT loadflags);

namespace SOUI{

    SResProviderZip::SResProviderZip(IRenderFactory *pRenderFac):m_renderFactory(pRenderFac)
	{
	}

	SResProviderZip::~SResProviderZip(void)
	{
	}

	HBITMAP SResProviderZip::LoadBitmap(LPCTSTR pszResName )
	{
		SStringT strPath=GetFilePath(pszResName,_T("BITMAP"));
		if(strPath.IsEmpty()) return NULL;
		CZipFile zf;
		if(!m_zipFile.GetFile(strPath,zf)) return NULL;

		HDC hDC = GetDC(NULL);
		//读取位图头
		BITMAPFILEHEADER *pBmpFileHeader=(BITMAPFILEHEADER *)zf.GetData(); 
		//检测位图头
		if (pBmpFileHeader->bfType != ((WORD) ('M'<<8)|'B')) 
		{
			return NULL; 
		} 
		//判断位图长度
		if (pBmpFileHeader->bfSize > (UINT)zf.GetSize()) 
		{ 
			return NULL; 
		} 
		LPBITMAPINFO lpBitmap=(LPBITMAPINFO)(pBmpFileHeader+1); 
		LPVOID lpBits=(LPBYTE)zf.GetData()+pBmpFileHeader->bfOffBits;
		HBITMAP hBitmap= CreateDIBitmap(hDC,&lpBitmap->bmiHeader,CBM_INIT,lpBits,lpBitmap,DIB_RGB_COLORS);
		ReleaseDC(NULL,hDC);
		
		return hBitmap;
	}

	HICON SResProviderZip::LoadIcon(LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
	{
		SStringT strPath=GetFilePath(pszResName,_T("ICON"));
		if(strPath.IsEmpty()) return NULL;
		CZipFile zf;
		if(!m_zipFile.GetFile(strPath,zf)) return NULL;

        return CURSORICON_LoadFromBuf(zf.GetData(),zf.GetSize(),cx,cy,FALSE,LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
	}

    HCURSOR SResProviderZip::LoadCursor( LPCTSTR pszResName )
    {
        SStringT strPath=GetFilePath(pszResName,_T("CURSOR"));
        if(strPath.IsEmpty()) return NULL;
        CZipFile zf;
        if(!m_zipFile.GetFile(strPath,zf)) return NULL;
        return (HCURSOR)CURSORICON_LoadFromBuf(zf.GetData(),zf.GetSize(),0,0,TRUE,LR_DEFAULTSIZE|LR_DEFAULTCOLOR);
    }

	IBitmap * SResProviderZip::LoadImage( LPCTSTR strType,LPCTSTR pszResName)
	{
		SStringT strPath=GetFilePath(pszResName,strType);
		if(strPath.IsEmpty()) return NULL;
		CZipFile zf;
		if(!m_zipFile.GetFile(strPath,zf)) return NULL;
        IBitmap * pBmp=NULL;
        m_renderFactory->CreateBitmap(&pBmp);
        if(!pBmp) return NULL;
        pBmp->LoadFromMemory(zf.GetData(),zf.GetSize(),strType);
        return pBmp;
	}


	BOOL SResProviderZip::Init( LPCTSTR pszZipFile )
	{
		if(!m_zipFile.Open(pszZipFile)) return FALSE;
		return LoadSkin();
	}

	BOOL SResProviderZip::Init( HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType/*=_T("ZIP")*/ )
	{
		if(!m_zipFile.Open(hInst,pszResName,pszType)) return FALSE;
		return LoadSkin();
	}

	SStringT SResProviderZip::GetFilePath( LPCTSTR pszResName,LPCTSTR pszType )
	{
		SResID resID(pszType,pszResName);
		SMap<SResID,SStringT>::CPair *p = m_mapFiles.Lookup(resID);
		if(!p) return _T("");
		return p->m_value;
	}

	size_t SResProviderZip::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
	{
		SStringT strPath=GetFilePath(pszResName,strType);
		if(strPath.IsEmpty()) return FALSE;
		ZIP_FIND_DATA zfd;
		HANDLE hf=m_zipFile.FindFirstFile(strPath,&zfd);
		if(INVALID_HANDLE_VALUE==hf) return 0;
		m_zipFile.FindClose(hf);
		return zfd.nFileSizeUncompressed;
	}

	BOOL SResProviderZip::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
	{
		SStringT strPath=GetFilePath(pszResName,strType);
		if(strPath.IsEmpty()) return FALSE;
		CZipFile zf;
		if(!m_zipFile.GetFile(strPath,zf)) return NULL;
		if(size<zf.GetSize())
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return FALSE;
		}
		memcpy(pBuf,zf.GetData(),zf.GetSize());
		return TRUE;
	}

	BOOL SResProviderZip::HasResource( LPCTSTR strType,LPCTSTR pszResName )
	{
		SResID resID(strType,pszResName);
		SMap<SResID,SStringT>::CPair *p = m_mapFiles.Lookup(resID);
		return p!=NULL;
	}

	BOOL SResProviderZip::LoadSkin()
	{
		CZipFile zf;
		BOOL bIdx=m_zipFile.GetFile(_T("uiskin.idx"),zf);
		if(!bIdx) return FALSE;

		pugi::xml_document xmlDoc;
		SStringA strFileName;
		if(!xmlDoc.load_buffer_inplace(zf.GetData(),zf.GetSize(),pugi::parse_default,pugi::encoding_utf8)) return FALSE;
		pugi::xml_node xmlElem=xmlDoc.child(L"resource");
        if(!xmlElem) return FALSE;
        pugi::xml_node resType=xmlElem.first_child();
		while(resType)
		{
            pugi::xml_node resFile=resType.first_child();
            while(resFile)
            {
                SResID id(S_CW2T(resType.name()),S_CW2T(resFile.attribute(L"name").value()));
                m_mapFiles[id] = S_CW2T(resFile.attribute(L"path").value());
                resFile=resFile.next_sibling();
            }
            resType = resType.next_sibling();
		}
		return TRUE;
	}

}//namespace SOUI