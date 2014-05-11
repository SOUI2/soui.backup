#pragma once

#include "duiimage-i.h"
#include "duiimage.h"

namespace SOUI
{

class SOUI_EXP CDuiImgDecoder_Def
    :public IDuiImgDecoder
{
public:
    CDuiImgDecoder_Def(void);
    ~CDuiImgDecoder_Def(void);

    virtual IDuiImage* CreateDuiImage(LPCTSTR pszType);
    virtual void DestoryDuiImage(IDuiImage* pImg);
    virtual LPCTSTR GetSupportTypes()
    {
        return _T("BMP\0IMGX\0");
    }
};

}//end of namespace SOUI