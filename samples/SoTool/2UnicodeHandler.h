#pragma once
#include "FolderHander.h"

class C2UnicodeHandler : public CFolderHander
{
    friend class CMainDlg;
public:
    C2UnicodeHandler(void);
    ~C2UnicodeHandler(void);

    void OnInit(SWindow *pPageRoot);
protected:
    void OnGo();

    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pFolderRoot)
        EVENT_NAME_COMMAND(L"btn_go",OnGo)
        CHAIN_EVENT_MAP(CFolderHander)
    EVENT_MAP_END()

protected:
    typedef SMap<SStringW,int> FILETYPEMAP;
    void EnumFileInfo(const FILETYPEMAP &cfg, const SStringW & strPath, HSTREEITEM hItem,SList<SStringW> & lstFileInfo , int &nDirs);
};
