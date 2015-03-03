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
    m_lfDefault.lfHeight  = -12;
    m_lfDefault.lfQuality = CLEARTYPE_QUALITY;
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

const TCHAR  KFontFace[]      =   _T("face:");
const TCHAR  KFontBold[]      =   _T("bold:");
const TCHAR  KFontUnderline[] =   _T("underline:");
const TCHAR  KFontItalic[]    =   _T("italic:");
const TCHAR  KFontStrike[]    =   _T("strike:");
const TCHAR  KFontAdding[]    =   _T("adding:");
const TCHAR  KFontSize[]      =   _T("size:");
const TCHAR  KFontCharset[]   =   _T("charset:");


#define LEN_FACE    (ARRAYSIZE(KFontFace)-1)
#define LEN_BOLD    (ARRAYSIZE(KFontBold)-1)
#define LEN_UNDERLINE    (ARRAYSIZE(KFontUnderline)-1)
#define LEN_ITALIC  (ARRAYSIZE(KFontItalic)-1)
#define LEN_STRIKE  (ARRAYSIZE(KFontStrike)-1)
#define LEN_ADDING  (ARRAYSIZE(KFontAdding)-1)
#define LEN_SIZE    (ARRAYSIZE(KFontSize)-1)
#define LEN_CHARSET (ARRAYSIZE(KFontCharset)-1)

IFontPtr SFontPool::GetFont( const SStringW & strFont )
{
    FONTSTYLE fntStyle(0);
    fntStyle.byCharset = DEFAULT_CHARSET;
    
    SStringT strFace;                                         
    SStringT attr=S_CW2T(strFont);                           
    attr.MakeLower();                                         
    int nPosBegin=attr.Find(KFontFace);                     
    if(nPosBegin!=-1)                                         
    {                                                         
        nPosBegin+=LEN_FACE;                                             
        int nPosEnd=attr.Find(_T(";"),nPosBegin);
        if(nPosEnd==-1) nPosEnd=attr.Find(_T(","),nPosBegin);
        if(nPosEnd==-1) nPosEnd=attr.GetLength();
        strFace=attr.Mid(nPosBegin,nPosEnd-nPosBegin);
    }                                                         
    nPosBegin=attr.Find(KFontBold);                         
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fBold=attr.Mid(nPosBegin+LEN_BOLD,1)!=_T("0");                   
    }                                                         
    nPosBegin=attr.Find(KFontUnderline);                    
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fUnderline=attr.Mid(nPosBegin+LEN_UNDERLINE,1)!=_T("0");             
    }                                                         
    nPosBegin=attr.Find(KFontItalic);                       
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fItalic=attr.Mid(nPosBegin+LEN_ITALIC,1)!=_T("0");                 
    }                                                         
    nPosBegin=attr.Find(KFontStrike);                       
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.fStrike=attr.Mid(nPosBegin+LEN_STRIKE,1)!=_T("0");                 
    }                                                         
    nPosBegin=attr.Find(KFontAdding);                       
    if(nPosBegin!=-1)                                         
    {                                                         
        fntStyle.cSize=(short)_ttoi((LPCTSTR)attr+nPosBegin+LEN_ADDING);           
    }else
    {
        nPosBegin=attr.Find(KFontSize);
        if(nPosBegin != -1)
        {
            fntStyle.cSize=(short)_ttoi((LPCTSTR)attr+nPosBegin+LEN_SIZE);
            fntStyle.fAbsSize=1;       
        }                       
    }
    
    //允许定义字符集
    nPosBegin = attr.Find(KFontCharset);
    if(nPosBegin!=-1)
    {
        fntStyle.byCharset = (BYTE)_ttoi((LPCTSTR)attr+nPosBegin+LEN_CHARSET);
    }
    return GetFont(fntStyle, strFace);
}

void SFontPool::SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize,BYTE byCharset/*=DEFAULT_CHARSET*/)
{
    SASSERT(GetCount()==1);//初始化前才可以调用该接口
    RemoveKeyObject(FontKey(0));

    _tcscpy_s(m_lfDefault.lfFaceName,_countof(m_lfDefault.lfFaceName),lpszFaceName);
    m_lfDefault.lfHeight = -abs(lSize);
    if(byCharset!=DEFAULT_CHARSET) 
        m_lfDefault.lfCharSet = byCharset;
    
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
        
    lfNew.lfQuality = CLEARTYPE_QUALITY;
    
    if(style.byCharset!=DEFAULT_CHARSET) lfNew.lfCharSet = style.byCharset;
    
    _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),  strFaceName);
    
    return _CreateFont(lfNew);
}


}//namespace SOUI