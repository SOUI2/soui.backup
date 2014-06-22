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
    COLORREF m_crBorder;
    COLORREF m_crText[4];
    IFontPtr m_ftText[4];

    int m_nMarginX;
    int m_nMarginY;
    int m_nSpacing;
    int m_nLineSpacing;
    BOOL m_bDotted;

    LPCTSTR m_lpCursorName;
    CDuiStringA m_strSkinName,m_strNcSkinName;

    UINT GetTextAlign();
    int GetStates();
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
        ATTR_COLOR("crborder", m_crBorder, TRUE)

        ATTR_FONT("font", m_ftText[0], TRUE)
        ATTR_FONTEX("font2", m_ftText[0], TRUE)
        ATTR_FONT("font.hover", m_ftText[1], TRUE)
        ATTR_FONTEX("font2.hover", m_ftText[1], TRUE)
        ATTR_FONT("font.push", m_ftText[2], TRUE)
        ATTR_FONTEX("font2.push", m_ftText[2], TRUE)
        ATTR_FONT("font.disable", m_ftText[3], TRUE)
        ATTR_FONTEX("font2.disable", m_ftText[3], TRUE)

        ATTR_COLOR("crtext", m_crText[0], TRUE)
        ATTR_COLOR("crtext.hover", m_crText[1], TRUE)
        ATTR_COLOR("crtext.push", m_crText[2], TRUE)
        ATTR_COLOR("crtext.disable", m_crText[3], TRUE)

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