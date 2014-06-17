#pragma once

#include "DuiSingletonMap.h"

namespace SOUI
{

typedef struct tagTIMERINFO
{
    HSWND hDuiWnd;
    UINT_PTR uTimerID;
} TIMERINFO;


class SOUI_EXP CDuiTimerEx:public DuiSingletonMap<CDuiTimerEx,TIMERINFO,UINT_PTR>
{
public:
    static BOOL SetTimer(HSWND hDuiWnd,UINT_PTR uTimerID,UINT nElapse)
    {
        return getSingleton()._SetTimer(hDuiWnd,uTimerID,nElapse);
    }

    static void KillTimer(HSWND hDuiWnd,UINT_PTR uTimerID)
    {
        getSingleton()._KillTimer(hDuiWnd,uTimerID);
    }

    static void KillTimer(HSWND hDuiWnd)
    {
        getSingleton()._KillTimer(hDuiWnd);
    }
protected:
    BOOL _SetTimer(HSWND hDuiWnd,UINT_PTR uTimerID,UINT nElapse);

    void _KillTimer(HSWND hDuiWnd,UINT_PTR uTimerID);

    void _KillTimer(HSWND hDuiWnd);

    static VOID CALLBACK _TimerProc(HWND hwnd,
                                    UINT uMsg,
                                    UINT_PTR idEvent,
                                    DWORD dwTime
                                   );
};

}//namespace SOUI