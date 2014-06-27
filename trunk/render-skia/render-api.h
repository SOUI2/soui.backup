#pragma once

#ifndef _LIB
#ifdef RENDERSKIA_EXPORTS
#define RENDERSKIA_API __declspec(dllexport)
#else
#define RENDERSKIA_API __declspec(dllimport)
#endif
#else
#define RENDERSKIA_API
#endif

#define RENDER_API RENDERSKIA_API

#include <render/render-i.h>
#include <render/imgdecoder-i.h>


extern "C" RENDERSKIA_API BOOL CreateRenderFactory_Skia(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory);

