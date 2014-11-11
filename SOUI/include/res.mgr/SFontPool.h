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
        DWORD cSize:16;//字体大小，为short有符号类型
        DWORD fItalic:1;//斜体标志位
        DWORD fUnderline:1;//下划线标志位
        DWORD fBold:1;//粗体标志位
        DWORD fStrike:1;//删除线标志位
        DWORD fAbsSize:1;//字体的cSize是绝对大小还是相对于默认字体的大小,1代表cSize为绝对大小
    };
    
    FONTSTYLE(DWORD _dwStyle):dwStyle(_dwStyle){}
}; 

#define FF_DEFAULTFONT FONTSTYLE(0)

/**
* @class     FontKey
* @brief      一个FONT的KEY
* 
* Describe    用于实现一个font map
*/
class SOUI_EXP FontKey
{
public:
    FontKey(DWORD _dwStyle,LPCTSTR pszFaceName=NULL)
    {
        if(pszFaceName)
        {
            _tcscpy_s(strFaceName,LF_FACESIZE,pszFaceName);
        }
        else
        {
            strFaceName[0]=0;
        }
        dwStyle=_dwStyle;
    }
    
    DWORD    dwStyle;
    TCHAR    strFaceName[LF_FACESIZE+1];
};


/**
* @class     CElementTraits< FontKey >
* @brief      FontKey的Hash及比较模板
* 
* Describe    用于实现一个font map
*/
template<>
class SOUI::CElementTraits< FontKey > :
    public SOUI::CElementTraitsBase<FontKey >
{
public:
    static ULONG Hash( INARGTYPE fontKey )
    {
        ULONG_PTR uRet=0;
        SStringT strType=fontKey.strFaceName;
        strType.MakeLower();
        for(int i=0; i<strType.GetLength(); i++)
        {
            uRet=uRet*68+strType[i];
        }

        return (ULONG)(uRet*10000+(UINT)fontKey.dwStyle+1);
    }

    static bool CompareElements( INARGTYPE element1, INARGTYPE element2 )
    {
        return _tcsicmp(element1.strFaceName,element2.strFaceName)==0 && element1.dwStyle==element2.dwStyle;
    }

    static int CompareElementsOrdered( INARGTYPE element1, INARGTYPE element2 )
    {
        int nRet=_tcsicmp(element1.strFaceName,element2.strFaceName);
        if(nRet<0) return -1;
        if(nRet>0) return 1;
        return element1.dwStyle-element2.dwStyle;
    }
};

namespace SOUI
{
    typedef IFont * IFontPtr;

    /**
    * @class      SFontPool
    * @brief      font pool
    * 
    * Describe
    */
    class SOUI_EXP SFontPool :public SSingletonMap<SFontPool,IFontPtr,FontKey>
    {
    public:
        SFontPool(IRenderFactory *pRendFactory);

        
        /**
         * GetFont
         * @brief    获得与指定的strFont对应的IFontPtr
         * @param    const SStringW & strFont --  font描述字符串
         * @return   IFontPtr -- font对象
         *
         * Describe  描述字符串格式如：face:宋体;bold:0;italic:1;underline:1;strike:1;adding:10
         */
        IFontPtr GetFont(const SStringW & strFont);

        /**
         * GetFont
         * @brief    获得与指定的font key对应的IFontPtr
         * @param    FONTSTYLE style --  字体风格
         * @param    LPCTSTR strFaceName --  字体名
         * @return   IFontPtr -- font对象
         * Describe  
         */    
        IFontPtr GetFont(FONTSTYLE style,LPCTSTR strFaceName=_T(""));
        
        /**
         * SetDefaultFont
         * @brief    设置默认字体
         * @param    LPCTSTR lpszFaceName --  字体名
         * @param    LONG lSize --  字体大小
         * @return   void
         * Describe  
         */    
        void SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize);
    protected:
        static void OnKeyRemoved(const IFontPtr & obj)
        {
            obj->Release();
        }

        IFontPtr _CreateFont(const LOGFONT &lf);
        
        IFontPtr _CreateFont(FONTSTYLE style,const SStringT & strFaceName);

        LOGFONT m_lfDefault;
        CAutoRefPtr<IRenderFactory> m_RenderFactory;
    };

}//namespace SOUI

