#pragma once


#include "propgrid\SPropertyGrid.h"

namespace SOUI
{
	class SPropertyGridEx : public SPropertyGrid
	{
		SOUI_CLASS_NAME(SPropertyGridEx, L"propgridex");



	public:
		SPropertyGridEx(void);
		~SPropertyGridEx(void);


		virtual void DrawItem(IRenderTarget *pRT, CRect &rc, int iItem);
        virtual void OnInplaceActiveWndCreate(IPropertyItem *pItem,SWindow *pWnd,pugi::xml_node xmlInit);
	
	protected:
		COLORREF m_crGroup;       //Group背景颜色
		COLORREF m_crItem;        //Item背景颜色
		COLORREF m_crItemText;    //Item文本颜色
		COLORREF m_crItemSel;     //Item选中时的背景色
		SStringT m_strEditBkgndColor; //edit的背景色;
		SStringT m_strEditTextColor; //edit的文本颜色;
		COLORREF m_crBorder;      //边框颜色
		SStringT    m_strEnableAutoWordSel;    /**< enable Word style auto word selection?  */

		SOUI_ATTRS_BEGIN()
			//ATTR_INT(L"itemHeight", m_nItemHei, FALSE)
			//ATTR_SKIN(L"itemSkin", m_pItemSkin, TRUE)
			//ATTR_SKIN(L"iconSkin", m_pIconSkin, TRUE)
			//ATTR_COLOR(L"colorItemBkgnd",m_crItemBg,FALSE)
			//ATTR_COLOR(L"colorItemBkgnd2", m_crItemBg2, FALSE)
			//ATTR_COLOR(L"colorItemSelBkgnd",m_crItemSelBg,FALSE)
			//ATTR_COLOR(L"colorItemHotBkgnd",m_crItemHotBg,FALSE)
			//ATTR_COLOR(L"colorText",m_crText,FALSE)
			ATTR_COLOR(L"ColorGroup",m_crGroup,FALSE)
			ATTR_COLOR(L"ColorItem",m_crItem,FALSE)
			ATTR_COLOR(L"ColorItemText",m_crItemText,FALSE)
			ATTR_COLOR(L"ColorItemText",m_crItemText,FALSE)
			ATTR_COLOR(L"ColorItemSel",m_crItemSel,FALSE)
			ATTR_STRINGT(L"EditBkgndColor",m_strEditBkgndColor,FALSE)
			ATTR_STRINGT(L"EditTextColor",m_strEditTextColor,FALSE)
			ATTR_COLOR(L"ColorBorder",m_crBorder,FALSE)
            ATTR_STRINGT(L"autoWordSel",m_strEnableAutoWordSel,FALSE)
			//ATTR_INT(L"icon-x", m_ptIcon.x, FALSE)
			//ATTR_INT(L"icon-y", m_ptIcon.y, FALSE)
			//ATTR_INT(L"text-x", m_ptText.x, FALSE)
			//ATTR_INT(L"text-y", m_ptText.y, FALSE)
			//ATTR_INT(L"hotTrack",m_bHotTrack,FALSE)
			SOUI_ATTRS_END()

		//	const COLORREF KColorHead  = RGBA(128,128,128,255);   //头的颜色             灰色
		//const COLORREF KColorGroup = RGBA(128,128,128,255);   //组的颜色             灰色
		//const COLORREF KColorItem  = RGBA(255,255,255,255);   //未选中的颜色         白色
		//const COLORREF KColorItemSel  = RGBA(0,0,128,255);    //选中时item的颜色     暗蓝
		//const COLORREF KColorBorder = RGBA(0,0,0,255);        //边框颜色             黑色




	};

}