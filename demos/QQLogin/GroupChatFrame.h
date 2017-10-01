// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "imre\SImRichedit.h"
#include "helper\SMenuEx.h"
#include "../controls.extend/SListboxex.h"
#include "control\SDropDown.h"
#include <wtypes.h>
#include <vector>
#include "MenuWrapper.h"
#include <shellapi.h>
#include "imre\ClipboardConverter.h"

class EventItemSelected : public TplEventArgs<EventItemSelected>
{
    SOUI_CLASS_NAME(EventItemSelected, L"on_item_selected")
public:
    EventItemSelected(SObject *pSender) :TplEventArgs<EventItemSelected>(pSender) {}
    enum { EventID = EVT_EXTERNAL_BEGIN + 1 };
    int nCurrSelect;
};

// 点击就要选中，是down还是up不要紧
// 回车就要选中
// 上下移动不要关闭
class DropDownListWnd : public SDropDownWnd
{
public:
    DropDownListWnd(ISDropDownOwner* pOwner) :SDropDownWnd(pOwner),
        m_pListBox(NULL)
    {
        m_evtSet.addEvent(EVENTID(EventItemSelected));
    }

    BOOL PreTranslateMessage(MSG* pMsg)
    {
        SDropDownWnd::PreTranslateMessage(pMsg);

        if (pMsg->message == WM_MOUSEMOVE && m_pListBox)
        {
            POINT pt = { LOWORD(pMsg->lParam), HIWORD(pMsg->lParam) };
            CRect rc = m_pListBox->GetClientRect();
            if (rc.PtInRect(pt))
            {
                int nItem = (pt.y - rc.top) / m_pListBox->GetItemHeight();
                m_pListBox->SetCurSel(nItem);
            }
        }

        if (pMsg->message == WM_MOUSEWHEEL ||
            ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) &&
            (pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)))
        {
            if (pMsg->wParam == VK_RETURN && m_pListBox)
            {
                EventItemSelected evt(this);
                evt.nCurrSelect = m_pListBox->GetCurSel();
                FireEvent(evt);
            }
            //截获滚轮及上下键消息
            CSimpleWnd::SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
            return TRUE;
        }

        return FALSE;
    }

    virtual BOOL Create(LPCRECT lpRect, LPVOID lParam, DWORD dwStyle = WS_POPUP, DWORD dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST)
    {
        if (!SDropDownWnd::Create(lpRect, lParam, dwStyle, dwExStyle))
            return FALSE;

        pugi::xml_document xmlDoc;
        LOADXML(xmlDoc, L"SelectionDropDownWnd", RT_LAYOUT);

        if (xmlDoc)
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

        int nWndWidth = GetClientRect().Width();
        int nWndHeight = m_pListBox->GetItemCount() * m_pListBox->GetItemHeight();
        if (nWndHeight > 160)
        {
            nWndHeight = 160;
        }

        int nYOffset = GetClientRect().Height() - m_pListBox->GetClientRect().Height();
        nWndHeight += nYOffset;         //加上阴影偏移

        SetWindowPos(HWND_TOP,
            0, 0, nWndWidth, nWndHeight,    // left,top,width,height
            SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
    }

    BOOL FireEvent(EventArgs &evt)
    {
        if (evt.GetID() == EventOfPanel::EventID)
        {
            EventOfPanel *pEvtOfPanel = (EventOfPanel*)&evt;
            if (pEvtOfPanel->pOrgEvt->GetID() == EventCmd::EventID)
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

class GroupChatFrame : public SHostWnd, public ISDropDownOwner
{
    class UserInfo
    {
    public:
        UserInfo(LPCWSTR pName, LPCWSTR pAccount, LPCWSTR pAvatarId, LPCWSTR pAvatarPath, LPCWSTR pTitle) :
            Name(pName),
            Account(pAccount),
            AvatarId(pAvatarId),
            AvatarPath(pAvatarPath),
            Title(pTitle)
        {
        }

        SStringW Name;
        SStringW Account;
        SStringW AvatarId;
        SStringW AvatarPath;
        SStringW Title;
    };

public:

    GroupChatFrame();
    ~GroupChatFrame();

protected:

    UserInfo& GetRandomUser();
    void InsertMessages();

    void AddFetchMoreBlock();
    void AddSeparatorBar();
    UINT AddCenterMessage(UserInfo& info, const SStringW& body, UINT insertTo = RECONTENT_LAST);
    UINT AddCenterMessageWhitoutBuggle(UserInfo& info, const SStringW& body, UINT insertTo = RECONTENT_LAST);
    UINT AddLeftMessage(UserInfo& info, const SStringW& body, UINT insertTo = RECONTENT_LAST);
    UINT AddRightMessage(UserInfo& info, const SStringW& body, UINT insertTo = RECONTENT_LAST);
    void AddCustomCenterMessage();
    void ShowMemberSelectionWnd(const CRect& rcCursor, const SStringW& strKeyWord);
    void FillRClickAvatarMenu(MenuWrapper& menu, RichEditContent* pContent);
    void FillRClickImageMenu(MenuWrapper& menu, RichEditContent* pContent);
    void FillRClickFileMenu(MenuWrapper& menu, RichEditContent* pContent);
    void FillRClickSelRegionMenu(MenuWrapper& menu, RichEditContent* pContent);
    void FillRClickBubbleMenu(MenuWrapper& menu, RichEditContent* pContent);
    void FillRClickNothingMenu(MenuWrapper& menu);
    void DragDropFiles(RichFormatConv::DropFiles& files);

protected:

    virtual SWindow * GetDropDownOwner();
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);
    void OnSend();
    void OnSendPicture();
    void OnShowSendTypeMenu();
    void OnTimer(UINT_PTR idEvent);
    void OnDropFiles(HDROP hDropInfo);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    void OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags);
    bool OnEditorAcceptData(SOUI::EventArgs *pEvt);
    bool OnShowInputRichEditMenu(SOUI::EventArgs *pEvt);
    bool OnShowMessageRichEditMenu(SOUI::EventArgs *pEvt);
    bool OnInputEditorChange(SOUI::EventArgs *pEvt);
    bool OnMemberSelected(SOUI::EventArgs *pEvt);
    bool OnInputRichObjEvent(SOUI::EventArgs *pEvt);
    bool OnMsgRichObjEvent(SOUI::EventArgs *pEvt);
    bool OnMsgRichScrollEvent(SOUI::EventArgs *pEvt);

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"send-pic", OnSendPicture)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_NAME_COMMAND(L"btn_send", OnSend)
        EVENT_NAME_COMMAND(L"btn_showsendmenu", OnShowSendTypeMenu)
        EVENT_MAP_END()

        BEGIN_MSG_MAP_EX(GroupChatFrame)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        MSG_WM_DROPFILES(OnDropFiles)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_KEYDOWN(OnKeyDown)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
        END_MSG_MAP()

private:

    enum SendType
    {
        PRESS_ENTER_TO_SEND = 1,
        PRESS_ENTER_AND_CTRL_TO_SEND,
    };

    typedef std::vector<UserInfo> UserInfoVec;

    UserInfoVec         _users;
    BOOL			    _initialized;
    SImRichEdit *       _pMsgRichEdit;
    SImRichEdit *       _pInputRichEdit;
    DropDownListWnd *   _pMemberSelWnd;     /**< 群聊时@的时候用来选人 */
    int                 _msgAdded;          // 用来判断获取更多ole的状态
    time_t              _lastWhellTime;
    int                 _totalWhellDelta;
    int                 _sendType;          // 输入enter发送或者enter+ctrl发送
};
