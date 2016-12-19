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
    
	enum{
		SIZE_WRAP_CONTENT=-1,
		SIZE_MATCH_PARENT=-2,
		SIZE_SPEC = 0,
	};

    struct ILayoutParam : IObject,IObjRef
    {
        virtual bool IsMatchParent(ORIENTATION orientation) const = 0;
		virtual bool IsWrapContent(ORIENTATION orientation) const = 0;
        virtual bool IsSpecifiedSize(ORIENTATION orientation) const = 0;
        virtual int GetSpecifiedSize(ORIENTATION orientation) const = 0;
    };

    struct ILayout : IObject , IObjRef{
		virtual bool IsParamAcceptable(ILayoutParam *pLayoutParam) const = 0;
        virtual void LayoutChildren(SWindow * pParent) = 0;
        virtual ILayoutParam * CreateLayoutParam() const = 0;
		virtual CSize MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const =0;
    };
}
