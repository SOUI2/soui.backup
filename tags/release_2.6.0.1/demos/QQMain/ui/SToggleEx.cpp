#include "stdafx.h"
#include "SToggleEx.h"

namespace SOUI
{
	void SToggleEx::OnLButtonUp(UINT nFlags,CPoint pt)
	{
		if(GetWindowRect().PtInRect(pt)) m_bToggled=!m_bToggled;
		__super::OnLButtonUp(nFlags,pt);
	}

}
 