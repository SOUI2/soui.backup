#include "souistd.h"
#include "core/SWindowMgr.h"
#include "helper/STimerEx.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
template<> SWindowMgr* SSingleton<SWindowMgr>::ms_Singleton=0;


SWindowMgr::SWindowMgr()
    : m_hNextWnd(NULL)
{
    ::InitializeCriticalSection(&m_lockWndMap);
}

SWindowMgr::~SWindowMgr()
{
    ::DeleteCriticalSection(&m_lockWndMap);
}

// Get DuiWindow pointer from handle
SWindow* SWindowMgr::GetWindow(SWND hDuiWnd)
{
    if(!hDuiWnd) return NULL;
    SWindow *pRet=NULL;
    ::EnterCriticalSection(&getSingleton().m_lockWndMap);

    getSingleton().GetKeyObject(hDuiWnd,pRet);
    ::LeaveCriticalSection(&getSingleton().m_lockWndMap);
    return pRet;
}

// Specify a handle to a DuiWindow
SWND SWindowMgr::NewWindow(SWindow *pDuiWnd)
{
    ASSERT(pDuiWnd);
    ::EnterCriticalSection(&getSingleton().m_lockWndMap);

    SWND hDuiWndNext = ++ getSingleton().m_hNextWnd;
    getSingleton().AddKeyObject(hDuiWndNext,pDuiWnd);
    ::LeaveCriticalSection(&getSingleton().m_lockWndMap);

    return hDuiWndNext;
}

// Destroy DuiWindow
BOOL SWindowMgr::DestroyWindow(SWND hDuiWnd)
{
    ::EnterCriticalSection(&getSingleton().m_lockWndMap);

    BOOL bRet=getSingleton().RemoveKeyObject(hDuiWnd);
    STimerEx::KillTimer(hDuiWnd);

    ::LeaveCriticalSection(&getSingleton().m_lockWndMap);

    return bRet;
}

}//namespace SOUI