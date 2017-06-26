#include "stdafx.h"
#include "RichEditUnitConverter.h"

BOOL RichEditUintConverter::GetDPI(UINT &dpi, BOOL bIsHeightPx)
{
    HDC hdc = GetDC(NULL);
    if (hdc == NULL)
    {
        return FALSE;
    }

    if (bIsHeightPx)
    {
        //Number of pixels per logical inch along the screen width. 
        dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    }
    else
    {
        //Number of pixels per logical inch along the screen height.
        dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    }

    ReleaseDC(NULL, hdc);
    return TRUE;
}

BOOL RichEditUintConverter::PointToPixel(FLOAT pt, FLOAT &px)
{
    UINT dpi;
    if (!GetDPI(dpi))
    {
        return FALSE;
    }

    px = pt * dpi / 72;
    return TRUE;
}

void RichEditUintConverter::PointToPixel(FLOAT pt, UINT dpi, FLOAT &px)
{
    px = pt * dpi / 72;
}

// px to pt
BOOL RichEditUintConverter::PixelToPoint(FLOAT px, FLOAT &pt)
{
    UINT dpi;
    if (!GetDPI(dpi))
    {
        return FALSE;
    }

    pt = px * 72 / dpi;
    return TRUE;
}

void RichEditUintConverter::PixelToPoint(FLOAT px, UINT dpi, FLOAT &pt)
{
    pt = px * 72 / dpi;
}

// px to twips
BOOL RichEditUintConverter::PixelToTwips(FLOAT px, FLOAT &ltwips)
{
    UINT dpi;
    if (!GetDPI(dpi))
    {
        return FALSE;
    }
    ltwips = px * 1440 / dpi;

    return TRUE;
}

void RichEditUintConverter::PixelToTwips(FLOAT px, UINT dpi, FLOAT &ltwips)
{
    ltwips = px * 1440 / dpi;
}
