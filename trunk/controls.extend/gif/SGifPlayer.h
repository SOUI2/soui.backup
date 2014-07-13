#pragma once
#include <core/swnd.h>
#include "SSkinGif.h"

namespace SOUI
{

class SGifPlayer : public SWindow
{
	SOUI_CLASS_NAME(SGifPlayer, L"gifplayer")
public:
	SGifPlayer();
	~SGifPlayer();

    BOOL PlayGifFile(LPCTSTR pszFileName);

	SOUI_ATTRS_BEGIN()		
		ATTR_CUSTOM(L"skin", OnAttrGif)		
	SOUI_ATTRS_END()

protected:
	HRESULT OnAttrGif(const SStringW & strValue, BOOL bLoading);

	virtual CSize GetDesiredSize(LPRECT pRcContainer);

	void OnPaint(IRenderTarget *pRT);
	void OnTimer(char cTimerID);
	void OnShowWindow(BOOL bShow, UINT nStatus);

	SOUI_MSG_MAP_BEGIN()	
		MSG_WM_TIMER_EX(OnTimer)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_PAINT_EX(OnPaint)			
    SOUI_MSG_MAP_END()	

private:
	SSkinGif *m_pgif;
	int	m_iCurFrame;
};

}