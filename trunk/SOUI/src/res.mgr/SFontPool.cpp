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
    :m_RenderFactory(pRendFactory)
{
    ::GetObjectA(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &m_lfDefault);
    m_lfDefault.lfHeight = -12;
    m_lfDefault.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy(m_lfDefault.lfFaceName,_T("宋体"));

    m_pFunOnKeyRemoved=OnKeyRemoved;
    SetKeyObject(FontKey(0,m_lfDefault.lfFaceName),_CreateFont(m_lfDefault));
}

IFontPtr SFontPool::GetFont(FONTSTYLE style,LPCTSTR pszFaceName)
{
    SStringT strFaceName(pszFaceName);
    if(strFaceName == m_lfDefault.lfFaceName) strFaceName = _T("");
    
    IFontPtr hftRet=0;
    FontKey key(style.dwStyle,strFaceName);
    if(HasKey(key))
    {
        hftRet=GetKeyObject(key);
    }
    else
    {
        if(strFaceName.IsEmpty()) strFaceName = m_lfDefault.lfFaceName;
        hftRet = _CreateFont(style,strFaceName);
        AddKeyObject(key,hftRet);
    }
    return hftRet;
}

IFontPtr SFontPool::GetFont( const SStringW & strFont )
{
    FONTSTYLE fntStyle(0);
    
    SStringT strFace;                                         
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
        fntStyle.fBold=attr.Mid(nPosBegin+5,1)!=_T("0");                   
    }                                                         
    nPosBegin=attr.Find(_T("underline:"));                    
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fUnderline=attr.Mid(nPosBegin+10,1)!=_T("0");             
    }                                                         
    nPosBegin=attr.Find(_T("italic:"));                       
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fItalic=attr.Mid(nPosBegin+7,1)!=_T("0");                 
    }                                                         
    nPosBegin=attr.Find(_T("strike:"));                       
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fStrike=attr.Mid(nPosBegin+7,1)!=_T("0");                 
    }                                                         
    nPosBegin=attr.Find(_T("adding:"));                       
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.cSize=(short)_ttoi((LPCTSTR)attr+nPosBegin+7);           
    }else
    {
        nPosBegin=attr.Find(_T("size:"));
        if(nPosBegin != -1)
        {
            fntStyle.cSize=(short)_ttoi((LPCTSTR)attr+nPosBegin+5);
            fntStyle.fAbsSize=1;       
        }                       
    }
    return GetFont(fntStyle, strFace);
}

void SFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize)
{
    SASSERT(GetCount()==1);//初始化前才可以调用该接口
    RemoveKeyObject(FontKey(0));

    _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),lpszFaceName);
    m_lfDefault.lfHeight = -abs(lSize);

    SetKeyObject(FontKey(0),_CreateFont(m_lfDefault));
}

IFontPtr SFontPool::_CreateFont(const LOGFONT &lf)
{
    
    SASSERT(m_RenderFactory);

    IFontPtr pFont=NULL;
    m_RenderFactory->CreateFont(&pFont,lf);

    return pFont;
}

IFontPtr SFontPool::_CreateFont(FONTSTYLE style,const SStringT & strFaceName)
{
    LOGFONT lfNew;
        
    memcpy(&lfNew, &m_lfDefault, sizeof(LOGFONT));
    lfNew.lfWeight      = (style.fBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != style.fUnderline);
    lfNew.lfItalic      = (FALSE != style.fItalic);
    lfNew.lfStrikeOut   = (FALSE != style.fStrike);
    if(style.fAbsSize)
        lfNew.lfHeight = -abs((short)style.cSize);
    else
        lfNew.lfHeight -= (short)style.cSize;  //cSize为正代表字体变大，否则变小
        
    lfNew.lfQuality = CLEARTYPE_NATURAL_QUALITY;
    _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),  strFaceName);

    return _CreateFont(lfNew);
}


}//namespace SOUI