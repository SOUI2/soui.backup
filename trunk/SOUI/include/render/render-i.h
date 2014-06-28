#pragma once

#include <unknown/obj-ref-i.h>


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
	virtual HRESULT Init(int nWid,int nHei)=0;
	virtual HRESULT LoadFromFile(LPCTSTR pszFileName,LPCTSTR pszType)=0;
	virtual HRESULT LoadFromMemory(LPBYTE pBuf,size_t szLen,LPCTSTR pszType)=0;
	
	virtual UINT    Width() =0;
	virtual UINT    Height() =0;
	virtual SIZE    Size() =0;
};

struct IFont : public IRenderObj
{
	virtual const UINT ObjectType() const
	{
		return OT_FONT;
	}
    virtual const LOGFONT * LogFont() const =0;
    virtual LPCTSTR FamilyName()=0;
    virtual int TextSize()=0;
	virtual BOOL IsBold()=0;
	virtual BOOL IsUnderline()=0;
	virtual BOOL IsItalic()=0;
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
    virtual void Offset(POINT pt)=0;
    virtual void Clear()=0;
};

enum EXPEND_MODE
{
    EM_NULL=0,      //不扩大
    EM_STRETCH,     //拉伸
    EM_TILE,        //平铺
};

//创建设备相关资源
struct IRenderTarget: public IObjRef
{
	virtual HRESULT CreateCompatibleRenderTarget(SIZE szTarget,IRenderTarget **ppRenderTarget)=0;
	virtual HRESULT CreatePen(int iStyle,COLORREF cr,int cWidth,IPen ** ppPen)=0;
	virtual HRESULT CreateSolidColorBrush(COLORREF cr,IBrush ** ppBrush)=0;
	virtual HRESULT CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )=0;

	virtual HRESULT BindDC(HDC,LPCRECT)=0;
	virtual HRESULT BeginDraw()=0;
	virtual HRESULT EndDraw()=0;
	virtual HRESULT Resize(SIZE sz)=0;
    
    virtual HRESULT OffsetViewportOrg(int xOff, int yOff, LPPOINT lpPoint=NULL)=0;
    virtual HRESULT GetViewportOrg(LPPOINT lpPoint) =0;

	virtual HRESULT PushClipRect(LPCRECT pRect,UINT mode=RGN_AND)=0;
	virtual HRESULT PopClipRect()=0;

	virtual HRESULT PushClipRegion(IRegion *pRegion,UINT mode=RGN_AND)=0;
	virtual HRESULT PopClipRegion()=0;

    virtual HRESULT GetClipRegion(IRegion **ppRegion)=0;
    virtual HRESULT GetClipBound(LPRECT prcBound)=0;

	virtual HRESULT DrawText(LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat,BYTE byAlpha =0xFF)=0;
    virtual HRESULT MeasureText(LPCTSTR pszText,int cchLen, SIZE *psz) =0;
    virtual HRESULT TextOut(int x,int y, LPCTSTR lpszString,int nCount,BYTE byAlpha = 0xFF) =0;

	virtual HRESULT DrawRectangle(LPRECT pRect)=0;
	virtual HRESULT FillRectangle(LPRECT pRect)=0;
    virtual HRESULT FillSolidRect(LPCRECT pRect,COLORREF cr)=0;
    virtual HRESULT DrawRoundRect(LPCRECT pRect,POINT pt)=0;
    virtual HRESULT FillRoundRect(LPCRECT pRect,POINT pt)=0;
    
    virtual HRESULT DrawLines(LPPOINT pPt,size_t nCount) =0;
    virtual HRESULT GradientFill(LPCRECT pRect,BOOL bVert,COLORREF crBegin,COLORREF crEnd,BYTE byAlpha=0xFF)=0;
    
    virtual HRESULT DrawIconEx(int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags)=0;
    virtual HRESULT DrawBitmap(LPCRECT pRcDest,IBitmap *pBitmap,int xSrc,int ySrc,BYTE byAlpha=0xFF)=0;
    virtual HRESULT DrawBitmapEx(LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,EXPEND_MODE expendMode, BYTE byAlpha=0xFF)=0;
    virtual HRESULT DrawBitmap9Patch(LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,EXPEND_MODE expendMode,BYTE byAlpha=0xFF) =0;
	virtual HRESULT BitBlt(LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop=SRCCOPY)=0;

	virtual IRenderObj * GetCurrentObject(OBJTYPE uType) =0;
	virtual HRESULT SelectObject(IRenderObj *pObj,IRenderObj ** pOldObj = NULL) =0;
	virtual COLORREF GetTextColor() =0;
	virtual COLORREF SetTextColor(COLORREF color)=0;
	
	//两个兼容GDI操作的接口
	virtual HDC GetDC(UINT uFlag=0)=0;
	virtual void ReleaseDC(HDC hdc) =0;
};

//用来创建设备无关资源
struct IRenderFactory : public IObjRef
{
	virtual BOOL CreateRenderTarget(IRenderTarget ** ppRenderTarget,int nWid,int nHei)=0;
    virtual BOOL CreateFont(IFont ** ppFont, const LOGFONT &lf)=0;
    virtual BOOL CreateBitmap(IBitmap ** ppBitmap)=0;
    virtual BOOL CreateRegion(IRegion **ppRgn)=0;
};

}//end of namespace SOUI
