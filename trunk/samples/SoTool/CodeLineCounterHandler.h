#pragma once

#include "CodeLineCounter.h"
class CCodeLineCounterHandler
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
    
    void OnBtnPickerDir();
    void OnBtnGo();

    void OnKillFocus_Dir(EventArgs *pEvt);

    void InitDirTree(HSTREEITEM hTreeItem,const SStringT & strPath);

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_picker_dir", OnBtnPickerDir)
        EVENT_NAME_COMMAND(L"btn_go", OnBtnGo)
        EVENT_NAME_HANDLER(L"edit_dir",EventKillFocus::EventID,OnKillFocus_Dir)
    EVENT_MAP_END()

    SWindow *   m_pPageRoot;
    SStringT    m_strDir;
    STreeCtrl * m_pDirTree;

protected:
    typedef SMap<SStringW,CCodeConfig> CODECFGMAP;

    DWORD EnumFileInfo(const CODECFGMAP &cfg, const SStringW & strPath,HSTREEITEM hItem,SList<FILEINFO> & lstFileInfo, int &nDirs);
    CODECFGMAP m_mapCodeCfg;
};
