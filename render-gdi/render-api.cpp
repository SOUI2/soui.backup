#include "StdAfx.h"
#include "render-api.h"

#include "render-gdi.h"

namespace RENDER_GDI
{
    BOOL CreateRenderFactory(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory)
    {
        *ppRenderFactory = new SOUI::SRenderFactory_GDI(pImgDecoderFactory);
        return TRUE;
    }
    
}
