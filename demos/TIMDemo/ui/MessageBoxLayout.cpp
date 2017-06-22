#include "StdAfx.h"
#include "MessageBoxLayout.h"

MessageBoxLayout::MessageBoxLayout(SWindow* pRoot)
	: VirtualDlgLayout(pRoot, _T("layout:layout_asyncmsgbox"))
	
{
	InitWnd(m_pTextTitle, L"msgbox_title");
	InitWnd(m_pTextContent, L"msgbox_text");
	InitWnd(m_pIcon, L"msgbox_icon");

	subscribeEvent(L"msgbox_btn_close", &MessageBoxLayout::OnEventOKCmd, this);
	subscribeEvent(L"msgbox_btn_ok", &MessageBoxLayout::OnEventOKCmd, this);	

	//m_CallBackFun = NULL;
}
/*
MessageBoxLayout::MessageBoxLayout(SWindow* pRoot, std::function<void(UINT)> callBackfun)
	: VirtualDlgLayout(pRoot, _T("layout:layout_asyncmsgbox"))
{
	InitWnd(m_pTextTitle, L"msgbox_title");
	InitWnd(m_pTextContent, L"msgbox_text");
	InitWnd(m_pIcon, L"msgbox_icon");

	
	subscribeEvent(L"msgbox_btn_close", &MessageBoxLayout::OnEventOKCmd, this);
	subscribeEvent(L"msgbox_btn_ok", &MessageBoxLayout::OnEventOKCmd, this);

	m_CallBackFun = callBackfun;
}*/

MessageBoxLayout::~MessageBoxLayout(void)
{
}

void MessageBoxLayout::ShowAsyncMsgBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT nFlags)
{
	m_pTextContent->SetWindowText(lpText);
	m_pTextTitle->SetWindowText(lpCaption);
	
	switch(nFlags & 0xF0)
	{
	case MB_ICONEXCLAMATION:
		m_pIcon->SetIcon(LoadIcon(NULL,IDI_EXCLAMATION));
		break;
	case MB_ICONINFORMATION:
		m_pIcon->SetIcon(LoadIcon(NULL,IDI_INFORMATION));
		break;
	case MB_ICONQUESTION:
		m_pIcon->SetIcon(LoadIcon(NULL,IDI_QUESTION));
		break;
	case MB_ICONHAND:
		m_pIcon->SetIcon(LoadIcon(NULL,IDI_HAND));
		break;
	default:
		m_pIcon->SetVisible(FALSE,TRUE);
		break;
	}


	VirtualDlgLayout::ShowLayout(true);
}

bool MessageBoxLayout::OnEventOKCmd(EventCmd* pEvt)
{
	if(NULL == pEvt) return true;
				
	ShowLayout(false);

	//if(NULL != m_CallBackFun)
	//	m_CallBackFun(IDOK);

	return true;
}

void MessageBoxLayout::ShowLayout(bool b)
{
	VirtualDlgLayout::ShowLayout(b);
}







