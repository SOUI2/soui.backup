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

HBITMAP DuiResProviderPE::LoadBitmap( LPCTSTR strType,LPCTSTR pszResName )
{
    return ::LoadBitmap(m_hResInst,pszResName);
}

HICON DuiResProviderPE::LoadIcon( LPCTSTR strType,LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
{
    return (HICON)::LoadImage(m_hResInst, pszResName, IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
}

IBitmap * DuiResProviderPE::LoadImage( LPCTSTR strType,LPCTSTR pszResName )
{
    if(!HasResource(strType,pszResName)) return NULL;
    
    return NULL;//todo:hjx
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
    if(_tcsicmp(strType,DUIRES_BMP_TYPE)==0) strType=MAKEINTRESOURCE(2);//RT_BITMAP;
    else if(_tcsicmp(strType,DUIRES_ICON_TYPE)==0) strType=MAKEINTRESOURCE(3);//RT_ICON;

    return ::FindResource(m_hResInst, pszResName, strType);
}

HCURSOR DuiResProviderPE::LoadCursor( LPCTSTR strType,LPCTSTR pszResName )
{
    return ::LoadCursor(m_hResInst,pszResName);
}


//////////////////////////////////////////////////////////////////////////

DuiResProviderFiles::DuiResProviderFiles()
{
}

CDuiStringT DuiResProviderFiles::GetRes( LPCTSTR strType,LPCTSTR pszResName )
{
    DuiResID resID(strType,pszResName);
    CDuiMap<DuiResID,CDuiStringT>::CPair *p=m_mapFiles.Lookup(resID);
    if(!p) return _T("");
    CDuiStringT strRet=m_strPath+_T("\\")+p->m_value;
    return strRet;
}

HBITMAP DuiResProviderFiles::LoadBitmap( LPCTSTR strType,LPCTSTR pszResName )
{
    CDuiStringT strPath=GetRes(strType,pszResName);
    if(strPath.IsEmpty()) return NULL;
    return (HBITMAP)::LoadImage(NULL, strPath, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
}

HICON DuiResProviderFiles::LoadIcon( LPCTSTR strType,LPCTSTR pszResName ,int cx/*=0*/,int cy/*=0*/)
{
    CDuiStringT strPath=GetRes(strType,pszResName);
    if(strPath.IsEmpty()) return NULL;
    return (HICON)::LoadImage(NULL, strPath, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
}

IBitmap * DuiResProviderFiles::LoadImage( LPCTSTR strType,LPCTSTR pszResName )
{
    if(!HasResource(strType,pszResName)) return NULL;
    return NULL;//todo:hjx
}

size_t DuiResProviderFiles::GetRawBufferSize( LPCTSTR strType,LPCTSTR pszResName )
{
    CDuiStringT strPath=GetRes(strType,pszResName);
    if(strPath.IsEmpty()) return 0;
    WIN32_FIND_DATA wfd;
    HANDLE hf=FindFirstFile(strPath,&wfd);
    if(INVALID_HANDLE_VALUE==hf) return 0;
    FindClose(hf);
    return wfd.nFileSizeLow;
}

BOOL DuiResProviderFiles::GetRawBuffer( LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size )
{
    CDuiStringT strPath=GetRes(strType,pszResName);
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
    CMyBuffer<char>  xmlBuf;
    CDuiStringT strPathIndex=pszPath;
    strPathIndex+=_T("\\");
    strPathIndex+=INDEX_XML;
    FILE *f=_tfopen(strPathIndex,_T("rb"));
    if(!f) return(FALSE);
    int nLen=_filelength(_fileno(f));
    if(nLen>100*1024)
    {
        fclose(f);
        return FALSE;
    }
    xmlBuf.Allocate(nLen);
    if(nLen!=fread(xmlBuf,1,nLen,f))
    {
        fclose(f);
        return FALSE;
    }
    fclose(f);

    pugi::xml_document xmlDoc;
    CDuiStringT strFileName;
    if(!xmlDoc.load_buffer(xmlBuf,xmlBuf.size(),pugi::parse_default,pugi::encoding_utf8)) return FALSE;

    pugi::xml_node xmlNode=xmlDoc.child("resid");
    while(xmlNode)
    {
        DuiResID id(DUI_CA2T(xmlNode.attribute("type").value(),CP_UTF8),DUI_CA2T(xmlNode.attribute("name").value(),CP_UTF8));
        CDuiStringT strFile=DUI_CA2T(xmlNode.attribute("file").value(),CP_UTF8);
        if(!m_strPath.IsEmpty()) strFile.Format(_T("%s\\%s"),(LPCTSTR)m_strPath,(LPCTSTR)strFile);
        m_mapFiles[id]=strFile;
        xmlNode=xmlNode.next_sibling("resid");
    }

    m_strPath=pszPath;
    return TRUE;
}

BOOL DuiResProviderFiles::HasResource( LPCTSTR strType,LPCTSTR pszResName )
{
    DuiResID resID(strType,pszResName);
    CDuiMap<DuiResID,CDuiStringT>::CPair *p=m_mapFiles.Lookup(resID);
    return (p!=NULL);
}

HCURSOR DuiResProviderFiles::LoadCursor( LPCTSTR strType,LPCTSTR pszResName )
{
    return (HCURSOR)::LoadImage(NULL, pszResName, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
}

}//namespace SOUI