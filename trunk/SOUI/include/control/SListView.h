#pragma once

#include "core/Swnd.h"
#include "interface/Adapter-i.h"

namespace SOUI
{
    interface IListViewItemLocator : public IObjRef
    {
        virtual int GetItemHeight(IAdapter *adapter,int iItem) PURE;
        virtual int GetTotalHeight(IAdapter *adapter) PURE;
        virtual int Item2Position(IAdapter *adapter,int iItem) PURE;
        virtual int Position2Item(IAdapter *adapter,int position,bool bTop) PURE;
        virtual int GetScrollLineSize() const PURE;
    };
    
    class SOUI_EXP SListViewItemLocatorFix : public TObjRefImpl<IListViewItemLocator>
    {
    public:
        SListViewItemLocatorFix(int nItemHei):m_nItemHeight(nItemHei){}

        virtual int GetItemHeight(IAdapter *adapter,int iItem){
            return m_nItemHeight;
        }
        virtual int GetTotalHeight(IAdapter *adapter)
        {
            if(!adapter) return 0;
            return m_nItemHeight * adapter->getCount();
        }
        virtual int Item2Position(IAdapter *adapter,int iItem)
        {
            return iItem * m_nItemHeight;
        }

        virtual int Position2Item(IAdapter *adapter,int position,bool bTop)
        {
            if(!adapter) return -1;
            int nRet = (position+(bTop?0:(m_nItemHeight-1)))/m_nItemHeight;

            if(nRet<0) nRet =0;
            if(nRet>adapter->getCount()) nRet = adapter->getCount();
            return nRet;
        }

        virtual int GetScrollLineSize() const 
        {
            return m_nItemHeight;
        }        
    protected:
        int m_nItemHeight;
    };
    
    class SOUI_EXP EventListViewSelChanged : public TplEventArgs<EventListViewSelChanged>
    {
        SOUI_CLASS_NAME(EventListViewSelChanged,L"on_listview_select_changed")
    public:
        EventListViewSelChanged(SObject *pSender):TplEventArgs<EventListViewSelChanged>(pSender){}
        enum{EventID=EVT_LV_SELCHANGED};
        
        int iOldSel;
        int iNewSel;
    };

    class SOUI_EXP EventListViewItemClick : public TplEventArgs<EventListViewItemClick>
    {
        SOUI_CLASS_NAME(EventListViewItemClick,L"on_listview_item_click")
    public:
        EventListViewItemClick(SObject *pSender):TplEventArgs<EventListViewItemClick>(pSender){}
        enum{EventID=EVT_LV_ITEMCLICK};

        int iClick;
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
        void SetItemLocator(IListViewItemLocator *pItemLocator);
        void EnsureVisible( int iItem );
        
        void SetSel(int iItem,BOOL bNotify=FALSE);
        int  GetSel()const{return m_iSelItem;}
        
        pugi::xml_node GetTemplate();
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
    protected:
        void UpdateScrollBar();
        void RedrawItem(SItemPanel *pItem);
        SItemPanel * HitTest(CPoint & pt);
        SItemPanel * GetItemPanel(int iItem);
        
        void UpdateVisibleItems(int minOld,int maxOld,int minNew,int maxNew);
        
        void RemoveVisibleItems(int nItems,bool bHeader);
        void AddVisibleItems(int iItem1,int iItem2,bool bHeader);
        
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

        HRESULT OnAttrItemHeight(const SStringW &strValue,BOOL bLoading);

    protected:
        CAutoRefPtr<IAdapter>           m_adapter;
        CAutoRefPtr<IDataSetObserver>   m_observer;
        CAutoRefPtr<IListViewItemLocator>  m_lvItemLocator;//列表项定位接口
        struct ItemInfo
        {
            SItemPanel *pItem;
            int nType;
        };
        
        SList<ItemInfo>                 m_lstItems; //当前正在显示的项
        SItemPanel*                     m_itemCapture;//The item panel that has been set capture.
        
        int                             m_iSelItem;
        SItemPanel*                     m_pHoverItem;
        
        SArray<SList<SItemPanel*> *>    m_itemRecycle;//item回收站,每一种样式在回收站中保持一个列表，以便重复利用
        
        BOOL                            m_bScrollUpdate; //滚动时更新窗口标志
        
        pugi::xml_document              m_xmlTemplate;
    };
}