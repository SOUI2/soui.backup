#include "souistd.h"
#include "SApp.h"
#include "core/SimpleWnd.h"
#include "core/SWindowMgr.h"
#include "core/SThreadActiveWndMgr.h"
#include "core/mybuffer.h"

#include "res.mgr/sfontpool.h"
#include "res.mgr/simgpool.h"

#include "helper/STimerEx.h"

#include "control/SRichEdit.h"
#include "control/Smessagebox.h"
namespace SOUI
{

template<> SApplication* SSingleton<SApplication>::ms_Singleton = 0;

SApplication::SApplication(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName/*=_T("DuiHostWnd")*/)
    :m_hInst(hInst)
    ,m_pScriptModule(NULL)
    ,m_RenderFactory(pRendFactory)
{
    createSingletons();
    CSimpleWndHelper::Init(hInst,pszHostClassName);
    CDuiTextServiceHelper::Init();
    SRicheditMenuDef::Init();
    m_lstMsgLoop.AddTail(&m_msgLoop);
}

SApplication::~SApplication(void)
{
    destroySingletons();
    CSimpleWndHelper::Destroy();
    CDuiTextServiceHelper::Destroy();
    SRicheditMenuDef::Destroy();
}

void SApplication::createSingletons()
{
    new SThreadActiveWndMgr();
    new SWindowMgr();
    new STimerEx();
    new SFontPool(m_RenderFactory);
    new SImgPool();
}

void SApplication::destroySingletons()
{
    delete SImgPool::getSingletonPtr();
    delete SFontPool::getSingletonPtr();
    delete STimerEx::getSingletonPtr();
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
        SFontPool::getSingleton().SetDefaultFont(DUI_CW2T(xmlFont.attribute(L"face").value()),nSize);
    }

    SPools::Init(root);

    return TRUE;
}

BOOL SApplication::SetMsgBoxTemplate( LPCTSTR pszXmlName,LPCTSTR pszType)
{
    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszXmlName,pszType)) return FALSE;
    pugi::xml_node uiRoot=xmlDoc.child(L"SOUI");   
    return SMessageBoxImpl::SetMsgTemplate(uiRoot);
}

BOOL SApplication::LoadXmlDocment( pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType/*=DUIRES_XML_TYPE*/ )
{

    DWORD dwSize=GETRESPROVIDER->GetRawBufferSize(pszType,pszXmlName);
    if(dwSize==0) return FALSE;

    CMyBuffer<char> strXml;
    strXml.Allocate(dwSize);
    GETRESPROVIDER->GetRawBuffer(pszType,pszXmlName,strXml,dwSize);

    pugi::xml_parse_result result= xmlDoc.load_buffer(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8);
    DUIRES_ASSERTW(result,L"parse xml error! xmlName=%s,desc=%s,offset=%d",pszXmlName,result.description(),result.offset);
    return result;

}


}//namespace SOUI