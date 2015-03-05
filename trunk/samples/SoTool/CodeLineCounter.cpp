#include "StdAfx.h"
#include "CodeLineCounter.h"

CCodeLineCounter::CCodeLineCounter(void):m_pPageRoot(NULL)
{
}

CCodeLineCounter::~CCodeLineCounter(void)
{
}

void CCodeLineCounter::OnInit( SWindow *pRoot )
{
    m_pPageRoot = pRoot->FindChildByName(L"page_codeline");
    SASSERT(m_pPageRoot);
    m_pDirTree = m_pPageRoot->FindChildByName2<STreeCtrl>(L"tree_dir");
}

void CCodeLineCounter::OnBtnPickerDir()
{

}

void CCodeLineCounter::OnKillFocus_Dir( EventArgs *pEvt )
{
    SEdit *pEdit = sobj_cast<SEdit>(pEvt->sender);
    SStringT strDir = pEdit->GetWindowText();
    DWORD attr = GetFileAttributes(strDir);
    if(attr==INVALID_FILE_ATTRIBUTES) return ;
    if((attr & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
    if(strDir == m_strDir) return;

    m_strDir = strDir;
    m_pDirTree->RemoveAllItems();
    HSTREEITEM hRoot = m_pDirTree->InsertItem(_T("root"),0,1);
    m_nDirs = 1;
    InitDirTree(hRoot,m_strDir);
}

void CCodeLineCounter::InitDirTree(HSTREEITEM hTreeItem,const SStringT & strPath)
{
    WIN32_FIND_DATA fd;
    SStringT strFind = strPath + _T("\\*.*");
    HANDLE hFind=FindFirstFile(strFind,&fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        while(FindNextFile(hFind,&fd))
        {
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                && !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                SStringT strName= fd.cFileName;
                if(strName != _T(".") && strName != _T(".."))
                {
                    HSTREEITEM hItem = m_pDirTree->InsertItem(strName,0,1,hTreeItem);
                    m_nDirs ++;
                    InitDirTree(hItem,strPath + _T("\\") + strName);
                }
            }
        }
        FindClose(hFind);
    }
}

void CCodeLineCounter::OnBtnGo()
{
    m_pPageRoot->FindChildByName2<STabCtrl>(L"tab_codeline")->SetCurSel(_T("page_result"),FALSE);
    SWindow *pPageResult = m_pPageRoot->FindChildByName(L"page_result");    
    SProgress *pProgBar = pPageResult->FindChildByName2<SProgress>(L"prog_run");
    pProgBar->SetRange(0,m_nDirs);
    pProgBar->SetValue(0);
    for(int i=0;i<m_nDirs;i++)
    {
        pProgBar->SetValue(i);
        pProgBar->UpdateWindow();
        Sleep(10);
    }
}
