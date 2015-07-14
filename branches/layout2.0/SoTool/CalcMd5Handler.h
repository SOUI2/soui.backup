#pragma once

#include "droptarget.h"
#include "MD5.h"

class CCalcMd5Handler : public IFileDropHandler, public ICalcMd5ProgHandler
{
    friend class CMainDlg;
public:
    CCalcMd5Handler(void);
    ~CCalcMd5Handler(void);

    void OnInit(SWindow *pRoot);

protected:
    virtual void OnFileDropdown(HDROP hDrop);
    virtual void OnCalcMd5Prog(DWORD dwTotal,DWORD dwProg);

    void CalcFileMd5(const SStringT &strFileName);
    void OnDirEnterFinish(EventArgs *pEvt);

    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
        EVENT_NAME_HANDLER(L"edit_input",EventKeyEnter::EventID,OnDirEnterFinish)
    EVENT_MAP_BREAK()

    SWindow *m_pPageRoot;
    DWORD    m_dwPrevProg;
};

