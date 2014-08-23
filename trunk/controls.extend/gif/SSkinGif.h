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
#include <interface/SSkinobj-i.h>
#include <unknown/obj-ref-impl.hpp>

namespace Gdiplus
{
class Bitmap;
}
namespace SOUI
{
    class SGifFrame
    {
    public:
        CAutoRefPtr<IBitmap> pBmp;
        int                  nDelay;
    };

    /**
    * @class     SSkinGif
    * @brief     GIF图片加载及显示对象
    * 
    * Describe
    */
    class SSkinGif : public ISkinObj
    {
        SOUI_CLASS_NAME(SSkinGif, L"gif")
    public:
        SSkinGif():m_nFrames(0),m_iFrame(0),m_pFrames(NULL)
        {

        }
        
        //初始化GDI+环境，由于这里需要使用GDI+来解码GIF文件格式
        static BOOL Gdiplus_Startup();
        //退出GDI+环境
        static void Gdiplus_Shutdown();

        virtual ~SSkinGif()
        {
            if(m_pFrames) delete [] m_pFrames;
        }

        /**
         * Draw
         * @brief    绘制指定帧的GIF图
         * @param    IRenderTarget * pRT --  绘制目标
         * @param    LPCRECT rcDraw --  绘制范围
         * @param    DWORD dwState --  绘制状态，这里被解释为帧号
         * @param    BYTE byAlpha --  透明度
         * @return   void
         * Describe  
         */    
        virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha=0xFF);

        /**
         * GetStates
         * @brief    获得GIF帧数
         * @return   int -- 帧数
         * Describe  
         */    
        virtual int GetStates(){return m_nFrames;}

        /**
         * GetSkinSize
         * @brief    获得图片大小
         * @return   SIZE -- 图片大小
         * Describe  
         */    
        virtual SIZE GetSkinSize()
        {
            SIZE sz={0};
            if(m_nFrames>0 && m_pFrames)
            {
                sz=m_pFrames[0].pBmp->Size();
            }
            return sz;
        }

        /**
         * GetFrameDelay
         * @brief    获得指定帧的显示时间
         * @param    int iFrame --  帧号,为-1时代表获得当前帧的延时
         * @return   long -- 延时时间(*10ms)
         * Describe  
         */    
        long GetFrameDelay(int iFrame=-1);

        /**
         * ActiveNextFrame
         * @brief    激活下一帧
         * @return   void 
         * Describe  
         */    
        void ActiveNextFrame();

        /**
         * SelectActiveFrame
         * @brief    激活指定帧
         * @param    int iFrame --  帧号
         * @return   void
         * Describe  
         */    
        void SelectActiveFrame(int iFrame);
        
        /**
         * LoadFromFile
         * @brief    从文件加载GIF
         * @param    LPCTSTR pszFileName --  文件名
         * @return   int -- GIF帧数，0-失败
         * Describe  
         */    
        int LoadFromFile(LPCTSTR pszFileName);

        /**
         * LoadFromMemory
         * @brief    从内存加载GIF
         * @param    LPVOID pBits --  内存地址
         * @param    size_t szData --  内存数据长度
         * @return   int -- GIF帧数，0-失败
         * Describe  
         */    
        int LoadFromMemory(LPVOID pBits,size_t szData);

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"src",OnAttrSrc)   //XML文件中指定的图片资源名,(type:name)
        SOUI_ATTRS_END()
    protected:
        LRESULT OnAttrSrc(const SStringW &strValue,BOOL bLoading);
        int LoadFromGdipImage(Gdiplus::Bitmap * pImg);
        int m_nFrames;
        int m_iFrame;

        SGifFrame * m_pFrames;
    };
}//end of name space SOUI
