/****************************************************************************
*  功    能：使用SOUI自主设计音乐播放器                                     *
*  作    者：小可                                                           *
*  添加时间：2014.01.09 17：00                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/

// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"

#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

int funCmpare(void* pCtx,const void *p1,const void *p2)
{
	int iCol=*(int*)pCtx;

	const DXLVITEM *plv1=(const DXLVITEM*)p1;
	const DXLVITEM *plv2=(const DXLVITEM*)p2;

	const student *pst1=(const student *)plv1->dwData;
	const student *pst2=(const student *)plv2->dwData;

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

CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
    m_bCut=false;
	m_3DType=false;
	m_bLayoutInited=FALSE;

	m_pBassMusic=NULL;
	m_pMusicState=NULL;
} 

CMainDlg::~CMainDlg()
{
	shellNotifyIcon.Hide();
	if (hStream)
	{
		BASS_ChannelStop(hStream);
		hStream=NULL;

	}
	
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// 		MARGINS mar = {5,5,30,5};
	// 		DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	shellNotifyIcon.Create(m_hWnd,GETRESPROVIDER->LoadIcon(_T("ICON_LOGO"),16));
	shellNotifyIcon.Show();
	//InitListCtrl();

	//拖拽功能
	SWindow *pListBox = FindChildByName(L"music_tree_box");
	if(pListBox)
	{
		HRESULT hr=::RegisterDragDrop(m_hWnd,GetDropTarget());
		RegisterDragDrop(pListBox->GetSwnd(),new CTestDropTarget1(pListBox));
	}

	//初始化声音组件
	m_pBassMusic = CBassMusicEngine::GetInstance();
	if ( m_pBassMusic == NULL )
	{
		if ( SMessageBox(NULL,TEXT("声音引擎初始化失败"),_T("警告"),MB_OK|MB_ICONEXCLAMATION) == IDOK )
		{
			PostQuitMessage(0);
			return TRUE;
		}
	}	
	m_pBassMusic->Init(m_hWnd,m_pMusicState);

    //测试：播放音乐
	OnButPlay();

	STreeBox *pTreeBox=FindChildByName2<STreeBox>(L"music_tree_box");

	if (pTreeBox)
	{
		//pTreeBox->SetCheck(_T("..."));
		HSTREEITEM item=pTreeBox->GetNextSiblingItem(pTreeBox->GetRootItem());
		pTreeBox->SetCheck(TRUE);
	}

	return 0;
}


//TODO:消息映射
void CMainDlg::OnClose()
{
	PostMessage(WM_QUIT);
	
}

void CMainDlg::OnMaximize()
{
	//SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
	if (!m_bCut)
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
		if(pTab)
		{
			pTab->SetCurSel(_T("在线音乐"));
			m_bCut=true;
		}
		
	}else
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
		if(pTab)
		{
			pTab->SetCurSel(_T("我的音乐"));
			m_bCut=false;
		}
	}

}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

#include <helper/smenu.h>
LRESULT CMainDlg::OnIconNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL/* bHandled*/)
{
	switch (lParam)
	{
	case  WM_RBUTTONDOWN:
	{
            //显示右键菜单
            SMenu menu;
            menu.LoadMenu(_T("menu_tray"),_T("smenu"));
            POINT pt;
            GetCursorPos(&pt);
            menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
	}break;
	case WM_LBUTTONDOWN:
	{
		//显示/隐藏主界面
		if (IsIconic())
		   ShowWindow(SW_SHOWNORMAL);
		else
		   ShowWindow(SW_MINIMIZE);
	}break;
	default:
		break;
	}
	return S_OK;
}

//演示如何响应菜单事件
void CMainDlg::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if (uNotifyCode == 0)
	{
		switch (nID)
		{
		case 6:
			PostMessage(WM_CLOSE);
			break;
		default:
			break;
		}
	}
}

void CMainDlg::InitListCtrl()
{
	//找到列表控件
	SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
	if(pList)
	{
		//列表控件的唯一子控件即为表头控件
		SWindow *pHeader=pList->GetWindow(GSW_FIRSTCHILD);
		//向表头控件订阅表明点击事件，并把它和OnListHeaderClick函数相连。
		pHeader->GetEventSet()->subscribeEvent(EVT_HEADER_CLICK,Subscriber(&CMainDlg::OnListHeaderClick,this));

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

//表头点击事件处理函数
bool CMainDlg::OnListHeaderClick(EventArgs *pEvtBase)
{
	//事件对象强制转换
	EventHeaderClick *pEvt =(EventHeaderClick*)pEvtBase;
	SHeaderCtrl *pHeader=(SHeaderCtrl*)pEvt->sender;
	//从表头控件获得列表控件对象
	SListCtrl *pList= (SListCtrl*)pHeader->GetParent();
	//列表数据排序
	SHDITEM hditem;
	hditem.mask=SHDI_ORDER;
	pHeader->GetItem(pEvt->iItem,&hditem);
	pList->SortItems(funCmpare,&hditem.iOrder);
	return true;
}

BOOL CMainDlg::OnTurn3D( EventArgs *pEvt )
{
	EventTurn3d *pEvt3dTurn = (EventTurn3d*)pEvt;
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
	if(pEvt3dTurn->bTurn2Front_)
	{
		pTab->SetCurSel(_T("musiclist"));
		
	}else
	{
		pTab->SetCurSel(_T("lrc"));
	}
	return TRUE;
}

void CMainDlg::OnTurn3DBut()
{
	if (!m_3DType)
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
		if(pTab)
		{
			STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
			if(pTurn3d)
			{
				pTurn3d->Turn(pTab->GetPage(_T("musiclist")),pTab->GetPage(_T("lrc")),FALSE);
				m_3DType=true;
			}
		}
	}else
	{
		STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
		if(pTab)
		{
			STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
			if(pTurn3d)
			{
				pTurn3d->Turn(pTab->GetPage(_T("lrc")),pTab->GetPage(_T("musiclist")),TRUE);
				m_3DType=false;
			}
		}

	}
	
}

void CMainDlg::OnBtnMyMusic()
{
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
	if(pTab)
	{
		pTab->SetCurSel(_T("我的音乐"));
	}

}

void CMainDlg::OnBtnOnlineMusic()
{
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
	if(pTab)
	{
		pTab->SetCurSel(_T("在线音乐"));
	}

}

void CMainDlg::OnBtnMyDevice()
{
	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_main");
	if(pTab)
	{
		pTab->SetCurSel(_T("我的设备"));
	}

}

void CMainDlg::OnButPrev()
{
}

void CMainDlg::OnButPlay()
{
	if (hStream)
	{
		BASS_ChannelStop(hStream);
		hStream=NULL;
	}

	CString lpszFileName="testMP3/BEYOND - 不再犹豫.mp3";
	hStream=BASS_StreamCreateFile(FALSE, lpszFileName.GetBuffer(),0,0,BASS_SAMPLE_MONO);
	if (hStream)
	{
		//开始播放
		BOOL bResult = BASS_ChannelPlay(hStream, FALSE);
		if (bResult)
		{
			int i=0;
		}
	}

}

void CMainDlg::OnButPlayNext()
{
}

void CMainDlg::OnButLyric()
{

	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
	if(pTab)
	{
		STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
		if(pTurn3d)
		{
			pTurn3d->Turn(pTab->GetPage(_T("musiclist")),pTab->GetPage(_T("lrc")),FALSE);
		}
	}
}

void CMainDlg::OnButMusicList()
{

	STabCtrl *pTab= FindChildByName2<STabCtrl>(L"tab_3d");
	if(pTab)
	{
		STurn3dView * pTurn3d = FindChildByName2<STurn3dView>(L"turn3d");
		if(pTurn3d)
		{
			pTurn3d->Turn(pTab->GetPage(_T("lrc")),pTab->GetPage(_T("musiclist")),TRUE);
		}
	}
}

void CMainDlg::OnFlywndState( EventArgs *pEvt )
{
    FlyStateEvent *pEvtFlywnd = (FlyStateEvent*)pEvt;
    if(pEvtFlywnd->nPercent == SAnimator::PER_END)
    {
        //测试：隐藏音乐频谱 (可以用定时器判断分层窗口的收缩状态来控制)

        SWindow *pSpectrum = FindChildByName2<SWindow>("spectrum");
        if(pSpectrum) pSpectrum->SetVisible(pEvtFlywnd->bEndPos,TRUE);        
//         if (!m_bCut)
//         {
// 
//             m_bCut=true;
// 
//         }else
//         {
//             SWindow *pSpectrum = FindChildByName2<SWindow>("spectrum");
//             if(pSpectrum) pSpectrum->SetVisible(FALSE,TRUE);
//             m_bCut=false;
//         }        
    }
}