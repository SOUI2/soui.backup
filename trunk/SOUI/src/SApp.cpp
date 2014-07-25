#include "souistd.h"
#include "SApp.h"
#include "core/SimpleWnd.h"
#include "core/SWindowMgr.h"
#include "core/SThreadActiveWndMgr.h"
#include "core/mybuffer.h"

#include "res.mgr/sfontpool.h"
#include "res.mgr/simgpool.h"
#include "res.mgr/SStringPool.h"
#include "res.mgr/SSkinPool.h"
#include "res.mgr/SStylePool.h"
#include "res.mgr/SObjDefAttr.h"

#include "helper/STimerEx.h"

#include "control/SRichEdit.h"
#include "control/Smessagebox.h"
namespace SOUI
{

class SNullTranslator : public TObjRefImpl<ITranslator>
{
public:
    BOOL CreateLang(ILang **pLang){return FALSE;}
    BOOL InstallLang(ILang * pLang){return FALSE;}
    BOOL UninstallLang(REFGUID id){return FALSE;}
    SStringW tr(const SStringW & strSrc,const SStringW & strCtx)
    {
        return strSrc;
    } 
};

template<> SApplication* SSingleton<SApplication>::ms_Singleton = 0;

SApplication::SApplication(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName)
    :m_hInst(hInst)
    ,m_RenderFactory(pRendFactory)
{
    createSingletons();
    CSimpleWndHelper::Init(hInst,pszHostClassName);
    STextServiceHelper::Init();
    SRicheditMenuDef::Init();
    m_Translator.Attach(new SNullTranslator);
}

SApplication::~SApplication(void)
{
    destroySingletons();
    CSimpleWndHelper::Destroy();
    STextServiceHelper::Destroy();
    SRicheditMenuDef::Destroy();
}

void SApplication::createSingletons()
{
    new SThreadActiveWndMgr();
    new SWindowMgr();
    new STimer2();
    new SFontPool(m_RenderFactory);
    new SImgPool();
    new SStringPool();
    new SSkinPool();
    new SStylePool();
    new SObjDefAttr();
}

void SApplication::destroySingletons()
{
    delete SObjDefAttr::getSingletonPtr();
    delete SStylePool::getSingletonPtr();
    delete SSkinPool::getSingletonPtr();
    delete SStringPool::getSingletonPtr();
    delete SImgPool::getSingletonPtr();
    delete SFontPool::getSingletonPtr();
    delete STimer2::getSingletonPtr();
    delete SThreadActiveWndMgr::getSingletonPtr();
    delete SWindowMgr::getSingletonPtr();
}

BOOL SApplication::Init( LPCTSTR pszName ,LPCTSTR pszType)
{
    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszName,pszType)) return FALSE;
    pugi::xml_node root=xmlDoc.child(L"UIDEF");
    if(!root) return FALSE;

    //init edit menu
    pugi::xml_node xmlMenu=root.child(L"editmenu");
    if(xmlMenu)
    {
        SRicheditMenuDef::getSingleton().SetMenuXml(xmlMenu);
    }

    //set default font
    pugi::xml_node xmlFont;
    xmlFont=root.child(L"font");
    if(xmlFont)
    {
        int nSize=xmlFont.attribute(L"size").as_int(12);
        SFontPool::getSingleton().SetDefaultFont(S_CW2T(xmlFont.attribute(L"face").value()),nSize);
    }
    
    SStringPool::getSingleton().Init(root.child(L"string"));
    SSkinPool::getSingleton().LoadSkins(root.child(L"skins"));
    SStylePool::getSingleton().Init(root.child(L"style"));
    SObjDefAttr::getSingleton().Init(root.child(L"objattr"));
    return TRUE;
}

BOOL SApplication::SetMsgBoxTemplate( LPCTSTR pszXmlName,LPCTSTR pszType)
{
    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszXmlName,pszType)) return FALSE;
    pugi::xml_node uiRoot=xmlDoc.child(L"SOUI");   
    return SMessageBoxImpl::SetMsgTemplate(uiRoot);
}

BOOL SApplication::LoadXmlDocment( pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType )
{

    DWORD dwSize=GETRESPROVIDER->GetRawBufferSize(pszType,pszXmlName);
    if(dwSize==0) return FALSE;

    CMyBuffer<char> strXml;
    strXml.Allocate(dwSize);
    GETRESPROVIDER->GetRawBuffer(pszType,pszXmlName,strXml,dwSize);

    pugi::xml_parse_result result= xmlDoc.load_buffer(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8);
    ASSERT_FMTW(result,L"parse xml error! xmlName=%s,desc=%s,offset=%d",pszXmlName,result.description(),result.offset);
    return result;

}

int SApplication::Run( HWND hMainWnd )
{
    SThreadActiveWndMgr::SetActive(hMainWnd);
    int nRet=SMessageLoop::Run();
    SThreadActiveWndMgr::SetActive(NULL);
    if(::IsWindow(hMainWnd)) DestroyWindow(hMainWnd);
    return nRet;
}

LPCTSTR SApplication::GetVersion()
{
	return SOUI_VERSION;
}

HINSTANCE SApplication::GetInstance()
{
	return m_hInst;
}

void SApplication::SetTranslator(ITranslator * pTrans)
{
	m_Translator = pTrans;
}

ITranslator * SApplication::GetTranslator()
{
	return m_Translator;
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

}//namespace SOUI