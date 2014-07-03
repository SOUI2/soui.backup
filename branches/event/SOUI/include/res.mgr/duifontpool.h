//////////////////////////////////////////////////////////////////////////
//  Class Name: DUIFontPool
// Description: Font Pool
//     Creator: ZhangXiaoxuan
//     Version: 2009.4.22 - 1.0 - Change stl::map to CAtlMap
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duisingletonmap.h"
#include "render/render-i.h"
#include "unknown/obj-ref-impl.hpp"


#define DUIF_BOLD        0x0004U
#define DUIF_UNDERLINE   0x0002U
#define DUIF_ITALIC      0x0001U

#define DUIF_MAKEKEY(bold, underline, italic, adding) \
    (WORD)((adding << 8) \
    | (bold ? DUIF_BOLD : 0) \
    | (underline ? DUIF_UNDERLINE : 0) \
    | (italic ? DUIF_ITALIC : 0))

#define DUIF_ISBOLD(key)         ((key & DUIF_BOLD) == DUIF_BOLD)
#define DUIF_ISUNDERLINE(key)    ((key & DUIF_UNDERLINE) == DUIF_UNDERLINE)
#define DUIF_ISITALIC(key)       ((key & DUIF_ITALIC) == DUIF_ITALIC)
#define DUIF_GETADDING(key)      (key >> 8)

#define DUIF_DEFAULTFONT         (DUIF_MAKEKEY(FALSE, FALSE, FALSE, 0))
#define DUIF_BOLDFONT            (DUIF_MAKEKEY(TRUE, FALSE, FALSE, 0))


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

class SOUI_EXP DuiFontPool :public DuiSingletonMap<DuiFontPool,IFontPtr,FontKey>
{
public:
    DuiFontPool(IRenderFactory *pRendFactory);

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

