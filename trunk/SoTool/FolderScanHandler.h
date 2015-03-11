#pragma once

class CFolderScanHandler
{
    friend class CMainDlg;
public:
    CFolderScanHandler(void);
    ~CFolderScanHandler(void);
    
    void OnInit(SWindow *pRoot);
protected:
    void OnGo(EventArgs *pEvt);
    bool OnTreeDbclick(EventArgs *pEvt);
    BOOL EnumFiles(SStringT strPath,HSTREEITEM hParent);
    BOOL DoSomething();
    
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
        EVENT_NAME_HANDLER(L"btn_go",EventCmd::EventID,OnGo)
        EVENT_NAME_HANDLER(L"edit_dir",EventKeyEnter::EventID,OnGo)
    EVENT_MAP_BREAK()

    SWindow *m_pPageRoot;
    SFolderTreeList *m_pTreelist;
};

