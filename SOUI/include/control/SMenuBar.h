#pragma once

#include <helper/SMenuEx.h>
#include <helper/SMenu.h>

namespace SOUI
{
	class SMenuBarItem;

	interface SOUI_EXP IMenuHolder
	{
		virtual ~IMenuHolder(){}
		virtual int TrackPopupMenu(__in UINT uFlags, __in int x, __in int y, __in HWND hWnd) PURE;
		virtual BOOL LoadMenu(const SStringW & strValue) PURE;
	};

	class SOUI_EXP SMenuBar : 
		public SWindow
	{
		SOUI_CLASS_NAME(SMenuBar, L"menubar")
		friend class SMenuBarItem;
	public:
		SMenuBar();
		~SMenuBar();

		BOOL Insert(LPCTSTR pszTitle, LPCTSTR pszResName, int iPos = -1);
		BOOL Insert(pugi::xml_node xmlNode, int iPos = -1);

		IMenuHolder *GetMenu(DWORD dwPos);

		int HitTest(CPoint pt);
	protected:
		SMenuBarItem *GetMenuItem(DWORD dwPos);
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);

		static LRESULT CALLBACK MenuSwitch(int code, WPARAM wParam, LPARAM lParam);

		SArray<SMenuBarItem*> m_lstMenuItem;
		BOOL m_bIsShow;
		SMenuBarItem *m_pNowMenu;
		pugi::xml_document m_itemStyle;
		int m_iNowMenu;
		CPoint m_ptMouse;

		static HHOOK m_hMsgHook;
		static SMenuBar *m_pMenuBar;
	};

}