// render-gdi.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "render-gdi.h"
#include "GradientFillHelper.h"
#include <gdialpha.h>

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    // SRenderFactory_GDI
    BOOL SRenderFactory_GDI::CreateRenderTarget( IRenderTarget ** ppRenderTarget ,int nWid,int nHei)
    {
        *ppRenderTarget = new SRenderTarget_GDI(this, nWid, nHei);
        return TRUE;
    }

    BOOL SRenderFactory_GDI::CreateFont( IFont ** ppFont , const LOGFONT &lf )
    {
        *ppFont = new SFont_GDI(this,&lf);
        return TRUE;
    }

    BOOL SRenderFactory_GDI::CreateBitmap( IBitmap ** ppBitmap )
    {
        *ppBitmap = new SBitmap_GDI(this);
        return TRUE;
    }

    BOOL SRenderFactory_GDI::CreateRegion( IRegion **ppRgn )
    {
        *ppRgn = new SRegion_GDI(this);
        return TRUE;
    }

    
    //////////////////////////////////////////////////////////////////////////
    //  SBitmap_GDI
    HBITMAP SBitmap_GDI::CreateGDIBitmap( int nWid,int nHei,void ** ppBits )
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

    HRESULT SBitmap_GDI::Init( int nWid,int nHei )
    {
        if(m_hBmp) DeleteObject(m_hBmp);
        m_hBmp = CreateGDIBitmap(nWid,nHei,NULL);
        if(m_hBmp) m_sz.cx=nWid,m_sz.cy=nHei;
        return m_hBmp?S_OK:E_OUTOFMEMORY;
    }

    HRESULT SBitmap_GDI::LoadFromFile( LPCTSTR pszFileName,LPCTSTR pszType )
    {
        CAutoRefPtr<IImgDecoder> imgDecoder;
        GetRenderFactory_GDI()->GetImgDecoderFactory()->CreateImgDecoder(&imgDecoder);
        if(imgDecoder->DecodeFromFile(DUI_CT2W(pszFileName))==0) return S_FALSE;
        return ImgFromDecoder(imgDecoder);
    }

    HRESULT SBitmap_GDI::LoadFromMemory(LPBYTE pBuf,size_t szLen,LPCTSTR pszType )
    {
        CAutoRefPtr<IImgDecoder> imgDecoder;
        GetRenderFactory_GDI()->GetImgDecoderFactory()->CreateImgDecoder(&imgDecoder);
        if(imgDecoder->DecodeFromMemory(pBuf,szLen)==0) return S_FALSE;
        return ImgFromDecoder(imgDecoder);
    }

    HRESULT SBitmap_GDI::ImgFromDecoder(IImgDecoder *imgDecoder)
    {
        IImgFrame *pFrame=imgDecoder->GetFrame(0);
        UINT nWid,nHei;
        pFrame->GetSize(&nWid,&nHei);
        m_sz.cx=nWid,m_sz.cy=nHei;
        
        if(m_hBmp) DeleteObject(m_hBmp);
        void * pBits=NULL;
        m_hBmp = CreateGDIBitmap(m_sz.cx,m_sz.cy,&pBits);

        if(!m_hBmp) return E_OUTOFMEMORY;
        const int stride = m_sz.cx*4;
        pFrame->CopyPixels(NULL, stride, stride * m_sz.cy,
            reinterpret_cast<BYTE*>(pBits));
        return S_OK;
    }

    UINT SBitmap_GDI::Width()
    {
        return m_sz.cx;
    }

    UINT SBitmap_GDI::Height()
    {
        return m_sz.cy;
    }

    SIZE SBitmap_GDI::Size()
    {
        return m_sz;
    }
    //////////////////////////////////////////////////////////////////////////
    //	SRegion_GDI
    SRegion_GDI::SRegion_GDI( IRenderFactory_GDI *pRenderFac )
        :TSkiaRenderObjImpl<IRegion>(pRenderFac)
    {
        m_hRgn = ::CreateRectRgn(0,0,0,0);
    }

    void SRegion_GDI::CombineRect( LPCRECT lprect,int nCombineMode )
    {
        HRGN hRgn=::CreateRectRgnIndirect(lprect);
        ::CombineRgn(m_hRgn,m_hRgn,hRgn,nCombineMode);
        DeleteObject(hRgn);
    }

    BOOL SRegion_GDI::PtInRegion( POINT pt )
    {
        return ::PtInRegion(m_hRgn,pt.x,pt.y);
    }

    BOOL SRegion_GDI::RectInRegion( LPCRECT lprect )
    {
        ASSERT(lprect);
        return ::RectInRegion(m_hRgn,lprect);
    }

    void SRegion_GDI::GetRgnBox( LPRECT lprect )
    {
        ASSERT(lprect);
        ::GetRgnBox(m_hRgn,lprect);
    }

    BOOL SRegion_GDI::IsEmpty()
    {
        RECT rcBox;
        GetRgnBox(&rcBox);
        return (rcBox.left == rcBox.right) || (rcBox.top== rcBox.bottom);
    }

    void SRegion_GDI::Offset( POINT pt )
    {
        ::OffsetRgn(m_hRgn,pt.x,pt.y);
    }

    HRGN SRegion_GDI::GetRegion() const
    {
        return m_hRgn;
    }

    void SRegion_GDI::SetRegion( const HRGN  rgn )
    {
        ::CombineRgn(m_hRgn,rgn,NULL,RGN_COPY);
    }

    void SRegion_GDI::Clear()
    {
        ::SetRectRgn(m_hRgn,0,0,0,0);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //	SRenderTarget_GDI
    //////////////////////////////////////////////////////////////////////////
    SRenderTarget_GDI::SRenderTarget_GDI( IRenderFactory_GDI* pRenderFactory ,int nWid,int nHei)
        :TSkiaRenderObjImpl<IRenderTarget>(pRenderFactory)
        ,m_hBindDC(0)
        ,m_hdc(NULL)
        ,m_curColor(0xFF000000)//默认黑色
        ,m_uGetDCFlag(0)
    {
        m_ptOrg.x=m_ptOrg.y=0;
        
        HDC hdc=::GetDC(NULL);
        m_hdc = CreateCompatibleDC(hdc);
        ::ReleaseDC(NULL,hdc);
        ::SetBkMode(m_hdc,TRANSPARENT);
        
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
        GetRenderFactory_GDI()->CreateBitmap(&pBmp);
        pBmp->Init(nWid,nHei);
        SelectObject(pBmp);

    }

    SRenderTarget_GDI::~SRenderTarget_GDI()
    {
        DeleteDC(m_hdc);
    }

    HRESULT SRenderTarget_GDI::CreateCompatibleRenderTarget( SIZE szTarget,IRenderTarget **ppRenderTarget )
    {
        SRenderTarget_GDI *pRT = new SRenderTarget_GDI(GetRenderFactory_GDI(),szTarget.cx,szTarget.cy);
        *ppRenderTarget = pRT;
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::CreatePen( int iStyle,COLORREF cr,int cWidth,IPen ** ppPen )
    {
        *ppPen = new SPen_GDI(GetRenderFactory_GDI(),iStyle,cr,cWidth);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::CreateSolidColorBrush( COLORREF cr,IBrush ** ppBrush )
    {
        *ppBrush = SBrush_GDI::CreateSolidBrush(GetRenderFactory_GDI(),cr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::CreateBitmapBrush( IBitmap *pBmp,IBrush ** ppBrush )
    {
        SBitmap_GDI *pBmpSkia = (SBitmap_GDI*)pBmp;
        *ppBrush = SBrush_GDI::CreateBitmapBrush(GetRenderFactory_GDI(),pBmpSkia->GetBitmap());
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::BindDC( HDC hdc,LPCRECT pSubRect )
    {
        m_hBindDC=hdc;
        m_rcBind = *pSubRect;
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::BeginDraw()
    {
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::EndDraw()
    {
        if(m_hBindDC)
        {//copy image to bind dc
            ::BitBlt(m_hBindDC,m_rcBind.left,m_rcBind.top,m_rcBind.right-m_rcBind.left,m_rcBind.bottom-m_rcBind.top,
            m_hdc,0,0,SRCCOPY);
        }
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::Resize( SIZE sz )
    {
        HBITMAP hBmp=CreateCompatibleBitmap(m_hdc,0,0);
        ::SelectObject(m_hdc,hBmp);
        m_curBmp->Init(sz.cx,sz.cy);
        ::SelectObject(m_hdc,m_curBmp->GetBitmap());
        ::DeleteObject(hBmp);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PushClipRect( LPCRECT pRect ,UINT mode/*=RGN_AND*/)
    {
        RECT rc=*pRect;
        ::OffsetRect(&rc,m_ptOrg.x,m_ptOrg.y);
        HRGN hRgn=::CreateRectRgnIndirect(pRect);
        ::SaveDC(m_hdc);
        ::ExtSelectClipRgn(m_hdc,hRgn,mode);
        ::DeleteObject(hRgn);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PopClipRect()
    {
        ::RestoreDC(m_hdc,-1);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PushClipRegion( IRegion *pRegion ,UINT mode/*=RGN_AND*/)
    {
        SRegion_GDI *pRgnGDI=(SRegion_GDI*)pRegion;
        HRGN hRgn=::CreateRectRgn(0,0,0,0);
        ::CombineRgn(hRgn,pRgnGDI->GetRegion(),NULL,RGN_COPY);
        ::OffsetRgn(hRgn,m_ptOrg.x,m_ptOrg.y);
        ::SaveDC(m_hdc);
        ::ExtSelectClipRgn(m_hdc,hRgn,mode);
        ::DeleteObject(hRgn);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::PopClipRegion()
    {
        ::RestoreDC(m_hdc,-1);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::GetClipRegion( IRegion **ppRegion )
    {
        SRegion_GDI *pRgn=new SRegion_GDI(GetRenderFactory_GDI());
        ::GetClipRgn(m_hdc,pRgn->GetRegion());
        *ppRegion = pRgn;
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::GetClipBound(LPRECT prcBound)
    {
        ::GetClipBox(m_hdc,prcBound);
        return S_OK;
    }


    HRESULT SRenderTarget_GDI::BitBlt( LPCRECT pRcDest,IRenderTarget *pRTSour,int xSrc,int ySrc,DWORD dwRop/*=SRCCOPY*/)
    {
        HDC hdcSrc=pRTSour->GetDC(0);
        HDC hdcDst=GetDC(0);
        ::BitBlt(hdcDst,pRcDest->left,pRcDest->top,pRcDest->right-pRcDest->left,pRcDest->bottom-pRcDest->top,hdcSrc,xSrc,ySrc,dwRop);
        ReleaseDC(hdcDst);
        pRTSour->ReleaseDC(hdcSrc);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawText( LPCTSTR pszText,int cchLen,LPRECT pRc,UINT uFormat ,BYTE byAlpha)
    {
        COLORREF crOld=::SetTextColor(m_hdc,m_curColor.toCOLORREF()&0x00ffffff);
        ALPHAINFO ai;
        if(!(uFormat&DT_CALCRECT))
        {
            ::DrawText(m_hdc,pszText,cchLen,pRc,uFormat|DT_CALCRECT);
            CGdiAlpha::AlphaBackup(m_hdc,pRc,ai);
        }
        ::DrawText(m_hdc,pszText,cchLen,pRc,uFormat);
        if(!(uFormat&DT_CALCRECT))
        {
            CGdiAlpha::AlphaRestore(m_hdc,ai);
        }
        ::SetTextColor(m_hdc,crOld);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::MeasureText( LPCTSTR pszText,int cchLen, SIZE *psz )
    {
        ::GetTextExtentPoint32(m_hdc,pszText,cchLen,psz);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawRectangle(LPRECT pRect)
    {
        HGDIOBJ oldBr=::SelectObject(m_hdc,GetStockObject(NULL_BRUSH));
        ::Rectangle(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom);
        ::SelectObject(m_hdc,oldBr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillRectangle(LPRECT pRect)
    {
        HGDIOBJ oldPen=::SelectObject(m_hdc,GetStockObject(NULL_PEN));
        ::Rectangle(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom);
        ::SelectObject(m_hdc,oldPen);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawRoundRect( LPCRECT pRect,POINT pt )
    {
        HGDIOBJ oldBr=::SelectObject(m_hdc,GetStockObject(NULL_BRUSH));
        ::RoundRect(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt.x,pt.y);
        ::SelectObject(m_hdc,oldBr);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillRoundRect( LPCRECT pRect,POINT pt )
    {
        HGDIOBJ oldPen=::SelectObject(m_hdc,GetStockObject(NULL_PEN));
        ::RoundRect(m_hdc,pRect->left,pRect->top,pRect->right,pRect->bottom,pt.x,pt.y);
        ::SelectObject(m_hdc,oldPen);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawLines(LPPOINT pPt,size_t nCount)
    {
        ::Polyline(m_hdc,pPt,nCount);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::TextOut( int x, int y, LPCTSTR lpszString, int nCount,BYTE byAlpha )
    {
        COLORREF crOld=::SetTextColor(m_hdc,m_curColor.toCOLORREF()&0x00ffffff);
        SIZE sz;
        MeasureText(lpszString,nCount,&sz);
        RECT rc={x,y,x+sz.cx,y+sz.cy};
        ALPHAINFO ai;
        CGdiAlpha::AlphaBackup(m_hdc,&rc,ai);
        ::TextOut(m_hdc,x,y,lpszString,nCount);
        CGdiAlpha::AlphaRestore(m_hdc,ai);
        ::SetTextColor(m_hdc,crOld);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::DrawIconEx( int xLeft, int yTop, HICON hIcon, int cxWidth,int cyWidth,UINT diFlags )
    {
        BOOL bRet=::DrawIconEx(m_hdc,xLeft,yTop,hIcon,cxWidth,cyWidth,0,NULL,diFlags);
        return bRet?S_OK:S_FALSE;
    }

    HRESULT SRenderTarget_GDI::DrawBitmap(LPCRECT pRcDest,IBitmap *pBitmap,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/ )
    {
        SBitmap_GDI *pBmp = (SBitmap_GDI*)pBitmap;
        HBITMAP bmp=pBmp->GetBitmap();
        HDC hmemdc=CreateCompatibleDC(m_hdc);
        ::SelectObject(hmemdc,bmp);
        BLENDFUNCTION bf={ AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
        int nWid=pRcDest->right-pRcDest->left;
        int nHei=pRcDest->bottom-pRcDest->top;
        AlphaBlend(m_hdc,pRcDest->left,pRcDest->top,nWid,nHei,
                   hmemdc,xSrc,ySrc,nWid,nHei,bf);
        DeleteDC(hmemdc);
        
        return S_OK;
    }


    HRESULT SRenderTarget_GDI::DrawBitmapEx( LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,EXPEND_MODE expendMode, BYTE byAlpha/*=0xFF*/ )
    {
        if(expendMode == EM_NULL)
            return DrawBitmap(pRcDest,pBitmap,pRcSrc->left,pRcSrc->top,byAlpha);

        SBitmap_GDI *pBmp = (SBitmap_GDI*)pBitmap;
        HBITMAP bmp=pBmp->GetBitmap();
        HDC hmemdc=CreateCompatibleDC(m_hdc);
        ::SelectObject(hmemdc,bmp);

        BLENDFUNCTION bf={ AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA};
        if(expendMode == EM_STRETCH)
        {
            AlphaBlend(m_hdc,pRcDest->left,pRcDest->top,pRcDest->right-pRcDest->left,pRcDest->bottom-pRcDest->top,
                hmemdc,pRcSrc->left,pRcSrc->top,pRcSrc->right-pRcSrc->left,pRcSrc->bottom-pRcSrc->top,bf);
        }else
        {
            ::SaveDC(m_hdc);
            ::IntersectClipRect(m_hdc,pRcDest->left,pRcDest->top,pRcDest->right,pRcDest->bottom);
            int nWid=pRcSrc->right-pRcSrc->left;
            int nHei=pRcSrc->bottom-pRcSrc->top;
            for(int y=pRcDest->top ;y<pRcDest->bottom;y+=nHei)
            {
                for(int x=pRcDest->left; x<pRcDest->right; x+=nWid)
                {
                    AlphaBlend(m_hdc,x,y,nWid,nHei,
                        hmemdc,pRcSrc->left,pRcSrc->top,nWid,nHei,
                        bf);                    
                }
            }
            ::RestoreDC(m_hdc,-1);
        }
        DeleteDC(hmemdc);
        return S_OK;

    }


    HRESULT SRenderTarget_GDI::DrawBitmap9Patch( LPCRECT pRcDest,IBitmap *pBitmap,LPCRECT pRcSrc,LPCRECT pRcSourMargin,EXPEND_MODE expendMode,BYTE byAlpha/*=0xFF*/ )
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

    IRenderObj * SRenderTarget_GDI::GetCurrentObject( OBJTYPE uType )
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

    HRESULT SRenderTarget_GDI::SelectObject( IRenderObj *pObj,IRenderObj ** ppOldObj /*= NULL*/ )
    {
        CAutoRefPtr<IRenderObj> pRet;
        switch(pObj->ObjectType())
        {
        case OT_BITMAP: 
            pRet=m_curBmp;
            m_curBmp=(SBitmap_GDI*)pObj;
            ::SelectObject(m_hdc,m_curBmp->GetBitmap());
            break;
        case OT_PEN:
            pRet=m_curPen;
            m_curPen=(SPen_GDI*)pObj;
            ::SelectObject(m_hdc,m_curPen->GetPen());
            break;
        case OT_BRUSH:
            pRet=m_curBrush;
            m_curBrush=(SBrush_GDI*)pObj;
            ::SelectObject(m_hdc,m_curBrush->GetBrush());
            break;
        case OT_FONT:
            pRet=m_curFont;
            m_curFont=(SFont_GDI*)pObj;
            ::SelectObject(m_hdc,m_curFont->GetFont());
            break;
        }
        if(pRet && ppOldObj)
        {//由调用者调用Release释放该RenderObj
            pRet->AddRef();
            *ppOldObj = pRet;
        }
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::OffsetViewportOrg( int xOff, int yOff, LPPOINT lpPoint )
    {
        if(lpPoint)
        {
            *lpPoint=m_ptOrg;
        }
        m_ptOrg.x+=xOff;
        m_ptOrg.y+=yOff;
        ::SetViewportOrgEx(m_hdc,m_ptOrg.x,m_ptOrg.y,NULL);
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::GetViewportOrg( LPPOINT lpPoint )
    {
        if(lpPoint)
        {
            *lpPoint=m_ptOrg;
        }
        return S_OK;
    }

    HDC SRenderTarget_GDI::GetDC( UINT uFlag )
    {
        m_uGetDCFlag = uFlag;
        return m_hdc;
    }

    void SRenderTarget_GDI::ReleaseDC( HDC hdc )
    {
        if(hdc == m_hdc)
        {
            m_uGetDCFlag =0;
        }
    }

    HRESULT SRenderTarget_GDI::GradientFill( LPCRECT pRect,BOOL bVert,COLORREF crBegin,COLORREF crEnd,BYTE byAlpha/*=0xFF*/ )
    {
        if(byAlpha!=0xFF)
        {
            int nWid=pRect->right-pRect->left;
            int nHei=pRect->bottom-pRect->top;
            RECT rc={0,0,nWid,nHei};
            HBITMAP hbmp=CreateCompatibleBitmap(m_hdc,nWid,nHei);
            HDC hmemdc=CreateCompatibleDC(m_hdc);
            ::SelectObject(hmemdc,hbmp);
            GradientFillRect(hmemdc,&rc,crBegin,crEnd,bVert);
            BLENDFUNCTION bf={AC_SRC_OVER,0,byAlpha,AC_SRC_ALPHA };
            AlphaBlend(m_hdc,pRect->left,pRect->top,nWid,nHei,hmemdc,0,0,nWid,nHei,bf);
            DeleteDC(hmemdc);
            DeleteObject(hbmp);
        }else
        {
            GradientFillRect(m_hdc,pRect,crBegin,crEnd,bVert);
        }
        return S_OK;
    }

    HRESULT SRenderTarget_GDI::FillSolidRect( LPCRECT pRect,COLORREF cr )
    {
        HBRUSH hbr=::CreateSolidBrush(cr&0x00FFFFFF);
        ALPHAINFO ai;
        CGdiAlpha::AlphaBackup(m_hdc,pRect,ai);
        ::FillRect(m_hdc,pRect,hbr);
        CGdiAlpha::AlphaRestore(m_hdc,ai);
        ::DeleteObject(hbr);
        return S_OK;    
    }

}
