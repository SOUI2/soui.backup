#pragma once

#include "DuiSingletonMap.h"

namespace SOUI
{

typedef struct tagTIMERINFO
{
    SWND hSWnd;
    UINT_PTR uTimerID;
} TIMERINFO;


class SOUI_EXP CDuiTimerEx:public DuiSingletonMap<CDuiTimerEx,TIMERINFO,UINT_PTR>
{
public:
    static BOOL SetTimer(SWND hDuiWnd,UINT_PTR uTimerID,UINT nElapse)
    {
        return getSingleton()._SetTimer(hDuiWnd,uTimerID,nElapse);
    }

    static void KillTimer(SWND hDuiWnd,UINT_PTR uTimerID)
    {
        getSingleton()._KillTimer(hDuiWnd,uTimerID);
    }

    static void KillTimer(SWND hDuiWnd)
    {
        getSingleton()._KillTimer(hDuiWnd);
    }
protected:
    BOOL _SetTimer(SWND hDuiWnd,UINT_PTR uTimerID,UINT nElapse);

    void _KillTimer(SWND hDuiWnd,UINT_PTR uTimerID);

    void _KillTimer(SWND hDuiWnd);

    static VOID CALLBACK _TimerProc(HWND hwnd,
                                    UINT uMsg,
                                    UINT_PTR idEvent,
                                    DWORD dwTime
                                   );
};

}//namespace SOUI