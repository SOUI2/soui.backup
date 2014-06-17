#pragma once

#include <core/skxfermode.h>

namespace SOUI
{
    class SkXfermode_SRCAND : public SkXfermode {
    public:
        SK_DECLARE_INST_COUNT(SkXfermode_SRCAND)

        virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
            const SkAlpha aa[]) const
            {
            
            }
        virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
            const SkAlpha aa[]) const
            {
            
            }
        virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
            const SkAlpha aa[]) const
            {
            
            }
    };
}
