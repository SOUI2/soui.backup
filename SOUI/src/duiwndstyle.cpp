//////////////////////////////////////////////////////////////////////////
//   File Name: DuiWndStyle.h
// Description: DuiWindow Styles Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "duiwndstyle.h"
#include "DuiSystem.h"
#include "mybuffer.h"

namespace SOUI
{

DuiStyle::DuiStyle()
    : m_uAlign(Align_Center)
	, m_uVAlign(VAlign_Middle)
    , m_nTextAlign(0)
    , m_crBg(CLR_INVALID)
    , m_crBgHover(CLR_INVALID)
    , m_crText(CLR_INVALID)
    , m_crHoverText(CLR_INVALID)
    , m_crDisabledText(RGB(0xA0, 0xA0, 0xA0))
    , m_crPushText(CLR_INVALID)
	, m_crBorder(CLR_INVALID)
	, m_crBorderHover(CLR_INVALID)
    , m_ftText(NULL)
    , m_ftHover(NULL)
    , m_ftPush(NULL)
    , m_nMarginX(0)
    , m_nMarginY(0)
    , m_nSpacing(0)
    , m_nLineSpacing(20)
    , m_lpCursorName(IDC_ARROW)
    , m_bDotted(FALSE)
{
}

UINT DuiStyle::GetTextAlign()
{
	if(m_nTextAlign) return m_nTextAlign;
	UINT uRet=0;
	switch(m_uAlign)
	{
	case Align_Center: uRet |= DT_CENTER;break;
	case Align_Right: uRet|= DT_RIGHT;break;
	}
	switch(m_uVAlign)
	{
	case VAlign_Middle:uRet |= DT_VCENTER|DT_SINGLELINE;break;
	case VAlign_Bottom:uRet|= DT_BOTTOM|DT_SINGLELINE;break;
	}
	if(m_bDotted) uRet|=DT_END_ELLIPSIS;
	return uRet;
}


}//namespace SOUI