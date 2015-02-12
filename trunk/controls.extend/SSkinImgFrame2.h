#pragma once
#include <core/sskin.h>

namespace SOUI
{
    /*usage
    <imgframe2 src="imgx:png_test{0,0,100,100}" xxx/>
    */
    class SSkinImgFrame2 : public SSkinImgFrame
    {
    SOUI_CLASS_NAME(SSkinImgFrame2,L"imgframe2")
    public:
        SSkinImgFrame2(void);
        ~SSkinImgFrame2(void);
        
        virtual int GetStates(){return 1;}
        virtual SIZE GetSkinSize(){return m_rcImg.Size();}
        
        virtual bool SetImage(IBitmap *pImg){return false;}
        
        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"src",OnAttrSrc)
        SOUI_ATTRS_END()
        
    protected:
        LRESULT OnAttrSrc(const SStringW & strValue,BOOL bLoading);
        CRect m_rcImg;
        SStringW m_strImgKey;
    };

}
