#include "stdafx.h"
#include "FindDlg.h"

namespace SOUI
{


	CFindDlg::CFindDlg(IFindListener *pFindListener)
		:SHostWnd(UIRES.LAYOUT.dlg_find)
		,m_pFindListener(pFindListener)
	{
	}

	CFindDlg::~CFindDlg(void)
	{
	}

	void CFindDlg::OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
		delete this;
	}

	void CFindDlg::OnClose()
	{
		CSimpleWnd::ShowWindow(SW_HIDE);
	}

	void CFindDlg::OnFindNext()
	{
		OnFind(true);
	}

	void CFindDlg::OnFindPrev()
	{
		OnFind(false);
	}

	void CFindDlg::OnFind(bool bNext)
	{
		if(!m_pFindListener) return;
		SStringT strText = m_pEditTarget->GetWindowText();
		if(strText.IsEmpty()) return;
		bool bMatchCase = !!m_pMatchCase->IsChecked();
		bool bMatchWholeWord = !!m_pMatchWholeWord->IsChecked();
		m_pFindListener->OnFind(strText,bNext,bMatchCase,bMatchWholeWord);
	}

	void CFindDlg::OnInit(EventArgs *e)
	{
		m_pEditTarget = FindChildByID2<SEdit>(R.id.edit_search);
		m_pMatchCase = FindChildByID2<SCheckBox>(R.id.chk_match_case);
		m_pMatchWholeWord = FindChildByID2<SCheckBox>(R.id.chk_match_whole_word);
	}

}
