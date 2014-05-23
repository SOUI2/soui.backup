#pragma once
#include "duisingleton.h"

#include "DuiPoolsStack.h"
#include "DuiWndFactoryMgr.h"
#include "DuiSkinFactoryMgr.h"

#include "DuiLogger.h"
#include "DuiScriptModule.h"
#include "DuiResProviderMgr.h"

#define SOUI_VERSION    _T("0.0.0.1")

#define GETSKIN(p1) DuiSystem::getSingleton().GetSkin(p1)
#define GETSTYLE(p1,p2) DuiSystem::getSingleton().GetStyle(p1,p2)
#define BUILDSTRING(p1) DuiSystem::getSingleton().BuildString(p1)
#define GETCSS(p1) DuiSystem::getSingleton().GetObjDefAttr(p1)

#define LOADXML(p1,p2,p3) DuiSystem::getSingleton().LoadXmlDocment(p1,p2,p3)
#define GETRESPROVIDER    DuiSystem::getSingletonPtr()
#define GETIMGDECODER   DuiSystem::getSingleton().GetImgDecoder

namespace SOUI
{

class SOUI_EXP DuiSystem :public Singleton<DuiSystem>
                        ,public DuiWindowFactoryMgr
                        ,public DuiSkinFactoryManager
                        ,public DuiPoolsStack
                        ,public DuiResProviderMgr
{
    friend class CSimpleWnd;
    friend class CDuiMessageBox;    //访问消息框模板
    friend class CDuiRichEdit;    //访问右键菜单资源
public:
    DuiSystem(HINSTANCE hInst,LPCTSTR pszHostClassName=_T("DuiHostWnd"));
    ~DuiSystem(void);


    HINSTANCE GetInstance()
    {
        return m_hInst;
    }

    LPCTSTR GetVersion(){return SOUI_VERSION;}

    DuiLogger *SetLogger(DuiLogger *pLogger)
    {
        DuiLogger *pRet=m_pLogger;
        m_pLogger=pLogger;
        return pRet;
    }
    DuiLogger * GetLogger()
    {
        return m_pLogger;
    }

    IScriptModule * GetScriptModule()
    {
        return m_pScriptModule;
    }

    IDuiImgDecoder * GetImgDecoder(){return m_pImgDecoder?m_pImgDecoder:m_pDefImgDecoder;}
    
    void SetImgDecoder(IDuiImgDecoder *pImgDecoder){
        m_pDefImgDecoder=pImgDecoder;
    }

    void SetScriptModule(IScriptModule *pScriptModule)
    {
        m_pScriptModule=pScriptModule;
    }

    void logEvent(LPCTSTR message, LoggingLevel level = Standard);

    void logEvent(LoggingLevel level , LPCTSTR format, ...);

    BOOL Init(LPCTSTR pszName ,LPCTSTR pszType=DUIRES_XML_TYPE);

    BOOL SetMsgBoxTemplate(LPCTSTR pszXmlName,LPCTSTR pszType=DUIRES_XML_TYPE);

    BOOL LoadXmlDocment(pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType=DUIRES_XML_TYPE);

protected:
    pugi::xml_node GetMsgBoxTemplate(){return m_xmlMsgBoxTempl;}
    pugi::xml_node GetEditMenuTemplate(){return m_xmlEditMenu;}

    void createSingletons();
    void destroySingletons();

    IScriptModule        * m_pScriptModule;
    DuiLogger * m_pLogger;
    HINSTANCE m_hInst;
    IDuiImgDecoder        * m_pDefImgDecoder;
    IDuiImgDecoder        * m_pImgDecoder;

    pugi::xml_document    m_xmlMsgBoxTempl;
    pugi::xml_document    m_xmlEditMenu;
};

}//namespace SOUI