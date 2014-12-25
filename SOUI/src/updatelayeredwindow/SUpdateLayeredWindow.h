#pragma once

namespace SOUI
{

    struct S_UPDATELAYEREDWINDOWINFO {
        DWORD cbSize;
        HDC hdcDst;
        const POINT *pptDst;
        const SIZE *psize;
        HDC hdcSrc;
        const POINT *pptSrc;
        COLORREF crKey;
        const BLENDFUNCTION *pblend;
        DWORD dwFlags;
        const RECT *prcDirty;
    };

    class SWndSurface{
    public:
        static BOOL Init();
        static BOOL SUpdateLayeredWindowIndirect(HWND hWnd, const S_UPDATELAYEREDWINDOWINFO *pULWInfo);        
    };
}
