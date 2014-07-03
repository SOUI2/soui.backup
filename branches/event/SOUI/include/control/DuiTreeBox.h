//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiTreeCtrl
// Description: CDuiTreeCtrl
//     Creator: huangjianxiong
//     Version: 2011.10.14 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////


#pragma once
#include "DuiPanel.h"
#include "stree.hpp"
#include "duiItempanel.h"

namespace SOUI
{

class SOUI_EXP STreeItem : public SItemPanel
{
public:
    STreeItem(SWindow *pFrameHost);

    BOOL m_bCollapsed;
    BOOL m_bVisible;
    int  m_nLevel;

    HSTREEITEM m_hItem;
};

class SOUI_EXP STreeBox
    : public SScrollView
    , public IItemContainer
    , protected CSTree<STreeItem *>
{
    SOUI_CLASS_NAME(STreeBox, L"treebox")
public:
    STreeBox();

    virtual ~STreeBox();

    HSTREEITEM InsertItem(pugi::xml_node xmlNode,DWORD dwData,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);
    STreeItem* InsertItem(LPCWSTR pszXml,DWORD dwData,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);

    BOOL RemoveItem(HSTREEITEM hItem);

    void RemoveAllItems();
    HSTREEITEM GetRootItem();
    HSTREEITEM GetNextSiblingItem(HSTREEITEM hItem);
    HSTREEITEM GetPrevSiblingItem(HSTREEITEM hItem);
    HSTREEITEM GetChildItem(HSTREEITEM hItem,BOOL bFirst =TRUE);
    HSTREEITEM GetParentItem(HSTREEITEM hItem);

    void PageUp();

    void PageDown();

    void OnDestroy();

    BOOL Expand(HSTREEITEM hItem , UINT nCode);

    BOOL EnsureVisible(HSTREEITEM hItem);

    //自动修改pt的位置为相对当前项的偏移量
    HSTREEITEM HitTest(CPoint &pt);

    STreeItem * GetItemPanel(HSTREEITEM hItem)
    {
        return GetItem(hItem);
    }
protected:
    void SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible);

    virtual void OnNodeFree(STreeItem * & pItem);

    virtual int GetScrollLineSize(BOOL bVertical);

    virtual BOOL CreateChildren(pugi::xml_node xmlNode);

    void LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlNode);

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    int GetItemShowIndex(HSTREEITEM hItemObj);

    void RedrawItem(HSTREEITEM hItem);

    void DrawItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem);
    void OnPaint(IRenderTarget *pRT);

    void OnLButtonDown(UINT nFlags,CPoint pt);
    void OnLButtonDbClick(UINT nFlags,CPoint pt);
    LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    void OnMouseMove(UINT nFlags,CPoint pt);
    void OnMouseLeave();

    void OnSetFocus();
    void OnKillFocus();

    LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );

    virtual LRESULT FireEvent(EventArgs &evt);
    virtual BOOL OnSetCursor(const CPoint &pt);
    virtual void OnViewOriginChanged( CPoint ptOld,CPoint ptNew );

    virtual UINT OnGetDlgCode()
    {
        return DUIC_WANTALLKEYS;
    }

    BOOL IsAncestor(HSTREEITEM hItem1,HSTREEITEM hItem2);
protected:
    virtual void OnItemSetCapture(SItemPanel *pItem,BOOL bCapture);
    virtual BOOL OnItemGetRect(SItemPanel *pItem,CRect &rcItem);
    virtual BOOL IsItemRedrawDelay(){return m_bItemRedrawDelay;}

    HSTREEITEM    m_hSelItem;
    HSTREEITEM    m_hHoverItem;

    int            m_nVisibleItems;

    SItemPanel    *    m_pCapturedFrame;

    int m_nItemHei,m_nIndent;
    COLORREF m_crItemBg,m_crItemSelBg;
    ISkinObj * m_pItemSkin;
    BOOL m_bItemRedrawDelay;
    pugi::xml_document m_xmlSwitch;

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"indent", m_nIndent, TRUE)
        ATTR_INT(L"itemhei", m_nItemHei, TRUE)
        ATTR_SKIN(L"itemskin", m_pItemSkin, TRUE)
        ATTR_COLOR(L"critembg",m_crItemBg,FALSE)
        ATTR_COLOR(L"critemselbg",m_crItemSelBg,FALSE)
        ATTR_INT(L"itemredrawdelay", m_bItemRedrawDelay, TRUE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR,OnKeyEvent)
   SOUI_MSG_MAP_END()
};

}//namespace SOUI