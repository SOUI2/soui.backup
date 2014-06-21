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

class SOUI_EXP DuiStyle : public SObject
{
    SOUI_CLASS_NAME(DuiStyle, "style")

    enum
    {
        // Specify by "cursor" attribute
        Cursor_Mask     = 0xF00UL,
        Cursor_Arrow    = 0x000UL,   // cursor = "arrow"
        Cursor_Hand     = 0x100UL,   // cursor = "hand"


        Align_Left                = 0x000UL, // valign = top
        Align_Center            = 0x100UL, // valign = middle
        Align_Right                = 0x200UL, // valign = bottom

        VAlign_Top                = 0x0000UL, // valign = top
        VAlign_Middle            = 0x1000UL, // valign = middle
        VAlign_Bottom            = 0x2000UL, // valign = bottom

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
    IFontPtr m_ftText;
    IFontPtr m_ftHover;
    IFontPtr m_ftPush;
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

    SOUI_ATTRS_BEGIN()
        ATTR_STRINGA("skin", m_strSkinName, TRUE)
        ATTR_STRINGA("ncskin", m_strNcSkinName, TRUE)
        ATTR_HEX("textmode", m_nTextAlign, TRUE)

        ATTR_ENUM_BEGIN("align", UINT, TRUE)
            ATTR_ENUM_VALUE("left", Align_Left)
            ATTR_ENUM_VALUE("center", Align_Center)
            ATTR_ENUM_VALUE("right", Align_Right)
        ATTR_ENUM_END(m_uAlign)
        ATTR_ENUM_BEGIN("valign", UINT, TRUE)
            ATTR_ENUM_VALUE("top", VAlign_Top)
            ATTR_ENUM_VALUE("middle", VAlign_Middle)
            ATTR_ENUM_VALUE("bottom", VAlign_Bottom)
        ATTR_ENUM_END(m_uVAlign)

        ATTR_COLOR("crbg", m_crBg, TRUE)
        ATTR_COLOR("crbghover", m_crBgHover, TRUE)
        ATTR_COLOR("crtext", m_crText, TRUE)
        ATTR_COLOR("crhover", m_crHoverText, TRUE)
        ATTR_COLOR("crpush", m_crPushText, TRUE)
        ATTR_COLOR("crdisabled", m_crDisabledText, TRUE)
        ATTR_COLOR("crborder", m_crBorder, TRUE)
        ATTR_COLOR("crborderhover", m_crBorderHover, TRUE)

        ATTR_FONT("font", m_ftText, TRUE)
        ATTR_FONT("hoverfont", m_ftHover, TRUE)
        ATTR_FONT("pushfont", m_ftPush, TRUE)
        ATTR_FONTEX("font2", m_ftText, TRUE)
        ATTR_FONTEX("hoverfont2", m_ftHover, TRUE)
        ATTR_FONTEX("pushfont2", m_ftPush, TRUE)
        ATTR_INT("x-margin", m_nMarginX, TRUE)
        ATTR_INT("y-margin", m_nMarginY, TRUE)
        ATTR_INT("margin", m_nMarginX = m_nMarginY, TRUE) // 这样比较bt，不过.....凑合用吧
        ATTR_INT("spacing", m_nSpacing, TRUE)
        ATTR_INT("linespacing", m_nLineSpacing, TRUE)
        ATTR_ENUM_BEGIN("cursor", LPCTSTR, FALSE)
            ATTR_ENUM_VALUE("hand", IDC_HAND)
            ATTR_ENUM_VALUE("arrow", IDC_ARROW)
        ATTR_ENUM_END(m_lpCursorName)
        ATTR_INT("dotted",m_bDotted,FALSE)
    SOUI_ATTRS_END()
};


}//namespace SOUI