
#include "SkBitmapOp.h"
#include "SkBlurMaskFilter.h"

SkBitmapOp::SkBitmapOp()
{
}

bool SkBitmapOp::CreateBlurBitmap(SkBitmap& bitmap, int w, int h, SkScalar radius)
{
	if (w < 1 || h < 1)
	{
		return false;
	}
	else
	{
		bitmap.reset();
		bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
		bitmap.allocPixels();

		SkRect rect = SkRect::MakeWH(w, h);
		SkCanvas canvas(bitmap);
		SkPaint paint;
		//SkMaskFilter* imgMask = SkBlurMaskFilter::Create(radius, SkBlurMaskFilter::BlurStyle::kSolid_BlurStyle);

		//paint.setMaskFilter(imgMask);
		//imgMask->unref();
		paint.setColor(0xFF0000FF);
		paint.setAntiAlias(true);
		paint.setFilterBitmap(true);

		canvas.drawRoundRect(rect, 8, 8, paint);

		return true;
	}
}