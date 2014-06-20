//////////////////////////////////////////////////////////////////////////
//   File Name: duitabctrl.h
// Description: Tab Control
//     Creator: Huang Jianxiong
//     Version: 2011.12.2 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duiwndnotify.h"
#include "duiwnd.h"

namespace SOUI
{

class SOUI_EXP CDuiTab : public SWindow
{
    SOUI_CLASS_NAME(CDuiTab, "tab")

public:
    CDuiTab()
    {
        m_bVisible = FALSE;
        m_dwState = DuiWndState_Invisible;
        m_dlgpos.uPositionType = SizeX_FitParent|SizeY_FitParent;
    }

    virtual ~CDuiTab()
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

protected:

    CDuiStringT m_strTitle;
    SOUI_ATTRS_BEGIN()
    ATTR_STRINGT("title", m_strTitle, FALSE)
    SOUI_ATTRS_END()
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
    friend class CDuiTabSlider;

    SOUI_CLASS_NAME(STabCtrl, "tabctrl")

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

    CDuiArray<CDuiTab*> m_lstPages;

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

    BOOL LoadChildren(pugi::xml_node xmlNode);

    BOOL InsertItem(LPCWSTR lpContent,int iInsert=-1);

    int InsertItem(pugi::xml_node xmlNode,int iInsert=-1,BOOL bLoading=FALSE);

    int GetItemCount()
    {
        return m_lstPages.GetCount();
    }

    CDuiTab* GetItem(int nIndex);


    BOOL RemoveItem(int nIndex, int nSelPage=0);

    void RemoveAllItems(void);
protected:
    virtual CRect GetChildrenLayoutRect();

    virtual BOOL GetItemRect(int nIndex, CRect &rcItem);

    virtual void DrawItem(IRenderTarget *pRT,const CRect &rcItem,int iItem,DWORD dwState);

    virtual UINT OnGetDuiCode()
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

    WND_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
    WND_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
        ATTR_INT("cursel", m_nCurrentPage, FALSE)
        ATTR_INT("tabwidth", m_nTabWidth, FALSE)
        ATTR_INT("tabheight", m_nTabHeight, FALSE)
        ATTR_INT("tabpos", m_nTabPos, FALSE)
        ATTR_INT("framepos", m_nFramePos, FALSE)
        ATTR_INT("tabspacing", m_nTabSpacing, FALSE)
        ATTR_SKIN("tabskin", m_pSkinTab, FALSE)
        ATTR_SKIN("iconskin", m_pSkinIcon, FALSE)
        ATTR_SKIN("splitterskin", m_pSkinSplitter, FALSE)
        ATTR_SKIN("frameskin", m_pSkinFrame, FALSE)
        ATTR_INT("icon-x", m_ptIcon.x, FALSE)
        ATTR_INT("icon-y", m_ptIcon.y, FALSE)
        ATTR_INT("text-x", m_ptText.x, FALSE)
        ATTR_INT("text-y", m_ptText.y, FALSE)
        ATTR_ENUM_BEGIN("tabalign", int, TRUE)
            ATTR_ENUM_VALUE("top", AlignTop)
            ATTR_ENUM_VALUE("left", AlignLeft)
        ATTR_ENUM_END(m_nTabAlign)
        ATTR_INT("animatesteps",m_nAnimateSteps,FALSE)
    SOUI_ATTRS_END()
};

}//namespace SOUI