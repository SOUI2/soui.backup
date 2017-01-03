#pragma once

#include "droptarget.h"
#include "MD5.h"
#include "SEdit2.h"

class CCalcMd5Handler : public IFileDropHandler, public ICalcMd5ProgHandler
{
    friend class CMainDlg;
public:
    CCalcMd5Handler(void);
    ~CCalcMd5Handler(void);

    void OnInit(SOUI::SWindow *pRoot);

protected:
    virtual void OnFileDropdown(HDROP hDrop);
    virtual void OnCalcMd5Prog(DWORD dwTotal,DWORD dwProg);

	void CalcFileMd5(const SOUI::SStringT &strFileName);
    void OnDirEnterFinish(SOUI::EventArgs *pEvt);

    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
        EVENT_NAME_HANDLER(L"edit_input",SOUI::EventKeyEnter::EventID,OnDirEnterFinish)
    EVENT_MAP_BREAK()

    SOUI::SWindow *m_pPageRoot;
    DWORD    m_dwPrevProg;
};

