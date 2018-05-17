// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "RealWndDlg.h"
#include "PlayListAdapter.h"
#include "FilesHelp.h"
#include "..\controls.extend\SRadioBox2.h"	
#include <shlobj.h>
#include <shellapi.h>
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif

const TCHAR STR_MOVE_FILE_FILTER[] =
	_T("媒体文件(所有类型)\0*.asf;*.avi;*.wm;*.wmp;*.wmv;*.ram; *.rm; *.rmvb; *.rp; *.rpm; *.rt; *.smi; *.smil;*.dat; *.m1v; *.m2p; *.m2t; *.m2ts; *.m2v; *.mp2v; *.mpeg; *.mpe; *.mpg; *.mpv2; *.pss; *.pva; *.tp; *.tpr; *.ts;*.m4b; *.m4p; *.m4v; *.mp4; *.mpeg4; *.mov; *.qt; *.f4v; *.flv; *.hlv; *.swf; *.ifo; *.vob;*.3g2; *.3gp; *.3gp2; *.3gpp; *.amv; *.bik; *.csf; *.divx; *.evo; *.ivm; *.mkv; *.mod; *.mts; *.ogm; *.pmp; *.scm; *.tod; *.vp6; *.webm; *.xlmv;*.aac; *.ac3; *.amr; *.ape; *.cda; *.dts; *.flac; *.mla; *.m2a; *.m4a; *.mid; *.midi; *.mka; *.mp2; *.mp3; *.mpa; *.ogg; *.ra; *.tak; *.tta; *.wav; *.wma; *.wv;\0")
	_T("window媒体(*.asf; *.avi; *.wm; *.wmp; *.wmv;)\0*.asf; *.avi; *.wm; *.wmp; *.wmv;\0")
	_T("real媒体(*.ram;*.rm;*.rmvb;*.rp;*.rpm;*.rt;*.smi;*.smil;)\0*.ram;*.rm;*.rmvb;*.rp;*.rpm;*.rt;*.smi;*.smil;\0")
	_T("MPEG1/2媒体(*.dat;*.m1v;*.m2p;*.m2t;*.m2ts;*.m2v;*.mp2v;*.mpeg;*.mpe;*.mpg;*.mpv2;*.pss;*.pva;*.tp;*.tpr;*.ts;)\0*.dat; *.m1v; *.m2p; *.m2t; *.m2ts; *.m2v; *.mp2v; *.mpeg; *.mpe; *.mpg; *.mpv2; *.pss; *.pva; *.tp; *.tpr; *.ts;\0")
	_T("MPEG4媒体(*.m4b;*.m4p;*.m4v;*.mp4;*.mpeg4;)\0*.m4b;*.m4p;*.m4v;*.mp4;*.mpeg4;\0")
	_T("3GPP媒体(*.3g2;*.3gp;*.3gp2;*.3gpp;)\0*.3g2;*.3gp;*.3gp2;*.3gpp;\0")
	_T("APPLE媒体(*.mov;*.qt;)\0*.mov;*.qt;\0")
	_T("Flash媒体(*.f4v;*.flv;*.hlv;*.swf;)\0*.f4v;*.flv;*.hlv;*.swf;\0")
	_T("DVD媒体(*.ifo;*.vob;)\0*.ifo;*.vob;\0")
	_T("其它视频文件(*.amv;*.bik;*.csf;*.*.divx;*.evo;*.ivm;*.mkv;*.mod;*.mts;*.ogm;*.pmp;*.scm;*.tod;*.vp6;*.webm;*.xlmv;)\0*.amv;*.bik;*.csf;*.*.divx;*.evo;*.ivm;*.mkv;*.mod;*.mts;*.ogm;*.pmp;*.scm;*.tod;*.vp6;*.webm;*.xlmv;\0")
	_T("其它音频文件(*.aac;*.ac3;*.amr;*.ape;*.cda;*.dts;*.flac;*.mla;*.m2a;*.m4a;*.mid;*.midi;*.mka;*.mp2;*.mp3;*.mpa;*.ogg;*.ra;*.tak;*.tta;*.wav;*.wma;*.wv;)\0*.aac;*.ac3;*.amr;*.ape;*.cda;*.dts;*.flac;*.mla;*.m2a;*.m4a;*.mid;*.midi;*.mka;*.mp2;*.mp3;*.mpa;*.ogg;*.ra;*.tak;*.tta;*.wav;*.wma;*.wv;\0");


const TCHAR STR_SUPPORT_FILE_EXT[] =
	_T("*.asf;*.avi;*.wm;*.wmp;*.wmv;*.ram;*.rm;*.rmvb;*.rp;*.rpm;*.rt;*.smi;*.smil;*.dat;*.m1v;*.m2p;*.m2t;*.m2ts;*.m2v;*.mp2v;*.mpeg;*.mpe;*.mpg;*.mpv2;*.pss;*.pva;*.tp;*.tpr;*.ts;*.m4b;*.m4p;*.m4v;*.mp4;*.mpeg4;*.mov;*.qt;*.f4v;*.flv;*.hlv;*.swf;*.ifo;*.vob;*.3g2;*.3gp;*.3gp2;*.3gpp;*.amv;*.bik;*.csf;*.divx;*.evo;*.ivm;*.mkv;*.mod;*.mts; *.ogm; *.pmp; *.scm; *.tod; *.vp6; *.webm; *.xlmv;*.aac; *.ac3; *.amr; *.ape; *.cda; *.dts; *.flac; *.mla; *.m2a; *.m4a; *.mid; *.midi; *.mka; *.mp2; *.mp3; *.mpa; *.ogg; *.ra; *.tak; *.tta; *.wav; *.wma; *.wv;");



//增加文件结构体
struct Thread_add
{
	SListView* plist;
	vector<SStringT> files;
	HWND hwnd;
};
//增加文件线程提高UI响应速度
static DWORD WINAPI threadadd(LPVOID lpParameter)
{
	struct Thread_add* prama1 = (struct Thread_add *)lpParameter;
	SListView  *mclist = prama1->plist;
	vector<SStringT> pfiles = prama1->files;
	HWND phwnd = prama1->hwnd;
	delete prama1;
	CPlayListWnd *pAdapter = (CPlayListWnd*)mclist->GetAdapter();
	for (vector<SStringT>::iterator it = pfiles.begin(); it != pfiles.end(); ++it)
	{
		SStringT  path_name = *it;
		pAdapter->ADD_files(path_name);
	}
	::PostMessage(prama1->hwnd,MS_ADD_FILESED,0,0);

	return 0;
}
//文件文件夹拖入
class CTestDropTarget1 : public CTestDropTarget
{
protected:
	SWindow *m_pmanwindow;
	HWND m_hwnd;
	SListView  *mclist;

public:
	CTestDropTarget1(SWindow *pwindow, HWND hwnd, SListView *pmclist) :m_pmanwindow(pwindow), m_hwnd(hwnd), mclist(pmclist)
	{
		if (m_pmanwindow) m_pmanwindow->AddRef();
	}
	~CTestDropTarget1()
	{
		if (m_pmanwindow) m_pmanwindow->Release();
	}
public:
	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		FORMATETC format =
		{
			CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
		};
		STGMEDIUM medium;
		if (FAILED(pDataObj->GetData(&format, &medium)))
		{
			return S_FALSE;
		}

		HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

		if (!hdrop)
		{
			return S_FALSE;
		}

		//////////////////////////////////////////////////////////////////////////
		UINT nFileCount = ::DragQueryFile(hdrop, -1, NULL, 0);

		TCHAR szFileName[_MAX_PATH] = _T("");
		DWORD dwAttribute;
		std::vector<SStringT> vctString;

		// 获取拖拽进来文件和文件夹
		for (UINT i = 0; i < nFileCount; i++)
		{
			::DragQueryFile(hdrop, i, szFileName, sizeof(szFileName));
			dwAttribute = ::GetFileAttributes(szFileName);

			// 是否为文件夹
			if (dwAttribute & FILE_ATTRIBUTE_DIRECTORY)
			{
				::SetCurrentDirectory(szFileName);
				CFileHelp::EnumerateFiles(vctString, STR_SUPPORT_FILE_EXT);
			}
			else
			{
				if (CFileHelp::FindFileExt(szFileName, STR_SUPPORT_FILE_EXT))
					vctString.push_back(szFileName);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		GlobalUnlock(medium.hGlobal);
		if (m_pmanwindow)
		{			
			struct Thread_add *prama1 = new Thread_add;
			prama1->files = vctString;
			prama1->plist = mclist;
			prama1->hwnd = m_hwnd;
			HANDLE hThread = CreateThread(NULL, 0, &threadadd, (LPVOID)prama1, 0, 0);
		}
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
};
//////////////////////////////////////////////////////////////////////////
HWND gPlayHwnd = NULL;
//枚举VLC窗口
BOOL CALLBACK EnumerateVLC(HWND hWndvlc, LPARAM lParam)
{
	TCHAR szWndTitle[1024];
	int nLen = GetWindowText(hWndvlc, szWndTitle, 1024);
	if (0 != nLen)
	{
		//禁用鼠标消息
		EnableWindow(hWndvlc, FALSE);
	}
	KillTimer(NULL, 1);
	return TRUE;
}
//定时器回调
void CALLBACK TimeProc(
	HWND hwnd,
	UINT message,
	UINT_PTR idTimer,
	DWORD dwTime)
{
	EnumChildWindows(gPlayHwnd, EnumerateVLC, NULL);
}
int m_LButtonDown = 0;//鼠标按下标记
std::string UnicodeConvert(const std::wstring& strWide, UINT uCodePage)
{
	std::string strANSI;
	int iLen = ::WideCharToMultiByte(uCodePage, 0, strWide.c_str(), -1, NULL, 0, NULL, NULL);

	if (iLen > 1)
	{
		strANSI.resize(iLen - 1);
		::WideCharToMultiByte(uCodePage, 0, strWide.c_str(), -1, &strANSI[0], iLen, NULL, NULL);
	}

	return strANSI;
}
std::string UnicodeToUTF8(const std::wstring& strWide)
{
	return UnicodeConvert(strWide, CP_UTF8);
}
void CallbackPlayer(void *data, UINT uMsg)
{
	CAVPlayer *pAVPlayer = (CAVPlayer *)data;

	if (pAVPlayer)
	{
		HWND hWnd = pAVPlayer->GetHWND();

		if (::IsWindow(hWnd) && ::IsWindow(::GetParent(hWnd)))
		{
			::PostMessage(::GetParent(hWnd), uMsg, (WPARAM)data, 0);
		}
	}
}
void CallbackPlaying(void *data)
{
	CallbackPlayer(data, WM_USER_PLAYING);
}

void CallbackPosChanged(void *data)
{
	CallbackPlayer(data, WM_USER_POS_CHANGED);
}

void CallbackEndReached(void *data)
{
	CallbackPlayer(data, WM_USER_END_REACHED);
}
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND")),
	m_bFullScreenMode(FALSE),
	m_iPlaylistIndex(-1),
	m_bPlayList_ShowWnd(FALSE),
	m_Exit_Save_HistroyList(false),
	m_emPlayMode(EM_PLAY_MODE_SEQUENCE)
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
	using namespace pugi;
	xml_document doc;
	doc.append_child(L"历史播放");
	for (set<SStringT>::iterator it= Hstory_List.begin();it!= Hstory_List.end();it++)
	{
		SStringT _path = *it;
		xml_node node = doc.child(L"历史播放").append_child(L"id");
		node.text().set(_path);
	}
	doc.append_child(L"播放痕迹");
	doc.child(L"播放痕迹").text().set(m_Exit_Save_HistroyList);
	doc.save_file(config_file);
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}
LRESULT CMainDlg::OnPlaying(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	SWindow *slierWnd = FindChildByID(121);
	if (slierWnd)
	{
		slierWnd->EnableWindow(TRUE,TRUE);
	}
	return TRUE;
}

LRESULT CMainDlg::OnPosChanged(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	SStringT  strTime;
	struct tm   tmTotal, tmCurrent;
	time_t      timeTotal = m_cAVPlayer.GetTotalTime() / 1000;
	time_t      timeCurrent = m_cAVPlayer.GetTime() / 1000;
	TCHAR       szTotal[MAX_PATH], szCurrent[MAX_PATH];

	gmtime_s(&tmTotal, &timeTotal);
	gmtime_s(&tmCurrent, &timeCurrent);
	_tcsftime(szTotal, MAX_PATH, _T("%X"), &tmTotal);
	_tcsftime(szCurrent, MAX_PATH, _T("%X"), &tmCurrent);
	strTime.Format(_T("[%s / %s]"), szCurrent, szTotal);
	FindChildByID(6000)->SetWindowTextW(strTime);
	if (m_LButtonDown == 0)
		m_Sliderbarpos->SetValue(m_cAVPlayer.GetPos());
	 g_pTaskbar->SetProgressValue(m_hWnd,m_cAVPlayer.GetPos(), 1000);//更新任务栏
	return TRUE;
}
//播放完成
LRESULT CMainDlg::OnEndReached(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	OnBtnStop();
	CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
	if(pAdapter->getCount()<0) return FALSE;
	Play(playNext(true));
	return TRUE;
}
bool CMainDlg::LoadhistoryXml()
{
	pugi::xml_document doc;
	if (doc.load_file(config_file))
	{
		pugi::xml_node histroy = doc.child(L"播放痕迹");
		if (histroy)
		{
			m_Exit_Save_HistroyList = histroy.text().as_bool();
		}
		if (m_Exit_Save_HistroyList==false) return false;
		pugi::xml_node form = doc.child(L"历史播放");
		if(!form) return false;

		for (pugi::xml_node input = form.first_child(); input; input = input.next_sibling())
		{
			if (CFileHelp::CheckFileExist(input.child_value()))
			{
				if(Hstory_List.size()<30)//这里取30个历史文件
					Hstory_List.insert(input.child_value());
				else  break;
			}

		}
		if(Hstory_List.empty()) return false;
		CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
		for (set<SStringT>::iterator it=Hstory_List.begin(); it!=Hstory_List.end();it++)
		{
			SStringT _path = *it;
			pAdapter->ADD_files(_path);
		}
		pAdapter->Release();
		return true;
	}
	return false;
}
BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	m_Sliderbarpos = FindChildByName2<SSliderBar>(L"sliderbarpos");
	if (m_Sliderbarpos)
	{
		m_Sliderbarpos->SetRange(0, 1000);
	}

	m_VolumeSlider = FindChildByName2<SSliderBar>(L"volumeSlider");
	if (m_VolumeSlider)
	{
		m_cAVPlayer.Volume(m_VolumeSlider->GetValue());

	}
	SRealWnd   *p_RealWnd = FindChildByName2<SRealWnd>(L"vlcwnd");
	if (p_RealWnd)
	{
		gPlayHwnd = p_RealWnd->GetRealHwnd();
		m_cAVPlayer.SetHWND(gPlayHwnd);
		m_cAVPlayer.SetCallbackPlaying(CallbackPlaying);
		m_cAVPlayer.SetCallbackPosChanged(CallbackPosChanged);
		m_cAVPlayer.SetCallbackEndReached(CallbackEndReached);

	}

	CSimpleWnd::SetWindowTextW(L"青蛙看看播放器");

	m_Play_List_Wnd = FindChildByName2<SListView>(L"lv_play_list");
	if (m_Play_List_Wnd)
	{
		HRESULT hr = ::RegisterDragDrop(m_hWnd, GetDropTarget());
		RegisterDragDrop(m_Play_List_Wnd->GetSwnd(), new CTestDropTarget1(m_Play_List_Wnd, m_hWnd, m_Play_List_Wnd));//注册拖动
		CPlayListWnd *pTvAdapter = new CPlayListWnd(m_hWnd);
		m_Play_List_Wnd->SetAdapter(pTvAdapter);
		pTvAdapter->Release();
	}

	config_file.Format(L"%s\\history.xml", SApplication::getSingleton().GetAppDir());
	if (LoadhistoryXml())//加载历史列表
	{
		FindChildByID(123)->SetVisible(true, true);
	}
	//注册用户消息win7任务栏
		WM_TASKBARBUTTONCREATED = ::RegisterWindowMessage(TEXT("TaskbarButtonCreated"));
	return 0;
}


//TODO:消息映射
void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}
void CMainDlg::OnTabPageRadioSwitch(EventArgs *pEvt)
{
	EventSwndStateChanged *pEvt2 = sobj_cast<EventSwndStateChanged>(pEvt);
	if (pEvt2->CheckState(WndState_Check) && (pEvt2->dwNewState & WndState_Check))
	{
		int id = pEvt->idFrom;
		STabCtrl *pTab = FindChildByName2<STabCtrl>(L"maintab");
		if (pTab) pTab->SetCurSel(id - 10000);
		if (id!=10000)
		{
			FindChildByID(121)->SetVisible(FALSE, TRUE);
		}else
			FindChildByID(121)->SetVisible(TRUE, TRUE);
	}
}
void CMainDlg::OnLButtonUp(UINT nFlags, CPoint pt)//处理进度条鼠标事件 播放指定位置 此处没有用EventSliderPos事件
{
	if (m_Sliderbarpos->GetWindowRect().PtInRect(pt) && m_LButtonDown == 1)
	{		
		m_cAVPlayer.SeekTo(m_Sliderbarpos->GetValue() + 1);		
	}
	SWindow   *m_realplay_wnd = FindChildByName(L"realplay_wnd");
	if (m_realplay_wnd)
	{
		if (m_realplay_wnd->GetWindowRect().PtInRect(pt))//单击播放/暂停
		{
			if (!m_cAVPlayer.IsOpen()) return;
			if (m_cAVPlayer.IsPlaying())
			{
				m_cAVPlayer.Pause();
				FindChildByID2<SImageButton>(201)->SetVisible(false, true);
				FindChildByID2<SImageButton>(200)->SetVisible(true, true);
			}
			else
			{
				m_cAVPlayer.Play();
				FindChildByID2<SImageButton>(200)->SetVisible(false, true);
				FindChildByID2<SImageButton>(201)->SetVisible(true, true);
			}
		}
	}
	if (m_VolumeSlider->GetWindowRect().PtInRect(pt) && m_LButtonDown == 1)
	{
		FindChildByID2<SStatic>(3500)->SetWindowTextW(L"");
	}
	m_LButtonDown = 0;
	SetMsgHandled(false);


}
void CMainDlg::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_VolumeSlider->GetWindowRect().PtInRect(pt) && m_LButtonDown == 1)
	{
		int pos = m_VolumeSlider->GetValue();
		SStatic   *m_tip = FindChildByID2<SStatic>(3500);
		m_tip->SetWindowTextW(SStringT().Format(L"%02d%%", pos));
		m_tip->SetAttribute(L"pos",SStringT().Format(L"%d,0",pt.x-10));
		m_cAVPlayer.Volume(pos);
	}
	SetMsgHandled(false);
}
void CMainDlg::OnLButtonDown(UINT nFlags, CPoint pt)//处理进度条鼠标事件 播放指定位置
{
	if (m_Sliderbarpos->GetWindowRect().PtInRect(pt))//处理进度条
		m_LButtonDown = 1;
	if (m_VolumeSlider->GetWindowRect().PtInRect(pt))//处理音量tip
		m_LButtonDown = 1;
	SetMsgHandled(false);


}
//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{

	if (uNotifyCode == 0)
	{
		if (nID == 6)
		{//nID==6对应menu_test定义的菜单的exit项。
			PostMessage(WM_CLOSE);
		}
		else if (nID == 11)//打开文件
		{
			OnBtnOpen();
		}
		else if (nID==12)//打开文件夹
		{
			On_directories();
		}
		else if (nID == 13)//打开URL
		{
			FindChildByID2<SRadioBox2>(10003)->SetCheck(true);
		}
		else if (nID == 1102)//单曲
		{
			m_emPlayMode = EM_PLAY_MODE_SINGLE_CIRCLE;			

		}
		else if (nID == 1101)//顺序
		{
			m_emPlayMode = EM_PLAY_MODE_SEQUENCE;

		}
		else if (nID == 1103)//随机
		{
			m_emPlayMode = EM_PLAY_MODE_RANDOM;

		}
		else if (nID==1106)//清空
		{
			CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
			pAdapter->DELL_ALL();
		}
		else if ( nID == 3000)//清空播放痕迹
		{
			CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
			pAdapter->DELL_ALL();
			Hstory_List.clear();
		}
		else if (nID == 1104)//名字排序
		{
			CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
			pAdapter->Sort_Play_list(1);
		}
		else if (nID == 1105)//类型排序
		{
			CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
			pAdapter->Sort_Play_list(2);
		}
		else if (nID==3001)
		{
			m_Exit_Save_HistroyList = false;//退出清除播放痕迹
		}
		else if (nID == 3002)
		{
			m_Exit_Save_HistroyList = true;//退出保留播放痕迹
		}

	}
}
//打开关闭播放列表
void CMainDlg::OnShowPlayList()
{
	SWindow  *p_playlist_window = FindChildByID(123);
	if (!FindChildByID2<SRadioBox2>(10000)->IsChecked())
	{	
		CRect rc = FindChildByID2<SImageButton>(124)->GetWindowRect();
		ClientToScreen(&rc);
		CTipWnd::ShowTip(rc.right, rc.top, CTipWnd::AT_LEFT_BOTTOM, _T("操作错误!\\n只能在【播放】窗口下才能显示列表"));
		return;
	}
	p_playlist_window->SetVisible(!p_playlist_window->IsVisible(), true);

}
void CMainDlg::Play(bool bPlay)                                         // 播放或暂停
{
	if (m_cAVPlayer.IsOpen())
	{
		SImageButton *pbtnPlay = FindChildByID2<SImageButton>(200);
		SImageButton *pbtnPause = FindChildByID2<SImageButton>(201);
		if (pbtnPlay && pbtnPause)
		{
			pbtnPlay->SetVisible(!bPlay);
			pbtnPause->SetVisible(bPlay);
		}

		if (bPlay)
		{
			m_cAVPlayer.Play();
		}
		else
		{
			m_cAVPlayer.Pause();
		}
	}
}
void CMainDlg::Play(LPCTSTR pszPath)
{
	if (!pszPath)
	{
		return;
	}

	m_strPath = pszPath;

	if (m_cAVPlayer.Play(UnicodeToUTF8(pszPath)))
	{
		FindChildByID(6000)->SetWindowTextW(L"");
		m_Sliderbarpos->SetValue(0);
		SImageButton *pbtnPlay = FindChildByID2<SImageButton>(200);
		SImageButton *pbtnPause = FindChildByID2<SImageButton>(201);
		if (pbtnPlay && pbtnPause)
		{
			pbtnPlay->SetVisible(FALSE);
			pbtnPause->SetVisible(TRUE);
		}
		SStringT name, exname;
		CFileHelp::SplitPathFileName(m_strPath, name, exname);
		CSimpleWnd::SetWindowTextW(name);
		::SetTimer(NULL, 1, 1000, TimeProc);
		Hstory_List.insert(m_strPath);
	}
}
void CMainDlg::OnBtnPlay()	//播放
{
	Play(true);
}

void CMainDlg::OnBtnPause()	//暂停
{
	Play(false);
}
void CMainDlg::OnBtnOpen()	//打开文件
{
	vector<SStringT> names;	
	CFileHelp::OpenFile(STR_MOVE_FILE_FILTER, GetHostHwnd(), names);
	if (names.empty()) return;
	struct Thread_add *prama1 = new Thread_add;
	prama1->files = names;
	prama1->plist = m_Play_List_Wnd;
	prama1->hwnd = m_hWnd;
	HANDLE hThread = CreateThread(NULL, 0, &threadadd, (LPVOID)prama1, 0, 0);
	SWindow   *playlist_wnd = FindChildByID(123);
	if (!playlist_wnd->IsVisible())
	{
		playlist_wnd->SetVisible(true, true);
	}
}
void CMainDlg::OnBtnStop()	// 停止
{
	m_cAVPlayer.Stop();
	FindChildByID2<SImageButton>(200)->SetVisible(true, true);
	FindChildByID2<SImageButton>(201)->SetVisible(false, true);
	m_Sliderbarpos->SetValue(0);
	FindChildByID(6000)->SetWindowTextW(L"");
	CSimpleWnd::SetWindowTextW(L"青蛙看看播放器");
	SWindow *slierWnd = FindChildByID(121);
	if (slierWnd)
	{
		slierWnd->EnableWindow(FALSE,TRUE);
	}
	g_pTaskbar->SetProgressValue(m_hWnd, 0, 1000);
}
void CMainDlg::OnSeekBackwardBtn()
{

	if (m_cAVPlayer.IsOpen())
	{
		m_cAVPlayer.SeekForward();
	}
}
void CMainDlg::OnSeekForwardBtn()
{
	if (m_cAVPlayer.IsOpen())
	{
		m_cAVPlayer.SeekBackward();
	}
}
void CMainDlg::OnBtnVolum_mute()	//静音
{
	SImageButton   *m_volumbtn = FindChildByID2<SImageButton>(206);	
	bool is_Mute = (bool)m_volumbtn->GetUserData();
	if (is_Mute)
	{
		m_volumbtn->SetAttribute(L"skin", L"image_btn_volume_zero_png");
		m_VolumeSlider->SetUserData(m_VolumeSlider->GetValue());
		m_VolumeSlider->SetValue(0);
		m_volumbtn->SetUserData(!(bool)is_Mute);
		m_cAVPlayer.Volume(0);
	}
	else
	{
		m_volumbtn->SetAttribute(L"skin", L"image_btn_volume_png");
		m_VolumeSlider->SetValue(m_VolumeSlider->GetUserData());
		m_cAVPlayer.Volume((int)m_VolumeSlider->GetUserData());
		m_volumbtn->SetUserData(!(bool)is_Mute);
	}

}
//打开播放模式菜单
void CMainDlg::OnOpenPlayListLoop_Menu()
{
	SMenu m_playlistLoop;
	m_playlistLoop.LoadMenu(_T("menu_playlistloop"), _T("LAYOUT"));
	CRect rc_menu;
	SWindow * pBtn = FindChildByID(300);
	if (pBtn)
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);
		switch (m_emPlayMode)
		{
		case EM_PLAY_MODE_SEQUENCE:
			{
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1101, MF_CHECKED);
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1102, MF_UNCHECKED);
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1103, MF_UNCHECKED);
			}break;
		case EM_PLAY_MODE_SINGLE_CIRCLE:
			{
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1101, MF_UNCHECKED);
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1102, MF_CHECKED);
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1103, MF_UNCHECKED);
			}break;
		case EM_PLAY_MODE_RANDOM:
			{
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1101, MF_UNCHECKED);
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1102, MF_UNCHECKED);
				::CheckMenuItem(m_playlistLoop.m_hMenu, 1103, MF_CHECKED);

			}break;

		default:
			break;
		}

		m_playlistLoop.TrackPopupMenu(0, rc_menu.left, rc_menu.bottom, m_hWnd);
	}
}
//打开主菜单 
void CMainDlg::OnOpenMainBtn_Menu()
{
	SMenu m_main_menu;
	m_main_menu.LoadMenu(_T("menu_main"), _T("LAYOUT"));
	CRect rc_menu;
	SWindow * pBtn = FindChildByID(501);
	if (pBtn)
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);
		m_main_menu.TrackPopupMenu(0, rc_menu.left, rc_menu.bottom, m_hWnd);
	}
}
void CMainDlg::FullScreen(bool bFull)
{
	int iBorderX = GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXBORDER);
	int iBorderY = GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYBORDER);
	m_bFullScreenMode = bFull;

	if (bFull)
	{
		::GetWindowPlacement(m_hWnd, &m_OldWndPlacement);

		if (::IsZoomed(m_hWnd))
		{
			::ShowWindow(m_hWnd, SW_SHOWDEFAULT);
		}

		::SetWindowPos(m_hWnd, HWND_TOPMOST, -iBorderX, -iBorderY, GetSystemMetrics(SM_CXSCREEN) + 2 * iBorderX, GetSystemMetrics(SM_CYSCREEN) + 2 * iBorderY, 0);
		SCaption  *cap = FindChildByID2<SCaption>(7000);
		cap->SetVisible(FALSE, TRUE);
		SWindow *m_playlistwnd = FindChildByID(123);
		m_playlistwnd->IsVisible()?m_bPlayList_ShowWnd=true:m_bPlayList_ShowWnd=false;
		m_playlistwnd->SetVisible(FALSE, TRUE);
		m_playlistwnd = FindChildByID(121);
		m_playlistwnd->SetVisible(FALSE, TRUE);
		cap = FindChildByID2<SCaption>(122);
		cap->SetVisible(FALSE, TRUE);

	}
	else
	{
		::SetWindowPlacement(m_hWnd, &m_OldWndPlacement);
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SCaption  *cap = FindChildByID2<SCaption>(7000);
		cap->SetVisible(TRUE, TRUE);
		SWindow *m_playlistwnd = FindChildByID(123);
		if(m_bPlayList_ShowWnd)
			m_playlistwnd->SetVisible(TRUE, TRUE);
		m_playlistwnd = FindChildByID(121);
		m_playlistwnd->SetVisible(TRUE, TRUE);
		cap = FindChildByID2<SCaption>(122);
		cap->SetVisible(TRUE, TRUE);
	}
}
void CMainDlg::OnFullScreenBtn()
{
	if (!FindChildByID2<SRadioBox2>(10000)->IsChecked())
	{
		CRect rc = FindChildByID2<SImageButton>(600)->GetWindowRect();
		ClientToScreen(&rc);
		CTipWnd::ShowTip(rc.right, rc.top, CTipWnd::AT_LEFT_BOTTOM, _T("错误的操作!\\n必须在【播放】的窗口下才能全屏"));
		return;
	}
	if(!m_bFullScreenMode)
		FullScreen(true);
}
//真窗口的消息处理
LRESULT CMainDlg::OnMyMsg_REALWND(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	int id = (int)lp;
	if (id==1)//打开文件
	{
		OnBtnOpen();
	}
	else if (id==2)//打开文件夹
	{
		On_directories();
	}
	else if(id==10)//真窗口VK_ESCAPE
	{
		if (m_bFullScreenMode)
		{
			FullScreen(false);
		}
	}else if (id==11)//空格暂停/播放
	{
		if (m_cAVPlayer.IsPlaying())
		{

			FindChildByID2<SImageButton>(200)->SetVisible(true, true);
			FindChildByID2<SImageButton>(201)->SetVisible(false, true);
			Play(false);

		}
		else
		{
			FindChildByID2<SImageButton>(201)->SetVisible(true, true);
			FindChildByID2<SImageButton>(200)->SetVisible(false, true);
			Play(true);
		}

	}

	else if (id==12)//双击全屏/退出
	{
		FullScreen(!m_bFullScreenMode);
	}
	else if (id==14)//单击播放/暂停
	{
		if (m_cAVPlayer.IsOpen())
		{
			if (m_cAVPlayer.IsPlaying())
			{
				m_cAVPlayer.Pause();
				FindChildByID2<SImageButton>(201)->SetVisible(false, true);
				FindChildByID2<SImageButton>(200)->SetVisible(true, true);
			}
			else
			{
				m_cAVPlayer.Play();
				FindChildByID2<SImageButton>(200)->SetVisible(false, true);
				FindChildByID2<SImageButton>(201)->SetVisible(true, true);
			}
		}

	}
	else if (id==15)//快退
	{
		OnSeekForwardBtn();
	}
	else if (id==16)//快进
	{	
		OnSeekBackwardBtn();
	}
	return 0;
}
//双击列表通知播放文件
LRESULT CMainDlg::OnMyMsg_PLAY_FILE(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)
{
	Play((LPCTSTR)lp);
	return 0;
}
void CMainDlg::On_directories() //打开目录
{
	//打开目录下的所有媒体文件
	vector<SStringT> names;
	SStringT path;
	if (CFileHelp::BrowseDir(path, GetHostHwnd(), L"打开媒体目录"))
	{
		::SetCurrentDirectory(path);
		CFileHelp::EnumerateFiles(names, STR_SUPPORT_FILE_EXT);
		if (names.empty()) return;
		struct Thread_add *prama1 = new Thread_add;
		prama1->files = names;
		prama1->plist = m_Play_List_Wnd;
		prama1->hwnd = m_hWnd;
		HANDLE hThread = CreateThread(NULL, 0, &threadadd, (LPVOID)prama1, 0, 0);

	}



}
LRESULT CMainDlg::OnMyMsg_ADD_FILED(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)//增加文件完成后的通知 如果有文件则显示列表
{
	CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
	if (pAdapter->getCount()>0)
	{
		SWindow   *playlist_wnd = FindChildByID(123);
		if (!playlist_wnd->IsVisible())
		{
			playlist_wnd->SetVisible(true, true);
		}
	}
	pAdapter->notifyDataSetChanged();
	return 0;
}

void CMainDlg::OnBtn_Menu_History()//打开播放痕迹菜单
{
	SMenu m_main_menu;
	m_main_menu.LoadMenu(_T("menu_history"), _T("LAYOUT"));
	CRect rc_menu;
	SImageButton * pBtn = FindChildByID2<SImageButton>(250);
	if (pBtn)
	{
		pBtn->GetClientRect(&rc_menu);
		ClientToScreen(&rc_menu);
		if (m_Exit_Save_HistroyList)
		{
			::CheckMenuItem(m_main_menu.m_hMenu, 3002, MF_CHECKED);
			::CheckMenuItem(m_main_menu.m_hMenu, 3001, MF_UNCHECKED);
		}
		else
		{
			::CheckMenuItem(m_main_menu.m_hMenu, 3001, MF_CHECKED);
			::CheckMenuItem(m_main_menu.m_hMenu, 3002, MF_UNCHECKED);
		}
		m_main_menu.TrackPopupMenu(0, rc_menu.left, rc_menu.bottom, m_hWnd);
	}
}
//删除列表文件
void CMainDlg::Ondellfiles_MenuBtn()
{
	CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
	int m_items=m_Play_List_Wnd->GetSel();
	if (m_items < 0) return;
	if (m_items==pAdapter->getCount()-1&&pAdapter->getCount()>1)
	{
		pAdapter->Del_File(m_items);
		m_Play_List_Wnd->SetSel(m_items - 1);
	}else
		pAdapter->Del_File(m_items);
}
//增加文件
void CMainDlg::OnAddfiles_MenuBtn()
{
	OnBtnOpen();
}
LRESULT CMainDlg::OnMyMsg_REALWND_URLPLAY(UINT uMsg, WPARAM wp, LPARAM lp, BOOL & bHandled)//url播放
{
	Play((LPCTSTR)lp);
	return 0;
}
void CMainDlg::OnBtnPlayPrev()
{
	CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
	if(pAdapter->getCount()<0) return;
	Play(playNext(false));
}
void CMainDlg::OnBtnPlayNext()
{
	CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
	if(pAdapter->getCount()<0) return;
	Play(playNext(true));
}
// 产生uRandNum个不相同的随机数，并添加到queRand末尾
void Rand(std::deque<unsigned int> &queRand, unsigned int uRandNum)
{
	if (uRandNum <= 0)
	{
		return;
	}

	unsigned uSizeOld = queRand.size();
	unsigned uSizeNew = uSizeOld + uRandNum;
	queRand.resize(uSizeNew);
	srand(unsigned(time(NULL)));

	for(unsigned i = uSizeOld; i < uSizeNew; i++)
	{
		queRand[i] = i;
	}

	for(unsigned i = uSizeOld; i < uSizeNew; i++)
	{
		std::swap(queRand[i], queRand[rand() % uSizeNew]);
	}
}
//播放下一个
SStringT CMainDlg::playNext(bool bNext)
{
	CPlayListWnd *pAdapter = (CPlayListWnd*)m_Play_List_Wnd->GetAdapter();
	int iIndexPlay=pAdapter->Get_Play_index();
	int iPlayCount=pAdapter->getCount();
	if (EM_PLAY_MODE_RANDOM == m_emPlayMode)
	{
		if (! m_queRand.size())
		{
			Rand(m_queRand, iPlayCount);
		}

		iIndexPlay = m_queRand.front();
		m_queRand.pop_front();
	} 
	else if (EM_PLAY_MODE_SEQUENCE == m_emPlayMode)
	{
		if (bNext)
		{
			iIndexPlay++;

			if (iIndexPlay >= iPlayCount)
			{
				iIndexPlay = 0;
			} 
		} 
		else
		{
			iIndexPlay--;

			if (iIndexPlay < 0)
			{
				iIndexPlay = iPlayCount-1;
			} 
		}
	}
	m_Play_List_Wnd->SetSel(iIndexPlay);
	pAdapter->Set_Play_index(iIndexPlay);
	return pAdapter->Get_index_Path(iIndexPlay);
}
//抓拍
void CMainDlg::OnBtnSnapshot()
{
	CRect rc = FindChildByID2<SImageButton>(251)->GetWindowRect();
	ClientToScreen(&rc);
	if (m_cAVPlayer.IsOpen())
	{
		SStringT snapshopPath;
		snapshopPath.Format(L"%s\\snapshot.jpg", SApplication::getSingleton().GetAppDir());
		if (0==m_cAVPlayer.Save_snapshot(S_CT2A(snapshopPath)))
		{
			CTipWnd::ShowTip(rc.right, rc.top, CTipWnd::AT_LEFT_BOTTOM, _T("操作成功!\\n保存在本程序目录"));
		}else 
		{
			CTipWnd::ShowTip(rc.right, rc.top, CTipWnd::AT_LEFT_BOTTOM, _T("操作失败!\\n没找到媒体文件"));
		}
	}else
	{
		CTipWnd::ShowTip(rc.right, rc.top, CTipWnd::AT_LEFT_BOTTOM, _T("操作失败!\\n没有打开文件"));
	}		
}
//win7 任务栏消息
LRESULT CMainDlg::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//创建接口对象ITaskbarList
	if(msg == WM_TASKBARBUTTONCREATED)
	{
		::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pTaskbar));
		g_pTaskbar->HrInit();
		g_pTaskbar->SetProgressState(m_hWnd, TBPF_NORMAL);
		g_pTaskbar->SetProgressValue(m_hWnd, 0, 1000);
		//g_pTaskbar->SetOverlayIcon(m_hWnd, g_hPrev, TEXT("Error"));
		g_pTaskbar->SetThumbnailTooltip(m_hWnd, TEXT("青蛙看看播放器"));
	/*	THUMBBUTTONMASK dwMask = THB_ICON | THB_TOOLTIP| THB_FLAGS;
		THUMBBUTTON thbButtons[4];

		thbButtons[0].dwMask = dwMask;
		thbButtons[0].iId = 66;
		thbButtons[0].hIcon = g_hPrev;
		StringCbCopy(thbButtons[0].szTip, sizeof(thbButtons[0].szTip), TEXT("上一首"));
		thbButtons[0].dwFlags = THBF_DISMISSONCLICK;
		thbButtons[1].dwMask = dwMask;
		thbButtons[1].iId = 77;
		thbButtons[1].hIcon = g_hPause;
		StringCbCopy(thbButtons[1].szTip, sizeof(thbButtons[1].szTip), TEXT("暂停"));
		thbButtons[1].dwFlags = THBF_HIDDEN;

		thbButtons[2].dwMask = dwMask;
		thbButtons[2].iId = 88;
		thbButtons[2].hIcon = g_hplay;
		StringCbCopy(thbButtons[2].szTip, sizeof(thbButtons[1].szTip), TEXT("播放"));
		thbButtons[2].dwFlags = THBF_DISMISSONCLICK;	


		thbButtons[3].dwMask = dwMask;
		thbButtons[3].iId = 99;
		thbButtons[3].hIcon = g_hnext;
		StringCbCopy(thbButtons[3].szTip, sizeof(thbButtons[3].szTip), TEXT("下一首"));
		thbButtons[3].dwFlags = THBF_DISMISSONCLICK;
		g_pTaskbar->ThumbBarAddButtons(m_hWnd, ARRAYSIZE(thbButtons), thbButtons);


		*/
	}
	return ::DefWindowProc(hWnd,msg,wParam,lParam);
}