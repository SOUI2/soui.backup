#pragma once

#include "duiwnd.h"
#include "DuiPanel.h"
#include "DuiHeaderCtrl.h"

namespace SOUI
{

    enum{
        DUI_LVIF_TEXT=0x01,
        DUI_LVIF_IMAGE=0x02,
        DUI_LVIF_INDENT=0x04,
    };

    typedef int (__cdecl  *PFNLVCOMPAREEX)(void *, const void *, const void *);//使用快速排序算法中的比较函数,参考qsort_s

    typedef struct _DXLVSUBITEM
    {
        _DXLVSUBITEM()
        {
            mask=0;
            nImage = 0;
            strText=NULL;
            cchTextMax=0;
            nIndent=0;
        }

        UINT mask;
        LPTSTR strText;
        int        cchTextMax;
        UINT    nImage;
        int        nIndent;
    } DXLVSUBITEM;

    typedef CDuiArray<DXLVSUBITEM>   ArrSubItem;

    typedef struct _DXLVITEM
    {
        _DXLVITEM()
        {
            dwData = 0;
            arSubItems=NULL;
        }

        ArrSubItem  *arSubItems;
        DWORD       dwData;
    } DXLVITEM;

    //////////////////////////////////////////////////////////////////////////
    //  CDuiListCtrl
    class SOUI_EXP CDuiListCtrl : public CDuiPanel
    {
        SOUI_CLASS_NAME(CDuiListCtrl, "listctrl")

    public:
        CDuiListCtrl();
        virtual ~CDuiListCtrl();

        int             InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, LPARAM lParam=0);
        int             InsertItem(int nItem, LPCTSTR pszText, int nImage=-1);

        BOOL            SetItemData(int nItem, DWORD dwData);
        DWORD           GetItemData(int nItem);

        BOOL            SetSubItem(int nItem, int nSubItem,const DXLVSUBITEM* plv);
        BOOL            GetSubItem(int nItem, int nSubItem, DXLVSUBITEM* plv);

        BOOL            SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText);

        int             GetSelectedItem();
        void            SetSelectedItem(int nItem);

        int             GetItemCount();
        BOOL            SetItemCount( int nItems ,int nGrowBy);
        int             GetColumnCount();

        int             GetCountPerPage(BOOL bPartial);

        void            DeleteItem(int nItem);
        void            DeleteColumn(int iCol);
        void            DeleteAllItems();

        virtual BOOL    LoadChildren(pugi::xml_node xmlNode);

        int             HitTest(const CPoint& pt);

        BOOL            SortItems( PFNLVCOMPAREEX pfnCompare, void * pContext );
    protected:
        int             GetTopIndex() const;

        CRect           GetItemRect(int nItem, int nSubItem=0);
        virtual void    DrawItem(CDCHandle dc, CRect rcItem, int nItem);

        void            RedrawItem(int nItem);

        void            NotifySelChange(int nOldSel, int nNewSel);

        void            OnPaint(CDCHandle dc);
        void            OnDestroy();

        bool            OnHeaderClick(CDuiWindow* pSender, LPDUINMHDR pNmhdr);
        bool            OnHeaderSizeChanging(CDuiWindow* pSender, LPDUINMHDR pNmhdr);
        bool            OnHeaderSwap(CDuiWindow* pSender, LPDUINMHDR pNmhdr);

        virtual BOOL    OnScroll(BOOL bVertical,UINT uCode,int nPos);
        virtual void    OnLButtonDown(UINT nFlags, CPoint pt);
        virtual void    OnLButtonUp(UINT nFlags, CPoint pt);
        virtual void    OnSize(UINT nType, CSize size);
        virtual void    UpdateChildrenPosition();

        CRect           GetListRect();
        void            UpdateScrollBar();
        void            UpdateHeaderCtrl();

    protected:
        int             m_nHeaderHeight;
        int             m_nItemHeight;

        //  当前选中的列表项
        int             m_nSelectItem;
        int             m_nHoverItem;
        BOOL            m_bHotTrack;

        CPoint          m_ptIcon;
        CPoint          m_ptText;

        int             m_nMargin;

        COLORREF        m_crItemBg;
        COLORREF        m_crItemBg2;
        COLORREF        m_crItemSelBg;
        COLORREF        m_crText;
        COLORREF        m_crSelText;

        CDuiSkinBase*    m_pItemSkin;
        CDuiSkinBase*    m_pIconSkin;

    protected:
        typedef CDuiArray<DXLVITEM> ArrLvItem;

        CDuiHeaderCtrl*  m_pHeader;
        ArrLvItem       m_arrItems;
        CPoint          m_ptOrigin;

    protected:
        SOUI_ATTRS_BEGIN()
            ATTR_INT("headerHeight", m_nHeaderHeight, FALSE)
            ATTR_INT("itemHeight", m_nItemHeight, FALSE)
            ATTR_SKIN("itemskin", m_pItemSkin, TRUE)
            ATTR_SKIN("iconskin", m_pIconSkin, TRUE)
            ATTR_COLOR("critembg", m_crItemBg, FALSE)
            ATTR_COLOR("critembg2", m_crItemBg2, FALSE)
            ATTR_COLOR("critemselbg", m_crItemSelBg, FALSE)
            ATTR_COLOR("crtext", m_crText, FALSE)
            ATTR_COLOR("crseltext", m_crSelText, FALSE)
            ATTR_INT("icon-x", m_ptIcon.x, FALSE)
            ATTR_INT("icon-y", m_ptIcon.y, FALSE)
            ATTR_INT("text-x", m_ptText.x, FALSE)
            ATTR_INT("text-y", m_ptText.y, FALSE)
            ATTR_INT("hottrack", m_bHotTrack, FALSE)
        SOUI_ATTRS_END()

        WND_MSG_MAP_BEGIN()
            MSG_WM_PAINT(OnPaint)
            MSG_WM_DESTROY(OnDestroy)
            MSG_WM_SIZE(OnSize)
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
        WND_MSG_MAP_END()
    };

}//end of namespace
