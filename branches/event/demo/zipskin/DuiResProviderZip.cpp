#include "stdafx.h"
#include "DuiResProviderZip.h"

namespace SOUI{

	DuiResProviderZip::DuiResProviderZip()
	{
	}

	DuiResProviderZip::~DuiResProviderZip(void)
	{
	}

	HBITMAP DuiResProviderZip::LoadBitmap( LPCTSTR strType,LPCTSTR pszResName )
	{
		CDuiStringT strPath=GetFilePath(pszResName,strType);
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

	HICON DuiResProviderZip::LoadIcon( LPCTSTR strType,LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
	{
		CDuiStringT strPath=GetFilePath(pszResName,strType);
		if(strPath.IsEmpty()) return NULL;
		CZipFile zf;
		if(!m_zipFile.GetFile(strPath,zf)) return NULL;
		TCHAR szTmp[MAX_PATH+1];
		GetTempPath(MAX_PATH,szTmp);
		GetTempFileName(szTmp,_T("ICO"),0,szTmp);
		FILE *f=_tfopen(szTmp,_T("wb"));
		if(!f) return NULL;
		fwrite(zf.GetData(),1,zf.GetSize(),f);
		fclose(f);
		HICON hIcon=(HICON)::LoadImage(NULL,szTmp,IMAGE_ICON,cx,cy,LR_DEFAULTSIZE|LR_DEFAULTCOLOR|LR_LOADFROMFILE);
		DeleteFile(szTmp);
		return hIcon;
	}

	IDuiImage * DuiResProviderZip::LoadImage( LPCTSTR strType,LPCTSTR pszResName)
	{
		CDuiStringT strPath=GetFilePath(pszResName,strType);
		if(strPath.IsEmpty()) return NULL;
		CZipFile zf;
		if(!m_zipFile.GetFile(strPath,zf)) return NULL;
		IDuiImage *pImg=GETIMGDECODER()->CreateDuiImage(strType);
		if(pImg) pImg->LoadFromMemory(zf.GetData(),zf.GetSize());
		return pImg;
	}

	BOOL DuiResProviderZip::Init( LPCTSTR pszZipFile )
	{
		if(!m_zipFile.Open(pszZipFile)) return FALSE;
		return LoadSkin();
	}

	BOOL DuiResProviderZip::Init( HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszType/*=_T("ZIP")*/ )
	{
		if(!m_zipFile.Open(hInst,pszResName,pszType)) return FALSE;
		return LoadSkin();
	}

	CDuiStringT DuiResProviderZip::GetFilePath( LPCTSTR pszResName,LPCTSTR pszType )
	{
		DuiResID resID(pszType,pszResName);
		CDuiMap<DuiResID,CDuiStringT>::CPair *p = m_mapFiles.Lookup(resID);
		if(!p) return _T("");
		return p->m_value;
	}

	size_t DuiResProviderZip::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
	{
		CDuiStringT strPath=GetFilePath(pszResName,strType);
		if(strPath.IsEmpty()) return FALSE;
		ZIP_FIND_DATA zfd;
		HANDLE hf=m_zipFile.FindFirstFile(strPath,&zfd);
		if(INVALID_HANDLE_VALUE==hf) return 0;
		m_zipFile.FindClose(hf);
		return zfd.nFileSizeUncompressed;
	}

	BOOL DuiResProviderZip::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
	{
		CDuiStringT strPath=GetFilePath(pszResName,strType);
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

	BOOL DuiResProviderZip::HasResource( LPCTSTR strType,LPCTSTR pszResName )
	{
		DuiResID resID(strType,pszResName);
		CDuiMap<DuiResID,CDuiStringT>::CPair *p = m_mapFiles.Lookup(resID);
		return p!=NULL;
	}

	BOOL DuiResProviderZip::LoadSkin()
	{
		CZipFile zf;
		BOOL bIdx=m_zipFile.GetFile(_T("index.xml"),zf);
		if(!bIdx) return FALSE;

		pugi::xml_document xmlDoc;
		CDuiStringA strFileName;
		if(!xmlDoc.load_buffer_inplace(zf.GetData(),zf.GetSize(),pugi::parse_default,pugi::encoding_utf8)) return FALSE;
		pugi::xml_node xmlElem=xmlDoc.child("resid");
		while(xmlElem)
		{
			DuiResID id(DUI_CA2T(xmlElem.attribute("type").value(),CP_UTF8),DUI_CA2T(xmlElem.attribute("name").value(),CP_UTF8));
			m_mapFiles[id] = DUI_CA2T(xmlElem.attribute("file").value(),CP_UTF8);
			xmlElem=xmlElem.next_sibling("resid");
		}
		return TRUE;
	}

}//namespace SOUI