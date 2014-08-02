/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       STranslator-i.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    
*/

#pragma once

#include <unknown/obj-ref-i.h>
#include <string/tstring.h>

namespace SOUI
{
    /** 
     * @struct     ITranslator
     * @brief      语言翻译接口
     *
     * Describe
     */
    struct ITranslator : public IObjRef
    {
        /**
         * Load
         * @brief    从资源中加载语言翻译数据
         * @param    LPVOID pData --  资源指针，具体含义由接口的实现来解释
         * @param    UINT uType --  资源类型，具体含义由接口的实现来解释
         * @return   BOOL true-加载成功, false-加载失败
         *
         * Describe  
         */
        virtual BOOL Load(LPVOID pData,UINT uType)=0;
        /**
         * name
         * @brief    获取翻译资源的name
         * @return   SOUI::SStringW 翻译资源的name
         *
         * Describe  
         */
        virtual SStringW name()=0;
        /**
         * guid
         * @brief    获取翻译资源的ID
         * @return   GUID 翻译资源的ID
         *
         * Describe  
         */
        virtual GUID     guid()=0;
        /**
         * tr
         * @brief    执行翻译的接口
         * @param    const SStringW & strSrc --  原字符串
         * @param    const SStringW & strCtx --  翻译上下文
         * @param    SStringW & strRet --  翻译后的字符串
         * @return   BOOL true-翻译成功，false-翻译失败
         *
         * Describe  
         */
        virtual BOOL tr(const SStringW & strSrc,const SStringW & strCtx,SStringW & strRet)=0;
    };

    /** 
     * @struct     ITranslatorMgr
     * @brief      语言翻译接口管理器
     *
     * Describe
     */
    struct ITranslatorMgr : public IObjRef
    {
        /**
         * CreateTranslator
         * @brief    创建一个语言翻译对象
         * @param [out] ITranslator * * ppTranslator --  接收语言翻译对象的指针
         * @return   BOOL true-成功，false-失败
         *
         * Describe  
         */
        virtual BOOL CreateTranslator(ITranslator ** ppTranslator)=0;
        /**
         * InstallTranslator
         * @brief    向管理器中安装一个语言翻译对象
         * @param    ITranslator * ppTranslator -- 语言翻译对象
         * @return   BOOL true-成功，false-失败
         *
         * Describe  
         */

        virtual BOOL InstallTranslator(ITranslator * ppTranslator) =0;
        /**
         * UninstallTranslator
         * @brief    从管理器中卸载一个语言翻译对象
         * @param    REFGUID id --  语言翻译对象的ID
         * @return   BOOL true-成功，false-失败
         *
         * Describe  
         */
        virtual BOOL UninstallTranslator(REFGUID id) =0;
        
        /**
         * tr
         * @brief    翻译字符串
         * @param    const SStringW & strSrc --  原字符串
         * @param    const SStringW & strCtx --  翻译上下文
         * @return   SOUI::SStringW 翻译后的字符串
         *
         * Describe  调用ITranslator的tr接口执行具体翻译过程
         */
        virtual SStringW tr(const SStringW & strSrc,const SStringW & strCtx)=0;
    };

}