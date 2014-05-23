#include "duistd.h"
#include "DuiSystem.h"
#include "SimpleWnd.h"
#include "DuiRichEdit.h"

#include "duifontpool.h"
#include "duiimgpool.h"
#include "DuiThreadActiveWndMgr.h"
#include "DuiWindowMgr.h"

#include "mybuffer.h"
#include "DuiImgDecoder_Def.h"

namespace SOUI
{

template<> DuiSystem* Singleton<DuiSystem>::ms_Singleton = 0;

DuiSystem::DuiSystem(HINSTANCE hInst,LPCTSTR pszHostClassName/*=_T("DuiHostWnd")*/)
    :m_hInst(hInst)
    ,m_pLogger(NULL)
    ,m_pScriptModule(NULL)
    ,m_pImgDecoder(NULL)
    ,m_pDefImgDecoder(new CDuiImgDecoder_Def)
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
    delete m_pDefImgDecoder;
}

void DuiSystem::createSingletons()
{
    new DuiThreadActiveWndMgr();
    new DuiWindowMgr();
    new CDuiTimerEx();
    new DuiFontPool();
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

void DuiSystem::logEvent( LPCTSTR message, LoggingLevel level /*= Standard*/ )
{
    if(m_pLogger) m_pLogger->logEvent(message,level);
}

void DuiSystem::logEvent(LoggingLevel level , LPCTSTR pszFormat, ...)
{
    if(!m_pLogger) return;
    TCHAR szBuffer[1025] = { 0 };
    va_list argList;
    va_start(argList, pszFormat);
    ::wvnsprintf(szBuffer,ARRAYSIZE(szBuffer)-1, pszFormat, argList);
    va_end(argList);
    m_pLogger->logEvent(szBuffer,level);
}

BOOL DuiSystem::Init( LPCTSTR pszName ,LPCTSTR pszType/*=DUIRES_XML_TYPE*/ )
{
    pugi::xml_document xmlDoc;
    if(!LOADXML(xmlDoc,pszName,pszType)) return FALSE;
    //init edit menu
    pugi::xml_node xmlMenu=xmlDoc.first_child().child("editmenu");
    if(xmlMenu)
    {
        m_xmlEditMenu.append_copy(xmlMenu);
    }

    //set default font
    pugi::xml_node xmlFont;
    xmlFont=xmlDoc.first_child().child("font");
    if(xmlFont)
    {
        int nSize=xmlFont.attribute("size").as_int(12);
        DuiFontPool::getSingleton().SetDefaultFont(DUI_CA2T(xmlFont.attribute("face").value(),CP_UTF8),nSize);
    }

    GetCurResMgr()->Init(xmlDoc.first_child());

    return TRUE;
}

BOOL DuiSystem::SetMsgBoxTemplate( LPCTSTR pszXmlName,LPCTSTR pszType/*=DUIRES_XML_TYPE*/ )
{
    if(!LOADXML(m_xmlMsgBoxTempl,pszXmlName,pszType)) goto format_error;
    if(!m_xmlMsgBoxTempl.child("SOUI").attribute("frame_size").value()[0]) goto format_error;
    if(!m_xmlMsgBoxTempl.child("SOUI").attribute("minsize").value()[0]) goto format_error;

    return TRUE;
format_error:
    m_xmlMsgBoxTempl.reset();
    return FALSE;
}

BOOL DuiSystem::LoadXmlDocment( pugi::xml_document & xmlDoc,LPCTSTR pszXmlName ,LPCTSTR pszType/*=DUIRES_XML_TYPE*/ )
{

    DWORD dwSize=GETRESPROVIDER->GetRawBufferSize(pszType,pszXmlName);
    if(dwSize==0) return FALSE;

    CMyBuffer<char> strXml;
    strXml.Allocate(dwSize);
    GETRESPROVIDER->GetRawBuffer(pszType,pszXmlName,strXml,dwSize);

    pugi::xml_parse_result result= xmlDoc.load_buffer(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8);
    DUIRES_ASSERTA(result,"parse xml error! xmlName=%s,desc=%s,offset=%d",pszXmlName,result.description(),result.offset);
    return result;

}

}//namespace SOUI