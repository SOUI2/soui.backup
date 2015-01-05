//////////////////////////////////////////////////////////////////////////
//   File Name: SwndStyle.h
//////////////////////////////////////////////////////////////////////////

#include "souistd.h"
#include "core/Swndstyle.h"

namespace SOUI
{

SwndStyle::SwndStyle()
    : m_uAlign(Align_Center)
    , m_uVAlign(VAlign_Middle)
    , m_nTextAlign(0)
    , m_crBg(CR_INVALID)
    , m_crBorder(CR_INVALID)
    , m_nMarginX(0)
    , m_nMarginY(0)
    , m_strCursor(_T("arrow"))
    , m_byAlpha(0xFF)
    , m_bApplyAlpha2Children(0)
    , m_bDotted(0)
    , m_bTrackMouseEvent(0)
    , m_bBkgndBlend(1)
    , m_bySepSpace(5)
{
    for(int i=0; i<4; i++)
    {
        m_ftText[i]=NULL;
        m_crText[i]=CR_INVALID;
    }
}

UINT SwndStyle::GetTextAlign()
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

int SwndStyle::GetStates()
{
    int fonts=1,colors=1;
    for(int i=1;i<4;i++)
    {
        if(m_ftText[i]!=NULL) fonts++;
        if(m_crText[i]!=CR_INVALID) colors++;
    }
    return max(fonts,colors);
}

COLORREF SwndStyle::GetTextColor( int iState )
{
    if(m_crText[iState]==CR_INVALID) iState=0;
    return m_crText[iState];
}

IFontPtr SwndStyle::GetTextFont( int iState )
{
    if(!m_ftText[iState]) iState=0;
    return m_ftText[iState];
}

}//namespace SOUI