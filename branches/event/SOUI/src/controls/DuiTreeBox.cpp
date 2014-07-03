//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiTreeCtrl
// Description: CDuiTreeCtrl
//     Creator: huangjianxiong
//     Version: 2011.10.14 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "control/duitreebox.h"
#include "control/DuiCmnCtrl.h"

namespace SOUI
{

#define IDC_SWITCH    65530

    STreeItem::STreeItem(SWindow *pFrameHost)
    : SItemPanel(pFrameHost,pugi::xml_node())
    , m_bCollapsed(FALSE)
    , m_bVisible(TRUE)
    , m_nLevel(0)
{
}


STreeBox::STreeBox()
    : m_nItemHei(20)
    , m_nIndent(10)
    , m_hSelItem(NULL)
    , m_hHoverItem(NULL)
    , m_pCapturedFrame(NULL)
    , m_crItemBg(CR_INVALID)
    , m_crItemSelBg(RGB(0,0,128))
    , m_pItemSkin(NULL)
    , m_nVisibleItems(0)
    , m_bItemRedrawDelay(TRUE)
{
    m_bTabStop=TRUE;
    addEvent(NM_LBITEMNOTIFY);
    addEvent(NM_ITEMMOUSEEVENT);
    addEvent(NM_GETTBDISPINFO);
    addEvent(NM_TBSELCHANGING);
    addEvent(NM_TBSELCHANGED);
}

STreeBox::~STreeBox()
{

}

HSTREEITEM STreeBox::InsertItem(pugi::xml_node xmlNode,DWORD dwData,HSTREEITEM hParent/*=STVI_ROOT*/, HSTREEITEM hInsertAfter/*=STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/)
{
    STreeItem *pItemObj=new STreeItem(this);
    pItemObj->InitFromXml(xmlNode);
    pItemObj->m_nLevel=GetItemLevel(hParent)+1;
    pItemObj->m_bCollapsed=FALSE;
    if(hParent!=STVI_ROOT)
    {
        STreeItem * pParentItem= GetItem(hParent);
        if(pParentItem->m_bCollapsed || !pParentItem->m_bVisible) pItemObj->m_bVisible=FALSE;
        if(!GetChildItem(hParent) && m_xmlSwitch.first_child())
        {
            SToggle *pToggle=new SToggle;
            pToggle->SetContainer(pParentItem->GetContainer());
            pToggle->InitFromXml(m_xmlSwitch.first_child());
            pParentItem->InsertChild(pToggle);
            pToggle->SetToggle(FALSE,FALSE);
            pToggle->SetID(IDC_SWITCH);
            pToggle->SendMessage(WM_WINDOWPOSCHANGED);
        }
    }
    pItemObj->SetItemData(dwData);
    pItemObj->SetColor(m_crItemBg,m_crItemSelBg);
    pItemObj->Move(CRect(0,0,m_rcClient.Width()-pItemObj->m_nLevel*m_nIndent,m_nItemHei));
    pItemObj->SetSkin(m_pItemSkin);

    HSTREEITEM hRet= CSTree<STreeItem*>::InsertItem(pItemObj,hParent,hInsertAfter);
    pItemObj->m_hItem=hRet;

    if(pItemObj->m_bVisible)
    {
        m_nVisibleItems++;

        CSize szView(m_rcWindow.Width(),m_nVisibleItems*m_nItemHei);
        if(szView.cy>m_rcWindow.Height()) szView.cx-=m_nSbWid;
        SetViewSize(szView);
        Invalidate();
    }

    if(bEnsureVisible) EnsureVisible(hRet);
    return hRet;
}

STreeItem* STreeBox::InsertItem(LPCWSTR pszXml,DWORD dwData,HSTREEITEM hParent/*=STVI_ROOT*/, HSTREEITEM hInsertAfter/*=STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/)
{
    pugi::xml_document xmlDoc;
    SStringA strXml=DUI_CW2A(pszXml,CP_UTF8);;

    if(!xmlDoc.load_buffer((LPCSTR)strXml,strXml.GetLength(),pugi::parse_default,pugi::encoding_utf8)) return NULL;

    HSTREEITEM hItem=InsertItem(xmlDoc.first_child(),dwData,hParent,hInsertAfter,bEnsureVisible);
    return GetItem(hItem);
}

BOOL STreeBox::RemoveItem(HSTREEITEM hItem)
{
    if(!hItem) return FALSE;
    HSTREEITEM hParent=GetParentItem(hItem);

    STreeItem * pItem= CSTree<STreeItem*>::GetItem(hItem);

    BOOL bVisible=pItem->m_bVisible;
    if(bVisible)
    {
        if(GetChildItem(hItem) && pItem->m_bCollapsed==FALSE)
        {
            SetChildrenVisible(hItem,FALSE);
        }
    }

    if(IsAncestor(hItem,m_hHoverItem)) m_hHoverItem=NULL;
    if(IsAncestor(hItem,m_hSelItem)) m_hSelItem=NULL;

    DeleteItem(hItem);

    if(hParent && !GetChildItem(hParent) && m_xmlSwitch)
    {
        //去掉父节点的展开标志
        STreeItem *pParent=GetItem(hParent);
        pParent->m_bCollapsed=FALSE;
        SWindow *pToggle=pParent->GetChild(IDC_SWITCH);
        ASSERT(pToggle);
        pParent->DestroyChild(pToggle);
        if(pParent->m_bVisible) InvalidateRect(pParent->GetItemRect());
    }


    if(bVisible)
    {
        m_nVisibleItems--;

        CSize szView(m_rcWindow.Width(),m_nVisibleItems*m_nItemHei);
        if(szView.cy>m_rcWindow.Height()) szView.cx-=m_nSbWid;
        SetViewSize(szView);
        Invalidate();
    }
    return TRUE;
}

void STreeBox::RemoveAllItems()
{
    DeleteAllItems();
    m_nVisibleItems=0;
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


void STreeBox::PageUp()
{
    OnScroll(TRUE,SB_PAGEUP,0);
}

void STreeBox::PageDown()
{
    OnScroll(TRUE,SB_PAGEDOWN,0);
}

void STreeBox::OnDestroy()
{
    DeleteAllItems();
    __super::OnDestroy();
}

BOOL STreeBox::Expand(HSTREEITEM hItem , UINT nCode)
{
    BOOL bRet=FALSE;
    if(CSTree<STreeItem*>::GetChildItem(hItem))
    {
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
            if(m_xmlSwitch.first_child())
            {
                SToggle *pSwitch=(SToggle*)pItem->GetChild(IDC_SWITCH);
                ASSERT(pSwitch);
                pSwitch->SetToggle(pItem->m_bCollapsed,FALSE);
            }
            CSize szView(m_rcWindow.Width(),m_nVisibleItems*m_nItemHei);
            if(szView.cy>m_rcWindow.Height()) szView.cx-=m_nSbWid;
            SetViewSize(szView);
            Invalidate();
        }
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
    int iVisible= GetItemShowIndex(hItem);
    int yOffset=iVisible*m_nItemHei;
    if(yOffset+m_nItemHei>m_ptOrigin.y+m_rcClient.Height())
    {
        SetScrollPos(TRUE,yOffset+m_nItemHei-m_rcClient.Height(),TRUE);
    }
    else if(yOffset<m_ptOrigin.y)
    {
        SetScrollPos(TRUE,yOffset,TRUE);
    }
    return TRUE;
}

//自动修改pt的位置为相对当前项的偏移量
HSTREEITEM STreeBox::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClient(&rcClient);
    CPoint pt2=pt;
    pt2.y -= rcClient.top - m_ptOrigin.y;
    int iItem=pt2.y/m_nItemHei;
    if( iItem >= m_nVisibleItems) return NULL;

    HSTREEITEM hRet=NULL;

    int iVisible=-1;
    HSTREEITEM hItem=CSTree<STreeItem*>::GetNextItem(STVI_ROOT);
    while(hItem)
    {
        STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);
        if(pItem->m_bVisible) iVisible++;
        if(iVisible == iItem)
        {
            CRect rcItem(m_nIndent*pItem->m_nLevel,0,m_rcWindow.Width(),m_nItemHei);
            rcItem.OffsetRect(m_rcWindow.left,m_rcWindow.top-m_ptOrigin.y+iVisible*m_nItemHei);
            pt-=rcItem.TopLeft();
            hRet=hItem;
            break;
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
        hItem=CSTree<STreeItem*>::GetNextItem(hItem);
    }
    return hRet;
}



void STreeBox::SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible)
{
    HSTREEITEM hChild=GetChildItem(hItem);
    while(hChild)
    {
        STreeItem *pItem=GetItem(hChild);
        pItem->m_bVisible=bVisible;
        m_nVisibleItems += bVisible?1:-1;
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


int STreeBox::GetScrollLineSize(BOOL bVertical)
{
    return m_nItemHei;
}

BOOL STreeBox::CreateChildren(pugi::xml_node xmlNode)
{
    if(!xmlNode) return FALSE;

    pugi::xml_node xmlSwitch=xmlNode.child(L"switch");

    if(xmlSwitch)   m_xmlSwitch.append_copy(xmlSwitch);

    RemoveAllItems();

    pugi::xml_node xmlItem=xmlNode.child(L"item");
    if(xmlItem) LoadBranch(STVI_ROOT,xmlItem);

    return TRUE;
}

void STreeBox::LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlItem)
{
    while(xmlItem)
    {
        int dwData=xmlItem.attribute(L"itemdata").as_int(0);
        HSTREEITEM hItem=InsertItem(xmlItem,dwData,hParent);

        pugi::xml_node xmlChild=xmlItem.child(L"item");
        if(xmlChild) LoadBranch(hItem,xmlChild);

        xmlItem=xmlItem.next_sibling(L"item");
    }
}

LRESULT STreeBox::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
{
    __super::OnNcCalcSize(bCalcValidRects,lParam);
    HSTREEITEM hItem=GetNextItem(STVI_ROOT);
    while(hItem)
    {
        STreeItem * pItem= GetItem(hItem);
        pItem->Move(CRect(0,0,m_rcClient.Width()-pItem->m_nLevel*m_nIndent,m_nItemHei));
        hItem=GetNextItem(hItem);
    }
    Invalidate();
    return 0;
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
    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(m_rcWindow.Height()+m_nItemHei-1)/m_nItemHei+1;
    int iItem=GetItemShowIndex(hItem);


    if(iItem!=-1 && iItem>=iFirstVisible && iItem<iFirstVisible+nPageItems)
    {
        STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);

        CRect rcItem(pItem->m_nLevel*m_nIndent,0,m_rcClient.Width(),m_nItemHei);
        rcItem.OffsetRect(0,m_nItemHei*iItem-m_ptOrigin.y);
        rcItem.OffsetRect(m_rcClient.TopLeft());

        IRenderTarget *pRT=GetRenderTarget(&rcItem,OLEDC_PAINTBKGND);
        SPainter painter;
        BeforePaint(pRT,painter);

        SendMessage(WM_ERASEBKGND,(WPARAM)(HDC)pRT);
        DrawItem(pRT,rcItem,hItem);

        AfterPaint(pRT,painter);
        ReleaseRenderTarget(pRT);
    }
}

void STreeBox::DrawItem(IRenderTarget * pRT, CRect & rc, HSTREEITEM hItem)
{
    STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);
    DUINMGETTBDISPINFO nms;
    nms.hdr.code    = NM_GETTBDISPINFO;
    nms.hdr.hDuiWnd = m_hSWnd;
    nms.hdr.idFrom  = GetID();
    nms.hdr.pszNameFrom = GetName();
    nms.bHover      = hItem==m_hHoverItem;
    nms.bSelect     = hItem == m_hSelItem;
    nms.hItem = hItem;
    nms.pItem = pItem;
    nms.pHostDuiWin   = this;
    LockUpdate();
    GetContainer()->OnFireEvent((LPSNMHDR)&nms);
    UnlockUpdate();
    pItem->Draw(pRT,rc);
}

void STreeBox::OnPaint(IRenderTarget *pRT)
{
    if(IsUpdateLocked()) return;

    SPainter painter;
    BeforePaint(pRT,painter);

    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(m_rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    int iVisible=-1;
    HSTREEITEM hItem=CSTree<STreeItem*>::GetNextItem(STVI_ROOT);
    while(hItem)
    {
        STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);
        if(pItem->m_bVisible) iVisible++;
        if(iVisible > iFirstVisible+nPageItems) break;
        if(iVisible>=iFirstVisible)
        {
            CRect rcItem(m_nIndent*pItem->m_nLevel,0,m_rcWindow.Width(),m_nItemHei);
            rcItem.OffsetRect(m_rcWindow.left,m_rcWindow.top-m_ptOrigin.y+iVisible*m_nItemHei);
            DrawItem(pRT,rcItem,hItem);
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
        hItem=CSTree<STreeItem*>::GetNextItem(hItem);
    }
    AfterPaint(pRT,painter);
}

void STreeBox::OnLButtonDown(UINT nFlags,CPoint pt)
{
    if(m_bTabStop) SetFocus();
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
        DUINMTBSELCHANGING nms2;
        nms2.hdr.code=NM_TBSELCHANGING;
        nms2.hdr.hDuiWnd=m_hSWnd;
        nms2.hdr.idFrom=GetID();
        nms2.hdr.pszNameFrom=GetName();
        nms2.hOldSel=m_hSelItem;
        nms2.hNewSel=m_hHoverItem;
        nms2.bCancel=FALSE;
        FireEvent((LPSNMHDR)&nms2);

        if(!nms2.bCancel)
        {
            DUINMTBSELCHANGED nms;
            nms.hdr.code=NM_TBSELCHANGED;
            nms.hdr.hDuiWnd=m_hSWnd;
            nms.hdr.idFrom=GetID();
            nms.hdr.pszNameFrom=GetName();
            nms.hOldSel=m_hSelItem;
            nms.hNewSel=m_hHoverItem;

            if(m_hSelItem)
            {
                CSTree<STreeItem*>::GetItem(m_hSelItem)->GetFocusManager()->SetFocusedHwnd(0);
                CSTree<STreeItem*>::GetItem(m_hSelItem)->ModifyItemState(0,DuiWndState_Check);
                RedrawItem(m_hSelItem);
            }
            m_hSelItem=m_hHoverItem;
            if(m_hSelItem)
            {
                CSTree<STreeItem*>::GetItem(m_hSelItem)->ModifyItemState(DuiWndState_Check,0);
                RedrawItem(m_hSelItem);
            }
            FireEvent((LPSNMHDR)&nms);
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
    else
    {
        DUINMITEMMOUSEEVENT nms;
        nms.hdr.code=NM_ITEMMOUSEEVENT;
        nms.hdr.hDuiWnd=m_hSWnd;
        nms.hdr.idFrom=GetID();
        nms.hdr.pszNameFrom=GetName();
        nms.pItem=NULL;
        nms.uMsg=WM_LBUTTONDBLCLK;
        nms.wParam=nFlags;
        nms.lParam=MAKELPARAM(pt.x,pt.y);
        FireEvent((LPSNMHDR)&nms);
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

LRESULT STreeBox::FireEvent(LPSNMHDR pnms)
{
    if(pnms->code==NM_LBITEMNOTIFY)
    {
        DUINMITEMNOTIFY *pItemNotify=(DUINMITEMNOTIFY*)pnms;
        if(pItemNotify->pOriginHdr->idFrom==IDC_SWITCH)
        {
            STreeItem *pItem=dynamic_cast<STreeItem*> (pItemNotify->pItem);
            Expand(pItem->m_hItem,TVE_TOGGLE);
            return 0;
        }
    }
    return __super::DuiNotify(pnms);
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
    STreeItem *pItemObj=dynamic_cast<STreeItem*>(pItem);
    if(pItemObj->m_bVisible==FALSE) return FALSE;

    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(m_rcWindow.Height()+m_nItemHei-1)/m_nItemHei+1;

    int iVisible=-1;
    HSTREEITEM hItem=CSTree<STreeItem*>::GetNextItem(STVI_ROOT);
    while(hItem)
    {
        STreeItem *pItem=CSTree<STreeItem*>::GetItem(hItem);
        if(pItem->m_bVisible) iVisible++;
        if(iVisible > iFirstVisible+nPageItems) break;
        if(iVisible>=iFirstVisible && pItem==pItemObj)
        {
            CRect rcRet(m_nIndent*pItemObj->m_nLevel,0,m_rcWindow.Width(),m_nItemHei);
            rcRet.OffsetRect(m_rcWindow.left,m_rcWindow.top-m_ptOrigin.y+iVisible*m_nItemHei);
            rcItem=rcRet;
            return TRUE;
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
        hItem=CSTree<STreeItem*>::GetNextItem(hItem);
    }
    return FALSE;
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

}//namespace SOUI