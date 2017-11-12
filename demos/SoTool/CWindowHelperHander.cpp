#include "stdafx.h"
#include "CWindowHelperHander.h"
#include <Psapi.h>
#pragma comment(lib,"psapi.lib")
BOOL EnablePrivilege()
{

	TOKEN_PRIVILEGES tkp;
	HANDLE  hToken;
	//提升程序权限  
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);	
	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);//修改进程权限  
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);//通知系统修改进程权限  

	return((GetLastError() == ERROR_SUCCESS));

}
CWindowHelperHander::CWindowHelperHander()
{
	EnablePrivilege();
}


CWindowHelperHander::~CWindowHelperHander()
{
}

void CWindowHelperHander::OnInit(SOUI::SWindow *pPageRoot)
{
	m_HostRoot = pPageRoot;
	m_hHostWnd = pPageRoot->GetContainer()->GetHostHwnd();
	m_wndFrame.Create();
}

void CWindowHelperHander::OnEventCaptureHost(EventArgs *pEvt)
{
	EventCapture *pEvt2 = (EventCapture*)pEvt;
	CPoint pt = pEvt2->pt_;
	::ClientToScreen(m_hHostWnd, &pt);
	HWND hWnd = ::WindowFromPoint(pt);
	if (hWnd == m_hHostWnd|| hWnd==m_hLastHwnd)
		return ;
	m_hLastHwnd = hWnd;
	
	CRect rcWnd;
	::GetWindowRect(hWnd, &rcWnd);
	HWND hParentWnd=hWnd,hTpWnd = hWnd;
	while (hTpWnd)
	{
		hTpWnd = GetParent(hTpWnd);
		if (hTpWnd)
			hParentWnd = hTpWnd;
	}
	m_wndFrame.Show(rcWnd,GetWindow(hParentWnd, GW_HWNDPREV));
	UpdataWindowInf(hWnd);
}
void CWindowHelperHander::UpdataWindowInf(HWND hWnd)
{
	SStringT str;
	str.Format(_T("0x%08x"), hWnd);
	SASSERT(m_HostRoot);
	m_HostRoot->FindChildByName(L"wnd_hwnd")->SetWindowText(str);
	TCHAR szBuf[266];
	::GetClassName(hWnd, szBuf, 266);
	m_HostRoot->FindChildByName(L"wnd_class")->SetWindowText(szBuf);
	::GetWindowText(hWnd, szBuf, 266);
	m_HostRoot->FindChildByName(L"wnd_title")->SetWindowText(szBuf);
	CRect rc;
	::GetWindowRect(hWnd, &rc);
	str.Format(_T("(%d,%d,%d,%d)[%d×%d]"), rc.left, rc.top,rc.right,rc.bottom, rc.right - rc.left, rc.bottom - rc.top);
	m_HostRoot->FindChildByName(L"wnd_rect")->SetWindowText(str);
	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	str.Format(_T("0x%08x", dwProcessId));
	m_HostRoot->FindChildByName(L"wnd_proid")->SetWindowText(str);
	HANDLE h_Process = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwProcessId);
	if (h_Process)
	{
		GetModuleFileNameEx(h_Process,NULL, szBuf, 266);
		m_HostRoot->FindChildByName(L"wnd_exepath")->SetWindowText(szBuf);
	}
}

void CWindowHelperHander::OnEventCaptureHostFinish(EventArgs *pEvt)
{	
	m_wndFrame.Hide();
	EventCaptureFinish *pEvt2 = (EventCaptureFinish*)pEvt;
	CPoint pt = pEvt2->pt_;
	ClientToScreen(m_hHostWnd, &pt);
	HWND hWnd = ::WindowFromPoint(pt);
	if (hWnd == m_hHostWnd) return;

	m_hLastHwnd = NULL;
}
