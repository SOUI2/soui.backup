#include "StdAfx.h"
#include "render-api.h"

#include "render-skia.h"

namespace RENDER_SKIA
{
    BOOL CreateRenderFactory(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory)
    {
        *ppRenderFactory = new SOUI::SRenderFactory_Skia(pImgDecoderFactory);
        return TRUE;
    }
    
}
