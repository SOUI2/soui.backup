#pragma once

#pragma once

#include "render-api.h"

#include <color.h>
#include <unknown/obj-ref-impl.hpp>

#include <string\tstring.h>
#include <string\strcpcvt.h>


namespace SOUI
{
    //实现一些和特定系统相关的接口
    struct IRenderFactory_GDI : public IRenderFactory
    {
        virtual IImgDecoderFactory * GetImgDecoderFactory()=0;
    };


    //////////////////////////////////////////////////////////////////////////
    // SRenderFactory_GDI
    class SRenderFactory_GDI : public TObjRefImpl<IRenderFactory_GDI>
    {
    public:
        SRenderFactory_GDI(IImgDecoderFactory *pImgDecoderFactory):m_imgDecoderFactory(pImgDecoderFactory)
        {
        }

        ~SRenderFactory_GDI()
        {
        }

        virtual BOOL CreateRenderTarget(IRenderTarget ** ppRenderTarget,int nWid,int nHei);
        virtual BOOL CreateFont(IFont ** ppFont , const LOGFONT &lf);
        virtual BOOL CreateBitmap(IBitmap ** ppBitmap);
        virtual BOOL CreateRegion(IRegion **ppRgn);

        IImgDecoderFactory * GetImgDecoderFactory(){return m_imgDecoderFactory;}
    protected:
        CAutoRefPtr<IImgDecoderFactory> m_imgDecoderFactory;
    };


    //////////////////////////////////////////////////////////////////////////
    // TSkiaRenderObjImpl
    template<class T>
    class TSkiaRenderObjImpl : public TObjRefImpl<T>
    {
    public:
        TSkiaRenderObjImpl(IRenderFactory_GDI * pRenderFac):m_pRenderFactory(pRenderFac)
        {

        }

        virtual IRenderFactory * GetRenderFactory() const
        {
            return m_pRenderFactory;
        }

        virtual IRenderFactory_GDI * GetRenderFactory_GDI() const
        {
            return m_pRenderFactory;
        }
    protected:
        IRenderFactory_GDI *m_pRenderFactory;
    };


    //////////////////////////////////////////////////////////////////////////
    // SPen_GDI
    class SPen_GDI : public TSkiaRenderObjImpl<IPen>
    {
    public:
        SPen_GDI(IRenderFactory_GDI * pRenderFac,int iStyle=PS_SOLID,COLORREF cr=0xFF000000,int cWidth=1)
            :TSkiaRenderObjImpl<IPen>(pRenderFac)
            ,m_nWidth(cWidth),m_style(iStyle),m_cr(cr)
            ,m_hPen(NULL)
        {
            m_hPen = ::CreatePen(iStyle,cWidth,cr);
        }
        ~SPen_GDI()
        {
            DeleteObject(m_hPen);
        }

        int GetWidth(){return m_nWidth;}

        int GetStyle(){return m_style;}

        void SetWidth(int nWid) {m_nWidth=nWid;}

        COLORREF GetColor(){return m_cr;}

        void SetColor(COLORREF cr){m_cr = cr;}
        
        HPEN GetPen(){return m_hPen;}
    protected:

        int		m_style;
        int		m_nWidth;
        COLORREF	m_cr;
        
        HPEN    m_hPen;
    };

    //////////////////////////////////////////////////////////////////////////
    // SFont_GDI
    class SFont_GDI: public TSkiaRenderObjImpl<IFont>
    {
    public:
        SFont_GDI(IRenderFactory_GDI * pRenderFac,const LOGFONT * plf)
            :TSkiaRenderObjImpl<IFont>(pRenderFac),m_hFont(NULL)
        {
            memcpy(&m_lf,plf,sizeof(LOGFONT));
            m_hFont=CreateFontIndirect(&m_lf);
        }
        ~SFont_GDI()
        {
            DeleteObject(m_hFont);
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
        
        HFONT GetFont(){return m_hFont;}
    protected:
        LOGFONT     m_lf;
        HFONT       m_hFont;
    };

    class SBrush_GDI : public TSkiaRenderObjImpl<IBrush>
    {
    public:
        static SBrush_GDI * CreateSolidBrush(IRenderFactory_GDI * pRenderFac,COLORREF cr){
            return new SBrush_GDI(pRenderFac,cr);
        }

        static SBrush_GDI * CreateBitmapBrush(IRenderFactory_GDI * pRenderFac,HBITMAP hBmp)
        {
            return new SBrush_GDI(pRenderFac,hBmp);
        }


        BOOL IsBitmap(){return m_fBmp;}
        
        HBRUSH GetBrush(){return m_hBrush;}
    protected:
        SBrush_GDI(IRenderFactory_GDI * pRenderFac,COLORREF cr)
            :TSkiaRenderObjImpl<IBrush>(pRenderFac),m_fBmp(FALSE)
        {
            m_hBrush = ::CreateSolidBrush(cr);
        }
        SBrush_GDI(IRenderFactory_GDI * pRenderFac,HBITMAP hBmp)
            :TSkiaRenderObjImpl<IBrush>(pRenderFac),m_fBmp(TRUE)
        {
            m_hBrush = ::CreatePatternBrush(hBmp);
        }
        ~SBrush_GDI()
        {
            DeleteObject(m_hBrush);
        }
        HBRUSH   m_hBrush;
        BOOL	 m_fBmp;
    };

    //////////////////////////////////////////////////////////////////////////
    // SBitmap_GDI
    class SImgDecoder_WIC;
    class SBitmap_GDI : public TSkiaRenderObjImpl<IBitmap>
    {
    public:
        SBitmap_GDI(IRenderFactory_GDI *pRenderFac)
            :TSkiaRenderObjImpl<IBitmap>(pRenderFac),m_hBmp(0)
        {
            m_sz.cx=m_sz.cy=0;
        }
        virtual ~SBitmap_GDI()
        {
            if(m_hBmp) DeleteObject(m_hBmp);
        }
        virtual HRESULT Init(int nWid,int nHei);
        virtual HRESULT LoadFromFile(LPCTSTR pszFileName,LPCTSTR pszType);
        virtual HRESULT LoadFromMemory(LPBYTE pBuf,size_t szLen,LPCTSTR pszType);

        virtual UINT Width();
        virtual UINT Height();
        virtual SIZE Size();

        HBITMAP  GetBitmap(){return m_hBmp;}
    protected:
        HBITMAP CreateGDIBitmap(int nWid,int nHei,void ** ppBits);

        HRESULT ImgFromDecoder(IImgDecoder *imgDecoder);
        SIZE        m_sz;
        HBITMAP     m_hBmp;     //标准的32位位图，和m_bitmap共享内存
    };

    //////////////////////////////////////////////////////////////////////////
    //	SRegion_GDI
    class SRegion_GDI: public TSkiaRenderObjImpl<IRegion>
    {
    public:
        SRegion_GDI(IRenderFactory_GDI *pRenderFac);
        ~SRegion_GDI(){
            DeleteObject(m_hRgn);
        }

        virtual void CombineRect(LPCRECT lprect,int nCombineMode);
        virtual BOOL PtInRegion(POINT pt);
        virtual BOOL RectInRegion(LPCRECT lprect);
        virtual void GetRgnBox(LPRECT lprect);
        virtual BOOL IsEmpty();
        virtual void Offset(POINT pt);
        virtual void Clear();

        HRGN GetRegion() const;

        void SetRegion(const HRGN rgn);
    protected:
        HRGN    m_hRgn;
    };


    //////////////////////////////////////////////////////////////////////////
    //	SRenderTarget_GDI
    //////////////////////////////////////////////////////////////////////////
    class SRenderTarget_GDI: public TSkiaRenderObjImpl<IRenderTarget>
    {
    public:
        SRenderTarget_GDI(IRenderFactory_GDI* pRenderFactory,int nWid,int nHei);
        ~SRenderTarget_GDI();

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

        virtual HDC GetDC(UINT uFlag=0);
        virtual void ReleaseDC(HDC hdc);

    protected:
        HDC               m_hdc;
        SColor            m_curColor;
        CAutoRefPtr<SBitmap_GDI> m_curBmp;
        CAutoRefPtr<SPen_GDI> m_curPen;
        CAutoRefPtr<SBrush_GDI> m_curBrush;
        CAutoRefPtr<SFont_GDI> m_curFont;
        POINT               m_ptOrg;

        HDC m_hBindDC;
        RECT m_rcBind;

        UINT m_uGetDCFlag;
    };
}