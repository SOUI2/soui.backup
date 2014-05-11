#include "StdAfx.h"
#include "DuiFramePreview.h"

CDuiFramePreview::CDuiFramePreview(LPCTSTR pszFrameName):CDuiHostWnd(pszFrameName)
{
}

CDuiFramePreview::~CDuiFramePreview(void)
{
}

void CDuiFramePreview::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if(nChar==VK_ESCAPE)
	{
		EndDialog(IDCANCEL);
	}else
	{
		SetMsgHandled(FALSE);
	}
}