#pragma once

#include "interface/slayout-i.h"
#include <sobject/sobject-state-impl.hpp>

namespace SOUI
{
    enum Gravity{
        G_Left=0,G_Top=0,
        G_Center=1,
        G_Right=2,G_Bottom=2,
    };


	class SLinearLayoutParam : public SObjectImpl<ILayoutParam>
    {
        SOUI_CLASS_NAME(SLinearLayoutParam,L"LinearLayoutParam")

    public:
        virtual bool IsMatchParent(ORIENTATION orientation) const;
		virtual bool IsWrapContent(ORIENTATION orientation) const;

        virtual bool IsSpecifiedSize(ORIENTATION orientation) const;

        virtual int GetSpecifiedSize(ORIENTATION orientation) const;

        HRESULT OnAttrWidth(const SStringW & strValue,BOOL bLoading);
        HRESULT OnAttrHeight(const SStringW & strValue,BOOL bLoading);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"width",OnAttrWidth)
            ATTR_CUSTOM(L"height",OnAttrHeight)
            ATTR_FLOAT(L"weight",m_weight,FALSE)
            ATTR_ENUM_BEGIN(L"gravity",Gravity,FALSE)
                ATTR_ENUM_VALUE(L"left",G_Left)
                ATTR_ENUM_VALUE(L"top",G_Top)
                ATTR_ENUM_VALUE(L"center",G_Center)
                ATTR_ENUM_VALUE(L"right",G_Right)
                ATTR_ENUM_VALUE(L"bottom",G_Bottom)
            ATTR_ENUM_END(m_gravity)
        SOUI_ATTRS_BREAK()

    public:
        int m_width,m_height;
        float m_weight;
        Gravity m_gravity;
    };

    class SLinearLayout : public SObjectImpl<ILayout>
    {
		SOUI_CLASS_NAME(SLinearLayout,L"linearLayout")
    public:
        SLinearLayout(void);
        ~SLinearLayout(void);

        virtual void LayoutChildren(SWindow * pParent);
        virtual ILayoutParam * CreateLayoutParam() const;
		virtual CSize MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const;
		virtual bool IsParamAcceptable(ILayoutParam *pLayoutParam) const;

        
        SOUI_ATTRS_BEGIN()
            ATTR_ENUM_BEGIN(L"orientation",ORIENTATION,FALSE)
                ATTR_ENUM_VALUE(L"horizontal",Horz)
                ATTR_ENUM_VALUE(L"vertical",Vert)
            ATTR_ENUM_END(m_orientation)
        SOUI_ATTRS_BREAK()


		ORIENTATION m_orientation;
    };

}

