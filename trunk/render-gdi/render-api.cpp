#include "StdAfx.h"
#include "render-api.h"

#include "render-gdi.h"

BOOL SCreateInstance(IObjRef ** ppRenderFactory)
{
    *ppRenderFactory = new SOUI::SRenderFactory_GDI;
    return TRUE;
}
