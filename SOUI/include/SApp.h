#pragma once
#include "core/ssingleton.h"
#include "render/render-i.h"
#include "unknown/obj-ref-impl.hpp"

#include "core/smsgloop.h"
#include "core/SWndFactoryMgr.h"
#include "core/SSkinFactoryMgr.h"

#include "core/SScriptModule-i.h"
#include "res.mgr/SResProviderMgr.h"
#include "res.mgr/SPools.h"

#define SOUI_VERSION    _T("0.0.0.1")

#define GETSKIN(p1) SApplication::getSingleton().GetSkin(p1)
#define GETSTYLE(p1,p2) SApplication::getSingleton().GetStyle(p1,p2)
#define BUILDSTRING(p1) SApplication::getSingleton().BuildString(p1)
#define GETCSS(p1) SApplication::getSingleton().GetDefAttribute(p1)

#define LOADXML(p1,p2,p3) SApplication::getSingleton().LoadXmlDocment(p1,p2,p3)
#define GETRESPROVIDER    SApplication::getSingletonPtr()
#define GETRENDERFACTORY SApplication::getSingleton().GetRenderFactory()

#define RT_UIDEF _T("UIDEF")
#define RT_LAYOUT _T("LAYOUT")
#define RT_XML _T("XML")

namespace SOUI
{

class SOUI_EXP SApplication :public SSingleton<SApplication>
                        ,public SWindowFactoryMgr
                        ,public SSkinFactoryMgr
                        ,public SResProviderMgr
                        ,public SPools
{
    friend class CSimpleWnd;
public:
    SApplication(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName=_T("SOUIHOST"));
    ~SApplication(void);


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

    SMessageLoop  * GetMessageLoop(){return m_lstMsgLoop.GetTail();}
    
    void PushMessageLoop(SMessageLoop* pMsgLoop)
    {
        m_lstMsgLoop.AddTail(pMsgLoop);
    }
    
    SMessageLoop * PopMessageLoop()
    {
        return m_lstMsgLoop.RemoveTail();
    }
protected:
    void createSingletons();
    void destroySingletons();

    IScriptModule        * m_pScriptModule;
    HINSTANCE m_hInst;
    
    SList<SMessageLoop*> m_lstMsgLoop;
    SMessageLoop         m_msgLoop;
    CAutoRefPtr<IRenderFactory> m_RenderFactory;
};

}//namespace SOUI