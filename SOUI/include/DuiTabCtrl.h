//////////////////////////////////////////////////////////////////////////
//   File Name: duitabctrl.h
// Description: Tab Control
//     Creator: Huang Jianxiong
//     Version: 2011.12.2 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "duiwndnotify.h"
#include "DuiCmnCtrl.h"

namespace SOUI
{

class SOUI_EXP CDuiTab : public CDuiWindow
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
    SOUO_ATTRIBUTES_BEGIN()
    DUIWIN_TSTRING_ATTRIBUTE("title", m_strTitle, FALSE)
    SOUI_ATTRIBUTES_END()
};

typedef enum tagSLIDEDIR
{
    SD_LEFTRIGHT=0,
    SD_RIGHTLEFT,
    SD_TOPBOTTOM,
    SD_BOTTOMTOP,
} SLIDEDIR;

class SOUI_EXP CDuiTabCtrl : public CDuiWindow
{
    friend class CDuiTabSlider;

    SOUI_CLASS_NAME(CDuiTabCtrl, "tabctrl")

protected:
    int m_nHoverTabItem;
    int m_nCurrentPage;
    int m_nTabSpacing;
    int m_nTabWidth;
    int m_nTabHeight;
    int m_nTabPos;
    int m_nFramePos;
    CDuiSkinBase *m_pSkinTab;
    CDuiSkinBase *m_pSkinIcon;
    CDuiSkinBase *m_pSkinSplitter;
    CDuiSkinBase *m_pSkinFrame;
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

    CDuiTabCtrl();
    virtual ~CDuiTabCtrl() {}

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

    virtual void DrawItem(CDCHandle dc,const CRect &rcItem,int iItem,DWORD dwState);

    virtual UINT OnGetDuiCode()
    {
        return DUIC_WANTARROWS;
    }

    void OnPaint(CDCHandle dc);

    void OnLButtonDown(UINT nFlags, CPoint point);

    void OnMouseMove(UINT nFlags, CPoint point);

    void OnMouseLeave()
    {
        OnMouseMove(0,CPoint(-1,-1));
    }

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnDestroy();

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_MOUSELEAVE(OnMouseLeave)
    MSG_WM_KEYDOWN(OnKeyDown)
    WND_MSG_MAP_END()

    SOUO_ATTRIBUTES_BEGIN()
    DUIWIN_INT_ATTRIBUTE("cursel", m_nCurrentPage, FALSE)
    DUIWIN_INT_ATTRIBUTE("tabwidth", m_nTabWidth, FALSE)
    DUIWIN_INT_ATTRIBUTE("tabheight", m_nTabHeight, FALSE)
    DUIWIN_INT_ATTRIBUTE("tabpos", m_nTabPos, FALSE)
    DUIWIN_INT_ATTRIBUTE("framepos", m_nFramePos, FALSE)
    DUIWIN_INT_ATTRIBUTE("tabspacing", m_nTabSpacing, FALSE)
    DUIWIN_SKIN_ATTRIBUTE("tabskin", m_pSkinTab, FALSE)
    DUIWIN_SKIN_ATTRIBUTE("iconskin", m_pSkinIcon, FALSE)
    DUIWIN_SKIN_ATTRIBUTE("splitterskin", m_pSkinSplitter, FALSE)
    DUIWIN_SKIN_ATTRIBUTE("frameskin", m_pSkinFrame, FALSE)
    DUIWIN_INT_ATTRIBUTE("icon-x", m_ptIcon.x, FALSE)
    DUIWIN_INT_ATTRIBUTE("icon-y", m_ptIcon.y, FALSE)
    DUIWIN_INT_ATTRIBUTE("text-x", m_ptText.x, FALSE)
    DUIWIN_INT_ATTRIBUTE("text-y", m_ptText.y, FALSE)
    DUIWIN_ENUM_ATTRIBUTE("tabalign", int, TRUE)
    DUIWIN_ENUM_VALUE("top", AlignTop)
    DUIWIN_ENUM_VALUE("left", AlignLeft)
    DUIWIN_ENUM_END(m_nTabAlign)
    DUIWIN_INT_ATTRIBUTE("animatesteps",m_nAnimateSteps,FALSE)
    SOUI_ATTRIBUTES_END()
};

}//namespace SOUI