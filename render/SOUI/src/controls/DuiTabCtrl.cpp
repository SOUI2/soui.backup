#include "duistd.h"
#include "control/duitabctrl.h"

namespace SOUI
{
    class CDuiTabSlider : public CDuiWindow
    {
        SOUI_CLASS_NAME(CDuiTabSlider, "tabslider")

    public:
        CDuiTabSlider( CDuiTabCtrl *pTabCtrl =NULL) :m_hMemDC(NULL),m_hBmp(NULL)
        {
            if(pTabCtrl)
            {
                CRect rcPage=pTabCtrl->GetChildrenLayoutRect();
                CDCHandle dcPage=pTabCtrl->GetDuiDC(&rcPage,OLEDC_NODRAW);
                m_hMemDC=CreateCompatibleDC(dcPage);
                m_hBmp=CGdiAlpha::CreateBitmap32(dcPage,rcPage.Width()*2,rcPage.Height());
                ::SelectObject(m_hMemDC,m_hBmp);
                pTabCtrl->ReleaseDuiDC(dcPage);
                pTabCtrl->InsertChild(this);
                Move(rcPage);
                SetVisible(FALSE);
            }
        }

        virtual ~CDuiTabSlider()
        {
            if(GetParent())
            {
                GetParent()->RemoveChild(this);
                if(m_hMemDC)    DeleteDC(m_hMemDC);
                if(m_hBmp)    DeleteObject(m_hBmp);
                m_hMemDC=NULL;
                m_hBmp=NULL;
            }
        }

        void Slide(SLIDEDIR sd,int nSteps=20)
        {
            CRect rcPage=m_rcWindow;

            SetVisible(TRUE);

            BringWindowToTop();
            for(int i=0; i<nSteps; i++)
            {
                CDCHandle dcPage=GetDuiDC(NULL,OLEDC_OFFSCREEN);
                switch(sd)
                {
                case SD_LEFTRIGHT:
                    BitBlt(dcPage,rcPage.left,rcPage.top,rcPage.Width()*(i+1)/nSteps,rcPage.Height(),m_hMemDC,rcPage.Width()+rcPage.Width()*(nSteps-i-1)/nSteps,0,SRCCOPY);
                    BitBlt(dcPage,rcPage.left+rcPage.Width()*(i+1)/nSteps,rcPage.top,rcPage.Width()*(nSteps-i-1)/nSteps,rcPage.Height(),m_hMemDC,0,0,SRCCOPY);
                    break;
                case SD_RIGHTLEFT:
                    BitBlt(dcPage,rcPage.left,rcPage.top,rcPage.Width(),rcPage.Height(),m_hMemDC,rcPage.Width()*(i+1)/nSteps,0,SRCCOPY);
                    break;
                case SD_TOPBOTTOM:
                    BitBlt(dcPage,rcPage.left,rcPage.top+rcPage.Height()*(nSteps-i-1)/nSteps,rcPage.Width(),rcPage.Height()*(i+1)/nSteps,
                        m_hMemDC,rcPage.Width(),0,SRCCOPY);//new
                    BitBlt(dcPage,rcPage.left,rcPage.top,rcPage.Width(),rcPage.Height()*(nSteps-i-1)/nSteps,
                        m_hMemDC,0,rcPage.Height()*(i+1)/nSteps,SRCCOPY);//old
                    break;
                case SD_BOTTOMTOP:
                    BitBlt(dcPage,rcPage.left,rcPage.top,rcPage.Width(),rcPage.Height()*(i+1)/nSteps,
                        m_hMemDC,rcPage.Width(),rcPage.Height()*(nSteps-i-1)/nSteps,SRCCOPY);//new
                    BitBlt(dcPage,rcPage.left,rcPage.top+rcPage.Height()*(i+1)/nSteps,rcPage.Width(),rcPage.Height()*(nSteps-i-1)/nSteps,
                        m_hMemDC,0,0,SRCCOPY);//old
                    break;
                }
                PaintForeground(dcPage,&rcPage);
                ReleaseDuiDC(dcPage);
                Sleep(10);
            }

            SetVisible(FALSE);
        }

        void InitPage(BOOL bPage1)
        {
            CRect rcPage;
            GetRect(&rcPage);
            CDCHandle dcPage=GetDuiDC(&rcPage,OLEDC_NODRAW);
            PaintBackground(dcPage,&rcPage);
            BitBlt(m_hMemDC,bPage1?0:rcPage.Width(),0,rcPage.Width(),rcPage.Height(),dcPage,rcPage.left,rcPage.top,SRCCOPY);
            ReleaseDuiDC(dcPage);
        }
    protected:

        void OnPaint(CDCHandle dc)
        {
        }

        HDC m_hMemDC;
        HBITMAP m_hBmp;
        WND_MSG_MAP_BEGIN()
            MSG_WM_PAINT(OnPaint)
        WND_MSG_MAP_END()
    };


//////////////////////////////////////////////////////////////////////////
// CDuiTabCtrl

CDuiTabCtrl::CDuiTabCtrl() : m_nCurrentPage(0)
    , m_pSkinTab(NULL)
    , m_pSkinIcon(NULL)
    , m_pSkinSplitter(NULL)
    , m_pSkinFrame(NULL)
    , m_nTabSpacing(0)
    , m_nTabWidth(0)
    , m_nTabHeight(0)
    , m_nTabPos(0)
    , m_nFramePos(0)
    , m_nHoverTabItem(-1)
    , m_nTabAlign(AlignTop)
    , m_nAnimateSteps(0)
    , m_ptText(-1,-1)
{
    m_bTabStop=TRUE;
    addEvent(NM_TAB_SELCHANGING);
    addEvent(NM_TAB_SELCHANGED);
    addEvent(NM_TAB_ITEMHOVER);
    addEvent(NM_TAB_ITEMLEAVE);
}

void CDuiTabCtrl::OnPaint( CDCHandle dc )
{
    DuiDCPaint duiDC;
    BeforePaint(dc,duiDC);
    CRect rcTabs;
    CRect rcItem,rcItemPrev;
    CRect rcSplit;
    DWORD dwState;
    int nSaveDC=dc.SaveDC();
    CRgn rgnDraw;

    GetClient(&rcTabs);
    if(m_nTabAlign==AlignLeft)
        rcTabs.right=rcTabs.left+m_nTabWidth;
    else
        rcTabs.bottom=rcTabs.top+m_nTabHeight;

    if (m_pSkinFrame)
    {
        CRect rcFrame;
        GetClient(rcFrame);
        rgnDraw.CreateRectRgnIndirect(&rcFrame);
        dc.SelectClipRgn(rgnDraw,RGN_AND);

        switch (m_nTabAlign)
        {
        case AlignTop:
            rcFrame.top += m_nTabHeight + m_nFramePos;
            break;
        case AlignLeft:
            rcFrame.left += m_nTabWidth + m_nFramePos;
            break;
        }
        m_pSkinFrame->Draw(dc, rcFrame, DuiWndState_Normal);
    }
    else
    {
        rgnDraw.CreateRectRgnIndirect(&rcTabs);
        dc.SelectClipRgn(rgnDraw,RGN_AND);
    }

    for(int i=0; i<GetItemCount(); i++)
    {
        dwState=DuiWndState_Normal;
        if(i == m_nCurrentPage) dwState=DuiWndState_PushDown;
        else if(i== m_nHoverTabItem) dwState=DuiWndState_Hover;

        GetItemRect(i,rcItem);
        if(i>0)
        {
            rcSplit=rcItem;
            if(m_nTabAlign==AlignLeft)
            {
                rcSplit.top=rcItemPrev.bottom;
            }
            else
            {
                rcSplit.left=rcItemPrev.right;
            }
        }

        if(!rcSplit.IsRectEmpty() && m_pSkinSplitter)
        {
            m_pSkinSplitter->Draw(dc,rcSplit,0);
        }
        DrawItem(dc,rcItem,i,dwState);
        rcItemPrev=rcItem;
    }
    dc.RestoreDC(nSaveDC);
    
    if(GetContainer()->GetDuiFocus()==m_hDuiWnd && IsTabStop())
    {
        CRect rc;
        GetItemRect(m_nCurrentPage,rc);
        rc.DeflateRect(2,2);
        DuiDrawDefFocusRect(dc,&rc);
    }
    AfterPaint(dc,duiDC);
}

CRect CDuiTabCtrl::GetChildrenLayoutRect()
{
    CRect rcRet=__super::GetChildrenLayoutRect();
    if(m_nTabAlign==AlignLeft)
        rcRet.left+= (m_nTabWidth+m_nFramePos);
    else
        rcRet.top+= (m_nTabHeight+m_nFramePos);
    return rcRet;
}

void CDuiTabCtrl::OnLButtonDown( UINT nFlags, CPoint point )
{
    CRect rcItem;
    BOOL bClickMove = TRUE;
    int nTabCount=GetItemCount();
    for (int i = 0; i < nTabCount; i ++)
    {
        GetItemRect(i, rcItem);

        if (rcItem.PtInRect(point))
        {
            bClickMove = FALSE;
            if (i == m_nCurrentPage)
                continue;

            SetCurSel(i);
            break;
        }
    }
    if (bClickMove)
    {
        __super::OnLButtonDown(nFlags,point);
    }else
    {
        SetDuiFocus();
    }
}

BOOL CDuiTabCtrl::RemoveItem( int nIndex , int nSelPage/*=0*/)
{
    CDuiTab * pTab = GetItem(nIndex);

    DestroyChild(pTab);
    m_lstPages.RemoveAt(nIndex);

    if (m_nCurrentPage == nIndex)
    {
        if(nSelPage<0) nSelPage=0;
        if(nSelPage>=GetItemCount()) nSelPage=GetItemCount()-1;
        m_nCurrentPage=-1;
        SetCurSel(nSelPage);
    }else
    {
        if(m_nCurrentPage>nIndex) m_nCurrentPage--;

        CRect rcTitle;
        GetClient(rcTitle);
        if(m_nTabAlign==AlignLeft)
            rcTitle.right = rcTitle.left + (m_nTabWidth+m_nFramePos);
        else
            rcTitle.bottom = rcTitle.top + (m_nTabHeight+m_nFramePos);
        
        NotifyInvalidateRect(rcTitle);
    }
    return TRUE;
}

void CDuiTabCtrl::RemoveAllItems( void )
{
    for (int i = GetItemCount()-1; i >= 0; i--)
    {
        CDuiTab * pTab = GetItem(i);
        DestroyChild(pTab);
        m_lstPages.RemoveAt(i);
    }
    NotifyInvalidate();
}

void CDuiTabCtrl::OnMouseMove( UINT nFlags, CPoint point )
{
    CRect rcItem;
    int nOldHover=m_nHoverTabItem;
    m_nHoverTabItem=-1;
    int nTabCount=GetItemCount();
    for (int i = 0; i < nTabCount; i ++)
    {
        GetItemRect(i, rcItem);

        if (rcItem.PtInRect(point))
        {
            m_nHoverTabItem=i;
            break;
        }
    }

    if (m_nHoverTabItem != nOldHover)
    {
        if(nOldHover!=-1)
        {
            GetItemRect(nOldHover, rcItem);
            if(nOldHover!=m_nCurrentPage)
                NotifyInvalidateRect(rcItem);
            DUINMTABITEMLEAVE nms;
            nms.hdr.code=NM_TAB_ITEMLEAVE;
            nms.hdr.hDuiWnd=m_hDuiWnd;
            nms.hdr.idFrom=GetCmdID();
            nms.hdr.pszNameFrom=GetName();
            nms.iItem=nOldHover;
            nms.rcItem=rcItem;
            DuiNotify((LPDUINMHDR)&nms);
        }
        if(m_nHoverTabItem!=-1)
        {
            GetItemRect(m_nHoverTabItem, rcItem);
            if(m_nHoverTabItem!=m_nCurrentPage)
                NotifyInvalidateRect(rcItem);
            DUINMTABITEMHOVER nms;
            nms.hdr.code = NM_TAB_ITEMHOVER;
            nms.hdr.hDuiWnd=m_hDuiWnd;
            nms.hdr.idFrom = GetCmdID();
            nms.hdr.pszNameFrom=GetName();
            nms.iItem = m_nHoverTabItem;
            nms.rcItem = rcItem;
            DuiNotify((LPDUINMHDR)&nms);
        }
    }
}

void CDuiTabCtrl::OnDestroy()
{
    for(int i=GetItemCount()-1; i>=0; i--)
    {
        DestroyChild(m_lstPages[i]);
    }
    m_lstPages.RemoveAll();
}


BOOL CDuiTabCtrl::SetCurSel( int nIndex )
{
    if( nIndex < 0 || nIndex> GetItemCount()-1 || (m_nCurrentPage == nIndex)) return FALSE;
    int nOldPage = m_nCurrentPage;

    DUINMTABSELCHANGE nms;
    nms.hdr.code = NM_TAB_SELCHANGING;
    nms.hdr.hDuiWnd=m_hDuiWnd;
    nms.hdr.idFrom = GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.uTabItemIDNew = nIndex;
    nms.uTabItemIDOld = nOldPage;
    nms.bCancel = FALSE;

    LRESULT lRet = DuiNotify((LPDUINMHDR)&nms);

    if (nms.bCancel)
        return FALSE;

    CDuiTab *pTab=GetItem(nIndex);

    CRect rcItem;

    GetItemRect(m_nCurrentPage, rcItem);
    NotifyInvalidateRect(rcItem);
    GetItemRect(nIndex, rcItem);
    NotifyInvalidateRect(rcItem);

    CDuiTabSlider *pTabSlider=NULL;

    if(m_nAnimateSteps && IsVisible(TRUE) && nOldPage!=-1)
    {
        pTabSlider=new CDuiTabSlider(this);
        pTabSlider->InitPage(TRUE);
    }

    if(nOldPage!=-1)
    {
        pTab = GetItem(nOldPage);
        if( pTab) pTab->DuiSendMessage(WM_SHOWWINDOW,FALSE);
    }

    m_nCurrentPage = nIndex;
    if(nIndex!=-1)
    {
        pTab = GetItem(m_nCurrentPage);
        if( pTab) pTab->DuiSendMessage(WM_SHOWWINDOW,TRUE);
    }

    DUINMTABSELCHANGED nms2;
    nms2.hdr.code = NM_TAB_SELCHANGED;
    nms2.hdr.hDuiWnd=m_hDuiWnd;
    nms2.hdr.idFrom = GetCmdID();
    nms2.hdr.pszNameFrom=GetName();
    nms2.uTabItemIDNew = nIndex;
    nms2.uTabItemIDOld = nOldPage;

    if(pTabSlider)
    {
        SLIDEDIR sd=SD_RIGHTLEFT;
        if(m_nTabAlign==AlignTop)
        {
            if(nOldPage<nIndex) sd=SD_RIGHTLEFT;
            else sd=SD_LEFTRIGHT;
        }
        else
        {
            if(nOldPage<nIndex) sd=SD_BOTTOMTOP;
            else sd=SD_TOPBOTTOM;
        }
        pTabSlider->InitPage(FALSE);

        pTabSlider->Slide(sd,m_nAnimateSteps);

        delete pTabSlider;
    }
    DuiNotify((LPDUINMHDR)&nms2);
    if(IsVisible(TRUE))
    {
        NotifyInvalidate();
    }

    return TRUE;
}

BOOL CDuiTabCtrl::SetCurSel( LPCTSTR pszTitle )
{
    for(UINT i=0;i<m_lstPages.GetCount();i++)
    {
        if(_tcscmp(m_lstPages[i]->GetTitle(),pszTitle)==0)
            return SetCurSel(i);
    }
    return FALSE;
}

BOOL CDuiTabCtrl::SetItemTitle( int nIndex, LPCTSTR lpszTitle )
{
    CDuiTab* pTab = GetItem(nIndex);
    if (pTab)
    {
        pTab->SetTitle(lpszTitle);

        CRect rcTabs;
        GetClient(&rcTabs);
        if(m_nTabAlign==AlignLeft)
            rcTabs.right=rcTabs.left+m_nTabWidth;
        else
            rcTabs.bottom=rcTabs.top+m_nTabHeight;
        NotifyInvalidateRect(rcTabs);
        return TRUE;
    }

    return FALSE;
}

BOOL CDuiTabCtrl::LoadChildren( pugi::xml_node xmlNode )
{
    for ( pugi::xml_node xmlChild = xmlNode; xmlChild; xmlChild = xmlChild.next_sibling())
    {
        InsertItem(xmlChild,-1,TRUE);
    }

    if(m_nCurrentPage==-1 || m_nCurrentPage>=(int)m_lstPages.GetCount())
    {
        m_nCurrentPage=0;
    }
    if(m_nCurrentPage!=-1)
    {
        GetItem(m_nCurrentPage)->DuiSendMessage(WM_SHOWWINDOW,TRUE);
    }
    return TRUE;
}

BOOL CDuiTabCtrl::InsertItem( LPCWSTR lpContent ,int iInsert/*=-1*/)
{
    CDuiStringA utf8_xml=DUI_CW2A(lpContent,CP_UTF8);
    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_buffer(utf8_xml,utf8_xml.GetLength(),pugi::parse_default,pugi::encoding_utf8)) return FALSE;

    pugi::xml_node xmlTab=xmlDoc.child("tab");

    return InsertItem(xmlTab,iInsert)!=-1;
}

int CDuiTabCtrl::InsertItem( pugi::xml_node xmlNode,int iInsert/*=-1*/,BOOL bLoading/*=FALSE*/ )
{
    CDuiTab *pChild=NULL;
    if (!CDuiTab::CheckAndNew(xmlNode.name(),(void**)&pChild)) return -1;

    if(iInsert==-1) iInsert=m_lstPages.GetCount();
    InsertChild(pChild);

    m_lstPages.InsertAt(iInsert,pChild);

    pChild->Load(xmlNode);
    pChild->SetPositionType(SizeX_FitParent|SizeY_FitParent);

    if(!bLoading && m_nCurrentPage>=iInsert) m_nCurrentPage++;

    if(!bLoading)
    {
        CRect rcContainer=GetChildrenLayoutRect();
        pChild->DuiSendMessage(WM_WINDOWPOSCHANGED,0,(LPARAM)&rcContainer);
        NotifyInvalidate();
    }

    return iInsert;
}

BOOL CDuiTabCtrl::GetItemRect( int nIndex, CRect &rcItem )
{
    if (nIndex < 0 || nIndex >= (int)GetItemCount())
        return FALSE;

    SIZE size = {0, 0};

    if (m_pSkinTab)
        size = m_pSkinTab->GetSkinSize();

    if (0 != m_nTabHeight)
        size.cy = m_nTabHeight;
    if(0 != m_nTabWidth)
        size.cx=m_nTabWidth;

    rcItem.SetRect(m_rcWindow.left, m_rcWindow.top, m_rcWindow.left + size.cx, m_rcWindow.top + size.cy);

    switch (m_nTabAlign)
    {
    case AlignTop:
        rcItem.OffsetRect(m_nTabPos + nIndex * (rcItem.Width()+ m_nTabSpacing),0);
        break;
    case AlignLeft:
        rcItem.OffsetRect(0, m_nTabPos + nIndex * (rcItem.Height()+ m_nTabSpacing));
        break;
    }
    CRect rcClient;
    GetClient(&rcClient);
    rcItem.IntersectRect(rcItem,rcClient);
    return TRUE;
}

CDuiTab* CDuiTabCtrl::GetItem( int nIndex )
{
    if(nIndex<0 || nIndex>=GetItemCount()) return NULL;
    return m_lstPages[nIndex];
}

void CDuiTabCtrl::DrawItem( CDCHandle dc,const CRect &rcItem,int iItem,DWORD dwState )
{
    if(m_pSkinTab)
        m_pSkinTab->Draw(dc,rcItem,IIF_STATE3(dwState,DuiWndState_Normal,DuiWndState_Hover,DuiWndState_PushDown),m_byAlpha);

    CRect rcIcon(m_ptIcon+rcItem.TopLeft(),CSize(0,0));
    if(m_pSkinIcon)
    {
        rcIcon.right=rcIcon.left+m_pSkinIcon->GetSkinSize().cx;
        rcIcon.bottom=rcIcon.top+m_pSkinIcon->GetSkinSize().cy;
        m_pSkinIcon->Draw(dc,rcIcon,iItem);
    }

    if(m_ptText.x!=-1 && m_ptText.y!=-1)
    {
        CGdiAlpha::TextOut(dc,rcItem.left+m_ptText.x,rcItem.top+m_ptText.y,GetItem(iItem)->GetTitle());
    }
    else
    {
        CRect rcText=rcItem;
        UINT alignStyle=m_style.GetTextAlign();
        UINT align=alignStyle;
        if(m_ptText.x==-1 && m_ptText.y!=-1)
        {
            rcText.top+=m_ptText.y;
            align=alignStyle&(DT_CENTER|DT_RIGHT|DT_END_ELLIPSIS);
        }
        else if(m_ptText.x!=-1 && m_ptText.y==-1)
        {
            rcText.left+=m_ptText.x;
            align=alignStyle&(DT_VCENTER|DT_BOTTOM|DT_SINGLELINE|DT_END_ELLIPSIS);
        }
        
        CGdiAlpha::DrawText(dc,GetItem(iItem)->GetTitle(),-1,&rcText,align);
    }
}

void CDuiTabCtrl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if(nChar==VK_LEFT || nChar==VK_UP)
    {
        if(!SetCurSel(m_nCurrentPage-1))
            SetCurSel(GetItemCount()-1);
    }else if(nChar==VK_RIGHT || nChar==VK_DOWN)
    {
        if(!SetCurSel(m_nCurrentPage+1))
            SetCurSel(0);
    }else if(nChar==VK_HOME)
    {
        SetCurSel(0);
    }else if(nChar==VK_END)
    {
        SetCurSel(GetItemCount()-1);
    }
}

}//namespace SOUI