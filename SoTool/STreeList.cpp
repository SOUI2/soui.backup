#include "stdafx.h"
#include "STreeList.h"

namespace SOUI
{
    SMCTreeCtrl::SMCTreeCtrl():m_nTreeWidth(100),m_nItemWid(-1)
    {
        m_evtSet.addEvent(EVENTID(EventScroll));
    }

    SMCTreeCtrl::~SMCTreeCtrl()
    {

    }

    void SMCTreeCtrl::OnFinalRelease()
    {
        delete this;
    }

    void SMCTreeCtrl::OnNodeFree(LPTVITEM & pItemData)
    {
        MCITEM *pMcItem = (MCITEM*)pItemData->lParam;
        delete pMcItem;
        STreeCtrl::OnNodeFree(pItemData);
    }

    void SMCTreeCtrl::OnInsertItem(LPTVITEM & pItemData)
    {
        MCITEM *pMcItem = new MCITEM;
        pMcItem->arrText.SetCount(m_arrColWidth.GetCount());
        pItemData->lParam = (LPARAM)pMcItem;
    }

    int SMCTreeCtrl::CalcItemWidth(const LPTVITEM pItem)
    {
        if(m_nItemWid<0)
        {
            m_nItemWid = m_nTreeWidth;
            for(int i=0;i<m_arrColWidth.GetCount();i++)
                m_nItemWid += m_arrColWidth[i];
        }
        return m_nItemWid;
    }
    
    BOOL SMCTreeCtrl::OnScroll(BOOL bVertical,UINT uCode,int nPos)
    {
        BOOL bRet = STreeCtrl::OnScroll(bVertical,uCode,nPos);
        if(!bRet) return FALSE;
        EventScroll evt(this);
        evt.bVertical = bVertical;
        evt.uSbCode = uCode;
        evt.nPos = nPos;
        FireEvent(evt);
        return TRUE;
    }

    int SMCTreeCtrl::InsertColumn(int iCol,int nWid)
    {
        if(iCol < 0) iCol = m_arrColWidth.GetCount();
        m_arrColWidth.InsertAt(iCol,nWid);
        
        HSTREEITEM hItem = GetRootItem();
        while(hItem)
        {
            MCITEM *pMcItem = (MCITEM*)STreeCtrl::GetItemData(hItem);
            pMcItem->arrText.InsertAt(iCol,SStringT());
            hItem = GetNextItem(hItem);
        }
        m_nItemWid = -1;
        CalcItemWidth(0);
        
        CSize szView = GetViewSize();
        szView.cx = m_nItemWid;
        SetViewSize(szView);
        Invalidate();
        return iCol;
    }

    BOOL SMCTreeCtrl::DeleteColumn(int iCol)
    {
        if(iCol<0 || iCol>=m_arrColWidth.GetCount()) 
            return FALSE;
        HSTREEITEM hItem = GetRootItem();
        while(hItem)
        {
            MCITEM *pMcItem = (MCITEM*)STreeCtrl::GetItemData(hItem);
            pMcItem->arrText.RemoveAt(iCol);
            hItem = GetNextItem(hItem);
        }
        int nColWid = m_arrColWidth[iCol];
        m_arrColWidth.RemoveAt(iCol);
        
        m_nItemWid = -1;
        CalcItemWidth(0);

        CSize szView = GetViewSize();
        szView.cx = m_nItemWid;
        SetViewSize(szView);
        Invalidate();
        return TRUE;
    }


    BOOL SMCTreeCtrl::SetItemText(HSTREEITEM hItem,int iCol,const SStringT strText)
    {
        if(iCol<0 || iCol>=m_arrColWidth.GetCount()) 
            return FALSE;
            
        MCITEM *pMcItem = (MCITEM*)STreeCtrl::GetItemData(hItem);
        SASSERT(pMcItem);
        pMcItem->arrText.SetAt(iCol,strText);
        return TRUE;
    }

    void SMCTreeCtrl::DrawItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem)
    {
        CRect rcTreeItem = rc;
        rcTreeItem.right = rcTreeItem.left + m_nTreeWidth;
        DrawTreeItem(pRT,rcTreeItem,hItem);
        
        CRect rcListItem = rc;
        rcListItem.left = rcTreeItem.right;
        DrawListItem(pRT,rcListItem,hItem);
    }

    void SMCTreeCtrl::DrawTreeItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem)
    {
        STreeCtrl::DrawItem(pRT,rc,hItem);
    }

    void SMCTreeCtrl::DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem)
    {
        MCITEM *pMcItem =(MCITEM*)STreeCtrl::GetItemData(hItem);
        CRect rcItem = rc;
        rcItem.right=rcItem.left;
        
        for(int i=0;i<m_arrColWidth.GetCount();i++)
        {
            rcItem.left = rcItem.right;
            rcItem.right += m_arrColWidth[i];
            pRT->DrawText(pMcItem->arrText[i],pMcItem->arrText[i].GetLength(),rcItem,DT_SINGLELINE|DT_LEFT|DT_VCENTER);
        }
    }

    BOOL SMCTreeCtrl::SetColWidth(int iCol,int nWid)
    {
        if(iCol<0 || iCol>=m_arrColWidth.GetCount()) 
            return FALSE;
        m_arrColWidth.SetAt(iCol,nWid);
        m_nItemWid = -1;
        CalcItemWidth(0);
        
        CSize szView = GetViewSize();
        szView.cx = m_nItemWid;
        SetViewSize(szView);

        Invalidate();
        return TRUE;
    }

    void SMCTreeCtrl::SetTreeWidth(int nWid)
    {
        m_nTreeWidth = nWid;
        m_nItemWid = -1;
        CalcItemWidth(0);

        CSize szView = GetViewSize();
        szView.cx = m_nItemWid;
        SetViewSize(szView);

        Invalidate();
    }

    void SMCTreeCtrl::SetItemData( HSTREEITEM hItem, LPARAM lParam )
    {
        MCITEM *pData = (MCITEM*)STreeCtrl::GetItemData(hItem);
        pData->lParam = lParam;
    }

    LPARAM SMCTreeCtrl::GetItemData( HSTREEITEM hItem )
    {
        MCITEM *pData = (MCITEM*)STreeCtrl::GetItemData(hItem);
        return pData->lParam;
    }

    HSTREEITEM SMCTreeCtrl::InsertItem( LPCTSTR pszText,int iImage,int iSelImage,LPARAM lParam,HSTREEITEM hParent/*=STVI_ROOT*/,HSTREEITEM hAfter /*= STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/ )
    {
        MCITEM *pData = new MCITEM;
        pData->arrText.SetCount(m_arrColWidth.GetCount());
        pData->lParam = lParam;
        return STreeCtrl::InsertItem(pszText,iImage,iSelImage,(LPARAM)pData,hParent,hAfter,bEnsureVisible);
    }

    //////////////////////////////////////////////////////////////////////////
    STreeList::STreeList(void)
    :m_nHeaderHeight(20)
    ,m_pHeader(NULL)
    ,m_pTreeCtrl(NULL)
    {
    }

    STreeList::~STreeList(void)
    {
    }

    BOOL STreeList::CreateChildren(pugi::xml_node xmlNode)
    {
        pugi::xml_node xmlHeader = xmlNode.child(L"headerstyle");
        pugi::xml_node xmlTreectrl = xmlNode.child(L"treectrlstyle");
        if(!xmlHeader || !xmlTreectrl) 
            return FALSE;
        m_pHeader = CreateHeader();
        InsertChild(m_pHeader);
        m_pHeader->InitFromXml(xmlHeader);
        m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemChanging::EventID, Subscriber(&STreeList::OnHeaderSizeChanging,this));
        m_pHeader->GetEventSet()->subscribeEvent(EventHeaderItemSwap::EventID, Subscriber(&STreeList::OnHeaderSwap,this));

        m_pTreeCtrl = CreateMcTreeCtrl();
        InsertChild(m_pTreeCtrl);
        m_pTreeCtrl->InitFromXml(xmlTreectrl);
        m_pTreeCtrl->GetEventSet()->subscribeEvent(EventScroll::EventID,Subscriber(&STreeList::OnScrollEvent,this));

        m_pHeader->InsertItem(0,m_strTreeLabel,m_pTreeCtrl->m_nTreeWidth,ST_NULL,0);
        for(int i=1;i<m_pHeader->GetItemCount();i++)
        {
            int nWid = m_pHeader->GetItemWidth(i);
            m_pTreeCtrl->InsertColumn(i-1,nWid);
        }
        return TRUE;
    }

    void STreeList::OnRelayout(const CRect &rcOld, const CRect & rcNew)
    {
        SASSERT(m_pHeader && m_pTreeCtrl);
        CRect rcTreectrl = rcNew;
        rcTreectrl.top = rcNew.top + m_nHeaderHeight;
        m_pTreeCtrl->Move(rcTreectrl);
        
        CPoint ptOrg = m_pTreeCtrl->GetViewOrigin();
        CRect rcHeader = rcNew;
        rcHeader.left -= ptOrg.x;
        rcHeader.right =rcHeader.left + max(rcNew.Width(),m_pTreeCtrl->GetViewSize().cx);
        rcHeader.bottom = rcNew.top + m_nHeaderHeight;
        m_pHeader->Move(rcHeader);
    }

    bool STreeList::OnHeaderClick(EventArgs *pEvt)
    {
        return true;
    }

    bool STreeList::OnHeaderSizeChanging(EventArgs *pEvt)
    {
        EventHeaderItemChanging *pEvt2=sobj_cast<EventHeaderItemChanging>(pEvt);
        m_pTreeCtrl->GetEventSet()->setMutedState(true);
        if(pEvt2->iItem == 0)
            m_pTreeCtrl->SetTreeWidth(pEvt2->nWidth);
        else
            m_pTreeCtrl->SetColWidth(pEvt2->iItem-1,pEvt2->nWidth);
        m_pTreeCtrl->GetEventSet()->setMutedState(false);
        return true;
    }

    bool STreeList::OnHeaderSwap(EventArgs *pEvt)
    {
        return true;
    }

    bool STreeList::OnScrollEvent( EventArgs *pEvt )
    {
        EventScroll *pEvt2 = sobj_cast<EventScroll>(pEvt);
        SASSERT(pEvt2);

        if(!pEvt2->bVertical)
        {
            CRect rcClient=GetClientRect();
            CSize szView = m_pTreeCtrl->GetViewSize();

            CRect rcHeader = rcClient;
            rcHeader.bottom=rcHeader.top + m_nHeaderHeight;
            rcHeader.right =rcHeader.left + max(szView.cx,rcClient.Width());
            CPoint ptOrg = m_pTreeCtrl->GetViewOrigin();
            rcHeader.OffsetRect(-ptOrg.x,0);
            m_pHeader->Move(rcHeader);
        }

        return false;
    }

    SHeaderCtrl * STreeList::CreateHeader()
    {
        return (SHeaderCtrl*)SApplication::getSingleton().CreateWindowByName(SHeaderCtrl::GetClassName());
    }

    SMCTreeCtrl * STreeList::CreateMcTreeCtrl()
    {
        return new SMCTreeCtrl;
    }
}
