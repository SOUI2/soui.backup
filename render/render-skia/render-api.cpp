#include "StdAfx.h"
#include "render-api.h"

#include "render-skia.h"

namespace RENDER_SKIA
{
    BOOL CreateRenderFactory(SOUI::IRenderFactory ** ppRenderFactory)
    {
        *ppRenderFactory = new SOUI::SRenderFactory_Skia();
        return TRUE;
    }
    
}
