#include "stdafx.h"
#include "SHeaderCtrl008.h"
#include "helper\DragWnd.h"

namespace SOUI
{

	//////////////////////////////////////////////////////////////////////////
	// SHeaderItem
	class SHeaderItem : public SWindow//, public SAnimator
	{
		SOUI_CLASS_NAME(SHeaderItem, L"headerItem")
		friend class SHeaderCtrl008;
	public:
		SHeaderItem(SHeaderCtrl008* pHost) :m_pHost(pHost), m_iOrder(-1), m_bcanSort(FALSE), m_pSkinSort(NULL), m_hDragImg(NULL), m_sortFlag(ST_NULL),m_bSwaping(false), m_bChangeSizing(false)
		{
			m_sortPos.x = m_sortPos.y = -100;
		}
// 		void MoveTo(const CRect & rcEnd)
// 		{
// 			m_rcBegin = GetWindowRect();
// 			m_rcEnd = rcEnd;
// 			if (m_rcBegin.EqualRect(m_rcEnd))
// 				return;
// 			Stop();
// 			Start(100);
// 		}
		BOOL IsDragable() { return m_iOrder != -1 && m_pHost->m_bItemSwapEnable; }

		SOUI_ATTRS_BEGIN()
			ATTR_BOOL(L"canSort", m_bcanSort, FALSE)
			ATTR_SKIN(L"sortSkin", m_pSkinSort, FALSE)
			ATTR_POINT(L"sortIconXY", m_sortPos, FALSE)
			ATTR_INT(L"sortIconX", m_sortPos.x, FALSE)
			ATTR_INT(L"sortIconY", m_sortPos.y, FALSE)
		SOUI_ATTRS_END()
		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_ACTIVATEAPP(OnActivateApp)
		SOUI_MSG_MAP_END()
		
	protected:
		virtual BOOL OnSetCursor(const CPoint &pt)override
		{
			if (m_pHost->m_bFixWidth || !HitTestSIZEWE(pt))
				return __super::OnSetCursor(pt);
			HCURSOR hCursor = GETRESPROVIDER->LoadCursor(IDC_SIZEWE);
			if (GetCursor() != hCursor)
				SetCursor(hCursor);
			return TRUE;
		}

		virtual BOOL CreateChildren(pugi::xml_node xmlNode)
		{
			m_pRootWnd = SApplication::getSingleton().CreateWindowByName(L"window");
			this->InsertChild(m_pRootWnd);
			m_pRootWnd->SetAttribute(L"size", L"-2,-2",TRUE);
			m_pRootWnd->SetAttribute(L"msgTransparent", L"1", TRUE);
			m_pRootWnd->SSendMessage(WM_CREATE);
			m_pRootWnd->CreateChildren(xmlNode);
			return TRUE;
		}
		void OnPaint(IRenderTarget *_pRT)
		{
			if (m_bSwaping)
				return;
			SWindow::OnPaint(_pRT);
			if (m_pSkinSort&&m_bcanSort)
			{
				CRect _sortIconRect;
				m_pSkinSort->Draw(_pRT, GetSortIconRect(_sortIconRect), m_sortFlag);
			}
		}
		const CRect &GetSortIconRect(CRect &_sortIconRect)
		{
			CRect _clientRect = GetClientRect();
			SIZE _skinSize = m_pSkinSort->GetSkinSize();
			//未设置X偏移
			if (m_sortPos.x == -100)
			{
				int _left = (_sortIconRect.right = _clientRect.right) - _skinSize.cx;
				_sortIconRect.left = _left < _clientRect.left ? _clientRect.left : _left;
			}
			else
			{
				int _right = (_sortIconRect.left = m_sortPos.x) + _skinSize.cx;
				_sortIconRect.right = _right > _clientRect.right ? _clientRect.right : _right;
			}
			//未设置Y偏移
			if (m_sortPos.y == -100)
			{
				int _top = _clientRect.top + (_clientRect.Height() - _skinSize.cy) / 2;
				_sortIconRect.top = _top < _clientRect.top ? _clientRect.top : _top;
				int _bottom = _sortIconRect.top + _skinSize.cy;
				_sortIconRect.bottom = _bottom > _clientRect.bottom ? _clientRect.bottom : _bottom;
			}
			else
			{
				int _bottom = (_sortIconRect.top = m_sortPos.y) + _skinSize.cx;
				_sortIconRect.bottom = _bottom > _clientRect.bottom ? _clientRect.bottom : _bottom;
			}
			return _sortIconRect;
		}
		

	protected:
		void OnActivateApp(BOOL bActive, DWORD dwThreadID)
		{
			if (m_bSwaping)
			{
				CDragWnd::EndDrag();
				::DeleteObject(m_hDragImg);
				m_hDragImg = NULL;
				m_bSwaping = false;
				//m_pHost->StopAni();
			}
		}
		bool HitTestSIZEWE(const CPoint & pt)
		{
			CRect    rcWnd;
			GetWindowRect(&rcWnd);
			if (!rcWnd.PtInRect(pt))
				return false;
			return pt.x > rcWnd.right - CX_HDITEM_MARGIN;
			
		}
		HBITMAP CreateDragImage()
		{
			CRect rcItem = GetWindowRect();
			CAutoRefPtr<IRenderTarget> pRT;
			GETRENDERFACTORY->CreateRenderTarget(&pRT, rcItem.Width(), rcItem.Height());
			BeforePaintEx(pRT);
			CPoint pt;
			pt -= rcItem.TopLeft();
			pRT->SetViewportOrg(pt);
			PaintBackground2(pRT, &rcItem);
			pRT->SetViewportOrg(CPoint());
			HBITMAP hBmp = CreateBitmap(rcItem.Width(), rcItem.Height(), 1, 32, NULL);
			HDC hdc = GetDC(NULL);
			HDC hMemDC = CreateCompatibleDC(hdc);
			::SelectObject(hMemDC, hBmp);
			HDC hdcSrc = pRT->GetDC(0);
			::BitBlt(hMemDC, 0, 0, rcItem.Width(), rcItem.Height(), hdcSrc, 0, 0, SRCCOPY);
			pRT->ReleaseDC(hdcSrc);			
			::DeleteDC(hMemDC);
			::ReleaseDC(NULL, hdc);
			return hBmp;
		}
// 		virtual void OnAnimatorState(int percent)
// 		{
// 			CRect rcTemp;
// 			rcTemp.left = m_rcBegin.left + (m_rcEnd.left - m_rcBegin.left)*percent / 100;
// 			rcTemp.top = m_rcBegin.top;
// 			rcTemp.right = m_rcBegin.right + (m_rcEnd.right - m_rcBegin.right)*percent / 100;
// 			rcTemp.bottom = m_rcBegin.bottom ;
// 			Move(rcTemp);
// 		}
		virtual void OnFinalRelease() { delete this; }
		void OnMouseMove(UINT nFlags, CPoint pt)
		{
			if ((nFlags & MK_LBUTTON))
			{
				if (!m_bChangeSizing)
				{
					if(HitTestSIZEWE(m_ptDrag))
						m_bChangeSizing = true;
				}
				else if (m_bChangeSizing)
				{
					m_pHost->ChangeItemSize(this,pt);
				}
				if (!m_bSwaping&&!m_bChangeSizing && IsDragable())
				{
					m_hDragImg = CreateDragImage();
					CPoint pt = m_ptDrag - GetWindowRect().TopLeft();
					CDragWnd::BeginDrag(m_hDragImg, pt, 0, 128, LWA_ALPHA | LWA_COLORKEY);
					m_bSwaping = true;
					m_pRootWnd->SetVisible(FALSE);
					this->Invalidate();
					//m_pHost->StartAni();
				}
				else if(m_bSwaping)
				{
					CPoint pt2(pt.x, m_ptDrag.y);
					m_pHost->ChangeItemPos(this, pt2);
					ClientToScreen(GetContainer()->GetHostHwnd(), &pt2);
					CDragWnd::DragMove(pt2);
				}
			}
		}
		void OnLButtonUp(UINT nFlags, CPoint pt)
		{
			SWindow::OnLButtonUp(nFlags, pt);
			if (m_bSwaping)
			{
				m_pHost->UpdateChildrenPosition();
				CDragWnd::EndDrag();
				DeleteObject(m_hDragImg);
				m_hDragImg = NULL;
				m_pRootWnd->SetVisible(TRUE);
				this->Invalidate();
				m_bSwaping = false;
				//m_pHost->StopAni();
			}
			else if (m_bChangeSizing)
			{
				m_bChangeSizing = false;
			}
		}
		void OnLButtonDown(UINT nFlags, CPoint pt)
		{
			SWindow::OnLButtonDown(nFlags, pt);
			m_ptDrag = pt;
			m_bSwaping = false, m_bChangeSizing=false;
		}
	private:
		CRect m_rcBegin, m_rcEnd;
		CPoint  m_ptDrag;
		int     m_iOrder;
		int     m_iTabIndex;
		bool    m_bSwaping,m_bChangeSizing;
		SHeaderCtrl008* m_pHost;		
		BOOL m_bcanSort;//是否可以排序
		POINT m_sortPos;//排序图标位置
		ISkinObj *    m_pSkinSort;  /**< 排序标志Skin */
		HBITMAP       m_hDragImg;  /**< 显示拖动窗口的临时位图 */
		SHDSORTFLAG m_sortFlag;
		SWindow *m_pRootWnd;
	};

	SHeaderCtrl008::SHeaderCtrl008(void)
		: m_bFixWidth(FALSE)
		, m_bItemSwapEnable(TRUE)
		, m_bSortHeader(TRUE)
		, m_pSkinItem(GETBUILTINSKIN(SKIN_SYS_HEADER))
		, m_pSkinSort(NULL)
		, m_dwHitTest((DWORD)-1)
		, m_bDragging(FALSE)
	{
		m_bClipClient = TRUE;
		m_evtSet.addEvent(EVENTID(EventHeaderClick));
		m_evtSet.addEvent(EVENTID(EventHeaderItemChanged));
		m_evtSet.addEvent(EVENTID(EventHeaderItemChanging));
		m_evtSet.addEvent(EVENTID(EventHeaderItemSwap));
	}

	SHeaderCtrl008::~SHeaderCtrl008(void)
	{
	}
	
	BOOL SHeaderCtrl008::GetItem(int iItem, SHDITEM008 *pItem)
	{
		if ((UINT)iItem >= m_arrItems.GetCount()) return FALSE;
		
		if (pItem->mask & SHDI_WIDTH) pItem->cx = m_arrItems[iItem]->GetWindowRect().Width();
		if (pItem->mask & SHDI_SORTFLAG) pItem->stFlag = m_arrItems[iItem]->m_sortFlag;
		if (pItem->mask & SHDI_ORDER) pItem->iOrder = m_arrItems[iItem]->m_iOrder;
		return TRUE;
	}

	BOOL SHeaderCtrl008::DeleteItem(int iItem)
	{
		if (iItem < 0 || (UINT)iItem >= m_arrItems.GetCount()) return FALSE;

		int iOrder = m_arrItems[iItem]->m_iOrder;
		m_arrItems.RemoveAt(iItem);
		DestroyChild(m_arrItems[iItem]);		
		//更新排序
		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
		{
			if (m_arrItems[i]->m_iOrder > iOrder)
				m_arrItems[i]->m_iOrder--;
		}
		UpdateChildrenPosition();
		return TRUE;
	}

	void SHeaderCtrl008::DeleteAllItems()
	{
		for(UINT iItem = 0; iItem< m_arrItems.GetCount(); iItem++)
			DestroyChild(m_arrItems[iItem]);
		m_arrItems.RemoveAll();
		UpdateChildrenPosition();
	}

// 	void SHeaderCtrl008::StartAni()
// 	{
// 		GetContainer()->RegisterTimelineHandler(this);
// 	}

// 	void SHeaderCtrl008::StopAni()
// 	{
// 		GetContainer()->UnregisterTimelineHandler(this);
// 	}

	void SHeaderCtrl008::OnDestroy()
	{
		DeleteAllItems();
		__super::OnDestroy();
	}
	
	bool SHeaderCtrl008::IsLastItem(int iOrder)
	{
		return iOrder == m_arrItems.GetCount()-1;
	}
	CSize SHeaderCtrl008::GetDesiredSize(LPCRECT pRcContainer)
	{
		CSize szRet=__super::GetDesiredSize(pRcContainer);
		szRet.cx = GetTotalWidth();
		return szRet;
	}
	void SHeaderCtrl008::UpdateChildrenPosition()
	{
		if (m_layoutDirty == dirty_self)
		{//当前窗口所有子窗口全部重新布局
			GetLayout()->LayoutChildren(this);
		}		
		CRect rcClient;
		GetClientRect(&rcClient);
		CRect rcTab = rcClient;
		int itemWid = 0;
		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
		{
			//itemWid = m_arrItems[i]->GetDesiredSize(-1, -1).cx;
			itemWid = m_arrItems[i]->GetWindowRect().Width();
			rcTab.right = rcTab.left +itemWid ;
			m_arrItems[i]->Move(rcTab);
			rcTab.OffsetRect(itemWid, 0);
		}

	}
	int SHeaderCtrl008::ChangeItemPos(SHeaderItem* pCurMove, CPoint ptCur)
	{
		int offset = 0;
		for (int i = 0; i < (int)m_arrItems.GetCount(); i++)
		{
			if (m_arrItems[i] == pCurMove)
			{
				continue;
			}			
			CRect rcWnd = m_arrItems[i]->GetWindowRect();
			CRect rcOffset = pCurMove->GetWindowRect();
			CPoint ptCenter = rcWnd.CenterPoint();
			if (ptCenter.x <= ptCur.x && rcWnd.right >= ptCur.x)
			{
				
				if (pCurMove->m_iOrder > m_arrItems[i]->m_iOrder)
				{
					rcWnd.OffsetRect(rcOffset.Width(), 0); offset += rcWnd.Width();
				}
				else
				{
					rcWnd.OffsetRect(-rcOffset.Width(), 0); offset -= rcWnd.Width();
				}
				int order = pCurMove->m_iOrder;
				pCurMove->m_iOrder = m_arrItems[i]->m_iOrder;
				m_arrItems[i]->m_iOrder = order;
				m_arrItems[i]->Move(rcWnd);
				SHeaderItem* pTemp = m_arrItems[i];
				m_arrItems[pCurMove->m_iOrder] = pCurMove;
				m_arrItems[pTemp->m_iOrder] = pTemp;
			}
		}
		if (offset)
		{
			CRect rcWnd = pCurMove->GetWindowRect();
			rcWnd.OffsetRect(-offset, 0);
			pCurMove->Move(rcWnd);
		}
		return 1;
	}

	void SHeaderCtrl008::ChangeItemSize(SHeaderItem *pHeaderItem, CPoint ptCur)
	{
		CRect itemRc = pHeaderItem->GetWindowRect();
		if (ptCur.x <= itemRc.left + 10)
			return;
		int offset = ptCur.x - itemRc.right;
		itemRc.right = ptCur.x;
		
		pHeaderItem->Move(itemRc);
		for (int i = pHeaderItem->m_iOrder+1; i < (int)m_arrItems.GetCount(); i++)
		{			
			CRect rcWnd = m_arrItems[i]->GetWindowRect();			
			rcWnd.OffsetRect(offset, 0);
			m_arrItems[i]->Move(rcWnd);
		}
		this->RequestRelayout();
	}

	BOOL SHeaderCtrl008::CreateChildren(pugi::xml_node xmlNode)
	{
		pugi::xml_node xmlItems = xmlNode.child(L"items");
		if (!xmlItems) return FALSE;
		pugi::xml_node xmlItem = xmlItems.child(L"item");

		this->SetAttribute(L"width", L"-1");
		
		int iOrder = 0;
		while (xmlItem)
		{
			SHeaderItem *item = new SHeaderItem(this);
			this->InsertChild(item);
			item->m_iOrder = iOrder++;
			//先从header里COPY一些通用属性，如果子项没有设置的话
			
			item->InitFromXml(xmlItem);
			m_arrItems.InsertAt(m_arrItems.GetCount(), item);
			xmlItem = xmlItem.next_sibling(L"item");
		}
		return TRUE;
	}


	int SHeaderCtrl008::GetTotalWidth()
	{
		int nRet = 0;
		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
		{
			nRet += m_arrItems[i]->m_bFloat? m_arrItems[i]->GetWindowRect().Width() :  m_arrItems[i]->GetDesiredSize(-1, -1).cx;
			//nRet += m_arrItems[i]->GetWindowRect().Width();
		}
		return nRet;
	}

	void SHeaderCtrl008::SetItemSort(int iItem, SHDSORTFLAG stFlag)
	{
		SASSERT(iItem >= 0 && iItem < (int)m_arrItems.GetCount());
		if (stFlag != m_arrItems[iItem]->m_sortFlag)
		{
			m_arrItems[iItem]->m_sortFlag = stFlag;
			m_arrItems[iItem]->Invalidate();
		}
	}

	void SHeaderCtrl008::SetItemVisible(int iItem, bool visible)
	{
		SASSERT(iItem >= 0 && iItem < (int)m_arrItems.GetCount());
		m_arrItems[iItem]->SetVisible(visible, TRUE);

		Invalidate();
		//发出调节宽度消息
		EventHeaderItemChanged evt(this);
		evt.iItem = iItem;
		evt.nWidth = m_arrItems[iItem]->GetWindowRect().Width();
		FireEvent(evt);
	}

	bool SHeaderCtrl008::IsItemVisible(int iItem) const
	{
		SASSERT(iItem >= 0 && iItem < (int)m_arrItems.GetCount());
		return m_arrItems[iItem]->IsVisible();
	}


// 	void SHeaderCtrl008::OnNextFrame()
// 	{
// 		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
// 		{
// 			m_arrItems[i]->Update();
// 		}
// 	}

}