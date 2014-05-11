//////////////////////////////////////////////////////////////////////////
//   File Name: DuiWndStyle.h
// Description: DuiWindow Styles Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duiskin.h"

namespace SOUI
{

class SOUI_EXP DuiStyle : public CDuiObject
{
    SOUI_CLASS_NAME(DuiStyle, "style")

    enum
    {
        // Specify by "cursor" attribute
        Cursor_Mask     = 0xF00UL,
        Cursor_Arrow    = 0x000UL,   // cursor = "arrow"
        Cursor_Hand     = 0x100UL,   // cursor = "hand"


		Align_Left				= 0x000UL, // valign = top
		Align_Center			= 0x100UL, // valign = middle
		Align_Right				= 0x200UL, // valign = bottom

		VAlign_Top				= 0x0000UL, // valign = top
		VAlign_Middle			= 0x1000UL, // valign = middle
		VAlign_Bottom			= 0x2000UL, // valign = bottom

    };
public:
    DuiStyle();


    COLORREF m_crBg;
    COLORREF m_crBgHover;
    COLORREF m_crText;
    COLORREF m_crHoverText;
    COLORREF m_crDisabledText;
    COLORREF m_crPushText;
	COLORREF m_crBorder;
	COLORREF m_crBorderHover;
    HFONT m_ftText;
    HFONT m_ftHover;
    HFONT m_ftPush;
    int m_nMarginX;
    int m_nMarginY;
    int m_nSpacing;
    int m_nLineSpacing;
	BOOL m_bDotted;

    LPCTSTR m_lpCursorName;
    CDuiStringA m_strSkinName,m_strNcSkinName;

	UINT GetTextAlign();
protected:
	UINT m_nTextAlign;
	UINT m_uAlign,m_uVAlign;

    SOUO_ATTRIBUTES_BEGIN()
    DUIWIN_STRING_ATTRIBUTE("skin", m_strSkinName, TRUE)
    DUIWIN_STRING_ATTRIBUTE("ncskin", m_strNcSkinName, TRUE)
    DUIWIN_HEX_ATTRIBUTE("textmode", m_nTextAlign, TRUE)

	DUIWIN_ENUM_ATTRIBUTE("align", UINT, TRUE)
	DUIWIN_ENUM_VALUE("left", Align_Left)
	DUIWIN_ENUM_VALUE("center", Align_Center)
	DUIWIN_ENUM_VALUE("right", Align_Right)
	DUIWIN_ENUM_END(m_uAlign)
	DUIWIN_ENUM_ATTRIBUTE("valign", UINT, TRUE)
	DUIWIN_ENUM_VALUE("top", VAlign_Top)
	DUIWIN_ENUM_VALUE("middle", VAlign_Middle)
	DUIWIN_ENUM_VALUE("bottom", VAlign_Bottom)
	DUIWIN_ENUM_END(m_uVAlign)

    DUIWIN_COLOR_ATTRIBUTE("crbg", m_crBg, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crbghover", m_crBgHover, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crtext", m_crText, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crhover", m_crHoverText, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crpush", m_crPushText, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crdisabled", m_crDisabledText, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crborder", m_crBorder, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("crborderhover", m_crBorderHover, TRUE)

    DUIWIN_FONT_ATTRIBUTE("font", m_ftText, TRUE)
    DUIWIN_FONT_ATTRIBUTE("hoverfont", m_ftHover, TRUE)
    DUIWIN_FONT_ATTRIBUTE("pushfont", m_ftPush, TRUE)
	DUIWIN_FONT2_ATTRIBUTE("font2", m_ftText, TRUE)
	DUIWIN_FONT2_ATTRIBUTE("hoverfont2", m_ftHover, TRUE)
	DUIWIN_FONT2_ATTRIBUTE("pushfont2", m_ftPush, TRUE)
    DUIWIN_INT_ATTRIBUTE("x-margin", m_nMarginX, TRUE)
    DUIWIN_INT_ATTRIBUTE("y-margin", m_nMarginY, TRUE)
    DUIWIN_INT_ATTRIBUTE("margin", m_nMarginX = m_nMarginY, TRUE) // 这样比较bt，不过.....凑合用吧
    DUIWIN_INT_ATTRIBUTE("spacing", m_nSpacing, TRUE)
    DUIWIN_INT_ATTRIBUTE("linespacing", m_nLineSpacing, TRUE)
    DUIWIN_ENUM_ATTRIBUTE("cursor", LPCTSTR, FALSE)
    DUIWIN_ENUM_VALUE("hand", IDC_HAND)
    DUIWIN_ENUM_VALUE("arrow", IDC_ARROW)
    DUIWIN_ENUM_END(m_lpCursorName)
    DUIWIN_INT_ATTRIBUTE("dotted",m_bDotted,FALSE)
    SOUI_ATTRIBUTES_END()
};


}//namespace SOUI