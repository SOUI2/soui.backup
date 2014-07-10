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

#include <interface/render-i.h>
#include <interface/imgdecoder-i.h>


extern "C" RENDERSKIA_API BOOL SCreateInstance(IObjRef ** ppRenderFactory);

