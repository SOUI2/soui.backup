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
#include "../utilities-def.h"

#include "string/tstring.h"
#include "pugixml/pugixml.hpp"
#include "string/strcpcvt.h"

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
	virtual LPCWSTR GetObjectClass()  const             \
	{                                                   \
		return classname;                               \
	}                                                   \
	\
	virtual BOOL IsClass(LPCWSTR lpszName) const        \
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
	struct UTILITIES_API IObject
    {
        /**
         * GetClassName
         * @brief    获得类名
         * @return   LPCWSTR -- 类名
         * Describe  静态函数
         */    
		static LPCWSTR GetClassName(){return L"object";}

		virtual ~IObject(){}


        /**
         * IsClass
         * @brief    判断this是不是属于指定的类型
         * @param    LPCWSTR lpszName --  测试类型名
         * @return   BOOL -- true是测试类型
         * Describe  
         */    
        virtual BOOL IsClass(LPCWSTR lpszName) const = 0;

        /**
         * GetObjectClass
         * @brief    获得类型名
         * @return   LPCWSTR -- 类型名
         * Describe  这是一个虚函数，注意与GetClassName的区别。
         */    
        virtual LPCWSTR GetObjectClass() const = 0;


        virtual HRESULT SetAttribute(const char*  strAttribName, const char*  strValue, BOOL bLoading) = 0;

        /**
         * SetAttribute
         * @brief    设置一个对象属性
         * @param    const SStringA & strAttribName --  属性名
         * @param    const SStringA & strValue --  属性值
         * @param    BOOL bLoading --  对象创建时由系统调用标志
         * @return   HRESULT -- 处理处理结果
         * Describe  
         */    
        virtual HRESULT SetAttribute(const SStringA &  strAttribName, const SStringA &  strValue, BOOL bLoading) = 0;

        /**
        * SetAttribute
        * @brief    设置一个对象属性
        * @param    const SStringA & strAttribName --  属性名
        * @param    const SStringA & strValue --  属性值
        * @param    BOOL bLoading --  对象创建时由系统调用标志
        * @return   HRESULT -- 处理处理结果
        * Describe  
        */    
        virtual HRESULT SetAttribute(const SStringW &  strAttribName, const SStringW &  strValue, BOOL bLoading) = 0;

        /**
         * OnAttribute
         * @brief    属性处理后调用的方法
         * @param    const SStringW & strAttribName --  属性名
         * @param    const SStringW & strValue --  属性名
         * @param    HRESULT hr --  属性处理结果
         * @return   HRESULT -- 属性处理结果
         * Describe  不做处理，直接返回
         */    
        virtual HRESULT AfterAttribute(const SStringW & strAttribName,const SStringW & strValue,HRESULT hr) = 0;

		/**
         * GetID
         * @brief    获取对象ID
         * @return   int -- 对象ID
         * Describe  
         */    
        virtual int GetID() const = 0;

        /**
         * GetName
         * @brief    获取对象Name
         * @return   LPCWSTR -- 对象Name
         * Describe  
         */    
        virtual LPCWSTR GetName() const = 0;

		virtual BOOL InitFromXml( pugi::xml_node xmlNode ) = 0;


		virtual HRESULT DefAttributeProc(const SStringW & strAttribName,const SStringW & strValue, BOOL bLoading) = 0;

		/**
         * OnInitFinished
         * @brief    属性初始化完成处理接口
         * @param    pugi::xml_node xmlNode --  属性节点
         * @return   void
         * Describe  
         */    
        virtual void OnInitFinished(pugi::xml_node xmlNode) = 0;

		virtual SStringW GetAttribute(const SStringW & strAttr) const = 0;
    };

    /**
     * sobj_cast
     * @brief    SOUI Object 的类型安全的类型转换接口
     * @param    SObject * pObj --  源对象
     * @return   T * -- 转换后的对象
     * Describe  如果源对象不是待转换对象类型，返回NULL
     */    
    template<class T>
    T * sobj_cast(IObject *pObj)
    {
        if(!pObj)
            return NULL;

        if(pObj->IsClass(T::GetClassName()))
            return (T*)pObj;
        else
            return NULL;
    }

}//namespace SOUI
