/********************************************************************
created:	2012/12/27
created:	27:12:2012   14:55
filename: 	DuiSkinGif.h
file base:	DuiSkinGif
file ext:	h
author:		huangjianxiong

purpose:	自定义皮肤对象
*********************************************************************/
#pragma once
#include <map>
#include <interface/SSkinobj-i.h>
#include <unknown/obj-ref-impl.hpp>

namespace Gdiplus
{
    class Bitmap;
}

namespace SOUI
{
    class SAntialiasSkin : public SSkinObjBase
    {
        SOUI_CLASS_NAME(SAntialiasSkin, L"antialias")

    public:

        SAntialiasSkin();
        ~SAntialiasSkin();

        virtual int GetStates();
        virtual SIZE GetSkinSize();

        int GetFrameCount();
        long GetFrameDelay(int frame = -1);
        void ActiveNextFrame();
        Gdiplus::Bitmap* SelectActiveFrame(int frame);
        void AddFrame(LPCWSTR pszFileName, int delay);
        int  LoadFromFile(LPCTSTR pszFileName);
        int  LoadFromMemory(LPVOID pBits, size_t szData);
        int  LoadFromHandle(HBITMAP hBmp);
        int  LoadFromIcon(HICON hIcon);
        void SetMaxSize(CSize size);
        void SetRound(BOOL round);
        void SetRoundCorner(int leftTop, int rightTop, int rightBottom, int leftBottom);
        void SetAutoZoom(BOOL autoZoom);
        void Rotate(int type);

        SAntialiasSkin* Clone();

    protected:

        void    ReleaseImageWhileBuffReady();
        BOOL    RoundBitmap(Gdiplus::Bitmap*& pBitmap);
        BOOL    SetBitmapRoundCorner(Gdiplus::Bitmap*& pBitmap);
        BOOL    ResizeBitmap(Gdiplus::Bitmap*& pBitmap, int width, int height);
        void    FreeImage();
        void    CalcSkinSize(Gdiplus::Bitmap * pImage);
        int     LoadFrameCount(Gdiplus::Bitmap * pImg);
        void    RotateImage(Gdiplus::Bitmap* pImage);
        LRESULT OnAttrSrc(const SStringW &strValue, BOOL bLoading);
        int     LoadFromGdipImage(Gdiplus::Bitmap*& pImg);
        void    LoadFrameInfos(Gdiplus::Bitmap * pImage);
        int     GetInterpolationMode(float fScaling);
        void    _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha = 0xFF);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"src", OnAttrSrc)
            ATTR_SIZE(L"maxSize", _maxSize, TRUE)
            SOUI_ATTRS_END()

    protected:

        struct  FrameInfo
        {
            FrameInfo() : Frame(NULL), Delay(0)
            {
            }

            Gdiplus::Bitmap* Frame;
            int Delay;
        };

        typedef std::map<int, FrameInfo> FrameMap;

        Gdiplus::Bitmap *   _pImage;        // 原始图片，当里面的图片都保存到缓存时被释放
        FrameMap            _frames;        // 图片缓存
        int                 _frameCount;
        int                 _currentFrame;
        CSize               _skinSize;
        CSize               _maxSize;
        BOOL                _isRound;
        BOOL                _autoZoom;      // 是否自动缩放，如果是则按照_maxSize为最大显示尺寸进行缩放

        int                 _leftTopRadius;
        int                 _rightTopRadius;
        int                 _rightBottomRadius;
        int                 _leftBottomRadius;
    };

}//end of name space SOUI
