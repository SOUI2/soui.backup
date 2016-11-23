/*
    SOUI窗口布局接口
*/
#pragma once

#include <sobject/sobject-i.h>

namespace SOUI{
    class SWindow;


    enum ORIENTATION{
        Horz,Vert
    };
    
    struct ILayoutParam : IObject
    {
        virtual bool IsMatchParent(ORIENTATION orientation) const = 0;
        virtual bool IsSpecifiedSize(ORIENTATION orientation) const = 0;
        virtual int GetSpecifiedSize(ORIENTATION orientation) const = 0;
    };

    struct ILayout : IObject{
        virtual void CalcPostionOfChildren(SWindow * pParent) = 0;
        virtual ILayoutParam * CreateLayoutParam() const = 0;
    };
}
