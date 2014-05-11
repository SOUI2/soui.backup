//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiImage
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "duiimage.h"

#pragma comment(lib,"gdiplus.lib")

namespace SOUI
{

static BOOL TransparentBlt2(
    HDC hdcDest,            // 目标DC
    int nXOriginDest,       // 目标X偏移
    int nYOriginDest,       // 目标Y偏移
    int nWidthDest,         // 目标宽度
    int nHeightDest,        // 目标高度
    HDC hdcSrc,             // 源DC
    int nXOriginSrc,        // 源X起点
    int nYOriginSrc,        // 源Y起点
    int nWidthSrc,          // 源宽度
    int nHeightSrc,         // 源高度
    UINT crTransparent      // 透明色,COLORREF类型
)
{
    HBITMAP hOldImageBMP = NULL, hImageBMP = NULL, hOldMaskBMP = NULL, hMaskBMP = NULL;
    HDC     hImageDC = NULL, hMaskDC = NULL;

    hImageBMP = ::CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);   // 创建兼容位图
    hMaskBMP = ::CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);           // 创建单色掩码位图
    hImageDC = ::CreateCompatibleDC(hdcDest);
    hMaskDC = ::CreateCompatibleDC(hdcDest);

    hOldImageBMP = (HBITMAP)::SelectObject(hImageDC, hImageBMP);
    hOldMaskBMP = (HBITMAP)::SelectObject(hMaskDC, hMaskBMP);

    // 将源DC中的位图拷贝到临时DC中
    if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
        ::BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
    else
        ::StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
                     hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);

    // 设置透明色
    ::SetBkColor(hImageDC, crTransparent);

    // 生成透明区域为白色，其它区域为黑色的掩码位图
    ::BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);

    // 生成透明区域为黑色，其它区域保持不变的位图
    ::SetBkColor(hImageDC, RGB(0,0,0));
    ::SetTextColor(hImageDC, RGB(255,255,255));
    ::BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

    // 透明部分保持屏幕不变，其它部分变成黑色
    COLORREF crBg = ::SetBkColor(hdcDest, RGB(255, 255, 255));
    COLORREF crText = ::SetTextColor(hdcDest, RGB(0, 0, 0));
    ::BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

    // "或"运算,生成最终效果
    ::BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCPAINT);

    // 清理、恢复
    ::SelectObject(hImageDC, hOldImageBMP);
    ::DeleteDC(hImageDC);
    ::SelectObject(hMaskDC, hOldMaskBMP);
    ::DeleteDC(hMaskDC);
    ::DeleteObject(hImageBMP);
    ::DeleteObject(hMaskBMP);

    ::SetBkColor(hdcDest, crBg);
    ::SetTextColor(hdcDest, crText);

    return TRUE;
}


CDuiBitmap::CDuiBitmap():m_hBitmap(NULL),m_crMask(CLR_INVALID)
{
}

CDuiBitmap::CDuiBitmap(HBITMAP hBitmap):m_hBitmap(hBitmap),m_crMask(CLR_INVALID)
{

}

CDuiBitmap::operator HBITMAP()  const
{
    return m_hBitmap;
}

void CDuiBitmap::Clear()
{
    if(m_hBitmap) DeleteObject(m_hBitmap);
    m_hBitmap=NULL;
}

BOOL CDuiBitmap::IsEmpty() const
{
    return m_hBitmap==NULL;
}
BOOL CDuiBitmap::GetImageSize(SIZE & sz)
{
    if(IsEmpty()) return FALSE;
    BITMAP bm;
    GetObject(m_hBitmap,sizeof(bm),&bm);
    sz.cx=bm.bmWidth;
    sz.cy=bm.bmHeight;
    return TRUE;
}

int CDuiBitmap::GetWidth()
{
    if(IsEmpty()) return 0;
    BITMAP bm;
    GetObject(m_hBitmap,sizeof(bm),&bm);
    return bm.bmWidth;
}
int CDuiBitmap::GetHeight()
{
    if(IsEmpty()) return 0;
    BITMAP bm;
    GetObject(m_hBitmap,sizeof(bm),&bm);
    return bm.bmHeight;
}

BOOL CDuiBitmap::BitBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/)
{
    if(IsEmpty()) return FALSE;
    BOOL bRet=FALSE;
    HDC hMemdc=CreateCompatibleDC(hdc);
    HGDIOBJ hOldBmp=SelectObject(hMemdc,m_hBitmap);
    if(m_crMask!=CLR_INVALID)
    {
        bRet=TransparentBlt2(hdc,x,y,nWid,nHei,hMemdc,xSrc,ySrc,nWid,nHei,m_crMask);
    }
    else
    {
        bRet=::BitBlt(hdc,x,y,nWid,nHei,hMemdc,xSrc,ySrc,SRCCOPY);
    }
    SelectObject(hMemdc,hOldBmp);
    DeleteDC(hMemdc);
    return bRet;
}

BOOL CDuiBitmap::StretchBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha/*=0xFF*/)
{
    if(IsEmpty()) return FALSE;
    if(nWid==nWidSrc && nHei==nHeiSrc) return BitBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,byAlpha);
    BOOL bRet=FALSE;
    HDC hMemdc=CreateCompatibleDC(hdc);
    HGDIOBJ hOldBmp=SelectObject(hMemdc,m_hBitmap);
    if(m_crMask!=CLR_INVALID)
    {
        bRet=TransparentBlt2(hdc,x,y,nWid,nHei,hMemdc,xSrc,ySrc,nWidSrc,nHeiSrc,m_crMask);
    }
    else
    {
        bRet=::StretchBlt(hdc,x,y,nWid,nHei,hMemdc,xSrc,ySrc,nWidSrc,nHeiSrc,SRCCOPY);
    }
    SelectObject(hMemdc,hOldBmp);
    DeleteDC(hMemdc);
    return bRet;
}

BOOL CDuiBitmap::TileBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha/*=0xFF*/)
{
    if(IsEmpty()) return FALSE;
    if(nWid==nWidSrc && nHei==nHeiSrc) return BitBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,byAlpha);
    BOOL bRet=FALSE;
    HDC hMemdc=CreateCompatibleDC(hdc);
    HGDIOBJ hOldBmp=SelectObject(hMemdc,m_hBitmap);

    BOOL bTransDraw=m_crMask!=CLR_INVALID;

    int yt=y;
    while(yt<y+nHei)
    {
        int xt=x;
        int nHeiT=min(nHeiSrc,y+nHei-yt);
        while(xt<x+nWid)
        {
            int nWidT=min(nWidSrc,x+nWid-xt);
            if(bTransDraw)
            {
                bRet=TransparentBlt2(hdc,xt,yt,nWidT,nHeiT,hMemdc,xSrc,ySrc,nWidT,nHeiT,m_crMask);
            }
            else
            {
                bRet=::BitBlt(hdc,xt,yt,nWidT,nHeiT,hMemdc,xSrc,ySrc,SRCCOPY);
            }
            xt+=nWidT;
        }
        yt+=nHeiT;
    }
    SelectObject(hMemdc,hOldBmp);
    DeleteDC(hMemdc);
    return TRUE;
}


void CDuiBitmap::SetAttributes( pugi::xml_node xmlNode )
{
    if(!xmlNode) return;
    const char * pszMask=xmlNode.attribute("mask").value();
    if(pszMask)
    {
        int r=0,g=0,b=0;
        sscanf(pszMask,"%02x%02x%02x",&r,&g,&b);
        m_crMask=RGB(r,g,b);
    }
}

BOOL CDuiBitmap::LoadFromResource( HINSTANCE hInst,LPCTSTR pszType,LPCTSTR pszName )
{
    assert(m_hBitmap==NULL);
    m_hBitmap = (HBITMAP)::LoadImage(hInst, pszName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    return m_hBitmap!=NULL;
}

BOOL CDuiBitmap::LoadFromFile( LPCTSTR pszPath )
{
    assert(m_hBitmap==NULL);
    m_hBitmap = (HBITMAP)::LoadImage(0, pszPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    return m_hBitmap!=NULL;
}

BOOL CDuiBitmap::LoadFromMemory( LPVOID pBuf,DWORD dwSize )
{
    assert(m_hBitmap==NULL);

    HDC hDC = GetDC(NULL);
    //读取位图头
    BITMAPFILEHEADER *pBmpFileHeader=(BITMAPFILEHEADER *)pBuf;
    //检测位图头
    if (pBmpFileHeader->bfType != ((WORD) ('M'<<8)|'B'))
    {
        return NULL;
    }
    //判断位图长度
    if (pBmpFileHeader->bfSize > (UINT)dwSize)
    {
        return NULL;
    }
    LPBITMAPINFO lpBitmap=(LPBITMAPINFO)(pBmpFileHeader+1);
    LPVOID lpBits=(LPBYTE)pBuf+pBmpFileHeader->bfOffBits;
    m_hBitmap= CreateDIBitmap(hDC,&lpBitmap->bmiHeader,CBM_INIT,lpBits,lpBitmap,DIB_RGB_COLORS);
    ReleaseDC(NULL,hDC);
    return m_hBitmap!=NULL;
}


//////////////////////////////////////////////////////////////////////////
// CDuiImgX

CDuiImgX::CDuiImgX():m_pImg(NULL)
{

}

CDuiImgX::CDuiImgX(Gdiplus::Image * pImg):m_pImg(pImg)
{
}

void CDuiImgX::Clear()
{
    if(m_pImg) delete m_pImg;
    m_pImg=NULL;
}
BOOL CDuiImgX::IsEmpty() const
{
    return m_pImg==NULL;
}
BOOL CDuiImgX::GetImageSize(SIZE &sz)
{
    if(IsEmpty()) return FALSE;
    sz.cx=m_pImg->GetWidth();
    sz.cy=m_pImg->GetHeight();
    return TRUE;
}

int CDuiImgX::GetWidth()
{
    if(IsEmpty()) return 0;
    return m_pImg->GetWidth();
}

int CDuiImgX::GetHeight()
{
    if(IsEmpty()) return 0;
    return m_pImg->GetHeight();
}

BOOL CDuiImgX::BitBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,BYTE byAlpha/*=0xFF*/)
{
    if(IsEmpty()) return FALSE;
	if(nWid==0 || nHei==0) return TRUE;

	Gdiplus::Graphics graphics(hdc);
	Gdiplus::ImageAttributes ImgAtt;
	ImgAtt.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
	if(byAlpha!=0xFF)
	{
		Gdiplus::ColorMatrix ClrMatrix =
		{
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, float(byAlpha)/0xFF, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		ImgAtt.SetColorMatrix(&ClrMatrix);
	}

	return S_OK==graphics.DrawImage(m_pImg,Gdiplus::Rect(x,y,nWid,nHei),xSrc,ySrc,nWid,nHei,Gdiplus::UnitPixel,&ImgAtt);
}

BOOL CDuiImgX::StretchBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha/*=0xFF*/)
{
    if(IsEmpty()) return FALSE;
    Gdiplus::Graphics graphics(hdc);
	Gdiplus::ImageAttributes ImgAtt;
	ImgAtt.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    if(byAlpha!=0xFF)
    {
        Gdiplus::ColorMatrix ClrMatrix =
        {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, float(byAlpha)/0xFF, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        ImgAtt.SetColorMatrix(&ClrMatrix);
    }

    return S_OK==graphics.DrawImage(m_pImg,Gdiplus::Rect(x,y,nWid,nHei),xSrc,ySrc,nWidSrc,nHeiSrc,Gdiplus::UnitPixel,&ImgAtt);
}

BOOL CDuiImgX::TileBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha/*=0xFF*/)
{
    if(IsEmpty()) return FALSE;
    if(nWid==nWidSrc && nHei==nHeiSrc) return BitBlt(hdc,x,y,nWid,nHei,xSrc,ySrc,byAlpha);
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::ImageAttributes *pImgAttr=NULL;
    if(byAlpha!=0xFF)
    {
        Gdiplus::ColorMatrix ClrMatrix =
        {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, float(byAlpha)/0xFF, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        pImgAttr=new Gdiplus::ImageAttributes;
        pImgAttr->SetColorMatrix(&ClrMatrix);
    }

	Gdiplus::TextureBrush br(m_pImg,Gdiplus::WrapModeTile,Gdiplus::Rect(xSrc,ySrc,nWidSrc,nHeiSrc));
	graphics.FillRectangle(&br,Gdiplus::Rect(x,y,nWid,nHei));
    if(byAlpha!=0xFF)
    {
        delete pImgAttr;
    }
    return TRUE;
}

BOOL CDuiImgX::LoadFromResource( HINSTANCE hInst,LPCTSTR pszType,LPCTSTR pszName )
{
    assert(m_pImg==NULL);

    HRSRC hRsrc = ::FindResource(hInst, pszName, pszType);

    if (NULL == hRsrc)
        return NULL;

    DWORD dwSize=::SizeofResource(hInst, hRsrc);
    if(dwSize==0) return NULL;


    HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
    if (NULL == hGlobal)
    {
        return NULL;
    }

    LPVOID pBuffer = ::LockResource(hGlobal);
    if (NULL == pBuffer)
    {
        ::FreeResource(hGlobal);
        return NULL;
    }

    HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
    BYTE* pMem = (BYTE*)::GlobalLock(hMem);
    memcpy(pMem,pBuffer,dwSize);

    ::FreeResource(hGlobal);

    IStream* pStm = NULL;
    ::CreateStreamOnHGlobal(hMem, TRUE, &pStm);

    m_pImg = Gdiplus::Image::FromStream(pStm);

    pStm->Release();
    ::GlobalUnlock(hMem);

    return m_pImg!=NULL;
}

BOOL CDuiImgX::LoadFromFile( LPCTSTR pszPath )
{
    assert(m_pImg==NULL);
	CDuiStringW strPath=DUI_CT2W(pszPath);
    m_pImg=Gdiplus::Image::FromFile(strPath);
    return m_pImg!=NULL;
}

BOOL CDuiImgX::LoadFromMemory( LPVOID pBuf,DWORD dwSize )
{
    assert(m_pImg==NULL);

    HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
    BYTE* pMem = (BYTE*)::GlobalLock(hMem);

    memcpy(pMem, pBuf, dwSize);

    IStream* pStm = NULL;
    ::CreateStreamOnHGlobal(hMem, TRUE, &pStm);

    m_pImg = Gdiplus::Image::FromStream(pStm);

    pStm->Release();
    ::GlobalUnlock(hMem);
    // 	GlobalFree(hMem);
    return m_pImg!=NULL;
}

ULONG_PTR CDuiImgX::s_gdiplusToken = 0;

BOOL CDuiImgX::GdiplusStartup()
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::Status st=Gdiplus::GdiplusStartup(&s_gdiplusToken, &gdiplusStartupInput, NULL);
    return st==0;
}

void CDuiImgX::GdiplusShutdown()
{
    Gdiplus::GdiplusShutdown(s_gdiplusToken);
}

}//namespace SOUI