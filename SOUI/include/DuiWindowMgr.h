//////////////////////////////////////////////////////////////////////////
// SWindow Handle Manager
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "DuiSingletonMap.h"

namespace SOUI
{

class SWindow;

class SOUI_EXP DuiWindowMgr :public DuiSingletonMap<DuiWindowMgr,SWindow*,SWND>
{
public:

    DuiWindowMgr();

    ~DuiWindowMgr();

    // Get DuiWindow pointer from handle
    static SWindow* GetWindow(SWND hDuiWnd);

    // Specify a handle to a DuiWindow
    static SWND NewWindow(SWindow *pDuiWnd);

    // Destroy DuiWindow
    static BOOL DestroyWindow(SWND hDuiWnd);
protected:

    CRITICAL_SECTION m_lockWndMap;

    SWND m_hNextWnd;
};

}//namespace SOUI