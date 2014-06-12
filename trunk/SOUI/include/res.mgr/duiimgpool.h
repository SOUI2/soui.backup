//////////////////////////////////////////////////////////////////////////
//  Class Name: DuiImgPool
// Description: Image Pool
//     Creator: Huang Jianxiong
//     Version: 2012.8.24 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duiimage-i.h"
#include "duiresprovider-i.h"
#include "DuiSingletonMap.h"

namespace SOUI
{

typedef IDuiImage * CDuiImgBasePtr;
class SOUI_EXP DuiImgPool:public DuiSingletonMap<DuiImgPool,CDuiImgBasePtr,DuiResID>
{
public:
    DuiImgPool();
    virtual ~DuiImgPool();

    IDuiImage * GetImage(LPCTSTR pszImgName,LPCTSTR pszType=NULL);

protected:
    static void OnImageRemoved(const CDuiImgBasePtr & obj)
    {
        obj->Release();
    }
};

}//namespace SOUI