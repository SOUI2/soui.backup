#pragma once

#ifndef _LIB
#ifdef RENDERGDI_EXPORTS
#define RENDERGDI_API __declspec(dllexport)
#else
#define RENDERGDI_API __declspec(dllimport)
#endif
#else
#define RENDERGDI_API
#endif

#include <interface/render-i.h>
#include <interface/imgdecoder-i.h>

extern "C" RENDERGDI_API BOOL CreateRenderFactory_GDI(SOUI::IRenderFactory ** ppRenderFactory,SOUI::IImgDecoderFactory *pImgDecoderFactory);

