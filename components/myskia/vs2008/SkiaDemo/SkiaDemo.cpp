// SkiaDemo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SkiaDemo.h"

#include "SkCanvas.h"
#include "SkBitmap.h"
#include "SkTypeface.h"
#include "SkCamera.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkRandom.h"
#include "SkTableMaskFilter.h"
#include "SkBlurDrawLooper.h"
#include "SkLayerRasterizer.h"
#include "SkLightingImageFilter.h"
#include "SkGradientShader.h"

#include "./src/SkTextOp.h"
#include "./src/SkBitmapOp.h"

int CalcTextSize(int size)
{
	HDC hdc = ::GetDC(NULL);
    int iCaps = ::GetDeviceCaps(hdc,  LOGPIXELSY);
    int iSize = MulDiv(size, iCaps, 72);
    ::ReleaseDC(NULL, hdc);

	return iSize;
}

void DrawGdi(HDC mdc)
{
	int iSize = CalcTextSize(12);

    HFONT hfont = ::CreateFontA(
        -iSize,				       // nHeight
        0,						   // nWidth8
        0,						   // nEscapement
        0,						   // nOrientation
        FW_NORMAL,				   // nWeight
        FALSE,					   // bItalic
        false,				   // bUnderline
        0,                         // cStrikeOut
        DEFAULT_CHARSET,           // nCharSet
        OUT_DEFAULT_PRECIS,        // nOutPrecision
        CLIP_DEFAULT_PRECIS,       // nClipPrecision
        PROOF_QUALITY ,             // nQuality
        DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
        "宋体");             // lpszFacename

    char* strText = "c:\\program files\\测试目录\\images\\file";
    int len = strlen(strText);

	int iMode = ::SetBkMode(mdc, TRANSPARENT);
    HFONT hold = (HFONT)::SelectObject(mdc, hfont);
    //TextOutA(mdc, 10, 360,  strText, len);
	RECT rect = {10, 360, 40, 400};
	DrawTextA(mdc, strText, len, &rect, DT_SINGLELINE|DT_PATH_ELLIPSIS);
    ::SelectObject(mdc, hold);
	::SetBkMode(mdc, iMode);
    DeleteObject(hfont);
}

void DrawBitmap(HWND hwnd, HDC hdc, SkBitmap& bitmap)
{
    RECT rect;
    ::GetClientRect(hwnd, &rect);

    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;

    BITMAP bmp;
    bmp.bmType = 0;
    bmp.bmWidth  = w;
    bmp.bmHeight = h;
    bmp.bmWidthBytes = w<<2;
    bmp.bmPlanes = 1;
    bmp.bmBitsPixel = 32;
    bmp.bmBits = bitmap.getPixels();

    HDC mdc = CreateCompatibleDC(hdc);

    if (NULL != mdc)
    {
        HBITMAP hBitmap = CreateBitmapIndirect(&bmp);
        HBITMAP hOldBitmap = (HBITMAP)::SelectObject(mdc,hBitmap);

        DrawGdi(mdc);
        BitBlt(hdc,0,0,w,h,mdc,0,0,SRCCOPY);

        SelectObject(mdc, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(mdc);
    }
}

void DrawLCD(SkCanvas* cv, int wid, int hei) 
{
    SkBitmap bitmap;

	SkScalar y = 0;
    SkScalar w = wid - 20;
    SkScalar h = hei - 100;

    bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bitmap.allocPixels();

    SkCanvas canvas(bitmap);
	SkPaint paint;

    canvas.clear(0xFFFFFF66);
    paint.setAntiAlias(true);
    SkTypeface* skface = SkTypeface::CreateFromName("宋体", SkTypeface::kNormal);
    paint.setTypeface(skface);
    
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
	

    paint.setColor(0xFF0000FF);
    paint.setUnderlineText(true);
    paint.setAutohinted(true);
    paint.setFakeBoldText(true);
	paint.setLCDRenderText(true);
    //paint.setTextSkewX(-0.4);
	//paint.setEmbeddedBitmapText(true);

    wchar_t* strText = L"互联网公司，International，\ncanvas->drawText(str, len, 10, y, paint) \n互联网公司，International，canvas->drawText(str, len, 10, y, paint) 互联网公司，International，canvas->drawText(str, len, 10, y, paint)";
    int len = wcslen(strText);

	wchar_t* strText1 = L"c:\\program files\\测试目录\\images\\file";
    int len1 = wcslen(strText1);

    paint.setAutohinted(true);
    //paint.setSubpixelText(true);
    paint.setTextSize(CalcTextSize(12));

	SkRect skrc = SkRect::MakeLTRB(0, y, w, h - 50);
	canvas.save();
	canvas.clipRect(skrc);

	// 绘制多行文本
	//SkTextOp::DrawSingleText(&canvas, paint, 0, y, w, h, strText, len);
	SkPoint offset = SkTextOp::DrawWrapText(&canvas, paint, SkRect::MakeLTRB(0, y, w, h), strText, len);
	y = offset.fY;

	canvas.restore();

	SkTextOp::DrawPathEllipsisText(&canvas, paint, 0, y + 20, 250, strText1, len1);
 
     paint.setFilterBitmap(true);
 
     cv->save();
 
     cv->drawBitmap(bitmap, 10, 10, &paint);
     cv->restore();
}

 void DoPaint(HWND hwnd, HDC hdc)
 {
     SkBitmap bitmap;
     RECT rect;
     ::GetClientRect(hwnd, &rect);

     int w = rect.right - rect.left;
     int h = rect.bottom - rect.top;

     bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
     bitmap.allocPixels();

     SkCanvas canvas(bitmap);
     canvas.clear(0xFFFFFFFF);

     DrawLCD(&canvas, w, h);
     //DrawEffectText(&canvas, w, h);

	 //SkBitmap blurBmp;
	 //SkBitmapOp::CreateBlurBitmap(blurBmp, 160, 50, 12);

	 SkPaint paint;
	 SkMaskFilter* imgMask = SkBlurMaskFilter::Create(8, SkBlurMaskFilter::BlurStyle::kSolid_BlurStyle,SkBlurMaskFilter::BlurFlags::kAll_BlurFlag);

	 paint.setMaskFilter(imgMask);
	 imgMask->unref();

	 SkPoint3 direc(200, 200, 50);
	 //SkImageFilter* imgFilter = SkBlurImageFilter::Create(1, 1);
	 //SkImageFilter* imgFilter = SkLightingImageFilter::CreatePointLitSpecular(direc, 0xFFFFFFFF, 100, 200, 100);
	 //paint.setImageFilter(imgFilter);
#ifndef _DEBUG
	 //imgFilter->unref();
#endif
	 //paint.setColor(0xFF00FF00);

	 SkPoint ptCenter = {200, 200};
	 SkPoint pt[2] = {{50, 300},{210, 300}};
	 SkColor clr[2] = {0xFF00FF00, 0xFF0000FF};
	 SkScalar pos[2] = {0, 1};

	 SkShader* pShader = SkGradientShader::CreateLinear(pt, clr, pos, 2, SkShader::TileMode::kMirror_TileMode);
	 //SkShader* pShader = SkGradientShader::CreateRadial(ptCenter, 200, clr, pos, 2, SkShader::TileMode::kMirror_TileMode);
	 paint.setShader(pShader);
	 pShader->unref();

	 SkRect rect1 = SkRect::MakeXYWH(50, 200, 160, 50);
	 paint.setColor(0xFF00FF00);
	 paint.setAntiAlias(true);
	 paint.setFilterBitmap(true);
	 //paint.setStyle(SkPaint::Style::kStroke_Style);
	 canvas.drawRoundRect(rect1, 8, 8, paint);

	 //canvas.drawCircle(200, 200, 100, paint);

	 //canvas.drawBitmap(blurBmp, 50, 200, &paint);
	
     DrawBitmap(hwnd, hdc, bitmap);
 }

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#include "../../include/ports/sktypeface_win.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKIADEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SKIADEMO));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SKIADEMO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SKIADEMO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DoPaint(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
    case WM_ERASEBKGND:
        return 1;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
