#include "souistd.h"
#include "layout\SGridLayout.h"

namespace SOUI
{
	HRESULT SGridLayoutParam::OnAttrSize(const SStringW & strValue,BOOL bLoading)
	{
		SStringWList szStr ;
		if(2!=SplitString(strValue,L',',szStr)) return E_FAIL;

		OnAttrWidth(szStr[0],bLoading);
		OnAttrHeight(szStr[1],bLoading);
		return S_OK;
	}

	HRESULT SGridLayoutParam::OnAttrWidth(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0)
			width.setMatchParent();
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			width.setWrapContent();
		else
			width.parseString(strValue);
		return S_OK;
	}

	HRESULT SGridLayoutParam::OnAttrHeight(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0)
			height.setMatchParent();
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			height.setWrapContent();
		else
			height.parseString(strValue);
		return S_OK;
	}

	SGridLayoutParam::SGridLayoutParam()
	{
		Clear();
	}


	void SGridLayoutParam::Clear()
	{
		width.setWrapContent();
		height.setWrapContent();
		iRow=iCol=-1;
		nColSpan=nRowSpan=1;
		xGravity = gCenter;
		yGravity = gMiddle;
	}

	void SGridLayoutParam::SetMatchParent(ORIENTATION orientation)
	{
		switch(orientation)
		{
		case Horz:
			width.setMatchParent();
			break;
		case Vert:
			height.setMatchParent();
			break;
		case Both:
			width.setMatchParent();
			height.setMatchParent();
			break;
		}
	}

	void SGridLayoutParam::SetWrapContent(ORIENTATION orientation)
	{
		switch(orientation)
		{
		case Horz:
			width.setWrapContent();
			break;
		case Vert:
			height.setWrapContent();
			break;
		case Both:
			width.setWrapContent();
			height.setWrapContent();
			break;
		}
	}

	void SGridLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const SLayoutSize& layoutSize)
	{
		switch(orientation)
		{
		case Horz:
			width = layoutSize;
			break;
		case Vert:
			height = layoutSize;
			break;
		case Both:
			width = height = layoutSize;
			break;
		}

	}

	bool SGridLayoutParam::IsMatchParent(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width.isMatchParent();
		case Vert:
			return height.isMatchParent();
		case Any:
			return IsMatchParent(Horz)|| IsMatchParent(Vert);
		case Both:
		default:
			return IsMatchParent(Horz) && IsMatchParent(Vert);
		}
	}

	bool SGridLayoutParam::IsWrapContent(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width.isWrapContent();
		case Vert:
			return height.isWrapContent();
		case Any:
			return IsWrapContent(Horz)|| IsWrapContent(Vert);
		case Both:
		default:
			return IsWrapContent(Horz) && IsWrapContent(Vert);
		}
	}

	bool SGridLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width.isSpecifiedSize();
		case Vert:
			return height.isSpecifiedSize();
		case Any:
			return IsSpecifiedSize(Horz)|| IsSpecifiedSize(Vert);
		case Both:
		default:
			return IsSpecifiedSize(Horz) && IsSpecifiedSize(Vert);
		}

	}

	SLayoutSize SGridLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width;
		case Vert:
			return height;
		case Any: 
		case Both:
		default:
			SASSERT_FMTA(FALSE,"GetSpecifiedSize can only be applied for Horz or Vert");
			return SLayoutSize();
		}
	}

	void * SGridLayoutParam::GetRawData()
	{
		return (SGridLayoutParam*)this;
	}


	//////////////////////////////////////////////////////////////////////////
	SGridLayout::SGridLayout(void)
	{
	}

	SGridLayout::~SGridLayout(void)
	{
	}

	bool SGridLayout::IsParamAcceptable(ILayoutParam *pLayoutParam) const
	{
		return !!pLayoutParam->IsClass(SGridLayoutParam::GetClassName());
	}

	void SGridLayout::LayoutChildren(SWindow * pParent)
	{
	}

	ILayoutParam * SGridLayout::CreateLayoutParam() const
	{
		return new SGridLayoutParam();
	}


	CSize SGridLayout::MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const
	{
		return CSize();
	}

}
