#pragma once
#include "MainDlg.h"
class CRealWndDlg_URL : public SHostWnd
{
public:
	CRealWndDlg_URL():SHostWnd(_T("LAYOUT:XML_Readwnd_url"))	{}
	~CRealWndDlg_URL(void){}
	void On_Open_URLBtn()
	{
		SEdit *m_URL_text = FindChildByID2<SEdit>(8001);
		if (m_URL_text)
		{
			if (m_URL_text->GetWindowTextW().IsEmpty())
			{
				CRect rc = m_URL_text->GetWindowRect();
				ClientToScreen(&rc);
				CTipWnd::ShowTip(rc.right, rc.top, CTipWnd::AT_LEFT_BOTTOM, _T("地址错误!\\n地址为空，无法解析"));
				return;
			}
			::SendMessageW(SApplication::getSingleton().GetMainWnd(), MS_REALWND_URLPLAY, 0, (LPARAM)(LPCTSTR)m_URL_text->GetWindowTextW());
		}
	}


protected:
	//soui消息
		EVENT_MAP_BEGIN()
			EVENT_ID_COMMAND(8000, On_Open_URLBtn)
		EVENT_MAP_END()
		//消息映射表
		BEGIN_MSG_MAP_EX(CRealWndDlg_URL)

		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
		END_MSG_MAP()
private:


};
