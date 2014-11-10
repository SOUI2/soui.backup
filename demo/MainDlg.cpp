// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "helper/SMenu.h"
#include "../controls.extend/FileHelper.h"

#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")

#include <shellapi.h>
class CTestDropTarget:public IDropTarget
{
public:
    CTestDropTarget()
    {
        nRef=0;
    }
    
    virtual ~CTestDropTarget(){}
    
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


protected:
    int nRef;
};

class CTestDropTarget1 : public CTestDropTarget
{
protected:
    SWindow *m_pEdit;
public:
    CTestDropTarget1(SWindow *pEdit):m_pEdit(pEdit)
    {
        if(m_pEdit) m_pEdit->AddRef();
    }
    ~CTestDropTarget1()
    {
        if(m_pEdit) m_pEdit->Release();
    }
public:
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
        success=!!DragQueryFile(hdrop, 0, filename, MAX_PATH);
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);

        if(success && m_pEdit)
        {
            m_pEdit->SetWindowText(filename);
        }

        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }
};



int CMainDlg::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
//     MARGINS mar = {5,5,30,5};
//     DwmExtendFrameIntoClientArea ( m_hWnd, &mar );//打开这里可以启用Aero效果
	SetMsgHandled(FALSE);
	return 0;
}

void CMainDlg::OnShowWindow( BOOL bShow, UINT nStatus )
{
	if(bShow)
	{
 		AnimateHostWindow(200,AW_CENTER);
	}
}

struct student{
    TCHAR szName[100];
    TCHAR szSex[10];
    int age;
    int score;
};

//init listctrl
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

void CMainDlg::OnDestory()
{
    SListCtrl *pList=FindChildByName2<SListCtrl>(L"lc_test");
    if(pList)
    {
        for(int i=0;i<pList->GetItemCount();i++)
        {
            student *pst=(student*) pList->GetItemData(i);
            delete pst;
        }
    }
    SetMsgHandled(FALSE); 
}


LRESULT CMainDlg::OnInitDialog( HWND hWnd, LPARAM lParam )
{
    m_bLayoutInited=TRUE;
    InitListCtrl();
    
#ifdef DLL_CORE
    SWindow *pTst=FindChildByName(L"btn_tstevt");
    if(pTst) SApplication::getSingleton().GetScriptModule()->subscribeEvent(pTst,EVT_CMD,"onEvtTstClick");
#endif
    
    //演示在SOUI中的拖放
    SWindow *pEdit1 = FindChildByName(L"edit_drop_top");
    SWindow *pEdit2 = FindChildByName(L"edit_drop_bottom");
    if(pEdit1 && pEdit2)
    {
        HRESULT hr=::RegisterDragDrop(m_hWnd,GetDropTarget());
        RegisterDragDrop(pEdit1->GetSwnd(),new CTestDropTarget1(pEdit1));
        RegisterDragDrop(pEdit2->GetSwnd(),new CTestDropTarget1(pEdit2));
    }
    return 0;
}

void CMainDlg::OnBtnWebkitGo()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        SEdit *pEdit=FindChildByName2<SEdit>(L"edit_url");
        SStringT strUrl=pEdit->GetWindowText();
        pWebkit->SetAttribute(L"url",S_CT2W(strUrl),FALSE);
    }
}

void CMainDlg::OnBtnWebkitBackward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goBack();
    }
}

void CMainDlg::OnBtnWebkitForeward()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->goForward();
    }
}

void CMainDlg::OnBtnWebkitRefresh()
{
    SWkeWebkit *pWebkit= FindChildByName2<SWkeWebkit>(L"wke_test");
    if(pWebkit)
    {
        pWebkit->GetWebView()->reload();
    }
}

void CMainDlg::OnBtnSelectGIF()
{
    SGifPlayer *pGifPlayer = FindChildByName2<SGifPlayer>(L"giftest");
    if(pGifPlayer)
    {
        CFileDialogEx openDlg(TRUE,_T("gif"),0,6,_T("gif files(*.gif)\0*.gif\0All files (*.*)\0*.*\0\0"));
        if(openDlg.DoModal()==IDOK)
            pGifPlayer->PlayGifFile(openDlg.m_szFileName);
    }
}

void CMainDlg::OnBtnMenu()
{
    SMenu menu;
    menu.LoadMenu(_T("menu_test"),_T("LAYOUT"));
    POINT pt;
    GetCursorPos(&pt);
    menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}

//演示如何响应菜单事件
void CMainDlg::OnCommand( UINT uNotifyCode, int nID, HWND wndCtl )
{
    if(uNotifyCode==0)
    {
        if(nID==6)
        {//nID==6对应menu_test定义的菜单的exit项。
            PostMessage(WM_CLOSE);
        }else if(nID==54)
        {//about SOUI
            STabCtrl *pTabCtrl = FindChildByName2<STabCtrl>(L"tab_main");
            if(pTabCtrl) pTabCtrl->SetCurSel(_T("about"));
        }
    }
}

void CMainDlg::OnBtnHideTest()
{
    SWindow * pBtn = FindChildByName(L"btn_hidetst");
    if(pBtn) pBtn->SetVisible(FALSE,TRUE);
}

#include "skinole\ImageOle.h"

void CMainDlg::OnBtnInsertGif2RE()
{
    SRichEdit *pEdit = FindChildByName2<SRichEdit>(L"re_gifhost");
    if(pEdit)
    {
        ISkinObj * pSkin = GETSKIN(L"gif_penguin");
        if(pSkin)
        {
            RichEdit_InsertSkin(pEdit,pSkin);
        }
    }
}

void CMainDlg::OnBtnMsgBox()
{
    SMessageBox(NULL,_T("this is a message box"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
    SMessageBox(NULL,_T("this message box includes two buttons"),_T("haha"),MB_YESNO|MB_ICONQUESTION);
    SMessageBox(NULL,_T("this message box includes three buttons"),NULL,MB_ABORTRETRYIGNORE);
}

#include <render-skia/Render-Skia2-i.h>

void CMainDlg::OnSkiaTest()
{
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,100,100);
    CAutoRefPtr<IRenderTarget_Skia2> pRTSkia2;
    HRESULT hr=pRT->QueryInterface(__uuidof(IRenderTarget_Skia2),(IObjRef**)&pRTSkia2);
    if(SUCCEEDED(hr))
    {
        CRect rcUp(0,0,100,50);
        CRect rcDown(0,50,100,100);
        pRT->FillSolidRect(&rcUp,RGBA(255,0,0,255));
        pRT->FillSolidRect(&rcDown,RGBA(0,255,0,255));
        
        SWindow *pCanvas = FindChildByName(L"skia_canvas");
        if(pCanvas)
        {
            IRenderTarget* pRTDst= pCanvas->GetRenderTarget();
            CRect rcCanvas;
            pCanvas->GetWindowRect(&rcCanvas);
            
            CRect rcDst(rcCanvas.TopLeft(),CSize(100,100));
            pRTDst->BitBlt(&rcDst,pRT,0,0);
            
            pRTSkia2->Init(pRT);
            pRTSkia2->rotate(45.0f);
            pRT->FillSolidRect(&rcUp,RGBA(0,0,255,255));
            pRT->FillSolidRect(&rcDown,RGBA(0,255,255,255));
            
            
            rcDst.OffsetRect(100,0);
            pRTDst->BitBlt(&rcDst,pRT,0,0);
            
            pCanvas->ReleaseRenderTarget(pRTDst);
        }
    }else
    {
        SMessageBox(NULL,_T("当前使用的渲染引擎不是skia"),_T("错误"),MB_OK|MB_ICONSTOP);
    }
}
