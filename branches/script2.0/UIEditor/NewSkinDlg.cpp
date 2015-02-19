#include "StdAfx.h"
#include "NewSkinDlg.h"
#include "maindlg.h"

CNewSkinDlg::CNewSkinDlg(CMainDlg *pMainDlg)
:SHostDialog(_T("layout:dlg_newskin"))
,m_pMainDlg(pMainDlg)
{
}

CNewSkinDlg::~CNewSkinDlg(void)
{
}

LRESULT CNewSkinDlg::OnInitDialog( HWND wnd, LPARAM lParam )
{
	FindChildByName2<SRichEdit>(L"edit_imglist_src_name")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<SRichEdit>(L"edit_imglist_states")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<SRichEdit>(L"edit_imgframe_src_name")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<SRichEdit>(L"edit_imgframe_states")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<SRichEdit>(L"edit_imgframe_frame")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<SRichEdit>(L"edit_scrollbar_src_name")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	FindChildByName2<SRichEdit>(L"edit_imgframe_frame")->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
	return 0;
}

void CNewSkinDlg::OnEditNotify_imglist( EventArgs * pEvt )
{
    EventRENotify *pEvt2 = (EventRENotify*)pEvt;
	if(pEvt2->iNotify==EN_CHANGE)
	{
		UpdateimglistPreview();
	}
}

void CNewSkinDlg::UpdateimglistPreview()
{
	SRichEdit *pEdit=FindChildByName2<SRichEdit>(L"edit_imglist_src_name");
	SStringT strSrcName=pEdit->GetWindowText();
	SStringT strSrcFile=m_pMainDlg->GetImageSrcFile(strSrcName);
	if(!strSrcFile.IsEmpty())
	{
		CSkinView_ImgList *pImgView=FindChildByName2<CSkinView_ImgList>(L"imgview_imglist");
		pImgView->SetImageFile(strSrcFile);
		SStringT strStates=FindChildByName2<SRichEdit>(L"edit_imglist_states")->GetWindowText();
		int nStates=_ttoi(strStates);
		if(nStates<=0) nStates=1;
		pImgView->SetStates(nStates);
		pImgView->SetTile(FindChildByName(L"chk_imglist_tile")->IsChecked());
		pImgView->SetVertical(FindChildByName(L"chk_imglist_vert")->IsChecked());
		pImgView->Invalidate();
	}

}

void CNewSkinDlg::OnEditNotify_imgframe( EventArgs * pEvt )
{
    EventRENotify *pEvt2 = (EventRENotify*)pEvt;
    if(pEvt2->iNotify==EN_CHANGE)
	{
		UpdateImgframePreview();
	}
}


void CNewSkinDlg::UpdateImgframePreview()
{
	SRichEdit *pEdit=FindChildByName2<SRichEdit>(L"edit_imgframe_src_name");
	SStringT strSrcName=pEdit->GetWindowText();
	SStringT strSrcFile=m_pMainDlg->GetImageSrcFile(strSrcName);
	if(!strSrcFile.IsEmpty())
	{
		CSkinView_ImgFrame *pImgView=FindChildByName2<CSkinView_ImgFrame>(L"imgview_imgframe");
		pImgView->SetImageFile(strSrcFile);
		SStringT strStates=FindChildByName2<SRichEdit>(L"edit_imgframe_states")->GetWindowText();
		int nStates=_ttoi(strStates);
		if(nStates<=0) nStates=1;
		pImgView->SetStates(nStates);
		
		SStringT strMargin=FindChildByName2<SRichEdit>(L"edit_imgframe_frame")->GetWindowText();
		CRect rcMargin;
		int nsegs=_stscanf(strMargin,_T("%d,%d,%d,%d"),&rcMargin.left,&rcMargin.top,&rcMargin.right,&rcMargin.bottom);
		if(nsegs==4)
		{
			pImgView->SetMargin(rcMargin);
		}

		pImgView->SetTile(FindChildByName(L"chk_imgframe_tile")->IsChecked());
		pImgView->SetVertical(FindChildByName(L"chk_imgframe_vert")->IsChecked());

		pImgView->Invalidate();
	}

}

SStringW Color2Hex(COLORREF cr)
{
	SStringW str;
    str.Format(L"#%02x%02x%02x%02x",GetRValue(cr),GetGValue(cr),GetBValue(cr),GetAValue(cr));
	return str;
}

void CNewSkinDlg::OnOK()
{
	xml_document xmlInit;
	if(xmlInit.load_file(m_pMainDlg->m_strInitFile))
	{
		xml_node xmlSkins=xmlInit.first_child().child(L"skins");
		if(!xmlSkins) xmlSkins=xmlInit.first_child().append_child(L"skins");
		SStringT strSkinName=FindChildByName2<SRichEdit>(L"edit_skin_name")->GetWindowText();
		if(strSkinName.IsEmpty())
		{
			SMessageBox(GetActiveWindow(),_T("没有指定的皮肤名"),_T("错误"),MB_OK|MB_ICONSTOP);
			return;
		}
		SStringW strSkinNameW=S_CT2W(strSkinName);
		{
			//检查名字重复
			xml_node xmlSkin=xmlSkins.first_child();
			while(xmlSkin)
			{
				if(strSkinNameW==xmlSkin.attribute(L"name").value())
				{
					SMessageBox(GetActiveWindow(),_T("指定的皮肤名与现有皮肤重复"),_T("错误"),MB_OK|MB_ICONSTOP);
					return;
				}
				xmlSkin=xmlSkin.next_sibling();
			}
		}

		STabCtrl *pTabSkinType=FindChildByName2<STabCtrl>(L"tab_skin_type");
		wchar_t szTypes[][20]={
			L"imglist",L"imgframe",L"scrollbar",L"button",L"gradation"
		};
		int iType=pTabSkinType->GetCurSel();
		xml_node xmlSkin=xmlSkins.append_child(szTypes[iType]);

		xmlSkin.append_attribute(L"name").set_value(strSkinNameW);
		switch(iType)
		{
		case 0://imglist
			{
				SStringT strSrcName=FindChildByName2<SRichEdit>(L"edit_imglist_src_name")->GetWindowText();
				xmlSkin.append_attribute(L"src").set_value(strSkinNameW);
				SStringT strStates=FindChildByName2<SRichEdit>(L"edit_imglist_states")->GetWindowText();
				int nStates=_ttoi(strStates);
				if(nStates!=0) xmlSkin.append_attribute(L"states").set_value(nStates);
				if(FindChildByName(L"chk_imglist_vert")->IsChecked())
					xmlSkin.append_attribute(L"vertical").set_value(L"1");
				if(FindChildByName(L"chk_imglist_tile")->IsChecked())
					xmlSkin.append_attribute(L"tile").set_value(L"1");
			}
			break;
		case 1://imgframe
			{
				SStringT strSrcName=FindChildByName2<SRichEdit>(L"edit_imgframe_src_name")->GetWindowText();
				xmlSkin.append_attribute(L"src").set_value(strSkinNameW);
				SStringT strStates=FindChildByName2<SRichEdit>(L"edit_imgframe_states")->GetWindowText();
				int nStates=_ttoi(strStates);
				if(nStates!=0) xmlSkin.append_attribute(L"states").set_value(nStates);
				if(FindChildByName(L"chk_imgframe_vert")->IsChecked())
					xmlSkin.append_attribute(L"vertical").set_value(L"1");
				if(FindChildByName(L"chk_imgframe_tile")->IsChecked())
					xmlSkin.append_attribute(L"tile").set_value(L"1");

				SStringT strFrame=FindChildByName2<SRichEdit>(L"edit_imgframe_frame")->GetWindowText();
				CRect rcMargin;
				int nSegs=_stscanf(strFrame,_T("%d,%d,%d,%d"),&rcMargin.left,&rcMargin.top,&rcMargin.right,&rcMargin.bottom);
				if(nSegs==4)
				{
					xmlSkin.append_attribute(L"left").set_value(rcMargin.left);
					xmlSkin.append_attribute(L"top").set_value(rcMargin.top);
					xmlSkin.append_attribute(L"right").set_value(rcMargin.right);
					xmlSkin.append_attribute(L"bottom").set_value(rcMargin.bottom);
				}
			}
			break;
		case 2://scrollbar
			{
				SStringT strSrcName=FindChildByName2<SRichEdit>(L"edit_imgframe_src_name")->GetWindowText();
				xmlSkin.append_attribute(L"src").set_value(strSkinNameW);
				SStringT strMargin=FindChildByName2<SRichEdit>(L"edit_scrollbar_margin")->GetWindowText();
				int nMargin=_ttoi(strMargin);
				if(nMargin>0) xmlSkin.append_attribute(L"margin").set_value(nMargin);
				if(FindChildByName(L"chk_scrollbar_hasgripper")->IsChecked())
					xmlSkin.append_attribute(L"hasGripper").set_value(L"1");
				if(FindChildByName(L"chk_scrollbar_inactive")->IsChecked())
					xmlSkin.append_attribute(L"hasInactive").set_value(L"1");
			}
			break;
		case 3://button
			{
				xmlSkin.append_attribute(L"colorUp").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt1_up")->GetColor()));
				xmlSkin.append_attribute(L"colorDown").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt1_down")->GetColor()));
				xmlSkin.append_attribute(L"colorUpHover").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt2_up")->GetColor()));
				xmlSkin.append_attribute(L"colorDownHover").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt2_down")->GetColor()));
				xmlSkin.append_attribute(L"colorUpPush").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt3_up")->GetColor()));
				xmlSkin.append_attribute(L"colorDownPush").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt3_down")->GetColor()));
				xmlSkin.append_attribute(L"colorUpDisable").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt4_up")->GetColor()));
				xmlSkin.append_attribute(L"colorDownDisable").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt4_down")->GetColor()));
				xmlSkin.append_attribute(L"colorBorder").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_bt_border")->GetColor()));
			}
			break;
		case 4://gradation
			{
				xmlSkin.append_attribute(L"from").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_gradation_from")->GetColor()));
				xmlSkin.append_attribute(L"to").set_value(Color2Hex(FindChildByName2<SColorPicker>(L"crpk_gradation_to")->GetColor()));
				xmlSkin.append_attribute(L"dir").set_value(FindChildByName(L"chk_gradation_dir")->IsChecked()?L"vert":L"horz");
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

void CNewSkinDlg::OnCrChange_Button_Border( EventArgs * pEvt )
{
    EventColorChange *pEvt2 = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorBorder",pEvt2->crSel);
}

void CNewSkinDlg::OnCrChange_Button_Normal_Up( EventArgs * pEvt )
{
    EventColorChange *pEvt2 = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorUp",pEvt2->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Normal_Down( EventArgs * pEvt )
{
    EventColorChange *pEvt2 = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorDown",pEvt2->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Hover_Up( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorUpHover",pNmColor->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Hover_Down( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorDownHover",pNmColor->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Pushdown_Up( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorUpPush",pNmColor->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Pushdown_Down( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorDownPush",pNmColor->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Disable_Up( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorUpDisable",pNmColor->crSel);
}


void CNewSkinDlg::OnCrChange_Button_Disable_Down( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	SetButtonSkinColor(L"colorDownDisable",pNmColor->crSel);
}

void CNewSkinDlg::SetButtonSkinColor( const SStringW &strAttr,COLORREF cr )
{
	CSkinView_Button *pImgView=FindChildByName2<CSkinView_Button>(L"imgview_button");
	SASSERT(pImgView);
	pImgView->GetButtonSkin()->SetAttribute(strAttr,Color2Hex(cr));
	pImgView->Invalidate();
}

void CNewSkinDlg::OnCrChange_Gradataion_From( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation>(L"imgview_gradation");

	pImgView->GetGradationSkin()->SetColorFrom(pNmColor->crSel);
	pImgView->Invalidate();
}

void CNewSkinDlg::OnCrChange_Gradataion_To( EventArgs * pEvt )
{
    EventColorChange *pNmColor = (EventColorChange*)pEvt;
	CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation>(L"imgview_gradation");

	pImgView->GetGradationSkin()->SetColorTo(pNmColor->crSel);
	pImgView->Invalidate();
}

void CNewSkinDlg::OnGradationDir()
{
	BOOL bVertical=FindChildByName(L"chk_gradation_dir")->IsChecked();
	CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation>(L"imgview_gradation");

	pImgView->GetGradationSkin()->SetVertical(bVertical);
	pImgView->Invalidate();
}