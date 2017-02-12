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
                     FF_ISBOLD(uKey), FF_ISUNDERLINE(uKey), FF_ISITALIC(uKey),FF_ISSTRIKE(uKey), FF_GETADDING(uKey),strFaceName
                 );

        AddKeyObject(key,hftRet);
    }
    return hftRet;
}

IFontPtr SFontPool::GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic,BOOL bStrike, char chAdding /*= 0*/,LPCTSTR strFaceName/*=""*/)
{
    return GetFont(FF_MAKEKEY(bBold, bUnderline, bItalic, bStrike, chAdding),strFaceName);
}

IFontPtr SFontPool::GetFont( const SStringW & strFont )
{
    BOOL bBold=0,bItalic=0,bUnderline=0,bStrike=0;                
    SStringT strFace;                                         
    char  chAdding=0;                                          
    SStringT attr=S_CW2T(strFont);                           
    attr.MakeLower();                                         
    int nPosBegin=attr.Find(_T("face:"));                     
    if(nPosBegin!=-1)                                         
    {                                                         
        nPosBegin+=5;                                             
        int nPosEnd=attr.Find(_T(";"),nPosBegin);
        if(nPosEnd==-1) nPosEnd=attr.Find(_T(","),nPosBegin);
        if(nPosEnd==-1) nPosEnd=attr.GetLength();
        strFace=attr.Mid(nPosBegin,nPosEnd-nPosBegin);
    }                                                         
    nPosBegin=attr.Find(_T("bold:"));                         
    if(nPosBegin!=-1)                                         
    {                                                         
        bBold=attr.Mid(nPosBegin+5,1)!=_T("0");                   
    }                                                         
    nPosBegin=attr.Find(_T("underline:"));                    
    if(nPosBegin!=-1)                                         
    {                                                         
        bUnderline=attr.Mid(nPosBegin+10,1)!=_T("0");             
    }                                                         
    nPosBegin=attr.Find(_T("italic:"));                       
    if(nPosBegin!=-1)                                         
    {                                                         
        bItalic=attr.Mid(nPosBegin+7,1)!=_T("0");                 
    }                                                         
    nPosBegin=attr.Find(_T("strike:"));                       
    if(nPosBegin!=-1)                                         
    {                                                         
        bStrike=attr.Mid(nPosBegin+7,1)!=_T("0");                 
    }                                                         
    nPosBegin=attr.Find(_T("adding:"));                       
    if(nPosBegin!=-1)                                         
    {                                                         
        chAdding=(char)_ttoi((LPCTSTR)attr+nPosBegin+7);           
    }
    return GetFont(bBold, bUnderline, bItalic,bStrike, chAdding , strFace);
}

void SFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize)
{
    _tcscpy_s(m_szDefFontFace,_countof(m_szDefFontFace),lpszFaceName);
    m_lFontSize = -abs(lSize);

    SASSERT(GetCount()==1);//初始化前才可以调用该接口

    RemoveKeyObject(FontKey(FF_DEFAULTFONT));
    
    SetKeyObject(FontKey(FF_DEFAULTFONT),_CreateDefaultFont());
}

IFontPtr SFontPool::_CreateDefaultFont()
{
    ::GetObjectA(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDefault);

    m_lfDefault.lfHeight = -_GetFontAbsHeight(m_lFontSize);
    _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),  m_szDefFontFace);

    m_lfDefault.lfQuality = ANTIALIASED_QUALITY;
    
    SASSERT(m_RenderFactory);

    IFontPtr pFont=NULL;
    m_RenderFactory->CreateFont(&pFont,m_lfDefault);

    return pFont;
}

IFontPtr SFontPool::_CreateFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, BOOL bStrike,char chAdding,SStringT strFaceName)
{
    LOGFONT lfNew;

    memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));
    lfNew.lfWeight      = (bBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != bUnderline);
    lfNew.lfItalic      = (FALSE != bItalic);
    lfNew.lfStrikeOut   = (FALSE != bStrike);
    lfNew.lfHeight = -_GetFontAbsHeight(lfNew.lfHeight - chAdding);//lfNew.lfHeight应该为负值
    lfNew.lfQuality = CLEARTYPE_NATURAL_QUALITY;
    _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),  strFaceName);

    IFontPtr pFont=NULL;
    SASSERT(m_RenderFactory);
    m_RenderFactory->CreateFont(&pFont,lfNew);

    return pFont;
}

LONG SFontPool::_GetFontAbsHeight(LONG lSize)
{
    return lSize<0? (-lSize):lSize;
}

}//namespace SOUI