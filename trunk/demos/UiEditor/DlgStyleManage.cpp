#include "stdafx.h"
#include "DlgStyleManage.h"
#include "CDebug.h"
#include "helper/SplitString.h"

namespace SOUI
{

	SDlgStyleManage::SDlgStyleManage(SStringT strStyleName, SStringT strPath, BOOL bGetStyle):SHostDialog(_T("layout:样式管理"))
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
			}else
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
		LoadStyleFile();
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

	void SDlgStyleManage::LoadStyleFile()
	{
		pugi::xml_document xmlDocUiRes;

		if (!xmlDocUiRes.load_file(m_strUIResFile))
		{
			CDebug::Debug(_T("加载uires文件失败"));
			return;
		}

		pugi::xml_node xmlNode = xmlDocUiRes.root().child(L"resource").child(L"UIDEF").first_child();
		SStringW strPath;
		strPath = xmlNode.attribute(L"path").value();

		if (!strPath.IsEmpty())
		{
			SStringT strInitFile;
			strInitFile = m_strProPath + _T("\\") + strPath;

			m_strStyleFile = strInitFile;

			pugi::xml_parse_result result = m_xmlDocStyle.load_file(strInitFile);
			if (result)
			{

				pugi::xml_node xmlNode1 = m_xmlDocStyle.child(L"UIDEF").child(L"style");
				if (xmlNode1.attribute(L"src"))
				{
					SStringT strSrc = xmlNode1.attribute(L"src").value();
					SStringTList strLst;
					SplitString(strSrc,_T(':'),strLst);
					if(strLst.GetCount() != 2) 
					{
						SASSERT_FMTW(L"Parse pos attribute failed, src=%s",strSrc);
						return ;
					}

					strLst.GetAt(0).TrimBlank();

					strLst[0];

					m_strStyleFile = m_strProPath + _T("\\") + strLst[0] + _T("\\") + strLst[1] + _T(".xml");
					result = m_xmlDocStyle.load_file(m_strStyleFile);
					if (!result)
					{
						SMessageBox(NULL, _T("加载skin文件失败"), _T("加载skin文件失败"), MB_OK);
					}
				}
			}
		}

	}


	void SDlgStyleManage::InitStyleLB()
	{

		pugi::xml_node xmlNode;

		if (m_xmlDocStyle.child(L"style"))
		{
			xmlNode = m_xmlDocStyle.child(L"style").first_child();
		}else if(m_xmlDocStyle.child(L"UIDEF"))
		{
			xmlNode = m_xmlDocStyle.child(L"UIDEF").child(L"style").first_child();
		}



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


