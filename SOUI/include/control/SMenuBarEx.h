#pragma once

#include <helper/SMenuEx.h>
#include <helper/SMenu.h>

namespace SOUI
{
	class SMenuItemEx;

	class SOUI_EXP SMenuBarEx : 
		public SWindow
	{
		SOUI_CLASS_NAME(SMenuBarEx, L"menubarex")
		friend class SMenuItemEx;
	public:
		SMenuBarEx();
		~SMenuBarEx();

		BOOL Init(SHostWnd *pHostWnd);
		BOOL Insert(LPCTSTR pszTitle, LPCTSTR pszResName, int iPos = -1);
		BOOL Insert(pugi::xml_node xmlNode, int iPos = -1);

		SMenuEx *GetMenu(DWORD dwPos);

		int HitTest(CPoint pt);
	protected:
		SMenuItemEx *GetMenuItem(DWORD dwPos);
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);

		static LRESULT CALLBACK MenuInputFilter(int code, WPARAM wParam, LPARAM lParam);

		SArray<SMenuItemEx*> m_lstMenuItem;
		HWND m_hWnd;
		pugi::xml_document  m_xmlStyle;
		BOOL m_bIsShow;
		SMenuItemEx *m_pNowMenu;
		int m_iNowMenu;
		CPoint m_ptMouse;

		static HHOOK m_hMsgHook;
		static SMenuBarEx *m_pMenuBar;
	};

}