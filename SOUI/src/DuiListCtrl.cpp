#include "duistd.h"

#include "DuiListCtrl.h"
#include "DuiWndNotify.h"

#pragma warning(disable : 4267 4018)

namespace SOUI
{
//////////////////////////////////////////////////////////////////////////
//  CDuiListCtrl
CDuiListCtrl::CDuiListCtrl()
    : m_nHeaderHeight(20)
    , m_nItemHeight(20)
    , m_pHeader(NULL)
    , m_nSelectItem(-1)
    , m_crItemBg(RGB(255,255,255))
    , m_crItemBg2(RGB(226,226,226))
    , m_crItemSelBg(RGB(140,160,240))
    , m_crText(RGB(0,0,0))
    , m_crSelText(RGB(255,255,0))
    , m_pItemSkin(NULL)
    , m_pIconSkin(NULL)
    , m_ptIcon(-1,-1)
    , m_ptText(-1,-1)
    , m_bHotTrack(FALSE)
    , m_nMargin(4)
{
    m_bClipClient = TRUE;
}

CDuiListCtrl::~CDuiListCtrl()
{
}

int CDuiListCtrl::InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, LPARAM lParam)
{
    DUIASSERT(m_pHeader);

    int nRet = m_pHeader->InsertItem(nIndex, pszText, nWidth, ST_NULL, lParam);
    for(int i=0;i<GetItemCount();i++)
    {
        m_arrItems[i].arSubItems->SetCount(GetColumnCount());
    }
    UpdateScrollBar();
    return nRet;
}

BOOL CDuiListCtrl::LoadChildren(pugi::xml_node xmlNode)
{
    //  listctrl的子控件只能是一个header控件
    if (strcmp(xmlNode.name(), CDuiHeaderCtrl::GetClassName()) != 0)
        return FALSE;

    if (!__super::LoadChildren(xmlNode))
        return FALSE;

    m_pHeader = (CDuiHeaderCtrl*)GetDuiWindow(GDUI_FIRSTCHILD);
    DUIASSERT(m_pHeader->IsClass(CDuiHeaderCtrl::GetClassName()));
    CDuiStringA strPos;
    strPos.Format("0,0,-0,%d",m_nHeaderHeight);
    m_pHeader->SetAttribute("pos",strPos,TRUE);

    m_pHeader->subscribeEvent(NM_HDSIZECHANGING, Subscriber(&CDuiListCtrl::OnHeaderSizeChanging,this));
    m_pHeader->subscribeEvent(NM_HDSWAP, Subscriber(&CDuiListCtrl::OnHeaderSwap,this));

    return TRUE;
}

int CDuiListCtrl::InsertItem(int nItem, LPCTSTR pszText, int nImage)
{
    if(GetColumnCount()==0) return -1;
    if (nItem<0 || nItem>GetItemCount())
        nItem = GetItemCount();

    DXLVITEM lvi;
    lvi.dwData = 0;
    lvi.arSubItems=new ArrSubItem();
    lvi.arSubItems->SetCount(GetColumnCount());
    
    DXLVSUBITEM &subItem=lvi.arSubItems->GetAt(0);
    subItem.strText = _tcsdup(pszText);
    subItem.cchTextMax = _tcslen(pszText);
    subItem.nImage  = nImage;

    m_arrItems.InsertAt(nItem, lvi);

    UpdateScrollBar();

    return nItem;
}

BOOL CDuiListCtrl::SetItemData(int nItem, DWORD dwData)
{
    if (nItem >= GetItemCount())
        return FALSE;

    m_arrItems[nItem].dwData = dwData;

    return TRUE;
}

DWORD CDuiListCtrl::GetItemData(int nItem)
{
    if (nItem >= GetItemCount())
        return 0;

    DXLVITEM& lvi = m_arrItems[nItem];

    return (DWORD)lvi.dwData;
}

BOOL CDuiListCtrl::SetSubItem(int nItem, int nSubItem, const DXLVSUBITEM* plv)
{
    if (nItem>=GetItemCount() || nSubItem>=GetColumnCount())
        return FALSE;
    DXLVSUBITEM & lvsi_dst=m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    if(plv->mask & DUI_LVIF_TEXT)
    {
        if(lvsi_dst.strText) free(lvsi_dst.strText);
        lvsi_dst.strText=_tcsdup(plv->strText);
        lvsi_dst.cchTextMax=_tcslen(plv->strText);
    }
    if(plv->mask&DUI_LVIF_IMAGE)
        lvsi_dst.nImage=plv->nImage;
    if(plv->mask&DUI_LVIF_INDENT)
        lvsi_dst.nIndent=plv->nIndent;
    RedrawItem(nItem);
    return TRUE;
}

BOOL CDuiListCtrl::GetSubItem(int nItem, int nSubItem, DXLVSUBITEM* plv)
{
    if (nItem>=GetItemCount() || nSubItem>=GetColumnCount())
        return FALSE;

    const DXLVSUBITEM & lvsi_src=m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    if(plv->mask & DUI_LVIF_TEXT)
    {
        _tcscpy_s(plv->strText,plv->cchTextMax,lvsi_src.strText);
    }
    if(plv->mask&DUI_LVIF_IMAGE)
        plv->nImage=lvsi_src.nImage;
    if(plv->mask&DUI_LVIF_INDENT)
        plv->nIndent=lvsi_src.nIndent;
    return TRUE;
}

BOOL CDuiListCtrl::SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText)
{
    if (nItem < 0 || nItem >= GetItemCount())
        return FALSE;

    if (nSubItem < 0 || nSubItem >= GetColumnCount())
        return FALSE;

    DXLVSUBITEM &lvi=m_arrItems[nItem].arSubItems->GetAt(nSubItem);
    if(lvi.strText)
    {
        free(lvi.strText);
    }
    lvi.strText = _tcsdup(pszText);
    lvi.cchTextMax= _tcslen(pszText);
    
    CRect rcItem=GetItemRect(nItem,nSubItem);
    NotifyInvalidateRect(rcItem);
    return TRUE;
}


int CDuiListCtrl::GetSelectedItem()
{
    return m_nSelectItem;
}

void CDuiListCtrl::SetSelectedItem(int nItem)
{
    m_nSelectItem = nItem;

    NotifyInvalidate();
}

int CDuiListCtrl::GetItemCount()
{
    if (GetColumnCount() <= 0)
        return 0;

    return m_arrItems.GetCount();
}

BOOL CDuiListCtrl::SetItemCount( int nItems ,int nGrowBy)
{
    int nOldCount=GetItemCount();
    if(nItems<nOldCount) return FALSE;
    
    BOOL bRet=m_arrItems.SetCount(nItems,nGrowBy);
    if(bRet)
    {
        for(int i=nOldCount;i<nItems;i++)
        {
            DXLVITEM & lvi=m_arrItems[i];
            lvi.arSubItems=new ArrSubItem;
            lvi.arSubItems->SetCount(GetColumnCount());
        }
    }
    UpdateScrollBar();

    return bRet;
}

CRect CDuiListCtrl::GetListRect()
{
    CRect rcList;

    GetClient(&rcList);
    rcList.top += m_nHeaderHeight;

    return rcList;
}

//////////////////////////////////////////////////////////////////////////
//  更新滚动条
void CDuiListCtrl::UpdateScrollBar()
{
    CSize szView;
    szView.cx = m_pHeader->GetTotalWidth();
    szView.cy = GetItemCount()*m_nItemHeight;

    CRect rcClient;
    CDuiWindow::GetClient(&rcClient);//不计算滚动条大小
    rcClient.top+=m_nHeaderHeight;

    CSize size = rcClient.Size();
    //  关闭滚动条
    m_wBarVisible = DUISB_NULL;

    if (size.cy<szView.cy || (size.cy<szView.cy+m_nSbWid && size.cx<szView.cx))
    {
        //  需要纵向滚动条
        m_wBarVisible |= DUISB_VERT;
        m_siVer.nMin  = 0;
        m_siVer.nMax  = szView.cy-1;
        m_siVer.nPage = GetCountPerPage(FALSE)*m_nItemHeight;

        if (size.cx-m_nSbWid < szView.cx)
        {
            //  需要横向滚动条
            m_wBarVisible |= DUISB_HORZ;

            m_siHoz.nMin  = 0;
            m_siHoz.nMax  = szView.cx-1;
            m_siHoz.nPage = size.cx-m_nSbWid > 0 ? size.cx-m_nSbWid : 0;
        }
        else
        {
            //  不需要横向滚动条
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin  = 0;
            m_siHoz.nMax  = m_siHoz.nPage-1;
            m_siHoz.nPos  = 0;
            m_ptOrigin.x  = 0;
        }
    }
    else
    {
        //  不需要纵向滚动条
        m_siVer.nPage = size.cy;
        m_siVer.nMin  = 0;
        m_siVer.nMax  = size.cy-1;
        m_siVer.nPos  = 0;
        m_ptOrigin.y  = 0;

        if (size.cx < szView.cx)
        {
            //  需要横向滚动条
            m_wBarVisible |= DUISB_HORZ;
            m_siHoz.nMin  = 0;
            m_siHoz.nMax  = szView.cx-1;
            m_siHoz.nPage = size.cx;
        }
        else
        {
            //  不需要横向滚动条
            m_siHoz.nPage = size.cx;
            m_siHoz.nMin  = 0;
            m_siHoz.nMax  = m_siHoz.nPage-1;
            m_siHoz.nPos  = 0;
            m_ptOrigin.x  = 0;
        }
    }

    SetScrollPos(TRUE, m_siVer.nPos, TRUE);
    SetScrollPos(FALSE, m_siHoz.nPos, TRUE);

    //  重新计算客户区及非客户区
    DuiSendMessage(WM_NCCALCSIZE);

    //  根据需要调整原点位置
    if (HasScrollBar(FALSE) && m_ptOrigin.x+m_siHoz.nPage>szView.cx)
    {
        m_ptOrigin.x = szView.cx-m_siHoz.nPage;
    }

    if (HasScrollBar(TRUE) && m_ptOrigin.y+m_siVer.nPage>szView.cy)
    {
        m_ptOrigin.y = szView.cy-m_siVer.nPage;
    }

    NotifyInvalidate();
}

//更新表头位置
void CDuiListCtrl::UpdateHeaderCtrl()
{
    CRect rcClient;
    GetClient(&rcClient);
    CRect rcHeader(rcClient);
    rcHeader.bottom=rcHeader.top+m_nHeaderHeight;
    rcHeader.left-=m_ptOrigin.x;
    if(m_pHeader) m_pHeader->Move(rcHeader);
}

void CDuiListCtrl::DeleteItem(int nItem)
{
    if (nItem>=0 && nItem < GetItemCount())
    {
        DXLVITEM &lvi=m_arrItems[nItem];
        for(int i=0;i<GetColumnCount();i++)
        {
            DXLVSUBITEM &lvsi =lvi.arSubItems->GetAt(i);
            if(lvsi.strText) free(lvsi.strText);
        }
        delete lvi.arSubItems;
        m_arrItems.RemoveAt(nItem);

        UpdateScrollBar();
    }
}

void CDuiListCtrl::DeleteColumn( int iCol )
{
    if(m_pHeader->DeleteItem(iCol))
    {
        for(int i=0;i<GetItemCount();i++)
        {
            DXLVSUBITEM &lvsi=m_arrItems[i].arSubItems->GetAt(iCol);
            if(lvsi.strText) free(lvsi.strText);
            m_arrItems[i].arSubItems->RemoveAt(iCol);
        }
        UpdateScrollBar();
    }
}

void CDuiListCtrl::DeleteAllItems()
{
    m_nSelectItem = -1;
    for(int i=0;i<GetItemCount();i++)
    {
        DXLVITEM &lvi=m_arrItems[i];
        for(int j=0;j<GetColumnCount();j++)
        {
            DXLVSUBITEM &lvsi =lvi.arSubItems->GetAt(j);
            if(lvsi.strText) free(lvsi.strText);
        }
        delete lvi.arSubItems;
    }
    m_arrItems.RemoveAll();

    UpdateScrollBar();
}


CRect CDuiListCtrl::GetItemRect(int nItem, int nSubItem)
{
    if (!(nItem>=0 && nItem<GetItemCount() && nSubItem>=0 && nSubItem<GetColumnCount()))
        return CRect();

    CRect rcItem;
    rcItem.top    = m_nItemHeight*nItem;
    rcItem.bottom = rcItem.top+m_nItemHeight;
    rcItem.left   = 0;
    rcItem.right  = 0;

    for (int nCol = 0; nCol < GetColumnCount(); nCol++)
    {
        DUIHDITEM hdi;
        memset(&hdi, 0, sizeof(DUIHDITEM));

        hdi.mask = DUIHDI_WIDTH|DUIHDI_ORDER;

        m_pHeader->GetItem(nCol, &hdi);
        rcItem.left  = rcItem.right;
        rcItem.right = rcItem.left+hdi.cx;
        if (hdi.iOrder == nSubItem)
            break;
    }

    CRect rcList = GetListRect();
    //  变换到窗口坐标
    rcItem.OffsetRect(rcList.TopLeft());
    //  根据原点坐标修正
    rcItem.OffsetRect(-m_ptOrigin);

    return rcItem;
}

//////////////////////////////////////////////////////////////////////////
//  自动修改pt的位置为相对当前项的偏移量
int CDuiListCtrl::HitTest(const CPoint& pt)
{
    CRect rcList = GetListRect();

    CPoint pt2 = pt;
    pt2.y -= rcList.top - m_ptOrigin.y;

    int nRet = pt2.y / m_nItemHeight;
    if (nRet >= GetItemCount())
    {
        nRet = -1;
    }

    return nRet;
}

void CDuiListCtrl::RedrawItem(int nItem)
{
    if (!IsVisible(TRUE))
        return;

    CRect rcList = GetListRect();

    int nTopItem = GetTopIndex();
    int nPageItems    = (rcList.Height()+m_nItemHeight-1)/m_nItemHeight;

    if (nItem>=nTopItem && nItem<GetItemCount() && nItem<nTopItem+nPageItems)
    {
        CRect rcItem(0,0,rcList.Width(),m_nItemHeight);
        rcItem.OffsetRect(0, m_nItemHeight*nItem-m_ptOrigin.y);
        rcItem.OffsetRect(rcList.TopLeft());
        CRect rcDC;
        rcDC.IntersectRect(rcItem,rcList);
        CDCHandle dc = GetDuiDC(&rcDC, OLEDC_PAINTBKGND);
        DuiSendMessage(WM_ERASEBKGND, (WPARAM)(HDC)dc);

        DuiDCPaint dxDC;
        BeforePaint(dc, dxDC);

        DrawItem(dc, rcItem, nItem);

        AfterPaint(dc, dxDC);
        ReleaseDuiDC(dc);
    }
}

int CDuiListCtrl::GetCountPerPage(BOOL bPartial)
{
    CRect rcClient = GetListRect();

    // calculate number of items per control height (include partial item)
    div_t divHeight = div(rcClient.Height(), m_nItemHeight);

    // round up to nearest item count
    return max(bPartial && divHeight.rem > 0 ? divHeight.quot + 1 : divHeight.quot, 1);
}
BOOL CDuiListCtrl::SortItems(
               PFNLVCOMPAREEX pfnCompare,
               void * pContext 
               )
{
    qsort_s(m_arrItems.GetData(),m_arrItems.GetCount(),sizeof(DXLVITEM),pfnCompare,pContext);
    m_nSelectItem=-1;
    m_nHoverItem=-1;
    NotifyInvalidateRect(GetListRect());
    return TRUE;
}

void CDuiListCtrl::OnPaint(CDCHandle dc)
{
    DuiDCPaint dxDC;
    BeforePaint(dc, dxDC);

    CRect rcList = GetListRect();
    int nTopItem = GetTopIndex();

    int nSave=dc.SaveDC();
    CRgn rgn;
    rgn.CreateRectRgnIndirect(rcList);
    dc.SelectClipRgn(rgn,RGN_AND);
    CRect rcItem(rcList);

    rcItem.bottom = rcItem.top;
    rcItem.OffsetRect(0,-(m_ptOrigin.y%m_nItemHeight));
    for (int nItem = nTopItem; nItem <= (nTopItem+GetCountPerPage(TRUE)) && nItem<GetItemCount(); rcItem.top = rcItem.bottom, nItem++)
    {
        rcItem.bottom = rcItem.top + m_nItemHeight;

        DrawItem(dc, rcItem, nItem);
    }
    dc.RestoreDC(nSave);
    AfterPaint(dc, dxDC);
}

void CDuiListCtrl::DrawItem(CDCHandle dc, CRect rcItem, int nItem)
{
    BOOL bTextColorChanged = FALSE;
    int nBgImg = 0;
    COLORREF crOldText;
    COLORREF crItemBg = m_crItemBg;
    COLORREF crText = m_crText;
    DXLVITEM lvItem = m_arrItems[nItem];
    CRect rcIcon, rcText;


    if (nItem == m_nSelectItem)
    {
        if (m_pItemSkin != NULL)
            nBgImg = 2;
        else if (CLR_INVALID != m_crItemSelBg)
            crItemBg = m_crItemSelBg;

        if (CLR_INVALID != m_crSelText)
            crText = m_crSelText;
    }
    else if(nItem % 2)
    {
        if (m_pItemSkin != NULL)
            nBgImg = 1;
        else if (CLR_INVALID != m_crItemBg2)
            crItemBg = m_crItemBg2;
    }

    //  绘制背景
    if (m_pItemSkin != NULL)
        m_pItemSkin->Draw(dc, rcItem, nBgImg);
    else if (CLR_INVALID != crItemBg)
        CGdiAlpha::FillSolidRect(dc, rcItem, crItemBg);

    //  左边加上空白
    rcItem.left += m_nMargin;

    if (CLR_INVALID != crText)
    {
        bTextColorChanged = TRUE;
        crOldText = dc.SetTextColor(crText);
    }

    CRect rcCol(rcItem);
    rcCol.right = rcCol.left;
    rcCol.OffsetRect(-m_ptOrigin.x,0);

    for (int nCol = 0; nCol < GetColumnCount(); nCol++)
    {
        CRect rcVisiblePart;

        DUIHDITEM hdi;
        hdi.mask=DUIHDI_WIDTH|DUIHDI_ORDER;
        m_pHeader->GetItem(nCol,&hdi);
        rcCol.left=rcCol.right;
        rcCol.right = rcCol.left + hdi.cx;

        rcVisiblePart.IntersectRect(rcItem, rcCol);

        if (rcVisiblePart.IsRectEmpty())
            continue;

        DXLVSUBITEM& subItem = lvItem.arSubItems->GetAt(hdi.iOrder);

        if (subItem.nImage != -1 && m_pIconSkin)
        {
            int nOffsetX = m_ptIcon.x;
            int nOffsetY = m_ptIcon.y;
            CSize sizeSkin = m_pIconSkin->GetSkinSize();
            rcIcon.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);

            if (m_ptIcon.x == -1)
                nOffsetX = m_nItemHeight / 6;

            if (m_ptIcon.y == -1)
                nOffsetY = (m_nItemHeight - sizeSkin.cy) / 2;

            rcIcon.OffsetRect(rcCol.left + nOffsetX, rcCol.top + nOffsetY);
            m_pIconSkin->Draw(dc, rcIcon, subItem.nImage);
        }

        UINT align = DT_SINGLELINE;
        rcText = rcCol;

        if (m_ptText.x == -1)
            rcText.left = rcIcon.Width() > 0 ? rcIcon.right + m_nItemHeight / 6 : rcCol.left;
        else
            rcText.left = rcCol.left + m_ptText.x;

        if (m_ptText.y == -1)
            align |= DT_VCENTER;
        else
            rcText.top = rcCol.top + m_ptText.y;

        CGdiAlpha::DrawText(dc, subItem.strText, subItem.cchTextMax, rcText, align);
    }

    if (bTextColorChanged)
        dc.SetTextColor(crOldText);
}

void CDuiListCtrl::OnDestroy()
{
    DeleteAllItems();

    __super::OnDestroy();
}

int CDuiListCtrl::GetColumnCount()
{
    if (!m_pHeader)
        return 0;

    return m_pHeader->GetItemCount();
}

int CDuiListCtrl::GetTopIndex() const
{
    return m_ptOrigin.y / m_nItemHeight;
}

void CDuiListCtrl::NotifySelChange(int nOldSel, int nNewSel)
{
    DUINMLBSELCHANGE nms;
    nms.hdr.code     = NM_LBSELCHANGING;
    nms.hdr.hDuiWnd = m_hDuiWnd;
    nms.hdr.idFrom   = GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.nOldSel      = nOldSel;
    nms.nNewSel      = nNewSel;
    nms.uHoverID     = 0;

    if (S_OK != DuiNotify((LPDUINMHDR)&nms))
        return;

    m_nSelectItem = nNewSel;
    if (nOldSel != -1)
        RedrawItem(nOldSel);

    if (m_nSelectItem != -1)
        RedrawItem(m_nSelectItem);

    nms.hdr.idFrom = GetCmdID();
    nms.hdr.code   = NM_LBSELCHANGED;
    DuiNotify((LPDUINMHDR)&nms);            
}

BOOL CDuiListCtrl::OnScroll(BOOL bVertical, UINT uCode, int nPos)
{
    BOOL bRet = __super::OnScroll(bVertical, uCode, nPos);

    if (bVertical)
    {
        m_ptOrigin.y = m_siVer.nPos;
    }
    else
    {
        m_ptOrigin.x = m_siHoz.nPos;
        //  处理列头滚动
        UpdateHeaderCtrl();
    }

    NotifyInvalidate();
    if (uCode==SB_THUMBTRACK)
        ScrollUpdate();

    return bRet;
}

void CDuiListCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
    m_nHoverItem = HitTest(pt);

    if (m_nHoverItem!=m_nSelectItem && !m_bHotTrack)
        NotifySelChange(m_nSelectItem, m_nHoverItem);
}

void CDuiListCtrl::OnLButtonUp(UINT nFlags, CPoint pt)
{
    m_nHoverItem = HitTest(pt);

    if (m_bHotTrack || m_nHoverItem!=m_nSelectItem)
        NotifySelChange(m_nSelectItem, m_nHoverItem);
}


void CDuiListCtrl::UpdateChildrenPosition()
{
    __super::UpdateChildrenPosition();
    UpdateHeaderCtrl();
}

void CDuiListCtrl::OnSize(UINT nType, CSize size)
{
    UpdateScrollBar();
    UpdateHeaderCtrl();
}

bool CDuiListCtrl::OnHeaderClick(CDuiWindow* pSender, LPDUINMHDR pNmhdr)
{
    return true;
}

bool CDuiListCtrl::OnHeaderSizeChanging(CDuiWindow* pSender, LPDUINMHDR pNmhdr)
{
    UpdateScrollBar();
    NotifyInvalidateRect(GetListRect());

    return true;
}

bool CDuiListCtrl::OnHeaderSwap(CDuiWindow* pSender, LPDUINMHDR pNmhdr)
{
    NotifyInvalidateRect(GetListRect());

    return true;
}


}//end of namespace 
