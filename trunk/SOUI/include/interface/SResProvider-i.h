#ifndef _SRESPROVIDERBASE_
#define _SRESPROVIDERBASE_
#pragma once

#define OR_API SOUI_EXP

#include <unknown/obj-ref-i.h>
#include "render-i.h"


namespace SOUI
{

    struct SOUI_EXP IResProvider : public IObjRef
    {
        virtual BOOL HasResource(LPCTSTR strType,LPCTSTR pszResName)=0;
        virtual HICON   LoadIcon(LPCTSTR pszResName,int cx=0,int cy=0)=0;
        virtual HBITMAP    LoadBitmap(LPCTSTR pszResName)=0;
        virtual HCURSOR LoadCursor(LPCTSTR pszResName)=0;
        virtual IBitmap * LoadImage(LPCTSTR strType,LPCTSTR pszResName)=0;
        virtual IImgX   * LoadImgX(LPCTSTR strType,LPCTSTR pszResName)=0;
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
#endif//_SRESPROVIDERBASE_
