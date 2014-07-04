//////////////////////////////////////////////////////////////////////////
//  Class Name: DuiImgPool
// Description: Image Pool
//     Creator: Huang Jianxiong
//     Version: 2012.8.24 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "Sresprovider-i.h"
#include "core/SSingletonMap.h"

namespace SOUI
{

typedef IBitmap * IBitmapPtr;
class SOUI_EXP SImgPool:public SSingletonMap<SImgPool,IBitmapPtr,SResID>
{
public:
    SImgPool();
    virtual ~SImgPool();

    IBitmap * GetImage(LPCTSTR pszImgName,LPCTSTR pszType=NULL);

protected:
    static void OnImageRemoved(const IBitmapPtr & obj)
    {
        obj->Release();
    }
};

}//namespace SOUI