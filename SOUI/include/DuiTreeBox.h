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

class SOUI_EXP CDuiTreeItem : public CDuiItemPanel
{
public:
    CDuiTreeItem(CDuiWindow *pFrameHost);

    BOOL m_bCollapsed;
    BOOL m_bVisible;
    int  m_nLevel;

    HSTREEITEM m_hItem;
};

class SOUI_EXP CDuiTreeBox
    : public CDuiScrollView
    , public IDuiItemContainer
    , protected CSTree<CDuiTreeItem *>
{
    SOUI_CLASS_NAME(CDuiTreeBox, "treebox")
public:
    CDuiTreeBox();

    virtual ~CDuiTreeBox();

    HSTREEITEM InsertItem(pugi::xml_node xmlNode,DWORD dwData,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);
    CDuiTreeItem* InsertItem(LPCWSTR pszXml,DWORD dwData,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);

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

    CDuiTreeItem * GetItemPanel(HSTREEITEM hItem)
    {
        return GetItem(hItem);
    }
protected:
    void SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible);

    virtual void OnNodeFree(CDuiTreeItem * & pItem);

    virtual int GetScrollLineSize(BOOL bVertical);

    virtual BOOL LoadChildren(pugi::xml_node xmlNode);

    void LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlNode);

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    int GetItemShowIndex(HSTREEITEM hItemObj);

    void RedrawItem(HSTREEITEM hItem);

    void DrawItem(CDCHandle & dc, CRect & rc, HSTREEITEM hItem);
    void OnPaint(CDCHandle dc);

    void OnLButtonDown(UINT nFlags,CPoint pt);
    void OnLButtonDbClick(UINT nFlags,CPoint pt);
	LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    void OnMouseMove(UINT nFlags,CPoint pt);
    void OnMouseLeave();

	void OnSetDuiFocus();
	void OnKillDuiFocus();

	LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );

    virtual LRESULT DuiNotify(LPDUINMHDR pnms);
    virtual BOOL OnDuiSetCursor(const CPoint &pt);
	virtual void OnViewOriginChanged( CPoint ptOld,CPoint ptNew );

	virtual UINT OnGetDuiCode()
	{
		return DUIC_WANTALLKEYS;
	}

    BOOL IsAncestor(HSTREEITEM hItem1,HSTREEITEM hItem2);
protected:
    virtual void OnItemSetCapture(CDuiItemPanel *pItem,BOOL bCapture);
    virtual BOOL OnItemGetRect(CDuiItemPanel *pItem,CRect &rcItem);
	virtual BOOL IsItemRedrawDelay(){return m_bItemRedrawDelay;}

    HSTREEITEM	m_hSelItem;
    HSTREEITEM	m_hHoverItem;

    int			m_nVisibleItems;

    CDuiItemPanel	*	m_pCapturedFrame;

    int m_nItemHei,m_nIndent;
    COLORREF m_crItemBg,m_crItemSelBg;
    CDuiSkinBase * m_pItemSkin;
	BOOL m_bItemRedrawDelay;
	pugi::xml_document m_xmlSwitch;

    SOUO_ATTRIBUTES_BEGIN()
		DUIWIN_INT_ATTRIBUTE("indent", m_nIndent, TRUE)
		DUIWIN_INT_ATTRIBUTE("itemhei", m_nItemHei, TRUE)
		DUIWIN_SKIN_ATTRIBUTE("itemskin", m_pItemSkin, TRUE)
		DUIWIN_COLOR_ATTRIBUTE("critembg",m_crItemBg,FALSE)
		DUIWIN_COLOR_ATTRIBUTE("critemselbg",m_crItemSelBg,FALSE)
		DUIWIN_INT_ATTRIBUTE("itemredrawdelay", m_bItemRedrawDelay, TRUE)
    SOUI_ATTRIBUTES_END()

    WND_MSG_MAP_BEGIN()
		MSG_WM_PAINT(OnPaint)
		MSG_WM_NCCALCSIZE(OnNcCalcSize)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
		MSG_WM_KILLFOCUS_EX(OnKillDuiFocus)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
		MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
		MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_IME_CHAR,OnKeyEvent)
   WND_MSG_MAP_END()
};

}//namespace SOUI