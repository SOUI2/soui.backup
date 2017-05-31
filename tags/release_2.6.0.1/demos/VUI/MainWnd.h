#pragma once
#include "SkinMgr.h"
#include "WinBox.h"
#include "helper/SMenu.h"

class CMainWnd : public SHostWnd
{
public:
	CMainWnd();
	~CMainWnd();

	void OnClose()
	{
		PostMessage(WM_QUIT);
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
		if(nType==SIZE_MAXIMIZED)
		{
			FindChildByName(L"btn_restore")->SetVisible(TRUE);
			FindChildByName(L"btn_max")->SetVisible(FALSE);
		}else if(nType==SIZE_RESTORED)
		{
			FindChildByName(L"btn_restore")->SetVisible(FALSE);
			FindChildByName(L"btn_max")->SetVisible(TRUE);
		}
	}

	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

	//按钮控件的响应
	void OnBtnIcon();				//左上角icon按钮
	void OnBtnFeedback();			//意见反馈
	void OnBtnSkins();				//换肤按钮	
	void OnBtnBgOpen();				//播放区域打开文件
	void OnBtnBgOpenMenu();			//播放区域打开菜单
	void OnBtnTools();				//工具箱
	void OnBtnLEye();				//左眼
	void OnBtnLEyed();				//关闭左眼
	void OnBtnStop();				//停止
	void OnBtnPageUp();				//上一个
	void OnBtnPlay();				//播放
	void OnBtnPause();				//暂停
	void OnBtnPageDown();			//下一个
	void OnBtnOpen();				//打开文件
	void OnBtnVolume();				//静音
	void OnBtnVolumeQuit();				//静音
	void OnBtnFullscreen();			//全屏
	void OnBtnQuitFullscreen();		//关闭全屏
	void OnBtnListShow();			//显示播放列表
	void OnBtnListShowed();			//隐藏播放列表
	void OnBtnBox();				//打开暴风盒子
	void OnBtnBoxHide();				//关闭暴风盒子

	void OnBtnToolsWinClose();		//工具箱窗口的关闭按钮
	void OnBtnToolsWinPrev();		//上一个
	void OnBtnToolsWinNext();		//下一个
	void OnBtnTool3D();				//3D
	void OnBtnToolLefteye();		//左眼
	void OnBtnToolSurronudsound();	//环绕声
	void OnBtnToolFlyscreen();		//飞屏
	void OnBtnToolGame();			//游戏
	void OnBtnToolDownload();		//下载管理
	void OnBtnToolTranscode();		//转码
	void OnBtnToolMovielib();		//影视库
	void OnBtnToolBarrage();		//弹幕
	void OnBtnToolDlna();			//dlna
	void OnBtnToolNews();			//资讯
	void OnBtnToolScreebshots();	//截图
	void OnBtnToolShoot();			//连拍

	void OnBtnPage1SortordMenu();	//播放列表排序方式
	void OnBtnPage1Sortord();	//播放列表排序方向，向下或向上

	//播放列表tab页2
	void OnBtnAll()	;				//【全部】
	void OnBtnAdd()	;				//【+】
	void OnBtnDelete()	;			//【-】
	void OnBtnClear()	;			//清空列表按钮
	void OnBtnOrderPlay();			//顺序播放
	void OnBtnSinglePlay()	;		//单个播放
	void OnBtnRandomPlay()	;		//随机播放
	void OnBtnSingleCycle()	;		//单个循环
	void OnBtnListCycle()	;		//列表循环

	//DUI菜单响应函数
	void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);

	SMenu menu_sortord;				//tab页1中列表排序方式菜单
	SMenu menu_icon;				//icon按钮的弹出菜单
	SMenu menu_PlayArea;			//播放区域打开文件按钮的弹出菜单
	SMenu menu_PlayMode;			//播放模式菜单

	int up_or_down;					//排序方式向上或向下
	BOOL popularity_up_or_down;		//排序方式为“观众”时，向上或向下

protected:
	//按钮事件处理映射表
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close",OnClose)
		EVENT_NAME_COMMAND(L"btn_min",OnMinimize)
		EVENT_NAME_COMMAND(L"btn_max",OnMaximize)
		EVENT_NAME_COMMAND(L"btn_restore",OnRestore)

		EVENT_NAME_COMMAND(L"btn_icon",OnBtnIcon)
		EVENT_NAME_COMMAND(L"btn_skins",OnBtnSkins)
		EVENT_NAME_COMMAND(L"btn_feedback",OnBtnFeedback)
		EVENT_NAME_COMMAND(L"btn_bg_open",OnBtnBgOpen)
		EVENT_NAME_COMMAND(L"btn_bg_openmenu",OnBtnBgOpenMenu)
		EVENT_NAME_COMMAND(L"btn_tools",OnBtnTools)
		EVENT_NAME_COMMAND(L"btn_left_eye",OnBtnLEye)
		EVENT_NAME_COMMAND(L"btn_left_eyed",OnBtnLEyed)
		EVENT_NAME_COMMAND(L"btn_stop",OnBtnStop)
		EVENT_NAME_COMMAND(L"btn_page_up",OnBtnPageUp)
		EVENT_NAME_COMMAND(L"btn_play",OnBtnPlay)
		EVENT_NAME_COMMAND(L"btn_pause",OnBtnPause)
		EVENT_NAME_COMMAND(L"btn_page_down",OnBtnPageDown)
		EVENT_NAME_COMMAND(L"btn_open",OnBtnOpen)
		EVENT_NAME_COMMAND(L"btn_volume",OnBtnVolume)
		EVENT_NAME_COMMAND(L"btn_volume_mute",OnBtnVolumeQuit)
		EVENT_NAME_COMMAND(L"btn_fullscreen",OnBtnFullscreen)
		EVENT_NAME_COMMAND(L"btn_quit_fullscreen",OnBtnQuitFullscreen)
		EVENT_NAME_COMMAND(L"btn_list_show",OnBtnListShow)
		EVENT_NAME_COMMAND(L"btn_list_showed",OnBtnListShowed)
		EVENT_NAME_COMMAND(L"btn_box",OnBtnBox)
		EVENT_NAME_COMMAND(L"btn_boxed",OnBtnBoxHide)

		EVENT_NAME_COMMAND(L"btn_tools_close",OnBtnToolsWinClose)
		EVENT_NAME_COMMAND(L"btn_tool_prev",OnBtnToolsWinPrev)
		EVENT_NAME_COMMAND(L"btn_tool_next",OnBtnToolsWinNext)
		EVENT_NAME_COMMAND(L"btn_tool_3D",OnBtnTool3D)
		EVENT_NAME_COMMAND(L"btn_tool_lefteye",OnBtnToolLefteye)
		EVENT_NAME_COMMAND(L"btn_tool_surround_sound",OnBtnToolSurronudsound)
		EVENT_NAME_COMMAND(L"btn_tool_flyscreen",OnBtnToolFlyscreen)
		EVENT_NAME_COMMAND(L"btn_tool_game",OnBtnToolGame)
		EVENT_NAME_COMMAND(L"btn_tool_download",OnBtnToolDownload)
		EVENT_NAME_COMMAND(L"btn_tool_transcode",OnBtnToolTranscode)
		EVENT_NAME_COMMAND(L"btn_tool_movielib",OnBtnToolMovielib)
		EVENT_NAME_COMMAND(L"btn_tool_barrage",OnBtnToolBarrage)
		EVENT_NAME_COMMAND(L"btn_tool_dlna",OnBtnToolDlna)
		EVENT_NAME_COMMAND(L"btn_tool_news",OnBtnToolNews)
		EVENT_NAME_COMMAND(L"btn_tool_screenshots",OnBtnToolScreebshots)
		EVENT_NAME_COMMAND(L"btn_tool_shoot",OnBtnToolShoot)

		EVENT_NAME_COMMAND(L"btn_sortord_menu",OnBtnPage1SortordMenu)
		EVENT_NAME_COMMAND(L"btn_sortord",OnBtnPage1Sortord)

		EVENT_NAME_COMMAND(L"btn_all",OnBtnAll)
		EVENT_NAME_COMMAND(L"btn_add",OnBtnAdd)
		EVENT_NAME_COMMAND(L"btn_delete",OnBtnDelete)
		EVENT_NAME_COMMAND(L"btn_clear",OnBtnClear)
		EVENT_NAME_COMMAND(L"btn_OrderPlay",OnBtnOrderPlay)
		EVENT_NAME_COMMAND(L"btn_SinglePlay",OnBtnSinglePlay)
		EVENT_NAME_COMMAND(L"btn_RandomPlay",OnBtnRandomPlay)
		EVENT_NAME_COMMAND(L"btn_SingleCycle",OnBtnSingleCycle)
		EVENT_NAME_COMMAND(L"btn_ListCycle",OnBtnListCycle)
		EVENT_MAP_END()    

		//窗口消息处理映射表
		BEGIN_MSG_MAP_EX(CMainWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_COMMAND(OnCommand)
		CHAIN_MSG_MAP(SHostWnd)//注意将没有处理的消息交给基类处理
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
private:
	BOOL            m_bLayoutInited;
	CSkinMgr		m_dlgSkinMgr;		//皮肤管理窗口
	CWinBox			m_winBox;				//暴风盒子
};