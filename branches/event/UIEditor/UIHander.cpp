#include "StdAfx.h"
#include "UIHander.h"

#include "MainDlg.h"

#include "ScintillaWnd.h"
#include "ImgView.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atldlgs.h>

using namespace pugi;

CUIHander::CUIHander(CMainDlg * pMainDlg) : m_pMainDlg(pMainDlg)
{
}

CUIHander::~CUIHander(void)
{
}

LRESULT CUIHander::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SetMsgHandled(FALSE); 
    return 0; 
}

void CUIHander::OnBtnClick_ProjectOpen()
{
	CFileDialog fileDlg(TRUE,_T("xml"),_T("index.xml"),6,_T("xml files(*.xml)\0*.xml\0All files (*.*)\0*.*\0\0"));
	if(IDOK==fileDlg.DoModal())
	{
		xml_document xmlDoc;
		if(xmlDoc.load_file(fileDlg.m_szFileName))
		{
			xml_node xmlNode=xmlDoc.child("resid");

			CDuiListCtrl * pListFile=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("prj_list_file");
			CDuiListCtrl * pListSkin=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("prj_list_skin");
			DUIASSERT(pListSkin && pListFile);

			pListFile->DeleteAllItems();
			ClearSkinList();

			m_strPrjPath=fileDlg.m_szFileName;
			int index=m_strPrjPath.ReverseFind(_T('\\'));
			m_strPrjPath=m_strPrjPath.Left(index+1);

			m_strInitFile=_T("xml\\init.xml");

			int iItem=0;
			while(xmlNode)
			{
				CDuiStringT strType=DUI_CA2T(xmlNode.attribute("type").value(),CP_UTF8);
				pListFile->InsertItem(iItem,strType);
				pListFile->SetSubItemText(iItem,1,DUI_CA2T(xmlNode.attribute("name").value(),CP_UTF8));
				CDuiStringT strFile=DUI_CA2T(xmlNode.attribute("file").value(),CP_UTF8);
				pListFile->SetSubItemText(iItem,2,strFile);
				if(strType==_T("XML") && xmlNode.attribute("init").as_bool())
				{//获得初始化XML，包含skin等元素
					m_strInitFile=strFile;
				}
				pListFile->SetSubItemText(iItem,3,DUI_CA2T(xmlNode.attribute("layer").value(),CP_UTF8));
				iItem++;
				xmlNode=xmlNode.next_sibling("resid");
			}

			//加载skins
			xml_document xmlInit;
			if(xmlInit.load_file(m_strPrjPath+m_strInitFile))
			{
				xml_node xmlNode=xmlInit.first_child().child("skins");
				if(xmlNode)
				{
					iItem=0;
					xmlNode=xmlNode.first_child();
					while(xmlNode)
					{
						SkinInfo *pSkinInfo=new SkinInfo;
						pSkinInfo->strType=DUI_CA2T(xmlNode.name(),CP_UTF8);
						pSkinInfo->strName=DUI_CA2T(xmlNode.attribute("name").value(),CP_UTF8);
						pSkinInfo->strSrc=DUI_CA2T(xmlNode.attribute("src").value(),CP_UTF8);
						pSkinInfo->bTile=xmlNode.attribute("tile").as_bool(false);
						pSkinInfo->nState=xmlNode.attribute("states").as_int(1);
						if(pSkinInfo->strType==_T("imgframe"))
						{
							pSkinInfo->rcMargin.left=xmlNode.attribute("left").as_int(0);
							pSkinInfo->rcMargin.right=xmlNode.attribute("right").as_int(-1);
							pSkinInfo->rcMargin.top=xmlNode.attribute("top").as_int(0);
							pSkinInfo->rcMargin.bottom=xmlNode.attribute("bottom").as_int(-1);
							if(pSkinInfo->rcMargin.right == -1) pSkinInfo->rcMargin.right=pSkinInfo->rcMargin.left;
							if(pSkinInfo->rcMargin.bottom == -1) pSkinInfo->rcMargin.bottom=pSkinInfo->rcMargin.top;
						}

						pListSkin->InsertItem(iItem,pSkinInfo->strType);
						pListSkin->SetSubItemText(iItem,1,pSkinInfo->strName);
						pListSkin->SetSubItemText(iItem,2,pSkinInfo->strSrc);

						pListSkin->SetItemData(iItem,(DWORD)pSkinInfo);

						iItem++;
						xmlNode=xmlNode.next_sibling();
					}
				}
			}
		}
	}
}

LRESULT CUIHander::OnListCtrl_File_SelChanged( LPDUINMHDR pnmh )
{
	DUINMLBSELCHANGE *pLCNmhdr=(DUINMLBSELCHANGE*)pnmh;
	if(pLCNmhdr->nNewSel!=-1)
	{
		CDuiListCtrl * pListFile=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("prj_list_file");
		CDuiTabCtrl * pTabView=m_pMainDlg->FindChildByName2<CDuiTabCtrl *>("tab_view");
		TCHAR szBuf[1025]={0};
		DXLVSUBITEM item;
		item.mask=DUI_LVIF_TEXT;
		item.cchTextMax=1024;
		item.strText=szBuf;
		pListFile->GetSubItem(pLCNmhdr->nNewSel,0,&item);
		if(_tcscmp(szBuf,_T("XML"))==0)
		{
			pTabView->SetCurSel(1);
			pListFile->GetSubItem(pLCNmhdr->nNewSel,2,&item);
			CDuiStringT strXmlPath=m_strPrjPath+szBuf;
			CDuiRealWnd *pRealWnd=m_pMainDlg->FindChildByName2<CDuiRealWnd *>("xmleditor");
			CScintillaWnd *pXmlEditor=(CScintillaWnd *)pRealWnd->GetUserData();
			if(pXmlEditor) pXmlEditor->OpenFile(strXmlPath);
		}else
		{
			pListFile->GetSubItem(pLCNmhdr->nNewSel,2,&item);
			CDuiStringT strImgPath=m_strPrjPath+szBuf;
			pTabView->SetCurSel(0);
			CImgView *pImgView=m_pMainDlg->FindChildByName2<CImgView*>("img_view");
			if(pImgView) pImgView->SetImageFile(strImgPath);
		}
	}
	return 0;
}

LRESULT CUIHander::OnListCtrl_Skin_SelChanged( LPDUINMHDR pnmh )
{
	CDuiListCtrl * pListSkin=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("prj_list_skin");
	DUINMLBSELCHANGE *pLCNmhdr=(DUINMLBSELCHANGE*)pnmh;
	if(pLCNmhdr->nNewSel!=-1 && pListSkin)
	{
		SkinInfo *pSkinInfo=(SkinInfo *)pListSkin->GetItemData(pLCNmhdr->nNewSel);
		if(!pSkinInfo->strSrc.IsEmpty())
		{
			CDuiStringT strImgPath=GetImageSrcFile(pSkinInfo->strSrc);
			if(!strImgPath.IsEmpty())
			{
				CImgView *pImgView=m_pMainDlg->FindChildByName2<CImgView*>("img_view");
				if(pImgView)
				{
					pImgView->SetImageFile(strImgPath);
					pImgView->SetStates(pSkinInfo->nState);
					pImgView->SetTile(pSkinInfo->bTile);
					pImgView->SetMargin(pSkinInfo->rcMargin);
				}
			}
		}
	}
	return 0;
}

void CUIHander::OnDestroy()
{
	ClearSkinList();
	SetMsgHandled(FALSE);
}

void CUIHander::ClearSkinList()
{
	CDuiListCtrl * pListSkin=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("prj_list_skin");
	DUIASSERT(pListSkin);
	for(int i=0;i<pListSkin->GetItemCount();i++)
	{
		SkinInfo *pSkinInfo=(SkinInfo*)pListSkin->GetItemData(i);
		delete pSkinInfo;
	}
	pListSkin->DeleteAllItems();
}

CDuiStringT CUIHander::GetImageSrcFile( const CDuiStringT & strSrcName )
{
	CDuiListCtrl * pListFile=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("prj_list_file");
	if(pListFile)
	{
		TCHAR szBuf[1025]={0};
		DXLVSUBITEM item;
		item.mask=DUI_LVIF_TEXT;
		item.cchTextMax=1024;
		item.strText=szBuf;
		for(int i=0;i<pListFile->GetItemCount();i++)
		{
			pListFile->GetSubItem(i,1,&item);
			if(strSrcName==szBuf)
			{
				pListFile->GetSubItem(i,2,&item);
				return szBuf;
			}
		}
	}
	return _T("");
}