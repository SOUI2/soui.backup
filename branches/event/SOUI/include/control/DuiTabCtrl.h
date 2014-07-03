//////////////////////////////////////////////////////////////////////////
//   File Name: duitabctrl.h
// Description: Tab Control
//     Creator: Huang Jianxiong
//     Version: 2011.12.2 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duiwnd.h"

namespace SOUI
{

class SOUI_EXP STabPage : public SWindow
{
    SOUI_CLASS_NAME(STabPage, L"page")

public:
    STabPage()
    {
        m_bVisible = FALSE;
        m_dwState = DuiWndState_Invisible;
        m_dlgpos.uPositionType = SizeX_FitParent|SizeY_FitParent;
    }

    virtual ~STabPage()
    {
    }

    LPCTSTR GetTitle()
    {
        return m_strTitle;
    }

    void SetTitle(LPCTSTR lpszTitle)
    {
        m_strTitle = lpszTitle;
    }

    SOUI_ATTRS_BEGIN()
        ATTR_STRINGT(L"title", m_strTitle, FALSE)
    SOUI_ATTRS_END()
protected:

    SStringT m_strTitle;
};

typedef enum tagSLIDEDIR
{
    SD_LEFTRIGHT=0,
    SD_RIGHTLEFT,
    SD_TOPBOTTOM,
    SD_BOTTOMTOP,
} SLIDEDIR;

class SOUI_EXP STabCtrl : public SWindow
{
    friend class STabSlider;

    SOUI_CLASS_NAME(STabCtrl, L"tabctrl")

protected:
    int m_nHoverTabItem;
    int m_nCurrentPage;
    int m_nTabSpacing;
    int m_nTabWidth;
    int m_nTabHeight;
    int m_nTabPos;
    int m_nFramePos;
    ISkinObj *m_pSkinTab;
    ISkinObj *m_pSkinIcon;
    ISkinObj *m_pSkinSplitter;
    ISkinObj *m_pSkinFrame;
    CPoint m_ptIcon;
    CPoint m_ptText;
    int m_nTabAlign;

    SArray<STabPage*> m_lstPages;

    enum
    {
        AlignTop,
        AlignLeft,
    };

    int    m_nAnimateSteps;
public:

    STabCtrl();
    virtual ~STabCtrl() {}

    int GetCurSel()
    {
        return m_nCurrentPage;
    }

    BOOL SetCurSel(int nIndex);

    BOOL SetCurSel(LPCTSTR pszTitle);

    BOOL SetItemTitle(int nIndex, LPCTSTR lpszTitle);

    BOOL CreateChildren(pugi::xml_node xmlNode);

    BOOL InsertItem(LPCWSTR lpContent,int iInsert=-1);

    int InsertItem(pugi::xml_node xmlNode,int iInsert=-1,BOOL bLoading=FALSE);

    int GetItemCount()
    {
        return m_lstPages.GetCount();
    }

    STabPage* GetItem(int nIndex);


    BOOL RemoveItem(int nIndex, int nSelPage=0);

    void RemoveAllItems(void);
protected:
    virtual CRect GetChildrenLayoutRect();

    virtual BOOL GetItemRect(int nIndex, CRect &rcItem);

    virtual void DrawItem(IRenderTarget *pRT,const CRect &rcItem,int iItem,DWORD dwState);

    virtual UINT OnGetDlgCode()
    {
        return DUIC_WANTARROWS;
    }

    void OnPaint(IRenderTarget *pRT);

    void OnLButtonDown(UINT nFlags, CPoint point);

    void OnMouseMove(UINT nFlags, CPoint point);

    void OnMouseLeave()
    {
        OnMouseMove(0,CPoint(-1,-1));
    }

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnDestroy();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"cursel", m_nCurrentPage, FALSE)
        ATTR_INT(L"tabwidth", m_nTabWidth, FALSE)
        ATTR_INT(L"tabheight", m_nTabHeight, FALSE)
        ATTR_INT(L"tabpos", m_nTabPos, FALSE)
        ATTR_INT(L"framepos", m_nFramePos, FALSE)
        ATTR_INT(L"tabspacing", m_nTabSpacing, FALSE)
        ATTR_SKIN(L"tabskin", m_pSkinTab, FALSE)
        ATTR_SKIN(L"iconskin", m_pSkinIcon, FALSE)
        ATTR_SKIN(L"splitterskin", m_pSkinSplitter, FALSE)
        ATTR_SKIN(L"frameskin", m_pSkinFrame, FALSE)
        ATTR_INT(L"icon-x", m_ptIcon.x, FALSE)
        ATTR_INT(L"icon-y", m_ptIcon.y, FALSE)
        ATTR_INT(L"text-x", m_ptText.x, FALSE)
        ATTR_INT(L"text-y", m_ptText.y, FALSE)
        ATTR_ENUM_BEGIN(L"tabalign", int, TRUE)
            ATTR_ENUM_VALUE(L"top", AlignTop)
            ATTR_ENUM_VALUE(L"left", AlignLeft)
        ATTR_ENUM_END(m_nTabAlign)
        ATTR_INT(L"animatesteps",m_nAnimateSteps,FALSE)
    SOUI_ATTRS_END()
};

}//namespace SOUI