#include "stdafx.h"
#include "DlgFontSelect.h"
#include "CDebug.h"

namespace SOUI
{

	SDlgFontSelect::SDlgFontSelect(SStringT strFont,SDesignerView *pDesignerView):SHostDialog(_T("LAYOUT:UIDESIGNER_XML_FONTSELECT"))
	{
		strFont.TrimBlank();
		m_strFont = strFont;
		m_pDesignerView = pDesignerView;
	}

	//TODO:œ˚œ¢”≥…‰
	void SDlgFontSelect::OnCancel()
	{
		SHostDialog::OnCancel();
	}

	void SDlgFontSelect::OnOK()
	{
	
		m_strFont = GetFontStr();

		SHostDialog::OnOK();
	}

	BOOL SDlgFontSelect::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{

		m_LbFont = FindChildByName2<SListBox>(L"lbFont");
		m_edtSize = FindChildByName2<SEdit>(L"edtSize");

		m_chkBold = FindChildByName2<SCheckBox>(L"chkBold");
		m_chkItalic = FindChildByName2<SCheckBox>(L"chkItalic");
		m_chkUnderline = FindChildByName2<SCheckBox>(L"chkUnderline");
		m_chkStrike = FindChildByName2<SCheckBox>(L"chkStrike");

		m_edtSearch = FindChildByName2<SEdit>(L"edtSearch");
		m_wndPreview = FindChildByName2<SWindow>(L"wndPreview");

		m_LbFont->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED,Subscriber(&SDlgFontSelect::OnLBSelChanged,this));
		m_edtSearch->GetEventSet()->subscribeEvent(EventRENotify::EventID,Subscriber(&SDlgFontSelect::OnReNotify,this));
        m_edtSize->GetEventSet()->subscribeEvent(EventRENotify::EventID,Subscriber(&SDlgFontSelect::OnSizeReNotify,this));
		
		m_LbFont->DeleteAll();


		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET; // Initialize the LOGFONT structure

		_tcscpy(lf.lfFaceName,_T(""));

		::EnumFontFamiliesEx(GetDC(),&lf,
			(FONTENUMPROC) EnumFontFamProc,(LPARAM)&(m_map),0);


		SPOSITION pos = m_map.GetStartPosition();
		while (pos)
		{
			SMap<SStringT, SStringT>::CPair *p = m_map.GetNext(pos);
			m_LbFont->AddString(p->m_value);
		}

		if (!m_strFont.IsEmpty())
		{
			m_pDesignerView->UseEditorUIDef(false);
			IFontPtr ft = SFontPool::getSingleton().GetFont(m_strFont, 100);
			m_pDesignerView->UseEditorUIDef(true);
			InitInfo(ft);
		}
		else
		{
			m_pDesignerView->UseEditorUIDef(false);
			IFontPtr ft = SFontPool::getSingleton().GetFont(_T(""),100);
			m_pDesignerView->UseEditorUIDef(true);
			InitInfo(ft);
		}


		return TRUE;
	}

	int SDlgFontSelect::GetLbIndexFromText(SListBox *lb, SStringT strText)
	{
		int n = -1;
		if (lb->GetCount() == 0)
		{
			return -1;
		}

		SStringT strLbText;
		for (int i = 0; i < lb->GetCount(); i ++)
		{
			lb->GetText(i, strLbText);
			if (strLbText.CompareNoCase(strText) == 0)
			{
				n = i;
				break;
			}
		}


		return n;
	}


	bool SDlgFontSelect::OnLBSelChanged(EventArgs *pEvt)
	{
		PreviewFont();
		return true;
	}

	bool SDlgFontSelect::OnCBSelChanged(EventArgs *pEvt)
	{
		PreviewFont();
		return true;
	}


	void SDlgFontSelect::PreviewFont()
	{
		    SStringT strFontName = GetFontStr();
			m_wndPreview->SetAttribute(L"font", strFontName);
			m_wndPreview->Invalidate();		
	}



	bool SDlgFontSelect::OnReNotify(EventArgs *pEvt)
	{
		EventRENotify *pReEvt = (EventRENotify*)pEvt;
		if(pReEvt->iNotify == EN_CHANGE)
		{
			SStringT strValue = m_edtSearch->GetWindowText();
			m_LbFont->DeleteAll();

			SPOSITION pos = m_map.GetStartPosition();	
			while(pos)
			{
				SMap<SStringT, SStringT>::CPair *p = m_map.GetNext(pos);
				SStringT strFontName = p->m_value;
				if (strFontName.Find(strValue) >= 0)
				{
					m_LbFont->AddString(strFontName);
				}

			}

		}
		return true;
	}


	bool SDlgFontSelect::OnSizeReNotify(EventArgs *pEvt)
	{
		EventRENotify *pReEvt = (EventRENotify*)pEvt;
		if(pReEvt->iNotify == EN_CHANGE)
		{
			PreviewFont();
		}
		return true;
	}

	
	void SDlgFontSelect::OnCKClick()
	{
	    PreviewFont();	
	}


	void SDlgFontSelect::InitInfo(IFontPtr ft)
	{
		SStringT strName =	ft->FamilyName();

		int n = GetLbIndexFromText(m_LbFont, strName);

		m_LbFont->SetCurSel(n);

		m_LbFont->EnsureVisible(n);

		m_chkBold->SetCheck(ft->IsBold());
		m_chkItalic->SetCheck(ft->IsItalic());
		m_chkStrike->SetCheck(ft->IsStrikeOut());
		m_chkUnderline->SetCheck(ft->IsUnderline());

		n = ft->TextSize();
		SStringT strSize;
		strSize.Format(_T("%d"),abs(n));
		m_edtSize->SetWindowText(strSize);

	}

	SStringT SDlgFontSelect::GetFontStr()
	{
		int n = m_LbFont->GetCurSel();
		if (n < 0)
		{
			return _T("");
		}

		SStringT strFontName;
		m_LbFont->GetText(n, strFontName);
		strFontName = strFontName.Format(_T("face:%s"), strFontName);

		if (m_chkBold->IsChecked())
		{
			strFontName = strFontName + _T(",bold:1");
		}

		if (m_chkItalic->IsChecked())
		{
			strFontName = strFontName + _T(",italic:1");
		}

		if (m_chkUnderline->IsChecked())
		{
			strFontName = strFontName + _T(",underline:1");
		}

		if (m_chkStrike->IsChecked())
		{
			strFontName = strFontName + _T(",strike:1");
		}

		SStringT strSize = m_edtSize->GetWindowText();

		if (!strSize.IsEmpty())
		{
			strFontName = strFontName + _T(",size:") + strSize;
		}

		return strFontName;
	}
}



