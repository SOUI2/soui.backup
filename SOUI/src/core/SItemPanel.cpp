//////////////////////////////////////////////////////////////////////////
//  Class Name: SItemPanel
//     Creator: Huang Jianxiong
//     Version: 2011.10.20 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once
#include "souistd.h"
#include "core/SItempanel.h"

#pragma warning(disable:4018)

namespace SOUI
{


SItemPanel::SItemPanel(SWindow *pFrameHost,pugi::xml_node xmlNode,IItemContainer *pItemContainer)
    :SwndContainerImpl(this)
    ,m_pFrmHost(pFrameHost)
    ,m_pItemContainer(pItemContainer)
    ,m_dwData(0)
    ,m_crBk(CR_INVALID)
    ,m_crSelBk(RGBA(0,0,128,255))
    ,m_lpItemIndex(-1)
{
    SASSERT(m_pFrmHost);
    SASSERT(m_pItemContainer);
    SetContainer(this);
    if(xmlNode) 
    {
        InitFromXml(xmlNode);
        BuildWndTreeZorder();
    }
    m_evtSet.addEvent(EVT_ITEMPANEL_CLICK);
    m_evtSet.addEvent(EVT_ITEMPANEL_DBCLICK);
    m_evtSet.addEvent(EVT_ITEMPANEL_RCLICK);
}

void SItemPanel::OnFinalRelease()
{
    AddRef();//防止重复进入该函数
    SSendMessage(WM_DESTROY);
    __super::OnFinalRelease();
}

LRESULT SItemPanel::DoFrameEvent(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    AddRef();

    if(!IsDisabled() && m_pBgSkin && m_pBgSkin->GetStates()>1)
    {
        switch(uMsg)
        {
        case WM_MOUSEHOVER: 
            ModifyState(WndState_Hover,0,TRUE);
            break;
        case WM_MOUSELEAVE: 
            ModifyState(0,WndState_Hover,TRUE);
            break;
        }
    }

    if(uMsg == WM_LBUTTONDOWN)
    {
        EventCmnArgs evt(this,EVT_ITEMPANEL_CLICK);
        OnFireEvent(evt);
    }else if(uMsg == WM_RBUTTONDOWN)
    {
        EventCmnArgs evt(this,EVT_ITEMPANEL_RCLICK);
        OnFireEvent(evt);
    }else if(uMsg == WM_LBUTTONDBLCLK)
    {
        EventCmnArgs evt(this,EVT_ITEMPANEL_DBCLICK);
        OnFireEvent(evt);
    }

    SetMsgHandled(FALSE);
    LRESULT lRet=__super::DoFrameEvent(uMsg,wParam,lParam);
    Release();
    return lRet;
}

BOOL SItemPanel::OnFireEvent(EventArgs &evt)
{
    EventOfPanel evt2(this,&evt);
    evt2.nameFrom = m_pFrmHost->GetName();
    evt2.idFrom = m_pFrmHost->GetID();
    return m_pFrmHost->FireEvent(evt2);
}

CRect SItemPanel::GetContainerRect()
{
    CRect rcItem;
    m_pItemContainer->OnItemGetRect(this,rcItem);
    return rcItem;
}

IRenderTarget * SItemPanel::OnGetRenderTarget(const CRect & rc,DWORD gdcFlags)
{
    CRect rcItem=GetItemRect();
    CRect rcInvalid=rc;
    rcInvalid.OffsetRect(rcItem.TopLeft());
    IRenderTarget *pRT=m_pFrmHost->GetRenderTarget(rcInvalid,gdcFlags);
    if(gdcFlags == OLEDC_PAINTBKGND)
    {//调用frmhost的GetRenderTarget时，不会绘制frmHost的背景。注意此外只画背景，不画前景,因为itempanel就是前景
        m_pFrmHost->SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
    }
    pRT->OffsetViewportOrg(rcItem.left,rcItem.top);
    return pRT;
}

void SItemPanel::OnReleaseRenderTarget(IRenderTarget *pRT,const CRect &rc,DWORD gdcFlags)
{
    CRect rcItem=GetItemRect();
    pRT->OffsetViewportOrg(-rcItem.left,-rcItem.top);
    m_pFrmHost->ReleaseRenderTarget(pRT);
}

void SItemPanel::OnRedraw(const CRect &rc)
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
            m_pFrmHost->InvalidateRect(rc2);
        }else
        {
            IRenderTarget *pRT=OnGetRenderTarget(rc,OLEDC_PAINTBKGND);
            CAutoRefPtr<IRegion> rgn;
            GETRENDERFACTORY->CreateRegion(&rgn);
            rgn->CombineRect(&rc,RGN_COPY);
            RedrawRegion(pRT,rgn);
            OnReleaseRenderTarget(pRT,rc,OLEDC_PAINTBKGND);
        }
    }
}

BOOL SItemPanel::OnReleaseSwndCapture()
{
    if(!__super::OnReleaseSwndCapture()) return FALSE;
    m_pItemContainer->OnItemSetCapture(this,FALSE);
    return TRUE;
}

SWND SItemPanel::OnSetSwndCapture(SWND swnd)
{
    m_pItemContainer->OnItemSetCapture(this,TRUE);
    return __super::OnSetSwndCapture(swnd);
}

HWND SItemPanel::GetHostHwnd()
{
    return m_pFrmHost->GetContainer()->GetHostHwnd();
}

const SStringW & SItemPanel::GetTranslatorContext()
{
    return m_pFrmHost->GetContainer()->GetTranslatorContext();
}

BOOL SItemPanel::IsTranslucent()
{
    return m_pFrmHost->GetContainer()->IsTranslucent();
}

BOOL SItemPanel::SwndCreateCaret( HBITMAP hBmp,int nWidth,int nHeight )
{
    return m_pFrmHost->GetContainer()->SwndCreateCaret(hBmp,nWidth,nHeight);
}

BOOL SItemPanel::SwndShowCaret( BOOL bShow )
{
    return m_pFrmHost->GetContainer()->SwndShowCaret(bShow);
}

BOOL SItemPanel::SwndSetCaretPos( int x,int y )
{
    CRect rcItem=GetItemRect();
    x+=rcItem.left,y+=rcItem.top;
    return m_pFrmHost->GetContainer()->SwndSetCaretPos(x,y);
}

BOOL SItemPanel::SwndUpdateWindow()
{
    return m_pFrmHost->GetContainer()->SwndUpdateWindow();
}

void SItemPanel::ModifyItemState(DWORD dwStateAdd, DWORD dwStateRemove)
{
    ModifyState(dwStateAdd,dwStateRemove,FALSE);
}

SWND SItemPanel::SwndFromPoint(POINT ptHitTest, BOOL bOnlyText)
{
    SWND hRet=__super::SwndFromPoint(ptHitTest,bOnlyText);
    if(hRet==m_swnd) hRet=NULL;
    return hRet;
}

void SItemPanel::Draw(IRenderTarget *pRT,const CRect & rc)
{
    if((m_dwState & WndState_Check) && m_crSelBk != CR_INVALID) m_style.m_crBg=m_crSelBk;
    else m_style.m_crBg=m_crBk;

    pRT->OffsetViewportOrg(rc.left,rc.top);
    CAutoRefPtr<IRegion> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    BuildWndTreeZorder();
    RedrawRegion(pRT,rgn);
    pRT->OffsetViewportOrg(-rc.left,-rc.top);
}

void SItemPanel::SetSkin(ISkinObj *pSkin)
{
    m_pBgSkin=pSkin;
}

void SItemPanel::SetColor(COLORREF crBk,COLORREF crSelBk)
{
    m_crBk=crBk;
    m_crSelBk=crSelBk;
}

BOOL SItemPanel::NeedRedrawWhenStateChange()
{
    return TRUE;
}

CRect SItemPanel::GetItemRect()
{
    CRect rcItem;
    m_pItemContainer->OnItemGetRect(this,rcItem);
    return rcItem;
}
void SItemPanel::SetItemCapture(BOOL bCapture)
{
    m_pItemContainer->OnItemSetCapture(this,bCapture);
}

void SItemPanel::SetItemData(LPARAM dwData)
{
    m_dwData=dwData;
}

LPARAM SItemPanel::GetItemData()
{
    return m_dwData;
}

BOOL SItemPanel::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    CRect rcItem=GetItemRect();
    if(m_hHover==m_swnd)
    {
        tipInfo.swnd = m_swnd;
        tipInfo.dwCookie =0;
        tipInfo.rcTarget = rcItem;
        tipInfo.strTip = m_strToolTipText;
        return TRUE;
    }
    
    SWindow *pHover=SWindowMgr::GetWindow(m_hHover);
    if(!pHover || pHover->IsDisabled(TRUE))
    {
        tipInfo.swnd=0;
        return TRUE;
    }
    
    pt -= rcItem.TopLeft();
    BOOL bRet=pHover->OnUpdateToolTip(pt,tipInfo);
    if(bRet)
    {
        tipInfo.rcTarget.OffsetRect(rcItem.TopLeft());
    }
    return bRet;
}

void SItemPanel::OnSetCaretValidateRect( LPCRECT lpRect )
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcIntersect;
    rcIntersect.IntersectRect(&rcClient,lpRect);
    CRect rcItem=GetItemRect();
    rcIntersect.OffsetRect(rcItem.TopLeft());
    m_pFrmHost->OnSetCaretValidateRect(&rcIntersect);
}

BOOL SItemPanel::RegisterTimelineHandler( ITimelineHandler *pHandler )
{
    BOOL bRet=SwndContainerImpl::RegisterTimelineHandler(pHandler);
    if(bRet && m_lstTimelineHandler.GetCount()==1) m_pFrmHost->GetContainer()->RegisterTimelineHandler(this);
    return bRet;
}

BOOL SItemPanel::UnregisterTimelineHandler( ITimelineHandler *pHandler )
{
    BOOL bRet=SwndContainerImpl::UnregisterTimelineHandler(pHandler);
    if(bRet && m_lstTimelineHandler.IsEmpty()) m_pFrmHost->GetContainer()->UnregisterTimelineHandler(this);
    return bRet;
}

SMessageLoop * SItemPanel::GetMsgLoop()
{
    return m_pFrmHost->GetContainer()->GetMsgLoop();
}

IScriptModule * SItemPanel::GetScriptModule()
{
    return m_pFrmHost->GetContainer()->GetScriptModule();
}

}//namespace SOUI