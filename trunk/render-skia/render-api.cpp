#include "StdAfx.h"
#include "render-api.h"

#include "render-skia.h"

namespace RENDER_SKIA
{
    SOUI::IRenderFactory * CreateRenderFactory()
    {
        return new SOUI::SRenderFactory_Skia();
    }
    
}
