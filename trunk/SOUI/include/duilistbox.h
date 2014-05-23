//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiListBox
// Description: A DuiWindow Based ListBox Control.
//     Creator: JinHui
//     Version: 2012.12.18 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once

#include "DuiPanel.h"

namespace SOUI
{

typedef struct tagLBITEM
{
    CDuiStringT        strText;
    int            nImage;
    LPARAM      lParam;

    tagLBITEM()
    {
        nImage = -1;
        lParam = NULL;
    }

} LBITEM, *LPLBITEM;

class SOUI_EXP CDuiListBox :public CDuiScrollView
{
public:

    SOUI_CLASS_NAME(CDuiListBox, "listbox")

    CDuiListBox();

    virtual ~CDuiListBox();

    virtual BOOL Load(pugi::xml_node xmlNode);

    int GetCount() const;

    int GetCurSel() const;

    BOOL SetCurSel(int nIndex);

    inline int GetTopIndex() const;

    BOOL SetTopIndex(int nIndex);

    int GetItemHeight() const
    {
        return m_nItemHei;
    }

    LPARAM GetItemData(int nIndex) const;

    BOOL SetItemData(int nIndex, LPARAM lParam);

    int GetText(int nIndex, LPTSTR lpszBuffer) const;

    int GetText(int nIndex, CDuiStringT& strText) const;

    int GetTextLen(int nIndex) const;

    int GetItemHeight(int nIndex) const;

    BOOL SetItemHeight(int nIndex, int cyItemHeight);

    void DeleteAll();

    BOOL DeleteString(int nIndex);

    int AddString(LPCTSTR lpszItem, int nImage = -1, LPARAM lParam = 0);

    int InsertString(int nIndex, LPCTSTR lpszItem, int nImage = -1, LPARAM lParam = 0);

    void EnsureVisible(int nIndex);

    void RedrawItem(int iItem);

    //自动修改pt的位置为相对当前项的偏移量
    int HitTest(CPoint &pt);

protected:

    virtual BOOL LoadChildren(pugi::xml_node xmlNode);
    void LoadItemAttribute(pugi::xml_node xmlNode, LPLBITEM pItem);

    int InsertItem(int nIndex, LPLBITEM pItem);

    virtual int GetScrollLineSize(BOOL bVertical);

    void DrawItem(CDCHandle &dc, CRect &rc, int iItem);

    void NotifySelChange(int nOldSel,int nNewSel);

    UINT OnGetDuiCode();

protected:

    void OnSize(UINT nType,CSize size);

    void OnPaint(CDCHandle dc);

    void OnLButtonDown(UINT nFlags,CPoint pt);

    void OnLButtonDbClick(UINT nFlags,CPoint pt);

    void OnLButtonUp(UINT nFlags,CPoint pt);

    void OnMouseMove(UINT nFlags,CPoint pt);

    void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnDestroy();

    void OnShowWindow(BOOL bShow, UINT nStatus);

protected:

    CDuiArray<LPLBITEM>    m_arrItems;

    int        m_nItemHei;
    int        m_iSelItem;
    int        m_iHoverItem;
    int        m_iScrollSpeed;
    BOOL    m_bHotTrack;

    CPoint m_ptIcon;
    CPoint m_ptText;

    COLORREF m_crItemBg, m_crItemBg2, m_crItemSelBg;
    COLORREF m_crText, m_crSelText;
    CDuiSkinBase *m_pItemSkin, *m_pIconSkin;

public:

    SOUO_ATTRIBUTES_BEGIN()
    DUIWIN_INT_ATTRIBUTE("scrollspeed", m_iScrollSpeed, FALSE)
    DUIWIN_INT_ATTRIBUTE("itemheight", m_nItemHei, FALSE)
    DUIWIN_SKIN_ATTRIBUTE("itemskin", m_pItemSkin, TRUE)
    DUIWIN_SKIN_ATTRIBUTE("iconskin", m_pIconSkin, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("critembg",m_crItemBg,FALSE)
    DUIWIN_COLOR_ATTRIBUTE("critembg2", m_crItemBg2, FALSE)
    DUIWIN_COLOR_ATTRIBUTE("critemselbg",m_crItemSelBg,FALSE)
    DUIWIN_COLOR_ATTRIBUTE("crtext",m_crText,FALSE)
    DUIWIN_COLOR_ATTRIBUTE("crseltext",m_crSelText,FALSE)
    DUIWIN_INT_ATTRIBUTE("icon-x", m_ptIcon.x, FALSE)
    DUIWIN_INT_ATTRIBUTE("icon-y", m_ptIcon.y, FALSE)
    DUIWIN_INT_ATTRIBUTE("text-x", m_ptText.x, FALSE)
    DUIWIN_INT_ATTRIBUTE("text-y", m_ptText.y, FALSE)
    DUIWIN_INT_ATTRIBUTE("hottrack",m_bHotTrack,FALSE)
    SOUI_ATTRIBUTES_END()

    WND_MSG_MAP_BEGIN()
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    WND_MSG_MAP_END()
};

}//namespace SOUI