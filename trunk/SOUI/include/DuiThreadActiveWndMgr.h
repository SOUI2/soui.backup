#pragma once

#include "DuiSingletonMap.h"

namespace SOUI
{

class SOUI_EXP SThreadActiveWndMgr: public DuiSingletonMap<SThreadActiveWndMgr,HWND,DWORD>
{
public:
    SThreadActiveWndMgr();

    virtual ~SThreadActiveWndMgr();

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