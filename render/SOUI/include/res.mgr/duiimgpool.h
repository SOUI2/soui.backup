//////////////////////////////////////////////////////////////////////////
//  Class Name: DuiImgPool
// Description: Image Pool
//     Creator: Huang Jianxiong
//     Version: 2012.8.24 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duiresprovider-i.h"
#include "DuiSingletonMap.h"

namespace SOUI
{

typedef IBitmap * IBitmapPtr;
class SOUI_EXP DuiImgPool:public DuiSingletonMap<DuiImgPool,IBitmapPtr,DuiResID>
{
public:
    DuiImgPool();
    virtual ~DuiImgPool();

    IBitmap * GetImage(LPCTSTR pszImgName,LPCTSTR pszType=NULL);

protected:
    static void OnImageRemoved(const IBitmapPtr & obj)
    {
        obj->Release();
    }
};

}//namespace SOUI