/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       stabctrl.h
 * @brief      
 * @version    v1.0      
 * @author     soui      
 * @date       2014-07-06
 * 
 * Describe    SOUI应用程序入口 
 */

#pragma once
#include "core/ssingleton.h"
#include "unknown/obj-ref-impl.hpp"
#include "interface/render-i.h"
#include "interface/SScriptModule-i.h"
#include "interface/STranslator-i.h"

#include "res.mgr/SResProviderMgr.h"

#include "core/smsgloop.h"
#include "core/SWndFactoryMgr.h"
#include "core/SSkinFactoryMgr.h"


#define SOUI_VERSION    _T("0.9.0.1")

#define LOADXML(p1,p2,p3) SApplication::getSingleton().LoadXmlDocment(p1,p2,p3)
#define LOADIMAGE(p1,p2) SApplication::getSingleton().LoadImage(p1,p2)
#define GETRESPROVIDER    SApplication::getSingletonPtr()
#define GETRENDERFACTORY SApplication::getSingleton().GetRenderFactory()
#define TR(p1,p2)       SApplication::getSingleton().GetTranslator()->tr(p1,p2)
               
#define RT_UIDEF _T("UIDEF")
#define RT_LAYOUT _T("LAYOUT")

namespace SOUI
{

/** 
 * @class     SApplication
 * @brief     SOUI Application
 *
 * Describe   SOUI Application
 */
class SOUI_EXP SApplication :public SSingleton<SApplication>
                        ,public SWindowFactoryMgr
                        ,public SSkinFactoryMgr
                        ,public SResProviderMgr
                        ,public SMessageLoop
{
public:
    /**
     * SApplication
     * @brief    构造函数
     * @param    IRenderFactory * pRendFactory --  渲染模块
     * @param    HINSTANCE hInst --  应用程序句柄
     * @param    LPCTSTR pszHostClassName --  使用SOUI创建窗口时默认的窗口类名
     *
     * Describe  
     */
    SApplication(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName=_T("SOUIHOST"));
    ~SApplication(void);


    /**
     * GetInstance
     * @brief    获得应用程序句柄
     * @return   HINSTANCE 
     *
     * Describe  
     */
    HINSTANCE GetInstance();

    /**
     * GetVersion
     * @brief    获得SOUI的版本号
     * @return   LPCTSTR 
     *
     * Describe  
     */
    LPCTSTR GetVersion();

    /**
     * Init
     * @brief    初始化SOUI系统
     * @param    LPCTSTR pszName --  初始化SOUI的XML文件在资源中的name
     * @param    LPCTSTR pszType --  初始化SOUI的XML文件在资源中的type
     * @return   BOOL true-初始化成功, false-初始化失败
     *
     * Describe  初始化的XML必须满足SOUI的格式。
     */
    BOOL Init(LPCTSTR pszName ,LPCTSTR pszType=RT_UIDEF);

    /**
     * LoadSystemNamedResource
     * @brief    加载SOUI系统默认的命名资源
     * @param    IResProvider * pResProvider --  
     * @return   UINT 
     *
     * Describe  
     */
    UINT LoadSystemNamedResource(IResProvider *pResProvider);
    
    /**
     * LoadXmlDocment
     * @brief    从资源中加载一个XML Document。
     * @param [out] pugi::xml_document & xmlDoc --  输出的xml_document对象
     * @param    LPCTSTR pszXmlName --  XML文件在资源中的name
     * @param    LPCTSTR pszType --  XML文件在资源中的type
     * @return   BOOL true-加载成功, false-加载失败
     *
     * Describe  
     */
    BOOL LoadXmlDocment(pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType);

    /**
     * GetRenderFactory
     * @brief    获得当前的渲染模块
     * @return   IRenderFactory * 渲染模块指针
     *
     * Describe  
     */
    IRenderFactory * GetRenderFactory();

    /**
     * GetScriptModule
     * @brief    获取SOUI中引用的脚本模块
     * @return   IScriptModule * 脚本模块指针
     *
     * Describe  
     */
    IScriptModule * GetScriptModule();

    /**
     * SetScriptModule
     * @brief    设置SOUI中使用的脚本模块
     * @param    IScriptModule * pScriptModule --  脚本模块指针
     * @return   void 
     *
     * Describe  
     */
    void SetScriptModule(IScriptModule *pScriptModule);
    
    /**
     * GetTranslator
     * @brief    获取语言翻译模块
     * @return   ITranslator * 语言翻译模块指针
     *
     * Describe  
     */
    ITranslatorMgr * GetTranslator();
    
    /**
     * SetTranslator
     * @brief    设置语言翻译模块
     * @param    ITranslator * pTrans --  语言翻译模块指针
     * @return   void 
     *
     * Describe  
     */
    void SetTranslator(ITranslatorMgr * pTrans);
    
    /**
     * Run
     * @brief    启动SOUI的主消息循环
     * @param    HWND hMainWnd --  应用程序主窗口句柄
     * @return   int 消息循环结束时的返回值
     *
     * Describe  
     */
    int Run(HWND hMainWnd);
protected:
    void _CreateSingletons();
    void _DestroySingletons();
    BOOL _LoadXmlDocment(IResProvider* pResProvider, LPCTSTR pszXmlName ,LPCTSTR pszType ,pugi::xml_document & xmlDoc);

    HINSTANCE m_hInst;
    CAutoRefPtr<IScriptModule>  m_pScriptModule;
    CAutoRefPtr<IRenderFactory> m_RenderFactory;
    CAutoRefPtr<ITranslatorMgr>    m_Translator;
};

}//namespace SOUI