#pragma once

#include <core\SkCanvas.h>
#include <core\SkBitmap.h>
#include <core\SkTypeface.h>

#include "render-i.h"
#include "color.h"
#include "obj-ref-impl.hpp"
#include <list>

namespace SOUI
{
	//实现一些和特定系统相关的接口
	struct IRenderFactory_Skia : public IRenderFactory
	{
	};


	//////////////////////////////////////////////////////////////////////////
	// SRenderFactory_Skia
	class SRenderFactory_Skia : public TObjRefImpl<IRenderFactory_Skia>
	{
	public:
		SRenderFactory_Skia()
		{
			Init();
		}

		virtual BOOL CreateRenderTarget(IRenderTarget ** ppRenderTarget,int nWid,int nHei);

	protected:
		BOOL Init();

	};


	//////////////////////////////////////////////////////////////////////////
	// TSkiaRenderObjImpl
	template<class T>
	class TSkiaRenderObjImpl : public TObjRefImpl<T>
	{
	public:
		TSkiaRenderObjImpl(IRenderFactory_Skia * pRenderFac):m_pRenderFactory(pRenderFac)
		{

		}

		virtual IRenderFactory * GetRenderFactory() const
		{
			return m_pRenderFactory;
		}

		virtual IRenderFactory_Skia * GetRenderFactory_Skia() const
		{
			return m_pRenderFactory;
		}
	protected:
		IRenderFactory_Skia *m_pRenderFactory;
	};


	//////////////////////////////////////////////////////////////////////////
	// SPen_Skia
	class SPen_Skia : public TSkiaRenderObjImpl<IPen>
	{
	public:
		SPen_Skia(IRenderFactory_Skia * pRenderFac,int iStyle=PS_SOLID,COLORREF cr=0xFF000000,int cWidth=1)
			:TSkiaRenderObjImpl<IPen>(pRenderFac)
			,m_nWidth(cWidth),m_style(iStyle),m_cr(cr)
		{
		}

		int GetWidth(){return m_nWidth;}

		int GetStyle(){return m_style;}

		void SetWidth(int nWid) {m_nWidth=nWid;}

		void SetStyle(int nStyle){m_style = nStyle;}

		COLORREF GetColor(){return m_cr;}

		void SetColor(COLORREF cr){m_cr = cr;}
	protected:
	
		int		m_style;
		int		m_nWidth;
		COLORREF	m_cr;
	};

	//////////////////////////////////////////////////////////////////////////
	// SFont_Skia
	class SFont_Skia: public TSkiaRenderObjImpl<IFont>
	{
	public:
		SFont_Skia(IRenderFactory_Skia * pRenderFac,const LOGFONT * plf)
			:TSkiaRenderObjImpl<IFont>(pRenderFac),m_skFont(NULL)
		{
            CDuiStringA strFace=DUI_CT2A(plf->lfFaceName,CP_ACP);
            BYTE style=SkTypeface::kNormal;
            if(plf->lfItalic) style |= SkTypeface::kItalic;
            if(plf->lfWeight == FW_BOLD) style |= SkTypeface::kBold;

            m_skFont=SkTypeface::CreateFromName(strFace,(SkTypeface::Style)style);
            m_skPaint.setTextSize(plf->lfHeight);
            m_skPaint.setUnderlineText(plf->lfUnderline);
            m_skPaint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
            m_skPaint.setAntiAlias(true);
		}

        const SkPaint  GetPaint() const {return m_skPaint;}
        SkTypeface *GetFont()const {return m_skFont;}
	protected:
        SkTypeface *m_skFont;   //定义字体
        SkPaint     m_skPaint;  //定义文字绘制属性
	};

	class SBrush_Skia : public TSkiaRenderObjImpl<IBrush>
	{
	public:
		static SBrush_Skia * CreateSolidBrush(IRenderFactory_Skia * pRenderFac,COLORREF cr){
			return new SBrush_Skia(pRenderFac,cr);
		}

		static SBrush_Skia * CreateBitmapBrush(IRenderFactory_Skia * pRenderFac,SkBitmap bmp)
		{
			return new SBrush_Skia(pRenderFac,bmp);
		}

		SkBitmap GetBitmap(){return m_bmp;}

		COLORREF GetColor() {return m_cr;}

		BOOL IsBitmap(){return m_fBmp;}
	protected:
		SBrush_Skia(IRenderFactory_Skia * pRenderFac,COLORREF cr)
			:TSkiaRenderObjImpl<IBrush>(pRenderFac),m_cr(cr),m_fBmp(FALSE)
		{

		}
		SBrush_Skia(IRenderFactory_Skia * pRenderFac,SkBitmap bmp)
			:TSkiaRenderObjImpl<IBrush>(pRenderFac),m_bmp(bmp),m_fBmp(TRUE)
		{

		}

		COLORREF m_cr;		//颜色画刷
		SkBitmap m_bmp;		//位图画刷
		BOOL	 m_fBmp;
	};

	//////////////////////////////////////////////////////////////////////////
	// SBitmap_Skia
	class SBitmap_Skia : public TSkiaRenderObjImpl<IBitmap>
	{
	public:
		SBitmap_Skia(IRenderFactory_Skia *pRenderFac)
			:TSkiaRenderObjImpl<IBitmap>(pRenderFac)
		{

		}
		virtual HRESULT Init(IRenderTarget *pRT,int nWid,int nHei);
		virtual HRESULT LoadFromFile(IRenderTarget *pRT,LPCTSTR pszFileName,LPCTSTR pszType);
		virtual HRESULT LoadFromMemory(IRenderTarget *pRT,LPBYTE pBuf,size_t szLen,LPCTSTR pszType);

		SkBitmap GetBitmap(){return m_bitmap;}
	protected:

		SkBitmap m_bitmap;
	};

	//////////////////////////////////////////////////////////////////////////
	//	SRegion_Skia
	class SRegion_Skia: public TSkiaRenderObjImpl<IRegion>
	{
	public:
		SRegion_Skia(IRenderFactory_Skia *pRenderFac);
		virtual void CombineRect(LPCRECT lprect,int nCombineMode);
		virtual BOOL PtInRegion(POINT pt);
		virtual BOOL RectInRegion(LPCRECT lprect);
		virtual void GetRgnBox(LPRECT lprect);
		virtual BOOL IsEmpty();

        SkRegion GetRegion() const {
            return m_rgn;
        }
        
        void SetRegion(const SkRegion & rgn)
        {
            m_rgn=rgn;
        }
	protected:
        SkRegion    m_rgn;
	};


	//////////////////////////////////////////////////////////////////////////
	//	SRenderTarget_Skia
	//////////////////////////////////////////////////////////////////////////
	class SRenderTarget_Skia: public TSkiaRenderObjImpl<IRenderTarget>
	{
	public:
		SRenderTarget_Skia(IRenderFactory_Skia* pRenderFactory,int nWid,int nHei);
		~SRenderTarget_Skia();

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

        virtual HRESULT GetClipRegion(IRegion **ppRegion);
        
		virtual HRESULT BitBlt(LPRECT pRcDest,IRenderTarget *pRTSour,LPRECT pRcSour,UINT uDef);

		virtual HRESULT DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat ,BYTE byAlpha=0xFF);
		virtual HRESULT MeasureText(LPCTSTR pszText,int cchLen, SIZE *psz );

		virtual HRESULT DrawRectangle(int left, int top,int right,int bottom);
		virtual HRESULT FillRectangle(int left, int top,int right,int bottom);

		virtual HRESULT TextOut(
			int x,
			int y,
			LPCTSTR lpszString,
			int nCount,
            BYTE byAlpha =0xFF);

		virtual HRESULT DrawBitmap(LPRECT pRcDest,IBitmap *pBitmap,LPRECT pRcSour,BYTE byAlpha=0xFF);

		virtual IRenderObj * GetCurrentObject(OBJTYPE uType);
		virtual IRenderObj * SelectObject(IRenderObj *pObj);


		virtual COLORREF GetTextColor()
		{
			return m_curColor;
		}
		virtual COLORREF SetTextColor(COLORREF color)
		{
			COLORREF crOld=m_curColor;
 			m_curColor=color;
			return crOld;
		}
	protected:
		SkCanvas *m_SkCanvas;
        COLORREF            m_curColor;
		CAutoRefPtr<SBitmap_Skia> m_curBmp;
		CAutoRefPtr<SPen_Skia> m_curPen;
		CAutoRefPtr<SBrush_Skia> m_curBrush;
        CAutoRefPtr<SFont_Skia> m_curFont;
    

		HDC m_hBindDC;
		RECT m_rcBind;
	};
}