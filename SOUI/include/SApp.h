/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
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
#include "interface/stooltip-i.h"
#include "control/RealWndHandler-i.h"

#include "res.mgr/SResProviderMgr.h"

#include "core/smsgloop.h"
#include "core/SWndFactoryMgr.h"
#include "core/SSkinFactoryMgr.h"

#define GETRESPROVIDER      SApplication::getSingletonPtr()
#define GETRENDERFACTORY    SApplication::getSingleton().GetRenderFactory()
#define GETREALWNDHANDLER   SApplication::getSingleton().GetRealWndHander()
#define GETTOOLTIPFACTORY   SApplication::getSingleton().GetToolTipFactory()

#define LOADXML(p1,p2,p3)   SApplication::getSingleton().LoadXmlDocment(p1,p2,p3)
#define LOADIMAGE(p1,p2)    SApplication::getSingleton().LoadImage(p1,p2)
#define LOADIMAGE2(p1)      SApplication::getSingleton().LoadImage2(p1)
#define LOADICON(p1,p2)     SApplication::getSingleton().LoadIcon(p1,p2,p2)
#define LOADICON2(p1)       SApplication::getSingleton().LoadIcon2(p1)
#define TR(p1,p2)           SApplication::getSingleton().GetTranslator()->tr(p1,p2)
#define STR2ID(p1)          SNamedID::getSingleton().String2ID(p1)

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
     * @brief    创建脚本模块对象
     * @param [out] IScriptModule **ppScriptModule -- 脚本模块对象
     * @return   HRESULT -- S_OK 创建成功
     *
     * Describe  
     */
    HRESULT CreateScriptModule(IScriptModule **ppScriptModule);

    /**
     * SetScriptModule
     * @brief    设置SOUI中使用的脚本模块类厂
     * @param    IScriptFactory *pScriptModule --  脚本模块类厂
     * @return   void 
     *
     * Describe  
     */
    void SetScriptFactory(IScriptFactory *pScriptModule);
    
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
     * GetRealWndHander
     * @brief    获得RealWndHander
     * @return   IRealWndHandler * -- RealWndHander
     * Describe  
     */    
    IRealWndHandler * GetRealWndHander();

    /**
     * SetRealWndHandler
     * @brief    设置RealWnd处理接口
     * @param    IRealWndHandler * pRealHandler --  RealWnd处理接口
     * @return   void
     * Describe  
     */    
    void SetRealWndHandler(IRealWndHandler *pRealHandler);

    /**
     * GetToolTipFactory
     * @brief    获取ToolTip处理接口
     * @return   IToolTipFactory * -- ToolTip处理接口
     * Describe  
     */    
    IToolTipFactory * GetToolTipFactory();

    /**
     * SetToolTipFactory
     * @brief    设置ToolTip处理接口
     * @param    IToolTipFactory * pToolTipFac --  ToolTip处理接口
     * @return   void -- 
     * Describe  
     */    
    void SetToolTipFactory(IToolTipFactory* pToolTipFac);

    /**
     * Run
     * @brief    启动SOUI的主消息循环
     * @param    HWND hMainWnd --  应用程序主窗口句柄
     * @return   int 消息循环结束时的返回值
     *
     * Describe  
     */
    int Run(HWND hMainWnd);

    void SetAppDir(const SStringT & strAppDir){m_strAppDir = strAppDir;}

    SStringT GetAppDir()const{return m_strAppDir;}
protected:
    void _CreateSingletons();
    void _DestroySingletons();
    BOOL _LoadXmlDocment(LPCTSTR pszXmlName ,LPCTSTR pszType ,pugi::xml_document & xmlDoc);
    
    CAutoRefPtr<IRealWndHandler>    m_pRealWndHandler;
    CAutoRefPtr<IScriptFactory>     m_pScriptFactory;
    CAutoRefPtr<IRenderFactory>     m_RenderFactory;
    CAutoRefPtr<ITranslatorMgr>     m_translator;
    CAutoRefPtr<IToolTipFactory>    m_tooltipFactory;

    SStringT    m_strAppDir;
    HINSTANCE   m_hInst;
};

}//namespace SOUI