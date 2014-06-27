#include "StdAfx.h"
#include "render-api.h"

#include "render-skia.h"

BOOL CreateRenderFactory_Skia(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory)
{
    *ppRenderFactory = new SOUI::SRenderFactory_Skia(pImgDecoderFactory);
    return TRUE;
}
