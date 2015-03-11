#include "stdafx.h"
#include "FolderScanHandler.h"
#include "droptarget.h"

//////////////////////////////////////////////////////////////////////////
class CDropTarget_Dir2 : public CDropTarget
{
protected:
    SWindow *m_pEdit;
public:
    CDropTarget_Dir2(SWindow *pEdit):m_pEdit(pEdit)
    {
    }
    ~CDropTarget_Dir2()
    {
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
        if(success) 
        {
            if(GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY)
            {
                m_pEdit->SetWindowText(filename);
            }
        }
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);


        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }
};

//////////////////////////////////////////////////////////////////////////
CFolderScanHandler::CFolderScanHandler(void):m_pPageRoot(NULL)
{
}

CFolderScanHandler::~CFolderScanHandler(void)
{
}

void CFolderScanHandler::OnInit(SWindow *pRoot)
{
    m_pPageRoot = pRoot->FindChildByName(L"page_folderscan");
    SASSERT(m_pPageRoot);
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    SASSERT(pEditDir);
    IDropTarget *pDT = new CDropTarget_Dir2(pEditDir);
    m_pPageRoot->GetContainer()->RegisterDragDrop(pEditDir->GetSwnd(),pDT);
    pDT->Release();

    m_pTreelist = m_pPageRoot->FindChildByName2<SFolderTreeList>(L"tree_dir");
    SASSERT(m_pTreelist);
    m_pTreelist->GetFolderTreeCtrl()->GetEventSet()->subscribeEvent(EventTCDbClick::EventID,Subscriber(&CFolderScanHandler::OnTreeDbclick,this));
}

bool CFolderScanHandler::OnTreeDbclick(EventArgs *pEvt)
{
    EventTCDbClick *pEvt2 = sobj_cast<EventTCDbClick>(pEvt);
    pEvt2->bCancel = TRUE;
    HSTREEITEM hItem = pEvt2->hItem;
    HSTREEITEM hRoot = m_pTreelist->GetFolderTreeCtrl()->GetRootItem();
    
    SStringT strPath;
    while(hItem != hRoot)
    {
        SStringT strItem;
        m_pTreelist->GetFolderTreeCtrl()->GetItemText(hItem,strItem);
        strPath = strItem + _T("\\") + strPath;
        hItem = m_pTreelist->GetFolderTreeCtrl()->GetParentItem(hItem);
    }
    
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    
    SStringT strRoot = pEditDir->GetWindowText();

    strPath = strRoot + _T("\\") + strPath;
    
    SStringT strCmd = SStringT().Format(_T("/select, %s"),strPath);
    ShellExecute( NULL, _T("open"), _T("explorer.exe"), strCmd, NULL, SW_SHOWNORMAL ); 
    
    return true;
}

BOOL CFolderScanHandler::DoSomething()
{
    MSG msg;
    while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
        if(msg.message == PM_REMOVE)
        {
            PostThreadMessage(0,WM_QUIT,0,0);
            return FALSE;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return TRUE;
}

BOOL CFolderScanHandler::EnumFiles(SStringT strPath,HSTREEITEM hParent)
{
    BOOL bRet = TRUE;
    WIN32_FIND_DATA fd;
    SStringT strFind = strPath + _T("\\*.*");
    HANDLE hFind=FindFirstFile(strFind,&fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        while(bRet && FindNextFile(hFind,&fd))
        {
            SStringT strName= fd.cFileName;
            BOOL bFolder = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if(bFolder && (strName == _T(".") || strName == _T("..")))
                continue;
            __int64 nSize = ((__int64)fd.nFileSizeHigh)<<32|fd.nFileSizeLow;
            HSTREEITEM hItem = m_pTreelist->GetFolderTreeCtrl()->InsertItem(strName,bFolder,nSize,hParent);
            if(!DoSomething())
            {
                bRet = FALSE;
            }else if(bFolder)
            {
                bRet = EnumFiles(strPath + _T("\\") + strName,hItem);
            }
        }
        FindClose(hFind);
    }
    return bRet;
}


void CFolderScanHandler::OnGo(EventArgs *pEvt)
{
    SWindow *pBtn = m_pPageRoot->FindChildByName(L"btn_go");
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    pBtn->EnableWindow(FALSE,TRUE);
    pEditDir->EnableWindow(FALSE,TRUE);
    
    SStringT strDir = pEditDir->GetWindowText();
    DWORD dwAttr = GetFileAttributes(strDir);
    if(dwAttr ==INVALID_FILE_ATTRIBUTES)
        return;
    if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
        return;
    
    SWindow *pScanAni = m_pPageRoot->FindChildByName(L"ani_scan");
    pScanAni->SetVisible(TRUE,TRUE);
    SFolderTreeCtrl *pMcTreeCtrl = m_pTreelist->GetFolderTreeCtrl();
    pMcTreeCtrl->RemoveAllItems();
    HSTREEITEM hRoot = pMcTreeCtrl->InsertItem(_T("root"),TRUE,0,STVI_ROOT);
    if(EnumFiles(strDir,hRoot))
    {
        m_pTreelist->GetFolderTreeCtrl()->GetFileInfo(hRoot)->percent=100;
        m_pTreelist->GetFolderTreeCtrl()->UpdateTreeItemPercent(hRoot);
    }
    m_pTreelist->GetFolderTreeCtrl()->Invalidate();
    pBtn->EnableWindow(TRUE,TRUE);
    pEditDir->EnableWindow(TRUE,TRUE);
    pScanAni->SetVisible(FALSE,TRUE);

}
