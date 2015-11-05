#include "souistd.h"
#include "control/SListView.h"
#include "helper/SListViewItemLocator.h"

namespace SOUI
{


    class SListViewDataSetObserver : public TObjRefImpl<IDataSetObserver>
    {
    public:
        SListViewDataSetObserver(SListView *pView):m_pOwner(pView)
        {
        }
        virtual void onChanged();
        virtual void onInvalidated();

    protected:
        SListView * m_pOwner;
    };

    //////////////////////////////////////////////////////////////////////////
    void SListViewDataSetObserver::onChanged()
    {
        m_pOwner->onDataSetChanged();
    }

    void SListViewDataSetObserver::onInvalidated()
    {
        m_pOwner->onDataSetInvalidated();
    }



    //////////////////////////////////////////////////////////////////////////
    SListView::SListView()
        :m_iSelItem(-1)
        ,m_iFirstVisible(-1)
        ,m_pHoverItem(NULL)
        ,m_itemCapture(NULL)
        ,m_bScrollUpdate(TRUE)
        ,m_pSkinDivider(NULL)
        ,m_nDividerSize(0)
        ,m_bWantTab(FALSE)
    {
        m_bFocusable = TRUE;
        m_observer.Attach(new SListViewDataSetObserver(this));
        m_dwUpdateInterval= 40;
        m_evtSet.addEvent(EVENTID(EventLVSelChanged));
    }

    SListView::~SListView()
    {
        m_observer=NULL;
        m_lvItemLocator=NULL;
    }

    BOOL SListView::SetAdapter(IAdapter * adapter)
    {
        if(!m_lvItemLocator)
        {
            SASSERT_FMT(FALSE,_T("error: A item locator is in need before setting adapter!!!"));
            return FALSE;
        }

        if(m_adapter)
        {
            m_adapter->unregisterDataSetObserver(m_observer);

            //free all itemPanels in recycle
            for(size_t i=0;i<m_itemRecycle.GetCount();i++)
            {
                SList<SItemPanel*> *lstItemPanels = m_itemRecycle.GetAt(i);
                SPOSITION pos = lstItemPanels->GetHeadPosition();
                while(pos)
                {
                    SItemPanel * pItemPanel = lstItemPanels->GetNext(pos);
                    pItemPanel->DestroyWindow();
                }
                delete lstItemPanels;
            }
            m_itemRecycle.RemoveAll();

            //free all visible itemPanels
            SPOSITION pos=m_lstItems.GetHeadPosition();
            while(pos)
            {
                ItemInfo ii = m_lstItems.GetNext(pos);
                ii.pItem->DestroyWindow();
            }
            m_lstItems.RemoveAll();
        }

        m_adapter = adapter;
        if(m_lvItemLocator)
            m_lvItemLocator->SetAdapter(adapter);
        if(m_adapter) 
        {
            m_adapter->registerDataSetObserver(m_observer);
            for(int i=0;i<m_adapter->getViewTypeCount();i++)
            {
                m_itemRecycle.Add(new SList<SItemPanel*>());
            }
            onDataSetChanged();
        }
        return TRUE;
    }

    void SListView::UpdateScrollBar()
    {
        CRect rcClient=SWindow::GetClientRect();
        CSize size = rcClient.Size();
        CSize szView;
        szView.cx = rcClient.Width();
        szView.cy = m_lvItemLocator?m_lvItemLocator->GetTotalHeight():0;

        //  关闭滚动条
        m_wBarVisible = SSB_NULL;

        if (size.cy<szView.cy )
        {
            //  需要纵向滚动条
            m_wBarVisible |= SSB_VERT;
            m_siVer.nMin  = 0;
            m_siVer.nMax  = szView.cy-1;
            m_siVer.nPage = size.cy;
            m_siVer.nPos = min(m_siVer.nPos,m_siVer.nMax-(int)m_siVer.nPage);
        }
        else
        {
            //  不需要纵向滚动条
            m_siVer.nPage = size.cy;
            m_siVer.nMin  = 0;
            m_siVer.nMax  = size.cy-1;
            m_siVer.nPos  = 0;
        }

        SetScrollPos(TRUE, m_siVer.nPos, FALSE);

        //  重新计算客户区及非客户区
        SSendMessage(WM_NCCALCSIZE);

        InvalidateRect(NULL);
    }

    void SListView::onDataSetChanged()
    {
        if(!m_adapter) return;
        if(m_lvItemLocator) m_lvItemLocator->OnDataSetChanged();
        UpdateScrollBar();
        UpdateVisibleItems();
    }

    void SListView::onDataSetInvalidated()
    {
        UpdateVisibleItems();
    }

    void SListView::OnPaint(IRenderTarget *pRT)
    {
        SPainter duiDC;
        BeforePaint(pRT,duiDC);


        int iFirst = m_iFirstVisible;
        if(iFirst!=-1)
        {
            CRect rcClient;
            GetClientRect(&rcClient);
            pRT->PushClipRect(&rcClient,RGN_AND);

            CRect rcClip,rcInter;
            pRT->GetClipBox(&rcClip);

            int nOffset = m_lvItemLocator->Item2Position(iFirst)-m_siVer.nPos;

            CRect rcItem(rcClient);
            rcItem.bottom = rcItem.top + nOffset;

            SPOSITION pos= m_lstItems.GetHeadPosition();
            int i=0;
            for(;pos;i++)
            {
                ItemInfo ii = m_lstItems.GetNext(pos);
                rcItem.top=rcItem.bottom;
                rcItem.bottom = rcItem.top + m_lvItemLocator->GetItemHeight(iFirst+i);
                rcInter.IntersectRect(&rcClip,&rcItem);
                if(!rcInter.IsRectEmpty())
                    ii.pItem->Draw(pRT,rcItem);
                rcItem.top = rcItem.bottom;
                rcItem.bottom += m_lvItemLocator->GetDividerSize();
                if(m_pSkinDivider)
                {//绘制分隔线
                    m_pSkinDivider->Draw(pRT,rcItem,0);
                }
            }

            pRT->PopClip();
        }
        AfterPaint(pRT,duiDC);
    }

    BOOL SListView::OnScroll(BOOL bVertical,UINT uCode,int nPos)
    {
        int nOldPos = m_siVer.nPos;
        __super::OnScroll(bVertical, uCode, nPos);
        int nNewPos = m_siVer.nPos;
        if(nOldPos != nNewPos)
        {
            UpdateVisibleItems();

            //加速滚动时UI的刷新
            static DWORD dwTime1=0;
            DWORD dwTime=GetTickCount();
            if(dwTime-dwTime1>=m_dwUpdateInterval && m_bScrollUpdate)
            {
                UpdateWindow();
                dwTime1=dwTime;
            }

        }
        return TRUE;
    }


    void SListView::UpdateVisibleItems()
    {
        if(!m_adapter) return;
        int iOldFirstVisible = m_iFirstVisible;
        int iOldLastVisible = m_iFirstVisible + m_lstItems.GetCount();
        int nOldTotalHeight = m_lvItemLocator->GetTotalHeight();

        int iNewFirstVisible = m_lvItemLocator->Position2Item(m_siVer.nPos);
        int iNewLastVisible = iNewFirstVisible;
        int pos = m_lvItemLocator->Item2Position(iNewFirstVisible);


        ItemInfo *pItemInfos = new ItemInfo[m_lstItems.GetCount()];
        SPOSITION spos = m_lstItems.GetHeadPosition();
        int i=0;
        while(spos)
        {
            pItemInfos[i++]=m_lstItems.GetNext(spos);
        }

        m_lstItems.RemoveAll();

        if(iNewFirstVisible!=-1)
        {
            while(pos < m_siVer.nPos + (int)m_siVer.nPage && iNewLastVisible < m_adapter->getCount())
            {
                ItemInfo ii={NULL,-1};
                if(iNewLastVisible>=iOldFirstVisible && iNewLastVisible < iOldLastVisible)
                {//use the old visible item
                    int iItem = iNewLastVisible-iOldFirstVisible;//(iNewLastVisible-iNewFirstVisible) + (iNewFirstVisible-iOldFirstVisible);
                    SASSERT(iItem>=0 && iItem <= (iOldLastVisible-iOldFirstVisible));
                    ii = pItemInfos[iItem];
                    pItemInfos[iItem].pItem = NULL;//标记该行已经被重用
                }else
                {//create new visible item
                    ii.nType = m_adapter->getItemViewType(iNewLastVisible);
                    SList<SItemPanel *> *lstRecycle = m_itemRecycle.GetAt(ii.nType);
                    if(lstRecycle->IsEmpty())
                    {//创建一个新的列表项
                        ii.pItem = SItemPanel::Create(this,pugi::xml_node(),this);
                    }else
                    {
                        ii.pItem = lstRecycle->RemoveHead();
                    }
                    ii.pItem->SetItemIndex(iNewLastVisible);
                }
                CRect rcItem = GetClientRect();
                rcItem.MoveToXY(0,0);
                if(m_lvItemLocator->IsFixHeight())
                {
                    rcItem.bottom=m_lvItemLocator->GetItemHeight(iNewLastVisible);
                    ii.pItem->Move(rcItem);
                }
                m_adapter->getView(iNewLastVisible,ii.pItem,m_xmlTemplate.first_child());
                if(!m_lvItemLocator->IsFixHeight())
                {
                    rcItem.bottom=0;
                    CSize szItem = ii.pItem->GetDesiredSize(rcItem);
                    rcItem.bottom = rcItem.top + szItem.cy;
                    ii.pItem->Move(rcItem);
                    m_lvItemLocator->SetItemHeight(iNewLastVisible,szItem.cy);
                }                
                ii.pItem->UpdateLayout();
                if(iNewLastVisible == m_iSelItem)
                {
                    ii.pItem->ModifyItemState(WndState_Check,0);
                }
                
                m_lstItems.AddTail(ii);
                pos += rcItem.bottom + m_lvItemLocator->GetDividerSize();

                iNewLastVisible ++;
            }
        }

        //move old visible items which were not reused to recycle
        for(int i=0;i<(iOldLastVisible-iOldFirstVisible);i++)
        {
            ItemInfo ii = pItemInfos[i];
            if(!ii.pItem) continue;

            if(ii.pItem == m_pHoverItem)
            {
                m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE,0,0);
                m_pHoverItem=NULL;
            }
            if(ii.pItem->GetItemIndex() == m_iSelItem)
            {
                ii.pItem->ModifyItemState(0,WndState_Check);
                ii.pItem->GetFocusManager()->SetFocusedHwnd(0);
            }
            m_itemRecycle[ii.nType]->AddTail(ii.pItem);    
        }
        delete [] pItemInfos;

        m_iFirstVisible = iNewFirstVisible;

        if(!m_lvItemLocator->IsFixHeight() && m_lvItemLocator->GetTotalHeight() != nOldTotalHeight)
        {//update scroll range
            UpdateScrollBar();
            UpdateVisibleItems();//根据新的滚动条状态重新记录显示列表项
        }
    }

    void SListView::OnSize(UINT nType, CSize size)
    {
        __super::OnSize(nType,size);
        UpdateScrollBar();

        //update item window
        CRect rcClient=GetClientRect();
        SPOSITION pos = m_lstItems.GetHeadPosition();
        while(pos)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            int idx = (int)ii.pItem->GetItemIndex();
            int nHei = m_lvItemLocator->GetItemHeight(idx);
            CRect rcItem(0,0,rcClient.Width(),nHei);
            ii.pItem->Move(rcItem);
        }

        UpdateVisibleItems();
    }

    void SListView::OnDestroy()
    {
        //destroy all itempanel
        SPOSITION pos = m_lstItems.GetHeadPosition();
        while(pos)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            ii.pItem->Release();
        }
        m_lstItems.RemoveAll();

        for(int i=0;i<(int)m_itemRecycle.GetCount();i++)
        {
            SList<SItemPanel*> *pLstTypeItems = m_itemRecycle[i];
            SPOSITION pos = pLstTypeItems->GetHeadPosition();
            while(pos)
            {
                SItemPanel *pItem = pLstTypeItems->GetNext(pos);
                pItem->Release();
            }
            delete pLstTypeItems;
        }
        m_itemRecycle.RemoveAll();
        __super::OnDestroy();
    }


    //////////////////////////////////////////////////////////////////////////
    void SListView::OnItemRequestRelayout(SItemPanel *pItem)
    {
        pItem->UpdateChildrenPosition();
    }

    BOOL SListView::IsItemRedrawDelay()
    {
        return TRUE;
    }

    BOOL SListView::OnItemGetRect(SItemPanel *pItem,CRect &rcItem)
    {
        int iPosition = (int)pItem->GetItemIndex();
        int nOffset = m_lvItemLocator->Item2Position(iPosition)-m_siVer.nPos;
        rcItem = GetClientRect();
        rcItem.top += nOffset;
        rcItem.bottom = rcItem.top + m_lvItemLocator->GetItemHeight(iPosition);
        return TRUE;
    }

    void SListView::OnItemSetCapture(SItemPanel *pItem,BOOL bCapture)
    {
        if(bCapture)
        {
            GetContainer()->OnSetSwndCapture(m_swnd);
            m_itemCapture=pItem;
        }else
        {
            GetContainer()->OnReleaseSwndCapture();
            m_itemCapture=NULL;
        }
    }

    void SListView::RedrawItem(SItemPanel *pItem)
    {
        pItem->InvalidateRect(NULL);
    }

    SItemPanel * SListView::HitTest(CPoint & pt)
    {
        SPOSITION pos = m_lstItems.GetHeadPosition();
        while(pos)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            CRect rcItem = ii.pItem->GetItemRect();
            if(rcItem.PtInRect(pt)) 
            {
                pt-=rcItem.TopLeft();
                return ii.pItem;
            }
        }
        return NULL;
    }

    LRESULT SListView::OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam)
    {
        if(!m_adapter)
        {
            SetMsgHandled(FALSE);
            return 0;
        }

        LRESULT lRet=0;
        CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        if(uMsg == WM_LBUTTONDOWN)
            __super::OnLButtonDown(wParam,pt);

        if(m_itemCapture)
        {
            CRect rcItem=m_itemCapture->GetItemRect();
            pt.Offset(-rcItem.TopLeft());
            lRet = m_itemCapture->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
        }
        else
        {
            if(m_bFocusable && (uMsg==WM_LBUTTONDOWN || uMsg== WM_RBUTTONDOWN || uMsg==WM_LBUTTONDBLCLK))
                SetFocus();

            SItemPanel * pHover=HitTest(pt);
            if(pHover!=m_pHoverItem)
            {
                SItemPanel * nOldHover=m_pHoverItem;
                m_pHoverItem=pHover;
                if(nOldHover)
                {
                    nOldHover->DoFrameEvent(WM_MOUSELEAVE,0,0);
                    RedrawItem(nOldHover);
                }
                if(m_pHoverItem)
                {
                    m_pHoverItem->DoFrameEvent(WM_MOUSEHOVER,wParam,MAKELPARAM(pt.x,pt.y));
                    RedrawItem(m_pHoverItem);
                }
            }
            if(uMsg==WM_LBUTTONDOWN )
            {//选择一个新行的时候原有行失去焦点
                SWND hHitWnd = 0;
                int nSelNew = -1;
                if(m_pHoverItem)
                {
                    nSelNew = m_pHoverItem->GetItemIndex();
                    hHitWnd = m_pHoverItem->SwndFromPoint(pt,FALSE);
                }

                _SetSel(nSelNew,TRUE,hHitWnd);
            }
            if(m_pHoverItem)
            {
                m_pHoverItem->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
            }
        }
        
        CPoint pt2(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if(uMsg == WM_LBUTTONUP)
            __super::OnLButtonUp(wParam,pt2);
        else if(uMsg == WM_RBUTTONDOWN)
            __super::OnRButtonDown(uMsg, pt2);

        return 0;
    }

    LRESULT SListView::OnKeyEvent(UINT uMsg,WPARAM wParam,LPARAM lParam)
    {
        LRESULT lRet=0;
        SItemPanel *pItem = GetItemPanel(m_iSelItem);
        if(pItem)
        {
            lRet=pItem->DoFrameEvent(uMsg,wParam,lParam);
            SetMsgHandled(pItem->IsMsgHandled());
        }else
        {
            SetMsgHandled(FALSE);
        }
        return lRet;
    }

    void SListView::OnMouseLeave()
    {
        if(m_pHoverItem)
        {
            m_pHoverItem->DoFrameEvent(WM_MOUSELEAVE,0,0);
            m_pHoverItem = NULL;
        }

    }

    void SListView::OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
    {
        if(!m_adapter)
        {
            SetMsgHandled(FALSE);
            return;
        }
        
        if(m_iSelItem!=-1 && m_bWantTab)
        {
            SItemPanel *pItem = GetItemPanel(m_iSelItem);
            if(pItem)
            {
                pItem->DoFrameEvent(WM_KEYDOWN,nChar, MAKELONG(nFlags, nRepCnt));
                if(pItem->IsMsgHandled()) return;
            }
        }
        
        int  nNewSelItem = -1;
        SWindow *pOwner = GetOwner();
        if (pOwner && (nChar == VK_ESCAPE))
        {
            pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
            return;
        }

        m_bScrollUpdate=FALSE;
        if (nChar == VK_DOWN && m_iSelItem < m_adapter->getCount() - 1)
            nNewSelItem = m_iSelItem+1;
        else if (nChar == VK_UP && m_iSelItem > 0)
            nNewSelItem = m_iSelItem-1;
        else if (pOwner && nChar == VK_RETURN)//提供combobox响应回车选中
            nNewSelItem = m_iSelItem;
        else if(nChar == VK_PRIOR)
        {
            OnScroll(TRUE,SB_PAGEUP,0);
            if(!m_lstItems.IsEmpty())
            {
                nNewSelItem = m_lstItems.GetHead().pItem->GetItemIndex();
            }
        }else if(nChar == VK_NEXT)
        {
            OnScroll(TRUE,SB_PAGEDOWN,0);
            if(!m_lstItems.IsEmpty())
            {
                nNewSelItem = m_lstItems.GetTail().pItem->GetItemIndex();
            }
        }

        if(nNewSelItem!=-1)
        {
            EnsureVisible(nNewSelItem);
            SetSel(nNewSelItem);
            m_bScrollUpdate=TRUE;
        }
    }

    void SListView::EnsureVisible( int iItem )
    {
        if(iItem<0 || iItem>=m_adapter->getCount()) return;

        int iFirstVisible= m_iFirstVisible;
        int iLastVisible = m_iFirstVisible + m_lstItems.GetCount();

        if(iItem>=iFirstVisible && iItem<iLastVisible)
            return;

        int pos = m_lvItemLocator->Item2Position(iItem);

        if(iItem < iFirstVisible)
        {//scroll up
            OnScroll(TRUE,SB_THUMBPOSITION,pos);
        }else // if(iItem >= iLastVisible)
        {//scroll down
            int iTop = iItem;
            int pos2 = pos;
            int topSize = m_siVer.nPage - m_lvItemLocator->GetItemHeight(iItem);
            while(iTop>=0 && (pos - pos2) < topSize)
            {
                pos2 = m_lvItemLocator->Item2Position(--iTop);
            }
            OnScroll(TRUE,SB_THUMBPOSITION,pos2);
        }
    }

    BOOL SListView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        SItemPanel *pSelItem = GetItemPanel(m_iSelItem);
        if(pSelItem)
        {
            CRect rcItem = pSelItem->GetItemRect();
            CPoint pt2=pt-rcItem.TopLeft();
            if(pSelItem->DoFrameEvent(WM_MOUSEWHEEL,MAKEWPARAM(nFlags,zDelta),MAKELPARAM(pt2.x,pt2.y)))
                return TRUE;
        }
        return __super::OnMouseWheel(nFlags, zDelta, pt);
    }

    int SListView::GetScrollLineSize(BOOL bVertical)
    {
        return m_lvItemLocator->GetScrollLineSize();
    }

    SItemPanel * SListView::GetItemPanel(int iItem)
    {
        if(iItem<0 || iItem>=m_adapter->getCount()) 
            return NULL; 
        SPOSITION pos = m_lstItems.GetHeadPosition();
        while(pos)
        {
            ItemInfo ii = m_lstItems.GetNext(pos);
            if((int)ii.pItem->GetItemIndex() == iItem)
                return ii.pItem;
        }
        return NULL;
    }

    void SListView::SetSel(int iItem,BOOL bNotify/*=FALSE*/)
    {
        _SetSel(iItem,bNotify,0);
    }

    BOOL SListView::CreateChildren(pugi::xml_node xmlNode)
    {
        pugi::xml_node xmlTemplate = xmlNode.child(L"template");
        if(xmlTemplate)
        {
            m_xmlTemplate.append_copy(xmlTemplate);
            int nItemHei = xmlTemplate.attribute(L"itemHeight").as_int(-1);
            if(nItemHei>0)
            {//指定了itemHeight属性时创建一个固定行高的定位器
                IListViewItemLocator * pItemLocator = new  SListViewItemLocatorFix(nItemHei,m_nDividerSize);
                SetItemLocator(pItemLocator);
                pItemLocator->Release();
            }else
            {//创建一个行高可变的行定位器，从defHeight属性中获取默认行高
                IListViewItemLocator * pItemLocator = new  SListViewItemLocatorFlex(xmlTemplate.attribute(L"defHeight").as_int(30),m_nDividerSize);
                SetItemLocator(pItemLocator);
                pItemLocator->Release();
            }
        }
        return TRUE;
    }

    void SListView::SetItemLocator(IListViewItemLocator *pItemLocator)
    {
        m_lvItemLocator = pItemLocator;
        if(m_lvItemLocator) m_lvItemLocator->SetAdapter(GetAdapter());
        onDataSetChanged();
    }

    BOOL SListView::OnUpdateToolTip(CPoint pt, SwndToolTipInfo & tipInfo)
    {
        if(!m_pHoverItem)
            return __super::OnUpdateToolTip(pt,tipInfo);
        return m_pHoverItem->OnUpdateToolTip(pt,tipInfo);
    }

    void SListView::_SetSel(int iItem,BOOL bNotify, SWND hHitWnd)
    {
        if(!m_adapter) return;

        if(iItem>=m_adapter->getCount())
            return;

        if(iItem<0) iItem = -1;

        int nOldSel = m_iSelItem;
        int nNewSel = iItem;

        m_iSelItem = nNewSel;
        if(bNotify)
        {
            EventLVSelChanged evt(this);
            evt.iOldSel = nOldSel;
            evt.iNewSel = nNewSel;
            evt.hHitWnd =hHitWnd;
            FireEvent(evt);
            if(evt.bCancel) 
            {//Cancel SetSel and restore selection state
                m_iSelItem = nOldSel;
                return;
            }
        }

        if(nOldSel == nNewSel)
            return;

        m_iSelItem = nOldSel;
        SItemPanel *pItem = GetItemPanel(nOldSel);
        if(pItem)
        {
            pItem->GetFocusManager()->SetFocusedHwnd(-1);
            pItem->ModifyItemState(0,WndState_Check);
            RedrawItem(pItem);
        }
        m_iSelItem = nNewSel;
        pItem = GetItemPanel(nNewSel);
        if(pItem)
        {
            pItem->ModifyItemState(WndState_Check,0);
            RedrawItem(pItem);
        }
    }

    UINT SListView::OnGetDlgCode()
    {
        if(m_bWantTab) return SC_WANTALLKEYS;
        else return SC_WANTARROWS|SC_WANTSYSKEY;
    }

    void SListView::OnKillFocus()
    {
        __super::OnKillFocus();
        
        if(m_iSelItem==-1) return;
        
        SItemPanel *pSelPanel = GetItemPanel(m_iSelItem);
        if(pSelPanel) pSelPanel->GetFocusManager()->StoreFocusedView();
    }

    void SListView::OnSetFocus()
    {
        __super::OnSetFocus();
        if(m_iSelItem==-1) return;

        SItemPanel *pSelPanel = GetItemPanel(m_iSelItem);
        if(pSelPanel) pSelPanel->GetFocusManager()->RestoreFocusedView();
    }

}