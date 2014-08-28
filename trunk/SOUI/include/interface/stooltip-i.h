#pragma once

#include <unknown/obj-ref-i.h>
#include "../core/smsgloop.h"

namespace SOUI
{
    struct TIPID
    {
        DWORD dwHi;
        DWORD dwLow;
    };

    struct IToolTip : public IMessageFilter
    {
        virtual void UpdateTip(const TIPID &id, CRect rc,LPCTSTR pszTip) = 0;

        virtual void ClearTip() = 0;

        virtual void RelayEvent(const MSG *pMsg) = 0;

        virtual void SetDelayTime(DWORD dwType,UINT iTime) = 0;
    };

    struct IToolTipFactory : IObjRef
    {
        virtual IToolTip * CreateToolTip(HWND hHost) = 0;

        virtual void DestroyToolTip(IToolTip *pToolTip) = 0;
    };
}