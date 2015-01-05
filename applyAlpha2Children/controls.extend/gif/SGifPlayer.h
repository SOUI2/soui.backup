#pragma once
#include <core/swnd.h>

#include "sskingif.h"
#include "SSkinAPNG.h"

namespace SOUI
{

    /**
    * @class     SGifPlayer
    * @brief     GIF图片显示控件
    * 
    * Describe
    */
    class SGifPlayer : public SWindow
    {
        SOUI_CLASS_NAME(SGifPlayer, L"gifplayer")   //定义GIF控件在XM加的标签
    public:
        SGifPlayer();
        ~SGifPlayer();

        /**
         * PlayGifFile
         * @brief    在控件中播放一个GIF图片文件
         * @param    LPCTSTR pszFileName --  文件名
         * @return   BOOL -- true:成功
         * Describe  
         */    
        BOOL PlayGifFile(LPCTSTR pszFileName);

        /**
        * PlayAPNGFile
        * @brief    在控件中播放一个APNG图片文件
        * @param    LPCTSTR pszFileName --  文件名
        * @return   BOOL -- true:成功
        * Describe  
        */    
        BOOL PlayAPNGFile(LPCTSTR pszFileName);

    protected://SWindow的虚函数
        virtual CSize GetDesiredSize(LPCRECT pRcContainer);

    public://属性处理
        SOUI_ATTRS_BEGIN()		
            ATTR_CUSTOM(L"skin", OnAttrSkin) //为控件提供一个skin属性，用来接收SSkinObj对象的name
        SOUI_ATTRS_END()
    protected:
        HRESULT OnAttrSkin(const SStringW & strValue, BOOL bLoading);
        
    protected:
        BOOL _PlayFile(LPCTSTR pszFileName, BOOL bGif);
    protected://消息处理，SOUI控件的消息处理和WTL，MFC很相似，采用相似的映射表，相同或者相似的消息映射宏
        
        /**
         * OnPaint
         * @brief    窗口绘制消息响应函数
         * @param    IRenderTarget * pRT --  绘制目标
         * @return   void
         * Describe  注意这里的参数是IRenderTarget *,而不是WTL中使用的HDC，同时消息映射宏也变为MSG_WM_PAINT_EX
         */    
        void OnPaint(IRenderTarget *pRT);

        /**
         * OnTimer
         * @brief    SOUI窗口的定时器处理函数
         * @param    char cTimerID --  定时器ID，范围从0-127。
         * @return   void
         * Describe  SOUI控件的定时器是Host窗口定时器ID的分解，以方便所有的控件都通过Host获得定时器的分发。
         *           注意使用MSG_WM_TIMER_EX来映射该消息。定时器使用SWindow::SetTimer及SWindow::KillTimer来创建及释放。
         *           如果该定时器ID范围不能满足要求，可以使用SWindow::SetTimer2来创建。
         */    
        void OnTimer(char cTimerID);

        /**
         * OnShowWindow
         * @brief    处理窗口显示消息
         * @param    BOOL bShow --  true:显示
         * @param    UINT nStatus --  显示原因
         * @return   void 
         * Describe  参考MSDN的WM_SHOWWINDOW消息
         */    
        void OnShowWindow(BOOL bShow, UINT nStatus);

        //SOUI控件消息映射表
        SOUI_MSG_MAP_BEGIN()	
            MSG_WM_TIMER_EX(OnTimer)    //定时器消息
            MSG_WM_PAINT_EX(OnPaint)    //窗口绘制消息
            MSG_WM_SHOWWINDOW(OnShowWindow)//窗口显示状态消息
        SOUI_MSG_MAP_END()	

    private:
        SSkinAni *m_aniSkin;
        int	m_iCurFrame;
    };

}