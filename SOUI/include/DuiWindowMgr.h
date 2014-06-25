//////////////////////////////////////////////////////////////////////////
// SWindow Handle Manager
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "DuiSingletonMap.h"

namespace SOUI
{

class SWindow;

class SOUI_EXP DuiWindowMgr :public DuiSingletonMap<DuiWindowMgr,SWindow*,HSWND>
{
public:

    DuiWindowMgr();

    ~DuiWindowMgr();

    // Get DuiWindow pointer from handle
    static SWindow* GetWindow(HSWND hDuiWnd);

    // Specify a handle to a DuiWindow
    static HSWND NewWindow(SWindow *pDuiWnd);

    // Destroy DuiWindow
    static BOOL DestroyWindow(HSWND hDuiWnd);
protected:

    CRITICAL_SECTION m_lockWndMap;

    HSWND m_hNextWnd;
};

}//namespace SOUI