/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       Sobject.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/01
* 
* Describe    the base class used in SOUI, which provides type identify of class in runtime
*             and implement attributes dispatcher described in XML. 
*/

#pragma once
#include <trace.h>

//////////////////////////////////////////////////////////////////////////

// SObject Class Name Declaration
#define SOUI_CLASS_NAME(theclass, classname)            \
public:                                                 \
    static LPCWSTR GetClassName()                       \
    {                                                   \
        return classname;                               \
    }                                                   \
    \
    static LPCWSTR BaseClassName()                      \
    {                                                   \
        return __super::GetClassName();                 \
    }                                                   \
    \
    virtual LPCWSTR GetObjectClass()                    \
    {                                                   \
        return classname;                               \
    }                                                   \
    \
    virtual BOOL IsClass(LPCWSTR lpszName)              \
    {                                                   \
        if(wcscmp(GetClassName(), lpszName)  == 0)      \
            return TRUE;                                \
        return __super::IsClass(lpszName);              \
    }                                                   \


namespace SOUI
{

    /**
    * @class      SObject
    * @brief      SOUI系统中的对象基类
    * 
    * Describe    提供类RTTI机制，实现从XML节点中给派生类对象设置属性
    */
    class SOUI_EXP SObject
    {
    public:
        SObject()
        {
        }

        virtual ~SObject()
        {
        }

        /**
         * GetClassName
         * @brief    获得类名
         * @return   LPCWSTR -- 类名
         * Describe  静态函数
         */    
        static LPCWSTR GetClassName()
        {
            return NULL;
        }

        /**
         * BaseClassName
         * @brief    获得基类的类名
         * @return   LPCWSTR -- 基类的类名
         * Describe  
         */    
        static LPCWSTR BaseClassName()
        {
            return NULL;
        }

        /**
         * IsClass
         * @brief    判断this是不是属于指定的类型
         * @param    LPCWSTR lpszName --  测试类型名
         * @return   BOOL -- true是测试类型
         * Describe  
         */    
        virtual BOOL IsClass(LPCWSTR lpszName)
        {
            return FALSE;
        }

        /**
         * GetObjectClass
         * @brief    获得类型名
         * @return   LPCWSTR -- 类型名
         * Describe  这是一个虚函数，注意与GetClassName的区别。
         */    
        virtual LPCWSTR GetObjectClass()
        {
            return NULL;
        }

        /**
         * InitFromXml
         * @brief    从XML节点中初始化对象属性
         * @param    pugi::xml_node xmlNode --  XML节点数据
         * @return   BOOL -- true成功
         * Describe  
         */    
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);

        /**
         * SetAttribute
         * @brief    设置一个对象属性
         * @param    const SStringA & strAttribName --  属性名
         * @param    const SStringA & strValue --  属性值
         * @param    BOOL bLoading --  对象创建时由系统调用标志
         * @return   HRESULT -- 处理处理结果
         * Describe  
         */    
        virtual HRESULT SetAttribute(const SStringA &  strAttribName, const SStringA &  strValue, BOOL bLoading)
        {
            return DefAttributeProc(S_CA2W(strAttribName),S_CA2W(strValue),bLoading);
        }

        /**
        * SetAttribute
        * @brief    设置一个对象属性
        * @param    const SStringA & strAttribName --  属性名
        * @param    const SStringA & strValue --  属性值
        * @param    BOOL bLoading --  对象创建时由系统调用标志
        * @return   HRESULT -- 处理处理结果
        * Describe  
        */    
        virtual HRESULT SetAttribute(const SStringW &  strAttribName, const SStringW &  strValue, BOOL bLoading)
        {
            return DefAttributeProc(strAttribName,strValue,bLoading);
        }

        /**
         * DefAttributeProc
         * @brief    默认的属性处理接口
         * @param    const SStringA & strAttribName --  属性名
         * @param    const SStringA & strValue --  属性值
         * @param    BOOL bLoading --  对象创建时由系统调用标志
         * @return   HRESULT -- 处理处理结果
         * Describe  
         */    
        virtual HRESULT DefAttributeProc(const SStringW & strAttribName,const SStringW & strValue, BOOL bLoading)
        {
            STraceW(L"warning!!! unhandled attribute %s in class %s, value = %s",strAttribName,GetObjectClass(),strValue);
            return E_FAIL;
        }

        /**
         * tr
         * @brief    语言翻译接口
         * @param    const SStringW & strSrc --  源字符串
         * @return   SStringW -- 翻译后的字符串
         * Describe  
         */    
        virtual SStringW tr(const SStringW &strSrc);

        /**
         * GetID
         * @brief    获取对象ID
         * @return   int -- 对象ID
         * Describe  
         */    
        virtual int GetID() const {return 0;}

        /**
         * GetName
         * @brief    获取对象Name
         * @return   LPCWSTR -- 对象Name
         * Describe  
         */    
        virtual LPCWSTR GetName() const {return NULL;}
    protected:
        /**
         * OnInitFinished
         * @brief    属性初始化完成处理接口
         * @param    pugi::xml_node xmlNode --  属性节点
         * @return   void
         * Describe  
         */    
        virtual void OnInitFinished(pugi::xml_node xmlNode) {}

#ifdef    _DEBUG
    public:
        SStringW m_strXml;  //<** XML字符串，用于在调试时观察对象*/
#endif//_DEBUG
    };

}//namespace SOUI
