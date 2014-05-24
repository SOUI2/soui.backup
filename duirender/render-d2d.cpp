#include "stdafx.h"

#include "render-d2d.h"
#include <math.h>
#include <tchar.h>
#include "obj-ref-impl.hpp"

#pragma  comment(lib,"d2d1.lib")
#pragma  comment(lib,"dwrite.lib")

#include "img-decoder.h"

namespace SOUI
{

	inline D2D1_RECT_F RECTF(LPCRECT prect)
	{
		return D2D1::RectF((float)prect->left,(float)prect->top,(float)prect->right,(float)prect->bottom);
	}


	int Rect_Wid(LPCRECT pRc){return pRc->right-pRc->left;}
	int Rect_Hei(LPCRECT pRc){return pRc->bottom-pRc->top;}
	//////////////////////////////////////////////////////////////////////////
	//	CDuiRenderFactory_D2D
	//////////////////////////////////////////////////////////////////////////
	BOOL SRenderFactory_D2D::CreateHwndRenderTarget( HWND hWnd,SIZE szTarget,IRenderTarget ** ppRenderTarget )
	{
		if(!m_pD2DFactory) return FALSE;
		ID2D1HwndRenderTarget *pRenderTarget=NULL;
		D2D_SIZE_U size=D2D1::SizeU(szTarget.cx,szTarget.cy);
		HRESULT hr=m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&pRenderTarget);
		if(SUCCEEDED(hr))
		{
			SRenderTarget_D2D *pRenderTargetD2D=new SRenderTarget_D2D(pRenderTarget,this);
			pRenderTarget->Release();
			*ppRenderTarget = pRenderTargetD2D;
			return TRUE;
		}else
		{
			return FALSE;
		}
	}

	BOOL SRenderFactory_D2D::CreateDCRenderTarget(IRenderTarget ** ppRenderTarget )
	{
		if(!m_pD2DFactory) return FALSE;
		CAutoRefPtr<ID2D1DCRenderTarget> pRenderTarget=NULL;
		HRESULT hr=m_pD2DFactory->CreateDCRenderTarget(&D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT, 
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			&pRenderTarget);
		if(SUCCEEDED(hr))
		{
			*ppRenderTarget=new SRenderTarget_D2D(pRenderTarget,this);
			return TRUE;
		}else
		{
			return FALSE;
		}
	}

	BOOL SRenderFactory_D2D::Init()
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
		if(!SUCCEEDED(hr)) return FALSE;

		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
			);
		if(!SUCCEEDED(hr)) return FALSE;

		// Create WIC factory.
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&m_pWICImageFactory)
			);
		if(!SUCCEEDED(hr)) return FALSE;
		return TRUE;
	}


	//////////////////////////////////////////////////////////////////////////
	//	CDuiRenderTarget_D2D
	//////////////////////////////////////////////////////////////////////////

	void SRenderTarget_D2D::SetTextFormat( IDWriteTextFormat *txtFormat,UINT uFormat )
	{
		if(uFormat & DT_SINGLELINE) txtFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		else txtFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

		if(uFormat&DT_CENTER) txtFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		else if(uFormat&DT_RIGHT) txtFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		else txtFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

		if(uFormat&DT_VCENTER) txtFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		else if(uFormat&DT_BOTTOM) txtFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		else txtFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
	}

	HRESULT SRenderTarget_D2D::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat,BYTE byAlpha)
	{
		if(cchLen==-1) cchLen=_tcslen(pszText);
		D2D1_RECT_F rect=D2D1::RectF((float)pRc->left,(float)pRc->top,(float)pRc->right,(float)pRc->bottom);
		SetTextFormat(m_curTxtFmt,uFormat);
		CDuiStringW strW=DUI_CT2W(CDuiStringT(pszText,cchLen));
        m_curPen->GetBrush()->SetOpacity(((float)byAlpha)/255.0f);
		m_pD2DRenderTarget->DrawText(strW,strW.GetLength(),m_curTxtFmt,rect,m_curPen->GetBrush());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::MeasureText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat )
	{
		if(cchLen==-1) cchLen=_tcslen(pszText);
		CDuiStringW strW=DUI_CT2W(CDuiStringT(pszText,cchLen));
		SetTextFormat(m_curTxtFmt,uFormat);
		CAutoRefPtr<IDWriteTextLayout> pLayout;
		HRESULT hr=GetRenderFactory_D2D()->GetWriteFactory()->CreateTextLayout(strW,cchLen,m_curTxtFmt,(float)Rect_Wid(pRc),(float)Rect_Hei(pRc),&pLayout);
		if(SUCCEEDED(hr))
		{
			DWRITE_TEXT_METRICS metrics;
			hr=pLayout->GetMetrics(&metrics);
			if(SUCCEEDED(hr))
			{
				pRc->right=pRc->left+(int)ceil(metrics.widthIncludingTrailingWhitespace);
				pRc->bottom=pRc->top+(int)ceil(metrics.height);
			}
		}
		return hr;
	}


	HRESULT SRenderTarget_D2D::TextOut( int x, int y, LPCTSTR lpszString, int nCount  ,BYTE byAlpha)
	{
		RECT rc={x,y,x+10000,y+10000};
		return DrawText(lpszString,nCount,&rc,DT_SINGLELINE,byAlpha);
	}

	HRESULT SRenderTarget_D2D::GetTextExtentPoint32( LPCTSTR lpString, UINT cbCount, LPSIZE lpSize )
	{
		RECT rc={0,0,lpSize->cx,lpSize->cy};
		return MeasureText(lpString,cbCount,&rc,DT_SINGLELINE);
	}

	HRESULT SRenderTarget_D2D::BeginDraw()
	{
		m_pD2DRenderTarget->BeginDraw();
		m_pD2DRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::EndDraw()
	{
		return m_pD2DRenderTarget->EndDraw();
	}

	HRESULT SRenderTarget_D2D::Resize( SIZE sz )
	{
		D2D_SIZE_U szd2d;
		szd2d.width=sz.cx;
		szd2d.height=sz.cy;
		CAutoRefPtr<ID2D1HwndRenderTarget> pHwndTarget=NULL;
		m_pD2DRenderTarget->QueryInterface(__uuidof(ID2D1HwndRenderTarget),(void**) &pHwndTarget);
		if(pHwndTarget)
		{
			return pHwndTarget->Resize(szd2d);
		}else
		{
			CAutoRefPtr<ID2D1BitmapRenderTarget> pIBmp=NULL;
			HRESULT hr=m_pD2DRenderTarget->QueryInterface(__uuidof(ID2D1BitmapRenderTarget),(void**) &pIBmp);
			if(pIBmp)
			{
				CAutoRefPtr<ID2D1Bitmap> pBitmap=NULL;
				hr=pIBmp->GetBitmap(&pBitmap);
				//ASSERT(pBitmap);
				D2D1_SIZE_U szCur=pBitmap->GetPixelSize();
				if(szCur.width!=szd2d.width || szCur.height!=szd2d.height)
				{//创建一个新的位图表面来代替原来的位图表面
					CAutoRefPtr<ID2D1BitmapRenderTarget> pNewITarget=NULL;
					hr=pIBmp->CreateCompatibleRenderTarget(NULL,&szd2d,NULL,D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_GDI_COMPATIBLE,&pNewITarget);
					if(pNewITarget)
					{
						m_pD2DRenderTarget=pNewITarget;
					}
				}
			}
			return hr;
		}
	}

	HRESULT SRenderTarget_D2D::DrawRectangle( int left, int top,int right,int bottom )
	{
		D2D1_RECT_F rc=D2D1::RectF((float)left,(float)top,(float)right,(float)bottom);
		m_pD2DRenderTarget->DrawRectangle(rc,m_curPen->GetBrush(),(float)m_curPen->GetWidth(),m_curPen->GetStrokeStyle());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::FillRectangle( int left, int top,int right,int bottom )
	{
		D2D1_RECT_F rc=D2D1::RectF((float)left,(float)top,(float)right,(float)bottom);
		m_pD2DRenderTarget->FillRectangle(rc,m_curBrush->GetBrush());
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::CreateCompatibleRenderTarget( SIZE szTarget,IRenderTarget **ppRenderTarget )
	{
		D2D1_SIZE_U szPixel={szTarget.cx,szTarget.cy};
		CAutoRefPtr<ID2D1BitmapRenderTarget> pNewITarget=NULL;
		HRESULT hr=m_pD2DRenderTarget->CreateCompatibleRenderTarget(NULL,&szPixel,NULL,D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_GDI_COMPATIBLE,&pNewITarget);
		if(SUCCEEDED(hr))
		{
			*ppRenderTarget = new SRenderTarget_D2D(pNewITarget,m_pRenderFactory);
			return (*ppRenderTarget)?S_OK:E_OUTOFMEMORY;
		}
		return hr;
	}

	HRESULT SRenderTarget_D2D::CreatePen( int iStyle,COLORREF cr,int cWidth,IPen ** ppPen )
	{
		CDuiColor duicr(cr);
		D2D1_COLOR_F color=D2D1::ColorF(duicr.r/255.f,duicr.g/255.f,duicr.b/255.f,duicr.a/255.f);
		CAutoRefPtr<ID2D1SolidColorBrush> pIBrush=NULL;
		CAutoRefPtr<ID2D1StrokeStyle> pIStrokeStyle=NULL;

		HRESULT hr=m_pD2DRenderTarget->CreateSolidColorBrush(color,&pIBrush);
		if(!SUCCEEDED(hr)) return hr;

		D2D1_STROKE_STYLE_PROPERTIES styleProps;
		styleProps.startCap=D2D1_CAP_STYLE_ROUND;
		styleProps.endCap=D2D1_CAP_STYLE_ROUND;
		styleProps.dashCap=D2D1_CAP_STYLE_ROUND;
		styleProps.lineJoin=D2D1_LINE_JOIN_ROUND;
		styleProps.miterLimit=10.f;
		styleProps.dashStyle=D2D1_DASH_STYLE_CUSTOM;
		styleProps.dashOffset=0.f;
		float ps_solid[]={1.0f,0.f};
		float ps_dot[]={1.0f, 1.0f};
		float ps_dash[]={3.0f,1.0f,1.0f};
		float ps_dashdot[]={3.0f,1.0f,2.0f,1.0f,1.0f};
		float ps_dashdotdot[]={3.0f,1.0f,2.0f,1.0f,1.0f,1.0,1.0};
		float *dash;
		int dashcount=0;
		switch(iStyle)
		{
		case PS_DASH:
			dash=ps_dash,dashcount=ARRAYSIZE(ps_dash);break;
		case PS_DOT:
			dash=ps_dot,dashcount=ARRAYSIZE(ps_dot);break;
		case PS_DASHDOT:
			dash=ps_dashdot,dashcount=ARRAYSIZE(ps_dashdot);break;
		case PS_DASHDOTDOT:
			dash=ps_dashdotdot,dashcount=ARRAYSIZE(ps_dashdotdot);break;
		case PS_SOLID:default:
			dash=ps_solid,dashcount=ARRAYSIZE(ps_solid);break;
		}
		hr=GetRenderFactory_D2D()->GetD2DFactory()->CreateStrokeStyle(styleProps,dash,dashcount,&pIStrokeStyle);
		if(SUCCEEDED(hr))
		{
			*ppPen=new SPen_D2D(GetRenderFactory_D2D(),pIBrush,pIStrokeStyle,cWidth);
			return (*ppPen)?S_OK:E_OUTOFMEMORY;
		}
		return hr;
	}


	HRESULT SRenderTarget_D2D::CreateFont( const LOGFONT &lf,IFont ** ppFont )
	{
		*ppFont = new SFont_D2D(GetRenderFactory_D2D(),lf);
		return (*ppFont)?S_OK:E_OUTOFMEMORY;
	}

	HRESULT SRenderTarget_D2D::CreateSolidColorBrush( COLORREF cr,IBrush ** ppBrush )
	{
		CDuiColor duicr(cr);
		D2D1_COLOR_F color=D2D1::ColorF(duicr.r/255.f,duicr.g/255.f,duicr.b/255.f,duicr.a/255.f);
		CAutoRefPtr<ID2D1SolidColorBrush> pIBrush=NULL;
		HRESULT hr=m_pD2DRenderTarget->CreateSolidColorBrush(color,&pIBrush);
		if(SUCCEEDED(hr))
		{
			*ppBrush=new SBrush_D2D(GetRenderFactory_D2D(),pIBrush);
			return (*ppBrush)?S_OK:E_OUTOFMEMORY;
		}
		return hr;
	}

	HRESULT SRenderTarget_D2D::CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )
	{
		SBitmap_D2D * pBmp_D2D=(SBitmap_D2D*)pBmp;
		D2D1_BITMAP_BRUSH_PROPERTIES prop={D2D1_EXTEND_MODE_WRAP,D2D1_EXTEND_MODE_WRAP,D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR};

		CAutoRefPtr<ID2D1BitmapBrush> pIBrush=NULL;
		HRESULT hr = m_pD2DRenderTarget->CreateBitmapBrush(pBmp_D2D->GetBitmap(),prop,&pIBrush);
		if(SUCCEEDED(hr))
		{
			*ppBrush=new SBrush_D2D(GetRenderFactory_D2D(),pIBrush);
			return (*ppBrush)?S_OK:E_OUTOFMEMORY;
		}
		return hr;
	}

	SRenderTarget_D2D::SRenderTarget_D2D( ID2D1RenderTarget *pRenderTarget,IRenderFactory_D2D* pRenderFactory ) 
		:m_pD2DRenderTarget(pRenderTarget),TD2DRenderObjImpl<IRenderTarget>(pRenderFactory)
	{
        CAutoRefPtr<IBrush> pBrush=NULL;
		CreateSolidColorBrush(CDuiColor(255,255,255),&pBrush);
		SelectObject(pBrush);

		CAutoRefPtr<IPen> pPen=NULL;
		CreatePen(PS_SOLID,CDuiColor(0,0,0),1,&pPen);
		SelectObject(pPen);

		CAutoRefPtr<IFont> pFont=NULL;
		LOGFONT lf={0};
		lf.lfHeight=20;
		_tcscpy(lf.lfFaceName,_T("宋体"));
		CreateFont(lf,&pFont);
		SelectObject(pFont);
	}


	HRESULT SRenderTarget_D2D::CreateRegion( IRegion ** ppRegion )
	{
		*ppRegion = new SRegion_D2D(m_pRenderFactory);
		return (*ppRegion)?S_OK:E_OUTOFMEMORY;
	}


	HRESULT SRenderTarget_D2D::CreateBitmap( IBitmap ** ppBitmap )
	{
		*ppBitmap = new SBitmap_D2D(m_pRenderFactory);
		return (*ppBitmap)?S_OK:E_OUTOFMEMORY;
	}

	HRESULT SRenderTarget_D2D::DrawBitmap( LPRECT pRcDest,IBitmap *pBitmap,LPRECT pRcSour,BYTE byAlpha )
	{
		SBitmap_D2D *pBmpD2D=dynamic_cast<SBitmap_D2D*>(pBitmap);
		if(!pBmpD2D || !pBmpD2D->GetBitmap()) return S_FALSE;
		D2D1_RECT_F rcDest={(float)pRcDest->left,(float)pRcDest->top,(float)pRcDest->right,(float)pRcDest->bottom};

		D2D1_RECT_F rcSour;
		D2D1_RECT_F *prcSourD2D=NULL;
		if(pRcSour)
		{
			rcSour=D2D1::RectF((float)pRcSour->left,(float)pRcSour->top,(float)pRcSour->right,(float)pRcSour->bottom);
			prcSourD2D=&rcSour;
		}
		m_pD2DRenderTarget->DrawBitmap(pBmpD2D->GetBitmap(),&rcDest,byAlpha/255.f,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,prcSourD2D);
		return S_OK;
	}

	//只有Source Render Target是ID2D1BitmapRenderTarget才能使用该接口
	HRESULT SRenderTarget_D2D::BitBlt( LPRECT pRcDest,IRenderTarget *pRTSour,LPRECT pRcSour,UINT uDef )
	{
		HRESULT hr=S_FALSE;
		SRenderTarget_D2D *pRTSourD2D=(SRenderTarget_D2D *)pRTSour;
		if(pRTSourD2D)
		{
			ID2D1BitmapRenderTarget *pBmpRT=NULL;
			hr=pRTSourD2D->m_pD2DRenderTarget->QueryInterface(__uuidof(ID2D1BitmapRenderTarget),(void**)&pBmpRT);
			if(pBmpRT)
			{
				ID2D1Bitmap *pBitmap=NULL;
				hr=pBmpRT->GetBitmap(&pBitmap);
				if(pBitmap)
				{
					D2D1_RECT_F rectDest={(float)pRcDest->left,(float)pRcDest->top,(float)pRcDest->right,(float)pRcDest->bottom};
					D2D1_RECT_F rectSour={(float)pRcSour->left,(float)pRcSour->top,(float)pRcSour->right,(float)pRcSour->bottom};
					m_pD2DRenderTarget->DrawBitmap(pBitmap,rectDest,1.0,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,rectSour);
				}
			}
		}
		return hr;
	}

	HRESULT SRenderTarget_D2D::BindDC( HDC hdc ,LPCRECT pSubRect)
	{
		CAutoRefPtr<ID2D1DCRenderTarget> pDCRT;
		m_pD2DRenderTarget->QueryInterface(__uuidof(ID2D1DCRenderTarget),(void**)&pDCRT);
		if(!pDCRT) return FALSE;
		return pDCRT->BindDC(hdc,pSubRect);
	}

	HRESULT SRenderTarget_D2D::PushClipRect( LPCRECT pRect )
	{
		m_pD2DRenderTarget->PushAxisAlignedClip(D2D1::RectF((float)pRect->left,(float)pRect->top,(float)pRect->right,(float)pRect->bottom),
			D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::PopClipRect()
	{
		m_pD2DRenderTarget->PopAxisAlignedClip();
		return S_OK;
	}

	HRESULT SRenderTarget_D2D::PushClipRegion( IRegion *pRegion )
	{
		SRegion_D2D *pRegionD2D=(SRegion_D2D*)pRegion;
		ASSERT(pRegionD2D);
		CAutoRefPtr<ID2D1Layer> pLayer;
		HRESULT hr=m_pD2DRenderTarget->CreateLayer(&pLayer);
		if(SUCCEEDED(hr)) m_pD2DRenderTarget->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(),pRegionD2D->GetGeometry()),pLayer);
		return hr;
	}

	HRESULT SRenderTarget_D2D::PopClipRegion()
	{
		m_pD2DRenderTarget->PopLayer();
		return S_OK;
	}

	IRenderObj * SRenderTarget_D2D::GetCurrentObject( OBJTYPE uType )
	{
		switch(uType)
		{
		case OT_FONT:return m_curPen;
		case OT_BITMAP:return NULL;
		case OT_BRUSH:return m_curBrush;
		case OT_PEN:return m_curPen;
		}
		return NULL;
	}

	IRenderObj * SRenderTarget_D2D::SelectObject( IRenderObj *pObj )
	{
		CAutoRefPtr<IRenderObj>  pRet;
		switch(pObj->ObjectType())
		{
		case OT_FONT:
            {
                pRet= m_curFont;
                m_curFont=(SFont_D2D*)(pObj);
                CreateTextFormat(m_curFont->GetLogfont(),&m_curTxtFmt);
                break;
            }
		case OT_BITMAP:break;
		case OT_BRUSH:pRet= m_curBrush;m_curBrush=(SBrush_D2D*)(pObj);break;
		case OT_PEN:pRet= m_curPen;m_curPen=(SPen_D2D*)(pObj);break;
		}
		if(pRet)
		{//由调用者调用Release释放该RenderObj
			pRet->AddRef();
		}
		return pRet;
	}

    HRESULT SRenderTarget_D2D::CreateTextFormat( const LOGFONT & lf ,IDWriteTextFormat ** ppTxtFormat)
    {
        if(!GetRenderFactory_D2D()->GetWriteFactory()) return S_FALSE;

        DWRITE_FONT_WEIGHT weight=(DWRITE_FONT_WEIGHT)lf.lfWeight;
        if(weight==0) weight = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE style=DWRITE_FONT_STYLE_NORMAL;
        if(lf.lfItalic) style=DWRITE_FONT_STYLE_ITALIC;

        HRESULT hr =GetRenderFactory_D2D()->GetWriteFactory()->CreateTextFormat(DUI_CT2W(lf.lfFaceName),NULL,weight,style,DWRITE_FONT_STRETCH_NORMAL,(float)lf.lfHeight,L"",ppTxtFormat);
        return hr;
    }

	//////////////////////////////////////////////////////////////////////////
	//	CDuiDirtyRegion_D2D
	//////////////////////////////////////////////////////////////////////////

	void SRegion_D2D::CombineRect( LPCRECT lprect ,int nCombineMode)
	{
		CAutoRefPtr<ID2D1RectangleGeometry> pIRcGeo=NULL;
		D2D1_RECT_F rcF={(float)lprect->left,(float)lprect->top,(float)lprect->right,(float)lprect->bottom};
		GetRenderFactory_D2D()->GetD2DFactory()->CreateRectangleGeometry(rcF,&pIRcGeo);
		ASSERT(pIRcGeo);

		if(nCombineMode==RGN_COPY)
		{
			CAutoRefPtr<ID2D1PathGeometry> pathGeo;
			GetRenderFactory_D2D()->GetD2DFactory()->CreatePathGeometry(&pathGeo);
			CAutoRefPtr<ID2D1GeometrySink> geoSink;
			pathGeo->Open(&geoSink);
			geoSink->BeginFigure(D2D1::Point2F((float)lprect->left, (float)lprect->top), D2D1_FIGURE_BEGIN_FILLED);
			geoSink->AddLine(D2D1::Point2F((float)lprect->right, (float)lprect->top));
			geoSink->AddLine(D2D1::Point2F((float)lprect->right, (float)lprect->bottom));
			geoSink->AddLine(D2D1::Point2F((float)lprect->left, (float)lprect->bottom));
			geoSink->EndFigure(D2D1_FIGURE_END_CLOSED);
			geoSink->Close();

			m_d2dRegion=pathGeo;
			return ;
		}

		ASSERT(m_d2dRegion);
		CAutoRefPtr<ID2D1PathGeometry> pIPathGeo;
		GetRenderFactory_D2D()->GetD2DFactory()->CreatePathGeometry(&pIPathGeo);
		ASSERT(pIPathGeo);
		CAutoRefPtr<ID2D1GeometrySink> pSink;
		pIPathGeo->Open(&pSink);

		D2D1_COMBINE_MODE d2dCombineMode;
		switch(nCombineMode)
		{
		case RGN_OR:d2dCombineMode=D2D1_COMBINE_MODE_UNION;break;
		case RGN_AND:d2dCombineMode=D2D1_COMBINE_MODE_INTERSECT;break;
		case RGN_DIFF:d2dCombineMode=D2D1_COMBINE_MODE_EXCLUDE;break;
		case RGN_XOR:d2dCombineMode=D2D1_COMBINE_MODE_XOR;break;
		}
		m_d2dRegion->CombineWithGeometry(pIRcGeo,d2dCombineMode,NULL,NULL,pSink);
		pSink->Close();

		m_d2dRegion=pIPathGeo;//将m_d2dRegion指向合并后的区域
	}

	BOOL SRegion_D2D::PtInRegion( POINT pt )
	{
		BOOL bInRegin=FALSE;
		m_d2dRegion->FillContainsPoint(D2D1::Point2F((float)pt.x,(float)pt.y),NULL,0,&bInRegin);
		return bInRegin;
	}

	BOOL SRegion_D2D::RectInRegion( LPCRECT lprect )
	{
		D2D1_GEOMETRY_RELATION relation=D2D1_GEOMETRY_RELATION_UNKNOWN;
		CAutoRefPtr<ID2D1RectangleGeometry> pRcGeo;
		GetRenderFactory_D2D()->GetD2DFactory()->CreateRectangleGeometry(RECTF(lprect),&pRcGeo);
		m_d2dRegion->CompareWithGeometry(pRcGeo,NULL,0,&relation);
		return relation!=D2D1_GEOMETRY_RELATION_DISJOINT;
	}

	void SRegion_D2D::GetRgnBox( LPRECT lprect )
	{
		D2D1_RECT_F rc;
		m_d2dRegion->GetBounds(NULL,&rc);
		lprect->left=(int)rc.left;
		lprect->top=(int)rc.top;
		lprect->right=(int)rc.right;
		lprect->bottom=(int)rc.bottom;
	}

	BOOL SRegion_D2D::IsEmpty()
	{
		RECT rcBox;
		GetRgnBox(&rcBox);
		return ::IsRectEmpty(&rcBox);
	}

	SRegion_D2D::SRegion_D2D( IRenderFactory_D2D *pRenderFac ) :TD2DRenderObjImpl<IRegion>(pRenderFac)
	{
		SRenderFactory_D2D *pFac=(SRenderFactory_D2D*)pRenderFac;
		GetRenderFactory_D2D()->GetD2DFactory()->CreatePathGeometry(&m_d2dRegion);
		RECT rc={0};
		CombineRect(&rc,RGN_COPY);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	HRESULT SBitmap_D2D::Init( IRenderTarget *pRT,int nWid,int nHei )
	{
		D2D1_SIZE_U size={nWid,nHei};
		float dpix,dpiy;
		SRenderTarget_D2D *pRtD2d=(SRenderTarget_D2D *)pRT;
		pRtD2d->m_pD2DRenderTarget->GetDpi(&dpix,&dpiy);
		D2D1_BITMAP_PROPERTIES prop={{DXGI_FORMAT_R8G8B8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},dpix,dpiy};
		CAutoRefPtr<ID2D1Bitmap> pIBitmap=NULL;
		HRESULT hr=pRtD2d->m_pD2DRenderTarget->CreateBitmap(size,prop,&pIBitmap);
		if(SUCCEEDED(hr))
		{
			m_bitmap=pIBitmap;
		}
		return hr;
	}

	HRESULT SBitmap_D2D::LoadFromFile( IRenderTarget *pRT,LPCTSTR pszFileName,LPCTSTR pszType )
	{
		HRESULT hr = S_OK;

		SImgDecoder imgDocoder;
		int nImg = imgDocoder.DecodeFromFile(DUI_CT2W(pszFileName));
		
		if(nImg == 0) return S_FALSE;

		SRenderTarget_D2D * pRenderTarget = (SRenderTarget_D2D *)pRT;

		// Create a Direct2D bitmap from the WIC bitmap.
		CAutoRefPtr<ID2D1Bitmap> pIBitmap=NULL;
		hr = pRenderTarget->m_pD2DRenderTarget->CreateBitmapFromWicBitmap(
			imgDocoder.GetFrame(0),
			NULL,
			&pIBitmap
			);
		if(SUCCEEDED(hr))
		{
			m_bitmap=pIBitmap;
		}
		return hr;
	}

	HRESULT SBitmap_D2D::LoadFromMemory( IRenderTarget *pRT,LPBYTE pBuf,size_t szLen,LPCTSTR pszType )
	{
		HRESULT hr = S_OK;
		SImgDecoder imgDocoder;
		int nImg = imgDocoder.DecodeFromMemory(pBuf,szLen);

		if(nImg == 0) return S_FALSE;

		SRenderTarget_D2D * pRenderTarget = (SRenderTarget_D2D *)pRT;

		// Create a Direct2D bitmap from the WIC bitmap.
		CAutoRefPtr<ID2D1Bitmap> pIBitmap=NULL;
		hr = pRenderTarget->m_pD2DRenderTarget->CreateBitmapFromWicBitmap(
			imgDocoder.GetFrame(0),
			NULL,
			&pIBitmap
			);
		if(SUCCEEDED(hr))
		{
			m_bitmap=pIBitmap;
		}
		return hr;
	}

}//end of namespace SOUI
