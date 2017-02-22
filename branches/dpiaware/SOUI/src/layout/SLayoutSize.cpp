#include "souistd.h"
#include "layout/SLayoutSize.h"

namespace SOUI
{
	static const wchar_t* s_pszUnit[] =
	{
		L"px",L"dp",L"sp"
	};

	SLayoutSize::SLayoutSize() :fSize(0.0f),unit(px)
	{

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

	SLayoutSize SLayoutSize::fromString(const SStringW & strSize)
	{
		SLayoutSize ret;
		SStringW strUnit = strSize.Right(2);
		strUnit.MakeLower();
		ret.unit = px;
		for(int i=0; i< ARRAYSIZE(s_pszUnit);i++)
		{
			if(strUnit.Compare(s_pszUnit[i]) == 0)
			{
				ret.unit = (Unit)i;
				break;
			}
		}
		ret.fSize = (float)_wtof(strSize);
		return ret;
	}


}
