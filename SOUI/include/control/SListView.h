#pragma once

#include "core/Swnd.h"
#include "interface/Adapter-i.h"
#include "interface/LvItemLocator-i.h"

namespace SOUI
{
    
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
        virtual UINT OnGetDlgCode();
    protected:
        void _SetSel(int iItem,BOOL bNotify, SWND hHitWnd);

        void UpdateScrollBar();
        void RedrawItem(SItemPanel *pItem);
        SItemPanel * HitTest(CPoint & pt);
        SItemPanel * GetItemPanel(int iItem);
        
        void UpdateVisibleItems();
        
        void OnPaint(IRenderTarget *pRT);
        void OnSize(UINT nType, CSize size);
        void OnDestroy();

        LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

        LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );
        void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

        void OnMouseLeave();

        BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        
        void OnKillFocus();
        
        void OnSetFocus();

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_SIZE(OnSize)
            MSG_WM_DESTROY(OnDestroy)
            MSG_WM_MOUSEWHEEL(OnMouseWheel)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_KEYDOWN(OnKeyDown)
            MSG_WM_KILLFOCUS_EX(OnKillFocus)
            MSG_WM_SETFOCUS_EX(OnSetFocus)
            MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
            MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        SOUI_MSG_MAP_END()

        SOUI_ATTRS_BEGIN()
            ATTR_SKIN(L"dividerSkin",m_pSkinDivider,TRUE)
            ATTR_INT(L"dividerSize",m_nDividerSize,FALSE)
            ATTR_INT(L"wantTab",m_bWantTab,FALSE)
        SOUI_ATTRS_END()
    protected:
        CAutoRefPtr<IAdapter>           m_adapter;
        CAutoRefPtr<IDataSetObserver>   m_observer;
        CAutoRefPtr<IListViewItemLocator>  m_lvItemLocator;//�б��λ�ӿ�
        struct ItemInfo
        {
            SItemPanel *pItem;
            int nType;
        };
        
        int                             m_iFirstVisible;//��һ����ʾ������
        SList<ItemInfo>                 m_lstItems; //��ǰ������ʾ����
        SItemPanel*                     m_itemCapture;//The item panel that has been set capture.
        
        int                             m_iSelItem;
        SItemPanel*                     m_pHoverItem;
        
        SArray<SList<SItemPanel*> *>    m_itemRecycle;//item����վ,ÿһ����ʽ�ڻ���վ�б���һ���б����Ա��ظ�����
                
        pugi::xml_document              m_xmlTemplate;
        ISkinObj*                       m_pSkinDivider;
        int                             m_nDividerSize;
        BOOL                            m_bWantTab;
    };
}