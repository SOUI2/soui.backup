#pragma once

#include "interface/slayout-i.h"
#include "SGridLayoutParamStruct.h"

namespace SOUI
{
	class SGridLayoutParam : public SObjectImpl<TObjRefImpl<ILayoutParam>>
		, protected SGridLayoutParamStruct
	{
		SOUI_CLASS_NAME(SGridLayoutParam,L"GridLayoutParam")

		friend class SGridLayout;
	public:
		SGridLayoutParam();

		virtual bool IsMatchParent(ORIENTATION orientation) const;
		virtual bool IsWrapContent(ORIENTATION orientation) const;

		virtual bool IsSpecifiedSize(ORIENTATION orientation) const;

		virtual SLayoutSize GetSpecifiedSize(ORIENTATION orientation) const;

		virtual void Clear();

		virtual void SetMatchParent(ORIENTATION orientation);

		virtual void SetWrapContent(ORIENTATION orientation);

		virtual void SetSpecifiedSize(ORIENTATION orientation, const SLayoutSize& layoutSize);

		virtual void * GetRawData();

		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"row",iRow,TRUE)
			ATTR_INT(L"rowSpan",nRowSpan,TRUE)
			ATTR_INT(L"column",iCol,TRUE)
			ATTR_INT(L"columnSpan",nColSpan,TRUE)
			ATTR_CUSTOM(L"width",OnAttrWidth)
			ATTR_CUSTOM(L"height",OnAttrHeight)
			ATTR_CUSTOM(L"size",OnAttrSize)
			ATTR_ENUM_BEGIN(L"xGravity",GridGravityX,TRUE)
				ATTR_ENUM_VALUE(L"left",gLeft)
				ATTR_ENUM_VALUE(L"center",gCenter)
				ATTR_ENUM_VALUE(L"right",gRight)
			ATTR_ENUM_END(xGravity)
			ATTR_ENUM_BEGIN(L"yGravity",GridGravityY,TRUE)
				ATTR_ENUM_VALUE(L"top",gTop)
				ATTR_ENUM_VALUE(L"middle",gMiddle)
				ATTR_ENUM_VALUE(L"bottom",gBottom)
			ATTR_ENUM_END(yGravity)
		SOUI_ATTRS_BREAK()

	protected:
		HRESULT OnAttrSize(const SStringW & strValue,BOOL bLoading);
		HRESULT OnAttrWidth(const SStringW & strValue,BOOL bLoading);
		HRESULT OnAttrHeight(const SStringW & strValue,BOOL bLoading);

	};

	class SGridLayout: public SObjectImpl<TObjRefImpl<ILayout>>
	{
		SOUI_CLASS_NAME_EX(SGridLayout,L"gridLayout",Layout)
	public:
		SGridLayout(void);
		~SGridLayout(void);

		virtual bool IsParamAcceptable(ILayoutParam *pLayoutParam) const;

		virtual void LayoutChildren(SWindow * pParent);

		virtual ILayoutParam * CreateLayoutParam() const;

		virtual CSize MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const;


		SOUI_ATTRS_BEGIN()
			ATTR_INT(L"columnCount",m_nCols,TRUE)
			ATTR_INT(L"rowCount",m_nRows,TRUE)
			ATTR_LAYOUTSIZE(L"xInterval",m_xInterval,TRUE)
			ATTR_LAYOUTSIZE(L"yInterval",m_yInterval,TRUE)
		SOUI_ATTRS_BREAK()
	protected:

		int m_nCols;
		int m_nRows;
		SLayoutSize m_xInterval;
		SLayoutSize m_yInterval;
	};

}
