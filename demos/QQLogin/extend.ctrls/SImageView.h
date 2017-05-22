#pragma once
#include <core/swnd.h>

namespace SOUI
{
    //
    // 图片显示控件，包括GIF
    //

    class SImageView : public SWindow, public ITimelineHandler
    {
        SOUI_CLASS_NAME(SImageView, L"image")   //定义GIF控件在XM加的标签

    public:

        SImageView();
        ~SImageView();

        BOOL        ShowImageFile(LPCTSTR pszFileName);
        BOOL        IsPlaying();
        int         GetFrameCount();
        SIZE        GetImageSize();
        void        SetImageSize(SIZE size);
        void        Pause();
        void        Resume();
        void        ShowFrame(int frame, BOOL update = FALSE);
        SStringW    GetRealPath();
        void        SetSkin(ISkinObj* pSkin);
        ISkinObj*   GetSkin();

    protected:

        //SWindow的虚函数
        virtual CSize GetDesiredSize(LPCRECT pRcContainer);

        //ITimerLineHander
        virtual void OnNextFrame();

    protected:

        int  GetFrameDelay();

        HRESULT OnAttrSkin(const SStringW & strValue, BOOL bLoading);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"skin", OnAttrSkin)
            ATTR_INT(L"autoPlay", _isPlaying, FALSE)
            SOUI_ATTRS_END()

    protected:

        void OnPaint(IRenderTarget *pRT);
        void OnShowWindow(BOOL bShow, UINT nStatus);
        void OnDestroy();

        //SOUI控件消息映射表
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)    //窗口绘制消息
            MSG_WM_SHOWWINDOW(OnShowWindow)//窗口显示状态消息
            MSG_WM_DESTROY(OnDestroy)
            SOUI_MSG_MAP_END()

    protected:

        ISkinObj *  _pImageSkin;
        BOOL        _isPlaying;
        int	        _currentFrame;
        int         _nextFrameInterval;
        SIZE        _imageSize;
        SStringW    _realPath;      /*< 最终显示的图片路径 */
    };
}