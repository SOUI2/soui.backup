#include "StdAfx.h"
#include "NewSkinDlg.h"
#include "maindlg.h"

CNewSkinDlg::CNewSkinDlg(CMainDlg *pMainDlg)
:CDuiHostWnd(_T("IDR_DUI_ADDSKIN_DIALOG"))
,m_pMainDlg(pMainDlg)
{
}

CNewSkinDlg::~CNewSkinDlg(void)
{
}

LRESULT CNewSkinDlg::OnInitDialog( HWND wnd, LPARAM lParam )
{
	FindChildByName2<CDuiRichEdit*>("edit_imglst_src_name")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<CDuiRichEdit*>("edit_imglst_states")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<CDuiRichEdit*>("edit_imgframe_src_name")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<CDuiRichEdit*>("edit_imgframe_states")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<CDuiRichEdit*>("edit_imgframe_frame")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<CDuiRichEdit*>("edit_scrollbar_src_name")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<CDuiRichEdit*>("edit_imgframe_frame")->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	return 0;
}

LRESULT CNewSkinDlg::OnEditNotify_imglst( LPDUINMHDR pNHdr )
{
	LPDUIRICHEDITNOTIFY pEditNotify=(LPDUIRICHEDITNOTIFY)pNHdr;
	if(pEditNotify->iNotify==EN_CHANGE)
	{
		UpdateImglstPreview();
	}
	return S_OK;
}

void CNewSkinDlg::UpdateImglstPreview()
{
	CDuiRichEdit *pEdit=FindChildByName2<CDuiRichEdit*>("edit_imglst_src_name");
	CDuiStringT strSrcName=CUIHelper::GetEditText(pEdit);
	CDuiStringT strSrcFile=m_pMainDlg->GetImageSrcFile(strSrcName);
	if(!strSrcFile.IsEmpty())
	{
		CSkinView_ImgLst *pImgView=FindChildByName2<CSkinView_ImgLst*>("imgview_imglst");
		pImgView->SetImageFile(strSrcFile);
		CDuiStringT strStates=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imglst_states"));
		int nStates=_ttoi(strStates);
		if(nStates<=0) nStates=1;
		pImgView->SetStates(nStates);
		pImgView->SetTile(FindChildByName("chk_imglst_tile")->IsChecked());
		pImgView->SetVertical(FindChildByName("chk_imglst_vert")->IsChecked());
		pImgView->NotifyInvalidate();
	}

}

LRESULT CNewSkinDlg::OnEditNotify_imgframe( LPDUINMHDR pNHdr )
{
	LPDUIRICHEDITNOTIFY pEditNotify=(LPDUIRICHEDITNOTIFY)pNHdr;
	if(pEditNotify->iNotify==EN_CHANGE)
	{
		UpdateImgframePreview();
	}
	return S_OK;
}


void CNewSkinDlg::UpdateImgframePreview()
{
	CDuiRichEdit *pEdit=FindChildByName2<CDuiRichEdit*>("edit_imgframe_src_name");
	CDuiStringT strSrcName=CUIHelper::GetEditText(pEdit);
	CDuiStringT strSrcFile=m_pMainDlg->GetImageSrcFile(strSrcName);
	if(!strSrcFile.IsEmpty())
	{
		CSkinView_ImgFrame *pImgView=FindChildByName2<CSkinView_ImgFrame*>("imgview_imgframe");
		pImgView->SetImageFile(strSrcFile);
		CDuiStringT strStates=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imgframe_states"));
		int nStates=_ttoi(strStates);
		if(nStates<=0) nStates=1;
		pImgView->SetStates(nStates);
		
		CDuiStringT strMargin=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imgframe_frame"));
		CRect rcMargin;
		int nsegs=_stscanf(strMargin,_T("%d,%d,%d,%d"),&rcMargin.left,&rcMargin.top,&rcMargin.right,&rcMargin.bottom);
		if(nsegs==4)
		{
			pImgView->SetMargin(rcMargin);
		}

		pImgView->SetTile(FindChildByName("chk_imgframe_tile")->IsChecked());
		pImgView->SetVertical(FindChildByName("chk_imgframe_vert")->IsChecked());

		pImgView->NotifyInvalidate();
	}

}

CDuiStringA Color2Hex(COLORREF cr)
{
	CDuiStringA str;
	str.Format("%02x%02x%02x",GetRValue(cr),GetGValue(cr),GetBValue(cr));
	return str;
}

void CNewSkinDlg::OnOK()
{
	xml_document xmlInit;
	if(xmlInit.load_file(m_pMainDlg->m_strInitFile))
	{
		xml_node xmlSkins=xmlInit.first_child().child("skins");
		if(!xmlSkins) xmlSkins=xmlInit.first_child().append_child("skins");
		CDuiStringT strSkinName=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_skin_name"));
		if(strSkinName.IsEmpty())
		{
			DuiMessageBox(GetActiveWindow(),_T("没有指定的皮肤名"),_T("错误"),MB_OK|MB_ICONSTOP);
			return;
		}
		CDuiStringA strSkinNameA=DUI_CT2A(strSkinName,CP_UTF8);
		{
			//检查名字重复
			xml_node xmlSkin=xmlSkins.first_child();
			while(xmlSkin)
			{
				if(strSkinNameA==xmlSkin.attribute("name").value())
				{
					DuiMessageBox(GetActiveWindow(),_T("指定的皮肤名与现有皮肤重复"),_T("错误"),MB_OK|MB_ICONSTOP);
					return;
				}
				xmlSkin=xmlSkin.next_sibling();
			}
		}

		CDuiTabCtrl *pTabSkinType=FindChildByName2<CDuiTabCtrl*>("tab_skin_type");
		char szTypes[][20]={
			"imglst","imgframe","scrollbar","button","gradation"
		};
		int iType=pTabSkinType->GetCurSel();
		xml_node xmlSkin=xmlSkins.append_child(szTypes[iType]);

		xmlSkin.append_attribute("name").set_value(strSkinNameA);
		switch(iType)
		{
		case 0://imglst
			{
				CDuiStringT strSrcName=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imglst_src_name"));
				xmlSkin.append_attribute("src").set_value(DUI_CT2A(strSrcName,CP_UTF8));
				CDuiStringT strStates=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imglst_states"));
				int nStates=_ttoi(strStates);
				if(nStates!=0) xmlSkin.append_attribute("states").set_value(nStates);
				if(FindChildByName("chk_imglst_vert")->IsChecked())
					xmlSkin.append_attribute("vertical").set_value("1");
				if(FindChildByName("chk_imglst_tile")->IsChecked())
					xmlSkin.append_attribute("tile").set_value("1");
			}
			break;
		case 1://imgframe
			{
				CDuiStringT strSrcName=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imgframe_src_name"));
				xmlSkin.append_attribute("src").set_value(DUI_CT2A(strSrcName,CP_UTF8));
				CDuiStringT strStates=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imgframe_states"));
				int nStates=_ttoi(strStates);
				if(nStates!=0) xmlSkin.append_attribute("states").set_value(nStates);
				if(FindChildByName("chk_imgframe_vert")->IsChecked())
					xmlSkin.append_attribute("vertical").set_value("1");
				if(FindChildByName("chk_imgframe_tile")->IsChecked())
					xmlSkin.append_attribute("tile").set_value("1");

				CDuiStringT strFrame=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imgframe_frame"));
				CRect rcMargin;
				int nSegs=_stscanf(strFrame,_T("%d,%d,%d,%d"),&rcMargin.left,&rcMargin.top,&rcMargin.right,&rcMargin.bottom);
				if(nSegs==4)
				{
					xmlSkin.append_attribute("left").set_value(rcMargin.left);
					xmlSkin.append_attribute("top").set_value(rcMargin.top);
					xmlSkin.append_attribute("right").set_value(rcMargin.right);
					xmlSkin.append_attribute("bottom").set_value(rcMargin.bottom);
				}
			}
			break;
		case 2://scrollbar
			{
				CDuiStringT strSrcName=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_imgframe_src_name"));
				xmlSkin.append_attribute("src").set_value(DUI_CT2A(strSrcName,CP_UTF8));
				CDuiStringT strMargin=CUIHelper::GetEditText(FindChildByName2<CDuiRichEdit*>("edit_scrollbar_margin"));
				int nMargin=_ttoi(strMargin);
				if(nMargin>0) xmlSkin.append_attribute("margin").set_value(nMargin);
				if(FindChildByName("chk_scrollbar_hasgripper")->IsChecked())
					xmlSkin.append_attribute("hasgripper").set_value("1");
				if(FindChildByName("chk_scrollbar_inactive")->IsChecked())
					xmlSkin.append_attribute("hasinactive").set_value("1");
			}
			break;
		case 3://button
			{
				xmlSkin.append_attribute("bgup").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt1_up")->GetColor()));
				xmlSkin.append_attribute("bgdown").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt1_down")->GetColor()));
				xmlSkin.append_attribute("bguphover").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt2_up")->GetColor()));
				xmlSkin.append_attribute("bgdownhover").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt2_down")->GetColor()));
				xmlSkin.append_attribute("bguppush").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt3_up")->GetColor()));
				xmlSkin.append_attribute("bgdownpush").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt3_down")->GetColor()));
				xmlSkin.append_attribute("bgupdisable").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt4_up")->GetColor()));
				xmlSkin.append_attribute("bgdowndisable").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt4_down")->GetColor()));
				xmlSkin.append_attribute("border").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_bt_border")->GetColor()));
			}
			break;
		case 4://gradation
			{
				xmlSkin.append_attribute("from").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_gradation_from")->GetColor()));
				xmlSkin.append_attribute("to").set_value(Color2Hex(FindChildByName2<CDuiColorPicker*>("crpk_gradation_to")->GetColor()));
				xmlSkin.append_attribute("dir").set_value(FindChildByName("chk_gradation_dir")->IsChecked()?"vert":"horz");
			}
			break;
		}

		//save xml
		FILE *f=_tfopen(m_pMainDlg->m_strInitFile,_T("wb"));
		if(f)
		{
			xml_writer_file xmlPrinter(f);
			xmlInit.print(xmlPrinter);
			fclose(f);
			m_pMainDlg->InitSkinList();
		}

	}
	EndDialog(IDOK);
}

LRESULT CNewSkinDlg::OnCrChange_Button_Border( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("border",pNmColor->crSel);
	return S_OK;
}

LRESULT CNewSkinDlg::OnCrChange_Button_Normal_Up( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bgup",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Normal_Down( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bgdown",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Hover_Up( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bguphover",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Hover_Down( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bgdownhover",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Pushdown_Up( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bguppush",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Pushdown_Down( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bgdownpush",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Disable_Up( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bgupdisable",pNmColor->crSel);
	return S_OK;
}


LRESULT CNewSkinDlg::OnCrChange_Button_Disable_Down( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	SetButtonSkinColor("bgdowndisable",pNmColor->crSel);
	return S_OK;
}

void CNewSkinDlg::SetButtonSkinColor( const CDuiStringA &strAttr,COLORREF cr )
{
	CSkinView_Button *pImgView=FindChildByName2<CSkinView_Button*>("imgview_button");
	DUIASSERT(pImgView);
	CDuiStringA strColor;
	strColor.Format("%02x%02x%02x",GetRValue(cr),GetGValue(cr),GetBValue(cr));
	pImgView->GetButtonSkin()->SetAttribute(strAttr,strColor);
	pImgView->NotifyInvalidate();
}

LRESULT CNewSkinDlg::OnCrChange_Gradataion_From( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation*>("imgview_gradation");

	pImgView->GetGradationSkin()->SetColorFrom(pNmColor->crSel);
	pImgView->NotifyInvalidate();
	return S_OK;
}

LRESULT CNewSkinDlg::OnCrChange_Gradataion_To( LPDUINMHDR pNHdr )
{
	LPDUINMCOLORCHANGE pNmColor=(LPDUINMCOLORCHANGE)pNHdr;
	CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation*>("imgview_gradation");

	pImgView->GetGradationSkin()->SetColorTo(pNmColor->crSel);
	pImgView->NotifyInvalidate();
	return S_OK;
}

void CNewSkinDlg::OnGradationDir()
{
	BOOL bVertical=FindChildByName("chk_gradation_dir")->IsChecked();
	CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation*>("imgview_gradation");

	pImgView->GetGradationSkin()->SetVertical(bVertical);
	pImgView->NotifyInvalidate();
}