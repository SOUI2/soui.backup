//////////////////////////////////////////////////////////////////////////
//  Class Name: DUIFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duistd.h"
#include "res.mgr/duifontpool.h"

namespace SOUI
{

template<> DuiFontPool* Singleton<DuiFontPool>::ms_Singleton    = 0;


DuiFontPool::DuiFontPool(IRenderFactory *pRendFactory)
    : m_lFontSize(-11)
    , m_RenderFactory(pRendFactory)
{
    _tcscpy(m_szDefFontFace,_T("Tahoma"));
    m_pFunOnKeyRemoved=OnKeyRemoved;
    SetKeyObject(FontKey(DUIF_DEFAULTFONT),_CreateDefaultGUIFont());
}

IFontPtr DuiFontPool::GetFont(WORD uKey,LPCTSTR strFaceName)
{
    IFontPtr hftRet=0;
    FontKey key(uKey,strFaceName);
    if(HasKey(key))
    {
        hftRet=GetKeyObject(key);
    }
    else
    {
        hftRet = _CreateNewFont(
                     DUIF_ISBOLD(uKey), DUIF_ISUNDERLINE(uKey), DUIF_ISITALIC(uKey), DUIF_GETADDING(uKey),strFaceName
                 );

        AddKeyObject(key,hftRet);
    }
    return hftRet;
}

IFontPtr DuiFontPool::GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding /*= 0*/,LPCTSTR strFaceName/*=""*/)
{
    return GetFont(DUIF_MAKEKEY(bBold, bUnderline, bItalic, chAdding),strFaceName);
}

void DuiFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize)
{
    _tcscpy_s(m_szDefFontFace,_countof(m_szDefFontFace),lpszFaceName);
    m_lFontSize = -abs(lSize);

    ASSERT(GetCount()==1);//初始化前才可以调用该接口

    RemoveKeyObject(FontKey(DUIF_DEFAULTFONT));
    
    SetKeyObject(FontKey(DUIF_DEFAULTFONT),_CreateDefaultGUIFont());
}

IFontPtr DuiFontPool::_CreateDefaultGUIFont()
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

IFontPtr DuiFontPool::_CreateNewFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding,SStringT strFaceName/*=""*/)
{
    LOGFONT lfNew;

    memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));
    if(!strFaceName.IsEmpty())
        _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),strFaceName);
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

LONG DuiFontPool::_GetFontAbsHeight(LONG lSize)
{
    return lSize<0? (-lSize):lSize;
}

}//namespace SOUI