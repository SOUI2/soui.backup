#pragma once

#include "core/SSingletonMap.h"

#define UM_SCRIPTTIMER (WM_USER+3000)

namespace SOUI
{

    struct SCRIPTTIMERINFO
    {
        HWND hwnd;
        SStringA strScriptFunc;
        BOOL bRepeat;
    };

    class SScriptTimer : public SSingletonMap<SScriptTimer,SCRIPTTIMERINFO,UINT_PTR>
    {
    public:
        ~SScriptTimer();

        UINT SetTimer(HWND hwnd,const SStringA & strScriptFunc,UINT nElapse,BOOL bRepeat);

        void ClearTimer(UINT uID);

        static VOID CALLBACK _TimerProc(HWND hwnd,
            UINT uMsg,
            UINT_PTR idEvent,
            DWORD dwTime
            );
    };

}//namespace SOUI