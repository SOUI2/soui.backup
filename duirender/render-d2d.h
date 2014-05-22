#pragma once

#include "render-i.h"
#include "color.h"
#include "obj-ref-impl.hpp"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>


namespace SOUI
{

	//实现一些和特定系统相关的接口
	struct IRenderFactory_D2D : public IRenderFactory
	{
		virtual ID2D1Factory * GetD2DFactory() const = 0;
		virtual IDWriteFactory * GetWriteFactory() const = 0;
		virtual IWICImagingFactory * GetWICImagingFactory() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// SRenderFactory_D2D
	class SRenderFactory_D2D : public TObjRefImpl<IRenderFactory_D2D>
	{
		friend class SBitmap_D2D;
		friend class SRenderTarget_D2D;
	public:
		SRenderFactory_D2D()
		{
			Init();
		}

		virtual BOOL CreateHwndRenderTarget(HWND hWnd,SIZE szTarget,IRenderTarget ** ppRenderTarget);
		virtual BOOL CreateDCRenderTarget(IRenderTarget ** ppRenderTarget);

		virtual ID2D1Factory * GetD2DFactory()const {return m_pD2DFactory;}
		virtual IDWriteFactory * GetWriteFactory() const {return m_pDWriteFactory;}
		virtual IWICImagingFactory * GetWICImagingFactory() const {return m_pWICImageFactory;}

	protected:
		BOOL Init();

		CAutoRefPtr<ID2D1Factory>  m_pD2DFactory;
		CAutoRefPtr<IDWriteFactory> m_pDWriteFactory;
		CAutoRefPtr<IWICImagingFactory> m_pWICImageFactory;
	};


	//////////////////////////////////////////////////////////////////////////
	// TD2DRenderObjImpl
	template<class T>
	class TD2DRenderObjImpl : public TObjRefImpl<T>
	{
	public:
		TD2DRenderObjImpl(IRenderFactory_D2D * pRenderFac):m_pRenderFactory(pRenderFac)
		{

		}

		virtual IRenderFactory * GetRenderFactory() const
		{
			return m_pRenderFactory;
		}

		virtual IRenderFactory_D2D * GetRenderFactory_D2D() const
		{
			return m_pRenderFactory;
		}
	protected:
		IRenderFactory_D2D *m_pRenderFactory;
	};

	//////////////////////////////////////////////////////////////////////////
	// SPen_D2D
	class SPen_D2D : public TD2DRenderObjImpl<IPen>
	{
		friend class SRenderTarget_D2D;
	public:
		SPen_D2D(IRenderFactory_D2D * pRenderFac,ID2D1SolidColorBrush *pBrush,ID2D1StrokeStyle *pIStyle,int nWidth)
			:TD2DRenderObjImpl<IPen>(pRenderFac),m_brush(pBrush),m_style(pIStyle),m_nWidth(nWidth)
		{
		}
		ID2D1SolidColorBrush * GetBrush(){return m_brush;}
		ID2D1StrokeStyle *GetStrokeStyle(){return m_style;}
		int			GetWidth(){return m_nWidth;}
	protected:

		CAutoRefPtr<ID2D1SolidColorBrush> m_brush;
		CAutoRefPtr<ID2D1StrokeStyle>	m_style;
		int		m_nWidth;
	};

	//////////////////////////////////////////////////////////////////////////
	// SFont_D2D
	class SFont_D2D: public TD2DRenderObjImpl<IFont>
	{
	public:
		SFont_D2D(IRenderFactory_D2D * pRenderFac,IDWriteTextFormat *pTextFormat)
			:TD2DRenderObjImpl<IFont>(pRenderFac),m_pTextFormat(pTextFormat)
		{
		}

		IDWriteTextFormat * GetTextFormat(){return m_pTextFormat;}
	protected:
		CAutoRefPtr<IDWriteTextFormat> m_pTextFormat;
	};

	class SBrush_D2D : public TD2DRenderObjImpl<IBrush>
	{
	public:
		SBrush_D2D(IRenderFactory_D2D * pRenderFac,ID2D1Brush *pBrush)
			:TD2DRenderObjImpl<IBrush>(pRenderFac),m_brush(pBrush)
		{

		}

		ID2D1Brush * GetBrush(){return m_brush;}
	protected:
		CAutoRefPtr<ID2D1Brush> m_brush;
	};

	//////////////////////////////////////////////////////////////////////////
	// SBitmap_D2D
	class SBitmap_D2D : public TD2DRenderObjImpl<IBitmap>
	{
	public:
		SBitmap_D2D(IRenderFactory_D2D *pRenderFac)
			:TD2DRenderObjImpl<IBitmap>(pRenderFac)
		{

		}
		virtual HRESULT Init(IRenderTarget *pRT,int nWid,int nHei);
		virtual HRESULT LoadFromFile(IRenderTarget *pRT,LPCTSTR pszFileName,LPCTSTR pszType);
		virtual HRESULT LoadFromMemory(IRenderTarget *pRT,LPBYTE pBuf,size_t szLen,LPCTSTR pszType);

		ID2D1Bitmap * GetBitmap(){return m_bitmap;}
	protected:

		CAutoRefPtr<ID2D1Bitmap> m_bitmap;
	};

	//////////////////////////////////////////////////////////////////////////
	//	SRegion_D2D
	class SRegion_D2D: public TD2DRenderObjImpl<IRegion>
	{
	public:
		SRegion_D2D(IRenderFactory_D2D *pRenderFac);
		virtual void CombineRect(LPCRECT lprect,int nCombineMode);
		virtual BOOL PtInRegion(POINT pt);
		virtual BOOL RectInRegion(LPCRECT lprect);
		virtual void GetRgnBox(LPRECT lprect);
		virtual BOOL IsEmpty();

		ID2D1Geometry * GetGeometry() {return m_d2dRegion;}
	protected:

		CAutoRefPtr<ID2D1PathGeometry> m_d2dRegion;
	};


	//////////////////////////////////////////////////////////////////////////
	//	CDuiRenderTarget_D2D
	//////////////////////////////////////////////////////////////////////////
	class SRenderTarget_D2D: public TD2DRenderObjImpl<IRenderTarget>
	{
		friend class SBitmap_D2D;
	public:
		SRenderTarget_D2D(ID2D1RenderTarget *pRenderTarget,IRenderFactory_D2D* pRenderFactory);

		//只支持创建位图表面
		virtual HRESULT CreateCompatibleRenderTarget(SIZE szTarget,IRenderTarget **ppRenderTarget);

		virtual HRESULT CreatePen(int iStyle,COLORREF cr,int cWidth,IPen ** ppPen);
		virtual HRESULT CreateFont( const LOGFONT &lf,IFont ** ppFont );
		virtual HRESULT CreateSolidColorBrush(COLORREF cr,IBrush ** ppBrush);
		virtual HRESULT CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush );
		virtual HRESULT CreateRegion(IRegion ** ppRegion);
		virtual HRESULT CreateBitmap(IBitmap ** ppBitmap);

		virtual HRESULT BindDC(HDC hdc,LPCRECT pSubRect);
		virtual HRESULT BeginDraw();
		virtual HRESULT EndDraw();
		virtual HRESULT Resize(SIZE sz);

		virtual HRESULT PushClipRect(LPCRECT pRect);
		virtual HRESULT PopClipRect();

		virtual HRESULT PushClipRegion(IRegion *pRegion);
		virtual HRESULT PopClipRegion();

		virtual HRESULT BitBlt(LPRECT pRcDest,IRenderTarget *pRTSour,LPRECT pRcSour,UINT uDef);

		virtual HRESULT DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat );
		virtual HRESULT MeasureText(LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat );

		virtual HRESULT DrawRectangle(int left, int top,int right,int bottom);
		virtual HRESULT FillRectangle(int left, int top,int right,int bottom);

		virtual HRESULT TextOut(
			int x,
			int y,
			LPCTSTR lpszString,
			int nCount);

		virtual HRESULT GetTextExtentPoint32(
			LPCTSTR lpString,
			UINT cbCount,
			LPSIZE lpSize
			);

		virtual HRESULT DrawBitmap(LPRECT pRcDest,IBitmap *pBitmap,LPRECT pRcSour,BYTE byAlpha=0xFF);

		virtual IRenderObj * GetCurrentObject(OBJTYPE uType);
		virtual IRenderObj * SelectObject(IRenderObj *pObj);


		virtual COLORREF GetTextColor()
		{
			if(!m_curPen) return 0;

			ID2D1SolidColorBrush *pSolidBrush= m_curPen->GetBrush();
			D2D1_COLOR_F cr=pSolidBrush->GetColor();
			CDuiColor crdui((BYTE)(cr.r*255),(BYTE)(cr.g*255),(BYTE)(cr.b*255),(BYTE)(cr.a*255));
			return crdui;
		}
		virtual COLORREF SetTextColor(COLORREF color)
		{
			if(!m_curBrush) return 0;

			CDuiColor cr(color);
			D2D1_COLOR_F crd2d=D2D1::ColorF((float)cr.r/255.f,(float)cr.g/255.f,(float)cr.b/255.f,(float)cr.a/255.f);
			ID2D1SolidColorBrush *pSolidBrush=m_curPen->GetBrush();
			D2D1_COLOR_F crOld=pSolidBrush->GetColor();
			pSolidBrush->SetColor(crd2d);
			return CDuiColor((BYTE)(crOld.r*255),(BYTE)(crOld.g*255),(BYTE)(crOld.b*255),(BYTE)(crOld.a*255));
		}

	protected:

		void SetTextFormat(IDWriteTextFormat *pTxtFormat,UINT uFormat);
		CAutoRefPtr<SFont_D2D> m_curFont;
		CAutoRefPtr<SBrush_D2D> m_curBrush;
		CAutoRefPtr<SPen_D2D> m_curPen;

		CAutoRefPtr<ID2D1RenderTarget>	m_pD2DRenderTarget;
	};


}//end of namespace SOUI
