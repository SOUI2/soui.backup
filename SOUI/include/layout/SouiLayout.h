#pragma once
#include "interface/slayout-i.h"
#include <sobject/sobject-state-impl.hpp>
#include <helper/SplitString.h>

namespace SOUI{

	class SoutLayoutParam: public SObjectImpl<ILayoutParam>
	{
		SOUI_CLASS_NAME(SoutLayoutParam,L"SouiLayoutParam")

	public:
		virtual bool IsMatchParent(ORIENTATION orientation) const;

		virtual bool IsSpecifiedSize(ORIENTATION orientation) const;

		virtual int GetSpecifiedSize(ORIENTATION orientation) const;

	protected:
		HRESULT OnAttrWidth(const SStringW & strValue,BOOL bLoading)
		{
			if(strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
				m_width = SIZE_MATCH_PARENT;
			else if(strValue.CompareNoCase(L"wrapContent") == 0)
				m_width = SIZE_WRAP_CONTENT;
			else
				m_width = _wtoi(strValue);
			return S_OK;
		}

		HRESULT OnAttrHeight(const SStringW & strValue,BOOL bLoading)
		{
			if(strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
				m_height = SIZE_MATCH_PARENT;
			else if(strValue.CompareNoCase(L"wrapContent") == 0)
				m_height = SIZE_WRAP_CONTENT;
			else
				m_height = _wtoi(strValue);
			return S_OK;
		}

		HRESULT OnAttrSize(const SStringW & strValue,BOOL bLoading)
		{
			SStringWList values;
			if(2!=SplitString(strValue,L",",values))
				return E_FAIL;
			OnAttrWidth(values[0],bLoading);
			OnAttrHeight(values[1],bLoading);
			return S_OK;
		}

		HRESULT OnAttrPos(const SStringW & strValue,BOOL bLoading)
		{
			return S_OK;
		}

		HRESULT OnAttrOffset(const SStringW & strValue,BOOL bLoading)
		{
			float fx,fy;
			if(2!=swscanf(strValue,L"%f,%f",&fx,&fy))
			{
				return E_FAIL;
			}
			fOffsetX = fx;
			fOffsetY = fy;
			return S_OK;
		}

		SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"width",OnAttrWidth)
            ATTR_CUSTOM(L"height",OnAttrHeight)
			ATTR_CUSTOM(L"pos",OnAttrPos)
			ATTR_CUSTOM(L"size",OnAttrSize)
			ATTR_CUSTOM(L"offset",OnAttrOffset)
        SOUI_ATTRS_BREAK()


	protected:
		float fOffsetX,fOffsetY;    /**< 窗口坐标偏移量, x += fOffsetX * width, y += fOffsetY * height  */

		int  m_width;        /**<使用width属性定义的宽 nCount==0 时有效*/
		int  m_height;       /**<使用height属性定义的高 nCount==0 时有效*/
	};

	class SouiLayout: public SObjectImpl<ILayout>
	{
		SOUI_CLASS_NAME(SouiLayout,L"SouiLayout")

	public:
		SouiLayout(void);
		~SouiLayout(void);
	};


}
