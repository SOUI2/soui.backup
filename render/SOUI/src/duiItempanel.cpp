//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiItemPanel
// Description: A Framework wrapping frame to be used in a duiwindow.
//     Creator: Huang Jianxiong
//     Version: 2011.10.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once
#include "duistd.h"
#include "duiItempanel.h"

#include "duiwndnotify.h"

#pragma warning(disable:4018)

namespace SOUI
{


CDuiItemPanel::CDuiItemPanel(SWindow *pFrameHost,pugi::xml_node xmlNode,IDuiItemContainer *pItemContainer)
    :CDuiFrame(this)
    ,m_pFrmHost(pFrameHost)
    ,m_pItemContainer(pItemContainer)
    ,m_dwData(0)
    ,m_crBk(CLR_INVALID)
    ,m_crSelBk(RGB(0,0,128))
    ,m_lpItemIndex(-1)
{
    DUIASSERT(m_pFrmHost);
    if(!m_pItemContainer) m_pItemContainer=dynamic_cast<IDuiItemContainer*>(m_pFrmHost);
    DUIASSERT(m_pItemContainer);
    SetContainer(this);
    if(xmlNode) Load(xmlNode);
}

void CDuiItemPanel::OnFinalRelease()
{
    AddRef();//防止重复进入该函数
    DuiSendMessage(WM_DESTROY);
    __super::OnFinalRelease();
}

LRESULT CDuiItemPanel::DoFrameEvent(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    AddRef();

    if(!IsDisabled() && m_pBgSkin && m_pBgSkin->GetStates()>1)
    {
        switch(uMsg)
        {
        case WM_MOUSEHOVER: 
            ModifyState(DuiWndState_Hover,0,TRUE);
            break;
        case WM_MOUSELEAVE: 
            ModifyState(0,DuiWndState_Hover,TRUE);
            break;
        }
    }

    SetMsgHandled(FALSE);
    LRESULT lRet=__super::DoFrameEvent(uMsg,wParam,lParam);
    if(IsMsgHandled())
    {
        DUINMITEMMOUSEEVENT nms;
        nms.hdr.code=NM_ITEMMOUSEEVENT;
        nms.hdr.hDuiWnd=m_pFrmHost->GetDuiHwnd();
        nms.hdr.idFrom=m_pFrmHost->GetCmdID();
        nms.hdr.pszNameFrom= m_pFrmHost->GetName();
        nms.pItem=this;
        nms.uMsg=uMsg;
        nms.wParam=wParam;
        nms.lParam=lParam;
        m_pFrmHost->DuiNotify((LPDUINMHDR)&nms);
    }
    Release();
    return lRet;
}

LRESULT CDuiItemPanel::OnDuiNotify(LPDUINMHDR pHdr)
{
    DUINMITEMNOTIFY nmsItem;
    nmsItem.hdr.code=NM_LBITEMNOTIFY;
    nmsItem.hdr.hDuiWnd=m_pFrmHost->GetDuiHwnd();
    nmsItem.hdr.idFrom=m_pFrmHost->GetCmdID();
    nmsItem.hdr.pszNameFrom=m_pFrmHost->GetName();
    nmsItem.pItem=this;
    nmsItem.pHostDuiWin=(SWindow*)m_pFrmHost;
    nmsItem.pOriginHdr=pHdr;
    return m_pFrmHost->DuiNotify((LPDUINMHDR)&nmsItem);
}

CRect CDuiItemPanel::GetContainerRect()
{
    CRect rcItem;
    m_pItemContainer->OnItemGetRect(this,rcItem);
    return rcItem;
}

HDC CDuiItemPanel::OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)
{
    CRect rcItem=GetItemRect();
    CRect rcInvalid=rc;
    rcInvalid.OffsetRect(rcItem.TopLeft());
    HDC hdc=m_pFrmHost->GetRenderTarget(rcInvalid,gdcFlags);
    if(gdcFlags & OLEDC_PAINTBKGND)
    {//调用frmhost的GetDuiDC时，不会绘制frmHost的背景。注意此外只画背景，不画前景,因为itempanel就是前景
        m_pFrmHost->DuiSendMessage(WM_ERASEBKGND, (WPARAM)hdc);
    }
    OffsetViewportOrgEx(hdc,rcItem.left,rcItem.top,NULL);
    return hdc;
}

void CDuiItemPanel::OnReleaseRenderTarget(HDC hdc,const CRect &rc,DWORD gdcFlags)
{
    CRect rcItem=GetItemRect();
    OffsetViewportOrgEx(hdc,-rcItem.left,-rcItem.top,NULL);
    m_pFrmHost->ReleaseRenderTarget(hdc);
}

void CDuiItemPanel::OnRedraw(const CRect &rc)
{
    if(m_pFrmHost->IsUpdateLocked()) return;

    CRect rcItem=GetItemRect();
    if(!rcItem.IsRectNull() && m_pFrmHost->IsVisible(TRUE))
    {
        if(m_pItemContainer->IsItemRedrawDelay())
        {
            CRect rc2(rc);
            rc2.OffsetRect(rcItem.TopLeft());
            rc2.IntersectRect(rc2,rcItem);
            m_pFrmHost->NotifyInvalidateRect(rc2);
        }else
        {
            CDCHandle dc=OnGetRenderTarget(rc,OLEDC_PAINTBKGND);
            CRgn rgn;
            rgn.CreateRectRgnIndirect(&rc);
            RedrawRegion(dc,rgn);
            OnReleaseRenderTarget(dc,rc,OLEDC_PAINTBKGND);
        }
    }
}

BOOL CDuiItemPanel::OnReleaseDuiCapture()
{
    if(!__super::OnReleaseDuiCapture()) return FALSE;
    m_pItemContainer->OnItemSetCapture(this,FALSE);
    return TRUE;
}

HSWND CDuiItemPanel::OnSetDuiCapture(HSWND hDuiWNd)
{
    m_pItemContainer->OnItemSetCapture(this,TRUE);
    return __super::OnSetDuiCapture(hDuiWNd);
}

HWND CDuiItemPanel::GetHostHwnd()
{
    return m_pFrmHost->GetContainer()->GetHostHwnd();
}

BOOL CDuiItemPanel::IsTranslucent()
{
    return m_pFrmHost->GetContainer()->IsTranslucent();
}

BOOL CDuiItemPanel::DuiCreateCaret( HBITMAP hBmp,int nWidth,int nHeight )
{
    return m_pFrmHost->GetContainer()->DuiCreateCaret(hBmp,nWidth,nHeight);
}

BOOL CDuiItemPanel::DuiShowCaret( BOOL bShow )
{
    return m_pFrmHost->GetContainer()->DuiShowCaret(bShow);
}

BOOL CDuiItemPanel::DuiSetCaretPos( int x,int y )
{
    CRect rcItem=GetItemRect();
    x+=rcItem.left,y+=rcItem.top;
    return m_pFrmHost->GetContainer()->DuiSetCaretPos(x,y);
}

BOOL CDuiItemPanel::DuiUpdateWindow()
{
    return m_pFrmHost->GetContainer()->DuiUpdateWindow();
}

void CDuiItemPanel::ModifyItemState(DWORD dwStateAdd, DWORD dwStateRemove)
{
    ModifyState(dwStateAdd,dwStateRemove,FALSE);
}

HSWND CDuiItemPanel::DuiGetHWNDFromPoint(POINT ptHitTest, BOOL bOnlyText)
{
    HSWND hRet=__super::DuiGetHWNDFromPoint(ptHitTest,bOnlyText);
    if(hRet==m_hSWnd) hRet=NULL;
    return hRet;
}

void CDuiItemPanel::Draw(CDCHandle dc,const CRect & rc)
{
    CRgn rgnNull;
    if((m_dwState & DuiWndState_Check) && m_crSelBk != CLR_INVALID) SetBkColor(m_crSelBk);
    else SetBkColor(m_crBk);

    dc.OffsetViewportOrg(rc.left,rc.top);
    RedrawRegion(dc,rgnNull);
    dc.OffsetViewportOrg(-rc.left,-rc.top);
}

void CDuiItemPanel::SetSkin(ISkinObj *pSkin)
{
    m_pBgSkin=pSkin;
}

void CDuiItemPanel::SetColor(COLORREF crBk,COLORREF crSelBk)
{
    m_crBk=crBk;
    m_crSelBk=crSelBk;
}

BOOL CDuiItemPanel::NeedRedrawWhenStateChange()
{
    return TRUE;
}

CRect CDuiItemPanel::GetItemRect()
{
    CRect rcItem;
    m_pItemContainer->OnItemGetRect(this,rcItem);
    return rcItem;
}
void CDuiItemPanel::SetItemCapture(BOOL bCapture)
{
    m_pItemContainer->OnItemSetCapture(this,bCapture);
}

void CDuiItemPanel::SetItemData(LPARAM dwData)
{
    m_dwData=dwData;
}

LPARAM CDuiItemPanel::GetItemData()
{
    return m_dwData;
}

BOOL CDuiItemPanel::OnUpdateToolTip( HSWND hCurTipHost,HSWND &hNewTipHost,CRect &rcTip,CDuiStringT &strTip )
{
    if(hCurTipHost==m_hHover) return FALSE;
    if(m_hHover==m_hSWnd) return FALSE;

    SWindow *pHover=DuiWindowMgr::GetWindow(m_hHover);
    if(!pHover || pHover->IsDisabled(TRUE))
    {
        hNewTipHost=NULL;
        return TRUE;
    }
    BOOL bRet=pHover->OnUpdateToolTip(hCurTipHost,hNewTipHost,rcTip,strTip);
    if(bRet)
    {
        CRect rcItem=GetItemRect();
        rcTip.OffsetRect(rcItem.TopLeft());
    }
    return bRet;
}

void CDuiItemPanel::OnSetCaretValidateRect( LPCRECT lpRect )
{
    CRect rcClient;
    GetClient(&rcClient);
    CRect rcIntersect;
    rcIntersect.IntersectRect(&rcClient,lpRect);
    CRect rcItem=GetItemRect();
    rcIntersect.OffsetRect(rcItem.TopLeft());
    m_pFrmHost->OnSetCaretValidateRect(&rcIntersect);
}

BOOL CDuiItemPanel::RegisterTimelineHandler( ITimelineHandler *pHandler )
{
    BOOL bRet=CDuiFrame::RegisterTimelineHandler(pHandler);
    if(bRet && m_lstTimelineHandler.GetCount()==1) m_pFrmHost->GetContainer()->RegisterTimelineHandler(this);
    return bRet;
}

BOOL CDuiItemPanel::UnregisterTimelineHandler( ITimelineHandler *pHandler )
{
    BOOL bRet=CDuiFrame::UnregisterTimelineHandler(pHandler);
    if(bRet && m_lstTimelineHandler.IsEmpty()) m_pFrmHost->GetContainer()->UnregisterTimelineHandler(this);
    return bRet;
}
}//namespace SOUI