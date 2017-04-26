#pragma once
#include "MainDlg.h"
class CRealWndDlg : public SHostDialog
{
public:
    CRealWndDlg();
    ~CRealWndDlg(void);

	void On_Menu_Open();
	void On_Open_Files();
	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
	//接受键盘输入
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLbuttonDBLCLK(UINT nFlags, CPoint pt);
	void OnLbuttonup(UINT nFlags, CPoint pt);
	
protected:
	//soui消息
	EVENT_MAP_BEGIN()
		
		EVENT_ID_COMMAND(400, On_Menu_Open)
		EVENT_ID_COMMAND(402, On_Open_Files)
	EVENT_MAP_END()
    //消息映射表
    BEGIN_MSG_MAP_EX(CRealWndDlg)
		MSG_WM_COMMAND(OnCommand)
		//处理键盘按键消息
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDBLCLK(OnLbuttonDBLCLK)
		MSG_WM_LBUTTONUP(OnLbuttonup)
		
        CHAIN_MSG_MAP(SHostDialog)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
private:
	SMenuEx m_menu_open;
	
};
