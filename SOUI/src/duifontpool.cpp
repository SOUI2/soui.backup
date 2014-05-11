//////////////////////////////////////////////////////////////////////////
//  Class Name: DUIFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duistd.h"
#include "duifontpool.h"

namespace SOUI
{

template<> DuiFontPool* Singleton<DuiFontPool>::ms_Singleton	= 0;


DuiFontPool::DuiFontPool()
    :  m_lFontSize(-11)
{
    _tcscpy(m_szDefFontFace,_T("Tahoma"));
    m_pFunOnKeyRemoved=OnKeyRemoved;
    SetKeyObject(FontKey(DUIF_DEFAULTFONT),_CreateDefaultGUIFont());
}

HFONT DuiFontPool::GetFont(WORD uKey,LPCTSTR strFaceName)
{
    HFONT hftRet=0;
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
    DUIASSERT(GetObjectType(hftRet)==OBJ_FONT);
    return hftRet;
}

HFONT DuiFontPool::GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding /*= 0*/,LPCTSTR strFaceName/*=""*/)
{
    return GetFont(DUIF_MAKEKEY(bBold, bUnderline, bItalic, chAdding),strFaceName);
}

void DuiFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize)
{
    _tcscpy_s(m_szDefFontFace,_countof(m_szDefFontFace),lpszFaceName);
    m_lFontSize = -abs(lSize);

	DUIASSERT(GetCount()==1);//初始化前才可以调用该接口

    HFONT hftOld = GetKeyObject(FontKey(DUIF_DEFAULTFONT));

    SetKeyObject(FontKey(DUIF_DEFAULTFONT),_CreateDefaultGUIFont());

    ::DeleteObject(hftOld);
}

HFONT DuiFontPool::_CreateDefaultGUIFont()
{
    ::GetObjectA(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDefault);

    m_lfDefault.lfHeight = _GetFontAbsHeight(m_lFontSize);
    _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),  m_szDefFontFace);

    m_lfDefault.lfQuality = ANTIALIASED_QUALITY;

    return ::CreateFontIndirect(&m_lfDefault);
}

HFONT DuiFontPool::_CreateNewFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding,CDuiStringT strFaceName/*=""*/)
{
    LOGFONT lfNew;

    memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));
    if(!strFaceName.IsEmpty())
        _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),strFaceName);
    lfNew.lfWeight      = (bBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != bUnderline);
    lfNew.lfItalic      = (FALSE != bItalic);

    if(chAdding & 0x80) chAdding=-1*(chAdding&~0x80);
    lfNew.lfHeight = _GetFontAbsHeight(lfNew.lfHeight - chAdding);

	lfNew.lfQuality = CLEARTYPE_NATURAL_QUALITY;
    HFONT hFont= ::CreateFontIndirect(&lfNew);
    return hFont;
}

LONG DuiFontPool::_GetFontAbsHeight(LONG lSize)
{
    return lSize;
}

}//namespace SOUI