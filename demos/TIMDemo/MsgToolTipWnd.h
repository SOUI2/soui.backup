#pragma once
#include "resource.h"
#include <functional>
#include <map>

class CMsgToolTipWnd : public SHostWnd
{
public:
	CMsgToolTipWnd(std::function<void(UINT)> fun);
	~CMsgToolTipWnd(void);
			
	void OnBtnIgnore();
	void OnBtnViewAll();
public:
	BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
	void MoveButtomRight(int nPadding=10);
	
	void ShowUnreadMsg();				// 显示 窗口

	void AddUnreadMsg(UINT uUserId, LPCTSTR lpUserAlias, LPCTSTR lpContent, UINT uCount=1);
	void SetTotalCount(UINT uCount);
protected:
	//事件处理映射表
	EVENT_MAP_BEGIN()
		
		EVENT_NAME_COMMAND(L"btn_ignore", OnBtnIgnore)
		EVENT_NAME_COMMAND(L"btn_viewall", OnBtnViewAll)
	EVENT_MAP_END()

	//窗口消息处理映射表
	BEGIN_MSG_MAP_EX(CNotifyWnd)
		MSG_WM_INITDIALOG(OnInitDialog)
		CHAIN_MSG_MAP(SHostWnd)
	END_MSG_MAP()

protected:
	template<class T>
	inline void InitWnd(T*& pWnd, LPCTSTR lpWndName)
	{
		pWnd = FindChildByName2<T>(lpWndName);
		if(NULL == pWnd)
		{
			SStringT sErrorText;
			sErrorText.Format(_T("没有name为 <%s> 的控件"), lpWndName);
			SMessageBox(m_hWnd, sErrorText, _T("错误"), MB_ICONERROR);
		}
	}
private:
	std::function<void(UINT)>			m_funClick;			// 点击事件 回调
	UINT											m_uSenderId;
	UINT											m_uTotalCount;							// 所有 未读的总数
	std::map<UINT, UINT>				m_mapSenderCount;			// 所有 sender 的未读数 
private:
	SImageWnd*			m_pImgUser;
	SStatic*					m_pTextSender;				//发送人 
	SStatic*					m_pTextSendMsg;				//发送的消息
	SWindow*				m_pWinTotalCount;
	SWindow*				m_pWinCount;
};

