#pragma once
#include "VirtualDlgLayout.h"
#include "MessageBoxLayout.h"
#include <functional>

class MessageBoxLayout : public VirtualDlgLayout
{
public:
	MessageBoxLayout(SWindow* pRoot);
	//MessageBoxLayout(SWindow* pRoot, std::function<void(UINT)> callBackfun);
	~MessageBoxLayout(void);

public:
	void ShowAsyncMsgBox(LPCTSTR lpText, LPCTSTR lpCaption=_T("提示"), UINT nFlags=MB_ICONINFORMATION);
private:
	void ShowLayout(bool);
protected:
	bool OnEventOKCmd(EventCmd* pEvt);
	
protected:
	SStatic*					m_pTextTitle;
	SStatic*					m_pTextContent;
	SIconWnd*				m_pIcon;

	//std::function<void(UINT)> m_CallBackFun;
};

