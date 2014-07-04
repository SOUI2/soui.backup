//////////////////////////////////////////////////////////////////////////
// SWindow Handle Manager
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "core/SSingletonMap.h"

namespace SOUI
{

class SWindow;

class SOUI_EXP SWindowMgr :public SSingletonMap<SWindowMgr,SWindow*,SWND>
{
public:

    SWindowMgr();

    ~SWindowMgr();

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