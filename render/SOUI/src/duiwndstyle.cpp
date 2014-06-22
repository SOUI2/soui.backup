//////////////////////////////////////////////////////////////////////////
//   File Name: DuiWndStyle.h
// Description: DuiWindow Styles Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "duiwndstyle.h"

namespace SOUI
{

DuiStyle::DuiStyle()
    : m_uAlign(Align_Center)
    , m_uVAlign(VAlign_Middle)
    , m_nTextAlign(0)
    , m_crBg(CLR_INVALID)
    , m_crBorder(CLR_INVALID)
    , m_nMarginX(0)
    , m_nMarginY(0)
    , m_nSpacing(0)
    , m_nLineSpacing(20)
    , m_lpCursorName(IDC_ARROW)
    , m_bDotted(FALSE)
{
    for(int i=0; i<4; i++)
    {
        m_ftText[i]=NULL;
        m_crText[i]=CLR_INVALID;
    }
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

int DuiStyle::GetStates()
{
    int fonts=1,colors=1;
    for(int i=1;i<4;i++)
    {
        if(m_ftText[i]!=NULL) fonts++;
        if(m_crText[i]!=CLR_INVALID) colors++;
    }
    return max(fonts,colors);
}

}//namespace SOUI