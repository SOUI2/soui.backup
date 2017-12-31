#include "stdafx.h"
#include "SApp.h"
#include "SFileList.h"

//#pragma warning(disable : 4267 4018)

#define ITEM_MARGIN 4
namespace SOUI
{
//////////////////////////////////////////////////////////////////////////
//  SListCtrl
SFileList::SFileList()
    : m_nHeaderHeight(20)
    , m_nItemHeight(20)
    , m_pHeader(NULL)
    , m_nSelectItem(-1)
	, m_nHoverItem(-1)
	, m_nClickItem(-1)
    , m_crItemBg(RGBA(255,255,255,255))
    , m_crItemBg2(RGBA(226,226,226,255))
	, m_crItemSelBg(RGBA(200,232,255,255))
	, m_crItemHotBg(RGBA(200,232,255,128))
    , m_crText(RGBA(0,0,0,255))
    , m_crSelText(RGBA(0,0,0,255))
    , m_pItemSkin(NULL)
    , m_pIconSkin(NULL)
    , m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_CHECKBOX))
    , m_ptIcon(-1,-1)
    , m_ptText(-1,-1)
    , m_bHotTrack(FALSE)
    , m_bCheckBox(FALSE)
    , m_bMultiSelection(FALSE)
	, m_bLdbClicked(false)
{
	//m_iFLAdapter = NULL;
    m_bClipClient = TRUE;
    m_bFocusable = TRUE;
    m_evtSet.addEvent(EVENTID(EventLCSelChanging));
    m_evtSet.addEvent(EVENTID(EventLCSelChanged));
    m_evtSet.addEvent(EVENTID(EventFLClick));
	m_evtSet.addEvent(EVENTID(EventFLDBClick));
	m_evtSet.addEvent(EVENTID(EventFLMenu));
	m_evtSet.addEvent(EVENTID(EventFLColClick));
	m_evtSet.addEvent(EVENTID(EventFLBeginDrag));
}

SFileList::~SFileList()
{
	//delete m_iListDataSet;
	//m_iListDataSet = NULL;
}

int SFileList::InsertColumn(int nIndex, LPCTSTR pszText, int nWidth, LPARAM lParam)
{
    SASSERT(m_pHeader);

    int nRet = m_pHeader->InsertItem(nIndex, pszText, nWidth, ST_NULL, lParam);

	for(int i=0;i<GetItemCount();i++)
	{
		m_arrItems[i].arrSubItems->SetCount(GetColumnCount());
	}
    UpdateScrollBar();
    return nRet;
}

BOOL SFileList::CreateChildren(pugi::xml_node xmlNode)
{
    //  listctrl的子控件只能是一个header控件
    if (!__super::CreateChildren(xmlNode))
        return FALSE;
    m_pHeader = NULL;
    
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    while(pChild)
    {
        if(pChild->IsClass(SHeaderCtrl::GetClassName()))
        {
            m_pHeader = (SHeaderCtrl*)pChild;
            break;
        }
        pChild=pChild->GetWindow(GSW_NEXTSIBLING);
    }

    if(NULL == m_pHeader) return FALSE;
        
    SStringW strPos;
    strPos.Format(L"0,0,-0,%d",m_nHeaderHeight);
    m_pHeader->SetAttribute(L"pos",strPos,TRUE);

    m_pHeader->GetEventSet()->subscribeEvent<SFileList, EventHeaderItemChanging>(&SFileList::OnHeaderSizeChanging,this);
    m_pHeader->GetEventSet()->subscribeEvent<SFileList, EventHeaderItemSwap>(&SFileList::OnHeaderSwap, this);
	m_pHeader->GetEventSet()->subscribeEvent<SFileList, EventHeaderClick>(&SFileList::OnHeaderClick,this);

    return TRUE;
}

int SFileList::InsertItem(int nItem, LPCTSTR pszText, int nImage/*=-1*/)
{
	if(GetColumnCount() == 0) 
		return -1;

	if (nItem < 0 || nItem > GetItemCount())
		nItem = GetItemCount();

	FLItemData lvData;
	lvData.nImage = nImage;
	lvData.dwData = 0;
	lvData.arrSubItems = new SArray<FLSubData>();
	lvData.arrSubItems->SetCount(GetColumnCount());

	FLSubData& subItem = lvData.arrSubItems->GetAt(0);
	subItem.lpszText = _tcsdup(pszText);
	subItem.nTextLen = _tcslen(pszText);
	
	m_arrItems.InsertAt(nItem, lvData);

	UpdateScrollBar();

	return nItem;
}

BOOL SFileList::SetSubItemText(int nItem, int nSubItem, LPCTSTR pszText)
{
	if (nItem < 0 || nItem >= GetItemCount())
		return FALSE;

	if (nSubItem < 0 || nSubItem >= GetColumnCount())
		return FALSE;

	FLSubData& lvi = m_arrItems[nItem].arrSubItems->GetAt(nSubItem);
	if(NULL != lvi.lpszText)
	{
		free(lvi.lpszText);
	}
	lvi.lpszText = _tcsdup(pszText);
	lvi.nTextLen= _tcslen(pszText);

	CRect rcItem = GetItemRect(nItem,nSubItem);
	InvalidateRect(rcItem);
	return TRUE;
}

SStringT SFileList::GetSubItemText(int nItem, int nSubItem) const
{
	if (nItem < 0 || nItem >= GetItemCount() || nSubItem < 0 || nSubItem >= GetColumnCount())
		return _T("");

	const FLSubData& lvsi_src = m_arrItems[nItem].arrSubItems->GetAt(nSubItem);
	return lvsi_src.lpszText;
}

void SFileList::DeleteItem(int nItem)
{
	if (nItem < 0 || nItem >= GetItemCount())
		return ;

	FLItemData& lvi = m_arrItems[nItem];
	
	for(int i=0; i<GetColumnCount(); ++i)
	{
		FLSubData& lvsi = lvi.arrSubItems->GetAt(i);
		if(lvsi.lpszText) free(lvsi.lpszText);
	}
	delete lvi.arrSubItems;
	m_arrItems.RemoveAt(nItem);

	UpdateScrollBar();
}

void SFileList::DeleteAllItems()
{
	m_nSelectItem = -1;
	for(int i=0; i<GetItemCount(); ++i)
	{
		FLItemData& lvi = m_arrItems[i];
		
		for(int j=0; j<GetColumnCount(); ++j)
		{
			FLSubData& lvsi = lvi.arrSubItems->GetAt(j);
			if(lvsi.lpszText) 
				free(lvsi.lpszText);
		}
		delete lvi.arrSubItems;
	}
	m_arrItems.RemoveAll();

	UpdateScrollBar();
}

int SFileList::GetItemCount() const
{
	if (GetColumnCount() <= 0)
		return 0;

	return m_arrItems.GetCount();
}

BOOL SFileList::SetItemData(int nItem, DWORD dwData)
{
	if (nItem < 0 || nItem >= GetItemCount())
		return FALSE;

	m_arrItems[nItem].dwData = dwData;
	return TRUE;
}

DWORD SFileList::GetItemData(int nItem)
{
	if (nItem < 0 || nItem >= GetItemCount())
		return 0;

	FLItemData& lvi = m_arrItems[nItem];

	return (DWORD)lvi.dwData;
}

int SFileList::GetSelectedItem()
{
    return m_nSelectItem;
}

void SFileList::SetSelectedItem(int nItem)
{
    m_nSelectItem = nItem;
    
    Invalidate();
}

CRect SFileList::GetListRect()
{
    CRect rcList;

    GetClientRect(&rcList);
    rcList.top += m_nHeaderHeight;

    return rcList;
}

//////////////////////////////////////////////////////////////////////////
//  更新滚动条
void SFileList::UpdateScrollBar()
{
	//if(NULL == m_iFLAdapter)
	//	return ;

    CSize szView;
    szView.cx = m_pHeader->GetTotalWidth();
    szView.cy = GetItemCount() * m_nItemHeight;

    CRect rcClient;
    SWindow::GetClientRect(&rcClient);//不计算滚动条大小
    rcClient.top += m_nHeaderHeight;

    CSize size = rcClient.Size();
    //  关闭滚动条
    m_wBarVisible = SSB_NULL;

    if (size.cy<szView.cy || (size.cy<szView.cy+ GetSbWidth() && size.cx<szView.cx))
    {
        //  需要纵向滚动条
        m_wBarVisible |= SSB_VERT;
        m_siVer.nMin  = 0;
        m_siVer.nMax  = szView.cy-1;
        m_siVer.nPage = GetCountPerPage(FALSE)*m_nItemHeight;

        if (size.cx- GetSbWidth() < szView.cx)
        {
            //  需要横向滚动条
            m_wBarVisible |= SSB_HORZ;
            m_siVer.nPage=size.cy- GetSbWidth() > 0 ? size.cy- GetSbWidth() : 0;//注意同时调整纵向滚动条page信息

            m_siHoz.nMin  = 0;
            m_siHoz.nMax  = szView.cx-1;
            m_siHoz.nPage = size.cx- GetSbWidth() > 0 ? size.cx- GetSbWidth() : 0;
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
            m_wBarVisible |= SSB_HORZ;
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
    SSendMessage(WM_NCCALCSIZE);

    //  根据需要调整原点位置
    if (HasScrollBar(FALSE) && m_ptOrigin.x+m_siHoz.nPage>szView.cx)
    {
        m_ptOrigin.x = szView.cx-m_siHoz.nPage;
    }

    if (HasScrollBar(TRUE) && m_ptOrigin.y+m_siVer.nPage>szView.cy)
    {
        m_ptOrigin.y = szView.cy-m_siVer.nPage;
    }

    Invalidate();
}

//更新表头位置
void SFileList::UpdateHeaderCtrl()
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcHeader(rcClient);
    rcHeader.bottom=rcHeader.top+m_nHeaderHeight;
    rcHeader.left-=m_ptOrigin.x;
    if(m_pHeader) m_pHeader->Move(rcHeader);
}

void SFileList::DeleteColumn( int iCol )
{
    if(m_pHeader->DeleteItem(iCol))
    {
        UpdateScrollBar();
    }
}

CRect SFileList::GetItemRect(int nItem, int nSubItem)
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
        SHDITEM hdi;
        //memset(&hdi, 0, sizeof(SHDITEM));

        hdi.mask = SHDI_WIDTH|SHDI_ORDER;

        m_pHeader->GetItem(nCol, &hdi);
        rcItem.left  = rcItem.right;
        rcItem.right = rcItem.left+hdi.cx.toPixelSize(GetScale());
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

void SFileList::RedrawItem(int nItem)
{
    if (!IsVisible(TRUE))
        return;

    CRect rcList = GetListRect();

    int nTopItem = GetTopIndex();
    int nPageItems = (rcList.Height() + m_nItemHeight-1) / m_nItemHeight;

    if (nItem>=nTopItem && nItem<GetItemCount() && nItem<=nTopItem+nPageItems)
    {
        //CRect rcItem(0,0,rcList.Width(),m_nItemHeight);
		CRect rcItem(0, 0, m_pHeader->GetTotalWidth(), m_nItemHeight);
        rcItem.OffsetRect(0, m_nItemHeight * nItem-m_ptOrigin.y);
        rcItem.OffsetRect(rcList.TopLeft());
        CRect rcDC;
        rcDC.IntersectRect(rcItem, rcList);
        IRenderTarget *pRT = GetRenderTarget(&rcDC, OLEDC_PAINTBKGND);
        SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);

        DrawItem(pRT, rcItem, nItem);

        ReleaseRenderTarget(pRT);
    }
}

int SFileList::GetCountPerPage(BOOL bPartial)
{
    CRect rcClient = GetListRect();

    // calculate number of items per control height (include partial item)
    div_t divHeight = div(rcClient.Height(), m_nItemHeight);

    // round up to nearest item count
    return max(bPartial && divHeight.rem > 0 ? divHeight.quot + 1 : divHeight.quot, 1);
}

bool SFileList::SetHeaderItemSort(int nCol, bool bAsc)
{
	//更新表头的排序状态
	for(int i=0; i<m_pHeader->GetItemCount(); ++i)
	{
		m_pHeader->SetItemSort(i, ST_NULL);
	}
		
	if(-1 != nCol)
		m_pHeader->SetItemSort(nCol, bAsc ? ST_UP : ST_DOWN);

	return true;
}

BOOL SFileList::SortItems(pFileListSortFun pfnCompare, void * pContext )
{
	qsort_s(m_arrItems.GetData(), m_arrItems.GetCount(), sizeof(FLItemData), (int (__cdecl *)(void *,const void *,const void *))pfnCompare, pContext);
	m_nSelectItem = -1;
	m_nHoverItem = -1;
	m_nClickItem = -1;
	Invalidate();
	return TRUE;
}

void SFileList::OnPaint(IRenderTarget * pRT)
{
    SPainter painter;
    BeforePaint(pRT, painter);
	//绘制 选中框
	CRect rcSelect(m_ptClick, m_ptMove);
	//if(0 == m_ptClick.x && 0 == m_ptClick.y)
	//	rcSelect.SetRectEmpty();
	rcSelect.NormalizeRect();

	
    CRect rcList = GetListRect();

    int nTopItem = GetTopIndex();
    pRT->PushClipRect(&rcList);
    CRect rcItem(rcList);
	rcItem.right = rcItem.left + m_pHeader->GetTotalWidth();
    rcItem.bottom = rcItem.top;
    rcItem.OffsetRect(0, -(m_ptOrigin.y%m_nItemHeight));
	
    for (int nItem = nTopItem; nItem <= (nTopItem+GetCountPerPage(TRUE)) && nItem<GetItemCount(); rcItem.top = rcItem.bottom, nItem++)
    {
        rcItem.bottom = rcItem.top + m_nItemHeight;
	
		//只有 在 按下  和 rcselct 不为空 才 计算 选中区域 
		if(m_bStartSelect && (WndState_PushDown==(m_dwState&WndState_PushDown)) && !rcSelect.IsRectEmpty())
		{
			FLItemData& lvi = m_arrItems.GetAt(nItem);
			CRect rcIn;
			if(rcIn.IntersectRect(rcSelect, rcItem) && !rcIn.IsRectEmpty())
			{
				lvi.bChecked = true;
				m_nSelectItem = nItem;
			}
			else 
				lvi.bChecked = false;
		}
		
        DrawItem(pRT, rcItem, nItem);
    }
	if(m_bStartSelect && !rcSelect.IsRectNull())
	{
		pRT->FillSolidRect(rcSelect, RGBA(137,189,238,100));
		CAutoRefPtr<IPen> pPen, pOldPen;
		pRT->CreatePen(PS_SOLID, RGBA(137,189,238,255), 1, &pPen);
		pRT->SelectObject(pPen, (IRenderObj**)&pOldPen);
		
		pRT->DrawRectangle(rcSelect);
		pRT->SelectObject(pOldPen);
		/*
		//调整颜色值
		SDIBHelper::Colorize(m_colors.m_crBorder,m_crColorize);
		for(int i=0;i<4;i++)
		{
		SDIBHelper::Colorize(m_colors.m_crDown[i],m_crColorize);
		SDIBHelper::Colorize(m_colors.m_crUp[i],m_crColorize);
		}*/
	}

    pRT->PopClip();
    AfterPaint(pRT, painter);
}
#include <helper/SDIBHelper.h>
void SFileList::DrawItem(IRenderTarget * pRT, CRect rcItem, int nItem)
{
	BOOL bTextColorChanged = FALSE;
	int nBgImg = 0;
	COLORREF crOldText;
	COLORREF crItemBg = m_crItemBg;
	COLORREF crText = m_crText;

	FLItemData& lvi = m_arrItems.GetAt(nItem);
	if (lvi.bChecked)	//判断 是否 选中
	{
		if (m_pItemSkin != NULL)
			nBgImg = 2;

		if (CR_INVALID != m_crItemSelBg)
			crItemBg = m_crItemSelBg;

		if (CR_INVALID != m_crSelText)
			crText = m_crSelText;
	}
	else if  (m_bHotTrack && nItem == m_nHoverItem)		//判断鼠标hover  sel 会覆盖hover
	{
		if (m_pItemSkin != NULL)
			nBgImg = 1;
		else if (CR_INVALID != m_crItemHotBg)
			crItemBg = m_crItemHotBg;

		if (CR_INVALID != m_crSelText)
			crText = m_crSelText;
	}

	if (m_pItemSkin != NULL)//有skin，则画 skin
	{
		m_pItemSkin->Draw(pRT, rcItem, nBgImg); 
	}
	else if(CR_INVALID != crItemBg)//没有 就画背景
	{
		pRT->FillSolidRect(rcItem, crItemBg);
		if(lvi.bChecked && m_nHoverItem == nItem)
		{
			CAutoRefPtr<IPen> curPen,oldPen;
			//pRT->CreatePen(PS_SOLID, RGBA(137,189,238,200), 1, &curPen);
			int R = GetRValue(m_crItemSelBg) - 50;
			int G = GetGValue(m_crItemSelBg) - 50;
			int B = GetBValue(m_crItemSelBg) - 50;
			COLORREF dw = RGBA(R, G, B, 255);
			pRT->CreatePen(PS_SOLID, dw, 1, &curPen);
			pRT->SelectObject(curPen,(IRenderObj**)&oldPen);
			CRect rc = rcItem;
			--rc.bottom;
			--rc.right;
			pRT->DrawRectangle(&rc);
			pRT->SelectObject(oldPen);
		}		
	}
		
	//设置 文本 颜色 
	if (CR_INVALID != crText && m_crText != crText)
	{
		bTextColorChanged = TRUE;
		crOldText = pRT->SetTextColor(crText);
	}

	//  左边加上空白
	rcItem.left += ITEM_MARGIN;

	CRect rcCol(rcItem);
	rcCol.right = rcCol.left;
	rcCol.OffsetRect(-m_ptOrigin.x, 0);

	m_nFirstColOffsetX = 0;
	// 绘制 checkbox
	if (m_bCheckBox && NULL != m_pCheckSkin)
	{
		CSize sizeSkin = m_pCheckSkin->GetSkinSize();
		int nOffsetY = (m_nItemHeight - sizeSkin.cy) / 2;
		CRect rcCheck;
		rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
		rcCheck.OffsetRect(rcCol.left, rcCol.top + nOffsetY);
		m_pCheckSkin->Draw(pRT, rcCheck, lvi.bChecked ? 4 : 0);

		m_nFirstColOffsetX += sizeSkin.cx;
	}

	//int nImage = pData->GetIcon();
	if (NULL != m_pIconSkin && -1 != lvi.nImage)
	{
		int nOffsetX = m_ptIcon.x;
		int nOffsetY = m_ptIcon.y;
		CSize sizeSkin = m_pIconSkin->GetSkinSize();
		CRect rcIcon(0, 0, sizeSkin.cx, sizeSkin.cy);

		if (m_ptIcon.x == -1)
			nOffsetX = (m_nItemHeight - sizeSkin.cx) / 2;

		if (m_ptIcon.y == -1)
			nOffsetY = (m_nItemHeight - sizeSkin.cy) / 2;

		rcIcon.OffsetRect(rcCol.left + nOffsetX + m_nFirstColOffsetX, rcCol.top + nOffsetY);
		m_pIconSkin->Draw(pRT, rcIcon, lvi.nImage);

		m_nFirstColOffsetX += sizeSkin.cx + nOffsetX + ITEM_MARGIN;
	}


	for (int nCol = 0; nCol < GetColumnCount(); nCol++)
	{
		CRect rcVisiblePart;

		SHDITEM hdi;
		hdi.mask = SHDI_WIDTH | SHDI_ORDER;
		m_pHeader->GetItem(nCol,&hdi);
		rcCol.left = rcCol.right;
		rcCol.right = rcCol.left + hdi.cx.toPixelSize(GetScale());

		rcVisiblePart.IntersectRect(rcItem, rcCol);

		if (rcVisiblePart.IsRectEmpty())
			continue;

		UINT align = DT_SINGLELINE;

		if (m_ptText.x == -1)
		{
			if(0 == nCol)   //只有 第一列才 会计算这个
				rcCol.left += m_nFirstColOffsetX;
		}
		else
			rcCol.left = rcCol.left + m_ptText.x;

		if (m_ptText.y == -1)
			align |= DT_VCENTER;
		else
			rcCol.top = rcCol.top + m_ptText.y;

		LPCTSTR lpText = lvi.arrSubItems->GetAt(hdi.iOrder).lpszText;
		pRT->DrawText(lpText, lstrlen(lpText), rcCol, align);
	}

	if (bTextColorChanged)
		pRT->SetTextColor(crOldText);
}

//////////////////////////////////////////////////////////////////////////

int SFileList::HitTest(const CPoint& pt)
{
	CRect rcList = GetListRect();
	if(pt.x > (m_pHeader->GetTotalWidth() + rcList.left))
		return -1;

	CPoint pt2 = pt;
	pt2.y -= rcList.top - m_ptOrigin.y;		//  自动修改pt的位置为相对当前项的偏移量

	int nRet = pt2.y / m_nItemHeight;
	if (nRet >= GetItemCount())
	{
		nRet = -1;
	}

	return nRet;
}
/*
BOOL SFileList::HitCheckBox(const CPoint& pt)
{
    if (!m_bCheckBox)
        return FALSE;

    CRect rect = GetListRect();
    rect.left += ITEM_MARGIN;
    rect.OffsetRect(-m_ptOrigin.x,0);

    CSize sizeSkin = m_pCheckSkin->GetSkinSize();
	CRect rcCheck;
    rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
    rcCheck.OffsetRect(rect.left, 0);

    if (pt.x >= rcCheck.left && pt.x <= rcCheck.right)
        return TRUE;

    return FALSE;
}
*/
SFileList::EnHitItemPos SFileList::HitItemPos(int nItem, const CPoint& pt)
{
	if(-1 == nItem)
		return eHIP_NULL;

	EnHitItemPos ePos = eHIP_NULL;

	CRect rect = GetListRect();
	//rect.left += ITEM_MARGIN;
	rect.OffsetRect(-m_ptOrigin.x,0);

	if (m_bCheckBox)  //如果 有checkbox 就判断 
	{
		CSize sizeSkin = m_pCheckSkin->GetSkinSize();
		int nOffsetX = ITEM_MARGIN;  //因为 left
		CRect rcCheck;
		rcCheck.SetRect(0, 0, sizeSkin.cx, sizeSkin.cy);
		rcCheck.OffsetRect(rect.left + nOffsetX, 0);

		if (pt.x >= rcCheck.left && pt.x <= rcCheck.right)
		{
			return eHIP_CheckBox;
		}
	}

	if(NULL == m_pHeader)
	{
		return ePos;
	}

	int nPtX = pt.x - rect.left;

	// 判断 鼠标落到了  哪一列
	int nCol = 0;
	int nColTotalWidth = 0;
	int nColCount = m_pHeader->GetItemCount();
	for (int i=0; i<nColCount; ++i)
	{
		int nWidth = m_pHeader->GetItemWidth(i);
		if(nPtX < (nWidth + nColTotalWidth))
		{
			nCol = i;
			break;
		}
		//放到这里加  是因为下面 还要用到
		nColTotalWidth += nWidth;
	}

	LPCTSTR lpText = m_arrItems.GetAt(nItem).arrSubItems->GetAt(nCol).lpszText;
	
	//计算 文本长度 
	CAutoRefPtr<IRenderTarget> pRT;
	SApplication::getSingleton().GetRenderFactory()->CreateRenderTarget(&pRT, 0, 0);
	BeforePaintEx(pRT);	
	CSize szText;
	pRT->MeasureText(lpText, -1, &szText);
	if(0 == nCol && 0 != m_nFirstColOffsetX)
		szText.cx += m_nFirstColOffsetX;

	if((nColTotalWidth + szText.cx + ITEM_MARGIN) > nPtX)
		ePos = eHIP_TextContent;

	return ePos;
}

void SFileList::OnDestroy()
{    
    __super::OnDestroy();
}

int SFileList::GetColumnCount() const
{
    if (!m_pHeader)
        return 0;

    return m_pHeader->GetItemCount();
}

int SFileList::GetTopIndex() const
{
    return m_ptOrigin.y / m_nItemHeight;
}

void SFileList::NotifySelChange(int nOldSel, int nNewSel, BOOL checkBox)
{
    EventLCSelChanging evt1(this);
    evt1.nOldSel = nOldSel;
    evt1.nNewSel = nNewSel;

    FireEvent(evt1);
    if(evt1.bCancel) 
		return;

	if (checkBox)   //如果 是点击 checkbox  就值处理这一项 就行了
	{ 
		if(-1 == nNewSel)return ;

		FLItemData& lvi = m_arrItems.GetAt(nNewSel);
			
		lvi.bChecked = !lvi.bChecked;
		m_nSelectItem = nNewSel;
		RedrawItem(nNewSel);
    } 
	else     // 点击的是 其他的
	{
		// 多选 或者  有check   再 加  按住了 Ctrl键
        if ((m_bMultiSelection || m_bCheckBox) && GetKeyState(VK_CONTROL) < 0) 
		{
            if (nNewSel != -1) 
			{
				FLItemData& lvi = m_arrItems.GetAt(nNewSel);
				lvi.bChecked = !lvi.bChecked;
                m_nSelectItem = nNewSel;
                RedrawItem(nNewSel);
            }
        }
		// 多选 或者  有check   再 加  按住了 Shift键
		else if ((m_bMultiSelection || m_bCheckBox) && GetKeyState(VK_SHIFT) < 0) 
		{
            if (nNewSel != -1)
			{
                if (nOldSel == -1)
                    nOldSel = 0;

                int imax = (nOldSel > nNewSel) ? nOldSel : nNewSel;
                int imin = (imax == nOldSel) ? nNewSel : nOldSel;
                for (int i = 0; i < GetItemCount(); i++)
                {
					FLItemData& lvi = m_arrItems.GetAt(i);
                    bool last = lvi.bChecked;
                    if (i >= imin && i<= imax)
					{
                        lvi.bChecked = true;
                    }
					else
					{
                        lvi.bChecked = false;
                    }

                    if (last != lvi.bChecked)
                        RedrawItem(i);
                }
            }
        } 
		else   //只是 单击了 一项
		{
            m_nSelectItem = -1;
            for (int i = 0; i < GetItemCount(); i++)  
            {
				FLItemData& lvi = m_arrItems.GetAt(i);
                if (i != nNewSel && lvi.bChecked)  //先 循环  将选中的 全部取消选中   除了这项
                {
                    bool last = lvi.bChecked;
                    lvi.bChecked = false;			
                    if (last != lvi.bChecked)  //  只处理 选中的  
                        RedrawItem(i);
                }
            }
            if (nNewSel != -1)
			{
                FLItemData& lvi = m_arrItems.GetAt(nNewSel);
                lvi.bChecked = true;
                m_nSelectItem = nNewSel;
                RedrawItem(nNewSel);
            }
        }
    }
    
    EventLCSelChanged evt2(this);
    evt2.nOldSel=nOldSel;
    evt2.nNewSel=nNewSel;
    FireEvent(evt2);
}

BOOL SFileList::OnScroll(BOOL bVertical, UINT uCode, int nPos)
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

    Invalidate();
    if (uCode==SB_THUMBTRACK)
        ScrollUpdate();

    return bRet;
}

void SFileList::OnLButtonDbClick(UINT nFlags, CPoint point)
{
	int nClickItem = HitTest(point);
	EventFLDBClick eEvt(this);
	eEvt.iItem = nClickItem;
	FireEvent(eEvt);

	m_bLdbClicked = true;
}

void SFileList::OnLButtonDown(UINT nFlags, CPoint pt)
{
	__super::OnLButtonDown(nFlags,pt);
	m_bDroped = FALSE;
	m_nClickItem = HitTest(pt);
	
	if(-1 != m_nClickItem)
	{
		if(GetCheckState(m_nClickItem))			// 如果 点击的是 选中项  那么就不能 拖选
		{
			//m_ptClick.SetPoint(0, 0);
			m_bStartSelect = FALSE;
			return ;
		}
		EnHitItemPos ePos = HitItemPos(m_nClickItem, pt);
		if(eHIP_TextContent == ePos)				//  点击在 文本内容上  就直接选中  这和 win上explorer 一样
		{
			NotifySelChange(m_nSelectItem, m_nClickItem);
			//m_ptClick.SetPoint(0, 0);
			m_bStartSelect = FALSE;
			 
			EventFLClick eEvt(this);
			eEvt.iItem = m_nClickItem;
			FireEvent(eEvt);
			return ;
		}
		
	}
	m_bStartSelect = TRUE;
	m_ptClick = m_ptMove = pt;
	
}

void SFileList::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if(m_bLdbClicked)  //如果执行了 双击  那么这次单击 up 事件 就不处理了
	{
		m_bLdbClicked = false;
		__super::OnLButtonUp(nFlags, pt);
		return ;
	}

	m_bDroped = FALSE;

	//if(0 != m_ptMove.x && 0 != m_ptMove.y && m_ptClick != m_ptMove)
	if((m_ptClick != m_ptMove) && m_bStartSelect)  //相等表示 鼠标点击时 没有移动 
	{
		m_ptClick.SetPoint(0, 0);
		m_ptMove.SetPoint(0, 0);
		m_bStartSelect = FALSE;
		Invalidate();
	}
	else
	{
		int nClickUpItem = HitTest(pt);
		if(nClickUpItem == m_nClickItem)
		{
			if(-1 != nClickUpItem)
			{
				EnHitItemPos ePos = HitItemPos(nClickUpItem, pt);
				if (eHIP_CheckBox == ePos)  //点到了 checkbox 上  
				{
					NotifySelChange(m_nSelectItem, nClickUpItem, TRUE);
				}
				else if(eHIP_NULL == ePos) //点到 空白  
				{
					NotifySelChange(m_nSelectItem, nClickUpItem);
				}
				else if(eHIP_TextContent == ePos && GetCheckState(nClickUpItem))
				{
					NotifySelChange(m_nSelectItem, nClickUpItem);
				}
			}
			else  // 点击到空处  全部不选中 
			{
				NotifySelChange(m_nSelectItem, -1);
			}
			EventFLClick eEvt(this);
			eEvt.iItem = nClickUpItem;
			FireEvent(eEvt);
		}
	}
	
    __super::OnLButtonUp(nFlags, pt);
}

void SFileList::OnMouseMove( UINT nFlags, CPoint pt )
{
	//鼠标按下 的操作
	if(MK_LBUTTON == (nFlags & MK_LBUTTON) )
	{
		if(m_bStartSelect)
		{
			m_ptMove = pt;
			Invalidate();
		}
		else 
		{
			if(FALSE == m_bDroped)
			{
				EventFLBeginDrag eEvt(this);
				eEvt.iItem = m_nClickItem;
				FireEvent(eEvt);
				m_bDroped = TRUE;
				ReleaseCapture();
			}
		}
		return ;
	}

	// 正常 鼠标移动 
	int nHoverItem = HitTest(pt);
	if(m_bHotTrack && nHoverItem != m_nHoverItem)
	{
		//int nOldHoverItem = m_nHoverItem;
		m_nHoverItem = nHoverItem;
		Invalidate();   // 这里如果 不整个刷新  在滚动条的时候  界面有点怪异 
		/*if(nOldHoverItem > -1)
		RedrawItem(nOldHoverItem);
		if(m_nHoverItem > -1)
		RedrawItem(m_nHoverItem);*/
	}
}

void SFileList::OnMouseLeave()
{
	if(m_bHotTrack)
	{
		int nOldHoverItem = m_nHoverItem;
		m_nHoverItem = -1;
		if(nOldHoverItem > -1)
			RedrawItem(nOldHoverItem);
	}
	__super::OnMouseLeave();
}

void SFileList::OnRButtonDown(UINT nFlags, CPoint pt)
{
	//__super::OnRButtonDown(nFlags,pt);
	m_nClickItem = HitTest(pt);

	if(-1 != m_nClickItem)
	{
		if(GetCheckState(m_nClickItem))			// 如果 点击的是 直接返回   
		{
			return ;
		}
		EnHitItemPos ePos = HitItemPos(m_nClickItem, pt);
		if(eHIP_TextContent == ePos ) //点击在 文本内容上
		{
			NotifySelChange(m_nSelectItem, m_nClickItem);
			return ;
		}
	}
	
	NotifySelChange(m_nSelectItem, -1);
	return ;
}

void SFileList::OnRButtonUp(UINT nFlags, CPoint pt)
{
	int nClickUpItem = HitTest(pt);
	if(nClickUpItem == m_nClickItem)
	{
		EventFLMenu eEvt(this);
		eEvt.iItem = nClickUpItem;
		eEvt.pt = pt;
		FireEvent(eEvt);
	}
}

void SFileList::UpdateChildrenPosition()
{
    __super::UpdateChildrenPosition();
    UpdateHeaderCtrl();
}

void SFileList::OnSize(UINT nType, CSize size)
{
    __super::OnSize(nType,size);
    UpdateScrollBar();
    UpdateHeaderCtrl();
}

struct _SORTINFO
{
	int iCol;
	SHDSORTFLAG stFlag;
};

static int __cdecl SortCmp(void* pContext, const void* p1, const void* p2)
{
	_SORTINFO* pInfo = (_SORTINFO*)pContext;
	if(NULL == pInfo)
		return 0;

	int nRet = 0;
	do 
	{
		FLItemData* pItem1 = (FLItemData*)p1;
		if(NULL == pItem1)
		{
			nRet = -1;
			break;
		} 

		LPCTSTR lpText1 = pItem1->arrSubItems->GetAt(pInfo->iCol).lpszText;
		if(NULL == lpText1)
		{
			nRet = -1;
			break;
		} 
		
		FLItemData* pItem2 = (FLItemData*)p2;
		if(NULL == pItem2)
		{
			nRet = 1;
			break;
		} 

		LPCTSTR lpText2 = pItem2->arrSubItems->GetAt(pInfo->iCol).lpszText;
		if(NULL == lpText2)
		{
			nRet = 1;
			break;
		} 

		int nRet = _tcsicoll(lpText1, lpText2);
	} while (false);
	
	return (ST_UP == pInfo->stFlag ) ? nRet : -nRet;
}

bool SFileList::OnHeaderClick(EventHeaderClick* pEvt)
{
	//EventHeaderClick* pEvt = sobj_cast<EventHeaderClick>(e);
	if(NULL == pEvt) return false;

	//pEvt->iItem 只是当前的列，  hi.iOrder才是真正的列 的序号

	SHDITEM hi;
	hi.mask = SHDI_ORDER|SHDI_SORTFLAG;
	m_pHeader->GetItem(pEvt->iItem, &hi);
	if(ST_NULL == hi.stFlag)
	{
		////更新表头的排序状态
		//for(int i=0; i<m_pHeader->GetItemCount(); ++i)
		//{
		//	m_pHeader->SetItemSort(i, ST_NULL);
		//}
		hi.stFlag = ST_UP;
	}
	else
	{
		if(ST_DOWN == hi.stFlag)
			hi.stFlag = ST_UP;
		else if(ST_UP == hi.stFlag)
			hi.stFlag = ST_DOWN;
	}
	
	//m_pHeader->SetItemSort(pEvt->iItem, hi.stFlag);
	SetHeaderItemSort(pEvt->iItem, (ST_UP == hi.stFlag));

	EventFLColClick eEvt(this);
	eEvt.iCol = hi.iOrder;
	eEvt.bAscOrder = (ST_UP == hi.stFlag);
	FireEvent(eEvt);
	if(eEvt.handled > 0) //大于0 表示 有处理 
		return true;


	// 开始排序 
	_SORTINFO sortInfo = {hi.iOrder, hi.stFlag};
	qsort_s(m_arrItems.GetData(), m_arrItems.GetCount(), sizeof(FLItemData), SortCmp, &sortInfo);
	m_nSelectItem = -1;
	m_nHoverItem = -1;
	m_nClickItem = -1;
	Invalidate();
    return true;
}

bool SFileList::OnHeaderSizeChanging(EventHeaderItemChanging* pEvt)
{
    UpdateScrollBar();
    InvalidateRect(GetListRect());

    return true;
}

bool SFileList::OnHeaderSwap(EventHeaderItemSwap* pEvt)
{
    InvalidateRect(GetListRect());

    return true;
}

bool SFileList::GetCheckState(int nItem)
{
	if (nItem < 0 || nItem >= GetItemCount())
        return false;

	FLItemData& lvi = m_arrItems.GetAt(nItem);
	return lvi.bChecked;
}

bool SFileList::SetCheckState(int nItem, bool bCheck)
{
    if (nItem < 0 || nItem >= GetItemCount())
        return false;

	FLItemData& lvi = m_arrItems.GetAt(nItem);
	
	lvi.bChecked = bCheck;

	RedrawItem(nItem);
    return true;
}

int SFileList::GetCheckedItemCount()
{
    int ret = 0;

    for (int i = 0; i < GetItemCount(); i++)
    {
        if (m_arrItems.GetAt(i).bChecked)
            ret++;
    }

    return ret;
}

int SFileList::GetFirstCheckedItem()
{
    int ret = -1;
    for (int i = 0; i < GetItemCount(); i++)
    {
        if (m_arrItems.GetAt(i).bChecked) 
		{
            ret = i;
            break;
        }
    }

    return ret;
}

int SFileList::GetLastCheckedItem()
{
    int ret = -1;
    for (int i = GetItemCount() - 1; i >= 0; i--)
    {
		if(m_arrItems.GetAt(i).bChecked) 
		{
            ret = i;
            break;
        }
    }

    return ret;
}




}//end of namespace 
