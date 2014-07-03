#pragma once

class CDuiFramePreview :
	public CDuiHostWnd
{
public:
	CDuiFramePreview(LPCTSTR pszFrameName);
	~CDuiFramePreview(void);

protected:
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	BEGIN_MSG_MAP_EX(CDuiFramePreview)
		MSG_WM_KEYDOWN(OnKeyDown)
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

};
