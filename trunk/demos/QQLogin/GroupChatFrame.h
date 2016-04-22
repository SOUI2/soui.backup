// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "extend.ctrls\imre\SImRichedit.h"
#include "helper\SMenuEx.h"
#include "../controls.extend/SListboxex.h"
#include "control\SDropDown.h"
#include <wtypes.h>
#include <vector>

#define WM_MY_TIMER WM_USER+2300
 
typedef std::vector<SStringW> StringList;

class EventItemSelected : public TplEventArgs<EventItemSelected>
{
    SOUI_CLASS_NAME(EventItemSelected,L"on_item_selected")
public:
    EventItemSelected(SObject *pSender):TplEventArgs<EventItemSelected>(pSender){}
    enum{EventID=EVT_EXTERNAL_BEGIN+1};
    int nCurrSelect;
};

// 点击就要选中，是down还是up不要紧
// 回车就要选中
// 上下移动不要关闭
class DropDownListWnd : public SDropDownWnd
{
public:
    DropDownListWnd(ISDropDownOwner* pOwner):SDropDownWnd(pOwner),
        m_pListBox(NULL)
    {
        m_evtSet.addEvent(EVENTID(EventItemSelected));
    }

    BOOL PreTranslateMessage( MSG* pMsg )
    {
        SDropDownWnd::PreTranslateMessage(pMsg);

        if (pMsg->message == WM_MOUSEMOVE && m_pListBox)
        {
            POINT pt = {LOWORD(pMsg->lParam), HIWORD(pMsg->lParam)};
            CRect rc = m_pListBox->GetClientRect();
            if (rc.PtInRect(pt))
            {
                int nItem = (pt.y - rc.top)/ m_pListBox->GetItemHeight();
                m_pListBox->SetCurSel(nItem);
            }
        }

        if(pMsg->message==WM_MOUSEWHEEL || 
            ((pMsg->message == WM_KEYDOWN || pMsg->message==WM_KEYUP) && 
            (pMsg->wParam == VK_UP || pMsg->wParam==VK_DOWN || pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)))
        {
            if (pMsg->wParam==VK_RETURN && m_pListBox)
            {
                EventItemSelected evt(this);
                evt.nCurrSelect = m_pListBox->GetCurSel();
                FireEvent(evt);
            }
            //截获滚轮及上下键消息
            CSimpleWnd::SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
            return TRUE;    
        }

        return FALSE;
    }

    virtual BOOL Create(LPCRECT lpRect,LPVOID lParam,DWORD dwStyle=WS_POPUP,DWORD dwExStyle=WS_EX_TOOLWINDOW|WS_EX_TOPMOST)
    {
        if (!SDropDownWnd::Create(lpRect, lParam, dwStyle, dwExStyle)) 
            return FALSE;

        pugi::xml_document xmlDoc;
        LOADXML(xmlDoc, L"SelectionDropDownWnd", RT_LAYOUT);

        if(xmlDoc)
        {
            InitFromXml(xmlDoc.child(L"SOUI"));
            m_pListBox = FindChildByName2<SListBoxEx>(L"selectionlist");
            m_pListBox->SetOwner(this);
            m_pListBox->SetFocus();
            CSimpleWnd::SetCapture();
        }
        return TRUE;
    }

    void AdjustWindowSize()
    {
        if (!m_pListBox)
        {
            return;
        }

        int nWndWidth  = GetClientRect().Width();
        int nWndHeight = m_pListBox->GetItemCount() * m_pListBox->GetItemHeight();
        if (nWndHeight > 160)
        {
            nWndHeight = 160;
        }

        int nYOffset   = GetClientRect().Height() - m_pListBox->GetClientRect().Height();
        nWndHeight     += nYOffset;         //加上阴影偏移

        SetWindowPos(HWND_TOP,
            0, 0, nWndWidth, nWndHeight,    // left,top,width,height
            SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
    }

    BOOL FireEvent(EventArgs &evt)
    {
        if (evt.GetID() == EventOfPanel::EventID)
        {
            EventOfPanel *pEvtOfPanel = (EventOfPanel*) &evt;
            if(pEvtOfPanel->pOrgEvt->GetID() == EventCmd::EventID)
            {
                EventItemSelected evt(this);
                evt.nCurrSelect = m_pListBox->GetCurSel();
                __super::FireEvent(evt);

                EndDropDown(IDCANCEL);
                return TRUE;
            }
        }
        return SWindow::FireEvent(evt);
    }

private:
    SListBoxEx * m_pListBox;
};


class GroupChatFrame : public SHostWnd
    , public ISDropDownOwner
{
public:
    GroupChatFrame();
    ~GroupChatFrame();

    void OnClose()
    {
        //AnimateHostWindow(200,AW_CENTER|AW_HIDE);
        DestroyWindow();
    }
    void OnMaximize()
    {
        SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
    }
    void OnRestore()
    {
        SendMessage(WM_SYSCOMMAND,SC_RESTORE);
    }
    void OnMinimize()
    {
        SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
    }

    void OnSend();
    void OnSendPicture();
    void OnShowSendTypeMenu();

    void OnSize(UINT nType, CSize size)
    {
        SetMsgHandled(FALSE);
        if(!m_bLayoutInited) return;
        if(nType==SIZE_MAXIMIZED)
        {
            FindChildByName(L"btn_restore")->SetVisible(TRUE);
            FindChildByName(L"btn_max")->SetVisible(FALSE);
        }else if(nType==SIZE_RESTORED)
        {
            FindChildByName(L"btn_restore")->SetVisible(FALSE);
            FindChildByName(L"btn_max")->SetVisible(TRUE);
        }
    }


protected:
    virtual SWindow * GetDropDownOwner();
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    BOOL GetEditorCursorRect(SImRichEdit * pRichEdit, LONG cpStart, LONG cpEnd, CRect &rc);
    void ShowMemberSelectionWnd(const CRect& rcCursor, const SStringW& strKeyWord);

protected:
    bool OnInputEditorChange(SOUI::EventArgs *pEvt);
    bool OnMemberSelected(SOUI::EventArgs *pEvt);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close",OnClose)
        EVENT_NAME_COMMAND(L"send-pic",OnSendPicture)
        EVENT_NAME_COMMAND(L"btn_min",OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max",OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore",OnRestore)
        EVENT_NAME_COMMAND(L"btn_send", OnSend)
        EVENT_NAME_COMMAND(L"btn_showsendmenu", OnShowSendTypeMenu)
   EVENT_MAP_END()	

   BEGIN_MSG_MAP_EX(GroupChatFrame)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()

private:
    BOOL			    m_bLayoutInited;
    SImRichEdit       * m_preRecord;
    SImRichEdit       * m_preInput;
    DropDownListWnd   * m_pMemberSelWnd;     /**< 群聊时@的时候用来选人 */
    StringList          m_lstMembers;
};
