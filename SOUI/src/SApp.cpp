#include "souistd.h"
#include "SApp.h"
#include "core/SimpleWnd.h"
#include "core/SWindowMgr.h"
#include "core/SThreadActiveWndMgr.h"

#include "res.mgr/sfontpool.h"
#include "res.mgr/SStringPool.h"
#include "res.mgr/SSkinPool.h"
#include "res.mgr/SStylePool.h"
#include "res.mgr/SObjDefAttr.h"

#include "helper/STimerEx.h"
#include "helper/mybuffer.h"
#include "helper/SToolTip.h"
#include "helper/AppDir.h"

#include "control/SRichEdit.h"
#include "control/Smessagebox.h"
#include "updatelayeredwindow/SUpdateLayeredWindow.h"

namespace SOUI
{

class SNullTranslator : public TObjRefImpl<ITranslatorMgr>
{
public:
    BOOL CreateTranslator(ITranslator **pLang){return FALSE;}
    BOOL InstallTranslator(ITranslator * pLang){return FALSE;}
    BOOL UninstallTranslator(REFGUID id){return FALSE;}
    SStringW tr(const SStringW & strSrc,const SStringW & strCtx)
    {
        return strSrc;
    } 
};

class SDefToolTipFactory : public TObjRefImpl<IToolTipFactory>
{
public:
    /*virtual */IToolTip * CreateToolTip(HWND hHost)
    {
        STipCtrl *pTipCtrl = new STipCtrl;
        if(!pTipCtrl->Create())
        {
            delete pTipCtrl;
            return NULL;
        }
        return pTipCtrl;
    }

    /*virtual */void DestroyToolTip(IToolTip *pToolTip)
    {
        if(pToolTip)
        {
            STipCtrl *pTipCtrl= (STipCtrl *)pToolTip;
            pTipCtrl->DestroyWindow();
        }
    }
};

//////////////////////////////////////////////////////////////////////////
// SApplication

template<> SApplication* SSingleton<SApplication>::ms_Singleton = 0;

SApplication::SApplication(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName)
    :m_hInst(hInst)
    ,m_RenderFactory(pRendFactory)
{
    SWndSurface::Init();
    _CreateSingletons();
    CSimpleWndHelper::Init(m_hInst,pszHostClassName);
    STextServiceHelper::Init();
    SRicheditMenuDef::Init();
    m_translator.Attach(new SNullTranslator);
    m_tooltipFactory.Attach(new SDefToolTipFactory);
    
    SAppDir appDir(hInst);
    m_strAppDir = appDir.AppDir();
}

SApplication::~SApplication(void)
{
    _DestroySingletons();
    CSimpleWndHelper::Destroy();
    STextServiceHelper::Destroy();
    SRicheditMenuDef::Destroy();
}

void SApplication::_CreateSingletons()
{
    new SThreadActiveWndMgr();
    new SWindowMgr();
    new STimer2();
    new SFontPool(m_RenderFactory);
    new SStringPool();
    new SNamedID();
    new SSkinPoolMgr();
    new SStylePoolMgr();
    new SObjDefAttr();
}

void SApplication::_DestroySingletons()
{
    delete SObjDefAttr::getSingletonPtr();
    delete SStylePoolMgr::getSingletonPtr();
    delete SSkinPoolMgr::getSingletonPtr();
    delete SNamedID::getSingletonPtr();
    delete SStringPool::getSingletonPtr();
    delete SFontPool::getSingletonPtr();
    delete STimer2::getSingletonPtr();
    delete SThreadActiveWndMgr::getSingletonPtr();
    delete SWindowMgr::getSingletonPtr();
}

BOOL SApplication::_LoadXmlDocment( LPCTSTR pszXmlName ,LPCTSTR pszType ,pugi::xml_document & xmlDoc)
{
    DWORD dwSize=GetRawBufferSize(pszType,pszXmlName);
    if(dwSize==0) return FALSE;

    CMyBuffer<char> strXml;
    strXml.Allocate(dwSize);
    GetRawBuffer(pszType,pszXmlName,strXml,dwSize);

    pugi::xml_parse_result result= xmlDoc.load_buffer(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8);
    SASSERT_FMTW(result,L"parse xml error! xmlName=%s,desc=%s,offset=%d",pszXmlName,result.description(),result.offset);
    return result;
}

BOOL SApplication::LoadXmlDocment( pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType )
{
    return _LoadXmlDocment(pszXmlName,pszType,xmlDoc);
}

BOOL SApplication::Init( LPCTSTR pszName ,LPCTSTR pszType)
{
    SASSERT(m_RenderFactory);

    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszName,pszType)) return FALSE;
    pugi::xml_node root=xmlDoc.child(L"UIDEF");
    if(!root) return FALSE;

    //set default font
    pugi::xml_node xmlFont;
    xmlFont=root.child(L"font");
    if(xmlFont)
    {
        int nSize=xmlFont.attribute(L"size").as_int(12);
        BYTE byCharset=(BYTE)xmlFont.attribute(L"charset").as_int(DEFAULT_CHARSET);
        SFontPool::getSingleton().SetDefaultFont(S_CW2T(xmlFont.attribute(L"face").value()),nSize,byCharset);
    }
    
    SStringPool::getSingleton().Init(root.child(L"string"));
    SNamedID::getSingleton().Init(root.child(L"id"));

    SSkinPool *pSkinPool = new SSkinPool;
    pSkinPool->LoadSkins(root.child(L"skin"));
    SSkinPoolMgr::getSingletonPtr()->PushSkinPool(pSkinPool);
    pSkinPool->Release();
    
    SStylePool *pStylePool = new SStylePool;
    pStylePool->Init(root.child(L"style"));
    SStylePoolMgr::getSingleton().PushStylePool(pStylePool);
    pStylePool->Release();
    
    SObjDefAttr::getSingleton().Init(root.child(L"objattr"));
    return TRUE;
}

UINT SApplication::LoadSystemNamedResource( IResProvider *pResProvider )
{
    UINT uRet=0;
    AddResProvider(pResProvider);
    //load system skins
    {
        pugi::xml_document xmlDoc;
        if(_LoadXmlDocment(_T("SYS_XML_SKIN"),_T("XML"),xmlDoc))
        {
            SSkinPool * p= SSkinPoolMgr::getSingletonPtr()->GetBuiltinSkinPool();
            p->LoadSkins(xmlDoc.child(L"skin"));
        }else
        {
            uRet |= 0x01;
        }
    }
    //load edit context menu
    {
        pugi::xml_document xmlDoc;
        if(_LoadXmlDocment(_T("SYS_XML_EDITMENU"),_T("XML"),xmlDoc))
        {
            SRicheditMenuDef::getSingleton().SetMenuXml(xmlDoc.child(L"editmenu"));
        }else
        {
            uRet |= 0x02;
        }
    }
    //load messagebox template
    {
        pugi::xml_document xmlDoc;
        if(!_LoadXmlDocment(_T("SYS_XML_MSGBOX"),_T("XML"),xmlDoc)
        || !SMessageBoxImpl::SetMsgTemplate(xmlDoc.child(L"SOUI")))
        {
            uRet |= 0x04;
        }
    }
    RemoveResProvider(pResProvider);
    return uRet;
}

int SApplication::Run( HWND hMainWnd )
{
    SThreadActiveWndMgr::SetActive(hMainWnd);
    int nRet=SMessageLoop::Run();
    SThreadActiveWndMgr::SetActive(NULL);
    if(::IsWindow(hMainWnd)) DestroyWindow(hMainWnd);
    return nRet;
}

HINSTANCE SApplication::GetInstance()
{
	return m_hInst;
}

void SApplication::SetTranslator(ITranslatorMgr * pTrans)
{
	m_translator = pTrans;
}

ITranslatorMgr * SApplication::GetTranslator()
{
	return m_translator;
}

void SApplication::SetScriptModule(IScriptModule *pScriptModule)
{
	m_pScriptModule = pScriptModule;
}

IScriptModule * SApplication::GetScriptModule()
{
	return m_pScriptModule;
}

IRenderFactory * SApplication::GetRenderFactory()
{
	return m_RenderFactory;
}

void SApplication::SetRealWndHandler( IRealWndHandler *pRealHandler )
{
    m_pRealWndHandler = pRealHandler;
}

IRealWndHandler * SApplication::GetRealWndHander()
{
    return m_pRealWndHandler;
}

IToolTipFactory * SApplication::GetToolTipFactory()
{
    return m_tooltipFactory;
}

void SApplication::SetToolTipFactory( IToolTipFactory* pToolTipFac )
{
    m_tooltipFactory = pToolTipFac;
}

}//namespace SOUI