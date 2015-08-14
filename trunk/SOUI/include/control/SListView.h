#pragma once

#include "core/Swnd.h"
#include "interface/Adapter-i.h"
#include "stree.hpp"

namespace SOUI
{
    interface IListViewItemLocator : public IObjRef
    {
        virtual void SetAdapter(IAdapter *pAdapter) PURE;
        virtual void OnDataSetChanged() PURE;
        virtual bool IsFixHeight() const PURE;
        virtual int GetItemHeight(int iItem) const PURE;
        virtual void SetItemHeight(int iItem,int nHeight) PURE;
        virtual int GetTotalHeight() PURE;
        virtual int Item2Position(int iItem) PURE;
        virtual int Position2Item(int position) PURE;
        virtual int GetScrollLineSize() const PURE;
        virtual int GetDividerSize() const PURE;
    };
    
    class SOUI_EXP SListViewItemLocatorFix : public TObjRefImpl<IListViewItemLocator>
    {
    public:
        SListViewItemLocatorFix(int nItemHei,int nDividerSize=0);
        
        virtual void SetAdapter(IAdapter *pAdapter);
        
        virtual void OnDataSetChanged(){}

        virtual bool IsFixHeight() const;
        
        virtual int GetItemHeight(int iItem) const ;
        
        virtual void SetItemHeight(int iItem,int nHeight);
        

        virtual int GetTotalHeight();
        virtual int Item2Position(int iItem);

        virtual int Position2Item(int position);

        virtual int GetScrollLineSize() const;
        
        virtual int GetDividerSize() const
        {
            return m_nDividerSize;
        }
    
    protected:
        int GetFixItemHeight() const {return m_nItemHeight+m_nDividerSize;}
    
        int m_nItemHeight;
        int m_nDividerSize;
        
        CAutoRefPtr<IAdapter> m_adapter;
    };
    
    class SOUI_EXP SListViewItemLocatorFlex : public TObjRefImpl<IListViewItemLocator>
    {
    
    public:
        
        SListViewItemLocatorFlex(int nItemHei,int nDividerSize=0);
        ~SListViewItemLocatorFlex();
        
        
        virtual void SetAdapter(IAdapter *pAdapter);
        virtual void OnDataSetChanged();

        virtual bool IsFixHeight() const;

        virtual int GetItemHeight(int iItem) const;

        virtual void SetItemHeight(int iItem,int nHeight);


        virtual int GetTotalHeight();
        virtual int Item2Position(int iItem);

        virtual int Position2Item(int position);

        virtual int GetScrollLineSize() const;   
        
        virtual int GetDividerSize() const
        {
            return m_nDividerSize;
        }
     
    protected:
        void InitIndex(HSTREEITEM hParent,int nItems,int nSubBranchSize);
        int GetFixItemHeight() const {return m_nItemHeight+m_nDividerSize;}
        int GetIndexDeep() const;
        void Clear();
        int Branch2Offset(HSTREEITEM hBranch) const;
        int Branch2Index(HSTREEITEM hBranch) const;
        HSTREEITEM Offset2Branch(HSTREEITEM hParent,int nOffset);
        
        int m_nItemHeight;  //默认表项高度
        int m_nDividerSize;
        
        struct BranchInfo
        {
            int nBranchHei; //分枝高度
            int nBranchSize;//分枝中包含的节点数量
        };
        
        CSTree<BranchInfo>    m_itemPosIndex;//记录分枝高度
        class SegmentInfo
        {
        public:
            SegmentInfo(int nItems,HSTREEITEM hBranch):hItem(hBranch){
                this->nItems = nItems;
                pItemHeight = new int[nItems];
                memset(pItemHeight,0xff,nItems*sizeof(int));
            }
            ~SegmentInfo()
            {
                if(pItemHeight) delete[] pItemHeight;
            }
        
            HSTREEITEM hItem;
            int        nItems;
            int*       pItemHeight;//段中每一个表项的高度
        };
        
        SArray<SegmentInfo*>     m_segments;
        CAutoRefPtr<IAdapter>   m_adapter;
    };
    
    class SOUI_EXP SListView : public SPanel
        , protected IItemContainer
    {
        SOUI_CLASS_NAME(SListView,L"listview")

        friend class SListViewDataSetObserver;
    public:
        SListView();
        ~SListView();

        BOOL SetAdapter(IAdapter * adapter);
        
        IAdapter * GetAdapter() {
            return m_adapter;
        }
        
        IListViewItemLocator * GetItemLocator(){
            return m_lvItemLocator;
        }
        
        void SetItemLocator(IListViewItemLocator *pItemLocator);
        void EnsureVisible( int iItem );
        
        void SetSel(int iItem,BOOL bNotify=FALSE);
        int  GetSel()const{return m_iSelItem;}
        
    protected:
        virtual void OnItemSetCapture(SItemPanel *pItem,BOOL bCapture);
        virtual BOOL OnItemGetRect(SItemPanel *pItem,CRect &rcItem);
        virtual BOOL IsItemRedrawDelay();
        virtual void OnItemRequestRelayout(SItemPanel *pItem);
        
    protected:
        void onDataSetChanged();
        void onDataSetInvalidated();
        
    protected:
        virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);
        virtual int  GetScrollLineSize(BOOL bVertical);
        virtual BOOL CreateChildren(pugi::xml_node xmlNode);

        virtual BOOL OnUpdateToolTip(CPoint pt, SwndToolTipInfo & tipInfo);

    protected:
        void _SetSel(int iItem,BOOL bNotify, SWND hHitWnd);

        void UpdateScrollBar();
        void RedrawItem(SItemPanel *pItem);
        SItemPanel * HitTest(CPoint & pt);
        SItemPanel * GetItemPanel(int iItem);
        
        void UpdateVisibleItems();
        
        void OnPaint(IRenderTarget *pRT);
        void OnSize(UINT nType, CSize size);
        
        LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

        LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );
        void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

        void OnMouseLeave();

        BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_SIZE(OnSize)
            MSG_WM_MOUSEWHEEL(OnMouseWheel)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_KEYDOWN(OnKeyDown)
            MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        SOUI_MSG_MAP_END()

        SOUI_ATTRS_BEGIN()
            ATTR_SKIN(L"dividerSkin",m_pSkinDivider,TRUE)
            ATTR_INT(L"dividerSize",m_nDividerSize,FALSE)
        SOUI_ATTRS_END()
    protected:
        CAutoRefPtr<IAdapter>           m_adapter;
        CAutoRefPtr<IDataSetObserver>   m_observer;
        CAutoRefPtr<IListViewItemLocator>  m_lvItemLocator;//列表项定位接口
        struct ItemInfo
        {
            SItemPanel *pItem;
            int nType;
        };
        
        int                             m_iFirstVisible;//第一个显示项索引
        SList<ItemInfo>                 m_lstItems; //当前正在显示的项
        SItemPanel*                     m_itemCapture;//The item panel that has been set capture.
        
        int                             m_iSelItem;
        SItemPanel*                     m_pHoverItem;
        
        SArray<SList<SItemPanel*> *>    m_itemRecycle;//item回收站,每一种样式在回收站中保持一个列表，以便重复利用
        
        BOOL                            m_bScrollUpdate; //滚动时更新窗口标志
        
        pugi::xml_document              m_xmlTemplate;
        ISkinObj*                       m_pSkinDivider;
        int                             m_nDividerSize;
    };
}