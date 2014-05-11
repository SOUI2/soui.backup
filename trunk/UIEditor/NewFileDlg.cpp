#include "StdAfx.h"
#include "NewFileDlg.h"
#include "maindlg.h"

CNewFileDlg::CNewFileDlg(CMainDlg *pMainDlg)
:CDuiHostWnd(_T("IDR_DUI_ADDFILE_DIALOG"))
,m_pMainDlg(pMainDlg)
{

}

CNewFileDlg::~CNewFileDlg(void)
{
}

void CNewFileDlg::OnBtn_SelFile()
{
	CFileDialogEx fileDlg(TRUE,NULL,NULL,6,_T("All Resource Files\0*.xml;*.bmp;*.png;*.jpg;*.tiff;*.tga\0xml files(*.xml)\0*.xml\0Image files (*.bmp;*.png;*.jpg;*.tiff;*.tga)\0*.bmp;*.png;*.jpg;*.tiff;*.tga\0All files (*.*)\0*.*\0\0"));
	if(fileDlg.DoModal()==IDOK)
	{
		CDuiStringT strFilePath=fileDlg.m_szFileName;
		FindChildByName2<CDuiRichEdit*>("edit_src_file")->SetWindowText(strFilePath);
		int nPos=strFilePath.ReverseFind(_T('.'));
		CDuiStringT strExt=strFilePath.Right(strFilePath.GetLength()-nPos-1);
		strExt.MakeLower();
		CDuiComboBox * pCbxType=FindChildByName2<CDuiComboBox * >("cbx_res_type");

		if(strExt==_T("xml"))
		{
			pCbxType->SetCurSel(0);
		}else if(strExt==_T("ico"))
		{
			pCbxType->SetCurSel(1);
		}else if(strExt==_T("bmp"))
		{
			pCbxType->SetCurSel(2);
		}else
		{
			pCbxType->SetCurSel(3);
		}

		CDuiStringT strTitle(fileDlg.m_szFileTitle);
		CDuiStringT strResName=strExt+_T("_")+strTitle.Left(strTitle.GetLength()-strExt.GetLength()-1);
		strResName.MakeLower();
		FindChildByName2<CDuiRichEdit*>("edit_res_name")->SetWindowText(strResName);

		CDuiStringT strResPath;
		if(strExt==_T("xml")) 
		{
			strResPath=_T("xml");
			FindChildByName2<CDuiCheckBox*>("chk_layout")->SetVisible(TRUE,TRUE);
		}
		else
		{
			strResPath=_T("image");
			FindChildByName2<CDuiCheckBox*>("chk_layout")->SetVisible(FALSE,TRUE);
		}
		strResPath+=_T("\\")+strTitle;
		
		FindChildByName2<CDuiRichEdit*>("edit_res_path")->SetWindowText(strResPath);
	}
	
}

void CNewFileDlg::OnBtn_OK()
{
	m_strSrcFile=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_src_file"));
	m_strResPath=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_res_path"));
	m_strResName=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_res_name"));
	m_strResType=CUIHelper::GetComboboxText(FindChildByName2<CDuiComboBox*>("cbx_res_type"));
	m_bLayout = FindChildByName2<CDuiCheckBox*>("chk_layout")->IsChecked();
	EndDialog(IDOK);
}

LRESULT CNewFileDlg::OnCbx_ResType_SelChanged( LPDUINMHDR pnmhdr )
{
	LPDUINMLBSELCHANGE pCbxNmhdr=(LPDUINMLBSELCHANGE )pnmhdr;
	if(pCbxNmhdr->nNewSel==0)
	{//xml
		FindChildByName2<CDuiCheckBox*>("chk_layout")->SetVisible(TRUE,TRUE);
	}else
	{
		FindChildByName2<CDuiCheckBox*>("chk_layout")->SetVisible(FALSE,TRUE);
	}
	return 0;
}
