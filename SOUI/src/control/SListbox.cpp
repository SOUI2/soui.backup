//////////////////////////////////////////////////////////////////////////
//  Class Name: SListBox
// Description: A DuiWindow Based ListBox Control.
//     Creator: JinHui
//     Version: 2012.12.18 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once
#include "souistd.h"
#include "control/Slistbox.h"
#include "SApp.h"
#include "helper/mybuffer.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

namespace SOUI
{


SListBox::SListBox()
    : m_nItemHei(20)
    , m_iSelItem(-1)
    , m_iHoverItem(-1)
    , m_crItemBg(CR_INVALID)
    , m_crItemBg2(CR_INVALID)
    , m_crItemSelBg(RGBA(57,145,209,255))
	, m_crItemHotBg(RGBA(57,145,209,128))
    , m_crText(CR_INVALID)
    , m_crSelText(CR_INVALID)
    , m_pItemSkin(NULL)
    , m_pIconSkin(NULL)
    , m_ptIcon(-1,-1)
    , m_ptText(-1,-1)
    , m_bHotTrack(FALSE)
{
    m_bFocusable = TRUE;
    m_evtSet.addEvent(EVENTID(EventLBSelChanging));
    m_evtSet.addEvent(EVENTID(EventLBSelChanged));
    m_evtSet.addEvent(EVENTID(EventLBGetDispInfo));
}

SListBox::~SListBox()
{
}

int SListBox::GetCount() const
{
    return m_arrItems.GetCount();
}

int SListBox::GetCurSel() const
{
    return m_iSelItem;
}

BOOL SListBox::SetCurSel(int nIndex)
{
    if(nIndex >= GetCount())
        return FALSE;
        
    if(nIndex < 0 ) nIndex =-1;

    if(m_iSelItem == nIndex) return 0;
    int nOldSelItem = m_iSelItem;
    m_iSelItem = nIndex;

    if(IsVisible(TRUE))
    {
        if(nOldSelItem != -1)
            RedrawItem(nOldSelItem);
        if(m_iSelItem!=-1)
            RedrawItem(m_iSelItem);
    }
    return TRUE;
}

int SListBox::GetTopIndex() const
{
    return m_ptOrigin.y / m_nItemHei;
}

BOOL SListBox::SetTopIndex(int nIndex)
{
    if (nIndex < 0 || nIndex >= GetCount())
        return FALSE;

    OnScroll(TRUE,SB_THUMBPOSITION, nIndex*m_nItemHei);
    return TRUE;
}

LPARAM SListBox::GetItemData(int nIndex) const
{
    if (nIndex < 0 || nIndex >= GetCount())
        return 0;

    return m_arrItems[nIndex]->lParam;
}

BOOL SListBox::SetItemData(int nIndex, LPARAM lParam)
{
    if (nIndex < 0 || nIndex >= GetCount())
        return FALSE;

    m_arrItems[nIndex]->lParam = lParam;
    return TRUE;
}

int SListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
{
    int nRet = GetTextLen(nIndex);

    if(nRet != LB_ERR)
        _tcscpy(lpszBuffer, m_arrItems[nIndex]->strText);

    return nRet;
}

int SListBox::GetText(int nIndex, SStringT& strText) const
{
    int nRet = GetTextLen(nIndex);

    if(nRet != LB_ERR)
        strText = m_arrItems[nIndex]->strText;

    return nRet;
}

int SListBox::GetTextLen(int nIndex) const
{
    if (nIndex < 0 || nIndex >= GetCount())
        return LB_ERR;

    return m_arrItems[nIndex]->strText.GetLength();
}

int SListBox::GetItemHeight(int nIndex) const
{
    return m_nItemHei;
}

BOOL SListBox::SetItemHeight(int nIndex, int cyItemHeight)
{
    if (cyItemHeight < 0 || nIndex < 0 || nIndex >= GetCount())
        return FALSE;

    m_nItemHei = cyItemHeight;
    return TRUE;
}

void SListBox::DeleteAll()
{
    for(int i=0; i < GetCount(); i++)
    {
        if (m_arrItems[i])
            delete m_arrItems[i];
    }
    m_arrItems.RemoveAll();

    m_iSelItem=-1;
    m_iHoverItem=-1;

    SetViewSize(CSize(0,0));
    Invalidate();
}

BOOL SListBox::DeleteString(int nIndex)
{
    if(nIndex<0 || nIndex>=GetCount()) return FALSE;


    if (m_arrItems[nIndex])
        delete m_arrItems[nIndex];
    m_arrItems.RemoveAt(nIndex);

    if(m_iSelItem==nIndex) m_iSelItem=-1;
    else if(m_iSelItem>nIndex) m_iSelItem--;
    if(m_iHoverItem==nIndex) m_iHoverItem=-1;
    else if(m_iHoverItem>nIndex) m_iHoverItem--;

    CRect rcClient;
    SWindow::GetClientRect(&rcClient);
    CSize szView(rcClient.Width(),GetCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);

    return TRUE;
}

int SListBox::AddString(LPCTSTR lpszItem, int nImage, LPARAM lParam)
{
    return InsertString(-1, lpszItem, nImage,  lParam);
}

int SListBox::InsertString(int nIndex, LPCTSTR lpszItem, int nImage,  LPARAM lParam)
{
    //SASSERT(lpszItem);

    LPLBITEM pItem = new LBITEM;
    pItem->strText = lpszItem;
    pItem->nImage = nImage;
    pItem->lParam = lParam;

    return InsertItem(nIndex, pItem);
}

void SListBox::EnsureVisible(int nIndex)
{
    if(nIndex < 0 || nIndex >= GetCount()) return;

    CRect rcClient;
    GetClientRect(&rcClient);

    int iFirstVisible = (m_ptOrigin.y + m_nItemHei -1) / m_nItemHei;
    int nVisibleItems = rcClient.Height() / m_nItemHei;
    if(nIndex < iFirstVisible || nIndex > iFirstVisible+nVisibleItems-1)
    {
        int nOffset = GetScrollPos(TRUE);
        if(nIndex < iFirstVisible) nOffset = (nIndex-iFirstVisible)*m_nItemHei;
        else nOffset=(nIndex - iFirstVisible-nVisibleItems +1)*m_nItemHei;
        nOffset-=nOffset%m_nItemHei;//让当前行刚好显示
        OnScroll(TRUE,SB_THUMBPOSITION,nOffset + GetScrollPos(TRUE));
    }
}

//自动修改pt的位置为相对当前项的偏移量
int SListBox::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    if(!rcClient.PtInRect(pt)) return -1;
    
    CPoint pt2=pt;
    pt2.y -= rcClient.top - m_ptOrigin.y;
    int nRet=pt2.y/m_nItemHei;
    if(nRet >= GetCount()) nRet=-1;
    else
    {
        pt.x-=rcClient.left;
        pt.y=pt2.y%m_nItemHei;
    }

    return nRet;
}

BOOL SListBox::CreateChildren(pugi::xml_node xmlNode)
{
    if(!xmlNode) return TRUE;

    pugi::xml_node xmlItems=xmlNode.child(L"items");
    if(xmlItems)
    {
        pugi::xml_node xmlItem= xmlItems.child(L"item");
        while(xmlItem)
        {
            LPLBITEM pItemObj = new LBITEM;
            LoadItemAttribute(xmlItem, pItemObj);
            InsertItem(-1, pItemObj);
            xmlItem = xmlItem.next_sibling();
        }    
    }

    int nSelItem=xmlNode.attribute(L"curSel").as_int(-1);
    SetCurSel(nSelItem);

    return TRUE;
}

void SListBox::LoadItemAttribute(pugi::xml_node xmlNode, LPLBITEM pItem)
{
    pItem->nImage=xmlNode.attribute(L"icon").as_int(pItem->nImage);
    pItem->lParam=xmlNode.attribute(L"data").as_uint(pItem->lParam);
    pItem->strText =  S_CW2T(tr(xmlNode.attribute(L"text").value()));
    BUILDSTRING(pItem->strText);
}

int SListBox::InsertItem(int nIndex, LPLBITEM pItem)
{
    SASSERT(pItem);

    if(nIndex==-1 || nIndex>GetCount())
    {
        nIndex = GetCount();
    }

    m_arrItems.InsertAt(nIndex, pItem);

    if(m_iSelItem >= nIndex) m_iSelItem++;
    if(m_iHoverItem >= nIndex) m_iHoverItem++;

    CRect rcClient;
    SWindow::GetClientRect(&rcClient);
    CSize szView(rcClient.Width(),GetCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);

    return nIndex;
}

void SListBox::RedrawItem(int iItem)
{
    if(!IsVisible(TRUE)) return;

    CRect rcClient;
    GetClientRect(&rcClient);
    int iFirstVisible = GetTopIndex();
    int nPageItems=(rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    if(iItem>=iFirstVisible && iItem<GetCount() && iItem<iFirstVisible+nPageItems)
    {
        CRect rcItem(0,0,rcClient.Width(),m_nItemHei);
        rcItem.OffsetRect(0,m_nItemHei*iItem-m_ptOrigin.y);
        rcItem.OffsetRect(rcClient.TopLeft());
        IRenderTarget *pRT=GetRenderTarget(&rcItem,OLEDC_PAINTBKGND);

        SSendMessage(WM_ERASEBKGND,(WPARAM)(HDC)pRT);
        DrawItem(pRT,rcItem,iItem);

        ReleaseRenderTarget(pRT);
    }
}

void SListBox::DrawItem(IRenderTarget * pRT, CRect & rc, int iItem)
{
    if (iItem < 0 || iItem >= GetCount()) return;

    BOOL bTextColorChanged = FALSE;
    int nBgImg = 0;
    COLORREF crOldText;
    COLORREF crItemBg = m_crItemBg;
    COLORREF crText = m_crText;
    LPLBITEM pItem = m_arrItems[iItem];
    CRect rcIcon, rcText;

    if (iItem % 2)
    {
//         if (m_pItemSkin != NULL)
//             nBgImg = 1;
//         else if (CR_INVALID != m_crItemBg2)
//             crItemBg = m_crItemBg2;
		//上面的代码不要了，因为skin间隔效果没必要，只留下颜色间隔就好了
		if (CR_INVALID != m_crItemBg2)
				crItemBg = m_crItemBg2;
    }

    if ( iItem == m_iSelItem) 
    {//和下面那个if的条件分开，才会有sel和hot的区别
        if (m_pItemSkin != NULL)
            nBgImg = 2;
        else if (CR_INVALID != m_crItemSelBg)
            crItemBg = m_crItemSelBg;

        if (CR_INVALID != m_crSelText)
            crText = m_crSelText;
    }
	else if  ((iItem == m_iHoverItem || (m_iHoverItem==-1 && iItem== m_iSelItem)) && m_bHotTrack)
	{
		if (m_pItemSkin != NULL)
			nBgImg = 1;
		else if (CR_INVALID != m_crItemHotBg)
			crItemBg = m_crItemHotBg;

		if (CR_INVALID != m_crSelText)
			crText = m_crSelText;
	}

    //绘制背景
//     if (m_pItemSkin != NULL)
//         m_pItemSkin->Draw(pRT, rc, nBgImg);
//     else if (CR_INVALID != crItemBg)
//         pRT->FillSolidRect( rc, crItemBg);
//上面的代码在某些时候，【指定skin的时候，会导致背景异常】
	if (CR_INVALID != crItemBg)//先画背景
		pRT->FillSolidRect( rc, crItemBg);

	if (m_pItemSkin != NULL)//有skin，则覆盖背景
		m_pItemSkin->Draw(pRT, rc, nBgImg);


    if (CR_INVALID != crText)
    {
        bTextColorChanged = TRUE;
        crOldText = pRT->SetTextColor(crText);
    }

    if (pItem->nImage != -1 && m_pIconSkin)
    {
        int nOffsetX =m_ptIcon.x, nOffsetY = m_ptIcon.y;
        CSize sizeSkin = m_pIconSkin->GetSkinSize();
        rcIcon.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);

        if (m_ptIcon.x == -1)
            nOffsetX =  m_nItemHei / 6;

        if (m_ptIcon.y == -1)
            nOffsetY = (m_nItemHei - sizeSkin.cy) / 2;    //y 默认居中

        rcIcon.OffsetRect(rc.left + nOffsetX, rc.top + nOffsetY);
        m_pIconSkin->Draw(pRT, rcIcon, pItem->nImage);
    }

    UINT align = DT_SINGLELINE;
    rcText = rc;

    if (m_ptText.x == -1)
        rcText.left = rcIcon.Width() > 0 ? rcIcon.right + m_nItemHei / 6 : rc.left;
    else
        rcText.left = rc.left + m_ptText.x;

    if (m_ptText.y == -1)
        align |= DT_VCENTER;
    else
        rcText.top = rc.top + m_ptText.y;

    pRT->DrawText(pItem->strText,-1,rcText,align);

    if (bTextColorChanged)
        pRT->SetTextColor(crOldText);
}


void SListBox::NotifySelChange( int nOldSel,int nNewSel)
{
    EventLBSelChanging evt1(this);
    evt1.nOldSel=nOldSel;
    evt1.nNewSel=nNewSel;

    FireEvent(evt1);
    if(evt1.bCancel) return;
       
    m_iSelItem=nNewSel;
    if(nOldSel!=-1)
        RedrawItem(nOldSel);

    if(m_iSelItem!=-1)
        RedrawItem(m_iSelItem);
    
    EventLBSelChanged evt2(this);
    evt2.nOldSel=nOldSel;
    evt2.nNewSel=nNewSel;
    FireEvent(evt2);
}

void SListBox::OnPaint(IRenderTarget * pRT)
{
    SPainter painter;
    BeforePaint(pRT,painter);

    int iFirstVisible = GetTopIndex();
    int nPageItems = (m_rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    for(int iItem = iFirstVisible; iItem<GetCount() && iItem <iFirstVisible+nPageItems; iItem++)
    {
        CRect rcItem(0,0,m_rcClient.Width(),m_nItemHei);
        rcItem.OffsetRect(0,m_nItemHei*iItem-m_ptOrigin.y);
        rcItem.OffsetRect(m_rcClient.TopLeft());
        DrawItem(pRT,rcItem,iItem);
    }

    AfterPaint(pRT,painter);
}

void SListBox::OnSize(UINT nType,CSize size)
{
    __super::OnSize(nType,size);
    CRect rcClient;
    SWindow::GetClientRect(&rcClient);
    CSize szView(rcClient.Width(),GetCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);
}

void SListBox::OnLButtonDown(UINT nFlags,CPoint pt)
{
   SWindow::OnLButtonDown(nFlags,pt);
    if(!m_bHotTrack)
    {
        m_iHoverItem = HitTest(pt);
        if(m_iHoverItem!=m_iSelItem)
            NotifySelChange(m_iSelItem,m_iHoverItem);
    }
}

void SListBox::OnLButtonUp(UINT nFlags,CPoint pt)
{
    if(m_bHotTrack)
    {
        m_iHoverItem = HitTest(pt);
        if(m_iHoverItem!=m_iSelItem)
            NotifySelChange(m_iSelItem,m_iHoverItem);
    }
    SWindow::OnLButtonUp(nFlags,pt);
}

void SListBox::OnLButtonDbClick(UINT nFlags,CPoint pt)
{
}

void SListBox::OnMouseMove(UINT nFlags,CPoint pt)
{
    int nOldHover=m_iHoverItem;
    m_iHoverItem = HitTest(pt);
    
    if(m_bHotTrack && nOldHover!=m_iHoverItem)
    {
        if(nOldHover!=-1) RedrawItem(nOldHover);
        if(m_iHoverItem!=-1) RedrawItem(m_iHoverItem);
        if(m_iSelItem!=-1) RedrawItem(m_iSelItem);
    }
}

void SListBox::OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
{
    int  nNewSelItem = -1;
    int iCurSel=m_iSelItem;
    if(m_bHotTrack && m_iHoverItem!=-1)
        iCurSel=m_iHoverItem;
    if (nChar == VK_DOWN && m_iSelItem < GetCount() - 1)
        nNewSelItem = iCurSel+1;
    else if (nChar == VK_UP && m_iSelItem > 0)
        nNewSelItem = iCurSel-1;

    if(nNewSelItem!=-1)
    {
        int iHover=m_iHoverItem;
        if(m_bHotTrack)
            m_iHoverItem=-1;
        EnsureVisible(nNewSelItem);
        NotifySelChange(m_iSelItem,nNewSelItem);
        if(iHover!=-1 && iHover != m_iSelItem && iHover != nNewSelItem) 
            RedrawItem(iHover);
    }
}

void SListBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SWindow *pOwner = GetOwner();
    if (pOwner)
        pOwner->SSendMessage(WM_CHAR, nChar, MAKELONG(nFlags, nRepCnt));
}

UINT SListBox::OnGetDlgCode()
{
    return SC_WANTARROWS|SC_WANTCHARS;
}

void SListBox::OnDestroy()
{
    DeleteAll();
    __super::OnDestroy();
}

void SListBox::OnShowWindow( BOOL bShow, UINT nStatus )
{
    if(!bShow)
    {
        m_iHoverItem=-1;
    }
    __super::OnShowWindow(bShow,nStatus);
}

void SListBox::OnMouseLeave()
{
	__super::OnMouseLeave();
	if(m_iHoverItem!=-1)
	{
		int nOldHover=m_iHoverItem;
		m_iHoverItem=-1;
		RedrawItem(nOldHover);
	}
}

}//namespace SOUI