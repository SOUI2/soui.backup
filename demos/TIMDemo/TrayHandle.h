#pragma once
#include <ShellAPI.h>
class TrayHandle
{
public:
	TrayHandle(void);
	~TrayHandle(void);
public:
	bool Create(HWND hOwner, 
		HICON hIcon, 
		LPCTSTR lpNotifyText,
		UINT uCallbackMessage, 
		UINT uId,
		UINT uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP));

	bool Update();
	bool Delete();
	void Modify(LPCTSTR lpNotifyText);
	void Modify(HICON hIcon);
	void Modify(LPCTSTR lpNotifyText, HICON hIcon);
	void Twinkling();				//这个是 闪烁 调用的  只需要不停的调用   没有内置定时器
	bool ShowBalloon(LPCTSTR lpBalloonTitle, LPTSTR lpBalloonMsg, DWORD dwIcon=NIIF_NONE, UINT nTimeOut=10);
protected:
	NOTIFYICONDATA		m_NotifyIconData;
	HICON						m_hIcon;
};

