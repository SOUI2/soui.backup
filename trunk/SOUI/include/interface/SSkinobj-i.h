/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SSkinobj-i.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/01
* 
* Describe    interface of SSkinObj
*/
#pragma once

#include "sobject.h"
#include <unknown/obj-ref-i.h>
#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{
    /**
    * @struct     ISkinObj
    * @brief      Skin 对象
    * 
    * Describe
    */
    class SOUI_EXP ISkinObj : public SObject,public TObjRefImpl2<IObjRef,ISkinObj>
    {
    public:
        ISkinObj()
        {
        }
        virtual ~ISkinObj()
        {
        }

        /**
         * Draw
         * @brief    将this绘制到RenderTarget上去
         * @param    IRenderTarget * pRT --  绘制用的RenderTarget
         * @param    LPCRECT rcDraw --  绘制位置
         * @param    DWORD dwState --  绘制状态
         * @param    BYTE byAlpha --  透明度
         * @return   void
         * Describe  
         */    
        virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha=0xFF)=0;

        /**
         * GetSkinSize
         * @brief    获得Skin的默认大小
         * @return   SIZE -- Skin的默认大小
         * Describe  派生类应该根据skin的特点实现该接口
         */    
        virtual SIZE GetSkinSize()
        {
            SIZE ret = {0, 0};

            return ret;
        }

        /**
         * IgnoreState
         * @brief    查询skin是否有状态信息
         * @return   BOOL -- true有状态信息
         * Describe  
         */    
        virtual BOOL IgnoreState()
        {
            return TRUE;
        }

        /**
         * GetStates
         * @brief    获得skin对象包含的状态数量
         * @return   int -- 状态数量
         * Describe  默认为1
         */    
        virtual int GetStates()
        {
            return 1;
        }
    };
}//namespace SOUI
