#include "StdAfx.h"
#include "souistd.h"
#include <GdiPlus.h>
#include "SAntialiasSkin.h"
#include <helper/SplitString.h>
#include <interface/imgdecoder-i.h>
#include <interface/render-i.h>

#include "utils.h"
using namespace Gdiplus;

#pragma comment(lib,"gdiplus.lib")

namespace SOUI
{

    SAntialiasSkin::SAntialiasSkin() :
        _isRound(FALSE),
        _frameCount(0),
        _currentFrame(0),
        _autoZoom(TRUE),
        _pImage(NULL),
        _leftTopRadius(0),
        _rightTopRadius(0),
        _rightBottomRadius(0),
        _leftBottomRadius(0)
    {
        _maxSize.cx = _maxSize.cy = 150;
        _skinSize.cx = _skinSize.cy = 0;
    }

    SAntialiasSkin::~SAntialiasSkin()
    {
        FreeImage();
    }

    void SAntialiasSkin::FreeImage()
    {
        FrameMap::iterator it = _frames.begin();
        for (; it != _frames.end(); ++it)
        {
            if (it->second.Frame)
            {
                delete it->second.Frame;
            }
        }
        _frames.clear();

        if (_pImage)
        {
            delete _pImage;
            _pImage = NULL;
        }
    }

    LRESULT SAntialiasSkin::OnAttrSrc(const SStringW &strValue, BOOL bLoading)
    {
        SStringTList strLst;
        size_t nSegs = ParseResID(S_CW2T(strValue), strLst);
        LPBYTE pBuf = NULL;
        size_t szBuf = 0;

        if (nSegs == 2)
        {
            szBuf = GETRESPROVIDER->GetRawBufferSize(strLst[0], strLst[1]);
            if (szBuf)
            {
                pBuf = new BYTE[szBuf];
                GETRESPROVIDER->GetRawBuffer(strLst[0], strLst[1], pBuf, szBuf);
            }
        }
        if (pBuf)
        {
            LoadFromMemory(pBuf, szBuf);
            delete[]pBuf;
        }

        return S_OK;
    }

    BOOL SAntialiasSkin::RoundBitmap(Gdiplus::Bitmap*& pBitmap)
    {
        Gdiplus::Bitmap *pSrc = pBitmap;
        UINT iWidth = pSrc->GetWidth();
        UINT iHeight = pSrc->GetHeight();

        pBitmap = new Gdiplus::Bitmap(iWidth, iHeight, PixelFormat32bppARGB);
        if (pBitmap->GetLastStatus() != Ok)
        {
            delete pSrc;
            return FALSE;
        }

        Gdiplus::Graphics *pGraphics = Gdiplus::Graphics::FromImage(pBitmap);
        if (pGraphics->GetLastStatus() != Ok)
        {
            delete pSrc;
            delete pGraphics;
            return FALSE;
        }

        int nOffset = (int)((double)iWidth * 0.04);
        Gdiplus::TextureBrush brush(pSrc);
        RectF fillRect(0.0, 0.0, (REAL)pSrc->GetWidth() - nOffset, (REAL)pSrc->GetHeight() - nOffset);
        pGraphics->SetSmoothingMode(SmoothingModeAntiAlias);
        pGraphics->SetCompositingQuality(CompositingQualityHighQuality);
        pGraphics->FillEllipse(&brush, fillRect);

        delete pSrc;
        delete pGraphics;

        return TRUE;
    }

    BOOL SAntialiasSkin::SetBitmapRoundCorner(Gdiplus::Bitmap*& pBitmap)
    {
        if (_leftTopRadius == 0 &&
            _rightTopRadius == 0 &&
            _rightBottomRadius == 0 &&
            _leftBottomRadius == 0)
        {
            // 不需要设置圆角
            return TRUE;
        }

        Gdiplus::Bitmap *pSrc = pBitmap;
        UINT width = pSrc->GetWidth();
        UINT height = pSrc->GetHeight();

        pBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
        if (pBitmap->GetLastStatus() != Ok)
        {
            delete pSrc;
            return FALSE;
        }

        Gdiplus::Graphics *pGraphics = Gdiplus::Graphics::FromImage(pBitmap);
        if (pGraphics->GetLastStatus() != Ok)
        {
            delete pSrc;
            delete pGraphics;
            return FALSE;
        }

        Gdiplus::TextureBrush brush(pSrc);
        pGraphics->SetSmoothingMode(SmoothingModeAntiAlias);
        pGraphics->SetCompositingQuality(CompositingQualityHighQuality);

        int radius = 0;
        GraphicsPath gp;

        radius = _leftTopRadius == 0 ? 1 : _leftTopRadius;
        gp.AddArc(0, 0, radius, radius, 180, 90);

        radius = _rightTopRadius == 0 ? 1 : _rightTopRadius;
        gp.AddArc(width - radius, 0, radius, radius, 270, 90);

        radius = _rightBottomRadius == 0 ? 1 : _rightBottomRadius;
        gp.AddArc(width - radius, height - radius, radius, radius, 0, 90);

        radius = _leftBottomRadius == 0 ? 1 : _leftBottomRadius;
        gp.AddArc(0, height - radius, radius, radius, 90, 90);

        pGraphics->FillPath(&brush, &gp);

        delete pSrc;
        delete pGraphics;

        return TRUE;
    }

    BOOL SAntialiasSkin::ResizeBitmap(Gdiplus::Bitmap*& pBitmap, int width, int height)
    {
        Gdiplus::Bitmap *pSrc = pBitmap;

        pBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
        if (pBitmap->GetLastStatus() != Ok)
        {
            delete pSrc;
            return FALSE;
        }

        Gdiplus::Graphics *pGraphics = Gdiplus::Graphics::FromImage(pBitmap);
        if (pGraphics->GetLastStatus() != Ok)
        {
            delete pSrc;
            delete pGraphics;
            return FALSE;
        }

        pGraphics->Clear(Gdiplus::Color(0));
        pGraphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        pGraphics->DrawImage(pSrc, 0, 0, width, height);

        delete pSrc;
        delete pGraphics;

        return TRUE;
    }

    BOOL SAntialiasSkin::LoadFromFile(LPCTSTR pszFileName)
    {
        Gdiplus::Bitmap* pImage = Gdiplus::Bitmap::FromFile(pszFileName);
        if (pImage->GetLastStatus() != Gdiplus::Ok)
        {
            return FALSE;
        }

        FreeImage();
        LoadFromGdipImage(pImage);

        return TRUE;
    }

    int  SAntialiasSkin::LoadFromHandle(HBITMAP hBmp)
    {
        Gdiplus::Bitmap* pImage = Gdiplus::Bitmap::FromHBITMAP(hBmp, NULL);
        if (pImage->GetLastStatus() != Gdiplus::Ok)
        {
            return FALSE;
        }

        FreeImage();
        LoadFromGdipImage(pImage);

        return TRUE;
    }

    int SAntialiasSkin::LoadFromIcon(HICON hIcon)
    {
        Gdiplus::Bitmap* pImage = Gdiplus::Bitmap::FromHICON(hIcon);
        if (pImage->GetLastStatus() != Gdiplus::Ok)
        {
            return FALSE;
        }

        FreeImage();
        LoadFromGdipImage(pImage);

        return TRUE;
    }

    int SAntialiasSkin::LoadFromMemory(LPVOID pBuf, size_t dwSize)
    {

        HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
        BYTE* pMem = (BYTE*)::GlobalLock(hMem);

        memcpy(pMem, pBuf, dwSize);

        IStream* pStm = NULL;
        ::CreateStreamOnHGlobal(hMem, TRUE, &pStm);
        Gdiplus::Bitmap* pImage = Gdiplus::Bitmap::FromStream(pStm);
        if (!pImage || pImage->GetLastStatus() != Gdiplus::Ok)
        {
            pStm->Release();
            ::GlobalUnlock(hMem);
            return 0;
        }

        FreeImage();
        LoadFromGdipImage(pImage);

        return _frameCount;
    }

    int SAntialiasSkin::LoadFromGdipImage(Gdiplus::Bitmap*& pImage)
    {
        RotateImage(pImage);
        CalcSkinSize(pImage);
        LoadFrameCount(pImage);

        if (_frameCount == 1 && _isRound)
        {
            RoundBitmap(pImage);
        }

        LoadFrameInfos(pImage);

        return _frameCount;
    }

    void SAntialiasSkin::SetMaxSize(CSize size)
    {
        _maxSize = size;
    }

    void SAntialiasSkin::SetRound(BOOL round)
    {
        _isRound = round;
    }

    void SAntialiasSkin::SetRoundCorner(int leftTop, int rightTop, int rightBottom, int leftBottom)
    {
        //_leftTopRadius = leftTop * 10 * 2;
        //_rightTopRadius = rightTop * 10 * 2;
        //_rightBottomRadius = rightBottom * 10 * 2;
        //_leftBottomRadius = leftBottom * 10 * 2;

        _leftTopRadius = leftTop * 2;
        _rightTopRadius = rightTop * 2;
        _rightBottomRadius = rightBottom * 2;
        _leftBottomRadius = leftBottom * 2;
    }

    void SAntialiasSkin::SetAutoZoom(BOOL autoZoom)
    {
        _autoZoom = autoZoom;
    }

    void SAntialiasSkin::Rotate(int type)
    {
        if (_frameCount == 1)
        {
            _frames.begin()->second.Frame->RotateFlip((Gdiplus::RotateFlipType)type);
            CalcSkinSize(_frames.begin()->second.Frame);
        }
    }

    SAntialiasSkin* SAntialiasSkin::Clone()
    {
        SAntialiasSkin* pSkin = new SAntialiasSkin();

        pSkin->SetRound(_isRound);
        pSkin->SetMaxSize(_maxSize);
        pSkin->_skinSize = _skinSize;
        pSkin->_currentFrame = 0;
        pSkin->_frameCount = _frameCount;

        FrameMap::iterator it = _frames.begin();
        for (; it != _frames.end(); ++it)
        {
            FrameInfo info;
            info.Delay = it->second.Delay;

            if (it->second.Frame)
            {
                info.Frame = it->second.Frame->Clone(
                    0, 0,
                    it->second.Frame->GetWidth(),
                    it->second.Frame->GetHeight(),
                    PixelFormat32bppARGB);
            }
            else if (_pImage)
            {
                info.Frame = new Gdiplus::Bitmap(_skinSize.cx, _skinSize.cy, PixelFormat32bppPARGB);
                Gdiplus::Graphics * g = Gdiplus::Graphics::FromImage(info.Frame);
                _pImage->SelectActiveFrame(&FrameDimensionTime, it->first);
                g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
                g->DrawImage(_pImage, 0, 0, _pImage->GetWidth(), _pImage->GetHeight());
                delete g;
            }

            pSkin->_frames[it->first] = info;
        }
        return pSkin;
    }

    int SAntialiasSkin::GetStates()
    {
        return _frameCount;
    }

    SIZE SAntialiasSkin::GetSkinSize()
    {
        return _skinSize;
    }

    int SAntialiasSkin::GetFrameCount()
    {
        return _frameCount;
    }

    long SAntialiasSkin::GetFrameDelay(int frame/*= -1*/)
    {
        if (frame < 0)
        {
            frame = _currentFrame;
        }

        if (_frameCount > 1 && frame >= 0 && frame < _frameCount)
        {
            return _frames[_currentFrame].Delay;
        }

        return -1;
    }

    void SAntialiasSkin::ActiveNextFrame()
    {
        if (_frameCount > 1)
        {
            _currentFrame++;
            if (_currentFrame == _frameCount)
            {
                _currentFrame = 0;
            }

            SelectActiveFrame(_currentFrame);
        }
    }

    /**
     * SOUI::SAntialiasSkin::ReleaseImageWhileBuffReady:
     *   如果已经把_pImage的图片全都绘制到了缓存中，就可以释放_pImage了
     *
     * Parameters:
     *
     * Return Value:
     *
     * Remarks:
     *
     */
    void SAntialiasSkin::ReleaseImageWhileBuffReady()
    {
        if (!_pImage)
        {
            return;
        }

        size_t i = 0;
        for (; i < _frames.size(); ++i)
        {
            if (_frames[i].Frame == NULL)
            {
                break;
            }
        }

        if (i == _frames.size())
        {
            delete _pImage;
            _pImage = NULL;
        }
    }

    Gdiplus::Bitmap* SAntialiasSkin::SelectActiveFrame(int frame)
    {
        if (_frameCount > 1 && frame < _frameCount)
        {
            _currentFrame = frame;
        }

        FrameInfo& currentFrame = _frames[_currentFrame];
        if (currentFrame.Frame != NULL)
        {
            ReleaseImageWhileBuffReady();
            return currentFrame.Frame;
        }

        if (!_pImage)
        {
            return NULL;
        }

        //
        // 把_pImage的当前帧画到缓存里
        //
        currentFrame.Frame = new Gdiplus::Bitmap(_skinSize.cx, _skinSize.cy, PixelFormat32bppPARGB);
        Gdiplus::Graphics * g = Gdiplus::Graphics::FromImage(currentFrame.Frame);
        _pImage->SelectActiveFrame(&FrameDimensionTime, _currentFrame);
        g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g->DrawImage(_pImage, 0, 0, currentFrame.Frame->GetWidth(), currentFrame.Frame->GetHeight());
        delete g;

        //
        // 尝试释放_pImage
        //
        ReleaseImageWhileBuffReady();

        return currentFrame.Frame;
    }

    void SAntialiasSkin::AddFrame(LPCWSTR pszFileName, int delay)
    {
        Bitmap* pImage = Bitmap::FromFile(pszFileName);
        if (!pImage || pImage->GetLastStatus() != Ok)
        {
            return;
        }

        CalcSkinSize(pImage);

        FrameInfo frame;

        //
        // 分配图片内存
        // 把图片选进Graphics
        // 把原图缩放到每一帧图片里
        //
        frame.Frame = new Gdiplus::Bitmap(_skinSize.cx, _skinSize.cy, PixelFormat32bppARGB);
        if (frame.Frame->GetLastStatus() != Ok)
        {
            delete frame.Frame;
            return;
        }

        Gdiplus::Graphics *pGraphics = Gdiplus::Graphics::FromImage(frame.Frame);
        if (pGraphics->GetLastStatus() != Ok)
        {
            delete frame.Frame;
            delete pGraphics;
            return;
        }

        //pGraphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        pGraphics->DrawImage(pImage, 0, 0, frame.Frame->GetWidth(), frame.Frame->GetHeight());

        delete pGraphics;
        delete pImage;

        _frames[_frameCount++] = frame;
    }

    int SAntialiasSkin::GetInterpolationMode(float fScaling)
    {
        if (fScaling > 0.9f)
        {
            return InterpolationModeDefault;
        }

        if (_skinSize.cx < 800 && _skinSize.cy < 800)
        {
            return InterpolationModeHighQuality;
        }

        if (_skinSize.cx < 3000 && _skinSize.cy < 2000)
        {
            return InterpolationModeDefault;
        }

        // 最低质量
        return InterpolationModeNearestNeighbor;
    }

    void SAntialiasSkin::_Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha)
    {
        Gdiplus::Bitmap* pFrame = SelectActiveFrame(dwState);
        if (!pFrame)
        {
            return;
        }

        CRect rc = *rcDraw;
        Gdiplus::Rect rcDest(rc.left, rc.top, rc.Width(), rc.Height());

        // 根据缩放比例选择合适的画图模式，以提高绘图速度
        SIZE drawSize = { rcDest.Width, rcDest.Height };
        float fScaling = GetZoomRatio(_skinSize, drawSize);

        HDC dc = pRT->GetDC();
        Gdiplus::Graphics * g = new Gdiplus::Graphics(dc);
        g->SetInterpolationMode((InterpolationMode)GetInterpolationMode(fScaling));
        g->DrawImage(pFrame, rcDest, 0, 0, pFrame->GetWidth(), pFrame->GetHeight(), Gdiplus::UnitPixel);
        delete g;
        pRT->ReleaseDC(dc);
    }

    void SAntialiasSkin::CalcSkinSize(Gdiplus::Bitmap * pImage)
    {
        _skinSize.cx = pImage->GetWidth();
        _skinSize.cy = pImage->GetHeight();

        if (_autoZoom)
        {
            double fRatio = GetZoomRatio(_skinSize, _maxSize);
            _skinSize.cx = LONG((double)_skinSize.cx * fRatio);
            _skinSize.cy = LONG((double)_skinSize.cy * fRatio);
        }
    }

    //
    // 解析出图片的每一帧图片以及延迟
    //
    void SAntialiasSkin::LoadFrameInfos(Gdiplus::Bitmap * pImage)
    {
        _pImage = pImage;   // 接管图片

        Gdiplus::PropertyItem * pPropertyItem = NULL;
        UINT size = pImage->GetPropertyItemSize(PropertyTagFrameDelay);
        if (size > 1)
        {
            pPropertyItem = (Gdiplus::PropertyItem *)malloc(size);
            pImage->GetPropertyItem(PropertyTagFrameDelay, size, pPropertyItem);
        }

        for (int i = 0; i < _frameCount; ++i)
        {
            FrameInfo frame;

            //
            // 如果是多帧图片，不在这里一次性把内容画到缓存，因为如果图片较多，这里可能会造成卡顿。
            // 在_Draw里，等到需要画用到某一帧时，才把_pImage里的图片画到缓存。当把
            // _pImage的所有帧都画到缓存后，就释放_pImage。
            //
            if (_frameCount == 1)
            {
                frame.Frame = new Gdiplus::Bitmap(_skinSize.cx, _skinSize.cy, PixelFormat32bppPARGB);
                Gdiplus::Graphics *pGraphics = Gdiplus::Graphics::FromImage(frame.Frame);
                pImage->SelectActiveFrame(&FrameDimensionTime, i);
                pGraphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
                pGraphics->DrawImage(pImage, 0, 0, frame.Frame->GetWidth(), frame.Frame->GetHeight());
                delete pGraphics;

                SetBitmapRoundCorner(frame.Frame);

                delete _pImage;
                _pImage = NULL;
            }

            //
            // 读取延时信息
            //
            if (pPropertyItem != NULL)
            {
                frame.Delay = ((long*)pPropertyItem->value)[i];
            }

            _frames[i] = frame;
        }

        _currentFrame = 0;

        free(pPropertyItem);
    }

    int SAntialiasSkin::LoadFrameCount(Gdiplus::Bitmap * pImage)
    {
        _frameCount = 0;
        UINT nCount = pImage->GetFrameDimensionsCount();

        GUID* pDimensionIDs = new GUID[nCount];
        if (pDimensionIDs != NULL)
        {
            pImage->GetFrameDimensionsList(pDimensionIDs, nCount);
            _frameCount = pImage->GetFrameCount(&pDimensionIDs[0]);
            delete pDimensionIDs;
        }

        _currentFrame = 0;

        return _frameCount;
    }

    void SAntialiasSkin::RotateImage(Gdiplus::Bitmap* pImage)
    {
        int nOrientationSize = pImage->GetPropertyItemSize(PropertyTagOrientation);
        PropertyItem * pOrientation = (PropertyItem*)malloc(nOrientationSize);
        int nRet = pImage->GetPropertyItem(PropertyTagOrientation, nOrientationSize, pOrientation);
        if (nRet != Ok)
        {
            // 方位信息不存在,不需要旋转
            free(pOrientation);
            return;
        }

        unsigned char * pValue = (unsigned char*)pOrientation->value;
        int nOrientation = pValue[0];
        free(pOrientation);
        switch (nOrientation)
        {
        case 1:
            return;

        case 2:
            pImage->RotateFlip(RotateNoneFlipY);
            break;
        case 3:
            pImage->RotateFlip(Rotate180FlipNone);
            break;
        case 4:
            pImage->RotateFlip(Rotate180FlipY);
            break;
        case 5:
            pImage->RotateFlip(Rotate90FlipY);
            break;
        case 6:
            pImage->RotateFlip(Rotate90FlipNone);
            break;
        case 7:
            pImage->RotateFlip(Rotate270FlipY);
            break;
        case 8:
            pImage->RotateFlip(Rotate270FlipNone);
            break;
        }
    }

}//namespace SOUI