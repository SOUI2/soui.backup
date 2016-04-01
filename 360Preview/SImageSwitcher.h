#pragma once

/**
by 多点免费WIFI
 */
#include <core/Swnd.h>

namespace SOUI
{
    #define TIMER_MOVE		1
	class SImageSwitcher : public SWindow
	{
		SOUI_CLASS_NAME(SImageSwitcher,L"imageSwitcher")
	public:
		SImageSwitcher();
		virtual ~SImageSwitcher();

	public:
		void  Switch(int iSelect);
		void  InsertImage(UINT iTo, IBitmap * pImage);
		void  RemoveAll();
		
	private:
	    
	    SArray<IBitmap *> m_lstImages;
	    
		ISkinObj *m_pSkinLightLevel;

		BOOL m_bWantMove;
		int  m_iDownX;
		BOOL m_bTimerMove;
		int  m_iMoveWidth;
		int  m_iSelected;
		int m_iTimesMove;

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

        HRESULT OnAttrImages(const SStringW strValue,BOOL bLoading);
        
		SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"skinlightlevel", m_pSkinLightLevel, TRUE)
			ATTR_CUSTOM(L"images",OnAttrImages)
		SOUI_ATTRS_END()
	};
}