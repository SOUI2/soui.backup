#include "souistd.h"
#include "control\SHeaderCtrl.h"
#include "helper\DragWnd.h"

namespace SOUI
{

	//////////////////////////////////////////////////////////////////////////
	// SHeaderItem
	class SHeaderItem : public SWindow
	{
		SOUI_CLASS_NAME(SHeaderItem, L"headerItem")
			friend class SHeaderCtrl;
	public:

	protected:
		SHeaderItem(SHeaderCtrl* pHost) :m_pHost(pHost), m_iIdx(-1), m_bcanSort(FALSE), m_pSkinSort(NULL), m_hDragImg(NULL), m_sortFlag(ST_NULL), m_bSwaping(false), m_bChangeSizing(false)
		{
			m_sortPos.x = m_sortPos.y = -100;
			SWindow::m_bClipClient = TRUE;
		}
		BOOL IsDragable() { return m_iIdx != -1 && m_pHost->m_bItemSwapEnable; }
		
		virtual BOOL OnSetCursor(const CPoint &pt)override
		{
			if (!HitTestSIZEWE(pt))
				return __super::OnSetCursor(pt);
			HCURSOR hCursor = GETRESPROVIDER->LoadCursor(IDC_SIZEWE);
			if (GetCursor() != hCursor)
				SetCursor(hCursor);
			return TRUE;
		}

		void HideChildWnd(bool bHide)
		{
			SWindow *childWnd = GetWindow(GSW_FIRSTCHILD);
			while (childWnd)
			{
				childWnd->SetVisible(bHide ? FALSE : TRUE);
				childWnd = childWnd->GetWindow(GSW_NEXTSIBLING);
			}
			this->Invalidate();
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
				int _left = (_sortIconRect.right = _clientRect.right - CX_HDITEM_MARGIN) - _skinSize.cx;
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

		void OnActivateApp(BOOL bActive, DWORD dwThreadID)
		{
			if (m_bSwaping)
			{
				CDragWnd::EndDrag();
				::DeleteObject(m_hDragImg);
				m_hDragImg = NULL;
				m_bSwaping = false;
				HideChildWnd(false);
			}
		}
		int HitTestSIZEWE(const CPoint & pt)
		{
			if (m_pHost->m_bFixWidth||m_pHost->m_bRatable)
				return 0;
			CRect    rcWnd;
			GetWindowRect(&rcWnd);
			if (!rcWnd.PtInRect(pt))
				return 0;
			bool bLeft = pt.x < rcWnd.left + CX_HDITEM_MARGIN;
			if (bLeft)
				return m_iIdx != 0 ? 1 : 0;
			return (pt.x > rcWnd.right - CX_HDITEM_MARGIN) ? 2 : 0;

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
		void OnMouseMove(UINT nFlags, CPoint pt)
		{
			static SHeaderItem *item = NULL;
			if ((nFlags & MK_LBUTTON))
			{
				if (!m_bChangeSizing)
				{
					switch (HitTestSIZEWE(m_ptDrag))
					{
					case 1:
						item = m_pHost->GetPrvItem(m_iIdx);
						m_bChangeSizing = true; break;
					case 2:
						item = this;
						m_bChangeSizing = true; break;
					}
				}
				else if (m_bChangeSizing)
				{
					SASSERT(item);
					m_pHost->ChangeItemSize(item, pt);
				}
				if (!m_bSwaping && !m_bChangeSizing && IsDragable())
				{
					m_hDragImg = CreateDragImage();
					CPoint pt = m_ptDrag - GetWindowRect().TopLeft();
					CDragWnd::BeginDrag(m_hDragImg, pt, 0, 128, LWA_ALPHA | LWA_COLORKEY);
					m_bSwaping = true;
					HideChildWnd(true);
					this->Invalidate();
				}
				else if (m_bSwaping)
				{
					CPoint pt2(pt.x, m_ptDrag.y);
					m_pHost->ChangeItemPos(this, pt2);
					ClientToScreen(GetContainer()->GetHostHwnd(), &pt2);
					CDragWnd::DragMove(pt2);
				}
			}
		}

		void OnSuperLButtonUp(UINT nFlags, CPoint pt)
		{
			ReleaseCapture();
			if (!(GetState()&WndState_PushDown)) return;
			ModifyState(0, WndState_PushDown, TRUE);
			if (!GetWindowRect().PtInRect(pt)) return;

			EventLButtonUp evtLButtonUp(this);
			evtLButtonUp.pt = pt;
			FireEvent(evtLButtonUp);
			if (m_bSwaping || m_bChangeSizing)
				return;
			//必须有可排序标志才发出CMD
			if ((GetID() || GetName()) && m_bcanSort)
			{
				FireCommand();
			}
		}

		void OnLButtonUp(UINT nFlags, CPoint pt)
		{
			OnSuperLButtonUp(nFlags, pt);
			if (m_bSwaping)
			{
				m_pHost->UpdateChildrenPosition();
				CDragWnd::EndDrag();
				DeleteObject(m_hDragImg);
				m_hDragImg = NULL;
				HideChildWnd(false);
				this->Invalidate();
				m_bSwaping = false;
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
			m_bSwaping = false, m_bChangeSizing = false;
		}

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
	private:
		CRect m_rcBegin, m_rcEnd;
		CPoint  m_ptDrag;
		int     m_iIdx;
		int		m_iOrder;
		bool    m_bSwaping, m_bChangeSizing;
		SHeaderCtrl* m_pHost;
		BOOL m_bcanSort;//是否可以排序
		POINT m_sortPos;//排序图标位置
		ISkinObj *    m_pSkinSort;  /**< 排序标志Skin */
		HBITMAP       m_hDragImg;  /**< 显示拖动窗口的临时位图 */
		SHDSORTFLAG m_sortFlag;
	};

	SHeaderCtrl::SHeaderCtrl(void)
		: m_bFixWidth(FALSE)
		, m_bItemSwapEnable(TRUE)
		, m_bSortHeader(TRUE)
		, m_bRatable(FALSE)
	{
		m_bClipClient = TRUE;
		m_evtSet.addEvent(EVENTID(EventHeaderClick));
		m_evtSet.addEvent(EVENTID(EventHeaderItemChanged));
		m_evtSet.addEvent(EVENTID(EventHeaderItemChanging));
		m_evtSet.addEvent(EVENTID(EventHeaderItemSwap));
	}

	SHeaderCtrl::~SHeaderCtrl(void)
	{
	}

	int SHeaderCtrl::InsertItem(int iItem, LPCTSTR pszText, int nWidth, SHDSORTFLAG stFlag, LPARAM lParam)
	{
		SHeaderItem *item = new SHeaderItem(this);
		this->InsertChild(item);
		item->m_iIdx = iItem;

		m_arrItems.InsertAt(iItem, item);
		for (size_t i = iItem; i < GetItemCount(); i++)
		{
			m_arrItems[i]->m_iIdx++;
		}
		return iItem;
	}

	int SHeaderCtrl::GetItemWidth(int iItem)
	{
		if (iItem < 0 || (UINT)iItem >= m_arrItems.GetCount()) return -1;
		if (!m_arrItems[iItem]->IsVisible()) return 0;
		return m_arrItems[iItem]->GetWindowRect().Width();
	}

	BOOL SHeaderCtrl::GetItem(int iItem, SHDITEM *pItem)
	{
		if ((UINT)iItem >= m_arrItems.GetCount()) return FALSE;

		if (pItem->mask & SHDI_WIDTH) pItem->cx = m_arrItems[iItem]->GetWindowRect().Width();
		if (pItem->mask & SHDI_SORTFLAG) pItem->stFlag = m_arrItems[iItem]->m_sortFlag;
		if (pItem->mask & SHDI_ORDER) pItem->iOrder = m_arrItems[iItem]->m_iOrder;
		return TRUE;
	}

	BOOL SHeaderCtrl::DeleteItem(int iItem)
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

	void SHeaderCtrl::DeleteAllItems()
	{
		for (UINT iItem = 0; iItem < m_arrItems.GetCount(); iItem++)
			DestroyChild(m_arrItems[iItem]);
		m_arrItems.RemoveAll();
		UpdateChildrenPosition();
	}

	bool SHeaderCtrl::IsLastItem(int iIdx)
	{
		return iIdx == m_arrItems.GetCount() - 1;
	}
	SHeaderItem *SHeaderCtrl::GetPrvItem(int iIdx)
	{
		if (iIdx >= m_arrItems.GetCount() || iIdx == 0)
			return NULL;
		return m_arrItems[--iIdx];
	}
	CSize SHeaderCtrl::GetDesiredSize(LPCRECT pRcContainer)
	{
		CSize szRet = __super::GetDesiredSize(pRcContainer);
		if (m_bRatable)
			return szRet;
		szRet.cx = GetTotalWidth();
		return szRet;
	}
	void SHeaderCtrl::UpdateChildrenPosition()
	{
		if (m_layoutDirty == dirty_self)
		{//当前窗口所有子窗口全部重新布局
			if (!m_bRatable)
			{
				GetLayout()->LayoutChildren(this);
			}
			else//按比例分割
			{				
				SASSERT(!(GetLayoutParam()->IsWrapContent(Horz)));
				SWindow *pChild = GetWindow(GSW_FIRSTCHILD);;
				int totalWid = 0;
				while (pChild)
				{
					totalWid+=pChild->GetLayoutParam()->GetSpecifiedSize(Horz).fSize;
					pChild =pChild->GetWindow(GSW_NEXTSIBLING);
				}
				pChild = GetWindow(GSW_FIRSTCHILD);;
				CRect rcWnd = GetClientRect();
				int headerWid = rcWnd.Width();
				int reltotalWid = 0;
				rcWnd.MoveToXY(0, 0);
				while (pChild)
				{
					rcWnd.right= pChild->GetLayoutParam()->GetSpecifiedSize(Horz).fSize*headerWid/totalWid ;
					reltotalWid += rcWnd.right;
					pChild->Move(rcWnd);
					pChild = pChild->GetWindow(GSW_NEXTSIBLING);
				}
				reltotalWid -= headerWid;
				if (reltotalWid > 0)
				{
					rcWnd.right -= reltotalWid;
					pChild->Move(rcWnd);
				}
			}
		}
		CRect rcClient;
		GetClientRect(&rcClient);
		CRect rcTab = rcClient;
		int itemWid = 0;
		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
		{
			itemWid = m_arrItems[i]->GetWindowRect().Width();
			rcTab.right = rcTab.left + itemWid;
			m_arrItems[i]->Move(rcTab);
			rcTab.OffsetRect(itemWid, 0);
		}
	}
	int SHeaderCtrl::ChangeItemPos(SHeaderItem* pCurMove, CPoint ptCur)
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
				if (pCurMove->m_iIdx > m_arrItems[i]->m_iIdx)
				{
					rcWnd.OffsetRect(rcOffset.Width(), 0); offset += rcWnd.Width();
				}
				else
				{
					rcWnd.OffsetRect(-rcOffset.Width(), 0); offset -= rcWnd.Width();
				}
				int order = pCurMove->m_iIdx;
				pCurMove->m_iIdx = m_arrItems[i]->m_iIdx;
				m_arrItems[i]->m_iIdx = order;
				m_arrItems[i]->Move(rcWnd);
				SHeaderItem* pTemp = m_arrItems[i];
				m_arrItems[pCurMove->m_iIdx] = pCurMove;
				m_arrItems[pTemp->m_iIdx] = pTemp;

				EventHeaderItemSwap evt(this);
				evt.iOldIndex = order;
				evt.iNewIndex = pCurMove->m_iIdx;
				FireEvent(evt);
			}
		}		
		if (offset)
		{
			CRect rcWnd = pCurMove->GetWindowRect();
			rcWnd.OffsetRect(-offset, 0);
			pCurMove->Move(rcWnd);
		}
		UpdateChildrenPosition();//鼠标快速移动时不调用此可能会发生错乱的问题。
		return 1;
	}

	void SHeaderCtrl::ChangeItemSize(SHeaderItem *pHeaderItem, CPoint ptCur)
	{
		CRect itemRc = pHeaderItem->GetWindowRect();
		if (ptCur.x <= itemRc.left)
			return;
		int offset = ptCur.x - itemRc.right;
		itemRc.right = ptCur.x;

		pHeaderItem->Move(itemRc);
		EventHeaderItemChanging evt(this);
		evt.iItem = pHeaderItem->m_iOrder;
		evt.nWidth = itemRc.Width();
		FireEvent(evt);
		for (int i = pHeaderItem->m_iIdx + 1; i < (int)m_arrItems.GetCount(); i++)
		{
			CRect rcWnd = m_arrItems[i]->GetWindowRect();
			rcWnd.OffsetRect(offset, 0);
			m_arrItems[i]->Move(rcWnd);
		}
		this->RequestRelayout();
	}

	BOOL SHeaderCtrl::CreateChildren(pugi::xml_node xmlNode)
	{
		pugi::xml_node xmlItems = xmlNode.child(L"items");
		if (!xmlItems) return FALSE;
		pugi::xml_node xmlItem = xmlItems.child(L"item");
		
		int iOrder = 0;
		while (xmlItem)
		{
			SHeaderItem *item = new SHeaderItem(this);
			this->InsertChild(item);
			item->m_iIdx = item->m_iOrder = iOrder++;
			//先从header里COPY一些通用属性，如果子项没有设置的话
			if (xmlItem.attribute(L"sortSkin").empty() && !xmlNode.attribute(L"sortSkin").empty())
				xmlItem.append_attribute(L"sortSkin").set_value(xmlNode.attribute(L"sortSkin").as_string());
			//没有设置背景色，也没有设置skin,也没设置class。则使用headerctrl的设置，如果headerctrl没有itemSkin属性则使用系统内建皮肤
			if (xmlItem.attribute(L"skin").empty() && xmlItem.attribute(L"colorBkgnd").empty() && xmlItem.attribute(L"class").empty())
			{
				if (!xmlNode.attribute(L"itemSkin").empty())
					xmlItem.append_attribute(L"skin").set_value(xmlNode.attribute(L"itemSkin").as_string());
				else xmlItem.append_attribute(L"skin").set_value(L"_skin.sys.header");
			}
			//如果HeaderCtrl设置了sortHeader  子项又没有指定canSort属性，则使用父项的设置
			if (xmlItem.attribute(L"canSort").empty() && !xmlNode.attribute(L"sortHeader").empty())
				xmlItem.append_attribute(L"canSort").set_value(xmlNode.attribute(L"sortHeader").as_string());

			item->InitFromXml(xmlItem);
			m_arrItems.InsertAt(m_arrItems.GetCount(), item);

			item->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&SHeaderCtrl::ClickHeader, this));
			xmlItem = xmlItem.next_sibling(L"item");
		}
		return TRUE;
	}
	bool SHeaderCtrl::ClickHeader(EventArgs *pEvArg)
	{
		SHeaderItem *pHeaderItem = sobj_cast<SHeaderItem>(pEvArg->sender);
		SASSERT(pHeaderItem);
		EventHeaderClick evt(this);
		evt.iItem = pHeaderItem->m_iIdx;
		FireEvent(evt);
		return true;
	}
	int SHeaderCtrl::GetTotalWidth()
	{
		int nRet = 0;
		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
		{
			if (m_arrItems[i]->IsVisible())
				nRet += m_arrItems[i]->m_bFloat ? m_arrItems[i]->GetWindowRect().Width() : m_arrItems[i]->GetDesiredSize(-1, -1).cx;
		}
		return nRet;
	}

	void SHeaderCtrl::SetItemSort(int iItem, SHDSORTFLAG stFlag)
	{
		SASSERT(iItem >= 0 && iItem < (int)m_arrItems.GetCount());
		if (stFlag != m_arrItems[iItem]->m_sortFlag)
		{
			m_arrItems[iItem]->m_sortFlag = stFlag;
			m_arrItems[iItem]->Invalidate();
		}
	}

	void SHeaderCtrl::SetItemVisible(int iItem, bool visible)
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

	bool SHeaderCtrl::IsItemVisible(int iItem) const
	{
		SASSERT(iItem >= 0 && iItem < (int)m_arrItems.GetCount());
		return m_arrItems[iItem]->IsVisible();
	}
}