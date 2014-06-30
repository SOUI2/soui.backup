//////////////////////////////////////////////////////////////////////////
//   File Name: duiresprovider.cpp
// Description: Resource Provider
//////////////////////////////////////////////////////////////////////////
#include "duistd.h"
#include "res.mgr/duiresprovider.h"
#include "mybuffer.h"
#include <io.h>

namespace SOUI
{


DuiResProviderPE::DuiResProviderPE( HINSTANCE hInst)
    : m_hResInst(hInst)
{

}

HBITMAP DuiResProviderPE::LoadBitmap(LPCTSTR pszResName )
{
    return ::LoadBitmap(m_hResInst,pszResName);
}

HICON DuiResProviderPE::LoadIcon(LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
{
    return (HICON)::LoadImage(m_hResInst, pszResName, IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
}

HCURSOR DuiResProviderPE::LoadCursor(LPCTSTR pszResName )
{
    return ::LoadCursor(m_hResInst,pszResName);
}

IBitmap * DuiResProviderPE::LoadImage( LPCTSTR strType,LPCTSTR pszResName )
{
    if(!HasResource(strType,pszResName)) return NULL;
    IBitmap * pImg=NULL;
    GETRENDERFACTORY->CreateBitmap(&pImg);
    
    size_t szImgBuf= GetRawBufferSize(strType,pszResName);
    
    if(szImgBuf==0) return FALSE;

    CMyBuffer<BYTE> buf;
    buf.Allocate(szImgBuf);
    GetRawBuffer(strType,pszResName,buf,szImgBuf);
    
    HRESULT hr=pImg->LoadFromMemory(buf,szImgBuf,strType);

    if(!SUCCEEDED(hr))
    {
        pImg->Release();
        pImg=NULL;
    }

    return pImg;
}

size_t DuiResProviderPE::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
{
    HRSRC hRsrc = MyFindResource(strType,pszResName);

    if (NULL == hRsrc)
        return 0;

    return ::SizeofResource(m_hResInst, hRsrc);
}

BOOL DuiResProviderPE::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
{
    DUIASSERT(strType);
    HRSRC hRsrc = MyFindResource(strType,pszResName);

    if (NULL == hRsrc)
        return FALSE;

    size_t dwSize = ::SizeofResource(m_hResInst, hRsrc);
    if (0 == dwSize)
        return FALSE;

    if(size < dwSize)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    HGLOBAL hGlobal = ::LoadResource(m_hResInst, hRsrc);
    if (NULL == hGlobal)
        return FALSE;

    LPVOID pBuffer = ::LockResource(hGlobal);
    if (NULL == pBuffer)
        return FALSE;

    memcpy(pBuf,pBuffer,dwSize);

    ::FreeResource(hGlobal);

    return TRUE;
}

BOOL DuiResProviderPE::HasResource( LPCTSTR strType,LPCTSTR pszResName )
{
    DUIASSERT(strType);
    return MyFindResource(strType,pszResName)!=NULL;
}

HRSRC DuiResProviderPE::MyFindResource( LPCTSTR strType, LPCTSTR pszResName )
{
    if(_tcsicmp(strType,_T("BITMAP"))==0) strType=RT_BITMAP;
    else if(_tcsicmp(strType,_T("ICON"))==0) strType=RT_ICON;
    else if(_tcsicmp(strType,_T("CURSOR"))==0) strType=RT_CURSOR;

    return ::FindResource(m_hResInst, pszResName, strType);
}



//////////////////////////////////////////////////////////////////////////

DuiResProviderFiles::DuiResProviderFiles()
{
}

SStringT DuiResProviderFiles::GetRes( LPCTSTR strType,LPCTSTR pszResName )
{
    DuiResID resID(strType,pszResName);
    SMap<DuiResID,SStringT>::CPair *p=m_mapFiles.Lookup(resID);
    if(!p) return _T("");
    SStringT strRet=m_strPath+_T("\\")+p->m_value;
    return strRet;
}

HBITMAP DuiResProviderFiles::LoadBitmap(LPCTSTR pszResName )
{
    SStringT strPath=GetRes(_T("BITMAP"),pszResName);
    if(strPath.IsEmpty()) return NULL;
    return (HBITMAP)::LoadImage(NULL, strPath, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
}

HICON DuiResProviderFiles::LoadIcon(LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
{
    SStringT strPath=GetRes(_T("ICON"),pszResName);
    if(strPath.IsEmpty()) return NULL;
    return (HICON)::LoadImage(NULL, strPath, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
}

HCURSOR DuiResProviderFiles::LoadCursor(LPCTSTR pszResName )
{
    SStringT strPath=GetRes(_T("CURSOR"),pszResName);
    if(!strPath.IsEmpty())
    {
        return (HCURSOR)::LoadImage(NULL, pszResName, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
    }else
    {//不是文件内的光标资源时，尝试从系统中载入光标
        return ::LoadCursor(NULL,pszResName);
    }
}

IBitmap * DuiResProviderFiles::LoadImage( LPCTSTR strType,LPCTSTR pszResName )
{
    SStringT strPath=GetRes(strType,pszResName);
    if(strPath.IsEmpty()) return NULL;

    IBitmap * pImg=NULL;
    GETRENDERFACTORY->CreateBitmap(&pImg);
    
    HRESULT hr=pImg->LoadFromFile(strPath,strType);
    if(!SUCCEEDED(hr))
    {
        pImg->Release();
        pImg=NULL;
    }
    return pImg;
    
}

size_t DuiResProviderFiles::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
{
    SStringT strPath=GetRes(strType,pszResName);
    if(strPath.IsEmpty()) return 0;
    WIN32_FIND_DATA wfd;
    HANDLE hf=FindFirstFile(strPath,&wfd);
    if(INVALID_HANDLE_VALUE==hf) return 0;
    FindClose(hf);
    return wfd.nFileSizeLow;
}

BOOL DuiResProviderFiles::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
{
    SStringT strPath=GetRes(strType,pszResName);
    if(strPath.IsEmpty()) return FALSE;
    FILE *f=_tfopen(strPath,_T("rb"));
    if(!f) return FALSE;
    size_t len=_filelength(_fileno(f));
    if(len>size)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        fclose(f);
        return FALSE;
    }
    BOOL bRet=(len==fread(pBuf,1,len,f));

    fclose(f);
    return bRet;
}

BOOL DuiResProviderFiles::Init( LPCTSTR pszPath )
{
    SStringT strPathIndex=pszPath;
    strPathIndex+=_T("\\");
    strPathIndex+=UISKIN_INDEX;

    pugi::xml_document xmlDoc;
    SStringT strFileName;
    if(!xmlDoc.load_file(strPathIndex,pugi::parse_default,pugi::encoding_utf8)) return FALSE;

    pugi::xml_node xmlResource=xmlDoc.child(L"resource");
    if(!xmlResource) return FALSE;
    pugi::xml_node xmlType=xmlResource.first_child();
    while(xmlType)
    {
        SStringT strType=DUI_CW2T(xmlType.name());
        pugi::xml_node xmlFile=xmlType.child(L"file");
        while(xmlFile)
        {
            DuiResID id(strType,DUI_CW2T(xmlFile.attribute(L"name").value()));
            SStringT strFile=DUI_CW2T(xmlFile.attribute(L"path").value());
            if(!m_strPath.IsEmpty()) strFile.Format(_T("%s\\%s"),(LPCTSTR)m_strPath,(LPCTSTR)strFile);
            m_mapFiles[id]=strFile;
            xmlFile=xmlFile.next_sibling(L"file");
        }
        xmlType=xmlType.next_sibling();
    }

    m_strPath=pszPath;
    return TRUE;
}

BOOL DuiResProviderFiles::HasResource( LPCTSTR strType,LPCTSTR pszResName )
{
    DuiResID resID(strType,pszResName);
    SMap<DuiResID,SStringT>::CPair *p=m_mapFiles.Lookup(resID);
    return (p!=NULL);
}


}//namespace SOUI