#include "StdAfx.h"
#include "NewFileDlg.h"
#include "maindlg.h"

CNewFileDlg::CNewFileDlg(CMainDlg *pMainDlg)
:SHostDialog(_T("layout:dlg_newfile"))
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
		SStringT strFilePath=fileDlg.m_szFileName;
		FindChildByName2<SRichEdit>(L"edit_src_file")->SetWindowText(strFilePath);
		int nPos=strFilePath.ReverseFind(_T('.'));
		SStringT strExt=strFilePath.Right(strFilePath.GetLength()-nPos-1);
		strExt.MakeLower();
        SStringT strTitle(fileDlg.m_szFileTitle);
        SStringT strResName=strExt+_T("_")+strTitle.Left(strTitle.GetLength()-strExt.GetLength()-1);
        strResName.MakeLower();
        FindChildByName2<SRichEdit>(L"edit_res_name")->SetWindowText(strResName);

        SComboBox * pCbxType=FindChildByName2<SComboBox>(L"cbx_res_type");

        int iItem = pCbxType->FindString(strExt);
        if(iItem != -1) 
        {
            pCbxType->SetCurSel(iItem);
            SStringT strResPath = strExt+_T("\\")+strTitle;
            FindChildByName2<SRichEdit>(L"edit_res_path")->SetWindowText(strResPath);
        }
	}
	
}

void CNewFileDlg::OnBtn_OK()
{
	m_strSrcFile=FindChildByName2<SRichEdit>(L"edit_src_file")->GetWindowText();
	m_strResPath=FindChildByName2<SRichEdit>(L"edit_res_path")->GetWindowText();
	m_strResName=FindChildByName2<SRichEdit>(L"edit_res_name")->GetWindowText();
	m_strResType=FindChildByName2<SComboBox>(L"cbx_res_type")->GetWindowText();
	EndDialog(IDOK);
}

void CNewFileDlg::OnCbx_ResType_SelChanged(EventArgs *pEvt)
{
    EventCBSelChange *pEvt2 = (EventCBSelChange*)pEvt;
}
