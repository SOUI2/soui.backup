#include "duistd.h"
#include "DuiThreadActiveWndMgr.h"

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
//    DuiThreadActiveWndManager
//////////////////////////////////////////////////////////////////////////
template<> DuiThreadActiveWndMgr* Singleton<DuiThreadActiveWndMgr>::ms_Singleton = NULL;

DuiThreadActiveWndMgr::DuiThreadActiveWndMgr()
{
    ::InitializeCriticalSection(&m_lockRepaint);
    ::InitializeCriticalSection(&m_lockMapActive);
}

DuiThreadActiveWndMgr::~DuiThreadActiveWndMgr()
{
    ::DeleteCriticalSection(&m_lockRepaint);
    ::DeleteCriticalSection(&m_lockMapActive);
}

HWND DuiThreadActiveWndMgr::SetActive(HWND hWnd)
{
    return getSingleton()._SetActive(hWnd);
}

HWND DuiThreadActiveWndMgr::GetActive()
{
    return getSingleton()._GetActive();
}

void DuiThreadActiveWndMgr::EnterPaintLock()
{
    ::EnterCriticalSection(&getSingleton().m_lockRepaint);
}

void DuiThreadActiveWndMgr::LeavePaintLock()
{
    ::LeaveCriticalSection(&getSingleton().m_lockRepaint);
}

HWND DuiThreadActiveWndMgr::_SetActive(HWND hWnd)
{
    ::EnterCriticalSection(&m_lockMapActive);
    HWND    hWndLastActive    = _GetActive();
    SetKeyObject(::GetCurrentThreadId(),hWnd);
    ::LeaveCriticalSection(&m_lockMapActive);
    return hWndLastActive;
}

HWND DuiThreadActiveWndMgr::_GetActive()
{
    ::EnterCriticalSection(&m_lockMapActive);
    HWND    hWndAct = NULL;
    GetKeyObject(::GetCurrentThreadId(),hWndAct);
    ::LeaveCriticalSection(&m_lockMapActive);
    return hWndAct;
}

}//namespace SOUI