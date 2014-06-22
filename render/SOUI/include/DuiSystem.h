#pragma once
#include "duisingleton.h"
#include "render/render-i.h"
#include "unknown/obj-ref-impl.hpp"

#include "DuiWndFactoryMgr.h"
#include "DuiSkinFactoryMgr.h"

#include "DuiScriptModule.h"
#include "res.mgr/DuiResProviderMgr.h"
#include "res.mgr/DuiPools.h"

#define SOUI_VERSION    _T("0.0.0.1")

#define GETSKIN(p1) DuiSystem::getSingleton().GetSkin(p1)
#define GETSTYLE(p1,p2) DuiSystem::getSingleton().GetStyle(p1,p2)
#define BUILDSTRING(p1) DuiSystem::getSingleton().BuildString(p1)
#define GETCSS(p1) DuiSystem::getSingleton().GetDefAttribute(p1)

#define LOADXML(p1,p2,p3) DuiSystem::getSingleton().LoadXmlDocment(p1,p2,p3)
#define GETRESPROVIDER    DuiSystem::getSingletonPtr()
#define GETRENDERFACTORY DuiSystem::getSingleton().GetRenderFactory()

#define RT_UIDEF _T("UIDEF")
#define RT_LAYOUT _T("LAYOUT")
#define RT_XML _T("XML")

namespace SOUI
{

class SOUI_EXP DuiSystem :public Singleton<DuiSystem>
                        ,public DuiWindowFactoryMgr
                        ,public DuiSkinFactoryMgr
                        ,public DuiResProviderMgr
                        ,public DuiPools
{
    friend class CSimpleWnd;
    friend class CDuiRichEdit;    //访问右键菜单资源
public:
    DuiSystem(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName=_T("DuiHostWnd"));
    ~DuiSystem(void);


    HINSTANCE GetInstance()
    {
        return m_hInst;
    }

    LPCTSTR GetVersion(){return SOUI_VERSION;}

    IScriptModule * GetScriptModule()
    {
        return m_pScriptModule;
    }

    void SetScriptModule(IScriptModule *pScriptModule)
    {
        m_pScriptModule=pScriptModule;
    }

    BOOL Init(LPCTSTR pszName ,LPCTSTR pszType=RT_UIDEF);

    BOOL SetMsgBoxTemplate(LPCTSTR pszXmlName,LPCTSTR pszType=RT_LAYOUT);

    BOOL LoadXmlDocment(pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType);

    IRenderFactory * GetRenderFactory(){return m_RenderFactory;}

protected:
    pugi::xml_node GetEditMenuTemplate(){return m_xmlEditMenu;}

    void createSingletons();
    void destroySingletons();

    IScriptModule        * m_pScriptModule;
    HINSTANCE m_hInst;

    pugi::xml_document    m_xmlEditMenu;

    CAutoRefPtr<IRenderFactory> m_RenderFactory;
};

}//namespace SOUI