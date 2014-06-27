#include "StdAfx.h"
#include "render-api.h"

#include "render-gdi.h"

BOOL CreateRenderFactory_GDI(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory)
{
    *ppRenderFactory = new SOUI::SRenderFactory_GDI(pImgDecoderFactory);
    return TRUE;
}
