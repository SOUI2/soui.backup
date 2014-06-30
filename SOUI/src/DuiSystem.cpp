#include "duistd.h"
#include "DuiSystem.h"
#include "SimpleWnd.h"
#include "control/DuiRichEdit.h"

#include "res.mgr/duifontpool.h"
#include "res.mgr/duiimgpool.h"
#include "DuiThreadActiveWndMgr.h"
#include "DuiWindowMgr.h"

#include "mybuffer.h"
#include "DuiTimerEx.h"
#include "control/duimessagebox.h"
namespace SOUI
{

template<> DuiSystem* Singleton<DuiSystem>::ms_Singleton = 0;

DuiSystem::DuiSystem(IRenderFactory *pRendFactory,HINSTANCE hInst,LPCTSTR pszHostClassName/*=_T("DuiHostWnd")*/)
    :m_hInst(hInst)
    ,m_pScriptModule(NULL)
    ,m_RenderFactory(pRendFactory)
{
    createSingletons();
    CSimpleWndHelper::Init(hInst,pszHostClassName);
    CDuiTextServiceHelper::Init();
}

DuiSystem::~DuiSystem(void)
{
    destroySingletons();
    CSimpleWndHelper::Destroy();
    CDuiTextServiceHelper::Destroy();
}

void DuiSystem::createSingletons()
{
    new DuiThreadActiveWndMgr();
    new DuiWindowMgr();
    new CDuiTimerEx();
    new DuiFontPool(m_RenderFactory);
    new DuiImgPool();
}

void DuiSystem::destroySingletons()
{
    delete DuiImgPool::getSingletonPtr();
    delete DuiFontPool::getSingletonPtr();
    delete CDuiTimerEx::getSingletonPtr();
    delete DuiThreadActiveWndMgr::getSingletonPtr();
    delete DuiWindowMgr::getSingletonPtr();
}

BOOL DuiSystem::Init( LPCTSTR pszName ,LPCTSTR pszType)
{
    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszName,pszType)) return FALSE;
    pugi::xml_node root=xmlDoc.child(L"UIDEF");
    if(!root) return FALSE;

    //init edit menu
    pugi::xml_node xmlMenu=root.child(L"editmenu");
    if(xmlMenu)
    {
        m_xmlEditMenu.append_copy(xmlMenu);
    }

    //set default font
    pugi::xml_node xmlFont;
    xmlFont=root.child(L"font");
    if(xmlFont)
    {
        int nSize=xmlFont.attribute(L"size").as_int(12);
        DuiFontPool::getSingleton().SetDefaultFont(DUI_CW2T(xmlFont.attribute(L"face").value()),nSize);
    }

    DuiPools::Init(root);

    return TRUE;
}

BOOL DuiSystem::SetMsgBoxTemplate( LPCTSTR pszXmlName,LPCTSTR pszType)
{
    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszXmlName,pszType)) return FALSE;
    pugi::xml_node uiRoot=xmlDoc.child(L"UIFRAME");   
    return SMessageBoxImpl::SetMsgTemplate(uiRoot);
}

BOOL DuiSystem::LoadXmlDocment( pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType/*=DUIRES_XML_TYPE*/ )
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