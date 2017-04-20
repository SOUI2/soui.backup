#pragma once

class CSkinMgr : public SHostWnd
{
public:
	CSkinMgr(void);
	~CSkinMgr(void);

	void OnClose()
	{
		ShowWindow(SW_HIDE);
	}

	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	void OnBtnMaxspeed();			//极速
	void OnBtnDeepblue();			//深湖蓝
	void OnBtnSelfdefine();			//自定义
	void OnBtnBigbang();			//大片风暴
	void OnBtnPrev();				//上一个皮肤
	void OnBtnChoosing1();			//已有的皮肤1
	void OnBtnChoosing2();			//已有的皮肤2
	void OnBtnNext();				//下一个皮肤
	void OnBtnColor1();				//配色1
	void OnBtnColor2();				//配色2
	void OnBtnColor3();				//配色3
	void OnBtnColor4();				//配色4
	void OnBtnColor5();				//配色5
	void OnBtnColor6();				//配色6
	void OnBtnColor7();				//配色7
	void OnBtnColor8();				//配色8
	void OnBtnColor9();				//配色9
	void OnBtnColor10();			//配色10
	void OnBtnColor11();			//配色11
	void OnBtnColor12();			//配色12

protected:
	//按钮事件处理映射表
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_skin_close",OnClose)

		EVENT_NAME_COMMAND(L"btn_skinmgr_maxspeed",OnBtnMaxspeed)
		EVENT_NAME_COMMAND(L"btn_skinmgr_deepblue",OnBtnDeepblue)
		EVENT_NAME_COMMAND(L"btn_skinmgr_selfdefine",OnBtnSelfdefine)
		EVENT_NAME_COMMAND(L"btn_skinmgr_bigbang",OnBtnBigbang)
		EVENT_NAME_COMMAND(L"btn_skinmgr_prev",OnBtnPrev)
		EVENT_NAME_COMMAND(L"btn_skinmgr_choosing1",OnBtnChoosing1)
		EVENT_NAME_COMMAND(L"btn_skinmgr_choosing2",OnBtnChoosing2)
		EVENT_NAME_COMMAND(L"btn_skinmgr_next",OnBtnNext)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color1",OnBtnColor1)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color2",OnBtnColor2)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color3",OnBtnColor3)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color4",OnBtnColor4)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color5",OnBtnColor5)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color6",OnBtnColor6)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color7",OnBtnColor7)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color8",OnBtnColor8)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color9",OnBtnColor9)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color10",OnBtnColor10)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color11",OnBtnColor11)
		EVENT_NAME_COMMAND(L"btn_skinmgr_color12",OnBtnColor12)
		EVENT_MAP_END()    

		//窗口消息处理映射表
		BEGIN_MSG_MAP_EX(CSkinMgr)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		CHAIN_MSG_MAP(SHostWnd)//注意将没有处理的消息交给基类处理
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
private:
	BOOL            m_bLayoutInited;
};