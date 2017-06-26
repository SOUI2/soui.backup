#include "StdAfx.h"
#include "TrayHandle.h"


TrayHandle::TrayHandle(void)
{
	ZeroMemory(&m_NotifyIconData, sizeof(m_NotifyIconData));
	m_hIcon = NULL;
}


TrayHandle::~TrayHandle(void)
{
	Delete();
}

bool TrayHandle::Create(HWND hOwner,
						HICON hIcon,
						LPCTSTR lpNotifyText,
						UINT uCallbackMessage, 
						UINT uId,
						UINT uFlags)
{
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.hWnd = hOwner;
	m_NotifyIconData.uID = uId;
	m_NotifyIconData.uFlags = uFlags;//NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = uCallbackMessage;
	m_NotifyIconData.hIcon = hIcon;
	m_hIcon = hIcon;
	_tcscpy_s(m_NotifyIconData.szTip, 128, lpNotifyText);

	return TRUE == Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
}

bool TrayHandle::Update()
{
	m_NotifyIconData.hIcon = m_hIcon;
	BOOL bRet = Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
	if(FALSE == bRet)
	{
		bRet = Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
	}

	return TRUE == bRet;
}

bool TrayHandle::Delete()
{
	return (TRUE == Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData));
}

void TrayHandle::Twinkling()
{
	if(NULL == m_NotifyIconData.hIcon)
	{
		m_NotifyIconData.hIcon = m_hIcon;
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
	}
	else
	{
		m_NotifyIconData.hIcon = NULL;
		Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
	}
}

void TrayHandle::Modify(LPCTSTR lpNotifyText)
{
	_tcscpy_s(m_NotifyIconData.szTip, 128, lpNotifyText);
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	m_NotifyIconData.uFlags |= NIF_TIP;
	Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

void TrayHandle::Modify(HICON hIcon)
{
	m_NotifyIconData.hIcon = hIcon;
	Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

void TrayHandle::Modify(LPCTSTR lpNotifyText, HICON hIcon)
{
	_tcscpy_s(m_NotifyIconData.szTip, 128, lpNotifyText);
	m_NotifyIconData.hIcon = hIcon;
	Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

bool TrayHandle::ShowBalloon(LPCTSTR lpBalloonTitle, LPTSTR lpBalloonMsg, DWORD dwIcon, UINT nTimeOut)
{
	m_NotifyIconData.dwInfoFlags = dwIcon;
	m_NotifyIconData.uFlags |= NIF_INFO;
	m_NotifyIconData.uTimeout = nTimeOut;
	// Set the balloon title
	_tcscpy_s(m_NotifyIconData.szInfoTitle, 64, lpBalloonTitle);

	// Set balloon message
	_tcscpy_s(m_NotifyIconData.szInfo, 256, lpBalloonMsg);

	// Show balloon....
	return TRUE == Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}
