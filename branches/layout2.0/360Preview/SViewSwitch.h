#pragma once

/**
by 多点免费WIFI
 */
#include <core/Swnd.h>

namespace SOUI
{
#define MAX_VEIWPAGE_COUNT 6
#define TIMER_MOVE		1
	class SViewSwitch : public SWindow
	{
		SOUI_CLASS_NAME(SViewSwitch,L"viewswitch")
	public:
		SViewSwitch();
		virtual ~SViewSwitch();

	public:
		void  SWitch(int nSelect);
	private:
		DWORD   m_dwPageCount;
		ISkinObj *m_pSkin[MAX_VEIWPAGE_COUNT];
		ISkinObj *m_pSkinLightLevel;

		BOOL m_bWantMove;
		int  m_iDownX;
		BOOL m_bTimerMove;
		int  m_iMoveWidth;
		int  m_iSelected;
		int m_iTimesMove;
		int m_SelButton;

	protected:
		void OnPaint(IRenderTarget *pRT);
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnMouseMove(UINT nFlags,CPoint pt);
		void OnTimer(char nIDEvent);
	protected:
		SOUI_MSG_MAP_BEGIN()	
			MSG_WM_PAINT_EX(OnPaint)    //窗口绘制消息
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_TIMER_EX(OnTimer)
			SOUI_MSG_MAP_END()

			SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"skinlightlevel", m_pSkinLightLevel, TRUE)
			ATTR_SKIN(L"skin1", m_pSkin[0], TRUE)
			ATTR_SKIN(L"skin2", m_pSkin[1], TRUE)
			ATTR_SKIN(L"skin3", m_pSkin[2], TRUE)
			ATTR_SKIN(L"skin4", m_pSkin[3], TRUE)
			ATTR_SKIN(L"skin5", m_pSkin[4], TRUE)
			ATTR_SKIN(L"skin6", m_pSkin[5], TRUE)
			SOUI_ATTRS_END()
	};
}