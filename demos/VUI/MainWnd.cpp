#include "StdAfx.h"
#include "MainWnd.h"
#include "helper/SMenu.h"
#include "../controls.extend/FileHelper.h"

#include<string>
#include<iostream>
#include <ShellAPI.h>

CMainWnd::CMainWnd(void) : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited=FALSE;
}

CMainWnd::~CMainWnd(void)
{
}


BOOL CMainWnd::OnInitDialog( HWND hWnd, LPARAM lParam )
{
	// 创建皮肤管理窗口
	m_dlgSkinMgr.Create(m_hWnd);
	m_dlgSkinMgr.GetNative()->SendMessage(WM_INITDIALOG);

	// 创建暴风盒子窗口
	m_winBox.Create(m_hWnd);
	m_winBox.m_boxParent=(void *)this;
	m_winBox.GetNative()->SendMessage(WM_INITDIALOG);

	menu_sortord.LoadMenu(_T("menu_playlist_sortord"),_T("LAYOUT"));	//加载tab页1中列表排序方式菜单

	menu_icon.LoadMenu(_T("menu_test"),_T("LAYOUT"));					//icon按钮的弹出菜单

	menu_PlayArea.LoadMenu(_T("menu_open"),_T("LAYOUT"));				//播放区域打开文件按钮的弹出菜单

	menu_PlayMode.LoadMenu(_T("menu_playmode"),_T("LAYOUT"));			//播放模式菜单
	::CheckMenuItem(menu_PlayMode.m_hMenu,11101,MF_CHECKED);
	::CheckMenuItem(menu_PlayMode.m_hMenu,11102,MF_UNCHECKED);
	::CheckMenuItem(menu_PlayMode.m_hMenu,11103,MF_UNCHECKED);
	::CheckMenuItem(menu_PlayMode.m_hMenu,11104,MF_UNCHECKED);
	::CheckMenuItem(menu_PlayMode.m_hMenu,11105,MF_UNCHECKED);

	m_bLayoutInited=TRUE;

	up_or_down = 0;			//排序方式,向上或向下,初始化为0
	popularity_up_or_down = TRUE;	//排序方式为“观众”时，向上或向下,初始化为TRUE

	return 0;
}

void CMainWnd::OnBtnIcon()	// 左上角icon按钮
{



	CRect rc_menu;
	SWindow * pBtn = FindChildByName(L"btn_icon");
	if(pBtn) 
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);
		menu_icon.TrackPopupMenu(0, rc_menu.left, rc_menu.bottom, m_hWnd);
	}
}

void CMainWnd::OnBtnFeedback()	//意见反馈
{
	ShellExecute(NULL, _T("open"), _T("explorer.exe"), _T("http://www.zhihu.com/"), NULL, SW_SHOW);
}

void CMainWnd::OnBtnSkins()//打开皮肤管理
{
	CRect rc_temp;
	SWindow * pBtn = FindChildByName(L"btn_skins");
	if(pBtn) 
	{
		pBtn->GetClientRect(&rc_temp);
		ClientToScreen(&rc_temp);

		m_dlgSkinMgr.SetWindowPos(HWND_TOP, rc_temp.left, rc_temp.bottom,  360, 350, NULL); 
		m_dlgSkinMgr.ShowWindow(SW_SHOWNORMAL);
	}
}

void CMainWnd::OnBtnBgOpen()	//播放区域打开文件按钮
{
	CFileDialogEx openDlg(TRUE,_T("rmvb"),0,6,_T("视频文件(*.rmvb)\0*.rmvb\0All files (*.*)\0*.*\0\0"));
	if(openDlg.DoModal()==IDOK)
		SMessageBox(NULL,_T("OnBtnBgOpen"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnBgOpenMenu()	//播放区域打开文件按钮的弹出菜单
{
	CRect rc_menu;
	SWindow * pBtn = FindChildByName(L"btn_bg_open");
	if(pBtn) 
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);

		menu_PlayArea.TrackPopupMenu(0, rc_menu.left, rc_menu.bottom, m_hWnd);
	}
}
/******************************* 控制条 ***************************************************/
void CMainWnd::OnBtnTools()		// 工具箱
{
	SWindow * pBtn = FindChildByName(L"win_tools");
	if(pBtn) pBtn->SetVisible(!pBtn->IsVisible(TRUE),TRUE);
}

void CMainWnd::OnBtnLEye()	// 左眼
{
	SWindow * pBtn = FindChildByName(L"btn_left_eye");
	if(pBtn) pBtn->SetVisible(FALSE);

	pBtn = FindChildByName(L"btn_left_eyed");
	if(pBtn) pBtn->SetVisible(TRUE);
}

void CMainWnd::OnBtnLEyed()	// 关闭左眼
{
	SWindow * pBtn = FindChildByName(L"btn_left_eye");
	if(pBtn) pBtn->SetVisible(TRUE);

	pBtn = FindChildByName(L"btn_left_eyed");
	if(pBtn) pBtn->SetVisible(FALSE);
}

void CMainWnd::OnBtnStop()	// 停止
{
	SMessageBox(NULL,_T("OnBtnStop"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnPageUp()	//上一个
{
	SMessageBox(NULL,_T("OnBtnPageUp"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnPlay()	//播放
{
	SWindow * pBtn = FindChildByName(L"btn_play");
	if(pBtn) pBtn->SetVisible(FALSE);

	pBtn = FindChildByName(L"btn_pause");
	if(pBtn) pBtn->SetVisible(TRUE);
}

void CMainWnd::OnBtnPause()	//暂停
{
	SWindow * pBtn = FindChildByName(L"btn_play");
	if(pBtn) pBtn->SetVisible(TRUE);

	pBtn = FindChildByName(L"btn_pause");
	if(pBtn) pBtn->SetVisible(FALSE);
}

void CMainWnd::OnBtnPageDown()	//下一个
{
	SMessageBox(NULL,_T("OnBtnPageDown"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnOpen()	//打开文件
{
	CFileDialogEx openDlg(TRUE,_T("mp4"),0, OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT,_T("视频文件(*.mp4)\0*.mp4\0All files (*.*)\0*.*\0\0"));
	if(openDlg.DoModal()==IDOK)
		//SMessageBox(NULL,_T("OnBtnOpen"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
	{
		TCHAR szPath[MAX_PATH];  
		TCHAR szFileName[80 * MAX_PATH];  

		TCHAR * p;  
		int nLen = 0; 

		lstrcpyn(szPath, szFileName, openDlg.m_ofn.nFileOffset);  

		szPath[openDlg.m_ofn.nFileOffset] = '\0';  
		nLen = lstrlen(szPath);  

		if( szPath[nLen - 1] != '\\' )
		{  
			lstrcat(szPath, TEXT("\\"));  
		}  
		//p = szFile + openDlg.m_ofn.nFileOffset; //把指针移到第一个文件  
		p = szFileName + openDlg.m_ofn.nFileOffset; //把指针移到第一个文件
		int sum=0;
		while( *p )  
		{    
			ZeroMemory(szFileName, sizeof(szFileName)); 
			lstrcat(szFileName, szPath);  //给文件名加上路径    
			lstrcat(szFileName, p);    //加上文件名

			p += lstrlen(p) +1;     //移至下一个文件  
			sum++;
			//cout<<file_name<<endl ;
			lstrcat(szFileName, TEXT("\n")); //换行
			//STRACE("%s",szFileName);
		}
		printf("%d",sum);
		SMessageBox(NULL, szFileName, TEXT("MultiSelect"), MB_OK);
	}
}


void CMainWnd::OnBtnVolume()	//静音
{
	SWindow * pBtn = FindChildByName(L"btn_volume_mute");
	if(pBtn) pBtn->SetVisible(TRUE);

	pBtn = FindChildByName(L"btn_volume");
	if(pBtn) pBtn->SetVisible(FALSE);
}

void CMainWnd::OnBtnVolumeQuit()	//退出静音
{
	SWindow * pBtn = FindChildByName(L"btn_volume_mute");
	if(pBtn) pBtn->SetVisible(FALSE);

	pBtn = FindChildByName(L"btn_volume");
	if(pBtn) pBtn->SetVisible(TRUE);
}

void CMainWnd::OnBtnFullscreen()	//全屏
{
	SWindow * pBtn = FindChildByName(L"btn_quit_fullscreen");
	if(pBtn) pBtn->SetVisible(TRUE);

	pBtn = FindChildByName(L"btn_fullscreen");
	if(pBtn) pBtn->SetVisible(FALSE);

	//	SMessageBox(NULL,_T("OnBtnFullscreen"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnQuitFullscreen()	//关闭全屏
{
	SWindow * pBtn = FindChildByName(L"btn_quit_fullscreen");
	if(pBtn) pBtn->SetVisible(FALSE);

	pBtn = FindChildByName(L"btn_fullscreen");
	if(pBtn) pBtn->SetVisible(TRUE);

	//	SMessageBox(NULL,_T("OnBtnQuitFullscreen"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnListShow()	//显示播放列表
{
	SWindow * pBtn = FindChildByName(L"play_list");
	if(pBtn) 
	{
		pBtn->SetVisible(TRUE,TRUE);
	}

	pBtn = FindChildByName(L"btn_list_showed");
	if(pBtn) pBtn->SetVisible(TRUE,TRUE);

	pBtn = FindChildByName(L"btn_list_show");
	if(pBtn) pBtn->SetVisible(FALSE,TRUE);
}

void CMainWnd::OnBtnListShowed()	//隐藏播放列表
{
	SWindow * pBtn = FindChildByName(L"play_list");
	if(pBtn) 
	{
	pBtn->SetVisible(FALSE,TRUE);
	}


	pBtn = FindChildByName(L"btn_list_showed");
	if(pBtn) pBtn->SetVisible(FALSE,TRUE);

	pBtn = FindChildByName(L"btn_list_show");
	if(pBtn) pBtn->SetVisible(TRUE,TRUE);
}

void CMainWnd::OnBtnBox()	//打开暴风盒子
{
	//m_winBox.ShowWindow(SW_SHOWNORMAL);

	SWindow	*pBtn = FindChildByName(L"mainWnd");
	CRect rc_temp;
	if(pBtn) 
	{
		pBtn->GetClientRect(&rc_temp);
		ClientToScreen(&rc_temp);

		m_winBox.SetWindowPos(HWND_TOP, rc_temp.right -5, rc_temp.top -3,  250, rc_temp.Height() +10, NULL); 
		m_winBox.ShowWindow(SW_SHOWNORMAL);
	}

	pBtn = FindChildByName(L"btn_boxed");
	if(pBtn) pBtn->SetVisible(TRUE,TRUE);

	pBtn = FindChildByName(L"btn_box");
	if(pBtn) pBtn->SetVisible(FALSE,TRUE);
}

void CMainWnd::OnBtnBoxHide()	//关闭暴风盒子
{
	m_winBox.ShowWindow(SW_HIDE);

	SWindow* pBtn = FindChildByName(L"btn_boxed");
	if(pBtn) pBtn->SetVisible(FALSE,TRUE);

	pBtn = FindChildByName(L"btn_box");
	if(pBtn) pBtn->SetVisible(TRUE,TRUE);
}
/******************************* 工具箱窗口 ********************************************/
void CMainWnd::OnBtnToolsWinClose()		//关闭按钮
{
	SWindow* pWin = FindChildByName(L"win_tools");
	if(pWin) pWin->SetVisible(FALSE,TRUE);
}

void CMainWnd::OnBtnTool3D()		//3D
{
	SMessageBox(NULL,_T("3D"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolsWinPrev()		//上一个
{
	SWindow* pWin = FindChildByName(L"tool_page2");
	if(pWin) pWin->SetVisible(FALSE,TRUE);

	pWin = FindChildByName(L"tool_page1");
	if(pWin) pWin->SetVisible(TRUE,TRUE);
}

void CMainWnd::OnBtnToolsWinNext()		//下一个
{
	SWindow* pWin = FindChildByName(L"tool_page2");
	if(pWin) pWin->SetVisible(TRUE,TRUE);

	pWin = FindChildByName(L"tool_page1");
	if(pWin) pWin->SetVisible(FALSE,TRUE);
}

void CMainWnd::OnBtnToolLefteye()		//左眼
{
	SMessageBox(NULL,_T("左眼"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolSurronudsound()		//环绕声
{
	SMessageBox(NULL,_T("环绕声"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolFlyscreen()		//飞屏
{
	SMessageBox(NULL,_T("飞屏"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolGame()			//游戏
{
	SMessageBox(NULL,_T("游戏"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolDownload()		//下载管理
{
	SMessageBox(NULL,_T("下载管理"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolTranscode()		//转码
{
	SMessageBox(NULL,_T("转码"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolMovielib()		//影视库
{
	SMessageBox(NULL,_T("影视库"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolBarrage()		//弹幕
{
	SMessageBox(NULL,_T("弹幕"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolDlna()			//dlna
{
	SMessageBox(NULL,_T("dlna"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolNews()			//资讯
{
	SMessageBox(NULL,_T("资讯"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolScreebshots()	//截图
{
	SMessageBox(NULL,_T("截图"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnToolShoot()			//连拍
{
	SMessageBox(NULL,_T("连拍"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

/****************************** 播放列表tab页1 **************************************************/
void CMainWnd::OnBtnPage1SortordMenu()			//播放列表排序方式
{

	CRect rc_menu;
	SWindow * pBtn = FindChildByName(L"btn_sortord_menu");
	if(pBtn) 
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);

		menu_sortord.TrackPopupMenu(0, rc_menu.left - 50, rc_menu.bottom, m_hWnd);
	}
}

void CMainWnd::OnBtnPage1Sortord()			//播放列表排序方向，向下或向上
{

	SWindow *pDown = FindChildByName(L"sortord_down");
	SWindow *pUp = FindChildByName(L"sortord_up");
	if(pDown && pUp) 
	{
		if(!(pDown->IsVisible(FALSE)) && !(pUp->IsVisible(FALSE)))
			up_or_down = 0;
		else if(pDown->IsVisible(FALSE) && !(pUp->IsVisible(FALSE)))
			up_or_down = 1;
		else if(!(pDown->IsVisible(FALSE)) && pUp->IsVisible(FALSE))
			up_or_down = 2;
		else
			up_or_down = -1;
	}

	if(popularity_up_or_down)
	{
		switch(up_or_down)
		{
		case 0:
			pDown->SetVisible(TRUE, TRUE);
			pUp->SetVisible(FALSE, TRUE);
			up_or_down = 1;
			break;
		case 1:
			pDown->SetVisible(FALSE, TRUE);
			pUp->SetVisible(TRUE, TRUE);
			up_or_down = 2;
			break;
		case 2:
			pDown->SetVisible(FALSE, TRUE);
			pUp->SetVisible(FALSE, TRUE);
			up_or_down = 0;
		default:
			break;
		}
	}
	else
		switch(up_or_down)
	{
		case 1:
			pDown->SetVisible(FALSE, TRUE);
			pUp->SetVisible(TRUE, TRUE);
			up_or_down = 2;
			break;
		case 2:
			pDown->SetVisible(TRUE, TRUE);
			pUp->SetVisible(FALSE, TRUE);
			up_or_down = 1;
		default:
			break;
	}
}

/****************************** 播放列表tab页2 **************************************************/
void CMainWnd::OnBtnAll()					//【全部】
{
	SMessageBox(NULL,_T("【全部】"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnAdd()					//【+】
{
	SMessageBox(NULL,_T("【+】"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnDelete()				//【-】
{
	SMessageBox(NULL,_T("【-】"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnClear()				//清空列表按钮
{
	SMessageBox(NULL,_T("清空列表按钮"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CMainWnd::OnBtnOrderPlay()			//顺序播放
{
	POINT pt;
	GetCursorPos(&pt);
	menu_PlayMode.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

void CMainWnd::OnBtnSinglePlay()			//单个播放
{
	POINT pt;
	GetCursorPos(&pt);
	menu_PlayMode.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

void CMainWnd::OnBtnRandomPlay()			//随机播放
{
	POINT pt;
	GetCursorPos(&pt);
	menu_PlayMode.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

void CMainWnd::OnBtnSingleCycle()			//单个循环
{
	POINT pt;
	GetCursorPos(&pt);
	menu_PlayMode.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

void CMainWnd::OnBtnListCycle()			//列表循环
{
	POINT pt;
	GetCursorPos(&pt);
	menu_PlayMode.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}



//响应菜单事件
void CMainWnd::OnCommand( UINT uNotifyCode, int nID, HWND wndCtl )
{
	if(uNotifyCode==0)
	{
		if(nID==1101)
		{//nID==1101对应menu_playlist_sortord菜单的第一项
			SWindow* pBtn = FindChildByName(L"btn_sortord");
		//	if(pBtn) pBtn->SetWindowText (L"观众");

			pBtn = FindChildByName(L"sortord_down");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"sortord_up");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			up_or_down = -1;
			popularity_up_or_down = TRUE;

			::CheckMenuItem(menu_sortord.m_hMenu,1101,MF_CHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1102,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1103,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1104,MF_UNCHECKED);
		}
		else if(nID==1102)
		{//nID==1101对应menu_playlist_sortord菜单的第二项
			SWindow* pBtn = FindChildByName(L"btn_sortord");
		//	if(pBtn) pBtn->SetWindowText (L"名称");

			pBtn = FindChildByName(L"sortord_down");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"sortord_up");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			up_or_down = 0;
			popularity_up_or_down = FALSE;

			::CheckMenuItem(menu_sortord.m_hMenu,1101,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1102,MF_CHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1103,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1104,MF_UNCHECKED);
		}
		else if(nID==1103)
		{//nID==1103对应menu_playlist_sortord菜单的第三项
			SWindow* pBtn = FindChildByName(L"btn_sortord");
		//	if(pBtn) pBtn->SetWindowText (L"评分");

			pBtn = FindChildByName(L"sortord_down");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"sortord_up");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			up_or_down = 0;
			popularity_up_or_down = FALSE;

			::CheckMenuItem(menu_sortord.m_hMenu,1101,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1102,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1103,MF_CHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1104,MF_UNCHECKED);
		}
		else if(nID==1104)
		{//nID==1104对应menu_playlist_sortord菜单的第四项
			SWindow* pBtn = FindChildByName(L"btn_sortord");
		//	if(pBtn) pBtn->SetWindowText (L"评论");

			pBtn = FindChildByName(L"sortord_down");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"sortord_up");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			up_or_down = 0;
			popularity_up_or_down = FALSE;

			::CheckMenuItem(menu_sortord.m_hMenu,1101,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1102,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1103,MF_UNCHECKED);
			::CheckMenuItem(menu_sortord.m_hMenu,1104,MF_CHECKED);
		}

		else if(nID==11101)
		{//nID==11101对应menu_PlayMode菜单的第一项
			SWindow* pBtn = FindChildByName(L"btn_OrderPlay");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"btn_SinglePlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_RandomPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SingleCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_ListCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);


			::CheckMenuItem(menu_PlayMode.m_hMenu,11101,MF_CHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11102,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11103,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11104,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11105,MF_UNCHECKED);
		}
		else if(nID==11102)
		{//nID==11102对应menu_PlayMode菜单的第二项
			SWindow* pBtn = FindChildByName(L"btn_OrderPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SinglePlay");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"btn_RandomPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SingleCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_ListCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);


			::CheckMenuItem(menu_PlayMode.m_hMenu,11101,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11102,MF_CHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11103,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11104,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11105,MF_UNCHECKED);
		}
		else if(nID==11103)
		{//nID==11103对应menu_PlayMode菜单的第三项
			SWindow* pBtn = FindChildByName(L"btn_OrderPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SinglePlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_RandomPlay");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"btn_SingleCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_ListCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);


			::CheckMenuItem(menu_PlayMode.m_hMenu,11101,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11102,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11103,MF_CHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11104,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11105,MF_UNCHECKED);
		}
		else if(nID==11104)
		{//nID==11104对应menu_PlayMode菜单的第四项
			SWindow* pBtn = FindChildByName(L"btn_OrderPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SinglePlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_RandomPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SingleCycle");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);

			pBtn = FindChildByName(L"btn_ListCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);


			::CheckMenuItem(menu_PlayMode.m_hMenu,11101,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11102,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11103,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11104,MF_CHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11105,MF_UNCHECKED);
		}
		else if(nID==11105)
		{//nID==11105对应menu_PlayMode菜单的第五项
			SWindow* pBtn = FindChildByName(L"btn_OrderPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SinglePlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_RandomPlay");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_SingleCycle");
			if(pBtn) pBtn->SetVisible(FALSE,TRUE);

			pBtn = FindChildByName(L"btn_ListCycle");
			if(pBtn) pBtn->SetVisible(TRUE,TRUE);


			::CheckMenuItem(menu_PlayMode.m_hMenu,11101,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11102,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11103,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11104,MF_UNCHECKED);
			::CheckMenuItem(menu_PlayMode.m_hMenu,11105,MF_CHECKED);
		}
	}
}