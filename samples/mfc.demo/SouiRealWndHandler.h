#pragma once

#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{
    class CSouiRealWndHandler :public TObjRefImpl2<IRealWndHandler,CSouiRealWndHandler>
    {
    public:
        CSouiRealWndHandler(void);
        ~CSouiRealWndHandler(void);

        virtual HWND OnRealWndCreate(SRealWnd *pRealWnd);

        /**
        * SRealWnd::OnRealWndInit
        * @brief    初始化窗口
        * @param    SRealWnd *pRealWnd -- 窗口指针
        *
        * Describe  初始化窗口
        */
        virtual BOOL OnRealWndInit(SRealWnd *pRealWnd){return FALSE;}

        /**
        * SRealWnd::OnRealWndDestroy
        * @brief    销毁窗口
        * @param    SRealWnd *pRealWnd -- 窗口指针
        *
        * Describe  销毁窗口
        */
        virtual void OnRealWndDestroy(SRealWnd *pRealWnd);

        /**
        * SRealWnd::OnRealWndSize
        * @brief    调整窗口大小
        * @param    SRealWnd *pRealWnd -- 窗口指针
        *
        * Describe  调整窗口大小
        */
        virtual void OnRealWndSize(SRealWnd *pRealWnd);
    };

}
