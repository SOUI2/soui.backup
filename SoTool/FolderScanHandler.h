#pragma once

class CFolderScanHandler
{
    friend class CMainDlg;
public:
    CFolderScanHandler(void);
    ~CFolderScanHandler(void);
    
    void OnInit(SWindow *pRoot);
protected:
    void OnGo();
    
    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
        EVENT_NAME_COMMAND(L"btn_go",OnGo)
    EVENT_MAP_BREAK()
    
    SWindow *m_pPageRoot;
    STreeList *m_pTreelist;
};

