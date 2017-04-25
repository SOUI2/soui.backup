// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"

#include "NewFileDlg.h"
#include "NewSkinDlg.h"
#include <helper/splitstring.h>

CMainDlg::CMainDlg() : SHostDialog(_T("layout:dlg_main"))
{
} 

CMainDlg::~CMainDlg()
{
}


void CMainDlg::OnBtnClick_ProjectOpen()
{
	CFileDialogEx fileDlg(TRUE,_T("idx"),_T("uires.idx"),6,_T("soui skin index(*.idx)\0*.idx\0All files (*.*)\0*.*\0\0"));
	if(IDOK==fileDlg.DoModal())
	{
		OpenProject(fileDlg.m_szFileName);
	}
}

BOOL CMainDlg::OnListCtrl_File_SelChanged( EventArgs *pEvt )
{
    EventLCSelChanged *pEvtLCSel = (EventLCSelChanged*)pEvt;

    if(pEvtLCSel->nNewSel!=-1)
    {
        SListCtrl * pListFile=FindChildByName2<SListCtrl >(L"prj_list_file");
        STabCtrl * pTabView=FindChildByName2<STabCtrl >(L"tab_view");
        TCHAR szBuf[1025]={0};
        DXLVSUBITEM item;
        item.mask=S_LVIF_TEXT;
        item.cchTextMax=1024;
        item.strText=szBuf;
        pListFile->GetSubItem(pEvtLCSel->nNewSel,0,&item);
        SStringT strType=szBuf;
        strType.MakeLower();
        if(strType == _T("xml") || strType == _T("uidef") || strType == _T("layout") || strType == _T("translator"))
        {
            pTabView->SetCurSel(1);
            pListFile->GetSubItem(pEvtLCSel->nNewSel,2,&item);
            SStringT strXmlPath=m_strPrjPath+szBuf;
            SRealWnd *pRealWnd=FindChildByName2<SRealWnd >(L"xmleditor");
            CScintillaWnd *pXmlEditor=(CScintillaWnd *)pRealWnd->GetUserData();
            if(pXmlEditor) pXmlEditor->OpenFile(strXmlPath);
        }else
        {
            pListFile->GetSubItem(pEvtLCSel->nNewSel,2,&item);
            SStringT strImgPath=m_strPrjPath+szBuf;
            pTabView->SetCurSel(0);
            FindChildByName2<STabCtrl>(L"skinview_tab")->SetCurSel(_T("imglist"));
            CSkinView_ImgList *pImgView=FindChildByName2<CSkinView_ImgList>(L"skinview_imglist");
            if(pImgView)
            {
                pImgView->SetImageFile(strImgPath);
                pImgView->SetTile(0);
                pImgView->SetStates(1);
                pImgView->SetVertical(0);
            }
        }
    }
    return 0;
}

BOOL CMainDlg::OnListCtrl_Skin_SelChanged( EventArgs *pEvt )
{
    EventLCSelChanged *pEvt2 = (EventLCSelChanged*)pEvt;
    SListCtrl * pListSkin=FindChildByName2<SListCtrl >(L"prj_list_skin");
    
    STabCtrl * pTabView=FindChildByName2<STabCtrl >(L"tab_view");
    pTabView->SetCurSel(0);
    
    if(pEvt2->nNewSel!=-1 && pListSkin)
    {
        SkinInfo *pSkinInfo=(SkinInfo *)pListSkin->GetItemData(pEvt2->nNewSel);
        SStringT strImgPath=GetImageSrcFile(pSkinInfo->strSrc);
        if(pSkinInfo->strType == _T("imgframe"))
        {
            FindChildByName2<STabCtrl>(L"skinview_tab")->SetCurSel(_T("imgframe"));
            CSkinView_ImgFrame *pImgView=FindChildByName2<CSkinView_ImgFrame>(L"skinview_imgframe");
            if(pImgView)
            {
                pImgView->SetImageFile(strImgPath);
                pImgView->SetStates(pSkinInfo->nState);
                pImgView->SetTile(pSkinInfo->bTile);
                pImgView->SetMargin(pSkinInfo->rcMargin);
            }
        }else if(pSkinInfo->strType==_T("imglist"))
        {
            FindChildByName2<STabCtrl>(L"skinview_tab")->SetCurSel(_T("imglist"));
            CSkinView_ImgList *pImgView=FindChildByName2<CSkinView_ImgList>(L"skinview_imglist");
            if(pImgView)
            {
                pImgView->SetImageFile(strImgPath);
                pImgView->SetStates(pSkinInfo->nState);
                pImgView->SetTile(pSkinInfo->bTile);
            }
        }else if(pSkinInfo->strType==_T("button"))
        {
            FindChildByName2<STabCtrl>(L"skinview_tab")->SetCurSel(_T("button"));
            CSkinView_Button *pImgView=FindChildByName2<CSkinView_Button>(L"skinview_button");
            if(pImgView)
            {
                pImgView->GetButtonSkin()->SetColors(pSkinInfo->crUp,pSkinInfo->crDown,pSkinInfo->crBorder);
            }
        }else if(pSkinInfo->strType==_T("scrollbar"))
        {
            FindChildByName2<STabCtrl>(L"skinview_tab")->SetCurSel(_T("imglist"));
            CSkinView_ImgList *pImgView=FindChildByName2<CSkinView_ImgList>(L"skinview_imglist");
            if(pImgView)
            {
                pImgView->SetImageFile(strImgPath);
                pImgView->SetStates(1);
            }
        }else if(pSkinInfo->strType==_T("gradation"))
        {
            FindChildByName2<STabCtrl>(L"skinview_tab")->SetCurSel(_T("gradation"));
            CSkinView_Gradation *pImgView=FindChildByName2<CSkinView_Gradation>(L"skinview_gradation");
            if(pImgView)
            {
                pImgView->GetGradationSkin()->SetColorFrom(pSkinInfo->cr1);
                pImgView->GetGradationSkin()->SetColorTo(pSkinInfo->cr2);
                pImgView->GetGradationSkin()->SetVertical(pSkinInfo->dir);
            }
        }
    }
    return 0;
}

void CMainDlg::OnDestroy()
{
	ClearSkinList();
	SetMsgHandled(FALSE);
}

void CMainDlg::ClearSkinList()
{
	SListCtrl * pListSkin=FindChildByName2<SListCtrl >(L"prj_list_skin");
	if(pListSkin)
	{
		for(int i=0;i<pListSkin->GetItemCount();i++)
		{
			SkinInfo *pSkinInfo=(SkinInfo*)pListSkin->GetItemData(i);
			delete pSkinInfo;
		}
		pListSkin->DeleteAllItems();
	}
}

SStringT CMainDlg::GetImageSrcFile( const SStringT & strSrcName )
{
	SListCtrl * pListFile=FindChildByName2<SListCtrl >(L"prj_list_file");
	if(pListFile)
	{
        SStringTList lstName ;
        int nSegs = SplitString(strSrcName,_T(':'),lstName); 
        for(int j=0;j<lstName.GetCount();j++)
            lstName[j].MakeLower();

		for(int i=0;i<pListFile->GetItemCount();i++)
		{
            SStringT strType = pListFile->GetSubItemText(i,0);
            SStringT strName = pListFile->GetSubItemText(i,1);
            strType.MakeLower();
            strName.MakeLower();
			if((nSegs == 2 && lstName[0]==strType && lstName[1]==strName)
                || nSegs == 1 && lstName[0]==strName)
			{
                return pListFile->GetSubItemText(i,2);
			}
		}
	}
	return _T("");
}

void CMainDlg::OnBtnClick_Project_File_Add()
{
	CNewFileDlg dlg(this);
	if(!m_strPrjPath.IsEmpty() && IDOK==dlg.DoModal())
	{
		xml_document xmlDoc;
		if(!xmlDoc.load_file(m_strPrjIndex)) return;
        xml_node xmlRoot = xmlDoc.child(L"resource");
        if(!xmlRoot)
        {
            return;
        }
        xml_node xmlType=xmlRoot.child(S_CT2W(dlg.m_strResType));
        xml_node xmlNode=xmlType.child(L"file");
        SStringW strName = S_CT2W(dlg.m_strResName);
        while(xmlNode)
        {
            if(xmlNode.attribute(L"name").value() == strName)
            {
                break;
            }
            xmlNode = xmlNode.next_sibling(L"file");
        }
        if(xmlNode)
        {//检查type,name重复
            SMessageBox(GetActiveWindow(),_T("指定的资源名重复"),_T("错误"),MB_OK|MB_ICONSTOP);
            return;
        }

		BuildFilePath(m_strPrjPath,dlg.m_strResPath,FALSE);
		CopyFile(dlg.m_strSrcFile,m_strPrjPath+_T("\\")+dlg.m_strResPath,FALSE);
		//改写XML文件
        
        if(!xmlType)
        {
            xmlType = xmlRoot.append_child(S_CT2W(dlg.m_strResType));
        }
        xmlNode = xmlType.append_child(L"file");
		xmlNode.append_attribute(L"name").set_value(S_CT2W(dlg.m_strResName));
		xmlNode.append_attribute(L"path").set_value(S_CT2W(dlg.m_strResPath));

		FILE *f=_tfopen(m_strPrjIndex,_T("wb"));
		if(f)
		{
			xml_writer_file xmlfile(f);
 			xmlDoc.print(xmlfile);
			fclose(f);
 			OpenProject(m_strPrjIndex);			
		}

	}
}


BOOL CMainDlg::InitIndexList(LPCTSTR pszIndexFile)
{
	xml_document xmlDoc;
	if(!xmlDoc.load_file(pszIndexFile)) return FALSE;

	xml_node xmlNode=xmlDoc.child(L"resource");

	SListCtrl * pListFile=FindChildByName2<SListCtrl >(L"prj_list_file");

	pListFile->DeleteAllItems();

	m_strInitFile=_T("xml\\init.xml");

    int iItem=0;
    xml_node xmlType = xmlNode.first_child();
    while(xmlType)
    {
        SStringT strType=S_CW2T(xmlType.name());
        xml_node xmlFile = xmlType.child(L"file");
        while(xmlFile)
        {
            pListFile->InsertItem(iItem,strType);
            pListFile->SetSubItemText(iItem,1,S_CW2T(xmlFile.attribute(L"name").value()));
            pListFile->SetSubItemText(iItem,2,S_CW2T(xmlFile.attribute(L"path").value()));
            iItem++;
            xmlFile = xmlFile.next_sibling(L"file");
        }
        xmlType = xmlType.next_sibling();
    }
    
    xml_node xmlInit = xmlType.child(L"uidef");
    if(xmlInit)
    {
        m_strInitFile=S_CW2T(xmlInit.first_child().attribute(L"file").value());
    }
	return TRUE;
}

COLORREF Hex2Color(const SStringW & strColor,COLORREF crDef)
{
	if(strColor.IsEmpty()) return crDef;
	return GETCOLOR(strColor);
}

void CMainDlg::InitSkinList()
{
	SListCtrl * pListSkin=FindChildByName2<SListCtrl >(L"prj_list_skin");

	ClearSkinList();

	//加载skin
	xml_document xmlInit;
	if(xmlInit.load_file(m_strPrjPath+m_strInitFile))
	{
		xml_node xmlNode=xmlInit.first_child().child(L"skin");
		if(xmlNode)
		{
			int iItem=0;
			xmlNode=xmlNode.first_child();
			while(xmlNode)
			{
				SkinInfo *pSkinInfo=new SkinInfo;
				pSkinInfo->strType=S_CW2T(xmlNode.name());
				pSkinInfo->strName=S_CW2T(xmlNode.attribute(L"name").value());
				pSkinInfo->strSrc=S_CW2T(xmlNode.attribute(L"src").value());

				if(pSkinInfo->strType==_T("imglist"))
				{
					pSkinInfo->bTile=xmlNode.attribute(L"tile").as_bool(false);
					pSkinInfo->bVertical=xmlNode.attribute(L"vertical").as_bool(false);
					pSkinInfo->nState=xmlNode.attribute(L"states").as_int(1);
				}

				if(pSkinInfo->strType==_T("imgframe"))
				{
					pSkinInfo->bTile=xmlNode.attribute(L"tile").as_bool(false);
					pSkinInfo->bVertical=xmlNode.attribute(L"vertical").as_bool(false);
					pSkinInfo->nState=xmlNode.attribute(L"states").as_int(1);

					pSkinInfo->rcMargin.left=xmlNode.attribute(L"left").as_int(0);
					pSkinInfo->rcMargin.right=xmlNode.attribute(L"right").as_int(-1);
					pSkinInfo->rcMargin.top=xmlNode.attribute(L"top").as_int(0);
					pSkinInfo->rcMargin.bottom=xmlNode.attribute(L"bottom").as_int(-1);
					if(pSkinInfo->rcMargin.right == -1) pSkinInfo->rcMargin.right=pSkinInfo->rcMargin.left;
					if(pSkinInfo->rcMargin.bottom == -1) pSkinInfo->rcMargin.bottom=pSkinInfo->rcMargin.top;
				}

				if(pSkinInfo->strType==_T("button"))
				{
					pSkinInfo->crBorder=Hex2Color(xmlNode.attribute(L"border").value(),0);
					pSkinInfo->crUp[0]=Hex2Color(xmlNode.attribute(L"bgup").value(),0);
					pSkinInfo->crUp[1]=Hex2Color(xmlNode.attribute(L"bguphover").value(),0);
					pSkinInfo->crUp[2]=Hex2Color(xmlNode.attribute(L"bguppush").value(),0);
					pSkinInfo->crUp[3]=Hex2Color(xmlNode.attribute(L"bgupdisable").value(),0);
					pSkinInfo->crDown[0]=Hex2Color(xmlNode.attribute(L"bgdown").value(),0);
					pSkinInfo->crDown[1]=Hex2Color(xmlNode.attribute(L"bgdownhover").value(),0);
					pSkinInfo->crDown[2]=Hex2Color(xmlNode.attribute(L"bgdownpush").value(),0);
					pSkinInfo->crDown[3]=Hex2Color(xmlNode.attribute(L"bgdowndisable").value(),0);
				}
				
				if(pSkinInfo->strType==_T("gradation"))
				{
					pSkinInfo->cr1=Hex2Color(xmlNode.attribute(L"from").value(),0);
					pSkinInfo->cr2=Hex2Color(xmlNode.attribute(L"to").value(),0);
					pSkinInfo->dir=wcscmp(L"vert",xmlNode.attribute(L"dir").value())==0;
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

BOOL CMainDlg::OpenProject( LPCTSTR pszIndexXml )
{
	if(!InitIndexList(pszIndexXml)) return FALSE;

	m_strPrjIndex=m_strPrjPath=pszIndexXml;
	int index=m_strPrjPath.ReverseFind(_T('\\'));
	m_strPrjPath=m_strPrjPath.Left(index+1);

	InitSkinList();

	return TRUE;
}

void CMainDlg::OnBtnClick_Project_Skin_Add()
{
	CNewSkinDlg dlg(this);
	if(!m_strInitFile.IsEmpty() && IDOK==dlg.DoModal())
	{

	}
}
