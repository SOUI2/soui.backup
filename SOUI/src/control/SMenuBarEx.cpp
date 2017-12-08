#include "souistd.h"
#include "control/SMenuBarEx.h"

#define TIMER_POP	10


namespace SOUI
{
	HHOOK SMenuBarEx::m_hMsgHook = NULL;
	SMenuBarEx *SMenuBarEx::m_pMenuBar = NULL;

	static const wchar_t * KItemStyle = L"itemStyle";

	class SMenuBarExItem :
		public SButton
	{
		SOUI_CLASS_NAME(SMenuBarExItem, L"item")
			friend class SMenuBarEx;
	public:
		SMenuBarExItem(SMenuBarEx *pHostMenu);
		~SMenuBarExItem();

		void SetData(ULONG_PTR data) { m_data = data; }
		ULONG_PTR GetData() { return m_data; }

		bool IsMenuLoaded() const;
	protected:
		UINT PopupMenu();

		HRESULT OnAttrSrc(const SStringW & strValue, BOOL bLoading);

		virtual void OnFinalRelease() { delete this; }
		virtual CSize GetDesiredSize(LPCRECT pRcContainer);
		bool OnCmd(EventArgs *e);

		void OnTimer(UINT_PTR timerID);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_TIMER(OnTimer)
			SOUI_MSG_MAP_END()

			SOUI_ATTRS_BEGIN()
			ATTR_CUSTOM(_T("src"), OnAttrSrc)
			SOUI_ATTRS_END()

			ULONG_PTR m_data;
		SMenuBarEx* m_pHostMenu;
		BOOL m_bIsRegHotKey;
		int m_iIndex;
		TCHAR m_cAccessKey;
		SMenuEx m_MenuEx;
	};

	SMenuBarExItem::SMenuBarExItem(SMenuBarEx *pHostMenu) :
		m_data(0),
		m_pHostMenu(pHostMenu),
		m_bIsRegHotKey(FALSE),
		m_iIndex(-1),
		m_cAccessKey(0)
	{
		m_bDrawFocusRect = FALSE;
		GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&SMenuBarExItem::OnCmd, this));
	}

	SMenuBarExItem::~SMenuBarExItem()
	{
	}

	bool SMenuBarExItem::IsMenuLoaded() const
	{
		return true;
	}

	UINT SMenuBarExItem::PopupMenu()
	{
		m_pHostMenu->m_bIsShow = TRUE;
		m_pHostMenu->m_pNowMenu = this;
		m_pHostMenu->m_iNowMenu = m_iIndex;

		SetCheck(TRUE);

		HWND hHost = m_pHostMenu->GetContainer()->GetHostHwnd();
		CRect rcHost;
		::GetWindowRect(hHost, rcHost);
		CRect rcMenu = GetClientRect();

		if (SMenuBarEx::m_hMsgHook == NULL)
			SMenuBarEx::m_hMsgHook = ::SetWindowsHookEx(WH_MSGFILTER,
				SMenuBarEx::MenuInputFilter, NULL, GetCurrentThreadId());// m_bLoop may become TRUE

		int iRet = 0;
		iRet = m_MenuEx.TrackPopupMenu(TPM_RETURNCMD,
			rcHost.left + rcMenu.left, rcHost.top + rcMenu.bottom + 2, hHost);

		SetCheck(FALSE);
		m_pHostMenu->m_bIsShow = FALSE;
		if (m_pHostMenu->m_pNowMenu != this || iRet == 0)
		{
			m_pHostMenu->m_pNowMenu = NULL;
			m_pHostMenu->m_iNowMenu = -1;
			return iRet;
		}
		m_pHostMenu->m_iNowMenu = -1;
		m_pHostMenu->m_pNowMenu = NULL;

		// uninstall hook
		::UnhookWindowsHookEx(SMenuBarEx::m_hMsgHook);
		SMenuBarEx::m_hMsgHook = NULL;

		// 把选择事件发送过去
		EventSelectMenu evt(m_pHostMenu);
		evt.m_id = iRet;
		evt.m_pMenu = this;
		FireEvent(evt);

		return iRet;
	}

	HRESULT SMenuBarExItem::OnAttrSrc(const SStringW & strValue, BOOL bLoading)
	{
		return m_MenuEx.LoadMenu(strValue) ? S_OK : E_INVALIDARG;
	}

	CSize SMenuBarExItem::GetDesiredSize(LPCRECT pRcContainer)
	{
		return SWindow::GetDesiredSize(pRcContainer);
	}

	bool SMenuBarExItem::OnCmd(EventArgs * e)
	{
		e->bubbleUp = false;
		PopupMenu();
		return true;
	}

	void SMenuBarExItem::OnTimer(UINT_PTR timerID)
	{
		if (timerID == TIMER_POP)
		{
			if (!m_pHostMenu->m_bIsShow)
			{
				PopupMenu();
				KillTimer(timerID);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	SMenuBarEx::SMenuBarEx() :
		m_bIsShow(FALSE),
		m_pNowMenu(NULL),
		m_iNowMenu(-1)
	{
		m_evtSet.addEvent(EVENTID(EventSelectMenu));
		SMenuBarEx::m_pMenuBar = this;
	}

	SMenuBarEx::~SMenuBarEx()
	{
		if (SMenuBarEx::m_hMsgHook)
		{
			::UnhookWindowsHookEx(SMenuBarEx::m_hMsgHook);
			SMenuBarEx::m_hMsgHook = NULL;
		}
	}
	

	BOOL SMenuBarEx::Insert(LPCTSTR pszTitle, LPCTSTR pszResName, int iPos)
	{
		pugi::xml_document xmlDoc;
		xmlDoc.root().set_value(SMenuBarExItem::GetClassName());
		xmlDoc.root().append_attribute(L"text").set_value(S_CT2W(pszTitle));
		xmlDoc.root().append_attribute(L"src").set_value(S_CT2W(pszResName));
		return Insert(xmlDoc.root(),iPos);
	}

	BOOL SMenuBarEx::Insert(pugi::xml_node xmlNode, int iPos)
	{
		SMenuBarExItem *pNewMenu = new SMenuBarExItem(this);
		SASSERT(pNewMenu);
		InsertChild(pNewMenu);

		pNewMenu->InitFromXml(m_itemStyle.child(KItemStyle));
		pNewMenu->InitFromXml(xmlNode);

		if (!pNewMenu->IsMenuLoaded())
		{
			DestroyChild(pNewMenu);
			return FALSE;
		}

		SStringW strText = S_CT2W(pNewMenu->GetWindowText());

		int nPos = strText.ReverseFind(L'&');
		if (nPos > -1)
			pNewMenu->SetAttribute(L"accel", SStringW().Format(L"alt+%c", strText[nPos + 1]));

		if (iPos < 0) iPos = m_lstMenuItem.GetCount();
		m_lstMenuItem.InsertAt(iPos, pNewMenu);

		pNewMenu->m_iIndex = iPos;
		for (size_t i = iPos + 1; i < m_lstMenuItem.GetCount(); i++)
		{
			m_lstMenuItem[i]->m_iIndex++;
		}
		return TRUE;
	}
	SMenuEx * SMenuBarEx::GetMenu(DWORD dwPos)
	{
		if (dwPos >= m_lstMenuItem.GetCount())
			return NULL;
		return &m_lstMenuItem[dwPos]->m_MenuEx;
	}
	int SMenuBarEx::HitTest(CPoint pt)
	{
		for (size_t i = 0; i < m_lstMenuItem.GetCount(); i++)
		{
			SMenuBarExItem *pItem = m_lstMenuItem[i];
			CRect rcItem = pItem->GetClientRect();
			if (rcItem.PtInRect(pt))
				return i;
		}
		return -1;
	}
	SMenuBarExItem * SMenuBarEx::GetMenuItem(DWORD dwPos)
	{
		if (dwPos >= m_lstMenuItem.GetCount())
			return NULL;
		return m_lstMenuItem[dwPos];
	}

	BOOL SMenuBarEx::CreateChildren(pugi::xml_node xmlNode)
	{
		pugi::xml_node itemStyle = xmlNode.child(KItemStyle);
		if (itemStyle)
		{
			m_itemStyle.append_copy(itemStyle);
		}

		for (pugi::xml_node xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
		{
			if (_tcsicmp(xmlChild.name(), SMenuBarExItem::GetClassName()) != 0)
				continue;
			Insert(xmlChild);
		}

		return TRUE;
	}

	LRESULT SMenuBarEx::MenuInputFilter(int code, WPARAM wParam, LPARAM lParam)
	{
		if (code == MSGF_MENU)
		{
			MSG msg = *(MSG*)lParam;
			int nMsg = msg.message;
			switch (nMsg)
			{
			case WM_MOUSEMOVE:
			{
				CPoint pt = msg.lParam;
				if (SMenuBarEx::m_pMenuBar->m_ptMouse != pt &&
					SMenuBarEx::m_pMenuBar->m_iNowMenu != -1)
				{
					HWND hHost = SMenuBarEx::m_pMenuBar->GetContainer()->GetHostHwnd();
					SMenuBarEx::m_pMenuBar->m_ptMouse = pt;
					::ScreenToClient(hHost, &pt);
					int nIndex = SMenuBarEx::m_pMenuBar->HitTest(pt);
					if (nIndex != -1)
					{
						SMenuBarExItem *menuItem = SMenuBarEx::m_pMenuBar->GetMenuItem(nIndex);
						if (menuItem && SMenuBarEx::m_pMenuBar->m_iNowMenu != nIndex)
						{
							SMenuBarEx::m_pMenuBar->m_pNowMenu = menuItem;
							SMenuBarEx::m_pMenuBar->m_iNowMenu = nIndex;
							::PostMessage(msg.hwnd, WM_CANCELMODE, 0, 0);
							menuItem->SetTimer(TIMER_POP, 10);
							return TRUE;
						}
					}
				}
				break;
			}
			case WM_KEYDOWN:
			{
				TCHAR vKey = msg.wParam;
				if (SMenuBarEx::m_pMenuBar->m_iNowMenu == -1)
					return TRUE;
				if (vKey == VK_LEFT)
				{
					int nRevIndex = SMenuBarEx::m_pMenuBar->m_iNowMenu - 1;
					if (nRevIndex < 0) nRevIndex = SMenuBarEx::m_pMenuBar->m_lstMenuItem.GetCount() - 1;
					SMenuBarExItem *menuItem = SMenuBarEx::m_pMenuBar->m_lstMenuItem[nRevIndex];
					if (menuItem)
					{
						HWND hHost = SMenuBarEx::m_pMenuBar->GetContainer()->GetHostHwnd();

						SMenuBarEx::m_pMenuBar->m_pNowMenu = menuItem;
						SMenuBarEx::m_pMenuBar->m_iNowMenu = nRevIndex;
						::PostMessage(hHost, WM_KEYDOWN, VK_ESCAPE, 0);
						menuItem->SetTimer(TIMER_POP, 10);
						return TRUE;
					}
				}
				else if (vKey == VK_RIGHT)
				{
					int nNextIndex = SMenuBarEx::m_pMenuBar->m_iNowMenu + 1;
					if (nNextIndex >= (int)SMenuBarEx::m_pMenuBar->m_lstMenuItem.GetCount()) nNextIndex = 0;
					SMenuBarExItem *menuItem = SMenuBarEx::m_pMenuBar->GetMenuItem(nNextIndex);
					if (menuItem)
					{
						HWND hHost = SMenuBarEx::m_pMenuBar->GetContainer()->GetHostHwnd();

						SMenuBarEx::m_pMenuBar->m_pNowMenu = menuItem;
						SMenuBarEx::m_pMenuBar->m_iNowMenu = nNextIndex;
						::PostMessage(hHost, WM_KEYDOWN, VK_ESCAPE, 0);
						menuItem->SetTimer(TIMER_POP, 10);
						return TRUE;
					}
				}
			}
			}
		}
		return CallNextHookEx(m_hMsgHook, code, wParam, lParam);
	}
}