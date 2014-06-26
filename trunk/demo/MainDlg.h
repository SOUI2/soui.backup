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
		SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
	}
	void OnRestore()
	{
		SendMessage(WM_SYSCOMMAND,SC_RESTORE);
	}
	void OnMinimize()
	{
		SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
	}

	void OnSize(UINT nType, CSize size)
	{
		SetMsgHandled(FALSE);
		if(!m_bLayoutInited) return;
// 		if(nType==SIZE_MAXIMIZED)
// 		{
// 			FindChildByCmdID(3)->SetVisible(TRUE);
// 			FindChildByCmdID(2)->SetVisible(FALSE);
// 		}else if(nType==SIZE_RESTORED)
// 		{
// 			FindChildByCmdID(3)->SetVisible(FALSE);
// 			FindChildByCmdID(2)->SetVisible(TRUE);
// 		}
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

	SOUI_NOTIFY_MAP_BEGIN()
		SOUI_NOTIFY_ID_COMMAND(1, OnClose)
		SOUI_NOTIFY_ID_COMMAND(2, OnMaximize)
		SOUI_NOTIFY_ID_COMMAND(3, OnRestore)
		SOUI_NOTIFY_ID_COMMAND(5, OnMinimize)
		SOUI_NOTIFY_NAME_CONTEXTMENU("edit_1140",OnEditMenu)
	SOUI_NOTIFY_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
// 		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMainDlg>) //需要圆角窗口时启用
		MSG_WM_CREATE(OnCreate)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_DESTROY(OnDestory)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_SOUI_NOTIFY()
// 		CHAIN_MSG_MAP_MEMBER((*m_pUiHandler))
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL			m_bLayoutInited;
	int				m_iStep;
//	CUIHander *    m_pUiHandler; 
};
