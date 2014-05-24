#pragma once


#include <assert.h>
#define ASSERT(x) assert(x)

#include "obj-ref-i.h"

namespace SOUI
{
	struct IBrush;
	struct IPen;
	struct IFont;
	struct IBitmap;
	struct IRegion;
	struct IRenderTarget;
	struct IRenderFactory;


	enum OBJTYPE
	{
		OT_NULL=0,
		OT_PEN,
		OT_BRUSH,
		OT_BITMAP,
		OT_FONT,
		OT_RGN,
	};


struct IRenderObj : public IObjRef
{
	virtual const UINT ObjectType() const = 0;
	virtual IRenderFactory * GetRenderFactory() const = 0;
};


struct IBrush : public IRenderObj
{
	virtual const UINT ObjectType() const
	{
		return OT_BRUSH;
	}
};

struct IPen: public IRenderObj
{
	virtual const UINT ObjectType() const
	{
		return OT_PEN;
	}
};

struct IBitmap: public IRenderObj
{
	virtual const UINT ObjectType() const
	{
		return OT_BITMAP;
	}
	virtual HRESULT Init(IRenderTarget *pRT,int nWid,int nHei)=0;
	virtual HRESULT LoadFromFile(IRenderTarget *pRT,LPCTSTR pszFileName,LPCTSTR pszType)=0;
	virtual HRESULT LoadFromMemory(IRenderTarget *pRT,LPBYTE pBuf,size_t szLen,LPCTSTR pszType)=0;
};

struct IFont : public IRenderObj
{
	virtual const UINT ObjectType() const
	{
		return OT_FONT;
	}
};

struct IRegion : public IRenderObj
{
	virtual const UINT ObjectType() const
	{
		return OT_RGN;
	}

	virtual void CombineRect(LPCRECT lprect,int nCombineMode )=0;
	virtual BOOL PtInRegion(POINT pt)=0;
	virtual BOOL RectInRegion(LPCRECT lprect)=0;
	virtual void GetRgnBox(LPRECT lprect)=0;
	virtual BOOL IsEmpty()=0;
};

//创建设备相关资源
struct IRenderTarget: public IObjRef
{
	virtual HRESULT CreateCompatibleRenderTarget(SIZE szTarget,IRenderTarget **ppRenderTarget)=0;
	virtual HRESULT CreatePen(int iStyle,COLORREF cr,int cWidth,IPen ** ppPen)=0;
	virtual HRESULT CreateFont( const LOGFONT &lf,IFont ** ppFont )=0;
	virtual HRESULT CreateSolidColorBrush(COLORREF cr,IBrush ** ppBrush)=0;
	virtual HRESULT CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )=0;
	virtual HRESULT CreateRegion(IRegion ** ppRegion)=0;
	virtual HRESULT CreateBitmap(IBitmap ** ppBitmap)=0;

	virtual HRESULT BindDC(HDC,LPCRECT)=0;
	virtual HRESULT BeginDraw()=0;
	virtual HRESULT EndDraw()=0;
	virtual HRESULT Resize(SIZE sz)=0;

	virtual HRESULT PushClipRect(LPCRECT pRect)=0;
	virtual HRESULT PopClipRect()=0;

	virtual HRESULT PushClipRegion(IRegion *pRegion)=0;
	virtual HRESULT PopClipRegion()=0;

	virtual HRESULT DrawText(LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat,BYTE byAlpha =0xFF)=0;
    virtual HRESULT MeasureText(LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat ) =0;
    virtual HRESULT TextOut(int x,int y, LPCTSTR lpszString,int nCount,BYTE byAlpha = 0xFF) =0;

    virtual HRESULT GetTextExtentPoint32(LPCTSTR lpString,UINT cbCount,LPSIZE lpSize) =0;

	virtual HRESULT DrawRectangle(int left, int top,int right,int bottom)=0;
	virtual HRESULT FillRectangle(int left, int top,int right,int bottom)=0;
	virtual HRESULT DrawBitmap(LPRECT pRcDest,IBitmap *pBitmap,LPRECT pRcSour,BYTE byAlpha=0xFF)=0;
	virtual HRESULT BitBlt(LPRECT pRcDest,IRenderTarget *pRTSour,LPRECT rcSour,UINT uDef)=0;

	virtual IRenderObj * GetCurrentObject(OBJTYPE uType) =0;
	virtual IRenderObj * SelectObject(IRenderObj *pObj) =0;
	virtual COLORREF GetTextColor() =0;
	virtual COLORREF SetTextColor(COLORREF color)=0;
};

//用来创建设备无关资源
struct IRenderFactory : public IObjRef
{
	virtual BOOL CreateHwndRenderTarget(HWND hWnd,SIZE szTarget,IRenderTarget ** ppRenderTarget)=0;
	virtual BOOL CreateDCRenderTarget(IRenderTarget ** ppRenderTarget)=0;
};

}//end of namespace SOUI
