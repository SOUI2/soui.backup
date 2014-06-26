#pragma once

#ifdef RENDERGDI_EXPORTS
#define RENDERGDI_API __declspec(dllexport)
#else
#define RENDERGDI_API __declspec(dllimport)
#endif

#define RENDER_API RENDERGDI_API

#include <render/render-i.h>
#include <render/imgdecoder-i.h>


namespace RENDER_GDI
{

extern "C" RENDERGDI_API BOOL CreateRenderFactory(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory);

}

