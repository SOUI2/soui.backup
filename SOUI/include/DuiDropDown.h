//一个通用的下拉窗口模块
#pragma once

#include "duihostwnd.h"

namespace SOUI
{

	class CDuiDropDownWnd;
	class SOUI_EXP IDuiDropDownOwner
	{
	public:
		virtual CDuiWindow * GetDropDownOwner() =0;
		virtual void OnDropDown(CDuiDropDownWnd *pDropDown) = 0;
		virtual void OnCloseUp(CDuiDropDownWnd *pDropDown,UINT uCode) = 0;
	};

	///////////////////////////////////////////////////////////////////////////////////////////

	class SOUI_EXP CDuiDropDownWnd : public CDuiHostWnd , public CDuiMessageFilter
	{
	public:
		CDuiDropDownWnd(IDuiDropDownOwner* pOwner);
		virtual ~CDuiDropDownWnd();

		virtual BOOL Create(LPCRECT lpRect,LPVOID lParam,DWORD dwStyle=WS_POPUP,DWORD dwExStyle=WS_EX_TOOLWINDOW|WS_EX_TOPMOST);
		void EndDropDown(UINT uCode=IDCANCEL);
	protected:
		void OnLButtonDown(UINT nFlags, CPoint point);
		void OnLButtonUp(UINT nFlags, CPoint point);
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnKillFocus(HWND wndFocus);
		void OnDestroy();
		void OnActivateApp(BOOL bActive, DWORD dwThreadID);
		int OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message);

		virtual void OnFinalMessage(HWND);

		virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
		IDuiDropDownOwner* m_pOwner;
		BOOL				m_bClick;
		UINT				m_uExitCode;

		BEGIN_MSG_MAP_EX(CDuiDropDownWnd)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_KEYDOWN(OnKeyDown);
			MSG_WM_KILLFOCUS(OnKillFocus)
			MSG_WM_DESTROY(OnDestroy)
			MSG_WM_ACTIVATEAPP(OnActivateApp)
			MSG_WM_MOUSEACTIVATE(OnMouseActivate)
			CHAIN_MSG_MAP(CDuiHostWnd)
		END_MSG_MAP()
	};

}//end of namespace SOUI