// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

// #include "UIHander.h"
#include "wtlhelper/whwindow.h"

class CMainDlg : public CDuiHostWnd
// 	,public CWHRoundRectFrameHelper<CMainDlg>	//需要圆角窗口时启用
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose()
	{
		AnimateHostWindow(200,AW_CENTER|AW_HIDE);
		EndDialog(IDCANCEL);
	}
	void OnMaximize()
	{
		GetNative()->SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
	}
	void OnRestore()
	{
		GetNative()->SendMessage(WM_SYSCOMMAND,SC_RESTORE);
	}
	void OnMinimize()
	{
		GetNative()->SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
	}

	void OnSize(UINT nType, CSize size)
	{
		SetMsgHandled(FALSE);
		if(!m_bLayoutInited) return;
		if(nType==SIZE_MAXIMIZED)
		{
			FindChildByID(3)->SetVisible(TRUE);
			FindChildByID(2)->SetVisible(FALSE);
		}else if(nType==SIZE_RESTORED)
		{
			FindChildByID(3)->SetVisible(FALSE);
			FindChildByID(2)->SetVisible(TRUE);
		}
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnShowWindow(BOOL bShow, UINT nStatus);

protected:
    void InitListCtrl();
    bool OnListHeaderClick( SWindow * pSender, LPSNMHDR pNmhdr );
    
    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
    void OnDestory();

	LRESULT OnEditMenu(CPoint pt)
	{
		//演示屏蔽edit_1140的右键菜单
		return 1;
	}

	EVENT_MAP_BEGIN()
		EVENT_ID_COMMAND(1, OnClose)
		EVENT_ID_COMMAND(2, OnMaximize)
		EVENT_ID_COMMAND(3, OnRestore)
		EVENT_ID_COMMAND(5, OnMinimize)
		EVENT_NAME_CONTEXTMENU(L"edit_1140",OnEditMenu)
	EVENT_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
// 		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMainDlg>) //需要圆角窗口时启用
		MSG_WM_CREATE(OnCreate)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_DESTROY(OnDestory)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SHOWWINDOW(OnShowWindow)
// 		CHAIN_MSG_MAP_MEMBER((*m_pUiHandler))
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL			m_bLayoutInited;
	int				m_iStep;
//	CUIHander *    m_pUiHandler; 
};
