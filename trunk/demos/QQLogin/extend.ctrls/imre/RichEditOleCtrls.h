#pragma once
#include "RichEditOleBase.h"
#include <vector>
#include "..\SImagePlayer.h"

class RichEditImageOle : public RichEditOleBase
{    
    DECLARE_REOBJ(RichEditImageOle, L"img")

public:
    RichEditImageOle();
    BOOL    SetImage(LPCWSTR lpszImagePath);
    BOOL    InitOleWindow(IRichEditObjHost* pHost);
    HRESULT STDMETHODCALLTYPE Close(DWORD dwSaveOption);
    HRESULT STDMETHODCALLTYPE Draw(
        DWORD dwDrawAspect, LONG lindex,  
        void *pvAspect, 
        DVTARGETDEVICE *ptd, 
        HDC hdcTargetDev,
        HDC hdcDraw, 
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL ( STDMETHODCALLTYPE *pfnContinue )(ULONG_PTR dwContinue), 
        ULONG_PTR dwContinue);

protected:
    double GetZoomRatio(SIZE sizeImage, SIZE sizeMax);

    SOUI_ATTRS_BEGIN()
        ATTR_SIZE(L"max-size", m_sizeMax, TRUE)
        ATTR_STRINGW(L"src", m_strImagePath, TRUE)
    SOUI_ATTRS_END()

private:
    SIZE            m_sizeMax;
    SStringW        m_strImagePath;
    SImagePlayer *  m_pPlayer;
    BOOL            m_bClosed;
};

//////////////////////////////////////////////////////////////////////////
// 
class RichEditFileOle : public RichEditOleBase
{    
    DECLARE_REOBJ(RichEditFileOle, L"file")

public:
    RichEditFileOle();

protected:
    LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

//////////////////////////////////////////////////////////////////////////
// 
class RichEditFetchMoreOle : public RichEditOleBase
{    
    DECLARE_REOBJ(RichEditFetchMoreOle, L"fetchmore")

public:
    RichEditFetchMoreOle();
    BOOL    InitOleWindow(IRichEditObjHost* pHost);

protected:
    bool    OnFetchMore(EventArgs *pEvt);
    LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void    UpdatePosition();
};

//////////////////////////////////////////////////////////////////////////
// 
class RichEditSplitLineOle : public RichEditOleBase
{    
    DECLARE_REOBJ(RichEditSplitLineOle, L"split")

public:
    RichEditSplitLineOle();

protected:
    LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void    UpdatePosition();
};

//////////////////////////////////////////////////////////////////////////
// 
class RichEditReminderOle : public RichEditOleBase
{    
    DECLARE_REOBJ(RichEditReminderOle, L"remainder")

public:
    RichEditReminderOle();
    BOOL    InitFromXml(pugi::xml_node xmlNode);
    BOOL    InitOleWindow(IRichEditObjHost* pHost);

protected:
    void    CalculateNatureSize();
    void    InitAttributes();

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"color-border", m_crBorder, FALSE)
        ATTR_COLOR(L"color-bkgnd", m_crBk, FALSE)
        ATTR_COLOR(L"color-text", m_crText, FALSE)
        ATTR_INT(L"border-width", m_nBorderWidth, FALSE)
        ATTR_STRINGW(L"font", m_strFont, FALSE)
        ATTR_SIZE(L"max-size", m_sizeMax, FALSE)
    SOUI_ATTRS_END()

private:
    COLORREF    m_crBorder;
    COLORREF    m_crBk;
    COLORREF    m_crText;
    int         m_nBorderWidth;
    SStringW    m_strText;
    SStringW    m_strFont;
    SIZE        m_sizeMax;
};

//////////////////////////////////////////////////////////////////////////
//
class RichEditNewsOle : public RichEditOleBase
{    
    DECLARE_REOBJ(RichEditNewsOle, L"news")

public:
    struct NewsItem
    {
        SStringW strTitle;
        SStringW strDesc;
        SStringW strURL;
        SStringW strImagePath;
    };
    RichEditNewsOle();
    ~RichEditNewsOle();

    void        AddNews(LPCWSTR pszTitle, LPCWSTR pszDesc, LPCWSTR pszURL, LPCWSTR pszImage);
    void        ReLayout();

protected:
    void        LayoutForSingleNews();
    void        LayoutForMultiNews();
    bool        OnClickNotifyWnd(EventArgs *pEvt);
    LRESULT     ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void        UpdatePosition();

private:
    typedef SArray<NewsItem> ArrayNewsItem;
    ArrayNewsItem m_arrNews;
};
