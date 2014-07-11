#include "StdAfx.h"
#include "render-api.h"

#include "render-skia.h"

BOOL SCreateInstance(IObjRef ** ppRenderFactory)
{
    *ppRenderFactory = new SOUI::SRenderFactory_Skia;
    return TRUE;
}
