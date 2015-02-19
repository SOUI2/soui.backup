#pragma once

#include "interface/Sskinobj-i.h"
#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{

    class SOUI_EXP SSkinObjBase : public TObjRefImpl<ISkinObj>
    {
    public:
        SSkinObjBase():m_byAlpha(0xFF){}

        /**
        * GetAlpha
        * @brief    获得skin对象包含透明度
        * @return   BYTE -- 透明度
        * Describe  [0-255]
        */    
        BYTE GetAlpha() const
        {
            return m_byAlpha;
        }

        /**
        * SetAlpha
        * @brief    设定skin对象包含透明度
        * @param    BYTE byAlpha-- 透明度
        * @return   void
        * Describe  
        */    
        virtual void SetAlpha(BYTE byAlpha)
        {
            m_byAlpha = byAlpha;
        }

        virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha)
        {
            _Draw(pRT,rcDraw,dwState,byAlpha);
        }

        virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState)
        {
            Draw(pRT,rcDraw,dwState,GetAlpha());
        }

        virtual SIZE GetSkinSize()
        {
            SIZE ret = {0, 0};

            return ret;
        }

        virtual BOOL IgnoreState()
        {
            return TRUE;
        }

        virtual int GetStates()
        {
            return 1;
        }

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"alpha",m_byAlpha,TRUE)   //皮肤透明度
        SOUI_ATTRS_END()

    protected:

        /**
        * _Draw
        * @brief    Draw函数的实现
        * @param    IRenderTarget * pRT --  渲染目标
        * @param    LPCRECT rcDraw --  渲染位置
        * @param    DWORD dwState --  渲染状态
        * @param    BYTE byAlpha --  透明度
        * @return   void
        * Describe  每一个skin需要实现一个_Draw方法
        */    
        virtual void _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha)=0;

        BYTE m_byAlpha;
    };

}//namespace SOUI