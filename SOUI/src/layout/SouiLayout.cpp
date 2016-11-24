#include "souistd.h"
#include "layout\SouiLayout.h"

namespace SOUI{
	SouiLayout::SouiLayout(void)
	{
	}

	SouiLayout::~SouiLayout(void)
	{
	}


	bool SoutLayoutParam::IsMatchParent(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height == SIZE_MATCH_PARENT):(m_width == SIZE_MATCH_PARENT);
	}

	bool SoutLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height > SIZE_SPEC):(m_width > SIZE_SPEC);
	}

	int SoutLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height):(m_width);
	}

}
