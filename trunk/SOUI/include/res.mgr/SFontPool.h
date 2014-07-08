//////////////////////////////////////////////////////////////////////////
//  Class Name: SFontPool
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "core/ssingletonmap.h"
#include "interface/render-i.h"
#include "unknown/obj-ref-impl.hpp"


#define FF_BOLD        0x0004U
#define FF_UNDERLINE   0x0002U
#define FF_ITALIC      0x0001U

#define FF_MAKEKEY(bold, underline, italic, adding) \
    (WORD)((adding << 8) \
    | (bold ? FF_BOLD : 0) \
    | (underline ? FF_UNDERLINE : 0) \
    | (italic ? FF_ITALIC : 0))

#define FF_ISBOLD(key)         ((key & FF_BOLD) == FF_BOLD)
#define FF_ISUNDERLINE(key)    ((key & FF_UNDERLINE) == FF_UNDERLINE)
#define FF_ISITALIC(key)       ((key & FF_ITALIC) == FF_ITALIC)
#define FF_GETADDING(key)      (key >> 8)

#define FF_DEFAULTFONT         (FF_MAKEKEY(FALSE, FALSE, FALSE, 0))
#define FF_BOLDFONT            (FF_MAKEKEY(TRUE, FALSE, FALSE, 0))


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

class SOUI_EXP SFontPool :public SSingletonMap<SFontPool,IFontPtr,FontKey>
{
public:
    SFontPool(IRenderFactory *pRendFactory);

    IFontPtr GetFont(WORD uKey,LPCTSTR strFaceName=_T(""));

    IFontPtr GetFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding = 0,LPCTSTR strFaceName=_T(""));
    void SetDefaultFont(LPCTSTR lpszFaceName, LONG lSize);
protected:
    static void OnKeyRemoved(const IFontPtr & obj)
    {
        obj->Release();
    }

    IFontPtr _CreateDefaultGUIFont();

    IFontPtr _CreateNewFont(BOOL bBold, BOOL bUnderline, BOOL bItalic, char chAdding,SStringT strFaceName=_T(""));

    LONG _GetFontAbsHeight(LONG lSize);

    LOGFONT m_lfDefault;
    TCHAR m_szDefFontFace[LF_FACESIZE];
    LONG m_lFontSize;

    CAutoRefPtr<IRenderFactory> m_RenderFactory;
};

}//namespace SOUI

