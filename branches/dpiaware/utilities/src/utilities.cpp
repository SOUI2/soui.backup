#include "utilities.h"
#include "string/strcpcvt.h"

namespace SOUI
{
#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

    void SHiMetricToPixel(const SIZEL * lpSizeInHiMetric, LPSIZEL lpSizeInPix)
    {
        int nPixelsPerInchX;    // Pixels per logical inch along width
        int nPixelsPerInchY;    // Pixels per logical inch along height

        HDC hDCScreen = GetDC(NULL);
        nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
        nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
        ReleaseDC(NULL, hDCScreen);

        lpSizeInPix->cx = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cx, nPixelsPerInchX);
        lpSizeInPix->cy = MAP_LOGHIM_TO_PIX(lpSizeInHiMetric->cy, nPixelsPerInchY);
    }

    void SPixelToHiMetric(const SIZEL * lpSizeInPix, LPSIZEL lpSizeInHiMetric)
    {
        int nPixelsPerInchX;    // Pixels per logical inch along width
        int nPixelsPerInchY;    // Pixels per logical inch along height

        HDC hDCScreen = GetDC(NULL);
        nPixelsPerInchX = GetDeviceCaps(hDCScreen, LOGPIXELSX);
        nPixelsPerInchY = GetDeviceCaps(hDCScreen, LOGPIXELSY);
        ReleaseDC(NULL, hDCScreen);

        lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
        lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
    }

	//屏幕像素密度为96每英寸时,1px=1dp
	const int KPixelPerInch = 96;

	float SPx2Dp(int nPx, bool bVert, HWND hwnd /*= NULL*/)
	{
		HDC hdc = GetDC(hwnd);
		int pxPerInch = GetDeviceCaps(hdc, bVert?LOGPIXELSY:LOGPIXELSX);
		ReleaseDC(hwnd, hdc);

		return (float)((double)nPx*KPixelPerInch/pxPerInch);
	}

	int SDp2Px(float fDp,bool bVert, HWND hwnd /*= NULL*/)
	{
		HDC hdc = GetDC(hwnd);
		int pxPerInch = GetDeviceCaps(hdc, bVert?LOGPIXELSY:LOGPIXELSX);
		ReleaseDC(hwnd, hdc);
		
		return (int)((double)fDp * pxPerInch / KPixelPerInch);
	}

}//end of namespace SOUI
