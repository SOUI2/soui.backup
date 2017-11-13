#include "stdafx.h"
#include "CWindowHelperHander.h"
#include <Psapi.h>
#include <map>
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
	str.Format(_T("0x%08x"), dwProcessId);
	m_HostRoot->FindChildByName(L"wnd_proid")->SetWindowText(str);
	HANDLE h_Process = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, dwProcessId);
	if (h_Process)
	{
		GetModuleFileNameEx(h_Process,NULL, szBuf, 266);
		m_HostRoot->FindChildByName(L"wnd_exepath")->SetWindowText(szBuf);
	}
}
const std::map<LONG, LPCTSTR>::value_type init_style[] =
{
	std::map<LONG,LPCTSTR>::value_type(WS_BORDER,_T("The window has a thin-line border.")),
	std::map<LONG,LPCTSTR>::value_type(WS_CAPTION,_T("The window has a title bar (includes the WS_BORDER style).")),
	std::map<LONG,LPCTSTR>::value_type(WS_CHILD,_T("The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.")),
	std::map<LONG,LPCTSTR>::value_type(WS_CLIPCHILDREN,_T("Excludes the area occupied by child windows when drawing occurs within the parent window. This style is used when creating the parent window.")),
	std::map<LONG,LPCTSTR>::value_type(WS_CLIPSIBLINGS,_T("Clips child windows relative to each other; that is, when a particular child window receives a WM_PAINT message, the WS_CLIPSIBLINGS style clips all other overlapping child windows out of the region of the child window to be updated. If WS_CLIPSIBLINGS is not specified and child windows overlap, it is possible, when drawing within the client area of a child window, to draw within the client area of a neighboring child window.")),
	std::map<LONG,LPCTSTR>::value_type(WS_DISABLED,_T("The window is initially disabled. A disabled window cannot receive input from the user. To change this after a window has been created, use the EnableWindow function.")),
	std::map<LONG,LPCTSTR>::value_type(WS_DLGFRAME,_T("The window has a border of a style typically used with dialog boxes. A window with this style cannot have a title bar.")),
	std::map<LONG,LPCTSTR>::value_type(WS_GROUP,_T("The window is the first control of a group of controls. The group consists of this first control and all controls defined after it, up to the next control with the WS_GROUP style. The first control in each group usually has the WS_TABSTOP style so that the user can move from group to group. The user can subsequently change the keyboard focus from one control in the group to the next control in the group by using the direction keys.You can turn this style on and off to change dialog box navigation.To change this style after a window has been created, use the SetWindowLong function.")),
	std::map<LONG,LPCTSTR>::value_type(WS_HSCROLL,_T("The window has a horizontal scroll bar.")),
	std::map<LONG,LPCTSTR>::value_type(WS_MINIMIZE,_T("The window is initially minimized. Same as the WS_MINIMIZE style.")),
	std::map<LONG,LPCTSTR>::value_type(WS_MAXIMIZE,_T("The window is initially maximized.")),
	std::map<LONG,LPCTSTR>::value_type(WS_MAXIMIZEBOX,_T("The window has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified. ")),
	std::map<LONG,LPCTSTR>::value_type(WS_MINIMIZEBOX,_T("The window has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified. ")),
	std::map<LONG,LPCTSTR>::value_type(WS_OVERLAPPED,_T("The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style.")),
	std::map<LONG,LPCTSTR>::value_type(WS_POPUP,_T("The windows is a pop-up window. This style cannot be used with the WS_CHILD style.")),
	std::map<LONG,LPCTSTR>::value_type(WS_SIZEBOX,_T("The window has a sizing border. Same as the WS_THICKFRAME style.")),
	std::map<LONG,LPCTSTR>::value_type(WS_SYSMENU,_T("The window has a window menu on its title bar. The WS_CAPTION style must also be specified.")),
	std::map<LONG,LPCTSTR>::value_type(WS_TABSTOP,_T("The window is a control that can receive the keyboard focus when the user presses the TAB key. Pressing the TAB key changes the keyboard focus to the next control with the WS_TABSTOP style.	You can turn this style on and off to change dialog box navigation.To change this style after a window has been created, use the SetWindowLong function.For user - created windows and modeless dialogs to work with tab stops, alter the message loop to call the IsDialogMessage function.")),
	std::map<LONG,LPCTSTR>::value_type(WS_VISIBLE,_T("The window is initially visible.")),
	std::map<LONG,LPCTSTR>::value_type(WS_VSCROLL,_T("The window has a vertical scroll bar."))
};
const std::map<LONG, LPCTSTR> desMap(init_style, init_style + sizeof init_style /sizeof init_style[0]);
#if(WINVER < 0x0602)
#define WS_EX_NOREDIRECTIONBITMAP 0x00200000L
#endif /* WINVER >= 0x0602 */
const std::map<LONG, LPCTSTR>::value_type init_exstyle[] =
{
	std::map<LONG,LPCTSTR>::value_type(WS_EX_ACCEPTFILES,_T("The window accepts drag-drop files.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_APPWINDOW,_T("Forces a top-level window onto the taskbar when the window is visible. ")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_CLIENTEDGE,_T("The window has a border with a sunken edge.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_COMPOSITED,_T("Paints all descendants of a window in bottom-to-top painting order using double-buffering. For more information, see Remarks. This cannot be used if the window has a class style of either CS_OWNDC or CS_CLASSDC.	Windows 2000:  This style is not supported.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_CONTEXTHELP,_T("The title bar of the window includes a question mark. When the user clicks the question mark, the cursor changes to a question mark with a pointer. If the user then clicks a child window, the child receives a WM_HELP message. The child window should pass the message to the parent window procedure, which should call the WinHelp function using the HELP_WM_HELP command. The Help application displays a pop-up window that typically contains help for the child window.	WS_EX_CONTEXTHELP cannot be used with the WS_MAXIMIZEBOX or WS_MINIMIZEBOX styles.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_CONTROLPARENT,_T("The window itself contains child windows that should take part in dialog box navigation. If this style is specified, the dialog manager recurses into children of this window when performing navigation operations such as handling the TAB key, an arrow key, or a keyboard mnemonic.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_DLGMODALFRAME,_T("The window has a double border; the window can, optionally, be created with a title bar by specifying the WS_CAPTION style in the dwStyle parameter.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_LAYERED,_T("The window is a layered window. This style cannot be used if the window has a class style of either CS_OWNDC or CS_CLASSDC.Windows 8:  The WS_EX_LAYERED style is supported for top - level windows and child windows.Previous Windows versions support WS_EX_LAYERED only for top - level windows.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_LAYOUTRTL,_T("If the shell language is Hebrew, Arabic, or another language that supports reading order alignment, the horizontal origin of the window is on the right edge. Increasing horizontal values advance to the left. ")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_LEFT,_T("The window has generic left-aligned properties. This is the default.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_LEFTSCROLLBAR,_T("If the shell language is Hebrew, Arabic, or another language that supports reading order alignment, the vertical scroll bar (if present) is to the left of the client area. For other languages, the style is ignored.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_LTRREADING,_T("The window text is displayed using left-to-right reading-order properties. This is the default.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_MDICHILD,_T("The window is a MDI child window.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_NOACTIVATE,_T("A top-level window created with this style does not become the foreground window when the user clicks it. The system does not bring this window to the foreground when the user minimizes or closes the foreground window.To activate the window, use the SetActiveWindow or SetForegroundWindow function.The window does not appear on the taskbar by default.To force the window to appear on the taskbar, use the WS_EX_APPWINDOW style.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_NOINHERITLAYOUT,_T("The window does not pass its window layout to its child windows.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_NOPARENTNOTIFY,_T("The child window created with this style does not send the WM_PARENTNOTIFY message to its parent window when it is created or destroyed..")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_NOREDIRECTIONBITMAP,_T("The window does not render to a redirection surface. This is for windows that do not have visible content or that use mechanisms other than surfaces to provide their visual.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_RIGHT,_T("The window has generic \"right - aligned\" properties. This depends on the window class. This style has an effect only if the shell language is Hebrew, Arabic, or another language that supports reading-order alignment; otherwise, the style is ignored.Using the WS_EX_RIGHT style for static or edit controls has the same effect as using the SS_RIGHT or ES_RIGHT style, respectively.Using this style with button controls has the same effect as using BS_RIGHT and BS_RIGHTBUTTON styles. ")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_RIGHTSCROLLBAR,_T("The vertical scroll bar (if present) is to the right of the client area. This is the default.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_RTLREADING,_T("If the shell language is Hebrew, Arabic, or another language that supports reading-order alignment, the window text is displayed using right-to-left reading-order properties. For other languages, the style is ignored.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_STATICEDGE,_T("The window has a three-dimensional border style intended to be used for items that do not accept user input.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_TOOLWINDOW,_T("The window is intended to be used as a floating toolbar. A tool window has a title bar that is shorter than a normal title bar, and the window title is drawn using a smaller font. A tool window does not appear in the taskbar or in the dialog that appears when the user presses ALT+TAB. If a tool window has a system menu, its icon is not displayed on the title bar. However, you can display the system menu by right-clicking or by typing ALT+SPACE. ")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_TOPMOST,_T("The window should be placed above all non-topmost windows and should stay above them, even when the window is deactivated. To add or remove this style, use the SetWindowPos function.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_TRANSPARENT,_T("The window should not be painted until siblings beneath the window (that were created by the same thread) have been painted. The window appears transparent because the bits of underlying sibling windows have already been painted.To achieve transparency without these restrictions, use the SetWindowRgn function.")),
	std::map<LONG,LPCTSTR>::value_type(WS_EX_WINDOWEDGE,_T("The window has a border with a raised edge."))
};
const std::map<LONG, LPCTSTR> desexMap(init_exstyle, init_exstyle + sizeof init_exstyle / sizeof init_exstyle[0]);

void CWindowHelperHander::GetStyleList(LONG lStyle, SArray<StyleInf>& styleList)
{
	styleList.RemoveAll();
	if (lStyle&WS_OVERLAPPED)
	{
		auto it = desMap.find(WS_OVERLAPPED);		
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_OVERLAPPED"),it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_OVERLAPPED"), _T("")));
		}
		lStyle &= ~WS_OVERLAPPED;
	}
	if (lStyle&WS_POPUP)
	{
		auto it = desMap.find(WS_POPUP);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_POPUP"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_POPUP"), _T("The windows is a pop-up window. This style cannot be used with the WS_CHILD style.")));
		}
		lStyle &= ~WS_POPUP;
	}
	if (lStyle&WS_CHILD)
	{
		auto it = desMap.find(WS_CHILD);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_CHILD"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_CHILD"), _T("The window is a child window. A window with this style cannot have a menu bar. This style cannot be used with the WS_POPUP style.")));
		}
		lStyle &= ~WS_CHILD;
	}
	if (lStyle&WS_MINIMIZE)
	{
		auto it = desMap.find(WS_MINIMIZE);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_MINIMIZE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_MINIMIZE"), _T("")));
		}
		lStyle &= ~WS_MINIMIZE;
	}
	if (lStyle&WS_VISIBLE)
	{
		auto it = desMap.find(WS_VISIBLE);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_VISIBLE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_VISIBLE"), _T("")));
		}
		lStyle &= ~WS_VISIBLE;
	}
	if (lStyle&WS_DISABLED)
	{
		auto it = desMap.find(WS_DISABLED);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_DISABLED"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_DISABLED"), _T("")));
		}
		lStyle &= ~WS_DISABLED;
	}
	if (lStyle&WS_CLIPSIBLINGS)
	{
		auto it = desMap.find(WS_CLIPSIBLINGS);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_CLIPSIBLINGS"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_CLIPSIBLINGS"), _T("")));
		}
		lStyle &= ~WS_CLIPSIBLINGS;
	}
	if (lStyle&WS_CLIPCHILDREN)
	{
		auto it = desMap.find(WS_CLIPCHILDREN);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_CLIPCHILDREN"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_CLIPCHILDREN"), _T("")));
		}
		lStyle &= ~WS_CLIPCHILDREN;
	}
	if (lStyle&WS_MAXIMIZE)
	{
		auto it = desMap.find(WS_MAXIMIZE);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_MAXIMIZE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_MAXIMIZE"), _T("")));
		}
		lStyle &= ~WS_MAXIMIZE;
	}
	if (lStyle&WS_CAPTION)
	{
		auto it = desMap.find(WS_CAPTION);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_CAPTION"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_CAPTION"), _T("")));
		}
		lStyle &= ~WS_CAPTION;
	}
	if (lStyle&WS_BORDER)
	{
		auto it = desMap.find(WS_BORDER);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_BORDER"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_BORDER"), _T("")));
		}
		lStyle &= ~WS_BORDER;
	}
	if (lStyle&WS_DLGFRAME)
	{
		auto it = desMap.find(WS_DLGFRAME);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_DLGFRAME"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_DLGFRAME"), _T("")));
		}
		lStyle &= ~WS_DLGFRAME;
	}
	if (lStyle&WS_VSCROLL)
	{
		auto it = desMap.find(WS_VSCROLL);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_VSCROLL"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_VSCROLL"), _T("")));
		}
		lStyle &= ~WS_VSCROLL;
	}
	if (lStyle&WS_HSCROLL)
	{
		auto it = desMap.find(WS_HSCROLL);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_HSCROLL"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_HSCROLL"), _T("")));
		}
		lStyle &= ~WS_HSCROLL;
	}
	if (lStyle&WS_SYSMENU)
	{
		auto it = desMap.find(WS_SYSMENU);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_SYSMENU"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_SYSMENU"), _T("")));
		}
		lStyle &= ~WS_SYSMENU;
	}
	if (lStyle&WS_THICKFRAME)
	{
		auto it = desMap.find(WS_THICKFRAME);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_THICKFRAME"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_THICKFRAME"), _T("")));
		}
		lStyle &= ~WS_THICKFRAME;
	}
	if (lStyle&WS_GROUP)
	{
		auto it = desMap.find(WS_GROUP);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_GROUP"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_GROUP"), _T("")));
		}
		lStyle &= ~WS_GROUP;
	}
	if (lStyle&WS_TABSTOP)
	{
		auto it = desMap.find(WS_TABSTOP);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_TABSTOP"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_TABSTOP"), _T("")));
		}
		lStyle &= ~WS_TABSTOP;
	}
	if (lStyle&WS_MINIMIZEBOX)
	{
		auto it = desMap.find(WS_MINIMIZEBOX);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_MINIMIZEBOX"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_MINIMIZEBOX"), _T("")));
		}
		lStyle &= ~WS_MINIMIZEBOX;
	}
	if (lStyle&WS_MAXIMIZEBOX)
	{
		auto it = desMap.find(WS_MAXIMIZEBOX);
		if (it != desMap.end()) {
			styleList.Add(StyleInf(_T("WS_MAXIMIZEBOX"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_MAXIMIZEBOX"), _T("")));
		}
		lStyle &= ~WS_MAXIMIZEBOX;
	}
	if (lStyle)
	{
		styleList.Add(StyleInf(SStringT().Format(_T("0x%08x"),lStyle),_T("未知样式")));
	}
}

void CWindowHelperHander::GetExStyleList(LONG lStyle, SArray<StyleInf>& styleList)
{
	styleList.RemoveAll();
	
	if (lStyle&WS_EX_DLGMODALFRAME)
	{
		auto it = desexMap.find(WS_EX_DLGMODALFRAME);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_DLGMODALFRAME"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_DLGMODALFRAME"), _T("")));
		}
		lStyle &= ~WS_EX_DLGMODALFRAME;
	}
	if (lStyle&WS_EX_NOPARENTNOTIFY)
	{
		auto it = desexMap.find(WS_EX_NOPARENTNOTIFY);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_NOPARENTNOTIFY"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_NOPARENTNOTIFY"), _T("")));
		}
		lStyle &= ~WS_EX_NOPARENTNOTIFY;
	}
	if (lStyle&WS_EX_TOPMOST)
	{
		auto it = desexMap.find(WS_EX_TOPMOST);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_TOPMOST"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_TOPMOST"), _T("")));
		}
		lStyle &= ~WS_EX_TOPMOST;
	}
	if (lStyle&WS_EX_ACCEPTFILES)
	{
		auto it = desexMap.find(WS_EX_ACCEPTFILES);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_ACCEPTFILES"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_ACCEPTFILES"), _T("")));
		}
		lStyle &= ~WS_EX_ACCEPTFILES;
	}
	if (lStyle&WS_EX_TRANSPARENT)
	{
		auto it = desexMap.find(WS_EX_TRANSPARENT);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_TRANSPARENT"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_TRANSPARENT"), _T("")));
		}
		lStyle &= ~WS_EX_TRANSPARENT;
	}
	if (lStyle&WS_EX_MDICHILD)
	{
		auto it = desexMap.find(WS_EX_MDICHILD);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_MDICHILD"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_MDICHILD"), _T("")));
		}
		lStyle &= ~WS_EX_MDICHILD;
	}
	if (lStyle&WS_EX_TOOLWINDOW)
	{
		auto it = desexMap.find(WS_EX_TOOLWINDOW);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_TOOLWINDOW"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_TOOLWINDOW"), _T("")));
		}
		lStyle &= ~WS_EX_TOOLWINDOW;
	}
	if (lStyle&WS_EX_WINDOWEDGE)
	{
		auto it = desexMap.find(WS_EX_WINDOWEDGE);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_WINDOWEDGE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_WINDOWEDGE"), _T("")));
		}
		lStyle &= ~WS_EX_WINDOWEDGE;
	}
	if (lStyle&WS_EX_WINDOWEDGE)
	{
		auto it = desexMap.find(WS_EX_WINDOWEDGE);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_WINDOWEDGE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_WINDOWEDGE"), _T("")));
		}
		lStyle &= ~WS_EX_WINDOWEDGE;
	}
	if (lStyle&WS_EX_CLIENTEDGE)
	{
		auto it = desexMap.find(WS_EX_CLIENTEDGE);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_CLIENTEDGE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_CLIENTEDGE"), _T("")));
		}
		lStyle &= ~WS_EX_CLIENTEDGE;
	}
	if (lStyle&WS_EX_CONTEXTHELP)
	{
		auto it = desexMap.find(WS_EX_CONTEXTHELP);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_CONTEXTHELP"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_CONTEXTHELP"), _T("")));
		}
		lStyle &= ~WS_EX_CONTEXTHELP;
	}
	if (lStyle&WS_EX_RIGHT)
	{
		auto it = desexMap.find(WS_EX_RIGHT);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_RIGHT"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_RIGHT"), _T("")));
		}
		lStyle &= ~WS_EX_RIGHT;
	}
	if (lStyle&WS_EX_LEFT)
	{
		auto it = desexMap.find(WS_EX_LEFT);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_LEFT"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_LEFT"), _T("")));
		}
		lStyle &= ~WS_EX_LEFT;
	}
	if (lStyle&WS_EX_RTLREADING)
	{
		auto it = desexMap.find(WS_EX_RTLREADING);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_RTLREADING"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_RTLREADING"), _T("")));
		}
		lStyle &= ~WS_EX_RTLREADING;
	}
	if (lStyle&WS_EX_LTRREADING)
	{
		auto it = desexMap.find(WS_EX_LTRREADING);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_LTRREADING"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_LTRREADING"), _T("")));
		}
		lStyle &= ~WS_EX_LTRREADING;
	}
	if (lStyle&WS_EX_LEFTSCROLLBAR)
	{
		auto it = desexMap.find(WS_EX_LEFTSCROLLBAR);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_LEFTSCROLLBAR"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_LEFTSCROLLBAR"), _T("")));
		}
		lStyle &= ~WS_EX_LEFTSCROLLBAR;
	}
	if (lStyle&WS_EX_RIGHTSCROLLBAR)
	{
		auto it = desexMap.find(WS_EX_RIGHTSCROLLBAR);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_RIGHTSCROLLBAR"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_RIGHTSCROLLBAR"), _T("")));
		}
		lStyle &= ~WS_EX_RIGHTSCROLLBAR;
	}
	if (lStyle&WS_EX_CONTROLPARENT)
	{
		auto it = desexMap.find(WS_EX_CONTROLPARENT);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_CONTROLPARENT"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_CONTROLPARENT"), _T("")));
		}
		lStyle &= ~WS_EX_CONTROLPARENT;
	}
	if (lStyle&WS_EX_STATICEDGE)
	{
		auto it = desexMap.find(WS_EX_STATICEDGE);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_STATICEDGE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_STATICEDGE"), _T("")));
		}
		lStyle &= ~WS_EX_STATICEDGE;
	}
	if (lStyle&WS_EX_APPWINDOW)
	{
		auto it = desexMap.find(WS_EX_APPWINDOW);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_APPWINDOW"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_APPWINDOW"), _T("")));
		}
		lStyle &= ~WS_EX_APPWINDOW;
	}
	if (lStyle&WS_EX_OVERLAPPEDWINDOW)
	{
		auto it = desexMap.find(WS_EX_OVERLAPPEDWINDOW);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_OVERLAPPEDWINDOW"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_OVERLAPPEDWINDOW"), _T("")));
		}
		lStyle &= ~WS_EX_OVERLAPPEDWINDOW;
	}
	if (lStyle&WS_EX_PALETTEWINDOW)
	{
		auto it = desexMap.find(WS_EX_PALETTEWINDOW);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_PALETTEWINDOW"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_PALETTEWINDOW"), _T("")));
		}
		lStyle &= ~WS_EX_PALETTEWINDOW;
	}
	if (lStyle&WS_EX_LAYERED)
	{
		auto it = desexMap.find(WS_EX_LAYERED);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_LAYERED"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_LAYERED"), _T("")));
		}
		lStyle &= ~WS_EX_LAYERED;
	}
	if (lStyle&WS_EX_NOINHERITLAYOUT)
	{
		auto it = desexMap.find(WS_EX_NOINHERITLAYOUT);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_NOINHERITLAYOUT"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_NOINHERITLAYOUT"), _T("")));
		}
		lStyle &= ~WS_EX_NOINHERITLAYOUT;
	}

	if (lStyle&WS_EX_NOREDIRECTIONBITMAP)
	{
		auto it = desexMap.find(WS_EX_NOREDIRECTIONBITMAP);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_NOREDIRECTIONBITMAP"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_NOREDIRECTIONBITMAP"), _T("")));
		}
		lStyle &= ~WS_EX_NOREDIRECTIONBITMAP;
	}


	if (lStyle&WS_EX_LAYOUTRTL)
	{
		auto it = desexMap.find(WS_EX_LAYOUTRTL);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_LAYOUTRTL"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_LAYOUTRTL"), _T("")));
		}
		lStyle &= ~WS_EX_LAYOUTRTL;
	}
	if (lStyle&WS_EX_COMPOSITED)
	{
		auto it = desexMap.find(WS_EX_COMPOSITED);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_COMPOSITED"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_COMPOSITED"), _T("")));
		}
		lStyle &= ~WS_EX_COMPOSITED;
	}
	if (lStyle&WS_EX_NOACTIVATE)
	{
		auto it = desexMap.find(WS_EX_NOACTIVATE);
		if (it != desexMap.end()) {
			styleList.Add(StyleInf(_T("WS_EX_NOACTIVATE"), it->second));
		}
		else {
			styleList.Add(StyleInf(_T("WS_EX_NOACTIVATE"), _T("")));
		}
		lStyle &= ~WS_EX_NOACTIVATE;
	}
	if (lStyle)
	{
		styleList.Add(StyleInf(SStringT().Format(_T("0x%08x"), lStyle), _T("未知扩展样式")));
	}
}

pugi::xml_parse_result LoadXMLFormBuf(pugi::xml_document &doc,LPCWSTR lpContent)
{
	return doc.load_buffer(lpContent, wcslen(lpContent) * sizeof(wchar_t), pugi::parse_default, pugi::encoding_utf16);
}

void CWindowHelperHander::OnEventCaptureHostFinish(EventArgs *pEvt)
{	
	m_wndFrame.Hide();
	EventCaptureFinish *pEvt2 = (EventCaptureFinish*)pEvt;
	CPoint pt = pEvt2->pt_;
	ClientToScreen(m_hHostWnd, &pt);
	HWND hWnd = ::WindowFromPoint(pt);
	if (hWnd == m_hHostWnd) return;

	if (m_hLastHwnd != hWnd)
		UpdataWindowInf(hWnd);
	m_hLastHwnd = NULL;
	SArray<StyleInf> styleList;
	GetStyleList(GetWindowLong(hWnd, GWL_STYLE), styleList);
	
	SWindow* pStyleList=m_HostRoot->FindChildByName(L"wnd_style");
	if (pStyleList)
	{
		SWindow *pChild = pStyleList->GetWindow(GSW_FIRSTCHILD);
		while (pChild)
		{
			SWindow *pNextChild = pChild->GetWindow(GSW_NEXTSIBLING);			
			pStyleList->DestroyChild(pChild);
			pChild = pNextChild;
		}
		SApplication *theApp = SApplication::getSingletonPtr();
		for (int i = 0; i < styleList.GetCount(); i++)
		{
			SWindow *pChild = theApp->CreateWindowByName(L"text");
			pStyleList->InsertChild(pChild);
			pugi::xml_document doc;
			SStringW buf;
			buf.Format(L"<text>%s</text><text>%s</text>",S_CT2W(styleList[i].strStyle),S_CT2W(styleList[i].strDes));
			LoadXMLFormBuf(doc,buf);
			pugi::xml_node pChildNode = doc.first_child();
			pChild->InitFromXml(pChildNode);
			SWindow *pChild2 = theApp->CreateWindowByName(L"text");
			pStyleList->InsertChild(pChild2);
			pChild2->InitFromXml(pChildNode.next_sibling());
		}
		//pStyleList->RequestRelayout();
	}
	GetExStyleList(GetWindowLong(hWnd, GWL_EXSTYLE), styleList);
	pStyleList = m_HostRoot->FindChildByName(L"wnd_exstyle");
	if (pStyleList)
	{
		SWindow *pChild = pStyleList->GetWindow(GSW_FIRSTCHILD);
		while (pChild)
		{
			SWindow *pNextChild = pChild->GetWindow(GSW_NEXTSIBLING);
			pStyleList->DestroyChild(pChild);
			pChild = pNextChild;
		}
		SApplication *theApp = SApplication::getSingletonPtr();
		for (int i = 0; i < styleList.GetCount(); i++)
		{
			SWindow *pChild = theApp->CreateWindowByName(L"text");
			pStyleList->InsertChild(pChild);
			pugi::xml_document doc;
			SStringW buf;
			buf.Format(L"<text>%s</text><text>%s</text>", S_CT2W(styleList[i].strStyle), S_CT2W(styleList[i].strDes));
			LoadXMLFormBuf(doc, buf);
			pugi::xml_node pChildNode = doc.first_child();
			pChild->InitFromXml(pChildNode);
			SWindow *pChild2 = theApp->CreateWindowByName(L"text");
			pStyleList->InsertChild(pChild2);
			pChild2->InitFromXml(pChildNode.next_sibling());
		}
		//pStyleList->RequestRelayout();
	}
}
