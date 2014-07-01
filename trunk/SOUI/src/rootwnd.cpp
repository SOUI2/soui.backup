#include "duistd.h"
#include <rootwnd.h>

namespace SOUI
{
    SRootWindow::SRootWindow(void)
        : m_bResizable(FALSE)
        , m_bTranslucent(FALSE)
        , m_bAppWnd(FALSE)
        , m_bToolWnd(FALSE)
        , m_szMin(200, 200)
        , m_szInit(640,480)
        , m_dwStyle(0)
        , m_dwExStyle(0)
    {
    }

    SRootWindow::~SRootWindow(void)
    {
    }

    void SRootWindow::Cleanup()
    {
        m_bResizable=FALSE;
        m_bTranslucent=(FALSE);
        m_bAppWnd=FALSE;
        m_bToolWnd=FALSE;
        m_szMin=CSize(200,200);
        m_szInit=CSize(640,480);
        m_rcMargin=CRect();
        m_dwStyle=0;
        m_dwExStyle=0;
    }

    void SRootWindow::OnDestroy()
    {
        __super::OnDestroy();
        if(!m_strName.IsEmpty())
        {
            DuiSystem::getSingleton().FreeSkins(m_strName);    //free skin only used in the host window
        }
        Cleanup();
    }

    BOOL SRootWindow::InitFromXml( pugi::xml_node xmlNode )
    {
        SWindow::SendMessage(WM_DESTROY);
        
        m_strName=xmlNode.attribute(L"name").value();
        if(!m_strName.IsEmpty())
        {
            xmlNode.remove_attribute(L"name");
            DuiSystem::getSingleton().LoadSkins(m_strName);    //load skin only used in the host window
        }
        SObject::InitFromXml(xmlNode);
        return CreateChildren(xmlNode);
    }

}
