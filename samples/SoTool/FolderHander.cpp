#include "StdAfx.h"
#include "FolderHander.h"
#include <helper/SplitString.h>
#include "droptarget.h"

//////////////////////////////////////////////////////////////////////////
class CDropTarget_Dir : public CDropTarget
{
protected:
    CFolderHander *m_pLineCounterHandler;
public:
    CDropTarget_Dir(CFolderHander *pLineCounterHandler):m_pLineCounterHandler(pLineCounterHandler)
    {
    }
    ~CDropTarget_Dir()
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
                m_pLineCounterHandler->InitDir(filename);
            }
        }
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);


        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }
};

//////////////////////////////////////////////////////////////////////////
// CFolderHander
CFolderHander::CFolderHander(void):m_pFolderRoot(NULL)
{
}

CFolderHander::~CFolderHander(void)
{
}

void CFolderHander::OnInit( SWindow *pFolderRoot )
{
    m_pFolderRoot = pFolderRoot;
    SASSERT(m_pFolderRoot);

    SWindow *pEditDir = m_pFolderRoot->FindChildByName(L"edit_dir");
    SASSERT(pEditDir);
    IDropTarget *pDT = new CDropTarget_Dir(this);
    m_pFolderRoot->GetContainer()->RegisterDragDrop(pEditDir->GetSwnd(),pDT);
    pDT->Release();

    m_pDirTree = m_pFolderRoot->FindChildByName2<STreeCtrl>(L"tree_dir");
}


void CFolderHander::OnKillFocus_Dir( EventArgs *pEvt )
{
    SEdit *pEdit = sobj_cast<SEdit>(pEvt->sender);
    SStringT strDir = pEdit->GetWindowText();
    InitDir(strDir,TRUE);
}


void CFolderHander::InitDir( const SStringT & strDir,BOOL bInput/*=FALSE */)
{
    DWORD attr = GetFileAttributes(strDir);
    if(attr==INVALID_FILE_ATTRIBUTES) return ;
    if((attr & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
    if(strDir == m_strDir) return;

    if(!bInput)
    {
        m_pFolderRoot->FindChildByName(L"edit_dir")->SetWindowText(strDir);
    }
    m_strDir = strDir;
    m_pDirTree->RemoveAllItems();
    HSTREEITEM hRoot = m_pDirTree->InsertItem(_T("root"),0,1);
    InitDirTree(hRoot,m_strDir);
    m_pDirTree->SetCheckState(hRoot,TRUE);
}

void CFolderHander::InitDirTree(HSTREEITEM hTreeItem,const SStringT & strPath)
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
                    InitDirTree(hItem,strPath + _T("\\") + strName);
                }
            }
        }
        FindClose(hFind);
    }
}

void CFolderHander::OnBtnFileTypes(EventArgs *pEvt)
{
    SWindow *pBtn = sobj_cast<SWindow>(pEvt->sender);
    CRect rcBtn = pBtn->GetWindowRect();
    HWND hWnd = pBtn->GetContainer()->GetHostHwnd();
    ::ClientToScreen(hWnd,(LPPOINT)&rcBtn);
    ::ClientToScreen(hWnd,(LPPOINT)&rcBtn+1);

    SMenu menu;
    menu.LoadMenu(_T("menu_filetype"),_T("xml"));
    for(int i=0;i<m_lstLangExts.GetCount();i++)
    {
        SStringW strDesc = m_lstLangExts[i].strLang+L":"+m_lstLangExts[i].strExts;
        menu.InsertMenu(i,MF_BYPOSITION,i+10,S_CW2T(strDesc),-1);
    }

    int id = menu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_RETURNCMD,rcBtn.right,rcBtn.bottom,hWnd);
    if(id!=0)
    {
        id-=10;
        SEdit *pEdit = m_pFolderRoot->FindChildByName2<SEdit>(L"edit_filetypes");
        SASSERT(pEdit);
        if(pEdit->GetWindowTextLength()!=0)
        {
            pEdit->SetSel(-1);
            pEdit->ReplaceSel(L";");            
        }
        pEdit->SetSel(-1);
        pEdit->ReplaceSel(m_lstLangExts[id].strExts);
    }
}

void CFolderHander::InitLang( pugi::xml_node xmlNode )
{
    pugi::xml_node xmlLang = xmlNode.child(L"language");
    while(xmlLang)
    {
        LANGEXTS langExts;
        langExts.strLang = xmlLang.attribute(L"name").value();
        langExts.strExts = xmlLang.attribute(L"exts").value();
        m_lstLangExts.Add(langExts);

        xmlLang = xmlLang.next_sibling(L"language");
    }
}