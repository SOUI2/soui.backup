#include "StdAfx.h"

#include <core\SkShader.h>
#include <core\SkDevice.h>
#include <effects\SkDashPathEffect.h>
#include <effects\SkGradientShader.h>
#include <gdialpha.h>

#include "drawtext-skia.h"

#include "render-skia.h"
#include "Render-Skia2.h"
#include "trace.h"

#include "skia2rop2.h"

#define getTotalClip internal_private_getTotalClip
// #include <vld.h>

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
    
    void InflateSkRect(SkRect *pRect,SkScalar dx,SkScalar dy)
    {
        pRect->fLeft -= dx;
        pRect->fRight += dx;
        pRect->fTop -= dy;
        pRect->fBottom += dy;
    }
    
    int RectWid(LPCRECT pRc){return pRc->right-pRc->left;}
    int RectHei(LPCRECT pRc){return pRc->bottom-pRc->top;}
    
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

	BOOL SRenderFactory_Skia::CreateRenderTarget( IRenderTarget ** ppRenderTarget ,int nWid,int nHei)
	{
		*ppRenderTarget = new SRenderTarget_Skia(this, nWid, nHei);
		return TRUE;
	}

    BOOL SRenderFactory_Skia::CreateFont( IFont ** ppFont , const LOGFONT &lf )
    {
        *ppFont = new SFont_Skia(this,&lf);
        return TRUE;
    }

    BOOL SRenderFactory_Skia::CreateBitmap( IBitmap ** ppBitmap )
    {
        *ppBitmap = new SBitmap_Skia(this);
        return TRUE;
    }

    BOOL SRenderFactory_Skia::CreateRegion( IRegion **ppRgn )
    {
        *ppRgn = new SRegion_Skia(this);
        return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
	// SRenderTarget_Skia

	SRenderTarget_Skia::SRenderTarget_Skia( IRenderFactory* pRenderFactory ,int nWid,int nHei)
		:TSkiaRenderObjImpl<IRenderTarget>(pRenderFactory)
		,m_SkCanvas(NULL)
        ,m_curColor(0xFF000000)//默认黑色
        ,m_hGetDC(0)
        ,m_uGetDCFlag(0)
	{
        m_ptOrg.fX=m_ptOrg.fY=0.0f;
        
        m_SkCanvas = new SkCanvas();

        CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&m_defPen);
        SelectObject(m_defPen);

        CreateSolidColorBrush(SColor(0,0,0).toCOLORREF(),&m_defBrush);
        SelectObject(m_defBrush);

        LOGFONT lf={0};
        lf.lfHeight=20;
        _tcscpy(lf.lfFaceName,_T("宋体"));
        pRenderFactory->CreateFont(&m_defFont,lf);
        SelectObject(m_defFont);

        GetRenderFactory_Skia()->CreateBitmap(&m_defBmp);
        m_defBmp->Init(nWid,nHei);
        SelectObject(m_defBmp);
		CAutoRefPtr<IPen> pPen;
		CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&pPen);
		SelectObject(pPen);
        
//        m_SkCanvas = new SkCanvas(m_curBmp->GetSkBitmap());
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

	HRESULT SRenderTarget_Skia::CreateSolidColorBrush( COLORREF cr,IBrush ** ppBrush )
	{
		*ppBrush = SBrush_Skia::CreateSolidBrush(GetRenderFactory_Skia(),cr);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )
	{
		SBitmap_Skia *pBmpSkia = (SBitmap_Skia*)pBmp;
		*ppBrush = SBrush_Skia::CreateBitmapBrush(GetRenderFactory_Skia(),pBmpSkia->GetSkBitmap());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::Resize( SIZE sz )
	{
    	m_curBmp->Init(sz.cx,sz.cy);
        delete m_SkCanvas;
        m_SkCanvas = new SkCanvas(m_curBmp->GetSkBitmap());
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::PushClipRect( LPCRECT pRect ,UINT mode/*=RGN_AND*/)
	{
        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        
        m_SkCanvas->save();

        m_SkCanvas->clipRect(skrc,SRegion_Skia::RGNMODE2SkRgnOP(mode));
	    return S_OK;
	}

	HRESULT SRenderTarget_Skia::PushClipRegion( IRegion *pRegion ,UINT mode/*=RGN_AND*/)
	{
        SRegion_Skia * rgn_skia=(SRegion_Skia*)pRegion;
        SkRegion rgn=rgn_skia->GetRegion();
        rgn.translate((int)m_ptOrg.fX,(int)m_ptOrg.fY);

        m_SkCanvas->save();

        m_SkCanvas->clipRegion(rgn,SRegion_Skia::RGNMODE2SkRgnOP(mode));
        
		return S_OK;
	}

    HRESULT SRenderTarget_Skia::PopClip()
    {
        m_SkCanvas->restore();
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::ExcludeClipRect( LPCRECT pRc )
    {
        SkRect skrc=toSkRect(pRc);
        skrc.offset(m_ptOrg);
        m_SkCanvas->clipRect(skrc,SkRegion::kDifference_Op);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::IntersectClipRect( LPCRECT pRc )
    {
        SkRect skrc=toSkRect(pRc);
        skrc.offset(m_ptOrg);
        m_SkCanvas->clipRect(skrc,SkRegion::kIntersect_Op);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::SaveClip( int *pnState )
    {
        int nState=m_SkCanvas->save();
        if(pnState) *pnState=nState;
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::RestoreClip( int nState/*=-1*/ )
    {
        m_SkCanvas->restoreToCount(nState);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::GetClipRegion( IRegion **ppRegion )
    {
        SRegion_Skia *pRgn=new SRegion_Skia(GetRenderFactory_Skia());
        SkRegion rgn = m_SkCanvas->getTotalClip();
        //需要将rect的viewOrg还原
        rgn.translate((int)-m_ptOrg.fX,(int)-m_ptOrg.fY);
        pRgn->SetRegion(rgn);
        *ppRegion = pRgn;
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::GetClipBox(LPRECT prc)
    {
        SkRect skrc;
        m_SkCanvas->getClipBounds(&skrc);
        //需要将rect的viewOrg还原
        skrc.offset(-m_ptOrg);

        prc->left=(LONG)skrc.fLeft;
        prc->top=(LONG)skrc.fTop;
        prc->right=(LONG)skrc.fRight;
        prc->bottom=(LONG)skrc.fBottom;
        //需要4周缩小一个单位才是和GDI相同的剪裁区
        ::InflateRect(prc,-1,-1);
        return S_OK;
    }
    
	HRESULT SRenderTarget_Skia::BitBlt( LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=SRCCOPY*/)
	{
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        dwRop = dwRop & 0x7fffffff;
        switch(dwRop)
        {
        case SRCCOPY:
            paint.setXfermodeMode(SkXfermode::kSrc_Mode);
            break;
        case DSTINVERT:
            paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_Invert));
            break;
        case SRCINVERT:
            paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_Xor));
            break;
        case SRCAND:
            paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_And));
            break;
        default:
            SASSERT(FALSE);
            break;
        }

        SRenderTarget_Skia *pRtSourSkia=(SRenderTarget_Skia*)pRTSour;
        SkBitmap    bmpSrc=pRtSourSkia->m_curBmp->GetSkBitmap();
        POINT ptSourViewport;
        pRtSourSkia->GetViewportOrg(&ptSourViewport);
        xSrc += ptSourViewport.x;
        ySrc += ptSourViewport.y;
        
        
        SkIRect isrc={xSrc,ySrc,xSrc + pRcDest->right-pRcDest->left,ySrc+pRcDest->bottom-pRcDest->top};
        SkRect skrc=toSkRect(pRcDest);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawBitmapRect(bmpSrc,&isrc,skrc,&paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat)
	{
		if(cchLen<0) cchLen= _tcslen(pszText);
		if(cchLen==0)
        {
            if(uFormat & DT_CALCRECT)
            {
                pRc->right=pRc->left;
                pRc->bottom=pRc->top;
            }
            return S_OK;
        }
		
		SStringW strW=S_CT2W(SStringT(pszText,cchLen));
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setColor(m_curColor.toARGB());
        txtPaint.setTypeface(m_curFont->GetFont());
        if(uFormat & DT_CENTER)
            txtPaint.setTextAlign(SkPaint::kCenter_Align);
        else if(uFormat & DT_RIGHT)
            txtPaint.setTextAlign(SkPaint::kRight_Align);

        SkRect skrc=toSkRect(pRc);
        skrc.offset(m_ptOrg);
        skrc=DrawText_Skia(m_SkCanvas,strW,strW.GetLength(),skrc,txtPaint,uFormat);
        if(uFormat & DT_CALCRECT)
        {
            pRc->left=(int)skrc.fLeft;
            pRc->top=(int)skrc.fTop;
            pRc->right=(int)skrc.fRight;
            pRc->bottom=(int)skrc.fBottom;
        }
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::MeasureText( LPCTSTR pszText,int cchLen, SIZE *psz )
	{
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setTypeface(m_curFont->GetFont());
        SStringW strW=S_CT2W(SStringT(pszText,cchLen));
        psz->cx = (int)txtPaint.measureText(strW,strW.GetLength()*sizeof(wchar_t));
        
        SkPaint::FontMetrics metrics;
        txtPaint.getFontMetrics(&metrics);
        psz->cy = (int)(metrics.fBottom-metrics.fTop);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawRectangle(LPCRECT pRect)
	{
		SkPaint paint;
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SGetLineDashEffect skDash(m_curPen->GetStyle());
 		paint.setPathEffect(skDash.Get());
		paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
		paint.setStyle(SkPaint::kStroke_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        InflateSkRect(&skrc,-0.5f,-0.5f);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillRectangle(LPCRECT pRect)
	{
		SkPaint paint;
		
		if(m_curBrush->IsBitmap())
		{
			paint.setFilterBitmap(true);
			paint.setShader(SkShader::CreateBitmapShader(m_curBrush->GetBitmap(),SkShader::kRepeat_TileMode,SkShader::kRepeat_TileMode))->unref();
		}else
		{
			paint.setFilterBitmap(false);
			paint.setColor(SColor(m_curBrush->GetColor()).toARGB());
		}
		paint.setStyle(SkPaint::kFill_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        InflateSkRect(&skrc,-0.5f,-0.5f);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

    HRESULT SRenderTarget_Skia::DrawRoundRect( LPCRECT pRect,POINT pt )
    {
        SkPaint paint;
        paint.setColor(SColor(m_curPen->GetColor()).toARGB());
        SGetLineDashEffect skDash(m_curPen->GetStyle());
        paint.setPathEffect(skDash.Get());
        paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);

        SkRect skrc=toSkRect(pRect);
        InflateSkRect(&skrc,-0.5f,-0.5f);//要缩小0.5显示效果才和GDI一致。
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::FillRoundRect( LPCRECT pRect,POINT pt )
    {
        SkPaint paint;
        paint.setAntiAlias(true);

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
        InflateSkRect(&skrc,-0.5f,-0.5f);//要缩小0.5显示效果才和GDI一致。
        skrc.offset(m_ptOrg);

        m_SkCanvas->drawRoundRect(skrc,(SkScalar)pt.x,(SkScalar)pt.y,paint);
        return S_OK;
    }
    
    HRESULT SRenderTarget_Skia::FillSolidRoundRect(LPCRECT pRect,POINT pt,COLORREF cr)
    {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setFilterBitmap(false);
        paint.setColor(SColor(cr).toARGB());
        paint.setStyle(SkPaint::kFill_Style);

        SkRect skrc=toSkRect(pRect);
        InflateSkRect(&skrc,-0.5f,-0.5f);//要缩小0.5显示效果才和GDI一致。
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
        paint.setAntiAlias(true);
        paint.setColor(SColor(m_curPen->GetColor()).toARGB());
        SGetLineDashEffect skDash(m_curPen->GetStyle());
        paint.setPathEffect(skDash.Get());
        paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
        paint.setStyle(SkPaint::kStroke_Style);
        m_SkCanvas->drawPoints(SkCanvas::kPolygon_PointMode,nCount,pts,paint);
        delete []pts;

        return S_OK;
    }

	HRESULT SRenderTarget_Skia::TextOut( int x, int y, LPCTSTR lpszString, int nCount)
	{
		if(nCount<0) nCount= _tcslen(lpszString);
		SStringW strW=S_CT2W(SStringT(lpszString,nCount));
        SkPaint     txtPaint = m_curFont->GetPaint();
        SkPaint::FontMetrics metrics;
        txtPaint.getFontMetrics(&metrics);
        SkScalar fx = m_ptOrg.fX + x;
        SkScalar fy = m_ptOrg.fY + y;
        fy -= metrics.fAscent;

        txtPaint.setColor(m_curColor.toARGB());
        txtPaint.setTypeface(m_curFont->GetFont());
		m_SkCanvas->drawText((LPCWSTR)strW,strW.GetLength()*2,fx,fy,txtPaint);
		return S_OK;
	}

    HRESULT SRenderTarget_Skia::DrawIconEx( int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags )
    {
        HDC hdc=GetDC(0);
        BOOL bRet=::DrawIconEx(hdc,xLeft,yTop,hIcon,cxWidth,cyWidth,0,NULL,diFlags);
        ReleaseDC(hdc);
        return bRet?S_OK:S_FALSE;
    }

    HRESULT SRenderTarget_Skia::DrawBitmap(LPCRECT pRcDest,IBitmap *pBitmap,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/ )
    {
        SBitmap_Skia *pBmp = (SBitmap_Skia*)pBitmap;
        SkBitmap bmp=pBmp->GetSkBitmap();

        RECT rcSrc={xSrc,ySrc,xSrc+pRcDest->right-pRcDest->left,ySrc+pRcDest->bottom-pRcDest->top};

        SkRect skrcDst = toSkRect(pRcDest);
        SkRect skrcSrc= toSkRect(&rcSrc);
        skrcDst.offset(m_ptOrg);

        SkPaint paint;
        paint.setAntiAlias(true);
        
        if(byAlpha != 0xFF) paint.setAlpha(byAlpha);
        m_SkCanvas->drawBitmapRectToRect(bmp,&skrcSrc,skrcDst,&paint);
        return S_OK;
    }
    
    HRESULT SRenderTarget_Skia::AlphaBlend( LPCRECT pRcDest,IRenderTarget *pRTSrc,LPCRECT pRcSrc,BYTE byAlpha )
    {
        IBitmap *pBmp=(IBitmap*) pRTSrc->GetCurrentObject(OT_BITMAP);
        if(!pBmp) return S_FALSE;
        RECT rcSrc = *pRcSrc;
        POINT ptSrcOrg;
        pRTSrc->GetViewportOrg(&ptSrcOrg);
        OffsetRect(&rcSrc,ptSrcOrg.x,ptSrcOrg.y);
        return DrawBitmapEx(pRcDest,pBmp,&rcSrc,EM_STRETCH,byAlpha);
    }

    HRESULT SRenderTarget_Skia::DrawBitmapEx( LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,UINT expendMode, BYTE byAlpha/*=0xFF*/ )
    {
        UINT expendModeLow = LOWORD(expendMode);

        if(expendModeLow == EM_NULL || (RectWid(pRcDest)==RectWid(pRcSrc) && RectHei(pRcDest)==RectHei(pRcSrc)))
            return DrawBitmap(pRcDest,pBitmap,pRcSrc->left,pRcSrc->top,byAlpha);
            
        SBitmap_Skia *pBmp = (SBitmap_Skia*)pBitmap;
        SkBitmap bmp=pBmp->GetSkBitmap();

        RECT rcSour={0,0,bmp.width(),bmp.height()};
        if(!pRcSrc) pRcSrc = &rcSour;
        SkRect rcSrc = toSkRect(pRcSrc);
        SkRect rcDest= toSkRect(pRcDest);
        rcDest.offset(m_ptOrg);

        SkPaint paint;
        paint.setAntiAlias(true);
        if(byAlpha != 0xFF) paint.setAlpha(byAlpha);
        
        SkPaint::FilterLevel fl = SkPaint::kNone_FilterLevel;
        if(HIWORD(expendMode)!=0) fl=SkPaint::kHigh_FilterLevel;
        //skia 中实现的kLow_FilterLevel, kMedium_FilterLevel有问题，自动变为kHigh_FilterLevel
        paint.setFilterLevel(fl);
                
        if(expendModeLow == EM_STRETCH)
        {
            m_SkCanvas->drawBitmapRectToRect(bmp,&rcSrc,rcDest,&paint);
        }else
        {
            PushClipRect(pRcDest,RGN_AND);
            
            SkIRect rcSrc = toSkIRect(pRcSrc);
            SkRect rcSubDest={0.0f,0.0f,(float)rcSrc.width(),(float)rcSrc.height()};
            for(float y=rcDest.fTop;y<rcDest.fBottom;y+=rcSrc.height())
            {
                rcSubDest.offsetTo(rcDest.fLeft,y);               
                for(float x=rcDest.fLeft;x<rcDest.fRight;x += rcSrc.width())
                {
                    m_SkCanvas->drawBitmapRect(bmp,&rcSrc,rcSubDest,&paint);
                    rcSubDest.offset((float)rcSrc.width(),0.0f);
                }
            }
            
            PopClip();
        }
        return S_OK;

    }


    HRESULT SRenderTarget_Skia::DrawBitmap9Patch( LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,UINT expendMode,BYTE byAlpha/*=0xFF*/ )
    {
        int xDest[4] = {pRcDest->left,pRcDest->left+pRcSourMargin->left,pRcDest->right-pRcSourMargin->right,pRcDest->right};
        int xSrc[4] = {pRcSrc->left,pRcSrc->left+pRcSourMargin->left,pRcSrc->right-pRcSourMargin->right,pRcSrc->right};
        int yDest[4] = {pRcDest->top,pRcDest->top+pRcSourMargin->top,pRcDest->bottom-pRcSourMargin->bottom,pRcDest->bottom};
        int ySrc[4] = {pRcSrc->top,pRcSrc->top+pRcSourMargin->top,pRcSrc->bottom-pRcSourMargin->bottom,pRcSrc->bottom};
        
        //首先保证九宫分割正常
        if(!(xSrc[0] <= xSrc[1] && xSrc[1] <= xSrc[2] && xSrc[2] <= xSrc[3])) return S_FALSE;
        if(!(ySrc[0] <= ySrc[1] && ySrc[1] <= ySrc[2] && ySrc[2] <= ySrc[3])) return S_FALSE;
        
        //调整目标位置
        int nDestWid=pRcDest->right-pRcDest->left;
        int nDestHei=pRcDest->bottom-pRcDest->top;
        
        if((pRcSourMargin->left + pRcSourMargin->right) > nDestWid)
        {//边缘宽度大于目标宽度的处理
            if(pRcSourMargin->left >= nDestWid)
            {//只绘制左边部分
                xSrc[1] = xSrc[2] = xSrc[3] = xSrc[0]+nDestWid;
                xDest[1] = xDest[2] = xDest[3] = xDest[0]+nDestWid;
            }else if(pRcSourMargin->right >= nDestWid)
            {//只绘制右边部分
                xSrc[0] = xSrc[1] = xSrc[2] = xSrc[3]-nDestWid;
                xDest[0] = xDest[1] = xDest[2] = xDest[3]-nDestWid;
            }else
            {//先绘制左边部分，剩余的用右边填充
                int nRemain=xDest[3]-xDest[1];
                xSrc[2] = xSrc[3]-nRemain;
                xDest[2] = xDest[3]-nRemain;
            }
        }
        
        if(pRcSourMargin->top + pRcSourMargin->bottom > nDestHei)
        {
            if(pRcSourMargin->top >= nDestHei)
            {//只绘制上边部分
                ySrc[1] = ySrc[2] = ySrc[3] = ySrc[0]+nDestHei;
                yDest[1] = yDest[2] = yDest[3] = yDest[0]+nDestHei;
            }else if(pRcSourMargin->bottom >= nDestHei)
            {//只绘制下边部分
                ySrc[0] = ySrc[1] = ySrc[2] = ySrc[3]-nDestHei;
                yDest[0] = yDest[1] = yDest[2] = yDest[3]-nDestHei;
            }else
            {//先绘制左边部分，剩余的用右边填充
                int nRemain=yDest[3]-yDest[1];
                ySrc[2] = ySrc[3]-nRemain;
                yDest[2] = yDest[3]-nRemain;
            }
        }
        
        //定义绘制模式
        UINT mode[3][3]={
        {EM_NULL,expendMode,EM_NULL},
        {expendMode,expendMode,expendMode},
        {EM_NULL,expendMode,EM_NULL}
        };
        
        for(int y=0;y<3;y++)
        {
            if(ySrc[y] == ySrc[y+1]) continue;
            for(int x=0;x<3;x++)
            {
                if(xSrc[x] == xSrc[x+1]) continue;
                RECT rcSrc = {xSrc[x],ySrc[y],xSrc[x+1],ySrc[y+1]};
                RECT rcDest ={xDest[x],yDest[y],xDest[x+1],yDest[y+1]};
                DrawBitmapEx(&rcDest,pBitmap,&rcSrc,mode[y][x],byAlpha);
            }
        }
        
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


    HRESULT SRenderTarget_Skia::SelectDefaultObject(OBJTYPE objType,IRenderObj ** ppOldObj /*= NULL*/)
    {
        IRenderObj *pDefObj = NULL;
        switch(objType)
        {
        case OT_BITMAP: pDefObj = m_defBmp;break;
        case OT_PEN: pDefObj = m_defPen;break;
        case OT_BRUSH: pDefObj = m_defBrush;break;
        case OT_FONT: pDefObj = m_defFont;break;
        default:return E_INVALIDARG;
        }
        if(pDefObj == GetCurrentObject(objType)) 
            return S_FALSE;
        return SelectObject(pDefObj,ppOldObj);
    }

    HRESULT SRenderTarget_Skia::SelectObject( IRenderObj *pObj,IRenderObj ** ppOldObj /*= NULL*/ )
    {
        CAutoRefPtr<IRenderObj> pRet;
        switch(pObj->ObjectType())
        {
        case OT_BITMAP: 
            pRet=m_curBmp;
            m_curBmp=(SBitmap_Skia*)pObj;
            //重新生成clip
            SASSERT(m_SkCanvas);
            delete m_SkCanvas;
            m_SkCanvas = new SkCanvas(m_curBmp->GetSkBitmap());
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
    
    HRESULT SRenderTarget_Skia::SetViewportOrg( POINT pt )
    {
        m_ptOrg.fX = SkIntToScalar(pt.x);
        m_ptOrg.fY = SkIntToScalar(pt.y);
        return S_OK;
    }


    HDC SRenderTarget_Skia::GetDC( UINT uFlag )
    {
        if(m_hGetDC) return m_hGetDC;
        
        HBITMAP bmp=m_curBmp->GetGdiBitmap();//bmp可能为NULL
        HDC hdc_desk = ::GetDC(NULL);
        m_hGetDC = CreateCompatibleDC(hdc_desk);
        ::ReleaseDC(NULL,hdc_desk);
        
        ::SelectObject(m_hGetDC,bmp);
        
        if(m_SkCanvas->isClipEmpty())
        {
            ::IntersectClipRect(m_hGetDC,0,0,0,0);
        }else if(m_SkCanvas->isClipRect())
        {
            SkRect rcClip;
            m_SkCanvas->getClipBounds(&rcClip);
            RECT rc = {(int)rcClip.left(),(int)rcClip.top(),(int)rcClip.right(),(int)rcClip.bottom()};
            ::InflateRect(&rc,-1,-1);//注意需要向内缩小一个象素
            ::IntersectClipRect(m_hGetDC,rc.left,rc.top,rc.right,rc.bottom);
        }else
        {
            SkRegion rgn = m_SkCanvas->getTotalClip();
            SkRegion::Iterator it(rgn);
            int nCount=0;
            for(;!it.done();it.next())
            {
                nCount++;
            }
            it.rewind();

            int nSize=sizeof(RGNDATAHEADER)+nCount*sizeof(RECT);
            RGNDATA *rgnData=(RGNDATA*)malloc(nSize);
            memset(rgnData,0,nSize);
            rgnData->rdh.dwSize= sizeof(RGNDATAHEADER);
            rgnData->rdh.iType = RDH_RECTANGLES;
            rgnData->rdh.nCount=nCount;
            rgnData->rdh.rcBound.right=m_curBmp->Width();
            rgnData->rdh.rcBound.bottom=m_curBmp->Height();

            nCount=0;
            LPRECT pRc=(LPRECT)rgnData->Buffer;
            for(;!it.done();it.next())
            {
                SkIRect skrc=it.rect();
                RECT rc = {skrc.fLeft,skrc.fTop,skrc.fRight,skrc.fBottom};
                pRc[nCount++]= rc;
            }

            HRGN hRgn=ExtCreateRegion(NULL,nSize,rgnData);
            free(rgnData);
            ::SelectClipRgn(m_hGetDC,hRgn);
            DeleteObject(hRgn);
        }

        ::SetViewportOrgEx(m_hGetDC,(int)m_ptOrg.fX,(int)m_ptOrg.fY,NULL);

        m_uGetDCFlag = uFlag;
        return m_hGetDC;
    }

    void SRenderTarget_Skia::ReleaseDC( HDC hdc )
    {
        if(hdc == m_hGetDC)
        {
            DeleteDC(hdc);
            m_hGetDC = 0;
            m_uGetDCFlag =0;
        }
    }
    
    HRESULT SRenderTarget_Skia::GradientFillEx( LPCRECT pRect,const POINT* pts,COLORREF *colors,float *pos,int nCount,BYTE byAlpha/*=0xFF*/ )
    {
        SkRect skrc = toSkRect(pRect);
        skrc.offset(m_ptOrg);
        SkPoint *skPts = new SkPoint[nCount];
        SkColor *skColors= new SkColor[nCount];
        for(int i=0;i<nCount;i++)
        {
            skPts[i].iset(pts[i].x,pts[i].y);
            skPts[i].offset(m_ptOrg.x(),m_ptOrg.y());
            skColors[i] = SColor(colors[i],byAlpha).toARGB();
        }
        
        SkShader *pShader = SkGradientShader::CreateLinear(skPts, skColors, pos,nCount,SkShader::kMirror_TileMode);
        SkPaint paint;
        paint.setShader(pShader);
        pShader->unref();

        m_SkCanvas->drawRect(skrc,paint);

        delete []skColors;
        delete []skPts;
        return S_OK;
    }
    
    HRESULT SRenderTarget_Skia::GradientFill( LPCRECT pRect,BOOL bVert,COLORREF crBegin,COLORREF crEnd,BYTE byAlpha/*=0xFF*/ )
    {
        SkRect skrc = toSkRect(pRect);
        skrc.offset(m_ptOrg);

        SkPoint pts[2];
        pts[0].set(skrc.left(),skrc.top());

        if (bVert)
        {
            pts[1].set(skrc.left(),skrc.bottom());
        }
        else
        {
            pts[1].set(skrc.right(),skrc.top());
        }

        SColor cr1(crBegin,byAlpha);
        SColor cr2(crEnd,byAlpha);

        const SkColor colors[2] = {cr1.toARGB(),cr2.toARGB()};
        SkShader *pShader = SkGradientShader::CreateLinear(pts, colors, NULL,2,SkShader::kMirror_TileMode);
        SkPaint paint;
        paint.setShader(pShader);
        pShader->unref();

        m_SkCanvas->drawRect(skrc,paint);
        return S_OK;

    }

    HRESULT SRenderTarget_Skia::FillSolidRect( LPCRECT pRect,COLORREF cr )
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SColor(cr).toARGB());
        paint.setXfermodeMode(SkXfermode::kSrcOver_Mode);
        
        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawRect(skrc,paint);
        return S_OK;    
    }

    HRESULT SRenderTarget_Skia::ClearRect( LPCRECT pRect,COLORREF cr )
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SColor(cr).toARGB());
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawRect(skrc,paint);
        return S_OK;    
    }

    HRESULT SRenderTarget_Skia::InvertRect(LPCRECT pRect)
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setXfermode(new ProcXfermode(ProcXfermode::Rop2_Invert));
        SkRect skrc = toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawRect(skrc,paint);
        return S_OK;  
    }

    HRESULT SRenderTarget_Skia::DrawEllipse( LPCRECT pRect )
    {
        SkPaint paint;
        paint.setColor(SColor(m_curPen->GetColor()).toARGB());
        SGetLineDashEffect skDash(m_curPen->GetStyle());
        paint.setPathEffect(skDash.Get());
        paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
        paint.setStyle(SkPaint::kStroke_Style);

        SkRect skrc = toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawOval(skrc,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::FillEllipse( LPCRECT pRect )
    {
        SkPaint paint;
        if(m_curBrush->IsBitmap())
        {
            paint.setFilterBitmap(true);
            paint.setShader(SkShader::CreateBitmapShader(m_curBrush->GetBitmap(),SkShader::kRepeat_TileMode,SkShader::kRepeat_TileMode))->unref();
        }else
        {
            paint.setFilterBitmap(false);
            paint.setColor(SColor(m_curBrush->GetColor()).toARGB());
        }
        paint.setStyle(SkPaint::kFill_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawOval(skrc,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::FillSolidEllipse(LPCRECT pRect,COLORREF cr)
    {
        SkPaint paint;
        paint.setFilterBitmap(false);
        paint.setColor(SColor(cr).toARGB());
        paint.setStyle(SkPaint::kFill_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawOval(skrc,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::DrawArc( LPCRECT pRect,float startAngle,float sweepAngle,bool useCenter )
    {
        SkPaint paint;
        paint.setColor(SColor(m_curPen->GetColor()).toARGB());
        SGetLineDashEffect skDash(m_curPen->GetStyle());
        paint.setPathEffect(skDash.Get());
        paint.setStrokeWidth((SkScalar)m_curPen->GetWidth()-0.5f);
        paint.setStyle(SkPaint::kStroke_Style);

        SkRect skrc = toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawArc(skrc,startAngle,sweepAngle,useCenter,paint);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::FillArc( LPCRECT pRect,float startAngle,float sweepAngle )
    {
        SkPaint paint;
        if(m_curBrush->IsBitmap())
        {
            paint.setFilterBitmap(true);
            paint.setShader(SkShader::CreateBitmapShader(m_curBrush->GetBitmap(),SkShader::kRepeat_TileMode,SkShader::kRepeat_TileMode))->unref();
        }else
        {
            paint.setFilterBitmap(false);
            paint.setColor(SColor(m_curBrush->GetColor()).toARGB());
        }
        paint.setStyle(SkPaint::kFill_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawArc(skrc,startAngle, sweepAngle,true,paint);
        return S_OK;

    }

    HRESULT SRenderTarget_Skia::QueryInterface( REFGUID iid,IObjRef ** ppObj )
    {
        if(iid == __uuidof(IRenderTarget_Skia2))
        {
            *ppObj = new RenderTarget_Skia2;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    //////////////////////////////////////////////////////////////////////////
	// SBitmap_Skia
    static int s_cBmp = 0;
    SBitmap_Skia::SBitmap_Skia( IRenderFactory *pRenderFac ) :TSkiaRenderObjImpl<IBitmap>(pRenderFac),m_hBmp(0)
    {
//         STRACE(L"bitmap new; objects = %d",++s_cBmp);
    }

    SBitmap_Skia::~SBitmap_Skia()
    {
        m_bitmap.reset();
        if(m_hBmp) DeleteObject(m_hBmp);
//         STRACE(L"bitmap delete objects = %d",--s_cBmp);
    }

    HBITMAP SBitmap_Skia::CreateGDIBitmap( int nWid,int nHei,void ** ppBits )
    {
        BITMAPINFO bmi;
        memset(&bmi, 0, sizeof(bmi));
        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = nWid;
        bmi.bmiHeader.biHeight      = -nHei; // top-down image 
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage   = 0;

        HDC hdc=GetDC(NULL);
        LPVOID pBits=NULL;
        HBITMAP hBmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,ppBits,0,0);
        ReleaseDC(NULL,hdc);
        return hBmp;
    }

	HRESULT SBitmap_Skia::Init( int nWid,int nHei ,const LPVOID pBits/*=NULL*/)
	{
		m_bitmap.reset();
		m_bitmap.setInfo(SkImageInfo::Make(nWid,nHei,kN32_SkColorType,kPremul_SkAlphaType));
        if(m_hBmp) DeleteObject(m_hBmp);
    		
		LPVOID pBmpBits=NULL;
		m_hBmp=CreateGDIBitmap(nWid,nHei,&pBmpBits);
		if(!m_hBmp) return E_OUTOFMEMORY;
        if(pBits)
        {
            memcpy(pBmpBits,pBits,nWid*nHei*4);
        }else
        {
            memset(pBmpBits,0,nWid*nHei*4);
        }
		m_bitmap.setPixels(pBmpBits);
		return S_OK;
	}

    HRESULT SBitmap_Skia::Init( IImgFrame *pFrame )
    {
        UINT uWid=0,uHei =0;
        pFrame->GetSize(&uWid,&uHei);

        if(m_hBmp) DeleteObject(m_hBmp);
        m_bitmap.reset();
        m_bitmap.setInfo(SkImageInfo::Make(uWid, uHei,kN32_SkColorType,kPremul_SkAlphaType));
        void * pBits=NULL;
        m_hBmp = CreateGDIBitmap(uWid,uHei,&pBits);

        if(!m_hBmp) return E_OUTOFMEMORY;
        m_bitmap.setPixels(pBits);

        const int stride = m_bitmap.rowBytes();
        pFrame->CopyPixels(NULL, stride, stride * uHei,
            reinterpret_cast<BYTE*>(m_bitmap.getPixels()));
        return S_OK;
    }

	HRESULT SBitmap_Skia::LoadFromFile( LPCTSTR pszFileName)
	{
	    CAutoRefPtr<IImgX> imgDecoder;
	    GetRenderFactory_Skia()->GetImgDecoderFactory()->CreateImgX(&imgDecoder);
		if(imgDecoder->LoadFromFile(S_CT2W(pszFileName))==0) return S_FALSE;
		return ImgFromDecoder(imgDecoder);
	}

	HRESULT SBitmap_Skia::LoadFromMemory(LPBYTE pBuf,size_t szLen)
	{
        CAutoRefPtr<IImgX> imgDecoder;
        GetRenderFactory_Skia()->GetImgDecoderFactory()->CreateImgX(&imgDecoder);
		if(imgDecoder->LoadFromMemory(pBuf,szLen)==0) return S_FALSE;
        return ImgFromDecoder(imgDecoder);
	}

    HRESULT SBitmap_Skia::ImgFromDecoder(IImgX *imgDecoder)
    {
        IImgFrame *pFrame=imgDecoder->GetFrame(0);
        UINT uWid=0,uHei =0;
        pFrame->GetSize(&uWid,&uHei);

        if(m_hBmp) DeleteObject(m_hBmp);
        m_bitmap.reset();
        m_bitmap.setInfo(SkImageInfo::Make(uWid, uHei,kN32_SkColorType,kPremul_SkAlphaType));
        void * pBits=NULL;
        m_hBmp = CreateGDIBitmap(uWid,uHei,&pBits);
        
        if(!m_hBmp) return E_OUTOFMEMORY;
        m_bitmap.setPixels(pBits);
        
        const int stride = m_bitmap.rowBytes();
        pFrame->CopyPixels(NULL, stride, stride * uHei,
            reinterpret_cast<BYTE*>(m_bitmap.getPixels()));
        return S_OK;
    }

    UINT SBitmap_Skia::Width()
    {
        return m_bitmap.width();
    }

    UINT SBitmap_Skia::Height()
    {
        return m_bitmap.height();
    }

    SIZE SBitmap_Skia::Size()
    {
        SIZE sz={m_bitmap.width(),m_bitmap.height()};
        return sz;
    }

    LPVOID SBitmap_Skia::LockPixelBits()
    {
        return m_bitmap.getPixels();
    }

    void SBitmap_Skia::UnlockPixelBits( LPVOID )
    {
    }

    //////////////////////////////////////////////////////////////////////////
    static int s_cRgn =0;
	SRegion_Skia::SRegion_Skia( IRenderFactory *pRenderFac )
        :TSkiaRenderObjImpl<IRegion>(pRenderFac)
	{
//         STRACE(L"region new; objects = %d",++s_cRgn);
	}

    SRegion_Skia::~SRegion_Skia()
    {
//         STRACE(L"region delete; objects = %d",--s_cRgn);
    }

	void SRegion_Skia::CombineRect( LPCRECT lprect,int nCombineMode )
	{
        m_rgn.op(toSkIRect(lprect),RGNMODE2SkRgnOP(nCombineMode));
	}

    void SRegion_Skia::CombineRgn(const IRegion * pRgnSrc,int nCombineMode)
    {
        const SRegion_Skia * pRgnSrc2 = (const SRegion_Skia*)pRgnSrc;
        m_rgn.op(pRgnSrc2->GetRegion(),RGNMODE2SkRgnOP(nCombineMode));
    }

    void SRegion_Skia::SetRgn(const HRGN hRgn)
    {
        DWORD dwSize = GetRegionData(hRgn,0,NULL);
        RGNDATA *pData = (RGNDATA*)malloc(dwSize);
        GetRegionData(hRgn,dwSize,pData);
        SkIRect *pRcs= new SkIRect[pData->rdh.nCount];
        LPRECT pRcsSrc = (LPRECT)pData->Buffer;
        for(unsigned int i = 0 ;i< pData->rdh.nCount;i++)
        {
            pRcs[i].fLeft = pRcsSrc[i].left;
            pRcs[i].fTop = pRcsSrc[i].top;
            pRcs[i].fRight = pRcsSrc[i].right;
            pRcs[i].fBottom = pRcsSrc[i].bottom;
        }
        m_rgn.setRects(pRcs,pData->rdh.nCount);
        free(pData);
        delete []pRcs;
    }
    
	BOOL SRegion_Skia::PtInRegion( POINT pt )
	{
        return m_rgn.contains(pt.x,pt.y);
	}

	BOOL SRegion_Skia::RectInRegion( LPCRECT lprect )
	{
        SASSERT(lprect);
        return m_rgn.intersects(toSkIRect(lprect));
	}

	void SRegion_Skia::GetRgnBox( LPRECT lprect )
	{
        SASSERT(lprect);
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

    SkRegion SRegion_Skia::GetRegion() const
    {
        return m_rgn;
    }

    void SRegion_Skia::SetRegion( const SkRegion & rgn )
    {
        m_rgn=rgn;
    }

    SkRegion::Op SRegion_Skia::RGNMODE2SkRgnOP( UINT mode )
    {
        SkRegion::Op op;
        switch(mode)
        {
        case RGN_COPY: op = SkRegion::kReplace_Op;break;
        case RGN_AND: op = SkRegion::kIntersect_Op;break;
        case RGN_OR: op = SkRegion::kUnion_Op;break;
        case RGN_DIFF: op = SkRegion::kDifference_Op;break;
        case RGN_XOR: op = SkRegion::kXOR_Op;break;
        default:SASSERT(FALSE);break;
        }
        return op;
    }

    void SRegion_Skia::Clear()
    {
        m_rgn.setEmpty();
    }


    //////////////////////////////////////////////////////////////////////////
    // SFont_Skia
    static int s_cFont =0;
    SFont_Skia::SFont_Skia( IRenderFactory * pRenderFac,const LOGFONT * plf ) :TSkiaRenderObjImpl<IFont>(pRenderFac),m_skFont(NULL)
    {
        memcpy(&m_lf,plf,sizeof(LOGFONT));
        SStringA strFace=S_CT2A(plf->lfFaceName,CP_UTF8);
        BYTE style=SkTypeface::kNormal;
        if(plf->lfItalic) style |= SkTypeface::kItalic;
        if(plf->lfWeight == FW_BOLD) style |= SkTypeface::kBold;

        m_skFont=SkTypeface::CreateFromName(strFace,(SkTypeface::Style)style);

        m_skPaint.setTextSize(SkIntToScalar(abs(plf->lfHeight)));
        m_skPaint.setUnderlineText(!!plf->lfUnderline);
        m_skPaint.setStrikeThruText(!!plf->lfStrikeOut);

        m_skPaint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
        m_skPaint.setAntiAlias(true);
        m_skPaint.setLCDRenderText(true);

//         STRACE(L"font new: objects = %d", ++s_cFont);
    }

    SFont_Skia::~SFont_Skia()
    {
        if(m_skFont) m_skFont->unref();
//         STRACE(L"font delete: objects = %d", --s_cFont);
    }
    //////////////////////////////////////////////////////////////////////////
    namespace RENDER_SKIA
    {
        BOOL SCreateInstance( IObjRef ** ppRenderFactory )
        {
            *ppRenderFactory = new SRenderFactory_Skia;
            return TRUE;
        }
    }

}//end of namespace SOUI

