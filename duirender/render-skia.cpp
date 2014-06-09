#include "StdAfx.h"
#include "render-skia.h"

#include <core\SkShader.h>
#include <core\SkDevice.h>
#include <effects\SkDashPathEffect.h>
#include "drawtext-skia.h"

#ifdef _DEBUG
#pragma comment(lib,"../myskia/lib/vc90/skcore_d.lib")
#else
#pragma comment(lib,"../myskia/lib/vc90/skcore.lib")
#endif

#include "img-decoder.h"

namespace SOUI
{
	//PS_SOLID
	const float  ps_solid[] ={1.0f,0.0f};
	const float  ps_dash[] ={5.0f,5.0f};
	const float  ps_dot[] ={1.0f,4.0f};
	const float  ps_dashdot[] ={4.0f,1.0f,1.0f,1.0f};
	const float  ps_dashdotdot[] ={4.0f,1.0f,1.0f,1.0f,1.0f,1.0f};

	const struct LineDashEffect
	{
		const float  *fDash;
		int		nCount;
	}LINEDASHEFFECT[] =
	{
		{ps_solid,ARRAYSIZE(ps_solid)},
		{ps_dash,ARRAYSIZE(ps_dash)},
		{ps_dot,ARRAYSIZE(ps_dot)},
		{ps_dashdot,ARRAYSIZE(ps_dashdot)},
		{ps_dashdotdot,ARRAYSIZE(ps_dashdotdot)},
	};


    SkIRect toSkIRect(LPCRECT pRc)
    {
        SkIRect rc={pRc->left,pRc->top,pRc->right,pRc->bottom};
        return rc;
    }

    SkRect toSkRect(LPCRECT pRc)
    {
        SkIRect rc={pRc->left,pRc->top,pRc->right,pRc->bottom};
        return SkRect::MakeFromIRect(rc);
    }

	class SGetLineDashEffect
	{
	public:
		SGetLineDashEffect(int iStyle):pDashPathEffect(NULL)
		{
			if(iStyle>PS_SOLID && iStyle<=PS_DASHDOTDOT)
			{
				const LineDashEffect *pEff=&LINEDASHEFFECT[iStyle-1];
				pDashPathEffect=SkDashPathEffect::Create(pEff->fDash,pEff->nCount,0.0f);
			}
		}
		~SGetLineDashEffect()
		{
			if(pDashPathEffect) pDashPathEffect->unref();
		}

		SkDashPathEffect * Get() const{return pDashPathEffect;}
	private:
		SkDashPathEffect * pDashPathEffect;
	};
	//////////////////////////////////////////////////////////////////////////
	// SRenderFactory_Skia

	BOOL SRenderFactory_Skia::Init()
	{
		return TRUE;
	}

	BOOL SRenderFactory_Skia::CreateRenderTarget( IRenderTarget ** ppRenderTarget ,int nWid,int nHei)
	{
		*ppRenderTarget = new SRenderTarget_Skia(this, nWid, nHei);
		return TRUE;
	}


	//////////////////////////////////////////////////////////////////////////
	// SRenderTarget_Skia

	SRenderTarget_Skia::SRenderTarget_Skia( IRenderFactory_Skia* pRenderFactory ,int nWid,int nHei)
		:TSkiaRenderObjImpl<IRenderTarget>(pRenderFactory)
		,m_hBindDC(0)
		,m_SkCanvas(NULL)
        ,m_curColor(0xFF000000)//默认黑色
	{
        m_ptOrg.fX=m_ptOrg.fY=0.0f;

        if(nWid && nHei)
        {
            CAutoRefPtr<IBitmap> bmp;
            CreateBitmap(&bmp);
            bmp->Init(this,nWid,nHei);
            SelectObject(bmp);
        }

		CAutoRefPtr<IPen> pPen;
		CreatePen(PS_SOLID,CDuiColor(0,0,0),1,&pPen);
		SelectObject(pPen);

		CAutoRefPtr<IBrush> pBr;
		CreateSolidColorBrush(CDuiColor(0,0,0),&pBr);
		SelectObject(pBr);

        CAutoRefPtr<IFont> pFont;
        LOGFONT lf={0};
        lf.lfHeight=20;
        _tcscpy(lf.lfFaceName,_T("宋体"));
        CreateFont(lf,&pFont);
        SelectObject(pFont);
	}
	
	SRenderTarget_Skia::~SRenderTarget_Skia()
	{
		if(m_SkCanvas) delete m_SkCanvas;
	}

	HRESULT SRenderTarget_Skia::CreateCompatibleRenderTarget( SIZE szTarget,IRenderTarget **ppRenderTarget )
	{
        SRenderTarget_Skia *pRT = new SRenderTarget_Skia(GetRenderFactory_Skia(),szTarget.cx,szTarget.cy);
        *ppRenderTarget = pRT;
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreatePen( int iStyle,COLORREF cr,int cWidth,IPen ** ppPen )
	{
		*ppPen = new SPen_Skia(GetRenderFactory_Skia(),iStyle,cr,cWidth);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateFont( const LOGFONT &lf,IFont ** ppFont )
	{
		*ppFont = new SFont_Skia(GetRenderFactory_Skia(),&lf);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateSolidColorBrush( COLORREF cr,IBrush ** ppBrush )
	{
		*ppBrush = SBrush_Skia::CreateSolidBrush(GetRenderFactory_Skia(),cr);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )
	{
		SBitmap_Skia *pBmpSkia = (SBitmap_Skia*)pBmp;
		*ppBrush = SBrush_Skia::CreateBitmapBrush(GetRenderFactory_Skia(),pBmpSkia->GetBitmap());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateRegion( IRegion ** ppRegion )
	{
		*ppRegion = new SRegion_Skia(GetRenderFactory_Skia());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateBitmap( IBitmap ** ppBitmap )
	{
		*ppBitmap = new SBitmap_Skia(GetRenderFactory_Skia());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::BindDC( HDC hdc,LPCRECT pSubRect )
	{
		m_hBindDC=hdc;
		m_rcBind = *pSubRect;
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::BeginDraw()
	{
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::EndDraw()
	{
		if(m_hBindDC)
		{//copy image to bind dc
			SkBitmap bmp=m_curBmp->GetBitmap();

			BITMAPINFO bmi;
			memset(&bmi, 0, sizeof(bmi));
			bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth       = bmp.width();
			bmi.bmiHeader.biHeight      = -bmp.height(); // top-down image 
			bmi.bmiHeader.biPlanes      = 1;
			bmi.bmiHeader.biBitCount    = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage   = 0;

			bmp.lockPixels();
			SetDIBitsToDevice(m_hBindDC,
				m_rcBind.left,m_rcBind.top,m_rcBind.right-m_rcBind.left,m_rcBind.bottom-m_rcBind.top,
				m_rcBind.left,bmp.height()-m_rcBind.bottom,0,bmp.height(),
				bmp.getPixels(),&bmi,DIB_RGB_COLORS);
			bmp.unlockPixels();
		}
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::Resize( SIZE sz )
	{
		HRESULT hr=S_FALSE;
		if(!m_curBmp)
		{
			CAutoRefPtr<IBitmap> pBmp;
			hr=CreateBitmap(&pBmp);
			if(SUCCEEDED(hr))
			{
				pBmp->Init(this,sz.cx,sz.cy);
				SelectObject(pBmp);
			}
		}else
		{
			m_curBmp->Init(this,sz.cx,sz.cy);
		}
        if(m_SkCanvas) delete m_SkCanvas;
        m_SkCanvas = new SkCanvas(m_curBmp->GetBitmap());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PushClipRect( LPCRECT pRect )
	{
	    m_SkCanvas->save();
        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->clipRect(skrc);
	    return S_OK;
	}

	HRESULT SRenderTarget_Skia::PopClipRect()
	{
	    m_SkCanvas->restore();
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PushClipRegion( IRegion *pRegion )
	{
        SRegion_Skia * rgn_skia=(SRegion_Skia*)pRegion;
        SkRegion rgn=rgn_skia->GetRegion();
        
        m_SkCanvas->save();
        m_SkCanvas->clipRegion(rgn);
        
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PopClipRegion()
	{
        m_SkCanvas->restore();
        return S_OK;
	}

    HRESULT SRenderTarget_Skia::GetClipRegion( IRegion **ppRegion )
    {
        SRegion_Skia *pRgn=new SRegion_Skia(GetRenderFactory_Skia());
        pRgn->SetRegion(m_SkCanvas->getTotalClip());
        *ppRegion = pRgn;
        return S_OK;
    }

	HRESULT SRenderTarget_Skia::BitBlt( LPRECT pRcDest,IRenderTarget *pRTSour,LPRECT pRcSour,UINT uDef )
	{
	    SRenderTarget_Skia *pRTSourSkia=(SRenderTarget_Skia*)pRcSour;
	    
	    const SkBitmap & bmpSrc= pRTSourSkia->m_SkCanvas->getDevice()->accessBitmap(true);
        SkRect rcDst=toSkRect(pRcDest);
        rcDst.offset(m_ptOrg);
	    m_SkCanvas->drawBitmapRectToRect(bmpSrc,&toSkRect(pRcSour),rcDst);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat ,BYTE byAlpha)
	{
		if(cchLen<0) cchLen= _tcslen(pszText);
		CDuiStringW strW=DUI_CT2W(CDuiStringT(pszText,cchLen));
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setColor(m_curColor);
        txtPaint.setTypeface(m_curFont->GetFont());
        txtPaint.setAlpha(byAlpha);
        if(uFormat & DT_CENTER)
            txtPaint.setTextAlign(SkPaint::kCenter_Align);
        else if(uFormat & DT_RIGHT)
            txtPaint.setTextAlign(SkPaint::kRight_Align);

        SkRect skrc=toSkRect(pRc);
        skrc.offset(m_ptOrg);
        DrawText_Skia(m_SkCanvas,strW,strW.GetLength(),skrc,txtPaint,uFormat);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::MeasureText( LPCTSTR pszText,int cchLen, SIZE *psz )
	{
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setTypeface(m_curFont->GetFont());
        CDuiStringW strW=DUI_CT2W(CDuiStringT(pszText,cchLen));
        psz->cx = (int)txtPaint.measureText(strW,strW.GetLength());
        
        SkPaint::FontMetrics metrics;
        txtPaint.getFontMetrics(&metrics);
        psz->cy = (int)(metrics.fBottom-metrics.fTop);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawRectangle( int left, int top,int right,int bottom )
	{
		SkPaint paint;
		paint.setColor(m_curPen->GetColor());
		SGetLineDashEffect skDash(m_curPen->GetStyle());
 		paint.setPathEffect(skDash.Get());
		paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		paint.setStyle(SkPaint::kStroke_Style);

        RECT rc={left,top,right,bottom};
        SkRect skrc=toSkRect(&rc);
        skrc.offset(m_ptOrg);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillRectangle( int left, int top,int right,int bottom )
	{
		SkPaint paint;
		
		if(m_curBrush->IsBitmap())
		{
			paint.setFilterBitmap(true);
			paint.setShader(SkShader::CreateBitmapShader(m_curBrush->GetBitmap(),SkShader::kRepeat_TileMode,SkShader::kRepeat_TileMode))->unref();
		}else
		{
			paint.setFilterBitmap(false);
			paint.setColor(m_curBrush->GetColor());
		}
		paint.setStyle(SkPaint::kFill_Style);

        RECT rc={left,top,right,bottom};
        SkRect skrc=toSkRect(&rc);
        skrc.offset(m_ptOrg);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

    HRESULT SRenderTarget_Skia::DrawRoundRect( LPCRECT pRect,POINT pt )
    {
        SkPaint paint;
        paint.setColor(m_curPen->GetColor());
        SGetLineDashEffect skDash(m_curPen->GetStyle());
        paint.setPathEffect(skDash.Get());
        paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
        paint.setStyle(SkPaint::kStroke_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::FillRoundRect( LPCRECT pRect,POINT pt )
    {
        SkPaint paint;

        if(m_curBrush->IsBitmap())
        {
            paint.setFilterBitmap(true);
            paint.setShader(SkShader::CreateBitmapShader(m_curBrush->GetBitmap(),SkShader::kRepeat_TileMode,SkShader::kRepeat_TileMode))->unref();
        }else
        {
            paint.setFilterBitmap(false);
            paint.setColor(m_curBrush->GetColor());
        }
        paint.setStyle(SkPaint::kFill_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);

        m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::DrawLines(LPPOINT pPt,size_t nCount)
    {
        SkPoint *pts=new SkPoint[nCount];
        for(size_t i=0; i<nCount; i++ )
        {
            pts[i].fX = (SkScalar)pPt[i].x;
            pts[i].fY = (SkScalar)pPt[i].y;
        }
        SkPoint::Offset(pts,nCount,m_ptOrg);

        SkPaint paint;
        paint.setColor(m_curPen->GetColor());
        SGetLineDashEffect skDash(m_curPen->GetStyle());
        paint.setPathEffect(skDash.Get());
        paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
        paint.setStyle(SkPaint::kStroke_Style);
        m_SkCanvas->drawPoints(SkCanvas::kPolygon_PointMode,nCount,pts,paint);
        delete []pts;

        return S_OK;
    }

	HRESULT SRenderTarget_Skia::TextOut( int x, int y, LPCTSTR lpszString, int nCount,BYTE byAlpha )
	{
		if(nCount<0) nCount= _tcslen(lpszString);
		CDuiStringW strW=DUI_CT2W(lpszString,nCount);
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setColor(m_curColor);
        txtPaint.setTypeface(m_curFont->GetFont());
        txtPaint.setAlpha(byAlpha);
		m_SkCanvas->drawText((LPCWSTR)strW,strW.GetLength()*2,(SkScalar)x+m_ptOrg.fX,(SkScalar)y+m_ptOrg.fY,txtPaint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawBitmap( LPRECT pRcDest,IBitmap *pBitmap,LPRECT pRcSour,BYTE byAlpha/*=0xFF*/ )
	{
		SBitmap_Skia *pBmp = (SBitmap_Skia*)pBitmap;
		SkBitmap bmp=pBmp->GetBitmap();

		RECT rcSour={0,0,bmp.width(),bmp.height()};
		if(!pRcSour) pRcSour = &rcSour;

		SkRect rcSrc = toSkRect(pRcSour);
		SkRect rcDest= toSkRect(pRcDest);
        rcDest.offset(m_ptOrg);

		SkPaint paint;
		paint.setAntiAlias(true);
		m_SkCanvas->drawBitmapRectToRect(bmp,&rcSrc,rcDest,&paint);
		return S_OK;
	}

	IRenderObj * SRenderTarget_Skia::GetCurrentObject( OBJTYPE uType )
	{
		IRenderObj *pRet=NULL;
		switch(uType)
		{
		case OT_BITMAP: 
			pRet=m_curBmp;
			break;
		case OT_PEN:
			pRet=m_curPen;
			break;
		case OT_BRUSH:
			pRet=m_curBrush;
			break;
        case OT_FONT:
            pRet=m_curFont;
            break;
		}
		return pRet;
	}

    HRESULT SRenderTarget_Skia::SelectObject( IRenderObj *pObj,IRenderObj ** ppOldObj /*= NULL*/ )
    {
        CAutoRefPtr<IRenderObj> pRet;
        switch(pObj->ObjectType())
        {
        case OT_BITMAP: 
            pRet=m_curBmp;
            m_curBmp=(SBitmap_Skia*)pObj;
            break;
        case OT_PEN:
            pRet=m_curPen;
            m_curPen=(SPen_Skia*)pObj;
            break;
        case OT_BRUSH:
            pRet=m_curBrush;
            m_curBrush=(SBrush_Skia*)pObj;
            break;
        case OT_FONT:
            pRet=m_curFont;
            m_curFont=(SFont_Skia*)pObj;
            break;
        }
        if(pRet && ppOldObj)
        {//由调用者调用Release释放该RenderObj
            pRet->AddRef();
            *ppOldObj = pRet;
        }
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::OffsetViewportOrg( int xOff, int yOff, LPPOINT lpPoint )
    {
        if(lpPoint)
        {
            lpPoint->x = (LONG)m_ptOrg.fX;
            lpPoint->y = (LONG)m_ptOrg.fY;
        }
        m_ptOrg.offset((SkScalar)xOff,(SkScalar)yOff);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::GetViewportOrg( LPPOINT lpPoint )
    {
        if(lpPoint)
        {
            lpPoint->x = (LONG)m_ptOrg.fX;
            lpPoint->y = (LONG)m_ptOrg.fY;
        }
        return S_OK;
    }
    //////////////////////////////////////////////////////////////////////////
	// SBitmap_Skia
	HRESULT SBitmap_Skia::Init( IRenderTarget *pRT,int nWid,int nHei )
	{
		m_bitmap.reset();
		m_bitmap.setConfig(SkBitmap::kARGB_8888_Config,nWid,nHei);
		m_bitmap.allocPixels();
		return S_OK;
	}

	HRESULT SBitmap_Skia::LoadFromFile( IRenderTarget *pRT,LPCTSTR pszFileName,LPCTSTR pszType )
	{
		SImgDecoder imgDecoder;
		if(imgDecoder.DecodeFromFile(DUI_CT2W(pszFileName))==0) return S_FALSE;
		return ImgFromDecoder(imgDecoder);
	}

	HRESULT SBitmap_Skia::LoadFromMemory( IRenderTarget *pRT,LPBYTE pBuf,size_t szLen,LPCTSTR pszType )
	{
		SImgDecoder imgDecoder;
		if(imgDecoder.DecodeFromMemory(pBuf,szLen)==0) return S_FALSE;
        return ImgFromDecoder(imgDecoder);
	}

    HRESULT SBitmap_Skia::ImgFromDecoder(SImgDecoder &imgDecoder)
    {
        IWICBitmapSource* convertedBMP = imgDecoder.GetFrame(0);
        UINT uWid=0,uHei =0;
        convertedBMP->GetSize(&uWid,&uHei);

        m_bitmap.setConfig(SkBitmap::kARGB_8888_Config, uWid, uHei);
        m_bitmap.allocPixels();

        m_bitmap.lockPixels();
        const int stride = m_bitmap.rowBytes();
        convertedBMP->CopyPixels(NULL, stride, stride * uHei,
            reinterpret_cast<BYTE*>(m_bitmap.getPixels()));
        m_bitmap.unlockPixels();
        return S_OK;
    }

	//////////////////////////////////////////////////////////////////////////
	SRegion_Skia::SRegion_Skia( IRenderFactory_Skia *pRenderFac )
        :TSkiaRenderObjImpl<IRegion>(pRenderFac)
	{

	}

	void SRegion_Skia::CombineRect( LPCRECT lprect,int nCombineMode )
	{
        m_rgn.op(toSkIRect(lprect),SkRegion::kUnion_Op);
	}

	BOOL SRegion_Skia::PtInRegion( POINT pt )
	{
        return m_rgn.contains(pt.x,pt.y);
	}

	BOOL SRegion_Skia::RectInRegion( LPCRECT lprect )
	{
        DUIASSERT(lprect);
        SkIRect rc={lprect->left,lprect->top,lprect->right,lprect->bottom};
        return m_rgn.contains(rc);
	}

	void SRegion_Skia::GetRgnBox( LPRECT lprect )
	{
        DUIASSERT(lprect);
        SkIRect rc=m_rgn.getBounds();
        lprect->left=rc.left();
        lprect->top=rc.top();
        lprect->right=rc.right();
        lprect->bottom=rc.bottom();
	}

	BOOL SRegion_Skia::IsEmpty()
	{
        return m_rgn.isEmpty();
	}

    void SRegion_Skia::Offset( POINT pt )
    {
        m_rgn.translate(pt.x,pt.y);
    }
}