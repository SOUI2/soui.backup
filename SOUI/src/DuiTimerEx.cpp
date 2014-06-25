#include "duistd.h"
#include "DuiTimerEx.h"
#include "duiwnd.h"
#include "DuiWindowMgr.h"

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
//    CDuiTimerEx
//////////////////////////////////////////////////////////////////////////
template<> CDuiTimerEx * Singleton<CDuiTimerEx>::ms_Singleton=0;

BOOL CDuiTimerEx::_SetTimer( HSWND hDuiWnd,UINT_PTR uTimerID,UINT nElapse )
{
    _KillTimer(hDuiWnd,uTimerID);
    UINT_PTR idEvent=::SetTimer(NULL,uTimerID,nElapse,_TimerProc);
    if(idEvent==0) return FALSE;
    TIMERINFO ti= {hDuiWnd,uTimerID};
    (*m_mapNamedObj)[idEvent]=ti;
    return TRUE;
}

void CDuiTimerEx::_KillTimer( HSWND hDuiWnd,UINT_PTR uTimerID )
{
    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        CDuiMap<UINT_PTR,TIMERINFO>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value.hSWnd==hDuiWnd && p->m_value.uTimerID==uTimerID)
        {
            ::KillTimer(NULL,p->m_key);
            m_mapNamedObj->RemoveAtPos((POSITION)p);
            break;
        }
    }
}

void CDuiTimerEx::_KillTimer( HSWND hDuiWnd )
{
    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        CDuiMap<UINT_PTR,TIMERINFO>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value.hSWnd==hDuiWnd)
        {
            ::KillTimer(NULL,p->m_key);
            m_mapNamedObj->RemoveAtPos((POSITION)p);
        }
    }
}


VOID CALLBACK CDuiTimerEx::_TimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
    TIMERINFO ti;
    if(getSingleton().GetKeyObject(idEvent,ti))
    {
        SWindow *pDuiWnd=DuiWindowMgr::GetWindow(ti.hSWnd);
        if(pDuiWnd) pDuiWnd->DuiSendMessage(UM_DUI_TIMEREX,ti.uTimerID);
    }
}

}//namespace SOUI