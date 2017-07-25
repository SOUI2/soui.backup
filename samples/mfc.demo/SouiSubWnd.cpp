#include "StdAfx.h"
#include "SouiSubWnd.h"
#include "RealWndDlg.h"
using namespace SOUI;

CSouiSubWnd::CSouiSubWnd(void):SHostWnd(L"xml:subwnd")
{
}

CSouiSubWnd::~CSouiSubWnd(void)
{
}

void CSouiSubWnd::OnBtnOpenSoui()
{
	CRealWndDlg dlg;
	dlg.DoModal(m_hWnd);
}

void CSouiSubWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}
