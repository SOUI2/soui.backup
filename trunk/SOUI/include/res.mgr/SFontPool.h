//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "core/ssingletonmap.h"
#include "interface/render-i.h"
#include "unknown/obj-ref-impl.hpp"

#define FF_STRIKE      0x0008U
#define FF_BOLD        0x0004U
#define FF_UNDERLINE   0x0002U
#define FF_ITALIC      0x0001U

#define FF_MAKEKEY(bold, underline, italic,strike, adding) \
    (WORD)((adding << 8) \
    | (bold ? FF_BOLD : 0) \
    | (underline ? FF_UNDERLINE : 0) \
    | (italic ? FF_ITALIC : 0) \
    | (strike ? FF_STRIKE : 0))

#define FF_ISSTRIKE(key)       ((key & FF_STRIKE) == FF_STRIKE)
#define FF_ISBOLD(key)         ((key & FF_BOLD) == FF_BOLD)
#define FF_ISUNDERLINE(key)    ((key & FF_UNDERLINE) == FF_UNDERLINE)
#define FF_ISITALIC(key)       ((key & FF_ITALIC) == FF_ITALIC)
#define FF_GETADDING(key)      (key >> 8)

#define FF_DEFAULTFONT         (FF_MAKEKEY(FALSE, FALSE, FALSE,FALSE, 0))
#define FF_BOLDFONT            (FF_MAKEKEY(TRUE, FALSE, FALSE,FALSE, 0))

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
    TCHAR    strFaceName[LF_FACESIZE+1];
    DWORD     dwStyle;
};


/**
* @class     CElementTraits< FontKey >
* @brief      FontKey的Hash及比较模板
* 
* Describe    用于实现一个font map
*/
template<>
class _COLL_NS::CElementTraits< FontKey > :
    public _COLL_NS::CElementTraitsBase<FontKey >
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
         * @brief    获得与指定的font key对应的IFontPtr
         * @param    WORD uKey --  font 标志位
         * @param    LPCTSTR strFaceName --  font name
         * @return   IFontPtr -- font对象
         * Describe  
         */    
        IFontPtr GetFont(WORD uKey,LPCTSTR strFaceName=_T(""));

        /**
         * GetFont
         * @brief    获得与指定的font key对应的IFontPtr
         * @param    BOOL bBold --  粗体
         * @param    BOOL bUnderline --  下划线
         * @param    BOOL bItalic --  斜体
         * @param    BOOL bStrike --  删除线
         * @param    char chAdding --  字体大小变化量
         * @param    LPCTSTR strFaceName --  字体名
         * @return   IFontPtr -- font对象
         * Describe  
         */    
        IFontPtr GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, BOOL bStrike,char chAdding = 0,LPCTSTR strFaceName=_T(""));
        
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

        IFontPtr _CreateDefaultFont();

        IFontPtr _CreateFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, BOOL bStrike,char chAdding,SStringT strFaceName=_T(""));

        LONG _GetFontAbsHeight(LONG lSize);

        LOGFONT m_lfDefault;
        TCHAR m_szDefFontFace[LF_FACESIZE];
        LONG m_lFontSize;

        CAutoRefPtr<IRenderFactory> m_RenderFactory;
    };

}//namespace SOUI

