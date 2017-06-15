#include "stdafx.h"
#include "DlgStyleManage.h"
#include "CDebug.h"
#include "helper/SplitString.h"
#include "ResManger.h"

namespace SOUI
{
	SDlgStyleManage::SDlgStyleManage(SStringT strStyleName, SStringT strPath, BOOL bGetStyle) :
		SHostDialog(_T("layout:UIDESIGNER_XML_SYTLEMGR"))
	{
		m_strStyleName = strStyleName;
		m_strProPath = strPath.Mid(0, strPath.ReverseFind(_T('\\')));
		m_strUIResFile = strPath;
		m_bGetStyle = bGetStyle;
	}


	//TODO:消息映射
	void SDlgStyleManage::OnClose()
	{
		SHostDialog::OnCancel();
	}


	void SDlgStyleManage::OnOK()
	{
		if (m_bGetStyle)
		{
			if (m_lbStyle->GetCurSel() < 0)
			{
				CDebug::Debug(_T("请选择其中一项样式"));
				return;
			}
			else
			{

				m_strStyleName = GetLBCurSelText(m_lbStyle);
			}
		}

		SHostDialog::OnOK();
	}

	BOOL SDlgStyleManage::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_lbStyle = FindChildByName2<SListBox>(L"lbStyle");
		m_edtSearch = FindChildByName2<SEdit>(L"edtSearch");
		m_RealWnd = FindChildByName2<SRealWnd>(L"wndReal");
		m_wndView = FindChildByName2<SWindow>(L"wndView");

		m_lbStyle->DeleteAll();

		m_pResFileManger->LoadUIResFromFile(m_strUIResFile);
		InitStyleLB();

		return TRUE;
	}

	void SDlgStyleManage::OnBtnAdd()
	{

	}
	void SDlgStyleManage::OnBtnDel()
	{

	}
	void SDlgStyleManage::OnBtnSave()
	{

	}

	void SDlgStyleManage::InitStyleLB()
	{
		pugi::xml_node xmlNode;

		xmlNode = m_pResFileManger->GetResFirstNode(_T("style"));
		
		while (xmlNode)
		{
			pugi::xml_attribute attr = xmlNode.attribute(_T("name"));
			if (attr)
			{
				m_lbStyle->AddString(attr.value());
			}

			xmlNode = xmlNode.next_sibling();
		}
	}


	SStringT SDlgStyleManage::GetLBCurSelText(SListBox * lb)
	{
		SStringT s(_T(""));
		int n = lb->GetCurSel();

		if (n < 0)
		{
			return s;
		}

		lb->GetText(n, s);
		return s;
	}

}


