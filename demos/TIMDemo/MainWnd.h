#pragma once
#include "resource.h"
#include "CacheHandle.h"
#include "MsgToolTipWnd.h"
#include "ui\MessageBoxLayout.h"
#include "TrayHandle.h"
#include "ui\CustomEvent.h"
#include "ui\SIconRadio.hpp"
#include "ui\imre\SImRichedit.h"
#include "skin\SVscrollbar.h"
#include "skin\SAntialiasSkin.h"
#include "skin\WinFileIconSkin.hpp"
#include "skin\MaskSkin.hpp"
#include "adapter/RecentListAdapter.hpp"
#include "adapter/ContactTreeAdapter.hpp"
#include "adapter/EmotionTileAdapter.hpp"
#include "AsynFunctionT.hpp"
#include "SharkWinHandle.hpp"

#define WM_TRAYNOTIFY							WM_USER + 201
//*要处理这种情况，其实还是离不开消息映射，但与普通的消息映射又有点不同。
//explorer重启的时候会抛出WM_TASKBAR_CREATED消息，我们的程序要加入该消息的响应，
// 不同之处在于要先注册这个消息：
static UINT WM_TASKBAR_CREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

//#define		LAYOUT_MODULE		1

class CMainWnd : public SHostWnd
							, public TAutoEventMapReg<CMainWnd>//通知中心自动注册
{
private:
	struct AsyncSaveApproverParam
	{
		__int64			lBodyId;
		SStringT			szApproverText;
	};
public:
	CMainWnd(void);
	~CMainWnd(void);
	
	void OnBtnMin()
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	}
	void OnBtnClose()
	{
#ifdef LAYOUT_MODULE
		CloseMain();
#else
		//if(NULL == m_pWndMsgToolTip)
			CloseMain();
		//else
		//	ShowWindow(SW_HIDE);
#endif
	}
	void SetDlgItemVisible(LPCTSTR lpName, bool bShow)
	{
		auto p = FindChildByName(lpName);
		if(NULL != p)
			p->SetVisible(bShow ? TRUE : FALSE, TRUE);
	}
	void SetDlgItemText(LPCTSTR lpName, LPCTSTR lpText)
	{
		auto p = FindChildByName(lpName);
		if(NULL != p)
			p->SetWindowText(lpText);
	}
	void ShowInfoBox(LPCTSTR lpText, LPCTSTR lpCaption=L"提示", UINT uType=MB_ICONINFORMATION)
	{
		//return SMessageBox(m_hWnd, lpText, lpCaption, uType);
		m_pAsynMessageBox->ShowAsyncMsgBox(lpText, lpCaption, uType);
	}
	void ShowErrorBox(LPCTSTR lpText=L"错误")
	{
		//return SMessageBox(m_hWnd, lpText, _T("错误"), MB_ICONERROR);
		m_pAsynMessageBox->ShowAsyncMsgBox(lpText, _T("错误"), MB_ICONERROR);
	}
	void FlashWindow(DWORD dwFlags=FLASHW_TRAY | FLASHW_TIMERNOFG)
	{
		FLASHWINFO fwi;
		fwi.cbSize = sizeof(fwi);
		fwi.hwnd = m_hWnd;
		fwi.dwFlags = dwFlags;
		fwi.uCount = 0;
		fwi.dwTimeout = 0;

		::FlashWindowEx(&fwi);
	}
	void CloseMain()
	{
		m_Tray.Delete();
		__super::DestroyWindow();
	}
	// 窗口 抖动 
	void SharkWindow();
public:
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void OnTimer(UINT_PTR idEvent);
	//自定义控件事件函数
	void OnBtnSendMsg();
	void OnClickRadioTalk();
	void OnClickRadioContact();
	void OnClickRadioNotice();
	void OnBtnSendPic();
	void OnBtnLoadTalk();			// 加载 会话 按钮
	void OnBtnUploadFile();			// 发送附件
	
public:	// 通知中心 事件 接口
	bool OnCenterEventStartInit(EventStartInit* pEvt);
	bool OnCenterEventGetUnreadMsg(EventGetUnreadMsg* pEvt);	
	bool OnCenterEventSendMsg(EventSendMsg* pEvt);
public:  // im rich 事件
	bool OnRecordRichObjEvent(EventRichEditObj* pEvt);
	bool OnInputRichMenu(EventCtxMenu* pEvt);
protected:
	//事件处理映射表  频繁的 操作可以写到前面来 减少 判断 最后都用 绑定 
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_chat_upload", OnBtnUploadFile)
		EVENT_NAME_COMMAND(L"btn_chat_sendpic", OnBtnSendPic)
		EVENT_NAME_COMMAND(L"btn_close", OnBtnClose)
		EVENT_NAME_COMMAND(L"btn_min", OnBtnMin)
		EVENT_NAME_COMMAND(L"btn_send", OnBtnSendMsg)
		EVENT_NAME_COMMAND(L"radio_talk", OnClickRadioTalk)
		EVENT_NAME_COMMAND(L"radio_contact", OnClickRadioContact)
		EVENT_NAME_COMMAND(L"radio_notice", OnClickRadioNotice)
		EVENT_NAME_COMMAND(L"pagec_img_talk", OnBtnLoadTalk)
	EVENT_MAP_END()

	void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
	LRESULT OnTrayNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam);
	//窗口消息处理映射表
	BEGIN_MSG_MAP_EX(CMainWnd)
		MSG_WM_KEYDOWN(OnKeyDown)
		MESSAGE_HANDLER_EX(WM_TRAYNOTIFY, OnTrayNotify)
		MESSAGE_HANDLER_EX(WM_TASKBAR_CREATED, OnTaskbarCreated)//
		MSG_WM_TIMER(OnTimer)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnBtnClose)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()
public:
	// 最近联系人 item 点击 回调
	void CallBack_RecentListItemClick(int nIndex);
	// 最近联系人 item 里 btn 回调
	void CallBack_RecentListBtnClick(int nIndex);
	// 联系人界面 tree  单击 事件 回调
	void CallBack_ContactTvItemClick(UINT uId);
	// 联系人界面 tree  双击 事件 回调
	void CallBack_ContactTvItemDbClick(UINT uId);
	// 表情符号 选择 回调 
	void CallBack_Emotion(UINT uIndex);
	// 消息 提示框 的回调
	void CallBack_MsgToolTip(UINT uId);

	// 初始化  线程 函数
	void ThreadFun_StartInit(LPARAM lParam);
	// 获取未读消息 线程 处理函数
	void ThreadFun_GetUnreadMsg(LPARAM lParam);
	// 发送消息 线程 处理函数
	void ThreadFun_SendMsg(LPARAM lParam);
	
private:
	// 最近联系人 选择 人 
	void RecentListSelect(int nIndex);
	// 切换 用户 会话 
	void SwitchUserTalkSession(UINT uId, LPCTSTR lpUserAlias, LPCTSTR lpUserLevel);
	// 添加 正在发送的消息 一般都是在 right 
	void AddStateChatMsg(LPCTSTR lpRichName, LPCTSTR lpContent, EnChatState eState);
	// 调用这个函数之前 先设置当前聊天用户 
	void AddChatMsg(EnChatType eType, __int64 lBodyId, LPCTSTR lpMsgBody);
private:
	template<class T>
	inline void InitWnd(T*& pWnd, LPCTSTR lpWndName)
	{
		pWnd = FindChildByName2<T>(lpWndName);
		if(NULL == pWnd)
		{
			SStringT sErrorText;
			sErrorText.Format(_T("没有name为 <%s> 的控件"), lpWndName);
			ShowErrorBox(sErrorText);
		}
	}
	void ShowLoadingPage(BOOL bShow)
	{
		m_pLayLoading->SetVisible(bShow, TRUE);
	}
	
	void StartTrayTwinkle()
	{
		//m_Tray.Modify(lpTipInfo);
		//SetTimer(10, 500);
	}
	void EndTrayTwinkle()
	{
		//KillTimer(10);
		//m_Tray.Update();
	}
	// 切换 页面 在 会话 和联系人 
	void SwitchPage(int nSel);
	// 向 最近联系人列表添加
	int NewTalkSession(UINT uUserId, LPCTSTR lpUserAlias, LPCTSTR lpContent, UINT uCount=0);
	void DelRecentTalk(UINT uUserId);
	// 未读消息 处理
	void NewIMMsgHandle(UINT uSenderId, LPCTSTR lpMsg);
	//更新  会话 标题 未读 个数
	UINT UpdateTalkSessionUnreadCount();
		
	void LoadUserPng(LPCTSTR lpSkinName, LPCTSTR lpPngName);
	SStringT GetUserIconSkin(UINT uUserId);
	SStringT ParseContentToIM(const SStringW& sHtml, SStringT* pszBriefInfo);
public:
	SWindow*					m_pUnreadCount;				// 
	STabCtrl*					m_pTabMain;
	SWindow*					m_pLayLoading;					// 加载页面
	SWindow*					m_pLaySession;
	
	SStatic*						m_pSessionTitle;					//
	SWindow*					m_pSessionLevel;
	SImRichEdit*				m_pChatRecord;					// 消息 记录 框
	SImRichEdit*				m_pChatInput;					// 输入框 
	RecentListAdapter*		m_pRecentListAdapter;			// 最近聊天 好友
	ContactTreeAdapter*		m_pContactTreeAdapter;			// 联系人 
	EmotionTileAdapter*		m_pEmotionTileAdapter;		// 表情
	
	CMsgToolTipWnd*			m_pWndMsgToolTip;						// 右下角 弹出框

	MessageBoxLayout*			m_pAsynMessageBox;					 // 能模拟的 消息 提示框	

private:
	SStringT									m_szUserAlias;
	AsynTaskHandle<LPARAM>		m_AsynNetTask;
	UINT										m_uCurrentTalkUserId;
	//SStringT									m_sCurrentTalkTitle;
	SStringT									m_szUserIconPath;
	TrayHandle								m_Tray;
	std::map<UINT, SStringT>		m_mapUserSkin;
	SharkWinHandle						m_SharkWin;
};

