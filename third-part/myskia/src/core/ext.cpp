
#include "ext.h"
#include "SkDevice.h"

char* fontName[]={"SimSun","Microsoft YaHei",0};

const SkBitmap& Canvas2Bitmap(SkCanvas* canvas)
{
	SkBaseDevice *device = canvas->getDevice();
	return device->accessBitmap(false);
}

void* operator new( size_t nSize,int nLine)
{
    return  ::operator new( nSize );
}