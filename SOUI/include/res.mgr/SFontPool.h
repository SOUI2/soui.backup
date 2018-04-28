/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SFontPool.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    SOUI字体管理模块
*/

#pragma once

#include "core/ssingletonmap.h"
#include "interface/render-i.h"
#include "interface/STranslator-i.h"
#include "unknown/obj-ref-impl.hpp"

/**
* @union      FONTSTYLE
* @brief      FONT的风格
* 
* Describe    
*/
union FONTSTYLE{
    DWORD     dwStyle;  //DWORD版本的风格
    struct 
    {
        DWORD cSize:16; //字体大小，为short有符号类型
		DWORD byCharset:8;  //字符集
        DWORD fItalic:1;//斜体标志位
        DWORD fUnderline:1;//下划线标志位
        DWORD fBold:1;//粗体标志位
        DWORD fStrike:1;//删除线标志位
        DWORD fReserved:4;  //保留位
    }attr;
    
    FONTSTYLE(DWORD _dwStyle):dwStyle(_dwStyle){}
}; 

#define FF_DEFAULTFONT L""

/**
* @class     FontKey
* @brief      一个FONT的KEY
* 
* Describe    用于实现一个font map
*/
namespace SOUI
{
	struct FontInfo
	{
		SStringW strName;
		DWORD    dwStyle;
		SStringT strFaceName;
		SStringT strPropEx;
	};


    /**
    * @class     CElementTraits< FontKey >
    * @brief      FontKey的Hash及比较模板
    * 
    * Describe    用于实现一个font map
    */
    template<>
    class CElementTraits< FontInfo > :
        public CElementTraitsBase<FontInfo >
    {
    public:
        static ULONG Hash( INARGTYPE fontKey )
        {
            ULONG uRet=SOUI::CElementTraits<SStringT>::Hash(fontKey.strFaceName);
			uRet = (uRet<<5) + SOUI::CElementTraits<SStringW>::Hash(fontKey.strName);
            uRet = (uRet<<5) + SOUI::CElementTraits<SStringT>::Hash(fontKey.strPropEx);
            uRet = (uRet<<5) +(UINT)fontKey.dwStyle+1;
            return uRet;
        }

        static bool CompareElements( INARGTYPE element1, INARGTYPE element2 )
        {
            return element1.strFaceName==element2.strFaceName
                && element1.strPropEx==element2.strPropEx
                && element1.dwStyle==element2.dwStyle;
        }

        static int CompareElementsOrdered( INARGTYPE element1, INARGTYPE element2 )
        {
            int nRet= element1.strFaceName.Compare(element2.strFaceName);
            if(nRet == 0)
                nRet = element1.strPropEx.Compare(element2.strPropEx);
            if(nRet == 0)
                nRet = element1.dwStyle-element2.dwStyle;
            return nRet;
        }
    };

    typedef IFont * IFontPtr;

    /**
    * @class      SFontPool
    * @brief      font pool
    * 
    * Describe
    */
    class SOUI_EXP SFontPool :public SSingletonMap<SFontPool,IFontPtr,FontInfo>
    {
    public:
        SFontPool(IRenderFactory *pRendFactory);

        
        /**
         * GetFont
         * @brief    获得与指定的strFont对应的IFontPtr
         * @param    const SStringW & strFont --  font描述字符串
         * @return   IFontPtr -- font对象
         *
         * Describe  描述字符串格式如：face:宋体,bold:0,italic:1,underline:1,strike:1,adding:10
         */
        IFontPtr GetFont(const SStringW & strFont,int scale);

		/**
         * GetFont
         * @brief    获得与指定的font key对应的IFontPtr
         * @param    FONTSTYLE style --  字体风格
         * @param    LPCTSTR strFaceName --  字体名
         * @return   IFontPtr -- font对象
         * Describe  
         */    
		IFontPtr GetFont(const SStringW & strName,FONTSTYLE style,const SStringW& strFaceName = SStringW(),pugi::xml_node xmlExProp = pugi::xml_node());


		void UpdateFontsByTranslator(ITranslator * pTrans);

    protected:

		const FontInfo & GetDefFontInfo() const;

        static void OnKeyRemoved(const IFontPtr & obj)
        {
            obj->Release();
        }

		IFontPtr _CreateFont(const LOGFONT &lf);
        
        IFontPtr _CreateFont(FONTSTYLE style,const SStringT & strFaceName,pugi::xml_node xmlExProp);

        CAutoRefPtr<IRenderFactory> m_RenderFactory;
    };

}//namespace SOUI

