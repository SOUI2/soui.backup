#pragma once

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

    typedef SArray<DXLVSUBITEM>   ArrSubItem;

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
    class SOUI_EXP SListCtrl : public SPanel
    {
        SOUI_CLASS_NAME(SListCtrl, L"listctrl")

    public:
        SListCtrl();
        virtual ~SListCtrl();

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

        virtual BOOL    CreateChildren(pugi::xml_node xmlNode);

        int             HitTest(const CPoint& pt);

        BOOL            SortItems( PFNLVCOMPAREEX pfnCompare, void * pContext );
    protected:
        int             GetTopIndex() const;

        CRect           GetItemRect(int nItem, int nSubItem=0);
        virtual void    DrawItem(IRenderTarget *pRT, CRect rcItem, int nItem);

        void            RedrawItem(int nItem);

        void            NotifySelChange(int nOldSel, int nNewSel);

        void            OnPaint(IRenderTarget *pRT);
        void            OnDestroy();

        bool            OnHeaderClick(EventArgs *pEvt);
        bool            OnHeaderSizeChanging(EventArgs *pEvt);
        bool            OnHeaderSwap(EventArgs *pEvt);

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

        COLORREF        m_crItemBg;
        COLORREF        m_crItemBg2;
        COLORREF        m_crItemSelBg;
        COLORREF        m_crText;
        COLORREF        m_crSelText;

        ISkinObj*    m_pItemSkin;
        ISkinObj*    m_pIconSkin;

    protected:
        typedef SArray<DXLVITEM> ArrLvItem;

        SHeaderCtrl*  m_pHeader;
        ArrLvItem       m_arrItems;
        CPoint          m_ptOrigin;

    protected:
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"headerHeight", m_nHeaderHeight, FALSE)
            ATTR_INT(L"itemHeight", m_nItemHeight, FALSE)
            ATTR_SKIN(L"itemskin", m_pItemSkin, TRUE)
            ATTR_SKIN(L"iconskin", m_pIconSkin, TRUE)
            ATTR_COLOR(L"critembg", m_crItemBg, FALSE)
            ATTR_COLOR(L"critembg2", m_crItemBg2, FALSE)
            ATTR_COLOR(L"critemselbg", m_crItemSelBg, FALSE)
            ATTR_COLOR(L"crtext", m_crText, FALSE)
            ATTR_COLOR(L"crseltext", m_crSelText, FALSE)
            ATTR_INT(L"icon-x", m_ptIcon.x, FALSE)
            ATTR_INT(L"icon-y", m_ptIcon.y, FALSE)
            ATTR_INT(L"text-x", m_ptText.x, FALSE)
            ATTR_INT(L"text-y", m_ptText.y, FALSE)
            ATTR_INT(L"hottrack", m_bHotTrack, FALSE)
        SOUI_ATTRS_END()

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_DESTROY(OnDestroy)
            MSG_WM_SIZE(OnSize)
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
        SOUI_MSG_MAP_END()
    };

}//end of namespace
