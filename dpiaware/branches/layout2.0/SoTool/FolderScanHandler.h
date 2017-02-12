#pragma once
#include "droptarget.h"

class CFolderScanHandler : public IFileDropHandler
{
    friend class CMainDlg;
    friend class CDropTarget_Dir2;
public:
    CFolderScanHandler(void);
    ~CFolderScanHandler(void);
    
    void OnInit(SWindow *pRoot);
protected:
    virtual void OnFileDropdown(HDROP hDrop);

    void OnGo(EventArgs *pEvt);
    bool OnTreeDbclick(EventArgs *pEvt);
    BOOL EnumFiles(SStringT strPath,HSTREEITEM hParent);
    BOOL DoSomething();
    void InitDir(const SStringT & strPath);

    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
        EVENT_NAME_HANDLER(L"btn_go",EventCmd::EventID,OnGo)
        EVENT_NAME_HANDLER(L"edit_dir",EventKeyEnter::EventID,OnGo)
    EVENT_MAP_BREAK()

    SWindow *m_pPageRoot;
    SFolderTreeList *m_pTreelist;
};

