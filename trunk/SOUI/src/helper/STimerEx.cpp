#include "souistd.h"
#include "helper/STimerEx.h"
#include "core/SWnd.h"
#include "core/SWindowMgr.h"

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
//    CDuiTimerEx
//////////////////////////////////////////////////////////////////////////
template<> STimerEx * SSingleton<STimerEx>::ms_Singleton=0;

BOOL STimerEx::_SetTimer( SWND hDuiWnd,UINT_PTR uTimerID,UINT nElapse )
{
    _KillTimer(hDuiWnd,uTimerID);
    UINT_PTR idEvent=::SetTimer(NULL,uTimerID,nElapse,_TimerProc);
    if(idEvent==0) return FALSE;
    TIMERINFO ti= {hDuiWnd,uTimerID};
    (*m_mapNamedObj)[idEvent]=ti;
    return TRUE;
}

void STimerEx::_KillTimer( SWND hDuiWnd,UINT_PTR uTimerID )
{
    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        SMap<UINT_PTR,TIMERINFO>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value.Swnd==hDuiWnd && p->m_value.uTimerID==uTimerID)
        {
            ::KillTimer(NULL,p->m_key);
            m_mapNamedObj->RemoveAtPos((POSITION)p);
            break;
        }
    }
}

void STimerEx::_KillTimer( SWND Swnd )
{
    POSITION pos=m_mapNamedObj->GetStartPosition();
    while(pos)
    {
        SMap<UINT_PTR,TIMERINFO>::CPair *p=m_mapNamedObj->GetNext(pos);
        if(p->m_value.Swnd==Swnd)
        {
            ::KillTimer(NULL,p->m_key);
            m_mapNamedObj->RemoveAtPos((POSITION)p);
        }
    }
}


VOID CALLBACK STimerEx::_TimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
    TIMERINFO ti;
    if(getSingleton().GetKeyObject(idEvent,ti))
    {
        SWindow *pSwnd=SWindowMgr::GetWindow(ti.Swnd);
        if(pSwnd) pSwnd->SendSwndMessage(UM_DUI_TIMEREX,ti.uTimerID);
    }
}

}//namespace SOUI