#pragma once
#include <core/SkPaint.h>
#include <core/SkCanvas.h>

void DrawText_Skia(SkCanvas* canvas,const wchar_t *text,int len,SkRect box,const SkPaint& paint,UINT uFormat);