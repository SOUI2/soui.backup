#pragma once

#include "interface/slayout-i.h"
#include <sobject/sobject-state-impl.hpp>
#include "SLinearLayoutParamStruct.h"

namespace SOUI
{

	class SLinearLayoutParam : public SObjectImpl<TObjRefImpl<ILayoutParam>>
							 , protected SLinearLayoutParamStruct
    {
        SOUI_CLASS_NAME(SLinearLayoutParam,L"LinearLayoutParam")

		friend class SLinearLayout;
    public:
		SLinearLayoutParam();

        virtual bool IsMatchParent(ORIENTATION orientation) const;
		virtual bool IsWrapContent(ORIENTATION orientation) const;

        virtual bool IsSpecifiedSize(ORIENTATION orientation) const;

        virtual int GetSpecifiedSize(ORIENTATION orientation) const;

		virtual void Clear();

		virtual void SetMatchParent(ORIENTATION orientation);

		virtual void SetWrapContent(ORIENTATION orientation);

		virtual void SetSpecifiedSize(ORIENTATION orientation, int nSize);

		virtual void * GetRawData();

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"width",OnAttrWidth)
            ATTR_CUSTOM(L"height",OnAttrHeight)
            ATTR_CUSTOM(L"size",OnAttrSize)
            ATTR_FLOAT(L"weight",weight,FALSE)
            ATTR_ENUM_BEGIN(L"layout_gravity",Gravity,FALSE)
                ATTR_ENUM_VALUE(L"left",G_Left)
                ATTR_ENUM_VALUE(L"top",G_Top)
                ATTR_ENUM_VALUE(L"center",G_Center)
                ATTR_ENUM_VALUE(L"right",G_Right)
                ATTR_ENUM_VALUE(L"bottom",G_Bottom)
            ATTR_ENUM_END(gravity)
			ATTR_RECT(L"extend",rcExtend,FALSE)
			ATTR_INT(L"extend_left",rcExtend.left,FALSE)
			ATTR_INT(L"extend_top",rcExtend.top,FALSE)
			ATTR_INT(L"extend_right",rcExtend.right,FALSE)
			ATTR_INT(L"extend_bottom",rcExtend.bottom,FALSE)
        SOUI_ATTRS_BREAK()


	protected:
        HRESULT OnAttrSize(const SStringW & strValue,BOOL bLoading);
		HRESULT OnAttrWidth(const SStringW & strValue,BOOL bLoading);
		HRESULT OnAttrHeight(const SStringW & strValue,BOOL bLoading);

    };

    class SLinearLayout : public SObjectImpl<TObjRefImpl<ILayout>>
    {
		SOUI_CLASS_NAME(SLinearLayout,L"linearLayout")
    public:
        SLinearLayout(void);
        ~SLinearLayout(void);

		static HRESULT CreateLayoutParam(IObjRef ** ppObj);

		virtual LayoutType GetLayoutType() const {return Layout_Linear;}
        virtual void LayoutChildren(SWindow * pParent);
        virtual ILayoutParam * CreateLayoutParam() const;
		virtual CSize MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const;
		virtual bool IsParamAcceptable(ILayoutParam *pLayoutParam) const;

        
        SOUI_ATTRS_BEGIN()
            ATTR_ENUM_BEGIN(L"orientation",ORIENTATION,FALSE)
                ATTR_ENUM_VALUE(L"horizontal",Horz)
                ATTR_ENUM_VALUE(L"vertical",Vert)
            ATTR_ENUM_END(m_orientation)
            ATTR_ENUM_BEGIN(L"gravity",Gravity,FALSE)
                ATTR_ENUM_VALUE(L"left",G_Left)
                ATTR_ENUM_VALUE(L"top",G_Top)
                ATTR_ENUM_VALUE(L"center",G_Center)
                ATTR_ENUM_VALUE(L"right",G_Right)
                ATTR_ENUM_VALUE(L"bottom",G_Bottom)
            ATTR_ENUM_END(m_gravity)

        SOUI_ATTRS_BREAK()


	protected:
		ORIENTATION m_orientation;
        Gravity     m_gravity;
    };

	class SVBox : public SLinearLayout
	{
		SOUI_CLASS_NAME(SVBox,L"vbox")

	public:
		SVBox(){m_orientation = Vert;}
	};

	class SHBox : public SLinearLayout
	{
		SOUI_CLASS_NAME(SHBox,L"hbox")

	public:
		SHBox(){m_orientation = Horz;}
	};
}

