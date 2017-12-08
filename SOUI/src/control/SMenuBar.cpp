#include "souistd.h"
#include "control/SMenuBar.h"

#define TIMER_POP	10


namespace SOUI
{
	HHOOK SMenuBar::m_hMsgHook = NULL;
	SMenuBar *SMenuBar::m_pMenuBar = NULL;

	static const wchar_t* KItemStyle = L"itemStyle";

	class SMenuBarItem :
		public SButton,
		public SMenu
	{
		SOUI_CLASS_NAME(SMenuBarItem, L"item")
			friend class SMenuBar;
	public:
		SMenuBarItem(SMenuBar *pHostMenu);
		~SMenuBarItem();

		void SetData(ULONG_PTR data) { m_data = data; }
		ULONG_PTR GetData() { return m_data; }

		bool IsMenuLoaded() const;
	protected:
		UINT PopMenu();

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
		SMenuBar* m_pHostMenu;
		BOOL m_bIsRegHotKey;
		int m_iIndex;
		TCHAR m_cAccessKey;
	};

	SMenuBarItem::SMenuBarItem(SMenuBar *pHostMenu) :
		m_data(0),
		m_pHostMenu(pHostMenu),
		m_bIsRegHotKey(FALSE),
		m_iIndex(-1),
		m_cAccessKey(0)
	{
		m_bDrawFocusRect = FALSE;
		GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&SMenuBarItem::OnCmd, this));
	}

	SMenuBarItem::~SMenuBarItem()
	{
	}

	bool SMenuBarItem::IsMenuLoaded() const
	{
		return true;
	}

	UINT SMenuBarItem::PopMenu()
	{
		if (m_pHostMenu->m_pNowMenu != NULL)
			return 0;
		m_pHostMenu->m_bIsShow = TRUE;
		m_pHostMenu->m_pNowMenu = this;
		m_pHostMenu->m_iNowMenu = m_iIndex;

		SetCheck(TRUE);

		HWND hHostWnd = GetContainer()->GetHostHwnd();
		CRect rcHost;
		::GetWindowRect(hHostWnd, rcHost);
		CRect rcMenu = GetClientRect();

		if (SMenuBar::m_hMsgHook == NULL)
			SMenuBar::m_hMsgHook = ::SetWindowsHookEx(WH_MSGFILTER,
				SMenuBar::MenuSwitch, NULL, GetCurrentThreadId());// m_bLoop may become TRUE

		int iRet = 0;
		iRet = TrackPopupMenu(TPM_RETURNCMD,
			rcHost.left + rcMenu.left, rcHost.top + rcMenu.bottom + 2, hHostWnd);

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
		::UnhookWindowsHookEx(SMenuBar::m_hMsgHook);
		SMenuBar::m_hMsgHook = NULL;

		// 把选择事件发送过去
		EventSelectMenu evt(m_pHostMenu);
		evt.m_id = iRet;
		evt.m_pMenu = this;
		FireEvent(evt);

		return iRet;
	}

	HRESULT SMenuBarItem::OnAttrSrc(const SStringW & strValue, BOOL bLoading)
	{
		return LoadMenu(strValue) ? S_OK : E_INVALIDARG;
	}

	CSize SMenuBarItem::GetDesiredSize(LPCRECT pRcContainer)
	{
		CSize size = SWindow::GetDesiredSize(pRcContainer);
		size.cx += 13;
		size.cy += 3;
		return size;
	}

	bool SMenuBarItem::OnCmd(EventArgs * e)
	{
		e->bubbleUp = false;
		PopMenu();
		return true;
	}

	void SMenuBarItem::OnTimer(UINT_PTR timerID)
	{
		if (timerID == TIMER_POP)
		{
			if (!m_pHostMenu->m_bIsShow)
			{
				PopMenu();
				KillTimer(timerID);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	SMenuBar::SMenuBar() :
		m_bIsShow(FALSE),
		m_pNowMenu(NULL),
		m_iNowMenu(-1)
	{
		m_evtSet.addEvent(EVENTID(EventSelectMenu));
		SMenuBar::m_pMenuBar = this;
	}

	SMenuBar::~SMenuBar()
	{
		if (SMenuBar::m_hMsgHook)
		{
			::UnhookWindowsHookEx(SMenuBar::m_hMsgHook);
			SMenuBar::m_hMsgHook = NULL;
		}
	}

	BOOL SMenuBar::Insert(LPCTSTR pszTitle, LPCTSTR pszResName, int iPos)
	{
		pugi::xml_document xmlDoc;
		xmlDoc.root().set_value(SMenuBarItem::GetClassName());
		xmlDoc.root().append_attribute(L"text").set_value(S_CT2W(pszTitle));
		xmlDoc.root().append_attribute(L"src").set_value(S_CT2W(pszResName));
		return Insert(xmlDoc.root(),iPos);
	}

	BOOL SMenuBar::Insert(pugi::xml_node xmlNode, int iPos)
	{
		SMenuBarItem *pNewMenu = new SMenuBarItem(this);
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

	SMenu * SMenuBar::GetMenu(DWORD dwPos)
	{
		if (dwPos >= m_lstMenuItem.GetCount())
			return NULL;
		return m_lstMenuItem[dwPos];
	}
	int SMenuBar::HitTest(CPoint pt)
	{
		for (size_t i = 0; i < m_lstMenuItem.GetCount(); i++)
		{
			SMenuBarItem *pItem = m_lstMenuItem[i];
			CRect rcItem = pItem->GetClientRect();
			if (rcItem.PtInRect(pt))
				return i;
		}
		return -1;
	}
	SMenuBarItem * SMenuBar::GetMenuItem(DWORD dwPos)
	{
		if (dwPos >= m_lstMenuItem.GetCount())
			return NULL;
		return m_lstMenuItem[dwPos];
	}

	BOOL SMenuBar::CreateChildren(pugi::xml_node xmlNode)
	{
		pugi::xml_node itemStyle = xmlNode.child(KItemStyle);
		if(itemStyle) m_itemStyle.append_copy(itemStyle);

		for (pugi::xml_node xmlChild = xmlNode.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
		{
			if (_tcsicmp(xmlChild.name(), SMenuBarItem::GetClassName()) != 0)
				continue;
			Insert(xmlChild);
		}
		return TRUE;
	}

	LRESULT SMenuBar::MenuSwitch(int code, WPARAM wParam, LPARAM lParam)
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
				if (SMenuBar::m_pMenuBar->m_ptMouse != pt &&
					SMenuBar::m_pMenuBar->m_iNowMenu != -1)
				{
					HWND hHost = m_pMenuBar->GetContainer()->GetHostHwnd();
					SMenuBar::m_pMenuBar->m_ptMouse = pt;
					::ScreenToClient(hHost, &pt);
					int nIndex = SMenuBar::m_pMenuBar->HitTest(pt);
					if (nIndex != -1)
					{
						SMenuBarItem *menuItem = SMenuBar::m_pMenuBar->GetMenuItem(nIndex);
						if (menuItem && SMenuBar::m_pMenuBar->m_iNowMenu != nIndex)
						{
							SMenuBar::m_pMenuBar->m_pNowMenu = menuItem;
							SMenuBar::m_pMenuBar->m_iNowMenu = nIndex;
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
				if (SMenuBar::m_pMenuBar->m_iNowMenu == -1)
					return TRUE;
				if (vKey == VK_LEFT)
				{
					int nRevIndex = SMenuBar::m_pMenuBar->m_iNowMenu - 1;
					if (nRevIndex < 0) nRevIndex = SMenuBar::m_pMenuBar->m_lstMenuItem.GetCount() - 1;
					SMenuBarItem *menuItem = SMenuBar::m_pMenuBar->m_lstMenuItem[nRevIndex];
					if (menuItem)
					{
						HWND hHost = m_pMenuBar->GetContainer()->GetHostHwnd();

						SMenuBar::m_pMenuBar->m_pNowMenu = menuItem;
						SMenuBar::m_pMenuBar->m_iNowMenu = nRevIndex;
						::PostMessage(hHost, WM_KEYDOWN, VK_ESCAPE, 0);
						menuItem->SetTimer(TIMER_POP, 10);
						return TRUE;
					}
				}
				else if (vKey == VK_RIGHT)
				{
					int nNextIndex = SMenuBar::m_pMenuBar->m_iNowMenu + 1;
					if (nNextIndex >= (int)SMenuBar::m_pMenuBar->m_lstMenuItem.GetCount()) nNextIndex = 0;
					SMenuBarItem *menuItem = SMenuBar::m_pMenuBar->GetMenuItem(nNextIndex);
					if (menuItem)
					{
						HWND hHost = m_pMenuBar->GetContainer()->GetHostHwnd();

						SMenuBar::m_pMenuBar->m_pNowMenu = menuItem;
						SMenuBar::m_pMenuBar->m_iNowMenu = nNextIndex;
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