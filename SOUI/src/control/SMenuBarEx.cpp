#include "souistd.h"
#include "control/SMenuBarEx.h"

#define TIMER_POP	10


namespace SOUI
{
	HHOOK SMenuBarEx::m_hMsgHook = NULL;
	SMenuBarEx *SMenuBarEx::m_pMenuBar = NULL;

	const TCHAR XmlBtnStyle[] = _T("btnStyle");
	const TCHAR XmlMenus[] = _T("menus");

	class SMenuItemEx :
		public SButton
	{
		SOUI_CLASS_NAME(SMenuItemEx, L"menuItemEx")
			friend class SMenuBarEx;
	public:
		SMenuItemEx(SMenuBarEx *pHostMenu);
		~SMenuItemEx();

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
		SMenuBarEx* m_pHostMenu;
		BOOL m_bIsRegHotKey;
		int m_iIndex;
		TCHAR m_cAccessKey;
		SMenuEx m_MenuEx;
	};

	SMenuItemEx::SMenuItemEx(SMenuBarEx *pHostMenu) :
		m_data(0),
		m_pHostMenu(pHostMenu),
		m_bIsRegHotKey(FALSE),
		m_iIndex(-1),
		m_cAccessKey(0)
	{
		m_bDrawFocusRect = FALSE;
		GetEventSet()->subscribeEvent(EventCmd::EventID, Subscriber(&SMenuItemEx::OnCmd, this));
	}

	SMenuItemEx::~SMenuItemEx()
	{
	}

	bool SMenuItemEx::IsMenuLoaded() const
	{
		return true;
	}

	UINT SMenuItemEx::PopMenu()
	{
		m_pHostMenu->m_bIsShow = TRUE;
		m_pHostMenu->m_pNowMenu = this;
		m_pHostMenu->m_iNowMenu = m_iIndex;

		SetCheck(TRUE);

		CRect rcHost;
		::GetWindowRect(m_pHostMenu->m_hWnd, rcHost);
		CRect rcMenu = GetClientRect();

		if (SMenuBarEx::m_hMsgHook == NULL)
			SMenuBarEx::m_hMsgHook = ::SetWindowsHookEx(WH_MSGFILTER,
				SMenuBarEx::MenuInputFilter, NULL, GetCurrentThreadId());// m_bLoop may become TRUE

		int iRet = 0;
		iRet = m_MenuEx.TrackPopupMenu(TPM_RETURNCMD,
			rcHost.left + rcMenu.left, rcHost.top + rcMenu.bottom + 2, m_pHostMenu->m_hWnd);

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
		/*EventSelectMenu evt(m_pHostMenu);
		evt.m_id = iRet;
		evt.m_pMenu = this;
		FireEvent(evt);*/

		return iRet;
	}

	HRESULT SMenuItemEx::OnAttrSrc(const SStringW & strValue, BOOL bLoading)
	{
		return m_MenuEx.LoadMenu(strValue) ? S_OK : E_INVALIDARG;
	}

	CSize SMenuItemEx::GetDesiredSize(LPCRECT pRcContainer)
	{
		return SWindow::GetDesiredSize(pRcContainer);
	}

	bool SMenuItemEx::OnCmd(EventArgs * e)
	{
		if (!::IsWindow(m_pHostMenu->m_hWnd))
			return false;
		e->bubbleUp = false;
		PopMenu();
		return true;
	}

	void SMenuItemEx::OnTimer(UINT_PTR timerID)
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

	SMenuBarEx::SMenuBarEx() :
		m_bIsShow(FALSE),
		m_hWnd(NULL),
		m_pNowMenu(NULL),
		m_iNowMenu(-1)
	{
		//m_evtSet.addEvent(EVENTID(EventSelectMenu));
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
	BOOL SMenuBarEx::Init(SHostWnd * pHostWnd)
	{
		if (::IsWindow(pHostWnd->m_hWnd))
		{
			m_hWnd = pHostWnd->m_hWnd;
			return TRUE;
		}
		return FALSE;
	}
	BOOL SMenuBarEx::Insert(LPCTSTR pszTitle, LPCTSTR pszResName, int iPos)
	{
		if (!pszResName)
			return FALSE;

		SMenuItemEx *pNewMenu = new SMenuItemEx(this);
		SASSERT(pNewMenu);
		InsertChild(pNewMenu);

		pugi::xml_node xmlBtnStyle = m_xmlStyle.child(XmlBtnStyle);
		if (xmlBtnStyle)
			pNewMenu->InitFromXml(xmlBtnStyle);

		if (pszTitle)
			pNewMenu->SetWindowText(pszTitle);

		pNewMenu->SetAttribute(L"src", S_CT2W(pszResName));
		pNewMenu->SetWindowText(pszTitle);

		if (!pNewMenu->IsMenuLoaded())
		{
			DestroyChild(pNewMenu);
			return FALSE;
		}
		if (iPos < 0) iPos = m_lstMenuItem.GetCount();
		m_lstMenuItem.InsertAt(iPos, pNewMenu);

		pNewMenu->m_iIndex = iPos;
		for (size_t i = iPos + 1; i < m_lstMenuItem.GetCount(); i++)
		{
			m_lstMenuItem[i]->m_iIndex++;
		}
		return TRUE;
	}
	BOOL SMenuBarEx::Insert(pugi::xml_node xmlNode, int iPos)
	{
		SMenuItemEx *pNewMenu = new SMenuItemEx(this);
		SASSERT(pNewMenu);
		InsertChild(pNewMenu);

		pugi::xml_node xmlBtnStyle = m_xmlStyle.child(XmlBtnStyle);
		if (xmlBtnStyle)
			pNewMenu->InitFromXml(xmlBtnStyle);
		pNewMenu->InitFromXml(xmlNode);

		if (!pNewMenu->IsMenuLoaded())
		{
			DestroyChild(pNewMenu);
			return FALSE;
		}
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
			SMenuItemEx *pItem = m_lstMenuItem[i];
			CRect rcItem = pItem->GetClientRect();
			if (rcItem.PtInRect(pt))
				return i;
		}
		return -1;
	}
	SMenuItemEx * SMenuBarEx::GetMenuItem(DWORD dwPos)
	{
		if (dwPos >= m_lstMenuItem.GetCount())
			return NULL;
		return m_lstMenuItem[dwPos];
	}
	BOOL SMenuBarEx::CreateChildren(pugi::xml_node xmlNode)
	{
		pugi::xml_node xmlBtnStyle = xmlNode.child(XmlBtnStyle);
		if (xmlBtnStyle)
		{
			m_xmlStyle.append_copy(xmlBtnStyle);
		}
		pugi::xml_node xmlTMenus = xmlNode.child(XmlMenus);
		if (xmlTMenus)
		{
			for (pugi::xml_node xmlChild = xmlTMenus.first_child(); xmlChild; xmlChild = xmlChild.next_sibling())
			{
				if (_tcsicmp(xmlChild.name(), SMenuItemEx::GetClassName()) != 0)
					continue;
				Insert(xmlChild);
			}
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
					SMenuBarEx::m_pMenuBar->m_ptMouse = pt;
					::ScreenToClient(SMenuBarEx::m_pMenuBar->m_hWnd, &pt);
					int nIndex = SMenuBarEx::m_pMenuBar->HitTest(pt);
					if (nIndex != -1)
					{
						SMenuItemEx *menuItem = SMenuBarEx::m_pMenuBar->GetMenuItem(nIndex);
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
					SMenuItemEx *menuItem = SMenuBarEx::m_pMenuBar->m_lstMenuItem[nRevIndex];
					if (menuItem)
					{
						SMenuBarEx::m_pMenuBar->m_pNowMenu = menuItem;
						SMenuBarEx::m_pMenuBar->m_iNowMenu = nRevIndex;
						::PostMessage(SMenuBarEx::m_pMenuBar->m_hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
						menuItem->SetTimer(TIMER_POP, 10);
						return TRUE;
					}
				}
				else if (vKey == VK_RIGHT)
				{
					int nNextIndex = SMenuBarEx::m_pMenuBar->m_iNowMenu + 1;
					if (nNextIndex >= (int)SMenuBarEx::m_pMenuBar->m_lstMenuItem.GetCount()) nNextIndex = 0;
					SMenuItemEx *menuItem = SMenuBarEx::m_pMenuBar->GetMenuItem(nNextIndex);
					if (menuItem)
					{
						SMenuBarEx::m_pMenuBar->m_pNowMenu = menuItem;
						SMenuBarEx::m_pMenuBar->m_iNowMenu = nNextIndex;
						::PostMessage(SMenuBarEx::m_pMenuBar->m_hWnd, WM_KEYDOWN, VK_ESCAPE, 0);
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