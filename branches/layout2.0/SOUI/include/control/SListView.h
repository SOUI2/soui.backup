#pragma once

#include "core/Swnd.h"
#include "interface/Adapter-i.h"

namespace SOUI
{
    interface IListViewItemLocator : public IObjRef
    {
        virtual bool IsFixHeight() const PURE;
        virtual int GetItemHeight(IAdapter *adapter,int iItem)PURE;
        virtual int GetTotalHeight(IAdapter *adapter)PURE;
        virtual int Item2Position(IAdapter *adapter,int iItem) PURE;
        virtual int Position2Item(IAdapter *adapter,int position,bool bTop)PURE;
        virtual void SetItemHeight(int iItem,int nHeight)PURE;
    };
    
    class SOUI_EXP SListView : public SPanel
        , public IItemContainer
    {
        SOUI_CLASS_NAME(SListView,L"listview")

        friend class SListViewDataSetObserver;
    public:
        SListView();
        ~SListView();

        void SetAdapter(IAdapter * adapter);

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
        
    protected:
        void UpdateScrollBar();
        void RedrawItem(SItemPanel *pItem);
        SItemPanel * HitTest(CPoint & pt);
        
        void UpdateVisibleItems(int minOld,int maxOld,int minNew,int maxNew);
        
        void RemoveVisibleItems(int nItems,bool bHeader);
        void AddVisibleItems(int iItem1,int iItem2,bool bHeader);
        
        void OnPaint(IRenderTarget *pRT);
        void OnSize(UINT nType, CSize size);
        
        LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

        LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );

        void OnMouseLeave();

        BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_SIZE(OnSize)
            MSG_WM_MOUSEWHEEL(OnMouseWheel)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        SOUI_MSG_MAP_END()
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
        
    };
}