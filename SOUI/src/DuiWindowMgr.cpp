#include "duistd.h"
#include "DuiWindowMgr.h"
#include "DuiTimerEx.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
template<> DuiWindowMgr* Singleton<DuiWindowMgr>::ms_Singleton=0;


DuiWindowMgr::DuiWindowMgr()
    : m_hNextWnd(NULL)
{
    ::InitializeCriticalSection(&m_lockWndMap);
}

DuiWindowMgr::~DuiWindowMgr()
{
    ::DeleteCriticalSection(&m_lockWndMap);
}

// Get DuiWindow pointer from handle
CDuiWindow* DuiWindowMgr::GetWindow(HDUIWND hDuiWnd)
{
    if(!hDuiWnd) return NULL;
    CDuiWindow *pRet=NULL;
    ::EnterCriticalSection(&getSingleton().m_lockWndMap);

    getSingleton().GetKeyObject(hDuiWnd,pRet);
    ::LeaveCriticalSection(&getSingleton().m_lockWndMap);
    return pRet;
}

// Specify a handle to a DuiWindow
HDUIWND DuiWindowMgr::NewWindow(CDuiWindow *pDuiWnd)
{
    DUIASSERT(pDuiWnd);
    ::EnterCriticalSection(&getSingleton().m_lockWndMap);

    HDUIWND hDuiWndNext = ++ getSingleton().m_hNextWnd;
    getSingleton().AddKeyObject(hDuiWndNext,pDuiWnd);
    ::LeaveCriticalSection(&getSingleton().m_lockWndMap);

    return hDuiWndNext;
}

// Destroy DuiWindow
BOOL DuiWindowMgr::DestroyWindow(HDUIWND hDuiWnd)
{
    ::EnterCriticalSection(&getSingleton().m_lockWndMap);

    BOOL bRet=getSingleton().RemoveKeyObject(hDuiWnd);
    CDuiTimerEx::KillTimer(hDuiWnd);

    ::LeaveCriticalSection(&getSingleton().m_lockWndMap);

    return bRet;
}

}//namespace SOUI