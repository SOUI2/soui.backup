//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
// Description: Font Pool
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "souistd.h"
#include "res.mgr/sfontpool.h"
#include "helper/SplitString.h"
#include "layout/SLayoutSize.h"

namespace SOUI
{

template<> SFontPool* SSingleton<SFontPool>::ms_Singleton    = 0;

SFontPool::SFontPool(IRenderFactory *pRendFactory)
    :m_RenderFactory(pRendFactory)
{
    m_pFunOnKeyRemoved=OnKeyRemoved;
}


IFontPtr SFontPool::GetFont(const SStringW & strName,FONTSTYLE style, const SStringW & fontFaceName,pugi::xml_node xmlExProp)
{
	IFontPtr hftRet=0;

	SStringT strFace = S_CW2T(fontFaceName);
	if(strFace.IsEmpty()) strFace = GetDefFontInfo().strFaceName;
	
	pugi::xml_writer_buff writer;
	xmlExProp.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringT strXmlProp= S_CW2T(SStringW(writer.buffer(),writer.size()));

	FontInfo info = {S_CW2T(strName),style.dwStyle,strFace,strXmlProp};

	if(HasKey(info))
	{
		hftRet=GetKeyObject(info);
	}
	else
	{
		hftRet = _CreateFont(info.dwStyle,info.strFaceName,xmlExProp);
		AddKeyObject(info,hftRet);
	}
	return hftRet;

}

static const WCHAR  KFontPropSeprator=   (L',');   //字体属性之间的分隔符，不再支持其它符号。
static const WCHAR  KPropSeprator    =   (L':');   //一个属性name:value对之间的分隔符
static const WCHAR  KAttrFalse[]     =   (L"0");
static const WCHAR  KFontFace[]      =   (L"face");
static const WCHAR  KFontBold[]      =   (L"bold");
static const WCHAR  KFontUnderline[] =   (L"underline");
static const WCHAR  KFontItalic[]    =   (L"italic");
static const WCHAR  KFontStrike[]    =   (L"strike");
static const WCHAR  KFontAdding[]    =   (L"adding");
static const WCHAR  KFontSize[]      =   (L"size");
static const WCHAR  KFontCharset[]   =   (L"charset");
static const WCHAR  KFontName[]      =   (L"name");


#define LEN_FACE    (ARRAYSIZE(KFontFace)-1)
#define LEN_BOLD    (ARRAYSIZE(KFontBold)-1)
#define LEN_UNDERLINE    (ARRAYSIZE(KFontUnderline)-1)
#define LEN_ITALIC  (ARRAYSIZE(KFontItalic)-1)
#define LEN_STRIKE  (ARRAYSIZE(KFontStrike)-1)
#define LEN_ADDING  (ARRAYSIZE(KFontAdding)-1)
#define LEN_SIZE    (ARRAYSIZE(KFontSize)-1)
#define LEN_CHARSET (ARRAYSIZE(KFontCharset)-1)
#define LEN_NAME (ARRAYSIZE(KFontName)-1)

IFontPtr SFontPool::GetFont( const SStringW & strFont ,int scale)
{
    FONTSTYLE fntStyle(GetDefFontInfo().dwStyle);
	fntStyle.attr.cSize = 0;

    SStringW strFace;//不需要默认字体, 在后面GetFont里会检查.
	SStringW strName;
    SStringW attr=strFont; 
    attr.MakeLower();                                        
    SStringWList fontProp;
    SplitString(attr,KFontPropSeprator,fontProp);

	short cAdding = 0;
	short cSize = 0;

	pugi::xml_document docExProp;
	pugi::xml_node nodePropEx = docExProp.append_child(L"propex");
    for(int i=(int)fontProp.GetCount()-1;i>=0;i--)
    {
        SStringWList strPair;
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
			SLayoutSize sz;
			sz.parseString(strPair[1]);
            cAdding=(short)sz.toPixelSize(scale);;
        }else if(strPair[0] == KFontSize)
        {
			SLayoutSize sz;
			sz.parseString(strPair[1]);
            cSize=(short)sz.toPixelSize(scale);
        }else if(strPair[0] == KFontItalic)
        {
            fntStyle.attr.fItalic = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontBold)
        {
            fntStyle.attr.fBold = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontStrike)
        {
            fntStyle.attr.fStrike = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontUnderline)
        {
            fntStyle.attr.fUnderline = strPair[1] != KAttrFalse;
        }else if(strPair[0] == KFontCharset)
        {
            fntStyle.attr.byCharset = (BYTE)_wtoi(strPair[1]);
        }else if(strPair[0] == KFontName)
		{
			strName = strPair[1];
		}else
		{
			nodePropEx.append_attribute(strPair[0]).set_value(strPair[1]);
		}
    }

	if(cSize!=0)
	{//size property is higher than adding
		fntStyle.attr.cSize = cSize;
	}
	else
	{
		FONTSTYLE fontStyle(GetDefFontInfo().dwStyle);
		fntStyle.attr.cSize = fontStyle.attr.cSize * scale/100 + cAdding;  //cAdding为正代表字体变大，否则变小
	}

    return GetFont(strName,fntStyle, strFace,nodePropEx);
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
        
	lfNew.lfCharSet     = style.attr.byCharset;
    lfNew.lfWeight      = (style.attr.fBold ? FW_BOLD : FW_NORMAL);
    lfNew.lfUnderline   = (FALSE != style.attr.fUnderline);
    lfNew.lfItalic      = (FALSE != style.attr.fItalic);
    lfNew.lfStrikeOut   = (FALSE != style.attr.fStrike);
	lfNew.lfHeight = -abs((short)style.attr.cSize);
        
    lfNew.lfQuality = CLEARTYPE_QUALITY;
    
    
    _tcscpy_s(lfNew.lfFaceName,_countof(lfNew.lfFaceName),  strFaceName);
    
    IFontPtr ret = _CreateFont(lfNew);
	if(ret) ret->InitFromXml(xmlExProp);
	return ret;
}

const FontInfo & SFontPool::GetDefFontInfo() const
{
	return SUiDef::getSingleton().GetUiDef()->GetDefFontInfo();
}

void SFontPool::UpdateFontsByTranslator(ITranslator * pTrans)
{
	SPOSITION pos = m_mapNamedObj->GetStartPosition();
	while(pos)
	{
		FontInfo info;
		IFontPtr fontPtr;
		m_mapNamedObj->GetNextAssoc(pos,info,fontPtr);
		LOGFONT lf;
		memcpy(&lf,fontPtr->LogFont(),sizeof(lf));
		if(pTrans->updateLogfont(info.strName,&lf))
		{
			fontPtr->UpdateFont(&lf);
		}
	}
}


}//namespace SOUI