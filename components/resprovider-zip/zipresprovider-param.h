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
    };
}