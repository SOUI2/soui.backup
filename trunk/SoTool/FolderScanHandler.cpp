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

    m_pTreelist = m_pPageRoot->FindChildByName2<STreeList>(L"tree_dir");
    SASSERT(m_pTreelist);
}

void CFolderScanHandler::OnGo()
{
    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    SStringT strDir = pEditDir->GetWindowText();
    DWORD dwAttr = GetFileAttributes(strDir);
    if(dwAttr ==INVALID_FILE_ATTRIBUTES)
        return;
    if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
        return;
    
    SMCTreeCtrl *pMcTreeCtrl = m_pTreelist->GetMCTreeCtrl();
    pMcTreeCtrl->RemoveAllItems();
    HSTREEITEM hRoot = pMcTreeCtrl->InsertItem(_T("root"),0,0);
    pMcTreeCtrl->SetItemText(hRoot,0,_T("10G"));
    pMcTreeCtrl->SetItemText(hRoot,1,_T("100%"));
}
