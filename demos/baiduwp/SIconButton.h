#pragma once

#include <control/SCmnCtrl.h>

namespace SOUI
{
	class SIconButton : public SImageButton
	{
		SOUI_CLASS_NAME(SIconButton,L"iconbtn")
	protected:
		ISkinObj *m_pSkinIcon; /**< ISkibObj对象  */
		CPoint m_ptIcon;   /**< 图标位置 */
		CPoint m_ptText;   /**< 标题位置 */
	public:
		SIconButton(void);
		~SIconButton(void);

	protected:
		virtual void GetTextRect(LPRECT pRect);
		void OnPaint(IRenderTarget *pRT);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
		SOUI_MSG_MAP_END()

		SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"iconSkin", m_pSkinIcon, FALSE)            
			ATTR_INT(L"icon-x", m_ptIcon.x, FALSE)
			ATTR_INT(L"icon-y", m_ptIcon.y, FALSE)
			ATTR_INT(L"text-x", m_ptText.x, FALSE)
			ATTR_INT(L"text-y", m_ptText.y, FALSE)
		SOUI_ATTRS_END()
	};
}

