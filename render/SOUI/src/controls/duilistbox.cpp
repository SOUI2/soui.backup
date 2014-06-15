//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiListBox
// Description: A DuiWindow Based ListBox Control.
//     Creator: JinHui
//     Version: 2012.12.18 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma  once
#include "duistd.h"
#include "control/duilistbox.h"
#include "DuiSystem.h"
#include "mybuffer.h"

#pragma warning(disable:4018)
#pragma warning(disable:4267)

namespace SOUI
{


CDuiListBox::CDuiListBox()
    : m_nItemHei(20)
    , m_iScrollSpeed(-1)
    , m_iSelItem(-1)
    , m_iHoverItem(-1)
    , m_crItemBg(CLR_INVALID)
    , m_crItemBg2(CLR_INVALID)
    , m_crItemSelBg(CLR_INVALID)
    , m_crText(CLR_INVALID)
    , m_crSelText(CLR_INVALID)
    , m_pItemSkin(NULL)
    , m_pIconSkin(NULL)
    , m_ptIcon(-1,-1)
    , m_ptText(-1,-1)
    , m_bHotTrack(FALSE)
{
}

CDuiListBox::~CDuiListBox()
{
}

int CDuiListBox::GetCount() const
{
    return m_arrItems.GetCount();
}

int CDuiListBox::GetCurSel() const
{
    return m_iSelItem;
}

BOOL CDuiListBox::SetCurSel(int nIndex)
{
    if(m_iSelItem == nIndex) return 0;

    if(nIndex < 0 && nIndex >= GetCount())
        return FALSE;

    int nOldSelItem = m_iSelItem;
    m_iSelItem = nIndex;

    if(IsVisible(TRUE))
    {
        if(nOldSelItem != -1)
            RedrawItem(nOldSelItem);

        RedrawItem(nIndex);
    }
    return TRUE;
}

int CDuiListBox::GetTopIndex() const
{
    return m_ptOrigin.y / m_nItemHei;
}

BOOL CDuiListBox::SetTopIndex(int nIndex)
{
    if (nIndex < 0 || nIndex >= GetCount())
        return FALSE;

    OnScroll(TRUE,SB_THUMBPOSITION, nIndex*m_nItemHei);
    return TRUE;
}

LPARAM CDuiListBox::GetItemData(int nIndex) const
{
    if (nIndex < 0 || nIndex >= GetCount())
        return 0;

    return m_arrItems[nIndex]->lParam;
}

BOOL CDuiListBox::SetItemData(int nIndex, LPARAM lParam)
{
    if (nIndex < 0 || nIndex >= GetCount())
        return FALSE;

    m_arrItems[nIndex]->lParam = lParam;
    return TRUE;
}

int CDuiListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
{
    int nRet = GetTextLen(nIndex);

    if(nRet != LB_ERR)
        _tcscpy(lpszBuffer, m_arrItems[nIndex]->strText);

    return nRet;
}

int CDuiListBox::GetText(int nIndex, CDuiStringT& strText) const
{
    int nRet = GetTextLen(nIndex);

    if(nRet != LB_ERR)
        strText = m_arrItems[nIndex]->strText;

    return nRet;
}

int CDuiListBox::GetTextLen(int nIndex) const
{
    if (nIndex < 0 || nIndex >= GetCount())
        return LB_ERR;

    return m_arrItems[nIndex]->strText.GetLength();
}

int CDuiListBox::GetItemHeight(int nIndex) const
{
    return m_nItemHei;
}

BOOL CDuiListBox::SetItemHeight(int nIndex, int cyItemHeight)
{
    if (cyItemHeight < 0 || nIndex < 0 || nIndex >= GetCount())
        return FALSE;

    m_nItemHei = cyItemHeight;
    return TRUE;
}

void CDuiListBox::DeleteAll()
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
    NotifyInvalidate();
}

BOOL CDuiListBox::DeleteString(int nIndex)
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
    CDuiWindow::GetClient(&rcClient);
    CSize szView(rcClient.Width(),GetCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);

    return TRUE;
}

int CDuiListBox::AddString(LPCTSTR lpszItem, int nImage, LPARAM lParam)
{
    return InsertString(-1, lpszItem, nImage,  lParam);
}

int CDuiListBox::InsertString(int nIndex, LPCTSTR lpszItem, int nImage,  LPARAM lParam)
{
    DUIASSERT(lpszItem);

    LPLBITEM pItem = new LBITEM;
    pItem->strText = lpszItem;
    pItem->nImage = nImage;
    pItem->lParam = lParam;

    return InsertItem(nIndex, pItem);
}

void CDuiListBox::EnsureVisible(int nIndex)
{
    if(nIndex < 0 || nIndex >= GetCount()) return;

    CRect rcClient;
    GetClient(&rcClient);

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
int CDuiListBox::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClient(&rcClient);
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

BOOL CDuiListBox::Load(pugi::xml_node xmlNode)
{
    __super::Load(xmlNode);

    CDuiStringT strChildSrc = DUI_CA2T(xmlNode.attribute("itemsrc").value(),CP_UTF8);

    if (strChildSrc.IsEmpty())
        return TRUE;

    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,strChildSrc,DUIRES_XML_TYPE)) return FALSE;

    return LoadChildren(xmlDoc.first_child());
}

BOOL CDuiListBox::LoadChildren(pugi::xml_node xmlNode)
{
    if(!xmlNode) return TRUE;

    pugi::xml_node xmlParent=xmlNode.parent();
    pugi::xml_node xmlItem=xmlParent.child("items");
    while(xmlItem)
    {
        LPLBITEM pItemObj = new LBITEM;
        LoadItemAttribute(xmlItem, pItemObj);
        InsertItem(-1, pItemObj);
        xmlItem = xmlItem.next_sibling();
    }

    int nSelItem=xmlParent.attribute("cursel").as_int(-1);
    SetCurSel(nSelItem);

    return TRUE;
}

void CDuiListBox::LoadItemAttribute(pugi::xml_node xmlNode, LPLBITEM pItem)
{
    pItem->nImage=xmlNode.attribute("img").as_int(pItem->nImage);
    pItem->lParam=xmlNode.attribute("data").as_uint(pItem->lParam);

    pItem->strText =  DUI_CA2T(xmlNode.text().get(), CP_UTF8);
    BUILDSTRING(pItem->strText);
}

int CDuiListBox::InsertItem(int nIndex, LPLBITEM pItem)
{
    DUIASSERT(pItem);

    if(nIndex==-1 || nIndex>=GetCount())
    {
        nIndex = GetCount();
    }

    m_arrItems.InsertAt(nIndex, pItem);

    if(m_iSelItem >= nIndex) m_iSelItem++;
    if(m_iHoverItem >= nIndex) m_iHoverItem++;

    CRect rcClient;
    CDuiWindow::GetClient(&rcClient);
    CSize szView(rcClient.Width(),GetCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);

    return nIndex;
}

int CDuiListBox::GetScrollLineSize(BOOL bVertical)
{
    return m_iScrollSpeed >0 ? m_iScrollSpeed : m_nItemHei;
}

void CDuiListBox::RedrawItem(int iItem)
{
    if(!IsVisible(TRUE)) return;

    CRect rcClient;
    GetClient(&rcClient);
    int iFirstVisible = GetTopIndex();
    int nPageItems=(rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    if(iItem>=iFirstVisible && iItem<GetCount() && iItem<iFirstVisible+nPageItems)
    {
        CRect rcItem(0,0,rcClient.Width(),m_nItemHei);
        rcItem.OffsetRect(0,m_nItemHei*iItem-m_ptOrigin.y);
        rcItem.OffsetRect(rcClient.TopLeft());
        CDCHandle dc=GetDuiDC(&rcItem,OLEDC_PAINTBKGND);
        DuiDCPaint duiDC;
        BeforePaint(dc,duiDC);

        DuiSendMessage(WM_ERASEBKGND,(WPARAM)(HDC)dc);
        DrawItem(dc,rcItem,iItem);

        AfterPaint(dc,duiDC);
        ReleaseDuiDC(dc);
    }
}

void CDuiListBox::DrawItem(CDCHandle & dc, CRect & rc, int iItem)
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
        if (m_pItemSkin != NULL)
            nBgImg = 1;
        else if (CLR_INVALID != m_crItemBg2)
            crItemBg = m_crItemBg2;
    }

    if ( (!m_bHotTrack && iItem == m_iSelItem) || ((iItem == m_iHoverItem || (m_iHoverItem==-1 && iItem== m_iSelItem)) && m_bHotTrack))
    {
        if (m_pItemSkin != NULL)
            nBgImg = 2;
        else if (CLR_INVALID != m_crItemSelBg)
            crItemBg = m_crItemSelBg;

        if (CLR_INVALID != m_crSelText)
            crText = m_crSelText;
    }

    //绘制背景
    if (m_pItemSkin != NULL)
        m_pItemSkin->Draw(dc, rc, nBgImg);
    else if (CLR_INVALID != crItemBg)
        CGdiAlpha::FillSolidRect(dc, rc, crItemBg);

    if (CLR_INVALID != crText)
    {
        bTextColorChanged = TRUE;
        crOldText = dc.SetTextColor(crText);
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
        m_pIconSkin->Draw(dc, rcIcon, pItem->nImage);
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

    CGdiAlpha::DrawText(dc, pItem->strText,-1,rcText,align);

    if (bTextColorChanged)
        dc.SetTextColor(crOldText);
}


void CDuiListBox::NotifySelChange( int nOldSel,int nNewSel)
{
    DUINMLBSELCHANGE nms;
    nms.hdr.code=NM_LBSELCHANGING;
    nms.hdr.hDuiWnd=m_hDuiWnd;
    nms.hdr.idFrom=GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.nOldSel=nOldSel;
    nms.nNewSel=nNewSel;
    nms.uHoverID=0;

    if(S_OK!=DuiNotify((LPDUINMHDR)&nms)) return ;
    
    m_iSelItem=nNewSel;
    if(nOldSel!=-1)
        RedrawItem(nOldSel);

    if(m_iSelItem!=-1)
        RedrawItem(m_iSelItem);

    nms.hdr.idFrom=GetCmdID();
    nms.hdr.code=NM_LBSELCHANGED;
    DuiNotify((LPDUINMHDR)&nms);
}

void CDuiListBox::OnPaint(CDCHandle dc)
{
    DuiDCPaint duiDC;
    BeforePaint(dc,duiDC);

    int iFirstVisible = GetTopIndex();
    int nPageItems = (m_rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    for(int iItem = iFirstVisible; iItem<GetCount() && iItem <iFirstVisible+nPageItems; iItem++)
    {
        CRect rcItem(0,0,m_rcClient.Width(),m_nItemHei);
        rcItem.OffsetRect(0,m_nItemHei*iItem-m_ptOrigin.y);
        rcItem.OffsetRect(m_rcClient.TopLeft());
        DrawItem(dc,rcItem,iItem);
    }

    AfterPaint(dc,duiDC);
}

void CDuiListBox::OnSize(UINT nType,CSize size)
{
    CRect rcClient;
    CDuiWindow::GetClient(&rcClient);
    CSize szView(rcClient.Width(),GetCount()*m_nItemHei);
    if(szView.cy>rcClient.Height()) szView.cx-=m_nSbWid;
    SetViewSize(szView);
    __super::OnSize(nType,size);
}

void CDuiListBox::OnLButtonDown(UINT nFlags,CPoint pt)
{
}

void CDuiListBox::OnLButtonUp(UINT nFlags,CPoint pt)
{
    m_iHoverItem = HitTest(pt);

    if(m_iHoverItem!=m_iSelItem)
        NotifySelChange(m_iSelItem,m_iHoverItem);
}

void CDuiListBox::OnLButtonDbClick(UINT nFlags,CPoint pt)
{
}

void CDuiListBox::OnMouseMove(UINT nFlags,CPoint pt)
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

void CDuiListBox::OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
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

void CDuiListBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CDuiWindow *pOwner = GetOwner();
    if (pOwner)
        pOwner->DuiSendMessage(WM_CHAR, nChar, MAKELONG(nFlags, nRepCnt));
}

UINT CDuiListBox::OnGetDuiCode()
{
    return DUIC_WANTALLKEYS;
}

void CDuiListBox::OnDestroy()
{
    DeleteAll();
    __super::OnDestroy();
}

void CDuiListBox::OnShowWindow( BOOL bShow, UINT nStatus )
{
    if(!bShow)
    {
        m_iHoverItem=-1;
    }
    __super::OnShowWindow(bShow,nStatus);
}

}//namespace SOUI