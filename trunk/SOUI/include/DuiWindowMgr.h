//////////////////////////////////////////////////////////////////////////
// CDuiWindow Handle Manager
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "DuiSingletonMap.h"

namespace SOUI
{

class CDuiWindow;

class SOUI_EXP DuiWindowMgr :public DuiSingletonMap<DuiWindowMgr,CDuiWindow*,HDUIWND>
{
public:

    DuiWindowMgr();

    ~DuiWindowMgr();

    // Get DuiWindow pointer from handle
    static CDuiWindow* GetWindow(HDUIWND hDuiWnd);

    // Specify a handle to a DuiWindow
    static HDUIWND NewWindow(CDuiWindow *pDuiWnd);

    // Destroy DuiWindow
    static BOOL DestroyWindow(HDUIWND hDuiWnd);
protected:

    CRITICAL_SECTION m_lockWndMap;

    HDUIWND m_hNextWnd;
};

}//namespace SOUI