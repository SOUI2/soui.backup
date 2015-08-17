#include "souistd.h"
#include "control/SListView.h"
#include <math.h>

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
    // SListViewItemLocatorFix
    SListViewItemLocatorFix::SListViewItemLocatorFix(int nItemHei,int nDividerSize) 
        :m_nItemHeight(nItemHei)
        ,m_nDividerSize(nDividerSize)
    {

    }

    int SListViewItemLocatorFix::GetScrollLineSize() const
    {
        return GetFixItemHeight();
    }

    int SListViewItemLocatorFix::Position2Item(int position)
    {
        if(!m_adapter) return -1;
        int nRet = position/GetFixItemHeight();

        if(nRet<0) nRet =0;
        if(nRet>m_adapter->getCount()) nRet = m_adapter->getCount();
        return nRet;
    }

    int SListViewItemLocatorFix::Item2Position(int iItem)
    {
        return iItem * GetFixItemHeight();
    }

    int SListViewItemLocatorFix::GetTotalHeight()
    {
        if(!m_adapter || m_adapter->getCount() == 0) return 0;
        return m_nItemHeight * m_adapter->getCount() + (m_adapter->getCount()-1)*m_nDividerSize;
    }

    void SListViewItemLocatorFix::SetItemHeight(int iItem,int nHeight)
    {
    }

    int SListViewItemLocatorFix::GetItemHeight(int iItem) const
    {
        return m_nItemHeight;
    }

    bool SListViewItemLocatorFix::IsFixHeight() const
    {
        return true;
    }

    void SListViewItemLocatorFix::SetAdapter(IAdapter *pAdapter)
    {
        m_adapter = pAdapter;
    }


    //////////////////////////////////////////////////////////////////////////
    //  SListViewItemLocatorFlex
    double logbase(double a, double base)
    {
        return log(a) / log(base);
    }

#define SEGMENT_SIZE    50  //数据分组最大长度
#define INDEX_WIDTH     10  //索引表一级最大节点数

    SListViewItemLocatorFlex::SListViewItemLocatorFlex(int nItemHei,int nDividerSize) 
        :m_nItemHeight(nItemHei)
        ,m_nDividerSize(nDividerSize)
    {

    }

    SListViewItemLocatorFlex::~SListViewItemLocatorFlex()
    {
        Clear();
    }

    int SListViewItemLocatorFlex::GetScrollLineSize() const
    {
        return GetFixItemHeight();
    }

    int SListViewItemLocatorFlex::Position2Item(int position)
    {
        if(!m_adapter) return -1;
        if(position<0 || position>=GetTotalHeight()) 
            return -1;
        HSTREEITEM hItem = Offset2Branch(STVI_ROOT,position);
        SASSERT(hItem);
        int idx = Branch2Index(hItem);
        int offset = Branch2Offset(hItem);
        BranchInfo &bi = m_itemPosIndex.GetItemRef(hItem);
        SASSERT(bi.nBranchHei>=position-offset);

        int iSeg = idx/SEGMENT_SIZE;
        int nRemain = position - offset;

        SegmentInfo *psi = m_segments[iSeg];

        for(int i=0;i<psi->nItems;i++)
        {
            int nItemHei = psi->pItemHeight[i]==-1?GetFixItemHeight():psi->pItemHeight[i];
            if(nRemain<=nItemHei) return idx + i;
            nRemain -= nItemHei;
        }

        SASSERT(FALSE);
        return -1;
    }

    int SListViewItemLocatorFlex::Item2Position(int iItem)
    {
        if(!m_adapter) return 0;
        int iSeg = iItem/SEGMENT_SIZE;
        int iSubItem = iItem%SEGMENT_SIZE;
        SegmentInfo *psi = m_segments[iSeg];

        int nPos = Branch2Offset(psi->hItem);
        for(int i=0;i<iSubItem;i++)
        {
            nPos += psi->pItemHeight[i]==-1?GetFixItemHeight():psi->pItemHeight[i];
        }
        return nPos;
    }

    int SListViewItemLocatorFlex::GetTotalHeight()
    {
        if(!m_adapter) return 0;
        HSTREEITEM hItem = m_itemPosIndex.GetRootItem();
        int nRet = m_itemPosIndex.GetItem(hItem).nBranchHei;
        if(m_adapter->getCount()>0) nRet -= m_nDividerSize;
        return nRet;
    }

    void SListViewItemLocatorFlex::SetItemHeight(int iItem,int nHeight)
    {
        if(!m_adapter) return;

        int iSeg = iItem/SEGMENT_SIZE;
        int iSubItem = iItem%SEGMENT_SIZE;
        SegmentInfo *psi = m_segments[iSeg];
        int nOldHei = psi->pItemHeight[iSubItem];
        if(nOldHei==-1) nOldHei = GetFixItemHeight();

        nHeight += m_nDividerSize;
        psi->pItemHeight[iSubItem] = nHeight;
        if(nOldHei != nHeight)
        {
            int nHeiDif = nHeight - nOldHei;
            HSTREEITEM hBranch = psi->hItem;
            while(hBranch)
            {
                BranchInfo & bi = m_itemPosIndex.GetItemRef(hBranch);
                bi.nBranchHei += nHeiDif;
                hBranch = m_itemPosIndex.GetParentItem(hBranch);
            }
        }
    }

    int SListViewItemLocatorFlex::GetItemHeight(int iItem) const
    {
        if(!m_adapter) return 0;
        int iSeg = iItem/SEGMENT_SIZE;
        int iSubItem = iItem%SEGMENT_SIZE;
        SegmentInfo *psi = m_segments[iSeg];
        int nRet = psi->pItemHeight[iSubItem];
        if(nRet == -1) nRet = GetFixItemHeight();
        nRet -= m_nDividerSize;
        return nRet;
    }

    bool SListViewItemLocatorFlex::IsFixHeight() const
    {
        return false;
    }

    void SListViewItemLocatorFlex::SetAdapter(IAdapter *pAdapter)
    {
        m_adapter = pAdapter;
        OnDataSetChanged();
    }

    void SListViewItemLocatorFlex::OnDataSetChanged()
    {
        Clear();
        if(m_adapter)
        {
            int nTreeSize =  m_adapter->getCount();
            int nBranchSize = SEGMENT_SIZE;
            int nTreeDeep = GetIndexDeep();
            for(int i=0;i<nTreeDeep;i++)
                nBranchSize *= INDEX_WIDTH;
            InitIndex(STVI_ROOT,nTreeSize,nBranchSize);
        }
    }

    void SListViewItemLocatorFlex::InitIndex(HSTREEITEM hParent,int nItems,int nBranchSize)
    {
        BranchInfo bi;
        bi.nBranchHei = nItems*GetFixItemHeight();
        bi.nBranchSize = nItems;

        HSTREEITEM hBranch = m_itemPosIndex.InsertItem(bi,hParent);
        if(nItems > SEGMENT_SIZE)
        {//插入子节点
            int nRemain = nItems;
            int nSubBranchSize = nBranchSize/INDEX_WIDTH;
            while(nRemain>0)
            {
                int nItems2 = nSubBranchSize;
                if(nItems2>nRemain) nItems2 = nRemain;
                InitIndex(hBranch,nItems2,nSubBranchSize);
                nRemain -= nItems2;
            }
        }else
        {
            m_segments.Add(new SegmentInfo(nItems,hBranch));
        }
    }

    int SListViewItemLocatorFlex::GetIndexDeep() const
    {
        if(!m_adapter) return 0;
        if(m_adapter->getCount()==0) return 0;
        return (int)ceil(logbase((m_adapter->getCount()+SEGMENT_SIZE-1)/SEGMENT_SIZE,INDEX_WIDTH));
    }

    void SListViewItemLocatorFlex::Clear()
    {
        m_itemPosIndex.DeleteAllItems();
        for(int i=0;i<(int)m_segments.GetCount();i++)
        {
            delete m_segments[i];
        }
        m_segments.RemoveAll();
    }

    int SListViewItemLocatorFlex::Branch2Offset(HSTREEITEM hBranch) const
    {
        int nOffset = 0;
        HSTREEITEM hPrev = m_itemPosIndex.GetPrevSiblingItem(hBranch);
        while(hPrev)
        {
            nOffset += m_itemPosIndex.GetItem(hPrev).nBranchHei;
            hPrev = m_itemPosIndex.GetPrevSiblingItem(hPrev);
        }
        HSTREEITEM hParent = m_itemPosIndex.GetParentItem(hBranch);
        if(hParent)
        {
            nOffset += Branch2Offset(hParent);
        }
        return nOffset;
    }

    int SListViewItemLocatorFlex::Branch2Index(HSTREEITEM hBranch) const
    {
        int iIndex = 0;
        HSTREEITEM hPrev = m_itemPosIndex.GetPrevSiblingItem(hBranch);
        while(hPrev)
        {
            iIndex += m_itemPosIndex.GetItem(hPrev).nBranchSize;
            hPrev = m_itemPosIndex.GetPrevSiblingItem(hPrev);
        }
        HSTREEITEM hParent = m_itemPosIndex.GetParentItem(hBranch);
        if(hParent)
        {
            iIndex += Branch2Index(hParent);
        }
        return iIndex;
    }

    HSTREEITEM SListViewItemLocatorFlex::Offset2Branch(HSTREEITEM hParent,int nOffset)
    {
        HSTREEITEM hItem = m_itemPosIndex.GetChildItem(hParent);
        if(!hItem) return hParent;

        while(hItem)
        {
            BranchInfo bi = m_itemPosIndex.GetItem(hItem);
            if(nOffset>bi.nBranchHei)
            {
                nOffset -= bi.nBranchHei;
                hItem = m_itemPosIndex.GetNextSiblingItem(hItem);
            }else
            {
                return Offset2Branch(hItem,nOffset);
            }
        }
        return NULL;
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
    {
        m_bFocusable = TRUE;
        m_observer.Attach(new SListViewDataSetObserver(this));

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
            if(dwTime-dwTime1>50 && m_bScrollUpdate)
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
                if(iNewLastVisible>=iOldLastVisible && iNewLastVisible < iOldLastVisible)
                {//use the old visible item
                    int iItem = iNewLastVisible-(iNewFirstVisible-iOldFirstVisible);
                    SASSERT(iItem>=0 && iItem <= (iOldLastVisible-iOldFirstVisible));
                    m_lstItems.AddTail(pItemInfos[iItem]);
                    pos += m_lvItemLocator->GetItemHeight(iNewLastVisible);
                    pItemInfos[iItem].pItem = NULL;//标记该行已经被重用
                }else
                {//create new visible item
                    int nItemType = m_adapter->getItemViewType(iNewLastVisible);
                    SList<SItemPanel *> *lstRecycle = m_itemRecycle.GetAt(nItemType);

                    SItemPanel * pItemPanel = NULL;
                    if(lstRecycle->IsEmpty())
                    {//创建一个新的列表项
                        pItemPanel = SItemPanel::Create(this,pugi::xml_node(),this);
                    }else
                    {
                        pItemPanel = lstRecycle->RemoveHead();
                    }
                    pItemPanel->SetItemIndex(iNewLastVisible);

                    CRect rcItem = GetClientRect();
                    rcItem.MoveToXY(0,0);
                    if(m_lvItemLocator->IsFixHeight())
                    {
                        rcItem.bottom=m_lvItemLocator->GetItemHeight(iNewLastVisible);
                        pItemPanel->Move(rcItem);
                    }
                    m_adapter->getView(iNewLastVisible,pItemPanel,m_xmlTemplate.first_child());
                    if(!m_lvItemLocator->IsFixHeight())
                    {
                        rcItem.bottom=0;
                        CSize szItem = pItemPanel->GetDesiredSize(rcItem);
                        rcItem.bottom = rcItem.top + szItem.cy;
                        pItemPanel->Move(rcItem);
                        m_lvItemLocator->SetItemHeight(iNewLastVisible,szItem.cy);
                    }                
                    pItemPanel->UpdateChildrenPosition();
                    if(iNewLastVisible == m_iSelItem)
                    {
                        pItemPanel->ModifyItemState(WndState_Check,0);
                    }
                    ItemInfo ii;
                    ii.nType = nItemType;
                    ii.pItem = pItemPanel;
                    m_lstItems.AddTail(ii);
                    pos += rcItem.bottom + m_lvItemLocator->GetDividerSize();
                }
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
        if(uMsg == WM_LBUTTONUP)
            __super::OnLButtonUp(wParam,pt);

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
        else if (pOwner && nChar == VK_RETURN)
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
        }
        m_bScrollUpdate=TRUE;
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
            if((int)ii.pItem->GetItemIndex() == m_iSelItem)
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
}