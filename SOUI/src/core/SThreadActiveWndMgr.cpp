#include "souistd.h"
#include "core/SThreadActiveWndMgr.h"

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
//    DuiThreadActiveWndManager
//////////////////////////////////////////////////////////////////////////
template<> SThreadActiveWndMgr* SSingleton<SThreadActiveWndMgr>::ms_Singleton = NULL;

SThreadActiveWndMgr::SThreadActiveWndMgr()
{
    ::InitializeCriticalSection(&m_lockRepaint);
    ::InitializeCriticalSection(&m_lockMapActive);
}

SThreadActiveWndMgr::~SThreadActiveWndMgr()
{
    ::DeleteCriticalSection(&m_lockRepaint);
    ::DeleteCriticalSection(&m_lockMapActive);
}

HWND SThreadActiveWndMgr::SetActive(HWND hWnd)
{
    return getSingleton()._SetActive(hWnd);
}

HWND SThreadActiveWndMgr::GetActive()
{
    return getSingleton()._GetActive();
}

void SThreadActiveWndMgr::EnterPaintLock()
{
    ::EnterCriticalSection(&getSingleton().m_lockRepaint);
}

void SThreadActiveWndMgr::LeavePaintLock()
{
    ::LeaveCriticalSection(&getSingleton().m_lockRepaint);
}

HWND SThreadActiveWndMgr::_SetActive(HWND hWnd)
{
    ::EnterCriticalSection(&m_lockMapActive);
    HWND    hWndLastActive    = _GetActive();
    SetKeyObject(::GetCurrentThreadId(),hWnd);
    ::SetActiveWindow(hWnd);
    ::LeaveCriticalSection(&m_lockMapActive);
    return hWndLastActive;
}

HWND SThreadActiveWndMgr::_GetActive()
{
    ::EnterCriticalSection(&m_lockMapActive);
    HWND    hWndAct = NULL;
    GetKeyObject(::GetCurrentThreadId(),hWndAct);
    ::LeaveCriticalSection(&m_lockMapActive);
    return hWndAct;
}

}//namespace SOUI