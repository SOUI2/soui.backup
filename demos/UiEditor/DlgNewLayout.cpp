#include "stdafx.h"
#include "DlgNewLayout.h"
#include "CDebug.h"

namespace SOUI
{

	SDlgNewLayout::SDlgNewLayout(LPCTSTR pszXmlName, SStringT strProPath):SHostDialog(pszXmlName)
	{
		m_strProPath = strProPath;
	}

	//TODO:消息映射
	void SDlgNewLayout::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgNewLayout::OnOK()
	{
		m_strPath = m_edtPath->GetWindowText();
		m_strName = m_edtName->GetWindowText();
		if (m_strPath.IsEmpty() || m_strName.IsEmpty())
		{
			CDebug::Debug(_T("资源名称或路径不能为空"));
			return;

		}

		int n = m_strPath.Find(m_strProPath);
		if (n != 0)
		{
			CDebug::Debug(_T("请将资源保存到uires目录下"));
			return;
		}

		SHostDialog::OnOK();
	}

	BOOL SDlgNewLayout::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		
		m_edtName = FindChildByName2<SEdit>(L"NAME_UIDESIGNER_edit_ZY");
		m_edtPath = FindChildByName2<SEdit>(L"NAME_UIDESIGNER_edit_Path");
		

		return TRUE;
	}

    void SDlgNewLayout::OnBtnDlgOpenFile()	
	{
		CFileDialogEx OpenDlg(FALSE, NULL, NULL, 6, _T("所有文件 (*.*)\0*.*\0\0"));
		if (IDOK ==OpenDlg.DoModal())
		{
			SStringT strFileName = OpenDlg.m_szFileName;
			int n = strFileName.Find(m_strProPath);
			if (n != 0)
			{
				SMessageBox(NULL, _T("请将资源保存到uires目录下"), _T("提示"), MB_OK);
				return;
			}


			n = strFileName.Find(_T(".xml"));
			if(n == -1)
			{
				strFileName = strFileName + _T(".xml");
			}

			m_edtPath->SetWindowText(strFileName);

		}
	}
}


