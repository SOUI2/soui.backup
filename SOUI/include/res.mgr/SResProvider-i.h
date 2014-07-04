#ifndef _DUIRESPROVIDERBASE_
#define _DUIRESPROVIDERBASE_
#pragma once

#include "../render/render-i.h"

#define MAX_RES_TYPE        10            //资源类型预定义，最大长度不超过10
#define MAX_RES_NAME        100            //注意：给资源令名时，最大长度不要超过MAX_RES_NAME

class SOUI_EXP SResID
{
public:
    SResID(LPCTSTR pszType,LPCTSTR pszName)
    {
        memset(this,0,sizeof(SResID));
        if(pszType) _tcscpy_s(szType,MAX_RES_TYPE,pszType);
        if(pszName) _tcscpy_s(szName,MAX_RES_NAME,pszName);
        _tcslwr(szType);
        _tcslwr(szName);
    }

    TCHAR szType[MAX_RES_TYPE+1];
    TCHAR szName[MAX_RES_NAME+1];
};


template<>
class _COLL_NS::CElementTraits< SResID > :
    public _COLL_NS::CElementTraitsBase< SResID >
{
public:
    static ULONG Hash( INARGTYPE resid )
    {
        ULONG_PTR uRet=0;
        
        for(LPCTSTR p=resid.szType; *p; p++)
        {
            uRet=uRet*68+*p;
        }
        uRet*=10000;
        for(LPCTSTR p=resid.szName; *p; p++)
        {
            uRet=uRet*68+*p;
        }
        return (ULONG)uRet;
    }

    static bool CompareElements( INARGTYPE element1, INARGTYPE element2 )
    {
        return _tcscmp(element1.szType,element2.szType)==0
            && _tcscmp(element1.szName,element2.szName)==0;
    }

    static int CompareElementsOrdered( INARGTYPE element1, INARGTYPE element2 )
    {
        int nRet=_tcscmp(element1.szType,element2.szType);
        if(nRet==0) nRet=_tcscmp(element1.szName,element2.szName);
        return nRet;
    }
};

namespace SOUI
{

class SOUI_EXP IResProvider
{
public:
    virtual ~IResProvider(){}
    virtual BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName)=0;
    virtual HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0)=0;
    virtual HBITMAP    LoadBitmap(LPCTSTR pszResName)=0;
    virtual HCURSOR LoadCursor(LPCTSTR pszResName)=0;
    virtual IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName)=0;
    virtual size_t GetRawBufferSize(LPCTSTR strType,LPCTSTR pszResName)=0;
    virtual BOOL GetRawBuffer(LPCTSTR strType,LPCTSTR pszResName,LPVOID pBuf,size_t size)=0;

    //没有指定图片类型时默认从这些类别中查找
    virtual LPCTSTR FindImageType(LPCTSTR pszImgName)
    {
        //图片类型
        LPCTSTR IMGTYPES[]=
        {
            _T("IMGX"),
            _T("PNG"),
            _T("JPG"),
            _T("GIF"),
            _T("TGA"),
            _T("TIFF"),
        };
        for(int i=0;i< ARRAYSIZE(IMGTYPES);i++)
        {
            if(HasResource(IMGTYPES[i],pszImgName)) return IMGTYPES[i];
        }
        return NULL;
    }
};


}//namespace SOUI
#endif//_DUIRESPROVIDERBASE_
