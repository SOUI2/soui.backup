#include "StdAfx.h"
#include "UIHander.h"

#include "MainDlg.h"
#include "skinole/ImageOle.h"

class CTestDropTarget:public IDropTarget
{
public:
	CTestDropTarget()
	{
		nRef=0;
	}

	//////////////////////////////////////////////////////////////////////////
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		HRESULT hr=S_FALSE;
		if(riid==__uuidof(IUnknown))
			*ppvObject=(IUnknown*) this,hr=S_OK;
		else if(riid==__uuidof(IDropTarget))
			*ppvObject=(IDropTarget*)this,hr=S_OK;
		if(SUCCEEDED(hr)) AddRef();
		return hr;

	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void){return ++nRef;}

	virtual ULONG STDMETHODCALLTYPE Release( void) { 
		ULONG uRet= -- nRef;
		if(uRet==0) delete this;
		return uRet;
	}

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	virtual HRESULT STDMETHODCALLTYPE DragEnter( 
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect=DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver( 
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect=DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave( void)
	{
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Drop( 
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		FORMATETC format =
		{
			CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
		};
		STGMEDIUM medium;
		if(FAILED(pDataObj->GetData(&format, &medium)))
		{
			return S_FALSE;
		}

		HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

		if(!hdrop)
		{
			return S_FALSE;
		}

		bool success = false;
		TCHAR filename[MAX_PATH];
		success=DragQueryFile(hdrop, 0, filename, MAX_PATH);
		DragFinish(hdrop);
		GlobalUnlock(medium.hGlobal);

		if(success) DuiMessageBox(NULL,filename,NULL,MB_OK);

		*pdwEffect=DROPEFFECT_LINK;
		return S_OK;
	}
protected:
	int nRef;
};

CUIHander::CUIHander(CMainDlg * pMainDlg) : m_pMainDlg(pMainDlg)
{
}

CUIHander::~CUIHander(void)
{
}

bool Evt_Test2(SWindow * pSender, LPDUINMHDR pNmhdr)
{
	pSender->GetUserData();
// 	CUIHander * p=(CUIHander *)pSender->GetUserData();
// 	pSender->unsubscribeEvent(NM_COMMAND,Subscriber(Evt_Test2));
// 	pSender->subscribeEvent(NM_COMMAND,Subscriber(&CUIHander::Evt_Test,p));
// 	DuiMessageBox(NULL,_T("这个msgbox是在全局函数中使用event的obsever显示的"),_T("事件测试"),MB_OK|MB_ICONWARNING);
	return true;
}

bool CUIHander::Evt_Test(SWindow * pSender, LPDUINMHDR pNmhdr)
{
// 	pSender->subscribeEvent(NM_COMMAND,Subscriber(Evt_Test2));
// 	pSender->unsubscribeEvent(NM_COMMAND,Subscriber(&CUIHander::Evt_Test,this));
// 	pSender->SetUserData((ULONG_PTR)this);
// 	DuiMessageBox(NULL,_T("这个msgbox是在类成员函数中使用event的obsever显示的"),_T("事件测试"),MB_OK|MB_ICONWARNING);
	return true;
}

LRESULT CUIHander::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	HRESULT hr=::RegisterDragDrop(hWnd,m_pMainDlg->GetDropTarget());

	SWindow *pSlider=m_pMainDlg->FindChildByName("IDC_SLIDERTEST");
	m_pMainDlg->RegisterDragDrop(pSlider->GetDuiHwnd(),new CTestDropTarget);

	//初始化虚拟列表
	CDuiListBoxEx *pList2=(CDuiListBoxEx*)m_pMainDlg->FindChildByName("mylist2");
	if(pList2)
	{
		pList2->SetItemCount(100);
	}
	CDuiButton *pBtn=(CDuiButton *)m_pMainDlg->FindChildByCmdID(IDC_REPSEL);
	m_pMainDlg->GetFocusManager()->RegisterAccelerator(SOUI::CAccelerator(VK_RETURN,true),pBtn);//给repsel按钮注册一个热键Ctrl+ENTER。
#ifdef LUA_TEST
	SWindow *pTst=m_pMainDlg->FindChildByName("btn_tstevt");
	DuiSystem::getSingleton().GetScriptModule()->subscribeEvent(pTst,NM_COMMAND,"onEvtTstClick");
#endif

 	OnBtnInitListClick();
	m_pMainDlg->SetTimer(100,10,NULL);
// 	SetMsgHandled(FALSE); 
	//演示在程序初始化的时候通过如用户配置文件设置PANE的大小.
// 	CDuiSplitWnd *pSplit=(CDuiSplitWnd*)m_pMainDlg->FindChildByCmdID(1180);
// 	pSplit->SetPaneInfo(0,100,-1,-1);
// 	CDuiRichEdit *pEdit=(CDuiRichEdit*)m_pMainDlg->FindChildByCmdID(1140);
// 	pEdit->DuiSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);

    return 0; 
}

void CUIHander::OnDestory()
{
	::RevokeDragDrop(m_pMainDlg->m_hWnd);
	CDuiListCtrl *pList=m_pMainDlg->FindChildByName2<CDuiListCtrl*>("lc_test");
	for(int i=0;i<pList->GetItemCount();i++)
	{
		student *pst=(student*) pList->GetItemData(i);
		delete pst;
	}
	SetMsgHandled(FALSE); 
}

void CUIHander::OnAttrReposition()
{
	m_pMainDlg->FindChildByCmdID(测试)->SetAttribute("pos","|-100,|-15,|100,|15");
}


// #define  PATH_GIF _T("E:\\dui.work\\RichEditDemo\\RichEditDemo\\Face\\1.gif")

void CUIHander::OnRepEditSel()
{
	CDuiRichEdit *pEdit=(CDuiRichEdit *)m_pMainDlg->FindChildByCmdID(1140);
	if(pEdit)
	{
		for(int i=0;i<5;i++)
		{
			ISkinObj *pSkinGif=GETSKIN("face0");
// 			RichEdit_InsertImage(pEdit,PATH_GIF);//从文件加载 
			RichEdit_InsertSkin(pEdit,pSkinGif);
			ISkinObj *pSkin=GETSKIN("bmpmask");
			RichEdit_InsertSkin(pEdit,pSkin);
// 			RichEdit_InsertSkin(pEdit,pSkin);
// 			RichEdit_InsertSkin(pEdit,pSkin);
		}
	}

}

LRESULT CUIHander::OnEditNotify( LPDUINMHDR pNHdr )
{
	LPDUIRICHEDITNOTIFY pEditNotify=(LPDUIRICHEDITNOTIFY)pNHdr;
	if(pEditNotify->iNotify==EN_CHANGE)
	{
	}
	return S_OK;
}

//演示阻止combobox继续发送selchanged消息。
LRESULT CUIHander::OnComboListSelChanging( LPDUINMHDR pNHdr )
{
	LPDUINMLBSELCHANGE pLbSelChange=(LPDUINMLBSELCHANGE)pNHdr;
	return pLbSelChange->uHoverID==3?S_FALSE:S_OK;//点击删除按钮时下拉窗口不关闭
}
  
//演示combobox选择改变的事件响应
LRESULT CUIHander::OnComboListSelChanged( LPDUINMHDR pNHdr )
{
	LPDUINMLBSELCHANGE pLbSelChange=(LPDUINMLBSELCHANGE)pNHdr;
	CDuiComboBox *pCombobox=(CDuiComboBox*)m_pMainDlg->FindChildByCmdID(1310);
	return S_OK;
}

//演示响应combobox选项中按钮事件的响应
LRESULT CUIHander::OnComboListItemNotify( LPDUINMHDR pNHdr )
{
	LPDUINMITEMNOTIFY pItemNHdr=(LPDUINMITEMNOTIFY)pNHdr;
	if(pItemNHdr->pOriginHdr->idFrom==3) 
	{//delete button 
		CDuiComboBox *pCombobox=(CDuiComboBox*)m_pMainDlg->FindChildByCmdID(1310);
		CDuiListBoxEx *pListBox=(CDuiListBoxEx*)pItemNHdr->pHostDuiWin;
		int iItem=pListBox->GetItemObjIndex(pItemNHdr->pItem);
		pCombobox->DeleteString(iItem);
	}
	return S_OK;
}


void CUIHander::OnIECtrl()
{
	CDuiImageWnd *pImgWnd=(CDuiImageWnd*)m_pMainDlg->FindChildByCmdID(1139);
	if(pImgWnd)
	{
// 		CDuiImgX *pImg=new CDuiImgX;
// 		pImg->LoadImg(L"e:\\test.png");
// 		CDuiImageList *pSkin=new CDuiImageList;
// 		pSkin->SetImage(pImg);
// 		pImgWnd->SetSkin(pSkin);
// 		pSkin->Release();
	}
}


void CUIHander::OnDuiMenu()
{
	CPoint pt; 
	GetCursorPos(&pt);
	CDuiMenu menu;  
	menu.LoadMenu(_T("IDR_MENU_TEST")); //load menu
	CDuiMenu subMenu=menu.GetSubMenu(5);
	CheckMenuRadioItem(subMenu.m_hMenu,51,53,52,MF_BYCOMMAND);
// 	CDuiMenu subMenu2; 
// 	subMenu2.CreatePopupMenu();
// 	subMenu2.InsertMenu(1,MF_STRING|MF_BYPOSITION,59,_T("代码插入1"),3);
// 	subMenu2.InsertMenu(2,MF_STRING|MF_BYPOSITION|MF_GRAYED,58,_T("代码插入2"),3);
// 	subMenu.InsertMenu(1,MF_POPUP|MF_BYPOSITION,(UINT_PTR)&subMenu2,_T("插入子菜单"),5);
	UINT uRet=menu.TrackPopupMenu(0,pt.x,pt.y,m_pMainDlg->m_hWnd);
// 	ATLTRACE(L"\nmenu ret=%d",uRet); 
}

void CUIHander::OnCommand( UINT uNotifyCode, int nID, HWND wndCtl )
{
// 	ATLTRACE(L"\nOnCommand nID=%d",nID);  
// 	TCHAR szBuf[200];
// 	_stprintf(szBuf,_T("Menu Command ID=%d\\nSecond Line\\nSecond Line\\nSecond Line\\nSecond Line"),nID);
// 	_stprintf(szBuf,_T("Menu Command ID=%d\\nSecond Line \\na long long line :消息窗口内容自动换行测试，hahahaha haha haha"),nID);
// 	DuiMessageBox(NULL,szBuf,_T("tip"),MB_YESNOCANCEL|MB_ICONWARNING);
}

//演示列表中的按钮控件的响应
LRESULT CUIHander::OnListBtnClick( LPDUINMHDR pNHdr )
{
	DUINMITEMNOTIFY *pNHdrEx=(DUINMITEMNOTIFY*)pNHdr;

	return S_OK;
}

LRESULT CUIHander::OnListPredraw( LPDUINMHDR pNHdr )
{
	LPDUINMGETLBDISPINFO lpNHdrEx=(LPDUINMGETLBDISPINFO)pNHdr;
	CDuiStringT str;
	str.Format(_T("item:%d"),lpNHdrEx->nListItemID);
	lpNHdrEx->pItem->FindChildByName("idx")->SetInnerText(str);
	return S_OK;
}


//init listctrl
void CUIHander::OnBtnInitListClick()
{
	CDuiListCtrl *pList=m_pMainDlg->FindChildByName2<CDuiListCtrl *>("lc_test");
	if(pList)
	{
		SWindow *pHeader=pList->GetDuiWindow(GDUI_FIRSTCHILD);
		pHeader->subscribeEvent(NM_HDCLICK,Subscriber(&CUIHander::OnListHeaderClick,this));

		TCHAR szColNames[][20]={_T("name"),_T("sex"),_T("age"),_T("score")};
		for(int i=0;i<ARRAYSIZE(szColNames);i++)
			pList->InsertColumn(i,szColNames[i],50);
		TCHAR szSex[][5]={_T("男"),_T("女"),_T("人妖")};
		for(int i=0;i<100;i++)
		{
			student *pst=new student;
			_stprintf(pst->szName,_T("学生_%d"),i+1);
			_tcscpy(pst->szSex,szSex[rand()%3]);
			pst->age=rand()%30;
			pst->score=rand()%60+40;

			int iItem=pList->InsertItem(i,pst->szName);
			pList->SetItemData(iItem,(DWORD)pst);
			pList->SetSubItemText(iItem,1,pst->szSex);
			TCHAR szBuf[10];
			_stprintf(szBuf,_T("%d"),pst->age);
			pList->SetSubItemText(iItem,2,szBuf);
			_stprintf(szBuf,_T("%d"),pst->score);
			pList->SetSubItemText(iItem,3,szBuf);
		}
	}
}

int funCmpare(void* pCtx,const void *p1,const void *p2)
{
	int iCol=*(int*)pCtx;

	const DXLVITEM *plv1=(const DXLVITEM*)p1;
	const DXLVITEM *plv2=(const DXLVITEM*)p2;

	const CUIHander::student *pst1=(const CUIHander::student *)plv1->dwData;
	const CUIHander::student *pst2=(const CUIHander::student *)plv2->dwData;

	switch(iCol)
	{
	case 0://name
		return _tcscmp(pst1->szName,pst2->szName);
	case 1://sex
		return _tcscmp(pst1->szSex,pst2->szSex);
	case 2://age
		return pst1->age-pst2->age;
	case 3://score
		return pst1->score-pst2->score;
	default:
		return 0;
	}
}

bool CUIHander::OnListHeaderClick( SWindow * pSender, LPDUINMHDR pNmhdr )
{
	CDuiHeaderCtrl *pHeader=(CDuiHeaderCtrl*)pSender;
	LPDUINMHDCLICK pClick=(LPDUINMHDCLICK)pNmhdr;
	CDuiListCtrl *pList=m_pMainDlg->FindChildByName2<CDuiListCtrl*>("lc_test");

	DUIHDITEM hditem;
	hditem.mask=DUIHDI_ORDER;
	pHeader->GetItem(pClick->iItem,&hditem);
	pList->SortItems(funCmpare,&hditem.iOrder);
	return true;
}

void CUIHander::OnBtnAniList()
{
	SWindow *pList=m_pMainDlg->FindChildByName("lc_test");
	if(pList)
	{
		if(pList->IsVisible(TRUE))
		{
			pList->AnimateWindow(100,AW_SLIDE|AW_VER_NEGATIVE|AW_HIDE);
		}else
		{
			pList->AnimateWindow(100,AW_SLIDE|AW_VER_NEGATIVE);//AW_SLIDE|AW_VER_NEGATIVE|AW_HOR_POSITIVE
		}
	}
}

void CUIHander::OnTimer( UINT_PTR uEventID )
{
	if(uEventID==100)
	{
		CDuiProgress *pProg=m_pMainDlg->FindChildByName2<CDuiProgress *>("IDC_PROGTEST");
		if(pProg)
		{
			int nValue=pProg->GetValue();
			int nMin,nMax;
			pProg->GetRange(&nMin,&nMax);
			nValue++;
			if(nValue>nMax) nValue=nMin;
			pProg->SetValue(nValue);
		}
	}
	SetMsgHandled(FALSE);
}

void CUIHander::OnHideTestClick()
{
	SWindow *pImg=m_pMainDlg->FindChildByName("img_hidetst");
	
	pImg->SetVisible(!pImg->IsVisible(),TRUE);
}