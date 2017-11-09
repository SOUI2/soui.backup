#include "stdafx.h"
#include "SQrCtrl.h"


SQrCtrl::SQrCtrl()
{
}


SQrCtrl::~SQrCtrl()
{
}

void SQrCtrl::SetWindowText(LPCTSTR lpszText)
{
	__super::SetWindowText(lpszText);
	CreateQrImg(SWindow::m_strText.GetText());
}

BOOL SQrCtrl::OnRelayout(const CRect & rcWnd)
{
	BOOL bRet = __super::OnRelayout(rcWnd);
	if(bRet)
		if (m_pImg)
		{
			if((m_pImg->Width()!=rcWnd.Width())||(m_pImg->Height()!=rcWnd.Height()))
				CreateQrImg(SWindow::m_strText.GetText());
		}
		else CreateQrImg(SWindow::m_strText.GetText());
	return bRet;
}

