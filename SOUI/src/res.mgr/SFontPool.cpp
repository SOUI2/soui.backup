//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "res.mgr/sfontpool.h"

namespace SOUI
{

template<> SFontPool* SSingleton<SFontPool>::ms_Singleton    = 0;


SFontPool::SFontPool(IRenderFactory *pRendFactory)
    : m_lFontSize(-11)
    , m_RenderFactory(pRendFactory)
{
    _tcscpy(m_szDefFontFace,_T("宋体"));
    m_pFunOnKeyRemoved=OnKeyRemoved;
    SetKeyObject(FontKey(FF_DEFAULTFONT),_CreateDefaultFont());
}

IFontPtr SFontPool::GetFont(WORD uKey,LPCTSTR pszFaceName)
{
    SStringT strFaceName(pszFaceName);
    if(strFaceName.IsEmpty()) strFaceName = m_szDefFontFace;
    
    IFontPtr hftRet=0;
    FontKey key(uKey,strFaceName);
    if(HasKey(key))
    {
        hftRet=GetKeyObject(key);
    }
    else
    {
        hftRet = _CreateFont(
                     FF_ISBOLD(uKey), FF_ISUNDERLINE(uKey), FF_ISITALIC(uKey), FF_GETADDING(uKey),strFaceName
                 );

        AddKeyObject(key,hftRet);
    }
    return hftRet;
}

IFontPtr SFontPool::GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding /*= 0*/,LPCTSTR strFaceName/*=""*/)
{
    return GetFont(FF_MAKEKEY(bBold, bUnderline, bItalic, chAdding),strFaceName);
}

void SFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize)
{
    _tcscpy_s(m_szDefFontFace,_countof(m_szDefFontFace),lpszFaceName);
    m_lFontSize = -abs(lSize);

    ASSERT(GetCount()==1);//初始化前才可以调用该接口

    RemoveKeyObject(FontKey(FF_DEFAULTFONT));
    
    SetKeyObject(FontKey(FF_DEFAULTFONT),_CreateDefaultFont());
}

IFontPtr SFontPool::_CreateDefaultFont()
{
    ::GetObjectA(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDefault);

    m_lfDefault.lfHeight = -_GetFontAbsHeight(m_lFontSize);
    _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),  m_szDefFontFace);

    m_lfDefault.lfQuality = ANTIALIASED_QUALITY;
    
    ASSERT(m_RenderFactory);

    IFontPtr pFont=NULL;
    m_RenderFactory->CreateFont(&pFont,m_lfDefault);

    return pFont;
}

IFontPtr SFontPool::_CreateFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding,SStringT strFaceName)
{
    LOGFONT lfNew;

    memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));
    lfNew.lfWeight      = (bBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != bUnderline);
    lfNew.lfItalic      = (FALSE != bItalic);

    lfNew.lfHeight = -_GetFontAbsHeight(lfNew.lfHeight - chAdding);//lfNew.lfHeight应该为负值

    lfNew.lfQuality = CLEARTYPE_NATURAL_QUALITY;

    IFontPtr pFont=NULL;
    ASSERT(m_RenderFactory);
    m_RenderFactory->CreateFont(&pFont,lfNew);

    return pFont;
}

LONG SFontPool::_GetFontAbsHeight(LONG lSize)
{
    return lSize<0? (-lSize):lSize;
}

}//namespace SOUI