#pragma once

class CWinBox : public SHostWnd
{
public:
	CWinBox(void);
	~CWinBox(void);

	void OnClose();				//关闭暴风盒子
	void OnBtnBack();			//后退
	void OnBtnForward();		//前进
	void OnBtnRefresh();		//刷新
	void OnBtnMaximize();		//最大化
	void OnBtnRestore();		//还原

	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

protected:
	//按钮事件处理映射表
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_box_close",OnClose)
		EVENT_NAME_COMMAND(L"btn_box_back",OnBtnBack)
		EVENT_NAME_COMMAND(L"btn_box_forward",OnBtnForward)
		EVENT_NAME_COMMAND(L"btn_box_refresh",OnBtnRefresh)
		EVENT_NAME_COMMAND(L"btn_box_maximize",OnBtnMaximize)
		EVENT_NAME_COMMAND(L"btn_box_restore",OnBtnRestore)
		EVENT_MAP_END()    

		//窗口消息处理映射表
		BEGIN_MSG_MAP_EX(CWinBox)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		CHAIN_MSG_MAP(SHostWnd)//注意将没有处理的消息交给基类处理
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
private:
	BOOL            m_bLayoutInited;
public:
	void*			m_boxParent;
	RECT			rect_box;
};