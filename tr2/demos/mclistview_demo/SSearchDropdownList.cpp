#include "stdafx.h"
#include "SSearchDropdownList.h"

namespace SOUI
{
    const wchar_t * KNode_PopupStyle = L"popupStyle";
    const wchar_t * KName_ListView   = L"lv_dropdown";
    const wchar_t * KName_SearchEdit = L"edit_input";
    
    SSearchDropdownList::SSearchDropdownList(void):m_nDropAlign(DROPALIGN_RIGHT),m_nMaxDropHeight(150),m_pDropDownWnd(NULL)
    {
        GetEventSet()->addEvent(EVENTID(EventFillSearchDropdownList));
        GetEventSet()->addEvent(EVENTID(EventDropdownListSelected));
    }

    SSearchDropdownList::~SSearchDropdownList(void)
    {
    }

    bool SSearchDropdownList::OnEditNotify(EventArgs *e)
    {
        EventRENotify *pEvtNotify = sobj_cast<EventRENotify>(e);
        if(pEvtNotify->iNotify == EN_CHANGE)
        {
            if(!m_pDropDownWnd)
            {
                m_pDropDownWnd = new SDropdownList(this);
                m_pDropDownWnd -> Create(m_xmlDropdown.child(KNode_PopupStyle));
                SASSERT(m_pDropDownWnd);
            }

            EventFillSearchDropdownList evt(this);
            evt.strKey = sobj_cast<SEdit>(e->sender)->GetWindowText();
            evt.pDropdownWnd = m_pDropDownWnd;
            FireEvent(evt);
            
            if(evt.bPopup)
            {
                AdjustDropdownList();
            }else
            {
                CloseUp(IDCANCEL);
            }
        }
        return true;
    }

    void SSearchDropdownList::SetDropdownList(IDropdownList *p)
    {
    }

    void SSearchDropdownList::AdjustDropdownList()
    {
        SASSERT(m_pDropDownWnd);
        
        
        CRect rcWnd=GetWindowRect();
        GetContainer()->FrameToHost(rcWnd);

        ClientToScreen(GetContainer()->GetHostHwnd(),(LPPOINT)&rcWnd);
        ClientToScreen(GetContainer()->GetHostHwnd(),((LPPOINT)&rcWnd)+1);

        HMONITOR hMonitor = ::MonitorFromWindow(GetContainer()->GetHostHwnd(), MONITOR_DEFAULTTONULL);
        CRect rcMonitor;
        if (hMonitor)
        {
            MONITORINFO mi = {sizeof(MONITORINFO)};
            ::GetMonitorInfo(hMonitor, &mi);
            rcMonitor = mi.rcMonitor;
        }
        else
        {
            rcMonitor.right   =   GetSystemMetrics(   SM_CXSCREEN   );   
            rcMonitor.bottom  =   GetSystemMetrics(   SM_CYSCREEN   );
        }
        

        CSize szDropdown = m_pDropDownWnd->GetDesiredSize();
        if(szDropdown.cy > m_nMaxDropHeight) szDropdown.cy = m_nMaxDropHeight;
        
        CPoint pt;
        pt.x =  m_nDropAlign == DROPALIGN_LEFT?rcWnd.left : (rcWnd.right - szDropdown.cx);
        
        if(rcWnd.bottom+szDropdown.cy<=rcMonitor.bottom)
        {
            pt.y = rcWnd.bottom;
        }else
        {
            pt.y = rcWnd.top - szDropdown.cy;
        }
        m_pDropDownWnd->SetWindowPos(HWND_TOPMOST,pt.x,pt.y,szDropdown.cx,szDropdown.cy,SWP_SHOWWINDOW|SWP_NOACTIVATE);
        m_pDropDownWnd->CSimpleWnd::SetCapture();
    }

    SWindow * SSearchDropdownList::GetDropDownOwner()
    {
        return this;
    }

    void SSearchDropdownList::OnCreateDropDown(SDropDownWnd *pDropDown)
    {
    }

    void SSearchDropdownList::OnDestroyDropDown(SDropDownWnd *pDropDown)
    {
        if(pDropDown->GetExitCode() == IDOK)
        {
            //selected item
            EventDropdownListSelected evt(this);
            evt.pDropdownWnd = pDropDown;
            evt.nValue =  pDropDown->GetValue();
            FireEvent(evt);
        }
        m_pDropDownWnd = NULL;
    }


    void SSearchDropdownList::CloseUp(UINT uCode)
    {
        if(m_pDropDownWnd)
        {
            m_pDropDownWnd->EndDropDown(uCode);
        }
    }

    BOOL SSearchDropdownList::CreateChildren(pugi::xml_node xmlNode)
    {
        BOOL bRet = __super::CreateChildren(xmlNode);
        if(!bRet) return FALSE;
        SEdit *pEdit = FindChildByName2<SEdit>(KName_SearchEdit);
        if(!pEdit)  {
            return FALSE;
        }
        pEdit->SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
        pEdit->GetEventSet()->subscribeEvent(EVT_RE_NOTIFY,Subscriber(&SSearchDropdownList::OnEditNotify,this));

        m_xmlDropdown.append_copy(xmlNode.child(KNode_PopupStyle));
        return TRUE;
    }

    BOOL SSearchDropdownList::FireEvent(EventArgs &evt)
    {
        if(wcscmp(evt.nameFrom,KName_ListView) == 0)
        {
            if(evt.GetID() == EVT_CMD)
            {
                CloseUp(IDOK);
                return TRUE;
            }
        }
        return __super::FireEvent(evt);
    }

    //////////////////////////////////////////////////////////////////////////
       
    SDropdownList::SDropdownList(ISDropDownOwner * pOwner):SDropDownWnd(pOwner),m_pListView(NULL)
    {

    }
    
    BOOL SDropdownList::Create(pugi::xml_node popupStyle)
    {
        HWND hParent = m_pOwner->GetDropDownOwner()->GetContainer()->GetHostHwnd();
        HWND hWnd=CSimpleWnd::Create(NULL,WS_POPUP,WS_EX_TOPMOST|WS_EX_TOOLWINDOW,0,0,0,0,hParent,0);
        if(!hWnd) return FALSE;
        m_pOwner->OnCreateDropDown(this);
        if(popupStyle) 
        {
            InitFromXml(popupStyle);
            m_pListView =FindChildByName2<SListView>(KName_ListView);
            m_pListView->SetOwner(m_pOwner->GetDropDownOwner());
            m_pListView->SetFocus();
        }
        return TRUE;
    }

    SIZE SDropdownList::GetDesiredSize()
    {
        CRect rcOwner = m_pOwner->GetDropDownOwner()->GetWindowRect();
        CSize szRet = rcOwner.Size();
        SASSERT(m_pListView);
        szRet.cy = m_pListView->GetItemLocator()->GetTotalHeight()+2;
        return szRet;
    }

    int SDropdownList::GetValue() const
    {
        return m_pListView->GetSel();
    }


}
