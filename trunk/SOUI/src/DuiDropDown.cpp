#include "duistd.h"
#include "DuiDropDown.h"

namespace SOUI
{

	CDuiDropDownWnd::CDuiDropDownWnd(IDuiDropDownOwner* pOwner)
		:m_pOwner(pOwner)
		,m_bClick(FALSE)
	{
		MsgFilterRegister(m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd());
	}

	CDuiDropDownWnd::~CDuiDropDownWnd()
	{
		MsgFilterUnregister(m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd());
	}

	void CDuiDropDownWnd::OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}

	void CDuiDropDownWnd::OnKillFocus( HWND wndFocus )
	{
		if(wndFocus != m_hWnd)
		{
			EndDropDown();
		}
	}

	BOOL CDuiDropDownWnd::Create(LPCRECT lpRect ,LPVOID lParam,DWORD dwStyle,DWORD dwExStyle)
	{
		HWND hParent = m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd();
		HWND hWnd=CSimpleWnd::Create(NULL,dwStyle,dwExStyle,lpRect->left,lpRect->top,lpRect->right-lpRect->left,lpRect->bottom-lpRect->top,hParent,0);
		if(!hWnd) return FALSE;
		m_pOwner->OnDropDown(this);
		return TRUE;
	}

	void CDuiDropDownWnd::OnLButtonDown( UINT nFlags, CPoint point )
	{
		CRect rcWnd;
		GetWindowRect(&rcWnd);
		ClientToScreen(&point);
		if(!rcWnd.PtInRect(point))
		{
			EndDropDown();
		}else
		{
			m_bClick=TRUE;
			SetMsgHandled(FALSE);
		}
	}

	void CDuiDropDownWnd::OnLButtonUp( UINT nFlags, CPoint point )
	{
		if(m_bClick)
		{
			LRESULT lRes=0;
			HWND hWnd=m_hWnd;
			CDuiHostWnd::ProcessWindowMessage(m_hWnd,WM_LBUTTONUP,nFlags,MAKELPARAM(point.x,point.y),lRes);
			if(::IsWindow(hWnd)) EndDropDown();//强制关闭弹出窗口
		}
	}

	void CDuiDropDownWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
	{
		if(nChar==VK_ESCAPE) EndDropDown();
		else SetMsgHandled(FALSE);
	}

	void CDuiDropDownWnd::EndDropDown()
	{
		HWND hWnd=m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd();
		DestroyWindow();
		SetActiveWindow(hWnd);
	}

	void CDuiDropDownWnd::OnDestroy()
	{
		m_pOwner->OnCloseUp(this);
		SetMsgHandled(FALSE);
	}

	BOOL CDuiDropDownWnd::PreTranslateMessage( MSG* pMsg )
	{
		if(pMsg->message==WM_ACTIVATEAPP)
		{
			SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
			return FALSE;
		}
		if(!(pMsg->message>=WM_KEYFIRST && pMsg->message<=WM_KEYLAST) && pMsg->message!=WM_MOUSEWHEEL) return FALSE;
		if(!_PreTranslateMessage(pMsg))
		{
			SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
		}
		return TRUE;
	}

	void CDuiDropDownWnd::OnActivateApp( BOOL bActive, DWORD dwThreadID )
	{
		if(!bActive)
		{
			EndDropDown();
		}
	}

	int CDuiDropDownWnd::OnMouseActivate( HWND wndTopLevel, UINT nHitTest, UINT message )
	{
		return MA_NOACTIVATEANDEAT;
	}

}//end of namespace SOUI