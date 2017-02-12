//////////////////////////////////////////////////////////////////////////
//  Class Name: STreeCtrl
// Description: STreeCtrl
//     Creator: huangjianxiong
//     Version: 2011.10.14 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "souistd.h"
#include "control/Streebox.h"
#include "control/SCmnCtrl.h"

namespace SOUI
{

static const wchar_t NAME_SWITCH[] =   L"switch";

    STreeItem::STreeItem(SWindow *pFrameHost,IItemContainer *pContainer)
    : SItemPanel(pFrameHost,pugi::xml_node(),pContainer)
    , m_bCollapsed(FALSE)
    , m_bVisible(TRUE)
    , m_nLevel(0)
{
}


STreeBox::STreeBox()
    : m_nItemHeight(20)         //默认为固定高度的列表项
    , m_nIndent(10)
    , m_hSelItem(NULL)
    , m_hHoverItem(NULL)
    , m_pCapturedFrame(NULL)
    , m_crItemBg(CR_INVALID)
    , m_crItemSelBg(RGB(0,0,128))
    , m_pItemSkin(NULL)
    , m_bItemRedrawDelay(TRUE)
{
    m_bFocusable=TRUE;
    m_evtSet.addEvent(EVENTID(EventTBGetDispInfo));
    m_evtSet.addEvent(EVENTID(EventTBSelChanging));
    m_evtSet.addEvent(EVENTID(EventTBSelChanged));
    m_evtSet.addEvent(EVENTID(EventTBQueryItemHeight));
}

STreeBox::~STreeBox()
{

}

HSTREEITEM STreeBox::InsertItem(pugi::xml_node xmlNode,DWORD dwData,HSTREEITEM hParent/*=STVI_ROOT*/, HSTREEITEM hInsertAfter/*=STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/)
{
    STreeItem *pItemObj=new STreeItem(this,this);
    pItemObj->InitFromXml(xmlNode);
    pItemObj->m_nLevel=GetItemLevel(hParent)+1;
    pItemObj->m_bCollapsed=FALSE;
    if(hParent!=STVI_ROOT)
    {
        STreeItem * pParentItem= GetItem(hParent);
        if(pParentItem->m_bCollapsed || !pParentItem->m_bVisible) pItemObj->m_bVisible=FALSE;
    }

    HSTREEITEM hRet= CSTree<STreeItem*>::InsertItem(pItemObj,hParent,hInsertAfter);
    pItemObj->SetItemIndex((LPARAM)hRet);
    pItemObj->SetItemData(dwData);
    pItemObj->SetColor(m_crItemBg,m_crItemSelBg);

    int nHeight = m_nItemHeight;
    if(nHeight<=0)
    {
        EventTBQueryItemHeight evt(this);
        evt.nItemHeight = nHeight;
        evt.hItem = hRet;
        evt.dwState = WndState_Normal;
        FireEvent(evt);
        nHeight = evt.nItemHeight;
        SASSERT(nHeight>0);
    }
    pItemObj->SetSkin(m_pItemSkin);
    pItemObj->m_nItemHeight = nHeight;
    pItemObj->m_nBranchHeight = nHeight;

    UpdateAncestorBranchHeight(hRet,nHeight);

    pItemObj->Move(CRect(0,0,m_rcClient.Width()-pItemObj->m_nLevel*m_nIndent,nHeight));

    pItemObj->GetEventSet()->subscribeEvent(EventStateChanged::EventID,Subscriber(&STreeBox::OnItemStateChanged,this));

    UpdateSwitchState(hRet);
    if(hParent!=STVI_ROOT) 
    {
        UpdateSwitchState(hParent);
    }

    if(pItemObj->m_bVisible)
    {
        int nViewHei = GetViewSize().cy;
        SetViewHeight(nViewHei + nHeight);
    }

    if(bEnsureVisible) EnsureVisible(hRet);
    return hRet;
}

STreeItem* STreeBox::InsertItem(LPCWSTR pszXml,DWORD dwData,HSTREEITEM hParent/*=STVI_ROOT*/, HSTREEITEM hInsertAfter/*=STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/)
{
    pugi::xml_document xmlDoc;
    pugi::xml_node xmlItem;
    if(pszXml)
    {
        if(!xmlDoc.load_buffer(pszXml,wcslen(pszXml)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
            return NULL;
        xmlItem=xmlDoc.first_child();
    }else
    {
        int nLevel = 0;
        if(hParent!=STVI_ROOT)
        {
            STreeItem *pItem = GetItem(hParent);
            nLevel = pItem->m_nLevel+1;
        }
        xmlItem = m_xmlTemplate.child(L"template");
        for(int i=0;i<nLevel;i++)
        {
            xmlItem = xmlItem.child(L"template");
        }
    }
    if(!xmlItem)
        return NULL;
    HSTREEITEM hItem=InsertItem(xmlItem,dwData,hParent,hInsertAfter,bEnsureVisible);
    return GetItem(hItem);
}

BOOL STreeBox::RemoveItem(HSTREEITEM hItem)
{
    if(!hItem) return FALSE;
    HSTREEITEM hParent=GetParentItem(hItem);

    STreeItem * pItem= CSTree<STreeItem*>::GetItem(hItem);
    int nBranchHei = pItem->m_bVisible?pItem->m_nBranchHeight:0;

    if(IsAncestor(hItem,m_hHoverItem)) m_hHoverItem=NULL;
    if(IsAncestor(hItem,m_hSelItem)) m_hSelItem=NULL;

    UpdateSwitchState(hParent);
    if(nBranchHei!=0) 
    {
        UpdateAncestorBranchHeight(hItem,-nBranchHei);
    }

    DeleteItem(hItem);

    SetViewHeight(GetViewSize().cy - nBranchHei);

    return TRUE;
}

void STreeBox::RemoveAllItems()
{
    DeleteAllItems();
    m_hSelItem=0;
    m_hHoverItem=0;
    m_pCapturedFrame=NULL;
    ReleaseCapture();
    SetViewSize(CSize(0,0));
}

HSTREEITEM STreeBox::GetRootItem()
{
    return GetChildItem(STVI_ROOT);
}

HSTREEITEM STreeBox::GetNextSiblingItem(HSTREEITEM hItem)
{
    return CSTree<STreeItem*>::GetNextSiblingItem(hItem);
}

HSTREEITEM STreeBox::GetPrevSiblingItem(HSTREEITEM hItem)
{
    return CSTree<STreeItem*>::GetPrevSiblingItem(hItem);
}

HSTREEITEM STreeBox::GetChildItem(HSTREEITEM hItem,BOOL bFirst/* =TRUE*/)
{
    return CSTree<STreeItem*>::GetChildItem(hItem,bFirst);
}

HSTREEITEM STreeBox::GetParentItem(HSTREEITEM hItem)
{
    return CSTree<STreeItem*>::GetParentItem(hItem);
}


void STreeBox::OnDestroy()
{
    DeleteAllItems();
    __super::OnDestroy();
}

BOOL STreeBox::Expand(HSTREEITEM hItem , UINT nCode)
{
    if(!CSTree<STreeItem*>::GetChildItem(hItem)) return FALSE;
    BOOL bRet=FALSE;
    STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);
    if(nCode==TVE_COLLAPSE && !pItem->m_bCollapsed)
    {
        pItem->m_bCollapsed=TRUE;
        SetChildrenVisible(hItem,FALSE);
        bRet=TRUE;
    }
    if(nCode==TVE_EXPAND && pItem->m_bCollapsed)
    {
        pItem->m_bCollapsed=FALSE;
        SetChildrenVisible(hItem,TRUE);
        bRet=TRUE;
    }
    if(nCode==TVE_TOGGLE)
    {
        pItem->m_bCollapsed=!pItem->m_bCollapsed;
        SetChildrenVisible(hItem,!pItem->m_bCollapsed);
        bRet=TRUE;
    }
    if(bRet)
    {
        UpdateSwitchState(hItem);
        
        int nChildrenHeight = pItem->m_nBranchHeight - pItem->m_nItemHeight;
        if(pItem->m_bCollapsed) nChildrenHeight*=-1;

        UpdateAncestorBranchHeight(hItem,nChildrenHeight);

        int nViewHei = GetViewSize().cy+nChildrenHeight;
        SetViewHeight(nViewHei);
    }
    return bRet;
}

BOOL STreeBox::EnsureVisible(HSTREEITEM hItem)
{
    STreeItem *pItem=GetItem(hItem);
    if(!pItem->m_bVisible)
    {
        HSTREEITEM hParent=GetParentItem(hItem);
        while(hParent)
        {
            STreeItem *pParent=GetItem(hParent);
            if(pParent->m_bCollapsed) Expand(hParent,TVE_EXPAND);
            hParent=GetParentItem(hParent);
        }
    }

    CRect rcItemInView = GetItemRectInView(hItem);

    if(rcItemInView.top>m_ptOrigin.y+m_rcClient.Height())
    {
        SetScrollPos(TRUE,rcItemInView.bottom-m_rcClient.Height(),TRUE);
    }
    else if(rcItemInView.bottom<m_ptOrigin.y)
    {
        SetScrollPos(TRUE,rcItemInView.top,TRUE);
    }
    return TRUE;
}


HSTREEITEM STreeBox::_HitTest(HSTREEITEM hItem, int & yOffset, const CPoint & pt )
{
	if(!hItem) return NULL;
    CRect rcClient;
    GetClientRect(&rcClient);
    STreeItem * pItem = GetItem(hItem);
    if(pt.y>=yOffset && pt.y<yOffset+pItem->m_nItemHeight)
        return hItem;

    yOffset += pItem->m_nItemHeight;

    if(!pItem->m_bCollapsed)
    {
        HSTREEITEM hChild=GetChildItem(hItem,TRUE);
        if(hChild)
        {
            HSTREEITEM hRet = _HitTest(hChild,yOffset,pt);
            if(hRet) return hRet;
        }
    }

    hItem = GetNextSiblingItem(hItem);
    if(!hItem) return NULL;

    return _HitTest(hItem,yOffset,pt);
}

//自动修改pt的位置为相对当前项的偏移量
HSTREEITEM STreeBox::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    if(!rcClient.PtInRect(pt)) return NULL;
    int yOffset = 0;
    pt -= rcClient.TopLeft() - m_ptOrigin;//更新pt
    HSTREEITEM hHit = _HitTest(GetChildItem(STVI_ROOT,TRUE),yOffset,pt);
    if(hHit)
    {
        STreeItem *pItem = GetItem(hHit);
        if(pt.x >= pItem->m_nLevel*m_nIndent && pt.x< rcClient.Width())
        {
            pt.y-=yOffset;
            pt.x-=pItem->m_nLevel*m_nIndent;
            return hHit;
        }
    }
    return NULL;
}

void STreeBox::SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible)
{
    HSTREEITEM hChild=GetChildItem(hItem);
    while(hChild)
    {
        STreeItem *pItem=GetItem(hChild);
        pItem->m_bVisible=bVisible;
        if(!pItem->m_bCollapsed) SetChildrenVisible(hChild,bVisible);
        hChild=GetNextSiblingItem(hChild);
    }
}

void STreeBox::OnNodeFree(STreeItem * & pItem)
{
    if(m_pCapturedFrame==pItem)
    {
        m_pCapturedFrame=NULL;
        ReleaseCapture();
    }
    pItem->Release();
}

BOOL STreeBox::CreateChildren(pugi::xml_node xmlNode)
{
    if(!xmlNode) return FALSE;

    RemoveAllItems();
    pugi::xml_node xmlTemplate = xmlNode.child(L"template");
    if(xmlTemplate)
    {
        m_xmlTemplate.append_copy(xmlTemplate);
    }
    pugi::xml_node xmlItem=xmlNode.child(L"item");
    if(xmlItem) 
    {
        LoadBranch(STVI_ROOT,xmlItem);
    }

    return TRUE;
}

void STreeBox::LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlItem)
{
    while(xmlItem)
    {
        int dwData=xmlItem.attribute(L"itemdata").as_int(0);
        HSTREEITEM hItem=InsertItem(xmlItem,dwData,hParent);

        pugi::xml_node xmlChild=xmlItem.child(L"item");
        if(xmlChild)
        {
            LoadBranch(hItem,xmlChild);
            Expand(hItem,xmlItem.attribute(L"expand").as_bool(true)?TVE_EXPAND:TVE_COLLAPSE);
        }

        xmlItem=xmlItem.next_sibling(L"item");
    }
}

void STreeBox::OnSize( UINT nType,CSize size )
{
    __super::OnSize(nType,size);
    CSize szView = GetViewSize();
    CRect rcClient;
    SWindow::GetClientRect(&rcClient);
    if(szView.cy>rcClient.Height())
        szView.cx = rcClient.Width()-m_nSbWid;
    else
        szView.cx = rcClient.Width();
    SetViewSize(szView);
}

CPoint STreeBox::GetItemOffsetInView( HSTREEITEM hItem )
{
    CPoint ptOffset;
    STreeItem *pItem = GetItem(hItem);
    SASSERT(pItem->m_bVisible);

    ptOffset.x = pItem->m_nLevel * m_nIndent;

    HSTREEITEM hPreSibling = GetPrevSiblingItem(hItem);
    while(hPreSibling)
    {
        STreeItem *pPreItem = GetItem(hPreSibling);
        if(!pPreItem->m_bCollapsed)
            ptOffset.y+= pPreItem->m_nBranchHeight;
        else
            ptOffset.y += pPreItem->m_nItemHeight;
        hPreSibling = GetPrevSiblingItem(hPreSibling);
    }
    HSTREEITEM hParent = GetParentItem(hItem);
    if(hParent)
    {
        ptOffset.y+=GetItemOffsetInView(hParent).y;
        STreeItem *pParentItem = GetItem(hParent);
        ptOffset.y+= pParentItem->m_nItemHeight;
    }
    return ptOffset;
}


CRect STreeBox::GetItemRectInView(HSTREEITEM hItem)
{
    CRect rcItem;
    STreeItem *pItem = GetItem(hItem);
    if(pItem->m_bVisible)
    {
        CPoint ptOffset = GetItemOffsetInView(hItem);
        pItem->GetWindowRect(&rcItem);
        rcItem.OffsetRect(ptOffset);
    }
    return rcItem;
}

int STreeBox::GetItemShowIndex(HSTREEITEM hItemObj)
{
    int iVisible=-1;
    HSTREEITEM hItem=GetNextItem(STVI_ROOT);
    while(hItem)
    {
        STreeItem *pItem=GetItem(hItem);
        if(pItem->m_bVisible) iVisible++;
        if(hItem==hItemObj)
        {
            return iVisible;
        }
        if(pItem->m_bCollapsed)
        {
            //跳过被折叠的项
            HSTREEITEM hChild= GetChildItem(hItem,FALSE);
            while(hChild)
            {
                hItem=hChild;
                hChild= GetChildItem(hItem,FALSE);
            }
        }
        hItem=GetNextItem(hItem);
    }
    return -1;
}

void STreeBox::RedrawItem(HSTREEITEM hItem)
{
    if(!IsVisible(TRUE)) return;

    CRect rcItem = GetItemRectInView(hItem);
    if(rcItem.IsRectEmpty())
        return;
    rcItem.OffsetRect(-m_ptOrigin);

    CRect rcClient;
    GetClientRect(&rcClient);

    rcItem.OffsetRect(rcClient.TopLeft());

    if((rcItem & rcClient).IsRectEmpty())
        return;
    
    IRenderTarget *pRT=GetRenderTarget(&rcItem,OLEDC_PAINTBKGND);

    SSendMessage(WM_ERASEBKGND,(WPARAM)(HDC)pRT);
    DrawItem(pRT,rcItem,hItem);

    ReleaseRenderTarget(pRT);
}

void STreeBox::DrawItem(IRenderTarget * pRT, CRect & rc, HSTREEITEM hItem)
{
    STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);
    
    EventTBGetDispInfo evt(this);
    evt.bSel = hItem == m_hSelItem;
    evt.bHover =hItem==m_hHoverItem;
    evt.pItemWnd = pItem;
    evt.hItem = hItem;

    LockUpdate();
    GetContainer()->OnFireEvent(evt);
    UnlockUpdate();
    
    pItem->Draw(pRT,rc);
}


void STreeBox::PaintVisibleItem( IRenderTarget *pRT,IRegion *pRgn,HSTREEITEM hItem,int &yOffset )
{
	if(!hItem) return;
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcItem;
    STreeItem * pItem = GetItem(hItem);
    pItem->GetWindowRect(&rcItem);
    rcItem.OffsetRect(pItem->m_nLevel*m_nIndent,yOffset);
    rcItem.OffsetRect(-m_ptOrigin);
    rcItem.OffsetRect(rcClient.TopLeft());

    if(yOffset+pItem->m_nItemHeight> m_ptOrigin.y && pRgn->RectInRegion(&rcItem))
        DrawItem(pRT,rcItem,hItem);

    yOffset += rcItem.Height();
    
    int yOffsetMax = m_ptOrigin.y+rcClient.Height();
    if(yOffset>=yOffsetMax)
        return;

    if(!pItem->m_bCollapsed)
    {
        HSTREEITEM hChild=GetChildItem(hItem,TRUE);
        if(hChild && yOffset<yOffsetMax)
        {
            PaintVisibleItem(pRT,pRgn,hChild,yOffset);
        }
    }

    if(yOffset>yOffsetMax) 
        return;

    hItem = GetNextSiblingItem(hItem);
    if(hItem)
        PaintVisibleItem(pRT,pRgn,hItem,yOffset);
}

void STreeBox::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;
    BeforePaint(pRT,painter);
    
    CAutoRefPtr<IRegion> pClipRgn;
    pRT->GetClipRegion(&pClipRgn);
    
    int yOffset =0;
    PaintVisibleItem(pRT,pClipRgn,GetChildItem(STVI_ROOT,TRUE),yOffset);

    AfterPaint(pRT,painter);
}

void STreeBox::OnLButtonDown(UINT nFlags,CPoint pt)
{
    if(m_bFocusable) SetFocus();
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        if(!rcItem.IsRectEmpty())
        {
            pt.Offset(-rcItem.left,-rcItem.top);
            m_pCapturedFrame->DoFrameEvent(WM_LBUTTONDOWN,nFlags,MAKELPARAM(pt.x,pt.y));;
            return;
        }
    }
    m_hHoverItem=HitTest(pt);

    if(m_hHoverItem!=m_hSelItem)
    {
        EventTBSelChanging evt1(this);
        evt1.hOldSel=m_hSelItem;
        evt1.hNewSel=m_hHoverItem;
        evt1.bCancel=FALSE;
        FireEvent(evt1);

        if(!evt1.bCancel)
        {
            EventTBSelChanged evt2(this);
            evt2.hOldSel=m_hSelItem;
            evt2.hNewSel=m_hHoverItem;

            if(m_hSelItem)
            {
                CSTree<STreeItem*>::GetItem(m_hSelItem)->GetFocusManager()->SetFocusedHwnd(0);
                CSTree<STreeItem*>::GetItem(m_hSelItem)->ModifyItemState(0,WndState_Check);
                InvalidateRect(GetItemRect(m_hSelItem));
            }
            m_hSelItem=m_hHoverItem;
            if(m_hSelItem)
            {
                CSTree<STreeItem*>::GetItem(m_hSelItem)->ModifyItemState(WndState_Check,0);
                InvalidateRect(GetItemRect(m_hSelItem));
            }
            FireEvent(evt2);
        }
    }
    if(m_hHoverItem)
    {
        //pt 已经在HitTest中被修改过
        CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_LBUTTONDOWN,nFlags,MAKELPARAM(pt.x,pt.y));
    }
}

LRESULT STreeBox::OnMouseEvent( UINT uMsg,WPARAM wParam,LPARAM lParam )
{
    if(uMsg == WM_MOUSEWHEEL)
    {
        POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
        return SScrollView::OnMouseWheel(GET_KEYSTATE_WPARAM(wParam),GET_WHEEL_DELTA_WPARAM(wParam),pt);
    }
    CPoint pt(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        pt.Offset(-rcItem.left,-rcItem.top);
        return m_pCapturedFrame->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
    }
    m_hHoverItem=HitTest(pt);
    if(m_hHoverItem)
    {
        return CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
    }
    return 0;
}

void STreeBox::OnLButtonDbClick(UINT nFlags,CPoint pt)
{
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        if(!rcItem.IsRectEmpty())
        {
            pt.Offset(-rcItem.left,-rcItem.top);
            m_pCapturedFrame->DoFrameEvent(WM_LBUTTONDBLCLK,nFlags,MAKELPARAM(pt.x,pt.y));
            return;
        }
    }
    m_hHoverItem=HitTest(pt);
    if(m_hHoverItem)
    {
        Expand(m_hHoverItem,TVE_TOGGLE);
        CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_LBUTTONDBLCLK,nFlags,MAKELPARAM(pt.x,pt.y));
    }
}

void STreeBox::OnMouseMove(UINT nFlags,CPoint pt)
{
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        if(!rcItem.IsRectEmpty())
        {
            pt.Offset(-rcItem.left,-rcItem.top);
            m_pCapturedFrame->DoFrameEvent(WM_MOUSEMOVE,nFlags,MAKELPARAM(pt.x,pt.y));
            return;
        }
    }
    HSTREEITEM hHitTest=HitTest(pt);
    if(hHitTest!=m_hHoverItem)
    {
        if(m_hHoverItem)
            CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_MOUSELEAVE,0,0);
        m_hHoverItem=hHitTest;
        if(m_hHoverItem)
            CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_MOUSEHOVER,0,0);
    }
    if(m_hHoverItem)
    {
        CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_MOUSEMOVE,nFlags,MAKELPARAM(pt.x,pt.y));
    }
}

void STreeBox::OnMouseLeave()
{
    if(m_pCapturedFrame)
    {
        m_pCapturedFrame->DoFrameEvent(WM_MOUSELEAVE,0,0);
    }
    if(m_hHoverItem)
    {
        CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_MOUSELEAVE,0,0);
        m_hHoverItem=NULL;
    }
}

BOOL STreeBox::FireEvent(EventArgs &evt)
{
    if(evt.GetEventID()==EventOfPanel::EventID)
    {
        EventOfPanel *pEvt = (EventOfPanel *)&evt;
        if(pEvt->pOrgEvt->GetEventID()==EVT_CMD 
            && wcscmp(pEvt->pOrgEvt->nameFrom , NAME_SWITCH)==0)
        {
            STreeItem *pItem=(STreeItem*)pEvt->pPanel;
            SASSERT(pItem);
            Expand((HSTREEITEM)pItem->GetItemIndex(),TVE_TOGGLE);
            return TRUE;
        }
    }
    return __super::FireEvent(evt);
}

BOOL STreeBox::OnSetCursor(const CPoint &pt)
{
    BOOL bRet=FALSE;
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        bRet=m_pCapturedFrame->DoFrameEvent(WM_SETCURSOR,0,MAKELPARAM(pt.x-rcItem.left,pt.y-rcItem.top))!=0;
    }
    else if(m_hHoverItem)
    {
        CRect rcItem=CSTree<STreeItem*>::GetItem(m_hHoverItem)->GetItemRect();
        bRet=CSTree<STreeItem*>::GetItem(m_hHoverItem)->DoFrameEvent(WM_SETCURSOR,0,MAKELPARAM(pt.x-rcItem.left,pt.y-rcItem.top))!=0;
    }
    if(!bRet)
    {
        bRet=__super::OnSetCursor(pt);
    }
    return bRet;
}


BOOL STreeBox::IsAncestor(HSTREEITEM hItem1,HSTREEITEM hItem2)
{
    while(hItem2)
    {
        if(hItem2==hItem1) return TRUE;
        hItem2=GetParentItem(hItem2);
    }
    return FALSE;
}

void STreeBox::OnItemSetCapture( SItemPanel *pItem,BOOL bCapture )
{
    if(bCapture)
    {
        m_pCapturedFrame=pItem;
        SetCapture();
    }
    else if(pItem==m_pCapturedFrame)
    {
        ReleaseCapture();
        m_pCapturedFrame=NULL;
    }
}

BOOL STreeBox::OnItemGetRect( SItemPanel *pItem,CRect &rcItem )
{
    STreeItem *pItemObj=(STreeItem*)pItem;
    HSTREEITEM hItem = (HSTREEITEM)pItemObj->GetItemIndex();
    if(pItemObj->m_bVisible==FALSE) 
        return FALSE;
    
    rcItem = GetItemRect(hItem);
    return !rcItem.IsRectEmpty();
}


void STreeBox::OnSetFocus()
{
    __super::OnSetFocus();
    if(m_hSelItem) CSTree<STreeItem*>::GetItem(m_hSelItem)->DoFrameEvent(WM_SETFOCUS,0,0);
}

void STreeBox::OnKillFocus()
{
    __super::OnKillFocus();
    if(m_hSelItem) CSTree<STreeItem*>::GetItem(m_hSelItem)->DoFrameEvent(WM_KILLFOCUS,0,0);
}

void STreeBox::OnViewOriginChanged( CPoint ptOld,CPoint ptNew )
{
    if(m_hSelItem)
    {
        CSTree<STreeItem*>::GetItem(m_hSelItem)->DoFrameEvent(WM_KILLFOCUS,0,0);
        CSTree<STreeItem*>::GetItem(m_hSelItem)->DoFrameEvent(WM_SETFOCUS,0,0);
    }
}

void STreeBox::UpdateItemWidth(HSTREEITEM hItem,int nWidth)
{
    if(hItem != STVI_ROOT)
    {
        STreeItem * pItem = CSTree<STreeItem*>::GetItem(hItem);
        if(!pItem->m_bVisible) return;
        int xOffset = m_nIndent * pItem->m_nLevel;
        pItem->Move(0,0,nWidth-xOffset,pItem->m_nItemHeight);
    }

    HSTREEITEM hChild = GetChildItem(hItem,TRUE);
    while(hChild)
    {
        UpdateItemWidth(hChild,nWidth);
        hChild = GetNextSiblingItem(hChild);
    }
}

void STreeBox::OnViewSizeChanged( CSize szOld,CSize szNew )
{
    if(szOld.cx == szNew.cx) return;
    //显示宽度发生了变化
    UpdateItemWidth(STVI_ROOT,szNew.cx);
    Invalidate();
}

LRESULT STreeBox::OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam )
{
    LRESULT lRet=0;
    if(m_pCapturedFrame)
    {
        lRet=m_pCapturedFrame->DoFrameEvent(uMsg,wParam,lParam);
        SetMsgHandled(m_pCapturedFrame->IsMsgHandled());
    }
    else if(m_hSelItem)
    {
        STreeItem* pItem=CSTree<STreeItem*>::GetItem(m_hSelItem);
        lRet=pItem->DoFrameEvent(uMsg,wParam,lParam);
        SetMsgHandled(pItem->IsMsgHandled());
    }else
    {
        SetMsgHandled(FALSE);
    }
    return lRet;
}

void STreeBox::UpdateSwitchState( HSTREEITEM hItem )
{
    STreeItem *pItem = GetItem(hItem);
    SToggle *pSwitch = pItem->FindChildByName2<SToggle>(NAME_SWITCH);
    if(pSwitch)
    {
        pSwitch->SetVisible(GetChildItem(hItem)!=0);
        pSwitch->SetToggle(!pItem->m_bCollapsed);
    }
}

BOOL STreeBox::OnUpdateToolTip( CPoint pt, SwndToolTipInfo & tipInfo )
{
    if(m_hHoverItem==NULL)
        return __super::OnUpdateToolTip(pt,tipInfo);
    STreeItem *pItem = GetItem(m_hHoverItem);
    return pItem->OnUpdateToolTip(pt,tipInfo);
}

bool STreeBox::OnItemStateChanged( EventArgs *pEvt )
{
    if(!pEvt->sender->IsClass(STreeItem::GetClassName())) return false;
    EventStateChanged *pEvtStateChanged = (EventStateChanged*)pEvt;
    STreeItem * pItem = (STreeItem*) pEvt->sender;
    if(m_nItemHeight>0) //固定高度
        return true;
    
    
    EventTBQueryItemHeight evt(this);
    evt.nItemHeight = pItem->m_nItemHeight;
    evt.hItem = (HSTREEITEM)pItem->GetItemIndex();
    evt.dwState = pEvtStateChanged->dwNewState;
    FireEvent(evt);

    if(evt.nItemHeight != pItem->m_nItemHeight)
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        
        int nHeightChange = evt.nItemHeight - pItem->m_nItemHeight;
        pItem->Move(CRect(0,0,rcClient.Width()-m_nIndent*pItem->m_nLevel,evt.nItemHeight));
        pItem->m_nItemHeight = evt.nItemHeight;
        UpdateAncestorBranchHeight(evt.hItem,nHeightChange);
        SetViewHeight(GetViewSize().cy + nHeightChange);
    }

    return true;
}

void STreeBox::SetViewHeight( int nHeight )
{
    CRect rcClient;
    SWindow::GetClientRect(&rcClient);
    CSize szView = GetViewSize();
    szView.cy = nHeight;
    szView.cx = rcClient.Width();
    if(szView.cy > rcClient.Height())
        szView.cx -= m_nSbWid;
    SetViewSize(szView);
}

void STreeBox::UpdateAncestorBranchHeight(HSTREEITEM hItem, int nHeightChange )
{
    HSTREEITEM hParent = GetParentItem(hItem);
    while(hParent)
    {
        STreeItem * pItem = GetItem(hParent);
        if(pItem->m_bCollapsed) break;
        pItem->m_nBranchHeight += nHeightChange;
        hParent = GetParentItem(hParent);
    }
}

CRect STreeBox::GetItemRect( HSTREEITEM hItem )
{
    CRect rcItem;
    SASSERT(hItem && hItem != STVI_ROOT);

    STreeItem *pItemObj=GetItem(hItem);
    if(pItemObj->m_bVisible==FALSE) 
        return rcItem;

    CRect rcClient;
    GetClientRect(&rcClient);

    rcItem = GetItemRectInView(hItem);
    rcItem.OffsetRect(-m_ptOrigin);
    rcItem.OffsetRect(rcClient.TopLeft());

    if((rcClient & rcItem).IsRectEmpty()) 
        rcItem.SetRectEmpty();
    return rcItem;    
}

}//namespace SOUI