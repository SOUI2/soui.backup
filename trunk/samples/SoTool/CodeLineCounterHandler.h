#pragma once

#include "CodeLineCounter.h"
#include "FolderHander.h"

class CCodeLineCounterHandler : public CFolderHander
{
    friend class CMainDlg;
    struct FILEINFO
    {
        SStringT strFileName;
        DWORD    dwSize;
        CCodeConfig cfg;
    };
public:
    CCodeLineCounterHandler(void);
    ~CCodeLineCounterHandler(void);

    void OnInit(SWindow *pRoot);
protected:
    
    void OnBtnGo();

    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
        EVENT_NAME_COMMAND(L"btn_go", OnBtnGo)
        CHAIN_EVENT_MAP(CFolderHander)
    EVENT_MAP_END()

    SWindow * m_pPageRoot;
protected:
    typedef SMap<SStringW,CCodeConfig> CODECFGMAP;

    DWORD EnumFileInfo(const CODECFGMAP &cfg, const SStringW & strPath,HSTREEITEM hItem,SList<FILEINFO> & lstFileInfo, int &nDirs);
    CODECFGMAP m_mapCodeCfg;
};
