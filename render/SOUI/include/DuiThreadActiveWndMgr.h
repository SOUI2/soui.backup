#pragma once

#include "DuiSingletonMap.h"

namespace SOUI
{

class SOUI_EXP DuiThreadActiveWndMgr: public DuiSingletonMap<DuiThreadActiveWndMgr,HWND,DWORD>
{
public:
    DuiThreadActiveWndMgr();

    virtual ~DuiThreadActiveWndMgr();

    static HWND SetActive(HWND hWnd);

    static HWND GetActive();

    static void EnterPaintLock();

    static void LeavePaintLock();

protected:
    HWND _SetActive(HWND hWnd);
    HWND _GetActive();

protected:

    CRITICAL_SECTION        m_lockMapActive;
    CRITICAL_SECTION        m_lockRepaint;
};

}//namespace SOUI