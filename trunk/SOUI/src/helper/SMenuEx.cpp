#include "include\souistd.h"
#include "helper\SMenuEx.h"
#include "helper\SplitString.h"

namespace SOUI
{
    #define TIMERID_POPSUBMENU  100
    #define TIME_PUPSUBMENU     500
    //////////////////////////////////////////////////////////////////////////

    class SMenuExRoot;
    class SMenuExItem : public SWindow
    {
        SOUI_CLASS_NAME(SMenuExItem,L"menuItem")
    public:
        SMenuExItem(SMenuEx *pOwnerMenu,ISkinObj *pItemSkin)
        :m_pSubMenu(NULL)
        ,m_pOwnerMenu(pOwnerMenu)
        ,m_nItemID(0)
        {
            m_pBgSkin = pItemSkin;
            m_style.m_bTrackMouseEvent=TRUE;
        }
        
        ~SMenuExItem()
        {
            if(m_pSubMenu) 
            {
                //m_pSubMenu->DestroyWindow();
                delete m_pSubMenu;
            }
        }
        
        int GetMenuItemID() const 
        {
            return m_nItemID;
        }
        
        SMenuEx * GetSubMenu()
        {
            return m_pSubMenu;
        }
        
        virtual BOOL CreateChildren(pugi::xml_node xmlNode)
        {
            __super::CreateChildren(xmlNode);
            pugi::xml_node xmlChild = xmlNode.child(SMenuExItem::GetClassName());
            if(xmlChild)
            {//有子菜单
                m_pSubMenu = new SMenuEx(m_pOwnerMenu);
                m_pSubMenu->LoadMenu(xmlNode);
            }
            return TRUE;
        }

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"itemID",m_nItemID,FALSE)
        SOUI_ATTRS_END()
    protected:
        SMenuEx * m_pSubMenu;
        SMenuEx * m_pOwnerMenu;
        
        int       m_nItemID;
    };
    
    //////////////////////////////////////////////////////////////////////////
    class SMenuExRoot : public SWindow
    {
    SOUI_CLASS_NAME(SMenuExRoot,L"menuRoot")
    friend class SMenuEx;
    friend class SMenuExItem;
    protected:
    ISkinObj * m_pItemSkin;
    ISkinObj * m_pSepSkin;
    ISkinObj * m_pCheckSkin;
    SMenuEx  * m_pMenuEx;
    
    int         m_nItemHei;
    
    SOUI_ATTRS_BEGIN()
        ATTR_SKIN(L"itemSkin",m_pItemSkin,FALSE)
        ATTR_SKIN(L"sepSkin",m_pSepSkin,FALSE)
        ATTR_SKIN(L"checkSkin",m_pCheckSkin,FALSE)
        ATTR_INT(L"itemHeight",m_nItemHei,FALSE)
    SOUI_ATTRS_END()

    public:
    SMenuExRoot(SMenuEx * pMenuEx)
        :m_pItemSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SKIN))
        ,m_pSepSkin(GETBUILTINSKIN(SKIN_SYS_MENU_SEP))
        ,m_pCheckSkin(GETBUILTINSKIN(SKIN_SYS_MENU_CHECK))
        ,m_nItemHei(26)
        ,m_pMenuEx(pMenuEx)
    {
    
    }
    
    virtual BOOL InitFromXml(pugi::xml_node xmlNode)
    {
        BOOL bRet = __super::InitFromXml(xmlNode);
        
        //找到根节点，获取在根节点上配置的全局菜单对象属性
        pugi::xml_node xmlRoot = xmlNode.root().first_child();
        if(xmlNode != xmlRoot)
        {
            SObject::InitFromXml(xmlRoot);
        }
        return bRet;
    }    
    
    virtual BOOL CreateChildren(pugi::xml_node xmlNode)
    {
        pugi::xml_node xmlItem = xmlNode.child(SMenuExItem::GetClassName());
        while(xmlItem)
        {
            SMenuExItem *pMenuItem = new SMenuExItem(m_pMenuEx,m_pItemSkin);
            InsertChild(pMenuItem);
            pMenuItem->InitFromXml(xmlItem);
            pMenuItem->SetAttribute(L"pos",L"0,[0",TRUE);
            xmlItem = xmlItem.next_sibling(SMenuExItem::GetClassName());
        }
        return TRUE;
    }
    
    };

    //////////////////////////////////////////////////////////////////////////
    class SMenuExRunData
    {
    public:
        SMenuExRunData():m_bExit(FALSE),m_nCmdID(-1)
        {
        
        }
        
        BOOL IsMenuWnd(HWND hWnd)
        {
            SPOSITION pos = m_lstMenuWnd.GetTailPosition();
            while(pos)
            {
                if(m_lstMenuWnd.GetPrev(pos) == hWnd) return TRUE;
            }
            return FALSE;
        }
        
        void PushMenuWnd(HWND hWnd)
        {
            m_lstMenuWnd.AddTail(hWnd);
        }
        
        HWND GetMenuWnd()
        {
            if(m_lstMenuWnd.IsEmpty()) return 0;
            return m_lstMenuWnd.GetTail();
        }
        
        HWND PopMenuWnd()
        {
            return m_lstMenuWnd.RemoveTail();
        }
        
        BOOL IsMenuExited()
        {
            return m_bExit;
        }
        
        void ExitMenu(int nCmdID)
        {
            m_bExit=TRUE;
            m_nCmdID = nCmdID;
        }
        
        int GetCmdID(){return m_nCmdID;}
    protected:
        SList<HWND> m_lstMenuWnd;
        
        BOOL m_bExit;
        int  m_nCmdID;
    };
    
    static SMenuExRunData * s_MenuData=NULL;
    
    //////////////////////////////////////////////////////////////////////////
    SMenuEx::SMenuEx(void):m_pParent(NULL),m_pHoverItem(NULL)
    {
    }

    SMenuEx::SMenuEx(SMenuEx *pParent):m_pParent(pParent),m_pHoverItem(NULL)
    {

    }

    SMenuEx::~SMenuEx(void)
    {
        if(IsWindow())
            DestroyWindow();
    }

    BOOL SMenuEx::LoadMenu(LPCTSTR pszMenu)
    {
        SStringTList strMenu;
        if(1==SplitString<SStringT,TCHAR>(pszMenu,_T(':'),strMenu))
            strMenu.InsertAt(0,_T("SMENUEX"));
        
        pugi::xml_document xmlMenu;
        BOOL bLoad = LOADXML(xmlMenu,strMenu[1],strMenu[0]);
        if(!bLoad) return FALSE;
        return LoadMenu(xmlMenu.first_child());
    }

    BOOL SMenuEx::LoadMenu(pugi::xml_node xmlNode)
    {
        if(IsWindow()) return FALSE;
        if(xmlNode.name() != SStringW(SMenuExRoot::GetClassName())
         && xmlNode.name() != SStringW(SMenuExItem::GetClassName()))
            return FALSE;
            
        HWND hWnd = Create(NULL,WS_POPUP,WS_EX_TOOLWINDOW,0,0,0,0);
        if(!hWnd) return FALSE;
        
        
        SMenuExRoot *pMenuRoot = new SMenuExRoot(this);
        InsertChild(pMenuRoot);
        
        pMenuRoot->InitFromXml(xmlNode);
        pMenuRoot->GetLayout()->SetFitContent(PD_ALL);

        return TRUE;
    }

    UINT SMenuEx::TrackPopupMenu(UINT flag,int x,int y,HWND hOwner)
    {
        if(!IsWindow()) return -1;
        s_MenuData = new SMenuExRunData;

        ShowMenu(flag,x,y);
        RunMenu(hOwner);
        HideMenu();
        
        int nRet = s_MenuData->GetCmdID();
        delete s_MenuData;
        s_MenuData=NULL;
        
        if(flag & TPM_RETURNCMD)
        {
            return nRet;
        }else
        {
            ::SendMessage(hOwner,WM_COMMAND,MAKEWPARAM(nRet,0),0);
            return TRUE;
        }
        
    }

    void SMenuEx::ShowMenu(UINT uFlag,int x,int y)
    {
        CRect rcContainer(0,0,10000,10000);
        SWindow *pMenuRoot = GetRoot()->GetWindow(GSW_FIRSTCHILD);
        CSize szMenu = pMenuRoot->GetDesiredSize(&rcContainer);
        pMenuRoot->Move(CRect(CPoint(),szMenu));
        if(uFlag&TPM_CENTERALIGN)
        {
            x -= szMenu.cx/2;
        }else if(uFlag & TPM_RIGHTALIGN)
        {
            x -= szMenu.cx;
        }
        
        if(uFlag & TPM_VCENTERALIGN)
        {
            y -= szMenu.cy/2;
        }
        else if(uFlag&TPM_BOTTOMALIGN)
        {
            y -= szMenu.cy;
        }
        SetWindowPos(HWND_TOPMOST,x,y,szMenu.cx,szMenu.cy,SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);        
        s_MenuData->PushMenuWnd(m_hWnd);
    }

    void SMenuEx::HideMenu()
    {
        ShowWindow(SW_HIDE);
        s_MenuData->PopMenuWnd();
    }

    int SMenuEx::OnMouseActivate(HWND wndTopLevel, UINT nHitTest, UINT message)
    {
        return MA_NOACTIVATE;
    }


    void SMenuEx::RunMenu(HWND hOwner)
    {
        SASSERT(s_MenuData);
        
        
        SetForegroundWindow(hOwner);

        BOOL bMsgQuit(FALSE);
        
        for(;;)
        {

            if(s_MenuData->IsMenuExited())
            {
                break;
            }

            if(GetForegroundWindow() != hOwner)
            {
                break;
            }
            BOOL bInterceptOther(FALSE);
            MSG msg = {0};
            if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if(msg.message == WM_KEYDOWN
                    || msg.message == WM_SYSKEYDOWN
                    || msg.message == WM_KEYUP
                    || msg.message == WM_SYSKEYUP
                    || msg.message == WM_CHAR
                    || msg.message == WM_IME_CHAR)
                {
                    //transfer the message to menu window
                    msg.hwnd = s_MenuData->GetMenuWnd();
                }
                else if(msg.message == WM_LBUTTONDOWN
                    || msg.message  == WM_RBUTTONDOWN
                    || msg.message  == WM_NCLBUTTONDOWN
                    || msg.message  == WM_NCRBUTTONDOWN
                    ||msg.message   ==WM_LBUTTONDBLCLK)
                {
                    //click on other window
                    if(!s_MenuData->IsMenuWnd(msg.hwnd))
                    {
                        bInterceptOther = true;
                    }
                }else if (msg.message == WM_LBUTTONUP
                    ||msg.message==WM_RBUTTONUP
                    ||msg.message==WM_NCLBUTTONUP
                    ||msg.message==WM_NCRBUTTONUP
                    ||msg.message==WM_CONTEXTMENU)
                {
                    if(!s_MenuData->IsMenuWnd(msg.hwnd))
                    {
                        break;
                    }

                }
                else if(msg.message == WM_QUIT)
                {

                    bMsgQuit = TRUE;
                }

                //拦截非菜单窗口的MouseMove消息
                if (msg.message == WM_MOUSEMOVE)
                {
                    if (!s_MenuData->IsMenuWnd(msg.hwnd))
                    {
                        bInterceptOther=TRUE;
                    }
                }


                if (!bInterceptOther)
                {
                    TranslateMessage (&msg);
                    DispatchMessage (&msg);
                }

            }
            else
            {
                MsgWaitForMultipleObjects (0, 0, 0, 10, QS_ALLINPUT);
            }

            if(bMsgQuit)
            {
                PostQuitMessage(msg.wParam);
                break;
            }
        }
        
    }

    BOOL SMenuEx::_HandleEvent(EventArgs *pEvt)
    {
        if(pEvt->sender->IsClass(SMenuExItem::GetClassName()))
        {
            SMenuExItem *pMenuItem = sobj_cast<SMenuExItem>(pEvt->sender);
            if(pEvt->GetID() == EventSwndMouseHover::EventID)
            {
                if(pMenuItem->GetSubMenu() != NULL)
                {
                    CSimpleWnd::SetTimer(TIMERID_POPSUBMENU,TIME_PUPSUBMENU);
                    m_pHoverItem = pMenuItem;
                }
                return FALSE;
            }else if(pEvt->GetID() == EventSwndMouseLeave::EventID)
            {
                if(pMenuItem->GetSubMenu() != NULL)
                {
                    CSimpleWnd::KillTimer(TIMERID_POPSUBMENU);
                    m_pHoverItem=NULL;
                }
                return FALSE;
            }
            
            if(pEvt->GetID() != EventCmd::EventID) return FALSE;
            SASSERT(pMenuItem);
            if(pMenuItem->GetSubMenu())
            {
                PopupSubMenu(pMenuItem);
                return FALSE;
            }else if(pMenuItem->GetMenuItemID()==0)
            {
                return FALSE;
            }
            s_MenuData->ExitMenu(pMenuItem->GetMenuItemID());
            return TRUE;
        }else if(m_pOwner)
        {
            return m_pOwner->GetContainer()->OnFireEvent(*pEvt);
        }else
        {
            return FALSE;
        }
    }

    void SMenuEx::OnTimer(UINT_PTR timeID)
    {
        if(timeID == TIMERID_POPSUBMENU)
        {
            PopupSubMenu(m_pHoverItem);
        }else
        {
            SetMsgHandled(FALSE);
        }
    }

    void SMenuEx::PopupSubMenu(SMenuExItem * pItem)
    {
        CSimpleWnd::KillTimer(TIMERID_POPSUBMENU);

        SMenuEx * pSubMenu = pItem->GetSubMenu();
        SASSERT(pSubMenu);
        CRect rcItem = pItem->GetWindowRect();
        ClientToScreen(&rcItem);
        HMONITOR hMor = MonitorFromWindow(m_hWnd,MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi={sizeof(MONITORINFO),0};
        GetMonitorInfo(hMor,&mi);
        
        pSubMenu->ShowMenu(0,rcItem.right,rcItem.top);
        
    }

}
