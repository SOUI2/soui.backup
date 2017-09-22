#include "StdAfx.h"
#include "MainWnd.h"
#include "EDialog.hpp"
#include "BeginThread.h"
#include "EtimesTimeFun.hpp"
#include "KeyValueHanlde.hpp"
#include "helper\SMenu.h"
#include "ui\imre\ImgProvider.h"
#include "ui\imre\HTMLParser.h"
#include "ui\imre\RichEditObjEvents.h"

#include "FileFolderHelper.h"

#include "MenuWrapper.h"
#include "helper\SplitString.h"
#include "EFile.hpp"
// 定时器 id 
#define  TIMER_QUEYRHELPER			101

#define RichResendLoadingSkinName		_T("loading16")
#define RichResendWarningSkinName		_T("warning")

SStringT NewGuid()
{
	GUID guid;
	CoCreateGuid(&guid);
	return SStringT().Format(_T("%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"),

		guid.Data1, guid.Data2, guid.Data3,

		guid.Data4[0], guid.Data4[1],

		guid.Data4[2], guid.Data4[3],

		guid.Data4[4], guid.Data4[5],

		guid.Data4[6], guid.Data4[7]);
}


enum RichEditMenuId
{
	MENUID_COPY = 1,
	MENUID_CUT,
	MENUID_PASTE,
	MENUID_SEL_ALL,
	MENUID_SAVE_AS,
	MENUID_OPEN_FILE,
	MENUID_OPEN_FILE_DIR,
	MENUID_COPY_BUBBLE,
	MENUID_CLEAR,
	MENUID_SEND_MSG,
	MENUID_MAKE_CALL,
	MENUID_AT,
	MENUID_SHOW_INFO,
	MENUID_RECALL_MSG,
};

CMainWnd::CMainWnd(void)
	: SHostWnd(_T("layout:wnd_main"))//这里定义主界面需要使用的布局文件 在uires.idx文件中定义的
	, m_uCurrentTalkUserId(0)
	, m_pWndMsgToolTip(NULL)
	, m_AsynNetTask(3)
	, m_pAsynMessageBox(NULL)
{
	
}

#define  DEL_POINT(p)			if(NULL != p){delete p;p=NULL;}		

CMainWnd::~CMainWnd(void)
{
	DEL_POINT(m_pAsynMessageBox)
}

///////////////////////////////////界面 事件  函数///////////////////////////////////////
BOOL CMainWnd::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
{			
#ifdef _DEBUG

#endif

	m_szUserAlias = L"和哈";

	TCHAR lpTempPath[MAX_PATH] = { 0 };

	GetTempPath(MAX_PATH, lpTempPath);
	_tcscat_s(lpTempPath, MAX_PATH, L"TIM_AppData\\");

	theCache.Init(lpTempPath, L"text1");
	
	m_SharkWin.Init(m_hWnd);

	m_pAsynMessageBox = new MessageBoxLayout(this);

#ifdef _DEBUG
	DWORD dwThreadId = GetCurrentThreadId();
#endif		
	

	SetPropW(m_hWnd, _T("40125B5F-6825-4654-8BBF-641EB0AF4F9A"), HANDLE(1705180401));

	TCHAR lpPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, lpPath, MAX_PATH);
	PathRemoveFileSpec(lpPath);
	PathAddBackslash(lpPath);
	m_szUserIconPath = lpPath;
	m_szUserIconPath.Append(_T("user_icon\\"));
	
	//注册 线程事件  到通知中心 
	auto& notifyCenter = SNotifyCenter::getSingleton();
	notifyCenter.addEvent(EVENTID(EventStartInit));
	notifyCenter.subscribeEvent(&CMainWnd::OnCenterEventStartInit, this);

	notifyCenter.addEvent(EVENTID(EventGetUnreadMsg));
	notifyCenter.subscribeEvent(&CMainWnd::OnCenterEventGetUnreadMsg, this);
		
	notifyCenter.addEvent(EVENTID(EventSendMsg));
	notifyCenter.subscribeEvent(&CMainWnd::OnCenterEventSendMsg, this);
		
		
	//SImageWnd* pUserImg = NULL;
	//InitWnd(pUserImg, L"img_myuser");
	
	InitWnd(m_pUnreadCount, L"win_count");
	InitWnd(m_pTabMain, L"tab_main");
	InitWnd(m_pLaySession, L"lay_session");
	InitWnd(m_pLayLoading, L"lay_loading");
	InitWnd(m_pSessionTitle, L"si_text_title");
	InitWnd(m_pSessionLevel, L"si_level");
	InitWnd(m_pChatRecord, L"chat_record");
	InitWnd(m_pChatInput, L"chat_input");
		
	if(NULL != m_pChatRecord)
	{
		m_pChatRecord->GetEventSet()->subscribeEvent(&CMainWnd::OnRecordRichObjEvent, this);
	}

	if (NULL != m_pChatInput)
	{
		DWORD dwEvtMask = m_pChatInput->SSendMessage(EM_GETEVENTMASK);
		m_pChatInput->SSendMessage(EM_SETEVENTMASK, 0, dwEvtMask | ENM_CHANGE);

		m_pChatInput->GetEventSet()->subscribeEvent(&CMainWnd::OnInputRichMenu, this);
		//SUBSCRIBE(m_pChatInput, EVT_RE_NOTIFY, CMainWnd::OnInputEditorChange);
		//SUBSCRIBE(m_pChatInput, EVT_RE_QUERY_ACCEPT, CMainWnd::OnEditorAcceptData);
		//SUBSCRIBE(m_pChatInput, EVT_RE_OBJ, CMainWnd::OnInputRichObjEvent);
	}

	// 最近 联系人 列表
	SListView* pRecentList;
	InitWnd(pRecentList, L"lv_recent");
	
	auto funRecent1 = std::bind(&CMainWnd::CallBack_RecentListItemClick, this, std::placeholders::_1);
	auto funRecent2 = std::bind(&CMainWnd::CallBack_RecentListBtnClick, this, std::placeholders::_1);

	m_pRecentListAdapter = new RecentListAdapter(pRecentList);
	pRecentList->SetAdapter(m_pRecentListAdapter);
	m_pRecentListAdapter->Release();

	m_pRecentListAdapter->SetItemClickCallBack(funRecent1);
	m_pRecentListAdapter->SetBtnClickCallBack(funRecent2);

	// 从 xml 里加载 最近 联系人
	UserList userList;
	theCache.InitRecentTalkInfo(userList);
	for each(auto var in userList)
	{
		m_pRecentListAdapter->Add(var.uId, var.sName, var.sContent);
	}

	// 联系人
	auto funContact1 = std::bind(&CMainWnd::CallBack_ContactTvItemClick, this, std::placeholders::_1);
	auto funContact2 = std::bind(&CMainWnd::CallBack_ContactTvItemDbClick, this, std::placeholders::_1);

	STreeView* pContactTree;
	InitWnd(pContactTree, L"tv_contact");
	m_pContactTreeAdapter = new ContactTreeAdapter(pContactTree);
	pContactTree->SetAdapter(m_pContactTreeAdapter);
	m_pContactTreeAdapter->Release();

	m_pContactTreeAdapter->SetItemClickCallBack(funContact1);
	m_pContactTreeAdapter->SetItemDbClickCallBack(funContact2);

	

#if 0
	// 表情符号 
	STileView* pEmotionView = NULL;
	InitWnd(pEmotionView, L"tv_emotion");
	auto emotionFun = std::bind(&CMainWnd::CallBack_Emotion, this, std::placeholders::_1);
	SStringT szPath = m_szUserIconPath + _T("emoticons\\images\\");
	m_pEmotionTileAdapter = new EmotionTileAdapter(emotionFun, szPath);
	pEmotionView->SetAdapter(m_pEmotionTileAdapter);
	m_pEmotionTileAdapter->Release();
#endif


#ifdef LAYOUT_MODULE
	ShowLoadingPage(FALSE);
	m_pLaySession->SetVisible(TRUE, TRUE);

	m_pSessionTitle->SetWindowText(_T("吴测试"));
	m_pSessionLevel->SetWindowText(_T("非密"));
	
	SwitchPage(1);

#else
	m_AsynNetTask.AddTask(&CMainWnd::ThreadFun_StartInit, this, 0);
#endif
	
	OutputDebugString(_T("OnInitDialog -  Success !"));
	return TRUE;
}

void CMainWnd::OnTimer(UINT_PTR idEvent)
{
	if(TIMER_QUEYRHELPER == idEvent)
	{
		KillTimer(idEvent);
		
		ShowLoadingPage(FALSE);
	}
	else if(10 == idEvent)
	{
		m_Tray.Twinkling();
	}
	else if(11 == idEvent)
	{
		if(m_SharkWin.IsSharkDone())
			KillTimer(idEvent);
		else
			m_SharkWin.OnTimeSharkWin();
	}
	else
		__super::OnTimer(idEvent);
}

void CMainWnd::OnClickRadioTalk()
{
	m_pTabMain->SetCurSel(0);
}

void CMainWnd::OnClickRadioContact()
{
	m_pTabMain->SetCurSel(1);
}

void CMainWnd::OnClickRadioNotice()
{
	m_pTabMain->SetCurSel(2);
}

void CMainWnd::OnBtnSendMsg()
{	
	CHARRANGE chr = { 0, -1 };
	SStringW sContent = m_pChatInput->GetSelectedContent(&chr);
	pugi::xml_document  doc;

	if (!doc.load_buffer(sContent, sContent.GetLength() * sizeof(WCHAR)))
	{
		return;
	}
	sContent.Empty();

	SStringT szSendMsg;
	pugi::xml_node node = doc.child(L"RichEditContent").first_child();
	if(!node)   // 两种 必须 要有一个
	{
		return ;
	}
	for (; node; node = node.next_sibling())
	{
		const wchar_t* pNodeName = node.name();

		if (wcscmp(RichEditText::GetClassName(), pNodeName) == 0)
		{
			sContent += RichEditText::MakeFormatedText(node.text().get(), node.attribute(L"file-size").as_int(10));
			szSendMsg.Append(node.text().get());
		}
		else if (wcscmp(RichEditReminderOle::GetClassName(), pNodeName) == 0)
		{
			sContent += RichEditReminderOle::MakeFormattedText(node.text().get(), 13, RGB(00, 0x6e, 0xfe));
		}
		else if (wcscmp(RichEditImageOle::GetClassName(), pNodeName) == 0)
		{
			int nType = node.attribute(L"type").as_int(0);
			SStringT szPath = node.attribute(L"path").as_string();
			if(1 == nType)		// 表情
			{
				szSendMsg.AppendFormat(_T("<img emoticons=\"%s\" />"), node.attribute(L"encoding").as_string());
				sContent.AppendFormat(L"<img path=\"%s\" />", szPath);
			}
			else
			{
				SStringT szSkin = node.attribute(L"skin").as_string();
				SStringT szTemp = theCache.GetImgCachePath() + szSkin + _T(".png");
				CopyFile(szPath, szTemp, FALSE);

				sContent += RichEditImageOle::MakeFormattedText(
					0,
					L"",
					L"",
					szSkin,
					szTemp,
					L"",
					FALSE);
				
				// 将图片生成base64编码
				EFile file;
				file.Open(szTemp, EFile::modeRead);
				ULONG uSize = file.GetLength();
				unsigned char* pBuf = (unsigned char*)malloc(uSize);
				file.Read(pBuf, uSize);
				file.Close();

				//SStringT szPngBase = CryptoFun.EnBase64Bytes(pBuf, uSize);
				//szSendMsg.AppendFormat(_T("<img src=\"data:image/png;base64,%s\" />"), szPngBase);
			}
			
		}
		/*else if (wcscmp(RichEditMetaFileOle::GetClassName(), pNodeName) == 0)
		{
			sContent += RichEditFileOle::MakeFormattedText(node.attribute(L"file").as_string(),
				L"等待发送", 440227874, 0x04);
		}*/
	}
	
	// 先将 消息 上传 recordRich
	SStringT szRichId = NewGuid();

	AddStateChatMsg(szRichId, sContent, eCST_Waiting);
		
	// 添加异步 发送 
	AsyncSendMsgParam* pAsyncParam = new AsyncSendMsgParam;
	pAsyncParam->uRecipientId = m_uCurrentTalkUserId;
	pAsyncParam->szChatId = szRichId;
	pAsyncParam->szContent = szSendMsg;
	
	// 添加 正在发送信息 到缓存  以防 失败要重发
	theCache.AddSendingChat(m_uCurrentTalkUserId, sContent, *pAsyncParam);

	m_AsynNetTask.AddTask(&CMainWnd::ThreadFun_SendMsg, this, (LPARAM)pAsyncParam);
	
	m_pChatInput->Clear();
	m_pChatRecord->ScrollToBottom();

	ShowLoadingPage(TRUE);
	return ;
}

void CMainWnd::OnBtnSendPic()
{
	SharkWindow();
	return ;


	ShowErrorBox(_T("功能测试中"));
	return ;

	EFileDialog dialog(true, NULL, 0, 0,
		_T("图片文件\0*.gif;*.bmp;*.jpg;*.png\0\0"));
	auto nRet = dialog.DoModal(m_hWnd);
	if(IDOK != nRet)
	{
		return ;
	}

	SStringT szPicPath = dialog.GetFilePath();
	if(!PathFileExists(szPicPath))
	{
		ShowErrorBox(_T("图片不存在！"));
		return ;
	}

	SStringW str;
	str.Format(L"<RichEditContent>"
		L"<para break=\"0\" disable-layout=\"1\">"
		L"<img path=\"%s\" size=\"200,100\" scaring=\"1\" cursor=\"hand\" />"
		L"</para>"
		L"</RichEditContent>", szPicPath);
	

	m_pChatInput->InsertContent(str, RECONTENT_CARET);
	
}

void CMainWnd::ThreadFun_SendMsg(LPARAM lParam)
{
	AsyncSendMsgParam* lpAsyncParam = reinterpret_cast<AsyncSendMsgParam*>(lParam);
	if(NULL == lpAsyncParam)
		return ;

	//bool bRet = false;
	//Sleep(1000);
	// 通知 界面 消息发送完成 
	EventSendMsg* pEvt = new EventSendMsg(this);
	pEvt->uRecipierId = lpAsyncParam->uRecipientId;
	pEvt->szRichObjId = lpAsyncParam->szChatId;
	// soap
	Sleep(200);			//
	pEvt->bSuccess = true;
		
	SNotifyCenter::getSingleton().FireEventAsync(pEvt);
	pEvt->Release();

	delete lpAsyncParam;
	lpAsyncParam = NULL;
}

bool CMainWnd::OnCenterEventSendMsg(EventSendMsg* pEvt)
{
	ShowLoadingPage(FALSE);

	if(NULL == pEvt) return true;

	if(m_uCurrentTalkUserId == pEvt->uRecipierId) //相同 才会更新
	{		
		RichEditBkElement* pObj = sobj_cast<RichEditBkElement>(m_pChatRecord->GetOleByName(pEvt->szRichObjId));
		if(NULL != pObj)
		{
			if(pEvt->bSuccess)
			{
				pObj->SetInteractive(FALSE);
				pObj->SetVisible(FALSE);
			}
			else
			{
				pObj->SetAttribute(L"skin", RichResendWarningSkinName);
			}
			m_pChatRecord->Invalidate();
		}
	}

	// 更新缓存
	if(pEvt->bSuccess)
	{
		// 发送 成功后 就把 缓存里的 删除  
		theCache.DelStateChatAndSave(pEvt->uRecipierId, pEvt->szRichObjId, pEvt->lBodyId, pEvt->lTime);
	}
	else
	{
		theCache.UpdateStateChat(pEvt->uRecipierId, pEvt->szRichObjId, eCST_Error);
		return true;
	}
		
	return true;
}

void CMainWnd::OnBtnUploadFile()
{
	//ShowErrorBox(_T("功能测试中"));
	//return ;


	EFileDialog dialog(true);
	auto nRet = dialog.DoModal(m_hWnd);
	if(IDOK != nRet)
	{
		return ;
	}
	
	SStringT szFilePath = dialog.GetFilePath();
	if(!PathFileExists(szFilePath))
	{
		ShowErrorBox(_T("文件不存在！"));
		return ;
	}

	auto ulFileSize = FileFolderHelper::CalcFileSize(szFilePath);
	if(ulFileSize > 2147483648)
	{
		ShowErrorBox(_T("发送附件大小不能超过2GB！"));
		return ;
	}
	
	SStringT szChatId = NewGuid();
	szChatId.Format(L"id=\"%s\"", szChatId);
	SStringW str2 = RichEditFileOle::MakeFormattedText(szFilePath, L"等待发送",
	ulFileSize, 0x04,
	szChatId);

	AddChatMsg(eChT_Right, 0, str2);
	m_pChatRecord->ScrollToBottom();

	/*RichEditFileOle* pFileObj = sobj_cast<RichEditFileOle>(m_pChatRecord->GetOleById(L"12"));
	if(NULL != pFileObj)
	{
		pFileObj->SetFileStateString(L"49%");
	}*/

	//m_AsynNetTask.AddTask(&CMainWnd::UploadFileThreadFun, this, (LPARAM)lpParam);
}

void CMainWnd::OnBtnLoadTalk()
{
	
	//theFun.HttpUploadFile(L"C:\\123.docx", 11);
}

void CMainWnd::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RETURN)
	{
		// 回车 直接发送 Ctrl+回车 就换行
		bool bCtrl =  (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		if (!bCtrl)
		{
			OnBtnSendMsg();
			return;
		}
		//else if (bCtrl)
		//{
		//	//OnBtnSendMsg();
		//	return;
		//}
	}

	SetMsgHandled(FALSE);
}

void CMainWnd::CallBack_RecentListItemClick(int nIndex)
{
	if(nIndex < 0)
		return ;

	RecentListSelect(nIndex);

}

void CMainWnd::CallBack_RecentListBtnClick(int nIndex)
{
	if(nIndex < 0)
		return ;

	int nSel = m_pRecentListAdapter->GetSelectItem();
	m_pRecentListAdapter->DeleteItem(nIndex);

	if(nIndex < nSel)			//
	{
		m_pRecentListAdapter->SelectItem(nSel-1);
	}
	else if(nIndex == nSel)
	{
		--nIndex;
		if(nIndex < 0)		// 删的 没有了 就影藏 会话
		{
			m_pLaySession->SetVisible(FALSE, TRUE);
			return ;
		}
		m_pRecentListAdapter->SelectItem(nIndex);
		
		RecentListSelect(nIndex);
	}
	// 后面的就不用管了
}

void CMainWnd::CallBack_ContactTvItemClick(UINT uId)
{
	SetDlgItemVisible(L"lay_userinfo", true);

	SStringT szName;
	SStringT szLevel;
	if(!m_pContactTreeAdapter->GetUserById(uId, szName, szLevel))
	{
		ShowErrorBox(_T("该用户有问题！"));
		return ;
	}

	SetDlgItemText(L"pagec_text_user", szName);
	SetDlgItemText(L"pagec_text_info", szLevel);
}

void CMainWnd::CallBack_ContactTvItemDbClick(UINT uId)
{
	if(0 == uId) return ;

	SStringT szName;
	SStringT szLevel;
	if(!m_pContactTreeAdapter->GetUserById(uId, szName, szLevel))
	{
		ShowErrorBox(_T("该用户有问题！"));
		return ;
	}

	int nIndex = NewTalkSession(uId, szName, _T(""));
	
	SwitchPage(0);
	SwitchUserTalkSession(uId, szName, szLevel);

	m_pRecentListAdapter->SelectItem(nIndex);
}

void CMainWnd::CallBack_Emotion(UINT uIndex)
{

}

void CMainWnd::CallBack_MsgToolTip(UINT uId)
{
	if(uId > 0)
	{
		ShowWindow(SW_SHOWNORMAL);
		CallBack_ContactTvItemDbClick(uId);
	}

	EndTrayTwinkle();
}

LRESULT CMainWnd::OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_Tray.Update();
	return 0;
}

LRESULT CMainWnd::OnTrayNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD dwEventMsg = LOWORD(lParam);
	if(WM_LBUTTONDBLCLK == dwEventMsg)
	{    
		//theFun.OpenUrlBy(0);
		ShowWindow(SW_SHOWNORMAL);
		return 0;
	}
	else if(WM_LBUTTONUP == dwEventMsg)
	{
		SetForegroundWindow(m_hWnd);
		return 0;
	}
	else if(WM_MOUSEMOVE == dwEventMsg)
	{
		/*if(NULL != m_pWndMsgToolTip)
		m_pWndMsgToolTip->ShowUnreadMsg();*/
	}
	else if(WM_RBUTTONUP == dwEventMsg)	//右键单击事件
	{
		CPoint pt;
		GetCursorPos(&pt);
		//使用模拟菜单
		SMenu menuMain;
		if(FALSE == menuMain.LoadMenu(_T("menu_tray"),_T("layout")))
		{
			ShowErrorBox(_T("加载菜单 失败！"));
			return 0;
		}

		//WebUrlInfoVct& webList = theFun.GetWebUrlInfo();
		//int nWebCount = webList.size();
		//if(nWebCount <= 0)		//没有数据   就只
		//{
		//	menuMain.InsertMenu(0, MF_BYPOSITION, 1, _T("进入系统"), 0);
		//}
		//else
		//{
		//	SMenu menuSys;
		//	menuSys.m_hMenu = CreatePopupMenu();

		//	for (int i=0; i<nWebCount; ++i)
		//	{
		//		menuSys.InsertMenu(i, MF_BYPOSITION, 10 + i, webList[i].sWebTitle, 0);
		//	}

		//	menuMain.InsertMenu(0, MF_POPUP|MF_BYPOSITION, (UINT_PTR)&menuSys, _T("进入系统"), 0);
		//}

		//// 这里 是 显示 插件 菜单 的接口 
		//int nPluginMenuIdBegin = 40;		// 组件菜单 id 起点 
		//std::vector<int> menuIdList;
		//std::vector<SStringT> menuNameList;
		//theFun.MenuShowEvent(nPluginMenuIdBegin, menuIdList, menuNameList);
		//int nMenuCount = menuNameList.size();
		//if(nMenuCount > 0)
		//{
		//	if(1 == nMenuCount)		// 如果只有一个菜单  就放到 一级菜单
		//	{
		//		menuMain.InsertMenu(2, MF_BYPOSITION, menuIdList[0], menuNameList[0], 0);
		//	}
		//	else			// 多个 菜单  就放到二级菜单下
		//	{
		//		SMenu menuPlugin;
		//		menuPlugin.m_hMenu = CreatePopupMenu();
		//		for (int i=0; i<nMenuCount; ++i)
		//		{
		//			menuPlugin.InsertMenu(i, MF_BYPOSITION, menuIdList[i], menuNameList[i], 0);
		//		}

		//		menuMain.InsertMenu(2, MF_POPUP|MF_BYPOSITION, (UINT_PTR)&menuPlugin, _T("插件设置"), 0);
		//	}			
		//}


		UINT uCmd = menuMain.TrackPopupMenu(0, pt.x, pt.y, m_hWnd);
		//MenuHanld(uCmd);
		if(1 == uCmd)
		{
			ShowWindow(SW_SHOWNORMAL);
		}
		else if(9 == uCmd)
		{
			CloseMain();
		}
	}
	return 0;
}

void CMainWnd::ThreadFun_StartInit(LPARAM lpParam)
{	
	EventStartInit* pEvt = new EventStartInit(this);
	
	Sleep(200);
	pEvt->bSuccess = true;
	
	SNotifyCenter::getSingleton().FireEventAsync(pEvt);
	pEvt->Release();
}

bool CMainWnd::OnCenterEventStartInit(EventStartInit* pEvt)
{
	// 隐藏 加载页面
	ShowLoadingPage(FALSE);

	if(!pEvt->bSuccess)
	{
		ShowErrorBox(pEvt->szErrorText);
		CloseMain();
		return true;
	}
	// 登陆成功 就  

	// 加载 之后 显示 托盘
	HICON hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ETIMESIM));
	SStringT szTipInfo;
	szTipInfo.Format(_T("%s 在线\r\n开启消息提示"), m_szUserAlias);
	m_Tray.Create(m_hWnd, hIcon, szTipInfo, WM_TRAYNOTIFY, 0);
	//	UpdateTrayText();
	//m_Tray.ShowBalloon(_T("提示"), _T("您有新的消息了！"), NIIF_USER, 2000);

	// 托盘显示后  建立 未读消息窗口 
	auto fun = std::bind(&CMainWnd::CallBack_MsgToolTip, this, std::placeholders::_1);
	m_pWndMsgToolTip = new CMsgToolTipWnd(fun);
	m_pWndMsgToolTip->Create(m_hWnd);
	LPCTSTR lpUserAlias = m_szUserAlias;
	m_pWndMsgToolTip->GetNative()->SendMessage(WM_INITDIALOG, 0, (LPARAM)lpUserAlias);
	m_pWndMsgToolTip->MoveButtomRight();

	SStringT szGroupName[] = {L"家人", L"同事", L"朋友", L"好基友" , L"客户", L"老总", L"陌生人", L"黑名单" };

	SStringT szFriendName[] = { L"张三", L"李四", L"老王", L"劳务" , L"呵呵", L"滚蛋"};

	SStringT szName ;
	for(int i=0; i<8; ++i)
	{
		UINT uId = i + 10;
		auto hRoot = m_pContactTreeAdapter->AddUnit(uId, szGroupName[i]);

		for (int j=0; j<6; ++j)
		{
			UINT uId = i * j + 21;
			szName.Format(_T("%s - %d"), szFriendName[j], j+ 100 * i);
			
			m_pContactTreeAdapter->AddUser(uId, szName, L"30岁", hRoot);
		}
	}
		
	//如果当前没有会话  就跳到 联系人 页面
	if(m_pRecentListAdapter->IsEmpty())
	{
		SwitchPage(1);
	}

	return true;
}

void CMainWnd::ThreadFun_GetUnreadMsg(LPARAM lpParam)
{
	EventGetUnreadMsg* pEvt = new EventGetUnreadMsg(this);
	UINT uSenderId = (UINT)lpParam;
	pEvt->bSuccess = true;
		
	pEvt->uSenderId = uSenderId;
	
	SNotifyCenter::getSingleton().FireEventAsync(pEvt);
	pEvt->Release();
}

bool CMainWnd::OnCenterEventGetUnreadMsg(EventGetUnreadMsg* pEvt)
{
	if (NULL == pEvt) return true;

	if(!pEvt->bSuccess)
	{
		ShowErrorBox(pEvt->szErrorText);
		return true;
	}

	SStringT szBriefInfo;
	// 保存 未读消息
	size_t s_size = pEvt->bodyList.size();
	size_t s_i = 0;
	for each(auto var in pEvt->bodyList)
	{
		++s_i;
		
		SStringT szChatContent;
		if(!var->sBodyContent.IsEmpty())
		{
			if(s_i == s_size)				// 只 拼接 最后一条
				szChatContent = ParseContentToIM(var->sBodyContent, &szBriefInfo);
			else
				szChatContent = ParseContentToIM(var->sBodyContent, NULL);
		}
			
		if(szChatContent.IsEmpty())
		{
			continue;
		}

		if(pEvt->uSenderId == m_uCurrentTalkUserId)		// 还在当前user 的聊天界面
		{
			AddChatMsg(eChT_Left, var->lBodyId, szChatContent);
		}
		
		// 历史 是 一定要添加的 
		theCache.AddChatRecord(pEvt->uSenderId, eChT_Left, var->lBodyId, szChatContent, var->lTime);
	}
	
	// 还在当前user 的聊天界面
	if(pEvt->uSenderId == m_uCurrentTalkUserId)
	{
		m_pChatRecord->ScrollToBottom();
		int nIndex = m_pRecentListAdapter->Find(pEvt->uSenderId);
		if(szBriefInfo.IsEmpty())
			m_pRecentListAdapter->Update(nIndex, 0, NULL, NULL);
		else
			m_pRecentListAdapter->Update(nIndex, 0, szBriefInfo, NULL);
	}
	else
	{
		int nIndex = NewTalkSession(pEvt->uSenderId, NULL, szBriefInfo, s_size);
	}

	if(0 != s_size)
	{
		UINT uTotalCount = UpdateTalkSessionUnreadCount();
		m_pWndMsgToolTip->SetTotalCount(uTotalCount);

		// 仅任务栏，闪烁直到窗口激活，默认频率
		FlashWindow();
	}

	

	return true;
}

bool CMainWnd::OnRecordRichObjEvent(EventRichEditObj* pEvt)
{
	switch (pEvt->nSubEventId)
	{
	case DBLCLICK_IMAGEOLE :			// 双击了 图片
		{
			RichEditImageOle* pImageOle = sobj_cast<RichEditImageOle>(pEvt->pRichObj);
			if(NULL != pImageOle)
				ShellExecute(NULL, _T("open"), pImageOle->GetImagePath(), NULL, NULL, SW_SHOW);
		}
		break;
	case CLICK_FILEOLE :				// 点击了 file 模块 的按钮
		{
			RichEditFileOle* pFileOle = sobj_cast<RichEditFileOle>(pEvt->pRichObj);
			if(NULL == pFileOle)
			{
				ShowErrorBox(_T("错误的文件！"));
				break;
			}

			SStringT szPath = pFileOle->GetFilePath();

			auto eClick = (RichEditFileOle::LinkFlag)pEvt->wParam;
			if(RichEditFileOle::LINK_OPEN_FILE == eClick)			// 打开文件
			{
				if(PathFileExists(szPath))
				{
					ShellExecute(NULL, _T("open"), szPath, NULL, NULL, SW_SHOW);
				}
				else
				{
					ShowErrorBox(_T("文件不存在，请重新下载！"));
				}

				break;
			}

			if(RichEditFileOle::LINK_OPEN_DIR == eClick)		// 打开 目录
			{
				if(PathFileExists(szPath))
				{
					SStringW param;
					param.Format(_T("/select,\"%s\""), szPath);
					ShellExecute(NULL, _T("open"), _T("explorer.exe"), param, NULL, SW_SHOW);
				}
				else
				{
					ShowErrorBox(_T("文件不存在，请重新下载！"));
				}
				
				break;
			}

			if(RichEditFileOle::LINK_SAVEAS != eClick)
			{
				break;
			}

			RichEditContent* pRichContent = sobj_cast<RichEditContent>(pFileOle->GetParent()->GetParent());
			if(NULL == pRichContent)
			{
				ShowErrorBox(_T("错误的消息！"));
				break;
			}
			
			
			SStringT szFileName = pFileOle->GetFileName();
			EFileDialog dlg(false, NULL, szFileName);
			if(IDOK != dlg.DoModal(m_hWnd))
			{
				break;
			}
		}
		break;
	case CLICK_FETCHMOREOLE_MORE_MSG :
		//::SetTimer(m_hWnd, TIMERID_LOAD_MSG, 1000, NULL);
		break;
	case CLICK_FETCHMOREOLE_OPEN_LINK :
		::MessageBox(m_hWnd, _T("点击了打开历史记录"), _T("提示"), MB_OK);
		break;
	case CLICK_LINK :			// 链接
		{
			RichEditText * pText = sobj_cast<RichEditText>(pEvt->pRichObj);
			SStringW text;
			text.Format(_T("点击了模仿的链接：%s"), pText->GetText());
			::MessageBox(m_hWnd, text, _T("提示"), MB_OK);
		}
		break;
	case CLICK_BK_ELE:				// 点击了背景 
		{
			RichEditBkElement* pBkOle = sobj_cast<RichEditBkElement>(pEvt->pRichObj);
			if(NULL == pBkOle)
				break;

			if(L"resend" == pBkOle->GetData())
			{				
				SStringT szBkName = pBkOle->GetName();
				AsyncSendMsgParam* pAsyncParam = new AsyncSendMsgParam;
				if(!theCache.GetSendChatInfo(m_uCurrentTalkUserId, szBkName, *pAsyncParam))
				{
					ShowInfoBox(_T("无效的消息"));
					break;
				}
				pBkOle->SetAttribute(L"skin", RichResendLoadingSkinName);
				
				//m_pChatRecord->Invalidate();
				m_AsynNetTask.AddTask(&CMainWnd::ThreadFun_SendMsg, this, (LPARAM)pAsyncParam);

				ShowLoadingPage(TRUE);
			}
			else
			{
				ShowInfoBox(_T("点击了头像"));
			}
		
		}
		break;
	
	default :
		break;
	}

	return true;
}

bool CMainWnd::OnInputRichMenu(EventCtxMenu* pEvt)
{
	CHARRANGE chrSel;
    m_pChatInput->GetSel(&chrSel.cpMin, &chrSel.cpMax);
    RichEditOleBase* pOle = m_pChatInput->GetSelectedOle();

    /*
     * 构造菜单项
     */

    MenuWrapper menu(L"menu_template", L"layout");

    BOOL enableCopy = (chrSel.cpMax - chrSel.cpMin) >= 1;
    menu.AddMenu(_T("复制(&S)"), MENUID_COPY, enableCopy, FALSE);
    menu.AddMenu(_T("剪切(&X)"), MENUID_CUT, enableCopy, FALSE);
    menu.AddMenu(_T("粘贴(&C)"), MENUID_PASTE, TRUE, FALSE);
    menu.AddMenu(_T("全选(&A)"), MENUID_SEL_ALL, TRUE, FALSE);

    if (pOle && pOle->IsClass(RichEditImageOle::GetClassName()))
    {
        menu.AddMenu(_T("另存为"), MENUID_SAVE_AS, TRUE, FALSE);
    }
    else if (pOle && pOle->IsClass(RichEditMetaFileOle::GetClassName()))
    {
        menu.AddMenu(_T("打开"), MENUID_OPEN_FILE, TRUE, FALSE);
        menu.AddMenu(_T("打开文件夹"), MENUID_OPEN_FILE_DIR, TRUE, FALSE);
    }

    /*
     * 弹出菜单
     */
    int ret = 0;
    POINT pt;
    ::GetCursorPos(&pt);

    ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);

    /*
     * 处理菜单消息
     */
    switch (ret)
    {
    case MENUID_COPY:
        m_pChatInput->Copy();
        break;

    case MENUID_SEL_ALL:
        m_pChatInput->SelectAll();
        break;

    case MENUID_CUT:
        m_pChatInput->Cut();
        break;

    case MENUID_PASTE:
        m_pChatInput->Paste();
        break;

    case MENUID_SAVE_AS:
        if (pOle)
        {
            RichEditImageOle* pImageOle = static_cast<RichEditImageOle*>(pOle);
            //SaveImage(pImageOle->GetImagePath());
        }
        break;

    case MENUID_OPEN_FILE:
        if (pOle)
        {
            RichEditMetaFileOle* pMetaFileOle = static_cast<RichEditMetaFileOle*>(pOle);
            ShellExecute(NULL, _T("open"), pMetaFileOle->GetFilePath(), NULL, NULL, SW_SHOW);
        }
        break;

    case MENUID_OPEN_FILE_DIR:
        if (pOle)
        {
            RichEditMetaFileOle* pMetaFileOle = static_cast<RichEditMetaFileOle*>(pOle);
            SStringW param;
            param.Format(_T("/select,\"%s\""), pMetaFileOle->GetFilePath());
            ShellExecute(NULL, _T("open"), _T("explorer.exe"), param, NULL, SW_SHOW);
        }
        break;
    }
	return true;
}

void CMainWnd::RecentListSelect(int nIndex)
{
	UINT uId = 0;
	UINT uCount = 0;
	if(!m_pRecentListAdapter->Get(nIndex, uId, NULL, &uCount))
	{
		ShowErrorBox(_T("获取失败！"));
		return ;
	}

	SStringT sName;
	SStringT sLevel;
	if(!m_pContactTreeAdapter->GetUserById(uId, sName, sLevel))
	{
		ShowErrorBox(_T("没有该用户！"));
		return ;
	}

	SwitchUserTalkSession(uId, sName, sLevel);
}

void CMainWnd::SwitchUserTalkSession(UINT uId, LPCTSTR lpUserAlias, LPCTSTR lpUserLevel)
{
	if(!m_pLaySession->IsVisible())
	{
		m_pLaySession->SetVisible(TRUE, TRUE);
	}
	
		// 设置切换后的名称
	m_uCurrentTalkUserId = uId;
	m_pSessionTitle->SetWindowText(lpUserAlias);
	m_pSessionLevel->SetWindowText(lpUserLevel);
	
	// 先清空当前的 文本框内容
	m_pChatRecord->Clear();
	m_pChatInput->Clear();

	AddChatMsg(eChT_More, 0, NULL);

	// 加载当前会话id 之前的历史消息
	const ChatRecordList* pRecordList = theCache.GetChatRecordList(m_uCurrentTalkUserId);
	if(NULL != pRecordList)
	{
		for each(auto var in *pRecordList)
		{			
			AddChatMsg(var->eType, var->lBodyId, var->szContent);
		}
	}
	
	// 加载 正在发送 的 消息
	const SendingChatList* pSendList = theCache.GetSendingChatList(m_uCurrentTalkUserId);
	if(NULL != pSendList)
	{
		for each(auto var in *pSendList)
		{
			AddStateChatMsg(var->szChatId, var->szChatContent, var->eState);
		}
	}

	//  这里需要判断 
	//AddChatMsg(eChT_Split, NULL);
	
	m_AsynNetTask.AddTask(&CMainWnd::ThreadFun_GetUnreadMsg, this, m_uCurrentTalkUserId);
		
	m_pChatRecord->ScrollToBottom();
	m_pChatInput->SetFocus();
}

void CMainWnd::SwitchPage(int nSel)
{
	m_pTabMain->SetCurSel(nSel);
	if(0 == nSel)
	{
		FindChildByName(L"radio_talk")->SetCheck(TRUE);
	}
	else if(1 == nSel)
	{
		FindChildByName(L"radio_contact")->SetCheck(TRUE);
	}
	else if(2 == nSel)
	{
		FindChildByName(L"radio_notice")->SetCheck(TRUE);
	}
}

void CMainWnd::LoadUserPng(LPCTSTR lpSkinName, LPCTSTR lpPngName)
{
	if (!ImageProvider::IsExist(lpSkinName))
	{
		SAntialiasSkin* pSkin = new SAntialiasSkin();
		pSkin->SetRoundCorner(12, 12, 12, 12); // 添加圆角处理
		if (pSkin->LoadFromFile(m_szUserIconPath + lpPngName))
		{
			ImageProvider::Insert(lpSkinName, pSkin);
		}
		else
		{
			delete pSkin;
		}
	}
}

SStringT CMainWnd::GetUserIconSkin(UINT uUserId)
{
	auto ite = m_mapUserSkin.find(uUserId);
	if(ite == m_mapUserSkin.end())
		return L"default";

	return ite->second;
}

void CMainWnd::AddStateChatMsg(LPCTSTR lpRichName, LPCTSTR lpContent, EnChatState eState)
{
	//LPCWSTR pResendBtn = L"<img path=\"user_icon\\loading_16x16.gif\" name=\"BkEleSendFail\" right-skin=\"loading16\" right-pos=\"{-20,[-25,@16,@16\" cursor=\"hand\" interactive=\"1\"/>";
	/* 靠右显示的消息 */
	SStringT szStateSkin;
	if(eCST_Waiting == eState || eCST_Sending == eState)
		szStateSkin = RichResendLoadingSkinName;
	else if(eCST_Error == eState)
		szStateSkin = RichResendWarningSkinName;
	//else if

	SStringT szSendingChatContent;
	szSendingChatContent.Format(
		L"<RichEditContent type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
		L"<para break=\"1\" align=\"left\" />"
		L"<bkele skin=\"%s\" left-pos=\"0,]-4,@30,@30\" right-pos=\"-30,]-4,@30,@30\"  cursor=\"hand\" interactive=\"1\"/>"
		L"<para margin=\"45,0,30,0\" break=\"1\" simulate-align=\"1\">"
		L"%s"
		L"</para>"
		L"<bkele left-skin=\"rich_left_bubble\" right-skin=\"rich_right_bubble\" left-pos=\"35,{-9,[10,[10\" right-pos=\"{-10,{-9,-35,[10\" />"
		L"<bkele name=\"%s\" data=\"resend\" right-skin=\"%s\" right-pos=\"{-20,[-25,@16,@16\" cursor=\"hand\" interactive=\"1\" />"
		L"</RichEditContent>",

		L"my",
		lpContent,
		lpRichName,
		szStateSkin);

	m_pChatRecord->InsertContent(szSendingChatContent, RECONTENT_LAST);
}

void CMainWnd::AddChatMsg(EnChatType eType, __int64 lBodyId, LPCTSTR lpMsgBody)
{
	SStringW szChatContent;
	switch (eType)
	{
	case eChT_Left:
		{
			szChatContent.Format(
				L"<RichEditContent id=\"%I64d\" type=\"ContentLeft\" align=\"left\">"
				L"<para break=\"1\" align=\"left\" />"
				L"<bkele skin=\"%s\" pos=\"0,]-4,@30,@30\" cursor=\"hand\" interactive=\"1\"/>"
				L"<para margin=\"45,0,30,0\" break=\"1\" simulate-align=\"1\">"
				L"%s"
				L"</para>"
				L"<bkele left-skin=\"rich_left_bubble\" left-pos=\"35,{-9,[10,[10\" />"
				L"</RichEditContent>",

				lBodyId,
				GetUserIconSkin(m_uCurrentTalkUserId),
				lpMsgBody);
		}
		break;
	case eChT_Center:		// 直接 消息 不用拼接
		{
			szChatContent.Format(
			L"<RichEditContent type=\"ContentCenter\" >"
				L"<para break=\"1\" align=\"left\" />"
				L"<para margin=\"100,0,100,0\" align=\"center\" break=\"1\" >"
				L"%s"
				L"</para>"
				L"<bkele skin=\"rich_center_bk\" hittestable=\"0\" pos=\"{-10,{0,[10,[0\" />"
				L"</RichEditContent>",
				
				lpMsgBody);
		}
		break;
	case eChT_Right:
		{
			szChatContent.Format(
				L"<RichEditContent id=\"%I64d\" type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
				L"<para break=\"1\" align=\"left\" />"
				L"<bkele skin=\"%s\" left-pos=\"0,]-4,@30,@30\" right-pos=\"-30,]-4,@30,@30\" cursor=\"hand\" interactive=\"1\"/>"
				L"<para margin=\"45,0,30,0\" break=\"1\" simulate-align=\"1\">"
				L"%s"
				L"</para>"
				L"<bkele left-skin=\"rich_left_bubble\" right-skin=\"rich_right_bubble\" left-pos=\"35,{-9,[10,[10\" right-pos=\"{-10,{-9,-35,[10\" />"
				L"</RichEditContent>",

				lBodyId,
				L"my",
				lpMsgBody);
		}
		break;
	case eChT_More:
		{
			szChatContent.Format(
				L"<RichEditContent type=\"ContentFetchMore\" align=\"center\">"
				L"<para name=\"para\" margin=\"0,5,0,0\" break=\"1\">"
				L"<fetchmore name=\"%s\" selectable=\"0\" />"
				L"</para>"
				L"</RichEditContent>",
				_T("fetchName"));
		}
		break;
	case eChT_Split :
		{
			szChatContent.Format(
				L"<RichEditContent  type=\"ContentSeperator\" align=\"center\">"
				L"<para margin=\"0,10,0,0\" break=\"1\">"
				L"<split selectable=\"0\" />"
				L"</para>"
				L"</RichEditContent>");
		}
		break;
	case eChT_CenterWithoutBk :
		{
			szChatContent.Format(
				L"<RichEditContent type=\"ContentCenter\" >"
				L"<para margin=\"100,5,100,5\" align=\"center\" break=\"1\" >"
				L"%s"
				L"</para>"
				L"</RichEditContent>", 
				
				lpMsgBody);
		}
		break;
	default:
		break;
	}
	
	m_pChatRecord->InsertContent(szChatContent, RECONTENT_LAST);
}

SStringT CMainWnd::ParseContentToIM(const SStringW& sHtml, SStringT* pszBriefInfo)
{
	HTMLParser parse;
	if(!parse.Parse(sHtml))
		return L"";
	
	SStringT sContent;
	auto arrElement = parse.GetElements();
	for each(auto var in arrElement)
	{
		SStringW value;
		SStringW tag = var.GetName();

		if (0 == tag.CompareNoCase(L"text"))
		{
			value =var.GetAttrubite(_T("value"));
			sContent += RichEditText::MakeFormatedText(value);
		}
		else if (tag.CompareNoCase(L"br") == 0 ||
			tag.CompareNoCase(L"li") == 0)
		{
			sContent += L"<text>\r\n</text>";
		}
		else if (tag.CompareNoCase(L"img") == 0)
		{
			value = var.GetAttrubite(_T("src"));
			int nIndex = value.Find(',');
			if(-1 == nIndex) // 不是base64 的
			{
				SStringT szHttp(value, 4);
				if(0 == szHttp.CompareNoCase(_T("http")))
				{
					int nGif = value.Find(L"emoticons/images");
					if(nGif > 0)
					{
						SStringT szGifName = m_szUserIconPath + value.Mid(nGif);
						sContent.AppendFormat(L"<img path=\"%s\" />", szGifName);
					}
				}
				else
					sContent.AppendFormat(L"<img path=\"%s\" />", value);
			}
			else
			{
				SStringT szImgType = value.Mid(0, nIndex);
				SStringT szImgBase64 = value.Mid(nIndex+1);
				SStringT szImgPath = theCache.GetImgCachePath() + NewGuid();

				// 分析  html 里 img   data:image/png;base64,
				SStringTList strList;
				auto size = SplitString<SStringW,wchar_t>(szImgType, '/', strList);
				if(2 == size)
				{					
					if(0 == strList[0].CompareNoCase(L"data:image"))
					{
						SStringTList imgList;
						size = SplitString<SStringW,wchar_t>(strList[1], ';', imgList);
						if(2 == size)
						{
							if(0 == imgList[1].CompareNoCase(L"base64"))
							{
								szImgPath.AppendFormat(_T(".%s"), imgList[0]);
								//  
								//EFile file;
								//file.Open(szImgPath, EFile::modeCreate|EFile::modeWrite);
								//std::string sBuf = CryptoFun.DeBase64Bytes(szImgBase64);
								//file.Write(sBuf.data(), sBuf.size());
								//file.Close();
								//sContent.AppendFormat(L"<img path=\"%s\" />", szImgPath);
							}
						}
					}
				}
			}
		}
		
	}
	return sContent;
}

int CMainWnd::NewTalkSession(UINT uUserId, LPCTSTR lpUserAlias, LPCTSTR lpContent, UINT uCount)
{
	SStringT sName;
	if(NULL == lpUserAlias)
	{
		SStringT sLevel;
		m_pContactTreeAdapter->GetUserById(uUserId, sName, sLevel);
	}
	else
		sName = lpUserAlias;

	int nIndex = m_pRecentListAdapter->Find(uUserId);
	if(-1 == nIndex)
	{
		nIndex = m_pRecentListAdapter->Insert0(uUserId, sName, lpContent, uCount);
		theCache.NewTalkInfo(uUserId, lpUserAlias, lpContent);
	}
	else
	{
		m_pRecentListAdapter->MoveToBegin(nIndex);
		nIndex = 0;
	}

	return nIndex;
}

void CMainWnd::DelRecentTalk(UINT uUserId)
{
	int nIndex = m_pRecentListAdapter->Find(uUserId);
	if(nIndex >= 0)
	{
		
	}
}

void CMainWnd::SharkWindow()
{
	m_SharkWin.UpdateSize(GetWindowRect());
	SetTimer(11, 28);
}

void CMainWnd::NewIMMsgHandle(UINT uSenderId, LPCTSTR lpMsg)
{
	// 只有主窗口显示 和 消息来自 当前聊天界面  才会 直接 去 获取未读
	if(IsWindowVisible() && m_uCurrentTalkUserId == uSenderId)
	{
		m_AsynNetTask.AddTask(&CMainWnd::ThreadFun_GetUnreadMsg, this, uSenderId);
		return ;
	}
	
	// 如果 不在当前的聊天 界面上 或 窗口影藏了  那就 界面 增加 一条 未读 就可以了
			
	//先查询 用户
	UINT uCount = 0;
	SStringT szSenderName;
	int nIndex = m_pRecentListAdapter->Find(uSenderId, &szSenderName, &uCount);
	if(-1 == nIndex)
	{		
		NewTalkSession(uSenderId, NULL, lpMsg, 1);
	}
	else		// 直接更新
	{
		++uCount;			// 未读个数 加 1
		m_pRecentListAdapter->Update(nIndex, uCount, lpMsg);
	}

	UpdateTalkSessionUnreadCount();
		
	if(!IsWindowVisible())
	{
		m_pWndMsgToolTip->AddUnreadMsg(uSenderId, szSenderName, lpMsg);
		m_pWndMsgToolTip->ShowUnreadMsg();
	}
	else
		FlashWindow();
}

UINT CMainWnd::UpdateTalkSessionUnreadCount()
{
	if(NULL == m_pUnreadCount)
		return 0;
	
	UINT uTotalCount = m_pRecentListAdapter->GetTotalUnreadCount();
	if(0 == uTotalCount)
	{
		m_pUnreadCount->SetVisible(FALSE, TRUE);
		return 0;
	}
	
	SStringT szCount;
	if(uTotalCount > 99)
		szCount = _T("99+");
	else 
		szCount.Format(_T("%d"), uTotalCount);
	
	m_pUnreadCount->SetWindowText(szCount);
	
	if(!m_pUnreadCount->IsVisible())
		m_pUnreadCount->SetVisible(TRUE, TRUE);

	return uTotalCount;
}









