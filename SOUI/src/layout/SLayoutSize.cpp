#include "souistd.h"
#include "layout/SLayoutSize.h"
#include <math.h>

namespace SOUI
{
	static const wchar_t* s_pszUnit[] =
	{
		L"px",L"dp"
	};

	SLayoutSize::SLayoutSize() :fSize(0.0f),unit(px)
	{

	}

	static bool fequal(float a, float b)
	{
		return fabs(a-b)<0.00000001f;
	}

	SStringW SLayoutSize::toString() const
	{
		SStringW strValue = SStringW().Format(L"%f",fSize);
		//去掉sprintf("%f")生成的小数点最后无效的0
		LPCWSTR pszData = strValue;
		for(int i=strValue.GetLength()-1;i>=0;i--)
		{
			if(pszData[i]!=L'0')
			{
				if(pszData[i]==L'.') i--;
				strValue = strValue.Left(i+1);
				break;
			}
		}
		return SStringW().Format(L"%s%s",strValue,s_pszUnit[unit]);
	}


	bool SLayoutSize::isMatchParent() const
	{
		return fequal(fSize , SIZE_MATCH_PARENT);
	}

	void SLayoutSize::setMatchParent()
	{
		fSize = SIZE_MATCH_PARENT;
	}

	bool SLayoutSize::isWrapContent() const
	{
		return fequal(fSize , SIZE_WRAP_CONTENT);
	}

	void SLayoutSize::setWrapContent()
	{
		fSize = SIZE_WRAP_CONTENT;
	}

	bool SLayoutSize::isSpecifiedSize() const
	{
		return fSize>=SIZE_SPEC;
	}

	int SLayoutSize::toPixelSize() const
	{
		if(isMatchParent()) 
			return SIZE_MATCH_PARENT;
		else if(isWrapContent()) 
			return SIZE_WRAP_CONTENT;
		else if(unit == px)
			return (int)fSize;
		else
			return SDp2Px(fSize,isVert());
	}

	void SLayoutSize::parseString(const SStringW & strSize)
	{
		SStringW strUnit = strSize.Right(2);
		strUnit.MakeLower();
		unit = px;
		for(int i=0; i< ARRAYSIZE(s_pszUnit);i++)
		{
			if(strUnit.Compare(s_pszUnit[i]) == 0)
			{
				unit = (Unit)i;
				break;
			}
		}
		fSize = (float)_wtof(strSize);
	}

	bool SLayoutSize::isValid() const
	{
		return fequal(fSize,SIZE_UNDEF);
	}


	template<class T>
	T LayoutSizeFromString(const SStringW & strSize)
	{
		T ret;
		ret.parseString(strSize);
		return ret;			
	}

	bool SLayoutWidth::isVert() const
	{
		return false;
	}

	SLayoutWidth SLayoutWidth::fromString(const SStringW & strSize)
	{
		return LayoutSizeFromString<SLayoutWidth>(strSize);
	}

	SLayoutSize & SLayoutWidth::operator=(const SLayoutSize& src)
	{
		fSize = src.fSize;
		unit = src.unit;
		return *this;
	}


	bool SLayoutHeight::isVert() const
	{
		return true;
	}

	SLayoutHeight SLayoutHeight::fromString(const SStringW & strSize)
	{
		return LayoutSizeFromString<SLayoutHeight>(strSize);
	}

	SLayoutSize & SLayoutHeight::operator=(const SLayoutSize& src)
	{
		fSize = src.fSize;
		unit = src.unit;
		return *this;
	}

}
