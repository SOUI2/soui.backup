#include "stdafx.h"
#include "FolderScanHandler.h"

//////////////////////////////////////////////////////////////////////////
CFolderScanHandler::CFolderScanHandler(void):m_pPageRoot(NULL)
{
}

CFolderScanHandler::~CFolderScanHandler(void)
{
}

void CFolderScanHandler::OnFileDropdown( HDROP hDrop )
{
    bool success = false;
    TCHAR filename[MAX_PATH];
    success=!!DragQueryFile(hDrop, 0, filename, MAX_PATH);
    if(success) 
    {
        if(GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY)
        {
            InitDir(filename);
        }
    }
}

void CFolderScanHandler::OnInit(SWindow *pRoot)
{
    m_pPageRoot = pRoot->FindChildByName(L"page_folderscan");
    SASSERT(m_pPageRoot);
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    SASSERT(pEditDir);
    IDropTarget *pDT = new CDropTarget(this);
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
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    InitDir(pEditDir->GetWindowText());
}

void CFolderScanHandler::InitDir( const SStringT & strDir )
{
    SWindow *pBtn = m_pPageRoot->FindChildByName(L"btn_go");
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");

    DWORD dwAttr = GetFileAttributes(strDir);
    if(dwAttr ==INVALID_FILE_ATTRIBUTES)
        return;
    if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
        return;

    pEditDir->SetWindowText(strDir);

    pBtn->EnableWindow(FALSE,TRUE);
    pEditDir->EnableWindow(FALSE,TRUE);

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