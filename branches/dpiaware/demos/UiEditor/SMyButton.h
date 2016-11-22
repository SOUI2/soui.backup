#pragma once
#include <core/Swnd.h>
#include <control/SCmnCtrl.h>

namespace SOUI
{
	class SMyButton:public SButton
	{
		SOUI_CLASS_NAME(SSwitch,L"mybutton")
	public:
		//SToolbox():m_IconOffsetX(5), m_IconOffsetY(8), m_TextOffsetX(0), m_TextOffsetY(18), m_pIcon(NULL)
		//{

		//}
		SMyButton()
		{
			m_hLUpRDown=GETRESPROVIDER->LoadCursor(L"sizenwse");
			m_hAll=GETRESPROVIDER->LoadCursor(L"sizeall");
			m_hNormal=GETRESPROVIDER->LoadCursor(m_style.m_strCursor);
		}
		~SMyButton(){}

	protected://消息处理，SOUI控件的消息处理和WTL，MFC很相似，采用相似的映射表，相同或者相似的消息映射宏

		void OnMouseMove(UINT nFlags,CPoint pt);
		BOOL OnSetCursor(const CPoint &pt);
        void OnLButtonDown(UINT nFlags,CPoint pt);
	
		//BOOL OnNcHitTest(CPoint pt);
		//void OnPaint(IRenderTarget *pRT);
		//SOUI控件消息映射表
		SOUI_MSG_MAP_BEGIN()	
			MSG_WM_MOUSEMOVE(OnMouseMove)
			//MSG_WM_NCHITTEST(OnNcHitTest)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
		SOUI_MSG_MAP_END()

		//	ISkinObj    *m_pIcon;	//图标
		//SStringT    m_strText; //文字
		//int			m_IconOffsetX, m_IconOffsetY, m_TextOffsetX, m_TextOffsetY;//相对居中位置偏移量
		//SOUI_ATTRS_BEGIN()
		//	ATTR_SKIN(L"icon", m_pIcon, TRUE)
		//	ATTR_STRINGT(L"text", m_strText, FALSE)
		//	SOUI_ATTRS_END()

	public:
		INT Oldx;
		INT Oldy;
		HCURSOR m_hLUpRDown;
		HCURSOR m_hAll;
		HCURSOR m_hNormal;
	};
}