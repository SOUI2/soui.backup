//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiListBoxEx
// Description: A DuiWindow Based ListBox Control. Can contain control as an item
//     Creator: Huang Jianxiong
//     Version: 2011.8.27 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once

#include "duiitempanel.h"

namespace SOUI
{

class SOUI_EXP CDuiListBoxEx :public CDuiScrollView
    ,public IDuiItemContainer
{
public:

    SOUI_CLASS_NAME(CDuiListBoxEx, "listboxex")

    CDuiListBoxEx();

    virtual ~CDuiListBoxEx();


    void DeleteAllItems(BOOL bUpdate=TRUE);

    void DeleteItem(int iItem);

    int InsertItem(int iItem,pugi::xml_node xmlNode,DWORD dwData=0);

    int InsertItem(int iItem,LPCWSTR pszXml,DWORD dwData=0);

    BOOL SetCurSel(int iItem);

    void EnsureVisible(int iItem);

    int GetCurSel();

    int GetItemObjIndex(CDuiWindow *pItemObj);

    CDuiWindow * GetItemPanel(int iItem);

    LPARAM GetItemData(int iItem);

	void SetItemData(int iItem,LPARAM lParam);

    //************************************
    // Method:    SetItemCount
    // FullName:  SOUI::CDuiListBoxEx::SetItemCount
    // Access:    public 
    // Returns:   BOOL
    // Qualifier:
    // Parameter: int nItems 条目数量
    // Parameter: LPCTSTR pszXmlTemplate 显示时使用的XML模板，为空时使用XML脚本中指定的template子节点数据
    //************************************
    BOOL SetItemCount(int nItems,LPCTSTR pszXmlTemplate=NULL);

    int GetItemCount() ;

    int GetItemHeight()
    {
        return m_nItemHei;
    }

    void RedrawItem(int iItem);

    //自动修改pt的位置为相对当前项的偏移量
    int HitTest(CPoint &pt);

    virtual BOOL Load(pugi::xml_node xmlNode);

	BOOL IsVirtual(){return m_bVirtual;}
protected:
	virtual void OnItemSetCapture(CDuiItemPanel *pItem,BOOL bCapture);
	virtual BOOL OnItemGetRect(CDuiItemPanel *pItem,CRect &rcItem);
	virtual BOOL IsItemRedrawDelay(){return m_bItemRedrawDelay;}
protected:
	void UpdatePanelsIndex(UINT nFirst=0,UINT nLast=-1);

	CRect	GetItemRect(int iItem);


    virtual int GetScrollLineSize(BOOL bVertical);

    void OnPaint(CDCHandle dc);

	void OnSize(UINT nType, CSize size);

    virtual void OnDrawItem(CDCHandle & dc, CRect & rc, int iItem);

    virtual BOOL LoadChildren(pugi::xml_node xmlNode);
    // Get tooltip Info
    virtual BOOL OnUpdateToolTip(HDUIWND hCurTipHost,HDUIWND &hNewTipHost,CRect &rcTip,CDuiStringT &strTip);

    void NotifySelChange(int nOldSel,int nNewSel);

	LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

	LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );

    void OnMouseLeave();

	BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    UINT OnGetDuiCode();

    virtual BOOL OnDuiSetCursor(const CPoint &pt);

	virtual void OnViewOriginChanged(CPoint ptOld,CPoint ptNew);

    void OnDestroy();

	void OnSetDuiFocus();

	void OnKillDuiFocus();

	LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

	void Relayout();
protected:
    CDuiArray<CDuiItemPanel *> m_arrItems;

    int		m_iSelItem;
    int		m_iHoverItem;
    int		m_iScrollSpeed;

	CDuiItemPanel	* m_pTemplPanel;	//虚拟列表使用的DUI模板
	int		m_nItems;					//虚拟列表中记录列表项
	pugi::xml_document m_xmlTempl;		////列表模板XML
    CDuiItemPanel	*	m_pCapturedFrame;
    CDuiSkinBase * m_pItemSkin;
	COLORREF m_crItemBg,m_crItemSelBg;
	int		m_nItemHei;
	BOOL	m_bVirtual;
	BOOL	m_bItemRedrawDelay;			//表项重绘时缓冲
public:
    SOUO_ATTRIBUTES_BEGIN()
		DUIWIN_INT_ATTRIBUTE("scrollspeed", m_iScrollSpeed, FALSE)
		DUIWIN_INT_ATTRIBUTE("itemheight", m_nItemHei, FALSE)
		DUIWIN_INT_ATTRIBUTE("virtual", m_bVirtual, TRUE)
		DUIWIN_SKIN_ATTRIBUTE("itemskin", m_pItemSkin, TRUE)
		DUIWIN_COLOR_ATTRIBUTE("critembg",m_crItemBg,FALSE)
		DUIWIN_COLOR_ATTRIBUTE("critemselbg",m_crItemSelBg,FALSE)
		DUIWIN_INT_ATTRIBUTE("itemredrawdelay", m_bItemRedrawDelay, TRUE)
    SOUI_ATTRIBUTES_END()

    WND_MSG_MAP_BEGIN()
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
		MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
		MESSAGE_HANDLER_EX(WM_IME_CHAR,OnKeyEvent)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CHAR(OnChar)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
		MSG_WM_KILLFOCUS_EX(OnKillDuiFocus)
		MSG_WM_NCCALCSIZE(OnNcCalcSize)
    WND_MSG_MAP_END()
};

}//namespace SOUI