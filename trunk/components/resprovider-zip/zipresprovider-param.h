#pragma once

namespace SOUI
{
    struct IRenderFactory;
    struct ZIPRES_PARAM
    {
        enum {ZIPFILE,PEDATA} type;
        IRenderFactory *pRenderFac;
        union{
            LPCTSTR pszZipFile;
            struct{
                HINSTANCE hInst;
                LPCTSTR pszResName;
                LPCTSTR pszResType;
            }peInfo;
        };
        void ZipFile(IRenderFactory *_pRenderFac,LPCTSTR pszFile)
        {
            type=ZIPFILE;
            pszZipFile=pszFile;
            pRenderFac = _pRenderFac;
        }
        void ZipResource(IRenderFactory *_pRenderFac,HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszResType=_T("zip"))
        {
            type=PEDATA;
            pRenderFac = _pRenderFac;
            peInfo.hInst=hInst;
            peInfo.pszResName=pszResName;
            peInfo.pszResType=pszResType;
        }
    };
}