#pragma once

#include "render-api.h"

#include <color.h>
#include <unknown/obj-ref-impl.hpp>

#include <core\SkCanvas.h>
#include <core\SkBitmap.h>
#include <core\SkTypeface.h>

#include <string\tstring.h>
#include <string\strcpcvt.h>

#include "img-decoder.h"

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
		    SImgDecoder::InitImgDecoder();
		}
        
        ~SRenderFactory_Skia()
        {
            SImgDecoder::FreeImgDecoder();
        }
        
		virtual BOOL CreateRenderTarget(IRenderTarget ** ppRenderTarget,int nWid,int nHei);
        virtual BOOL CreateFont(IFont ** ppFont , const LOGFONT &lf);
        virtual BOOL CreateBitmap(IBitmap ** ppBitmap);
        virtual BOOL CreateRegion(IRegion **ppRgn);
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
		    memcpy(&m_lf,plf,sizeof(LOGFONT));
            CDuiStringA strFace=DUI_CT2A(plf->lfFaceName,CP_ACP);
            BYTE style=SkTypeface::kNormal;
            if(plf->lfItalic) style |= SkTypeface::kItalic;
            if(plf->lfWeight == FW_BOLD) style |= SkTypeface::kBold;

            m_skFont=SkTypeface::CreateFromName(strFace,(SkTypeface::Style)style);
            
            m_skPaint.setTextSize((SkScalar)plf->lfHeight);
            m_skPaint.setUnderlineText(!!plf->lfUnderline);
            m_skPaint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
            m_skPaint.setAntiAlias(true);
		}
        virtual const LOGFONT * LogFont() const {return &m_lf;}

        virtual LPCTSTR FamilyName()
        {
            return m_lf.lfFaceName;
        }
        virtual int TextSize(){return m_lf.lfHeight;}
        virtual BOOL IsBold(){ return m_lf.lfWeight == FW_BOLD;}
        virtual BOOL IsUnderline(){return m_lf.lfUnderline;}
        virtual BOOL IsItalic(){return m_lf.lfItalic;}
        
        const SkPaint  GetPaint() const {return m_skPaint;}
        SkTypeface *GetFont()const {return m_skFont;}
	protected:
        SkTypeface *m_skFont;   //定义字体
        SkPaint     m_skPaint;  //定义文字绘制属性
        LOGFONT     m_lf;
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
    class SImgDecoder;
	class SBitmap_Skia : public TSkiaRenderObjImpl<IBitmap>
	{
	public:
		SBitmap_Skia(IRenderFactory_Skia *pRenderFac)
			:TSkiaRenderObjImpl<IBitmap>(pRenderFac),m_hBmp(0)
		{

		}
		virtual HRESULT Init(int nWid,int nHei);
		virtual HRESULT LoadFromFile(LPCTSTR pszFileName,LPCTSTR pszType);
		virtual HRESULT LoadFromMemory(LPBYTE pBuf,size_t szLen,LPCTSTR pszType);

        virtual UINT Width();
        virtual UINT Height();
        virtual SIZE Size();
        
		SkBitmap GetSkBitmap(){return m_bitmap;}
		HBITMAP  GetGdiBitmap(){return m_hBmp;}
	protected:
	    HBITMAP CreateGDIBitmap(int nWid,int nHei,void ** ppBits);
	    
        HRESULT ImgFromDecoder(SImgDecoder &imgDecoder);

		SkBitmap    m_bitmap;   //skia 管理的BITMAP
		HBITMAP     m_hBmp;     //标准的32位位图，和m_bitmap共享内存
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
        virtual void Offset(POINT pt);
        virtual void Clear();
        
        SkRegion GetRegion() const;
        
        void SetRegion(const SkRegion & rgn);
        
        static SkRegion::Op RGNMODE2SkRgnOP(UINT mode);
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
		virtual HRESULT CreateSolidColorBrush(COLORREF cr,IBrush ** ppBrush);
		virtual HRESULT CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush );

		virtual HRESULT BindDC(HDC hdc,LPCRECT pSubRect);
		virtual HRESULT BeginDraw();
		virtual HRESULT EndDraw();
		virtual HRESULT Resize(SIZE sz);

        virtual HRESULT OffsetViewportOrg(int xOff, int yOff, LPPOINT lpPoint=NULL);
        virtual HRESULT GetViewportOrg(LPPOINT lpPoint);

		virtual HRESULT PushClipRect(LPCRECT pRect,UINT mode=RGN_AND);
		virtual HRESULT PopClipRect();

		virtual HRESULT PushClipRegion(IRegion *pRegion,UINT mode=RGN_AND);
		virtual HRESULT PopClipRegion();

        virtual HRESULT GetClipRegion(IRegion **ppRegion);
        virtual HRESULT GetClipBound(LPRECT prcBound);
        
		virtual HRESULT BitBlt(LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop=SRCCOPY);

		virtual HRESULT DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat ,BYTE byAlpha=0xFF);
		virtual HRESULT MeasureText(LPCTSTR pszText,int cchLen, SIZE *psz );

		virtual HRESULT DrawRectangle(LPRECT pRect);
		virtual HRESULT FillRectangle(LPRECT pRect);
        virtual HRESULT FillSolidRect(LPCRECT pRect,COLORREF cr);
        
        virtual HRESULT DrawRoundRect(LPCRECT pRect,POINT pt);
        virtual HRESULT FillRoundRect(LPCRECT pRect,POINT pt);
        virtual HRESULT DrawLines(LPPOINT pPt,size_t nCount);
        virtual HRESULT GradientFill(LPCRECT pRect,BOOL bVert,COLORREF crBegin,COLORREF crEnd,BYTE byAlpha=0xFF);

		virtual HRESULT TextOut(
			int x,
			int y,
			LPCTSTR lpszString,
			int nCount,
            BYTE byAlpha =0xFF);

        virtual HRESULT DrawIconEx(int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags);
        virtual HRESULT DrawBitmap(LPCRECT pRcDest,IBitmap *pBitmap,int xSrc,int ySrc,BYTE byAlpha=0xFF);
        virtual HRESULT DrawBitmapEx(LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,EXPEND_MODE expendMode, BYTE byAlpha=0xFF);
        virtual HRESULT DrawBitmap9Patch(LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,EXPEND_MODE expendMode,BYTE byAlpha=0xFF);

		virtual IRenderObj * GetCurrentObject(OBJTYPE uType);
        virtual HRESULT SelectObject(IRenderObj *pObj,IRenderObj ** ppOldObj = NULL);


		virtual COLORREF GetTextColor()
		{
			return m_curColor.toCOLORREF();
		}
		
		virtual COLORREF SetTextColor(COLORREF color)
		{
			COLORREF crOld=m_curColor.toCOLORREF();
 			m_curColor.setRGB(color);
			return crOld;
		}
		
        virtual HDC GetDC(UINT uFlag);
        virtual void ReleaseDC(HDC hdc);

    protected:
		SkCanvas *m_SkCanvas;
        SColor            m_curColor;
		CAutoRefPtr<SBitmap_Skia> m_curBmp;
		CAutoRefPtr<SPen_Skia> m_curPen;
		CAutoRefPtr<SBrush_Skia> m_curBrush;
        CAutoRefPtr<SFont_Skia> m_curFont;
    

		HDC m_hBindDC;
		RECT m_rcBind;

        SkPoint         m_ptOrg;
        
        HDC m_hGetDC;
        UINT m_uGetDCFlag;
	};
}