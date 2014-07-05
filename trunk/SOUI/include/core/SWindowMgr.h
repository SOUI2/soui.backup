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

    // Get SWindow pointer from handle
    static SWindow* GetWindow(SWND swnd);

    // Specify a handle to a SWindow
    static SWND NewWindow(SWindow *pWnd);

    // Destroy SWindow
    static BOOL DestroyWindow(SWND swnd);
protected:

    CRITICAL_SECTION m_lockWndMap;

    SWND m_hNextWnd;
};

}//namespace SOUI