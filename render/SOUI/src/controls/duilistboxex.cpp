//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiListBoxEx
// Description: A DuiWindow Based ListBox Control. Can contain control as an item
//     Creator: Huang Jianxiong
//     Version: 2011.8.27 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once
#include "duistd.h"
#include "control/duilistboxex.h"
#include "DuiSystem.h"
#include "mybuffer.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

namespace SOUI
{


CDuiListBoxEx::CDuiListBoxEx()
    : m_nItemHei(20)
    , m_iScrollSpeed(-1)
    , m_iSelItem(-1)
    , m_iHoverItem(-1)
    , m_pCapturedFrame(NULL)
    , m_pTemplPanel(NULL)
    , m_nItems(0)
    , m_pItemSkin(NULL)
    , m_crItemBg(CLR_INVALID)
    , m_crItemSelBg(RGB(0,0,128))
    , m_bVirtual(FALSE)
    , m_bItemRedrawDelay(TRUE)
{
    m_bTabStop=TRUE;
    addEvent(NM_LBITEMNOTIFY);
    addEvent(NM_ITEMMOUSEEVENT);
    addEvent(NM_GETLBDISPINFO);
    addEvent(NM_LBSELCHANGING);
    addEvent(NM_LBSELCHANGED);
}

CDuiListBoxEx::~CDuiListBoxEx()
{
}


void CDuiListBoxEx::DeleteAllItems(BOOL bUpdate/*=TRUE*/)
{
    if(IsVirtual())
    {
        m_nItems=0;
    }
    else
    {
        for(int i=0; i<GetItemCount(); i++)
        {
            m_arrItems[i]->Release();
        }
        m_arrItems.RemoveAll();
    }
    m_iSelItem=-1;
    m_iHoverItem=-1;
    m_pCapturedFrame=NULL;
    ReleaseDuiCapture();

    SetViewSize(CSize(0,0));
    if(bUpdate) NotifyInvalidate();
}

void CDuiListBoxEx::DeleteItem(int iItem)
{
    if(IsVirtual()) return;

    if(iItem<0 || iItem>=GetItemCount()) return;
    if(m_pCapturedFrame == m_arrItems[iItem])
    {
        m_pCapturedFrame=NULL;
        ReleaseDuiCapture();
    }

    m_arrItems[iItem]->Release();
    m_arrItems.RemoveAt(iItem);

    if(m_iSelItem==iItem) m_iSelItem=-1;
    else if(m_iSelItem>iItem) m_iSelItem--;
    if(m_iHoverItem==iItem) m_iHoverItem=-1;
    else if(m_iHoverItem>iItem) m_iHoverItem--;

    UpdatePanelsIndex(iItem,-1);

    CRect rcClient;
    CDuiWindow::GetClient(&rcClient);
    CSize szView(rcClient.Width(),GetItemCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);
}

int CDuiListBoxEx::InsertItem(int iItem,pugi::xml_node xmlNode,DWORD dwData/*=0*/)
{
    if(IsVirtual()) return -1;

    CDuiItemPanel *pItemObj=new CDuiItemPanel(this,xmlNode,this);

    if(iItem==-1 || iItem>=GetItemCount())
    {
        iItem=GetItemCount();
    }
    pItemObj->SetItemData(dwData);
    pItemObj->Move(CRect(0,0,m_rcClient.Width(),m_nItemHei));
    if(m_pItemSkin) pItemObj->SetSkin(m_pItemSkin);
    pItemObj->SetColor(m_crItemBg,m_crItemSelBg);

    m_arrItems.InsertAt(iItem,pItemObj);

    if(m_iSelItem>=iItem) m_iSelItem++;
    if(m_iHoverItem>=iItem) m_iHoverItem++;

    UpdatePanelsIndex(iItem,-1);

    CRect rcClient;
    CDuiWindow::GetClient(&rcClient);
    CSize szView(rcClient.Width(),GetItemCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);

    return iItem;
}

int CDuiListBoxEx::InsertItem(int iItem,LPCWSTR pszXml,DWORD dwData/*=0*/)
{
    if(IsVirtual()) return -1;

    if(!pszXml && !m_xmlTempl) return -1;
    if(pszXml)
    {
        CDuiStringA strUtf8=DUI_CW2A(pszXml,CP_UTF8);
        pugi::xml_document xmlDoc;
        if(!xmlDoc.load_buffer((LPCSTR)strUtf8,strUtf8.GetLength(),pugi::parse_default,pugi::encoding_utf8)) return -1;
        return InsertItem(iItem,xmlDoc.first_child(),dwData);
    }else
    {
        return InsertItem(iItem,m_xmlTempl.first_child(),dwData);
    }
}

BOOL CDuiListBoxEx::SetCurSel(int iItem)
{
    if(iItem<0 || iItem>=GetItemCount()) return FALSE;

    if(m_iSelItem==iItem) return FALSE;
    if(IsVirtual())
    {
        int nOldSel=m_iSelItem;
        m_iSelItem=iItem;
        if(IsVisible(TRUE))
        {
            if(nOldSel!=-1) RedrawItem(nOldSel);
            RedrawItem(m_iSelItem);
        }
    }else
    {
        int nOldSel=m_iSelItem;
        m_iSelItem=iItem;
        if(nOldSel!=-1)
        {
            m_arrItems[nOldSel]->ModifyItemState(0,DuiWndState_Check);
            if(IsVisible(TRUE)) RedrawItem(nOldSel);
        }
        m_arrItems[m_iSelItem]->ModifyItemState(DuiWndState_Check,0);
        if(IsVisible(TRUE)) RedrawItem(m_iSelItem);
    }
    return TRUE;
}

void CDuiListBoxEx::EnsureVisible( int iItem )
{
    if(iItem<0 || iItem>=GetItemCount()) return;
    int iFirstVisible=(m_ptOrigin.y + m_nItemHei -1) / m_nItemHei;
    CRect rcClient;
    GetClient(&rcClient);
    int nVisibleItems=rcClient.Height()/m_nItemHei;
    if(iItem<iFirstVisible || iItem> iFirstVisible+nVisibleItems-1)
    {
        int nOffset=GetScrollPos(TRUE);
        if(iItem<iFirstVisible) nOffset=(iItem-iFirstVisible)*m_nItemHei;
        else nOffset=(iItem - iFirstVisible-nVisibleItems +1)*m_nItemHei;
        nOffset-=nOffset%m_nItemHei;//让当前行刚好显示
        OnScroll(TRUE,SB_THUMBPOSITION,nOffset + GetScrollPos(TRUE));
    }
}

int CDuiListBoxEx::GetCurSel()
{
    return m_iSelItem;
}

int CDuiListBoxEx::GetItemObjIndex(CDuiWindow *pItemObj)
{
    CDuiItemPanel * pItemPanel= dynamic_cast<CDuiItemPanel *>(pItemObj);
    if (NULL == pItemPanel) return -1;
    return (int)pItemPanel->GetItemIndex();
}


CDuiWindow * CDuiListBoxEx::GetItemPanel(int iItem)
{
    if(iItem<0 || iItem>= GetItemCount()) return NULL;
    if(m_pTemplPanel) return m_pTemplPanel;
    else return m_arrItems[iItem];
}


LPARAM CDuiListBoxEx::GetItemData(int iItem)
{
    DUIASSERT(iItem>=0 || iItem< GetItemCount());
    if(m_pTemplPanel) return 0;
    return m_arrItems[iItem]->GetItemData();
}

void CDuiListBoxEx::SetItemData( int iItem,LPARAM lParam )
{
    DUIASSERT(iItem>=0 || iItem< GetItemCount());
    m_arrItems[iItem]->SetItemData(lParam);
}


BOOL CDuiListBoxEx::SetItemCount(int nItems,LPCTSTR pszXmlTemplate)
{
    if(m_arrItems.GetCount()!=0) return FALSE;
    if(pszXmlTemplate)
    {
        CDuiStringA strUtf8=DUI_CT2A(pszXmlTemplate,CP_UTF8);
        pugi::xml_document xmlDoc;
        if(!xmlDoc.load_buffer((LPCSTR)strUtf8,strUtf8.GetLength(),pugi::parse_default,pugi::encoding_utf8)) return FALSE;
        if(IsVirtual())
        {
            if(m_pTemplPanel)
            {
                m_pTemplPanel->DuiSendMessage(WM_DESTROY);
                m_pTemplPanel->Release();
            }
            m_pTemplPanel=new CDuiItemPanel(this,xmlDoc,this);
            if(m_pItemSkin) m_pTemplPanel->SetSkin(m_pItemSkin);
        }else
        {
            if(m_pTemplPanel) delete m_pTemplPanel;
            m_xmlTempl.append_copy(xmlDoc.first_child());
        }

    }
    if(IsVirtual())
    {
        DUIASSERT(m_pTemplPanel);
        m_nItems=nItems;
        CRect rcClient;
        CDuiWindow::GetClient(&rcClient);
        CSize szView(rcClient.Width(),GetItemCount()*m_nItemHei);
        if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
        SetViewSize(szView);
        GetClient(&rcClient);
        m_pTemplPanel->Move(0,0,rcClient.Width(),m_nItemHei);
        NotifyInvalidate();
        return TRUE;
    }else if(m_xmlTempl)
    {
        for(int i=0;i<nItems;i++)
        {
            InsertItem(i,m_xmlTempl.first_child());
        }
        return TRUE;
    }else
    {
        return FALSE;
    }

}

int CDuiListBoxEx::GetItemCount()
{
    if(IsVirtual()) 
        return m_nItems;
    else
        return m_arrItems.GetCount();
}

void CDuiListBoxEx::RedrawItem(int iItem)
{
    if(!IsVisible(TRUE)) return;
    CRect rcClient;
    GetClient(&rcClient);
    CRect rcItem=GetItemRect(iItem);
    CRect rcInter;
    rcInter.IntersectRect(&rcClient,&rcItem);
    if(rcInter.IsRectEmpty()) return;

    CDCHandle dc=GetDuiDC(&rcItem,OLEDC_PAINTBKGND);
    SPainter duiDC;
    BeforePaint(dc,duiDC);

    DuiSendMessage(WM_ERASEBKGND,(WPARAM)(HDC)dc);
    OnDrawItem(dc,rcItem,iItem);

    AfterPaint(dc,duiDC);
    ReleaseDuiDC(dc);
}

//自动修改pt的位置为相对当前项的偏移量
int CDuiListBoxEx::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClient(&rcClient);
    CPoint pt2=pt;
    pt2.y -= rcClient.top - m_ptOrigin.y;
    int nRet=pt2.y/m_nItemHei;
    if(nRet >= GetItemCount()) nRet=-1;
    else
    {
        pt.x-=rcClient.left;
        pt.y=pt2.y%m_nItemHei;
    }

    return nRet;
}

int CDuiListBoxEx::GetScrollLineSize(BOOL bVertical)
{
    return m_iScrollSpeed >0 ? m_iScrollSpeed : m_nItemHei;
}

void CDuiListBoxEx::OnPaint(CDCHandle dc)
{
    SPainter duiDC;
    BeforePaint(dc,duiDC);

    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(m_rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    CRect rcClip,rcInter;
    int nClip=dc.GetClipBox(&rcClip);

    for(int iItem = iFirstVisible; iItem<GetItemCount() && iItem <iFirstVisible+nPageItems; iItem++)
    {
        CRect rcItem(0,0,m_rcClient.Width(),m_nItemHei);
        rcItem.OffsetRect(0,m_nItemHei*iItem-m_ptOrigin.y);
        rcItem.OffsetRect(m_rcClient.TopLeft());
        rcInter.IntersectRect(&rcClip,&rcItem);
        if(nClip==NULLREGION || !rcInter.IsRectEmpty())
            OnDrawItem(dc,rcItem,iItem);
    }

    AfterPaint(dc,duiDC);
}


void CDuiListBoxEx::OnSize( UINT nType, CSize size )
{
    __super::OnSize(nType,size);
    Relayout();
}

void CDuiListBoxEx::OnDrawItem(CDCHandle & dc, CRect & rc, int iItem)
{
    if(IsVirtual())
    {//虚拟列表，由APP控制显示
        DUIASSERT(m_pTemplPanel);
        DUINMGETLBDISPINFO nms;
        nms.hdr.hDuiWnd=m_hDuiWnd;
        nms.hdr.code    = NM_GETLBDISPINFO;
        nms.hdr.idFrom  = GetCmdID();
        nms.hdr.pszNameFrom= GetName();
        nms.bHover      = iItem == m_iHoverItem;
        nms.bSelect     = iItem == m_iSelItem;
        nms.nListItemID = iItem;
        nms.pItem = m_pTemplPanel;
        nms.pHostDuiWin   = this;

        m_pTemplPanel->LockUpdate();

        m_pTemplPanel->SetItemIndex(iItem);

        if(!nms.bSelect) m_pTemplPanel->GetFocusManager()->StoreFocusedView();
        else m_pTemplPanel->GetFocusManager()->RestoreFocusedView();

        DWORD dwState=0;
        if(nms.bHover) dwState|=DuiWndState_Hover;
        if(nms.bSelect) dwState|=DuiWndState_PushDown;
        m_pTemplPanel->ModifyItemState(dwState,-1);

        m_pTemplPanel->UnlockUpdate();

        LockUpdate();
        GetContainer()->OnDuiNotify((LPDUINMHDR)&nms);
        UnlockUpdate();
        m_pTemplPanel->Draw(dc,rc);
    }else
    {
        m_arrItems[iItem]->Draw(dc,rc);
    }
}

BOOL CDuiListBoxEx::Load(pugi::xml_node xmlNode)
{
    if (!__super::Load(xmlNode))
        return FALSE;

    CDuiStringT strSrcName= DUI_CA2T(xmlNode.attribute("itemsrc").value(),CP_UTF8);

    if (strSrcName.IsEmpty())
        return TRUE;

    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,strSrcName,DUIRES_XML_TYPE))
        return FALSE;

    return LoadChildren(xmlDoc);
}

BOOL CDuiListBoxEx::LoadChildren(pugi::xml_node xmlNode)
{
    if(!xmlNode) return TRUE;

    pugi::xml_node xmlParent=xmlNode.parent();
    pugi::xml_node xmlTempl=xmlParent.child("template");
    pugi::xml_node xmlItems=xmlParent.child("items");

    if(!IsVirtual())
    {//普通列表
        if(xmlTempl) m_xmlTempl.append_copy(xmlTempl);

        if(xmlItems)
        {
            pugi::xml_node xmlItem=xmlItems.first_child();
            while(xmlItem)
            {
                if(strcmp(xmlItem.name(),"dlg")==0 || strcmp(xmlItem.name(),"item")==0)
                {
                    int dwData=xmlItem.attribute("itemdata").as_int(0);
                    InsertItem(-1,xmlItem,dwData);
                }
                xmlItem=xmlItem.next_sibling();
            }
            SetCurSel(xmlItems.attribute("cursel").as_int(-1));
        }

        return TRUE;
    }else
    {//虚拟列表
        DUIASSERT(xmlTempl);
        m_pTemplPanel=new CDuiItemPanel(this,xmlTempl,this);
        if(m_pItemSkin) m_pTemplPanel->SetSkin(m_pItemSkin);
        return TRUE;
    }
}


void CDuiListBoxEx::NotifySelChange( int nOldSel,int nNewSel)
{
    DUINMLBSELCHANGE nms;
    nms.hdr.code=NM_LBSELCHANGING;
    nms.hdr.hDuiWnd=m_hDuiWnd;
    nms.hdr.idFrom=GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.nOldSel=nOldSel;
    nms.nNewSel=nNewSel;
    nms.uHoverID=0;
    if(nNewSel!=-1)
    {
        if(IsVirtual())
        {
            DUIASSERT(m_pTemplPanel);
            CDuiWindow *pHover=DuiWindowMgr::GetWindow(m_pTemplPanel->GetDuiHover());
            if(pHover) nms.uHoverID=pHover->GetCmdID();
        }else
        {
            CDuiWindow *pHover=DuiWindowMgr::GetWindow(m_arrItems[nNewSel]->GetDuiHover());
            if(pHover) nms.uHoverID=pHover->GetCmdID();
        }
    }

    if(S_OK!=DuiNotify((LPDUINMHDR)&nms)) return ;

    m_iSelItem=nNewSel;
    if(nOldSel!=-1)
    {
        if(!m_pTemplPanel) m_arrItems[nOldSel]->ModifyItemState(0,DuiWndState_Check);
        RedrawItem(nOldSel);
    }
    if(m_iSelItem!=-1)
    {
        if(!m_pTemplPanel) m_arrItems[m_iSelItem]->ModifyItemState(DuiWndState_Check,0);
        RedrawItem(m_iSelItem);
    }

    nms.hdr.idFrom=GetCmdID();
    nms.hdr.code=NM_LBSELCHANGED;
    DuiNotify((LPDUINMHDR)&nms);
}

void CDuiListBoxEx::OnMouseLeave()
{
    __super::OnMouseLeave();
    if(m_iHoverItem!=-1)
    {
        int nOldHover=m_iHoverItem;
        m_iHoverItem=-1;
        if(IsVirtual())
        {
            DUIASSERT(m_pTemplPanel);
            RedrawItem(nOldHover);
            m_pTemplPanel->DoFrameEvent(WM_MOUSELEAVE,0,0);
        }
        else
            m_arrItems[nOldHover]->DoFrameEvent(WM_MOUSELEAVE,0,0);
    }
}

BOOL CDuiListBoxEx::OnDuiSetCursor(const CPoint &pt)
{
    BOOL bRet=FALSE;
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        bRet=m_pCapturedFrame->DoFrameEvent(WM_SETCURSOR,0,MAKELPARAM(pt.x-rcItem.left,pt.y-rcItem.top))!=0;
    }
    else if(m_iHoverItem!=-1)
    {
        CRect rcItem=GetItemRect(m_iHoverItem);
        if(IsVirtual())
        {
            DUIASSERT(m_pTemplPanel);
            bRet=m_pTemplPanel->DoFrameEvent(WM_SETCURSOR,0,MAKELPARAM(pt.x-rcItem.left,pt.y-rcItem.top))!=0;
        }else
        {
            bRet=m_arrItems[m_iHoverItem]->DoFrameEvent(WM_SETCURSOR,0,MAKELPARAM(pt.x-rcItem.left,pt.y-rcItem.top))!=0;
        }
    }
    if(!bRet)
    {
        bRet=__super::OnDuiSetCursor(pt);
    }
    return bRet;
}

void CDuiListBoxEx::OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
{
    int  nNewSelItem = -1;
    CDuiWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_ESCAPE))
    {
        pOwner->DuiSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    if (nChar == VK_DOWN && m_iSelItem < GetItemCount() - 1)
        nNewSelItem = m_iSelItem+1;
    else if (nChar == VK_UP && m_iSelItem > 0)
        nNewSelItem = m_iSelItem-1;
    else if (pOwner && nChar == VK_RETURN)
        nNewSelItem = m_iSelItem;

    if(nNewSelItem!=-1)
    {
        EnsureVisible(nNewSelItem);
        NotifySelChange(m_iSelItem,nNewSelItem);
    }
}

void CDuiListBoxEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CDuiWindow *pOwner = GetOwner();
    if (pOwner)
        pOwner->DuiSendMessage(WM_CHAR, nChar, MAKELONG(nFlags, nRepCnt));
}

UINT CDuiListBoxEx::OnGetDuiCode()
{
    return DUIC_WANTALLKEYS;
}

void CDuiListBoxEx::OnDestroy()
{
    DeleteAllItems(FALSE);
    if(IsVirtual())
    {
        DUIASSERT(m_pTemplPanel);
        m_pTemplPanel->DuiSendMessage(WM_DESTROY);
        m_pTemplPanel->Release();
        m_pTemplPanel=NULL;
    }
    __super::OnDestroy();
}

BOOL CDuiListBoxEx::OnUpdateToolTip(HDUIWND hCurTipHost,HDUIWND &hNewTipHost,CRect &rcTip,CDuiStringT &strTip)
{
    if(m_iHoverItem==-1)
        return __super::OnUpdateToolTip(hCurTipHost,hNewTipHost,rcTip,strTip);
    else if(IsVirtual())
        return m_pTemplPanel->OnUpdateToolTip(hCurTipHost,hNewTipHost,rcTip,strTip);
    else
        return m_arrItems[m_iHoverItem]->OnUpdateToolTip(hCurTipHost,hNewTipHost,rcTip,strTip);
}

void CDuiListBoxEx::OnItemSetCapture(CDuiItemPanel *pItem,BOOL bCapture )
{
    if(bCapture)
    {
        SetDuiCapture();
        m_pCapturedFrame=pItem;
    }
    else if(pItem==m_pCapturedFrame)
    {
        ReleaseDuiCapture();
        m_pCapturedFrame=NULL;
    }
}


CRect CDuiListBoxEx::GetItemRect( int iItem )
{
    CRect rcClient;
    GetClient(&rcClient);
    CRect rcRet(CPoint(0,iItem*m_nItemHei),CSize(rcClient.Width(),m_nItemHei));
    rcRet.OffsetRect(rcClient.TopLeft()-m_ptOrigin);
    return rcRet;
}

BOOL CDuiListBoxEx::OnItemGetRect(CDuiItemPanel *pItem,CRect &rcItem )
{
    int iItem=pItem->GetItemIndex();
    rcItem=GetItemRect(iItem);
    return TRUE;
}

LRESULT CDuiListBoxEx::OnMouseEvent( UINT uMsg,WPARAM wParam,LPARAM lParam )
{
    LRESULT lRet=0;
    CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    if(m_pCapturedFrame)
    {
        CRect rcItem=m_pCapturedFrame->GetItemRect();
        pt.Offset(-rcItem.TopLeft());
        lRet = m_pCapturedFrame->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
    }
    else
    {
        if(m_bTabStop && (uMsg==WM_LBUTTONDOWN || uMsg== WM_RBUTTONDOWN || uMsg==WM_LBUTTONDBLCLK))
            SetDuiFocus();
        int iHover=HitTest(pt);
        if(iHover!=m_iHoverItem)
        {
            int nOldHover=m_iHoverItem;
            m_iHoverItem=iHover;
            if(nOldHover!=-1)
            {
                RedrawItem(nOldHover);
                if(!IsVirtual())
                {
                    m_arrItems[nOldHover]->DoFrameEvent(WM_MOUSELEAVE,0,0);
                }else
                {
                    DUIASSERT(m_pTemplPanel);
                    m_pTemplPanel->DoFrameEvent(WM_MOUSELEAVE,0,0);
                }
            }
            if(m_iHoverItem!=-1)
            {
                RedrawItem(m_iHoverItem);
                if(!IsVirtual())
                {
                    m_arrItems[m_iHoverItem]->DoFrameEvent(WM_MOUSEHOVER,wParam,MAKELPARAM(pt.x,pt.y));
                }else
                {
                    DUIASSERT(m_pTemplPanel);
                    m_pTemplPanel->DoFrameEvent(WM_MOUSEHOVER,wParam,MAKELPARAM(pt.x,pt.y));
                }
            }
        }
        if(uMsg==WM_LBUTTONDOWN && m_iSelItem!=-1 && m_iSelItem != m_iHoverItem && !IsVirtual())
        {//选择一个新行的时候原有行失去焦点
            m_arrItems[m_iSelItem]->GetFocusManager()->SetFocusedHwnd(0);
        }
        if(m_iHoverItem!=-1)
        {
            if(!IsVirtual())
            {
                m_arrItems[m_iHoverItem]->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
            }else
            {
                DUIASSERT(m_pTemplPanel);
                m_pTemplPanel->DoFrameEvent(uMsg,wParam,MAKELPARAM(pt.x,pt.y));
            }
        }
    }
    if(uMsg==WM_LBUTTONUP && m_iHoverItem!=m_iSelItem)
        NotifySelChange(m_iSelItem,m_iHoverItem);
    return 0;
}

LRESULT CDuiListBoxEx::OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam )
{
    LRESULT lRet=0;
    if(m_pCapturedFrame)
    {
        lRet=m_pCapturedFrame->DoFrameEvent(uMsg,wParam,lParam);
        SetMsgHandled(m_pCapturedFrame->IsMsgHandled());
    }
    else if(m_iSelItem!=-1)
    {
        if(!IsVirtual())
        {
            lRet=m_arrItems[m_iSelItem]->DoFrameEvent(uMsg,wParam,lParam);
            SetMsgHandled(m_arrItems[m_iSelItem]->IsMsgHandled());
        }else
        {
            m_pTemplPanel->DoFrameEvent(uMsg,wParam,lParam);
            SetMsgHandled(m_pTemplPanel->IsMsgHandled());
        }
    }else
    {
        SetMsgHandled(FALSE);
    }
    return lRet;
}

//同步在CDuiItemPanel中的index属性，在执行了插入，删除等操作后使用
void CDuiListBoxEx::UpdatePanelsIndex(UINT nFirst,UINT nLast)
{
    if(IsVirtual()) return;
    for(UINT i=nFirst;i<m_arrItems.GetCount() && i<nLast;i++)
    {
        m_arrItems[i]->SetItemIndex(i);
    }
}

void CDuiListBoxEx::OnSetDuiFocus()
{
    __super::OnSetDuiFocus();
    if(IsVirtual())
    {
        m_pTemplPanel->DoFrameEvent(WM_SETFOCUS,0,0);
    }else
    {
        if(m_iSelItem!=-1) m_arrItems[m_iSelItem]->DoFrameEvent(WM_SETFOCUS,0,0);
    }
}

void CDuiListBoxEx::OnKillDuiFocus()
{
    __super::OnKillDuiFocus();
    if(IsVirtual())
    {
        m_pTemplPanel->DoFrameEvent(WM_KILLFOCUS,0,0);
    }else
    {
        if(m_iSelItem!=-1) m_arrItems[m_iSelItem]->DoFrameEvent(WM_KILLFOCUS,0,0);
    }
    if(m_iSelItem!=-1) RedrawItem(m_iSelItem);
}

LRESULT CDuiListBoxEx::OnNcCalcSize( BOOL bCalcValidRects, LPARAM lParam )
{
    LRESULT lRet=__super::OnNcCalcSize(bCalcValidRects,lParam);
    Relayout();
    return lRet;
}

void CDuiListBoxEx::Relayout()
{
    if(IsVirtual())
    {
        DUIASSERT(m_pTemplPanel);
        m_pTemplPanel->Move(CRect(0,0,m_rcClient.Width(),m_nItemHei));
    }
    else
    {
        for(int i=0; i<GetItemCount(); i++)
            m_arrItems[i]->Move(CRect(0,0,m_rcClient.Width(),m_nItemHei));
    }

}

void CDuiListBoxEx::OnViewOriginChanged( CPoint ptOld,CPoint ptNew )
{
    if(m_iSelItem!=-1 && GetContainer()->GetDuiFocus()==m_hDuiWnd)
    {//这里需要重新设置一下选中行的焦点状态来更新光标位置
        m_arrItems[m_iSelItem]->DoFrameEvent(WM_KILLFOCUS,0,0);
        m_arrItems[m_iSelItem]->DoFrameEvent(WM_SETFOCUS,0,0);
    }
}

BOOL CDuiListBoxEx::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
    return __super::OnMouseWheel(nFlags,zDelta,pt);
}
}//namespace SOUI