//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "res.mgr/sfontpool.h"
#include "helper/SplitString.h"

namespace SOUI
{

template<> SFontPool* SSingleton<SFontPool>::ms_Singleton    = 0;

static LPCTSTR KDefFaceName = _T("宋体");

SFontPool::SFontPool(IRenderFactory *pRendFactory)
    :m_RenderFactory(pRendFactory)
{
    m_pFunOnKeyRemoved=OnKeyRemoved;

	FontInfo fontInfo;
	FONTSTYLE fontSytle(0);
	fontSytle.cSize = 12;
	fontSytle.byCharset = DEFAULT_CHARSET;
	fontInfo.dwStyle = fontSytle.dwStyle;
	fontInfo.strFaceName = KDefFaceName;

    SetKeyObject(fontInfo,GetFont(fontInfo));
}

IFontPtr SFontPool::GetFont(const FontInfo &fontInfo)
{
	IFontPtr hftRet=0;

	FontInfo fontInfo2 = fontInfo;
	if(fontInfo2.strFaceName.IsEmpty())
		fontInfo2.strFaceName = m_defFont.strFaceName;
	if(HasKey(fontInfo2))
	{
		hftRet=GetKeyObject(fontInfo2);
	}
	else
	{
		pugi::xml_document xmlDoc;
		xmlDoc.load(fontInfo2.strPropEx);
		hftRet = _CreateFont(fontInfo2.dwStyle,fontInfo2.strFaceName,xmlDoc.first_child());
		AddKeyObject(fontInfo2,hftRet);
	}
	return hftRet;
}

IFontPtr SFontPool::GetFont(FONTSTYLE style, const SStringT & fontFaceName,pugi::xml_node xmlExProp)
{
	SStringW strExProp;
	for(pugi::xml_node::attribute_iterator it = xmlExProp.attributes_begin();it!=xmlExProp.attributes_end();it++)
	{
		strExProp += SStringW().Format(L"%s=%s,",it->name(),it->value());
	}

	FontInfo fontInfo={style.dwStyle,fontFaceName,strExProp};

	return GetFont(fontInfo);
}

static const TCHAR  KFontPropSeprator=  _T(',');   //字体属性之间的分隔符，不再支持其它符号。
static const TCHAR  KPropSeprator    =  _T(':');   //一个属性name:value对之间的分隔符
static const TCHAR  KAttrFalse[]     =   _T("0");
static const TCHAR  KFontFace[]      =   _T("face");
static const TCHAR  KFontBold[]      =   _T("bold");
static const TCHAR  KFontUnderline[] =   _T("underline");
static const TCHAR  KFontItalic[]    =   _T("italic");
static const TCHAR  KFontStrike[]    =   _T("strike");
static const TCHAR  KFontAdding[]    =   _T("adding");
static const TCHAR  KFontSize[]      =   _T("size");
static const TCHAR  KFontCharset[]   =   _T("charset");


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
    SStringTList fontProp;
    SplitString(attr,KFontPropSeprator,fontProp);

	pugi::xml_document docExProp;
	pugi::xml_node nodePropEx = docExProp.append_child(L"propex");
    for(int i=fontProp.GetCount()-1;i>=0;i--)
    {
        SStringTList strPair;
        if(2!=SplitString(fontProp[i],KPropSeprator,strPair))
        {
            fontProp.RemoveAt(i);
            continue;
        }
        if(strPair[0] == KFontFace)
        {
            strFace = strPair[1];
        }else if(strPair[0] == KFontAdding)
        {
            fntStyle.cSize=(short)_ttoi(strPair[1]); 
        }else if(strPair[0] == KFontSize)
        {
            fntStyle.cSize=(short)_ttoi(strPair[1]); 
            fntStyle.fAbsSize = 1;
        }else if(strPair[0] == KFontItalic)
        {
            fntStyle.fItalic = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontBold)
        {
            fntStyle.fBold = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontStrike)
        {
            fntStyle.fStrike = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontUnderline)
        {
            fntStyle.fUnderline = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontCharset)
        {
            fntStyle.byCharset = (BYTE)_ttoi(strPair[1]);
        }else
		{
			nodePropEx.append_attribute(S_CT2W(strPair[0])).set_value(S_CT2W(strPair[1]));
		}
    }
    return GetFont(fntStyle, strFace,nodePropEx);
}

#define HASFONT 2
int CALLBACK DefFontsEnumProc(  CONST LOGFONT *lplf,     // logical-font data
                              CONST TEXTMETRIC *lptm,  // physical-font data
                              DWORD dwType,            // font type
                              LPARAM lpData            // application-defined data
                              )
{
    return HASFONT;
}

void SFontPool::SetDefaultFont(const FontInfo & fontInfo)
{
	m_defFont = fontInfo;

	//确保字体存在
    HDC hdc = GetDC(NULL);
    int hasFont = EnumFonts(hdc,m_defFont.strFaceName,DefFontsEnumProc,0);
    ReleaseDC(NULL,hdc);

    if(hasFont != HASFONT)
		m_defFont.strFaceName = _T("宋体");
}


IFontPtr SFontPool::_CreateFont(const LOGFONT &lf)
{
    
    SASSERT(m_RenderFactory);
    
    
    IFontPtr pFont=NULL;
    m_RenderFactory->CreateFont(&pFont,lf);

    return pFont;
}

IFontPtr SFontPool::_CreateFont(FONTSTYLE style,const SStringT & strFaceName,pugi::xml_node xmlExProp)
{
	LOGFONT lfNew={0};
        
	lfNew.lfCharSet     = style.byCharset;
    lfNew.lfWeight      = (style.fBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != style.fUnderline);
    lfNew.lfItalic      = (FALSE != style.fItalic);
    lfNew.lfStrikeOut   = (FALSE != style.fStrike);
    if(style.fAbsSize)
        lfNew.lfHeight = -abs((short)style.cSize);
    else
	{
		FONTSTYLE fontStyle(m_defFont.dwStyle);
		lfNew.lfHeight = -(fontStyle.cSize + (short)style.cSize);  //cSize为正代表字体变大，否则变小
	}
        
    lfNew.lfQuality = CLEARTYPE_QUALITY;
    
    
    _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),  strFaceName);
    
    IFontPtr ret = _CreateFont(lfNew);
	if(ret) ret->InitFromXml(xmlExProp);
	return ret;
}


}//namespace SOUI