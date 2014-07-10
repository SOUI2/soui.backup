#include "StdAfx.h"

#include <core\SkShader.h>
#include <core\SkDevice.h>
#include <effects\SkDashPathEffect.h>
#include <effects\SkGradientShader.h>
#include <gdialpha.h>

#include "drawtext-skia.h"
#include "render-skia.h"

#include "trace.h"
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
		,m_hBindDC(0)
		,m_SkCanvas(NULL)
        ,m_curColor(0xFF000000)//默认黑色
        ,m_hGetDC(0)
        ,m_uGetDCFlag(0)
	{
        m_ptOrg.fX=m_ptOrg.fY=0.0f;

		CAutoRefPtr<IPen> pPen;
		CreatePen(PS_SOLID,SColor(0,0,0).toCOLORREF(),1,&pPen);
		SelectObject(pPen);

		CAutoRefPtr<IBrush> pBr;
		CreateSolidColorBrush(SColor(0,0,0).toCOLORREF(),&pBr);
		SelectObject(pBr);

        CAutoRefPtr<IFont> pFont;
        LOGFONT lf={0};
        lf.lfHeight=20;
        _tcscpy(lf.lfFaceName,_T("宋体"));
        pRenderFactory->CreateFont(&pFont,lf);
        SelectObject(pFont);

        CAutoRefPtr<IBitmap> pBmp;
        GetRenderFactory_Skia()->CreateBitmap(&pBmp);
        pBmp->Init(nWid,nHei);
        SelectObject(pBmp);
        
        m_SkCanvas = new SkCanvas(m_curBmp->GetSkBitmap());
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
			SkBitmap bmp=m_curBmp->GetSkBitmap();

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
        if(m_SkCanvas->isClipEmpty())
        {
            SkRect rcAll={0.0f,0.0f,(SkScalar)m_curBmp->Width(),(SkScalar)m_curBmp->Height()};
            m_SkCanvas->clipRect(rcAll,SkRegion::kReplace_Op);
        }
        m_SkCanvas->clipRect(skrc,SkRegion::kDifference_Op);
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::IntersectClipRect( LPCRECT pRc )
    {
        SkRect skrc=toSkRect(pRc);
        skrc.offset(m_ptOrg);
        if(m_SkCanvas->isClipEmpty())
        {
            SkRect rcAll={0.0f,0.0f,(SkScalar)m_curBmp->Width(),(SkScalar)m_curBmp->Height()};
            m_SkCanvas->clipRect(rcAll,SkRegion::kReplace_Op);
        }
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
        pRgn->SetRegion(m_SkCanvas->getTotalClip());
        *ppRegion = pRgn;
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::GetClipBound(LPRECT prcBound)
    {
        SkRect skrc;
        m_SkCanvas->getClipBounds(&skrc);
        prcBound->left=(LONG)skrc.fLeft;
        prcBound->top=(LONG)skrc.fTop;
        prcBound->right=(LONG)skrc.fRight;
        prcBound->bottom=(LONG)skrc.fBottom;
        return S_OK;
    }
    
	HRESULT SRenderTarget_Skia::BitBlt( LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=SRCCOPY*/)
	{
        if(dwRop == SRCCOPY)
        {
            SRenderTarget_Skia *pRTSourSkia=(SRenderTarget_Skia*)pRTSour;

            const SkBitmap & bmpSrc= pRTSourSkia->m_SkCanvas->getDevice()->accessBitmap(true);
            SkRect rcDst=toSkRect(pRcDest);
            rcDst.offset(m_ptOrg);
            RECT rc={xSrc,ySrc,xSrc+pRcDest->right-pRcDest->left,ySrc+pRcDest->bottom-pRcDest->top};
            SkRect rcSrc=toSkRect(&rc);
            rcSrc.offset(pRTSourSkia->m_ptOrg);
            m_SkCanvas->drawBitmapRectToRect(bmpSrc,&rcSrc,rcDst);
        }else
        {
            HDC hdcSrc=pRTSour->GetDC(0);
            HDC hdcDst=GetDC(0);
            ALPHAINFO ai;
            CGdiAlpha::AlphaBackup(hdcDst,pRcDest,ai);
            ::BitBlt(hdcDst,pRcDest->left,pRcDest->top,pRcDest->right-pRcDest->left,pRcDest->bottom-pRcDest->top,hdcSrc,xSrc,ySrc,dwRop);
            CGdiAlpha::AlphaRestore(ai);
            ReleaseDC(hdcDst);
            pRTSour->ReleaseDC(hdcSrc);
        }
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat ,BYTE byAlpha)
	{
		if(cchLen<0) cchLen= _tcslen(pszText);
		if(cchLen==0) return S_FALSE;
		
		SStringW strW=S_CT2W(SStringT(pszText,cchLen));
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setColor(m_curColor.toARGB());
        txtPaint.setTypeface(m_curFont->GetFont());
        txtPaint.setAlpha(byAlpha);
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

	HRESULT SRenderTarget_Skia::DrawRectangle(LPRECT pRect)
	{
		SkPaint paint;
		paint.setColor(SColor(m_curPen->GetColor()).toARGB());
		SGetLineDashEffect skDash(m_curPen->GetStyle());
 		paint.setPathEffect(skDash.Get());
		paint.setStrokeWidth((SkScalar)m_curPen->GetWidth());
		paint.setStyle(SkPaint::kStroke_Style);

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

	HRESULT SRenderTarget_Skia::FillRectangle(LPRECT pRect)
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
		m_SkCanvas->drawRect(skrc,paint);
		return S_OK;
	}

    HRESULT SRenderTarget_Skia::DrawRoundRect( LPCRECT pRect,POINT pt )
    {
        SkPaint paint;
        paint.setColor(SColor(m_curPen->GetColor()).toARGB());
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
        paint.setColor(SColor(m_curPen->GetColor()).toARGB());
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
		SStringW strW=S_CT2W(lpszString,nCount);
        SkPaint     txtPaint = m_curFont->GetPaint();
        txtPaint.setColor(m_curColor.toARGB());
        txtPaint.setTypeface(m_curFont->GetFont());
        txtPaint.setAlpha(byAlpha);
		m_SkCanvas->drawText((LPCWSTR)strW,strW.GetLength()*2,(SkScalar)x+m_ptOrg.fX,(SkScalar)y+m_ptOrg.fY,txtPaint);
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
    

    HRESULT SRenderTarget_Skia::DrawBitmapEx( LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,EXPEND_MODE expendMode, BYTE byAlpha/*=0xFF*/ )
    {
        if(expendMode == EM_NULL)
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

        if(expendMode == EM_STRETCH)
        {
            m_SkCanvas->drawBitmapRectToRect(bmp,&rcSrc,rcDest,&paint);
        }else
        {
            SkBitmap bmpSub;
            bmp.extractSubset(&bmpSub,toSkIRect(pRcSrc));
            paint.setShader(SkShader::CreateBitmapShader(bmpSub,SkShader::kRepeat_TileMode,SkShader::kRepeat_TileMode))->unref();
            m_SkCanvas->drawRect(rcDest,paint);
        }
        return S_OK;

    }


    HRESULT SRenderTarget_Skia::DrawBitmap9Patch( LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,EXPEND_MODE expendMode,BYTE byAlpha/*=0xFF*/ )
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
        EXPEND_MODE mode[3][3]={
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
    HDC SRenderTarget_Skia::GetDC( UINT uFlag )
    {
        if(m_hGetDC) return m_hGetDC;
        
        HBITMAP bmp=m_curBmp->GetGdiBitmap();
        ASSERT(bmp);
        HDC hdc_desk = ::GetDC(NULL);
        m_hGetDC = CreateCompatibleDC(hdc_desk);
        ::ReleaseDC(NULL,hdc_desk);
        
        ::SelectObject(m_hGetDC,bmp);
        
        CAutoRefPtr<SRegion_Skia> rgnClip;
        GetClipRegion((IRegion**)&rgnClip);
        if(!rgnClip->IsEmpty())
        {
            SkRegion skrgn=rgnClip->GetRegion();
            SkRegion::Iterator it(skrgn);
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

    HRESULT SRenderTarget_Skia::GradientFill( LPCRECT pRect,BOOL bVert,COLORREF crBegin,COLORREF crEnd,BYTE byAlpha/*=0xFF*/ )
    {
        SkPoint pts[2];
        pts[0].set((SkScalar)pRect->left,(SkScalar)pRect->top);
        pts[1].set((SkScalar)pRect->right,(SkScalar)pRect->top);
        if(bVert)
        {
            pts[1].set((SkScalar)pRect->left,(SkScalar)pRect->bottom);
        }else
        {
            pts[1].set((SkScalar)pRect->right,(SkScalar)pRect->top);
        }
        
        SColor cr1(crBegin,byAlpha);
        SColor cr2(crEnd,byAlpha);
        
        SkColor colors[2] = {cr1.toARGB(),cr2.toARGB()};
        SkShader *pShader = SkGradientShader::CreateLinear(pts, colors, NULL,2,SkShader::kMirror_TileMode);
        SkPaint paint;
        paint.setShader(pShader);
        pShader->unref();
        m_SkCanvas->drawRect(toSkRect(pRect),paint);        
        return S_OK;
    }

    HRESULT SRenderTarget_Skia::FillSolidRect( LPCRECT pRect,COLORREF cr )
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SColor(cr).toARGB());

        SkRect skrc=toSkRect(pRect);
        skrc.offset(m_ptOrg);
        m_SkCanvas->drawRect(skrc,paint);
        return S_OK;    
    }

    HRESULT SRenderTarget_Skia::Clear(COLORREF cr)
    {
        m_SkCanvas->clear(SColor(cr).toARGB());
        return S_OK;
    }
    //////////////////////////////////////////////////////////////////////////
	// SBitmap_Skia

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

	HRESULT SBitmap_Skia::Init( int nWid,int nHei )
	{
		m_bitmap.reset();
		m_bitmap.setConfig(SkBitmap::kARGB_8888_Config,nWid,nHei);
        if(m_hBmp) DeleteObject(m_hBmp);
    		
		LPVOID pBits=NULL;
		m_hBmp=CreateGDIBitmap(nWid,nHei,&pBits);
		if(!m_hBmp) return E_OUTOFMEMORY;
		m_bitmap.setPixels(pBits);
		return S_OK;
	}

	HRESULT SBitmap_Skia::LoadFromFile( LPCTSTR pszFileName,LPCTSTR pszType )
	{
	    CAutoRefPtr<IImgDecoder> imgDecoder;
	    GetRenderFactory_Skia()->GetImgDecoderFactory()->CreateImgDecoder(&imgDecoder);
		if(imgDecoder->DecodeFromFile(S_CT2W(pszFileName))==0) return S_FALSE;
		return ImgFromDecoder(imgDecoder);
	}

	HRESULT SBitmap_Skia::LoadFromMemory(LPBYTE pBuf,size_t szLen,LPCTSTR pszType )
	{
        CAutoRefPtr<IImgDecoder> imgDecoder;
        GetRenderFactory_Skia()->GetImgDecoderFactory()->CreateImgDecoder(&imgDecoder);
		if(imgDecoder->DecodeFromMemory(pBuf,szLen)==0) return S_FALSE;
        return ImgFromDecoder(imgDecoder);
	}

    HRESULT SBitmap_Skia::ImgFromDecoder(IImgDecoder *imgDecoder)
    {
        IImgFrame *pFrame=imgDecoder->GetFrame(0);
        UINT uWid=0,uHei =0;
        pFrame->GetSize(&uWid,&uHei);

        if(m_hBmp) DeleteObject(m_hBmp);
        m_bitmap.reset();
        m_bitmap.setConfig(SkBitmap::kARGB_8888_Config, uWid, uHei);
        void * pBits=NULL;
        m_hBmp = CreateGDIBitmap(uWid,uHei,&pBits);
        
        if(!m_hBmp) return E_OUTOFMEMORY;
        m_bitmap.setPixels(pBits);
        
        m_bitmap.lockPixels();
        const int stride = m_bitmap.rowBytes();
        pFrame->CopyPixels(NULL, stride, stride * uHei,
            reinterpret_cast<BYTE*>(m_bitmap.getPixels()));
        m_bitmap.unlockPixels();
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

	//////////////////////////////////////////////////////////////////////////
	SRegion_Skia::SRegion_Skia( IRenderFactory *pRenderFac )
        :TSkiaRenderObjImpl<IRegion>(pRenderFac)
	{

	}

	void SRegion_Skia::CombineRect( LPCRECT lprect,int nCombineMode )
	{
        m_rgn.op(toSkIRect(lprect),RGNMODE2SkRgnOP(nCombineMode));
	}

	BOOL SRegion_Skia::PtInRegion( POINT pt )
	{
        return m_rgn.contains(pt.x,pt.y);
	}

	BOOL SRegion_Skia::RectInRegion( LPCRECT lprect )
	{
        ASSERT(lprect);
        return m_rgn.intersects(toSkIRect(lprect));
	}

	void SRegion_Skia::GetRgnBox( LPRECT lprect )
	{
        ASSERT(lprect);
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
        default:ASSERT(FALSE);break;
        }
        return op;
    }

    void SRegion_Skia::Clear()
    {
        m_rgn.setEmpty();
    }



}