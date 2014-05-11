//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiTreeCtrl
// Description: CDuiTreeCtrl
//     Creator: JinHui
//     Version: 2012.12.16 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////


#pragma once
#include "DuiPanel.h"
#include "stree.hpp"

namespace SOUI{

enum {
    DuiTVIMask_Toggle		 = 0x00000001UL, 
    DuiTVIMask_CheckBox		 = 0x00000002UL, 
    DuiTVIMask_Icon		     = 0x00000004UL
};

enum {
	DuiTVICheckBox_UnChecked,		//未选中
	DuiTVICheckBox_Checked,			//选中
    DuiTVICheckBox_PartChecked		//部分选中
};

enum {
	DuiTVIBtn_None,					
    DuiTVIBtn_Toggle, 
    DuiTVIBtn_CheckBox
};

typedef struct tagTVITEM {    
	CDuiStringT		strText;
    int			nImage;
    int			nSelectedImage;
	LPARAM      lParam;	

	HSTREEITEM	hItem;					
	BOOL		bCollapsed;
	BOOL		bVisible;
	BOOL		bHasChildren;
	int			nLevel;
	int			nCheckBoxValue;
	int         nItemWidth;
	DWORD		dwToggleState;
	DWORD       dwCheckBoxState;

	tagTVITEM()
	{
		nImage = -1;
		nSelectedImage = -1;
		lParam = NULL;

		hItem = NULL;
		bCollapsed = FALSE;
		bVisible = TRUE;
		bHasChildren = FALSE;
		nLevel = 0;
		nCheckBoxValue = DuiTVICheckBox_UnChecked;
		nItemWidth = 0;
		dwToggleState = DuiWndState_Normal;
		dwCheckBoxState = DuiWndState_Normal;
	}

} TVITEM, *LPTVITEM;


class SOUI_EXP CDuiTreeCtrl
	: public CDuiScrollView
	, protected CSTree<LPTVITEM>
{
	DUIOBJ_DECLARE_CLASS_NAME(CDuiTreeCtrl, "treectrl")
public:
	CDuiTreeCtrl();

	virtual ~CDuiTreeCtrl();
	
	HSTREEITEM InsertItem(LPCTSTR lpszItem, HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST);
	HSTREEITEM InsertItem(LPCTSTR lpszItem, int nImage,
		int nSelectedImage, HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST);		
	HSTREEITEM InsertItem(LPCTSTR lpszItem, int nImage,
		int nSelectedImage, LPARAM lParam,
		HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST);

	BOOL RemoveItem(HSTREEITEM hItem);
	void RemoveAllItems();

	HSTREEITEM GetRootItem();
	HSTREEITEM GetNextSiblingItem(HSTREEITEM hItem);
	HSTREEITEM GetPrevSiblingItem(HSTREEITEM hItem);
	HSTREEITEM GetChildItem(HSTREEITEM hItem,BOOL bFirst =TRUE);
	HSTREEITEM GetParentItem(HSTREEITEM hItem);
	HSTREEITEM GetSelectedItem();

	BOOL GetItemText(HSTREEITEM hItem, CDuiStringT& strText) const;
	BOOL SetItemText(HSTREEITEM hItem, LPCTSTR lpszItem);
	BOOL GetItemImage(HSTREEITEM hItem, int& nImage, int& nSelectedImage) const;
	BOOL SetItemImage(HSTREEITEM hItem, int nImage, int nSelectedImage);
	LPARAM GetItemData(HSTREEITEM hItem) const;
	BOOL SetItemData(HSTREEITEM hItem, LPARAM lParam);
	BOOL ItemHasChildren(HSTREEITEM hItem);

	BOOL GetCheckState(HSTREEITEM hItem) const;	
	BOOL SetCheckState(HSTREEITEM hItem, BOOL bCheck);	

	BOOL Expand(HSTREEITEM hItem , UINT nCode = TVE_EXPAND );
	BOOL EnsureVisible(HSTREEITEM hItem);

	void PageUp();
	void PageDown();

	BOOL RedrawRegion(CDCHandle& dc, CRgn& rgn);

protected:

	virtual BOOL LoadChildren(pugi::xml_node xmlNode);
	virtual void LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlNode);
	virtual void LoadItemAttribute(pugi::xml_node xmlNode, LPTVITEM pItem);

	HSTREEITEM InsertItem(LPTVITEM pItemObj,HSTREEITEM hParent,HSTREEITEM hInsertAfter,BOOL bEnsureVisible);
	HSTREEITEM InsertItem(pugi::xml_node xmlNode,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);
	
	virtual int GetScrollLineSize(BOOL bVertical);	

	BOOL IsAncestor(HSTREEITEM hItem1,HSTREEITEM hItem2);

	void SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible);

	void SetChildrenState(HSTREEITEM hItem, int nCheckValue);
	BOOL CheckChildrenState(HSTREEITEM hItem, BOOL bCheck);	
	void CheckState(HSTREEITEM hItem, BOOL bCheck, BOOL bCheckChild = TRUE);	

	virtual void ItemLayout();
	virtual void CalaItemWidth(LPTVITEM pItem);
	virtual int  GetMaxItemWidth();
	virtual int  GetMaxItemWidth(HSTREEITEM hItem);
	int  GetItemShowIndex(HSTREEITEM hItemObj);
	BOOL GetItemRect( LPTVITEM pItem,CRect &rcItem );

	HSTREEITEM HitTest(CPoint &pt);		

	void RedrawItem(HSTREEITEM hItem);
	virtual void DrawItem(CDCHandle & dc, CRect & rc, HSTREEITEM hItem);

	int ItemHitTest(HSTREEITEM hItem,CPoint &pt) const;
    void ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);
	void ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove);

	void ItemLButtonDown(HSTREEITEM hItem, UINT nFlags,CPoint pt);
	void ItemLButtonUp(HSTREEITEM hItem, UINT nFlags,CPoint pt);
	void ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags,CPoint pt);

	void ItemMouseMove(HSTREEITEM hItem, UINT nFlags,CPoint pt);
	void ItemMouseLeave(HSTREEITEM hItem);

	void NotifyParent();
	
protected:

	void OnDestroy();

	void OnPaint(CDCHandle dc);

	void OnLButtonDown(UINT nFlags,CPoint pt);
	void OnLButtonUp(UINT nFlags,CPoint pt);
	void OnLButtonDbClick(UINT nFlags,CPoint pt);
	void OnRButtonDown(UINT nFlags, CPoint pt);
	void OnRButtonUp(UINT nFlags,CPoint pt);

	void OnMouseMove(UINT nFlags,CPoint pt);
	void OnMouseLeave();

	virtual void OnNodeFree(LPTVITEM & pItemData){
		delete pItemData;
	}

protected:

	HSTREEITEM	m_hSelItem;
	HSTREEITEM	m_hHoverItem;
	HSTREEITEM	m_hCaptureItem;

	int			m_nVisibleItems;
	int         m_nMaxItemWidth;

	UINT        m_uItemMask;
	int         m_nItemOffset;
	CRect       m_rcToggle;
	CRect       m_rcCheckBox;
	CRect       m_rcIcon;
	int			m_nItemHoverBtn;
	int			m_nItemPushDownBtn;

	int			m_nItemHei,m_nIndent, m_nItemMargin;
	BOOL		m_bCheckBox;
	BOOL		m_bRightClickSel;
	CDuiSkinBase * m_pItemBgSkin, * m_pItemSelSkin;
	CDuiSkinBase * m_pIconSkin, * m_pToggleSkin, * m_pCheckSkin;
	COLORREF m_crItemBg,m_crItemSelBg;
	COLORREF m_crItemText,m_crItemSelText;

	UINT		m_uPopMenuID;

	DUIWIN_DECLARE_ATTRIBUTES_BEGIN()		
		DUIWIN_INT_ATTRIBUTE("indent", m_nIndent, TRUE)
		DUIWIN_INT_ATTRIBUTE("itemhei", m_nItemHei, TRUE)
		DUIWIN_INT_ATTRIBUTE("itemmargin", m_nItemMargin, TRUE)
		DUIWIN_INT_ATTRIBUTE("checkbox", m_bCheckBox, TRUE)
		DUIWIN_INT_ATTRIBUTE("rightclicksel", m_bRightClickSel, TRUE)
		DUIWIN_SKIN_ATTRIBUTE("itembgskin", m_pItemBgSkin, TRUE)
		DUIWIN_SKIN_ATTRIBUTE("itemselskin", m_pItemSelSkin, TRUE)
		DUIWIN_SKIN_ATTRIBUTE("toggleskin", m_pToggleSkin, TRUE)			
		DUIWIN_SKIN_ATTRIBUTE("iconskin", m_pIconSkin, TRUE)
		DUIWIN_SKIN_ATTRIBUTE("checkskin", m_pCheckSkin, TRUE)
		DUIWIN_COLOR_ATTRIBUTE("critembg",m_crItemBg,FALSE)
		DUIWIN_COLOR_ATTRIBUTE("critemselbg",m_crItemSelBg,FALSE)
		DUIWIN_COLOR_ATTRIBUTE("critemtext",m_crItemText,FALSE)
		DUIWIN_COLOR_ATTRIBUTE("critemseltext",m_crItemSelText,FALSE)
	DUIWIN_DECLARE_ATTRIBUTES_END()

	DUIWIN_BEGIN_MSG_MAP()
		MSG_WM_PAINT(OnPaint)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_RBUTTONDOWN(OnRButtonDown);
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
	DUIWIN_END_MSG_MAP()
};

}//namespace SOUI
