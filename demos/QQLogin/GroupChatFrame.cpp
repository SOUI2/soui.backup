/*
 * GroupChatFrame.cpp
 * 仿QQ的聊天窗,目前只完成以下功能：
 * 1.图文混排
 * 2.完成部分ole控件
 * 3.群聊的@人选择框
 * 4.气泡显示
 *
 * 部分源码参考了另一个开源UI库：dragonui，SVN地址：
 * http://code.taobao.org/svn/dragon/trunk
 *
 * 注意以下几点：
 *
 * - 这只是个demo，还有大量功能没完成、细节要完善。例如拷贝、粘贴、拖拽，右键菜单等等。
 *   体验暂时也还做不到QQ那么好。
 *
 * - 这个demo是没有经过详细测试的，有bug很正常，请别大惊小怪。
 *   对GIF的支持效率极低，先这样吧，后面再改善
 *
 * - 由于本人水平不足，能力有限，要是看不顺眼的/有建议的欢迎拍砖。
 *   SOUI的QQ交流群：229313785
 */

#include "stdafx.h"
#include <shellapi.h>
#include <GdiPlus.h>
#include "GroupChatFrame.h"
#include "helper\SAdapterBase.h"
#include "helper\SMenuEx.h"
#include "../controls.extend\FileHelper.h"
#include "extend.ctrls/imre/ImgProvider.h"
#include "MenuWrapper.h"
#include "ExtendEvents.h"
#include "utils.h"
#include "imre/ClipboardConverter.h"
#include "imre/RichEditObjEvents.h"
#include "imre\RichEditOleCtrls.h"
#include "SAntialiasSkin.h"
using namespace Gdiplus;

class MemberListAdapter : public SAdapterBase
{
public:

    virtual int getCount()
    {
        return 1000;
    }

    virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
    {
        if (pItem->GetChildrenCount() == 0)
        {
            pItem->InitFromXml(xmlTemplate);
        }
    }
};

enum RichEditMenuId
{
    MENUID_COPY = 1,
    MENUID_CUT,
    MENUID_PASTE,
    MENUID_SEL_ALL,
    MENUID_SAVE_AS,
    MENUID_OPEN_FILE,
    MENUID_OPEN_FILE_DIR,
    MENUID_COPY_BUBBLE,
    MENUID_CLEAR,
    MENUID_SEND_MSG,
    MENUID_MAKE_CALL,
    MENUID_AT,
    MENUID_SHOW_INFO,
    MENUID_RECALL_MSG,
};

enum SendTypeMenuId
{
    MENUID_PRESS_ENTER_TO_SEND = 1,
    MENUID_PRESS_ENTER_AND_CTRL_TO_SEND
};

#define IMGTYPE_NORMAL _T("normal_img")
#define IMGTYPE_SMILEY _T("smiley_img")

#define TIMERID_LOAD_MSG 1499

#define OLENAME_FETCHMORE _T("FetchMoreOle")

//////////////////////////////////////////////////////////////////////////
GroupChatFrame::GroupChatFrame() : SHostWnd(_T("LAYOUT:group-chatframe"))
, _pMsgRichEdit(NULL)
, _pMemberSelWnd(NULL)
, _pInputRichEdit(NULL)
, _msgAdded(0)
, _totalWhellDelta(0)
, _lastWhellTime(0)
, _sendType(PRESS_ENTER_TO_SEND)
{
    _users.push_back(UserInfo(L"蓝先生", L"563992016", L"111", L"uires\\image\\icon1.png", L"融合"));
    _users.push_back(UserInfo(L"黄先生", L"123456", L"222", L"uires\\image\\icon2.png", L"分神"));
    _users.push_back(UserInfo(L"白先生", L"666666", L"333", L"uires\\image\\icon3.png", L"渡劫"));
    _users.push_back(UserInfo(L"紫先生", L"654321", L"444", L"uires\\image\\icon4.png", L"飞升"));
    _users.push_back(UserInfo(L"黑先生", L"8889988", L"555", L"uires\\image\\icon5.png", L"管理员"));

    _initialized = FALSE;
}

GroupChatFrame::~GroupChatFrame()
{
}
void GroupChatFrame::OnClose()
{
    DestroyWindow();
}

void GroupChatFrame::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}

void GroupChatFrame::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}

void GroupChatFrame::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void GroupChatFrame::OnSize(UINT nType, CSize size)
{
    SetMsgHandled(FALSE);
    if (!_initialized) return;
    if (nType == SIZE_MAXIMIZED)
    {
        FindChildByName(L"btn_restore")->SetVisible(TRUE);
        FindChildByName(L"btn_max")->SetVisible(FALSE);
    }
    else if (nType == SIZE_RESTORED)
    {
        FindChildByName(L"btn_restore")->SetVisible(FALSE);
        FindChildByName(L"btn_max")->SetVisible(TRUE);
    }
}

BOOL GroupChatFrame::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    ::DragAcceptFiles(m_hWnd, TRUE);
    ::RegisterDragDrop(m_hWnd, GetDropTarget());

    _initialized = TRUE;

    //行高固定的列表
    SListView *pLstViewFix = FindChildByName2<SListView>("group_member_list");
    if (pLstViewFix)
    {
        ILvAdapter *pAdapter = new MemberListAdapter;
        pLstViewFix->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    _pMsgRichEdit = FindChildByName2<SImRichEdit>("re-message");
    _pInputRichEdit = FindChildByName2<SImRichEdit>("re-input");

    ImageProvider::Insert(_T("avatar"), _T("uires\\image\\icon1.png"), CRect(0, 0, 0, 0));

    if (_pInputRichEdit)
    {
        _pInputRichEdit->SetFocus();
        DWORD dwEvtMask = _pInputRichEdit->SSendMessage(EM_GETEVENTMASK);
        _pInputRichEdit->SSendMessage(EM_SETEVENTMASK, 0, dwEvtMask | ENM_CHANGE);

        SUBSCRIBE(_pInputRichEdit, EVT_CTXMENU, GroupChatFrame::OnShowInputRichEditMenu);
        SUBSCRIBE(_pInputRichEdit, EVT_RE_NOTIFY, GroupChatFrame::OnInputEditorChange);
        SUBSCRIBE(_pInputRichEdit, EVT_RE_QUERY_ACCEPT, GroupChatFrame::OnEditorAcceptData);
        SUBSCRIBE(_pInputRichEdit, EVT_RE_OBJ, GroupChatFrame::OnInputRichObjEvent);
    }

    if (_pMsgRichEdit)
    {
        SUBSCRIBE(_pMsgRichEdit, EVT_RE_QUERY_ACCEPT, GroupChatFrame::OnEditorAcceptData);
        SUBSCRIBE(_pMsgRichEdit, EVT_CTXMENU, GroupChatFrame::OnShowMessageRichEditMenu);
        SUBSCRIBE(_pMsgRichEdit, EVT_RE_OBJ, GroupChatFrame::OnMsgRichObjEvent);
        SUBSCRIBE(_pMsgRichEdit, EVT_RE_SCROLLBAR, GroupChatFrame::OnMsgRichScrollEvent);
    }

    InsertMessages();
    _pMsgRichEdit->ScrollToBottom();

    return 0;
}

BOOL IsKeyDown(DWORD key) {
    return (GetKeyState(key) & 0x8000) != 0;
}

void GroupChatFrame::OnKeyDown(TCHAR nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_RETURN)
    {
        BOOL ctrlDown = IsKeyDown(VK_CONTROL);

        if (_sendType == PRESS_ENTER_TO_SEND && !ctrlDown)
        {
            OnSend();
            return;
        }
        else if (_sendType == PRESS_ENTER_AND_CTRL_TO_SEND && ctrlDown)
        {
            OnSend();
            return;
        }
    }

    SetMsgHandled(FALSE);
}

void GroupChatFrame::OnDropFiles(HDROP hDropInfo)
{
    RichFormatConv conv;
    if (conv.InitFromHDrop(hDropInfo))
    {
        ::SetForegroundWindow(m_hWnd);
        DragDropFiles(conv.GetDropFiles());
    }

    DragFinish(hDropInfo);
}

void GroupChatFrame::DragDropFiles(RichFormatConv::DropFiles& files)
{
    RichFormatConv::DropFiles::iterator it = files.begin();

    for (; it != files.end(); ++it)
    {
        SStringW content;
        content.Format(L"<RichEditContent><metafile file=\"%s\" /></RichEditContent>", *it);
        _pInputRichEdit->InsertContent(content, RECONTENT_CARET);
    }

    ::SetFocus(m_hWnd);
    _pInputRichEdit->SetFocus();
    _pInputRichEdit->GetContainer()->OnShowCaret(TRUE);
}

void GroupChatFrame::OnTimer(UINT_PTR idEvent)
{
    if (idEvent == TIMERID_LOAD_MSG)
    {
        ::KillTimer(m_hWnd, idEvent);

        // 增加一条消息
        _msgAdded += 1;

        if (_msgAdded % 2 == 0)
        {
            AddLeftMessage(GetRandomUser(), L"<text>这也是一条动态添加的文本</text>", 1);
        }
        else
        {
            AddRightMessage(GetRandomUser(), L"<text>这是一条动态添加的文本</text>", 1);
        }

        RichEditContent* pFirstContent = _pMsgRichEdit->GetContent((UINT)0);
        RichEditFetchMoreOle* pFetchMore =
            sobj_cast<RichEditFetchMoreOle>(pFirstContent->GetByName(OLENAME_FETCHMORE));

        if (pFetchMore)
        {
            if (_msgAdded >= 2)
                pFetchMore->ShowOpenLinkState();
            else
                pFetchMore->ResetState();
        }
    }

    SetMsgHandled(FALSE);
}

bool GroupChatFrame::OnInputRichObjEvent(SOUI::EventArgs *pEvt)
{
    EventRichEditObj * pev = (EventRichEditObj*)pEvt;

    if (pev->SubEventId == DBLCLICK_RICH_METAFILE)
    {
        RichEditMetaFileOle * pmf = sobj_cast<RichEditMetaFileOle>(pev->RichObj);
        ShellExecute(NULL, _T("open"), pmf->GetFilePath(), NULL, NULL, SW_SHOW);
    }
    else if (pev->SubEventId == DBLCLICK_IMAGEOLE)
    {
        RichEditImageOle * pImageOle = sobj_cast<RichEditImageOle>(pev->RichObj);

        if (pImageOle->GetImageType() == IMGTYPE_NORMAL)
        {
            ShellExecute(NULL, _T("open"), pImageOle->GetImagePath(), NULL, NULL, SW_SHOW);
        }
    }

    return true;
}

bool GroupChatFrame::OnMsgRichObjEvent(SOUI::EventArgs *pEvt)
{
    EventRichEditObj * pev = (EventRichEditObj*)pEvt;

    switch (pev->SubEventId)
    {
    case DBLCLICK_IMAGEOLE:
        {
            RichEditImageOle * pImageOle = sobj_cast<RichEditImageOle>(pev->RichObj);
            ShellExecute(NULL, _T("open"), pImageOle->GetImagePath(), NULL, NULL, SW_SHOW);
        }
        break;

    case CLICK_FILEOLE:
        {
            RichEditFileOle* pFileOle = sobj_cast<RichEditFileOle>(pev->RichObj);
            SStringW text;
            text.Format(_T("点击了文件按钮:%04x，文件路径:%s"), pev->wParam, pFileOle->GetFileName());
            ::MessageBox(m_hWnd, text, _T("提示"), MB_OK);
        }
        break;

    case CLICK_FETCHMOREOLE_MORE_MSG:
        ::SetTimer(m_hWnd, TIMERID_LOAD_MSG, 1000, NULL);
        break;

    case CLICK_FETCHMOREOLE_OPEN_LINK:
        ::MessageBox(m_hWnd, _T("点击了打开历史记录"), _T("提示"), MB_OK);
        break;

    case CLICK_LINK:
        {
            RichEditText * pText = sobj_cast<RichEditText>(pev->RichObj);
            SStringW text;
            text.Format(_T("点击了模仿的链接：%s"), pText->GetText());
            ::MessageBox(m_hWnd, text, _T("提示"), MB_OK);
        }
        break;

    case CLICK_BK_ELE:
        {
            RichEditBkElement* pBkEle = sobj_cast<RichEditBkElement>(pev->RichObj);
            if (pBkEle->GetData() == L"avatar")
            {
                ::MessageBox(m_hWnd, _T("点击了头像"), _T("提示"), MB_OK);
            }
            else if (pBkEle->GetData() == L"resend")
            {
                ::MessageBox(m_hWnd, _T("点击了重新发送"), _T("提示"), MB_OK);
            }
        }
        break;
    }

    return true;
}

bool GroupChatFrame::OnMsgRichScrollEvent(SOUI::EventArgs *pEvt)
{
    EventRichEditScroll* pev = (EventRichEditScroll*)pEvt;

    time_t now = GetTickCount();

    if (pev->ScrollAtTop && (now - _lastWhellTime) < 200)
    {
        _totalWhellDelta += pev->WheelDelta;
    }
    else
    {
        _totalWhellDelta = 0;
    }
    _lastWhellTime = now;

    if (_totalWhellDelta > 3)
    {
        RichEditContent * pContent = _pMsgRichEdit->GetContent(UINT(0));
        RichEditFetchMoreOle * pOle = NULL;
        pOle = sobj_cast<RichEditFetchMoreOle>(pContent->GetByName(OLENAME_FETCHMORE));

        if (!pOle || pOle->GetCurrentState() != RichEditFetchMoreOle::REFM_STATE_NORMAL)
        {
            return true;
        }

        pOle->ShowLoadingState();
        ::SetTimer(m_hWnd, TIMERID_LOAD_MSG, 1000, NULL);
    }

    return true;
}

bool GroupChatFrame::OnEditorAcceptData(SOUI::EventArgs *pEvt)
{
    EventQueryAccept * pev = (EventQueryAccept*)pEvt;

    if (pev->Conv->GetAvtiveFormat() == CF_HDROP)
    {
        ::SetForegroundWindow(m_hWnd);
        RichFormatConv::DropFiles files = pev->Conv->GetDropFiles();
        DragDropFiles(files);
        return true;
    }

    return false;
}

SWindow* GroupChatFrame::GetDropDownOwner()
{
    return this;
}

void GroupChatFrame::OnCreateDropDown(SDropDownWnd *pDropDown)
{
}

void GroupChatFrame::OnDestroyDropDown(SDropDownWnd *pDropDown)
{
    _pMemberSelWnd = NULL;
}

void GroupChatFrame::ShowMemberSelectionWnd(const CRect& rcCursor, const SStringW& strKeyWord)
{
    if (!_pMemberSelWnd)
    {
        _pMemberSelWnd = new DropDownListWnd(this);
        _pMemberSelWnd->Create(CRect(), 0);
        _pMemberSelWnd->GetEventSet()->subscribeEvent(
            EventItemSelected::EventID,
            Subscriber(&GroupChatFrame::OnMemberSelected, this));

        SListBoxEx * plb = _pMemberSelWnd->FindChildByName2<SListBoxEx>(L"selectionlist");
        UserInfoVec::iterator it = _users.begin();
        for (int i = 0; it != _users.end(); ++it, ++i)
        {
            SStringW strItem;

            strItem.Format(
                L"<item><text pos=\"5,0,-0,-0\" font=\"size:12\" valign=\"middle\" dotted=\"1\">%s(%s)</text></item>",
                it->Name,
                it->Account);

            int npos = plb->InsertItem(-1, strItem);
            SWindow * pPanel = plb->GetItemPanel(npos);

            pPanel->SetAttribute(L"colorNormal", i % 2 == 0 ? L"#f0f7fc" : L"#e4eff8");
            pPanel->SetAttribute(L"colorSelected", L"#f2ebcd");
            pPanel->SetAttribute(L"colorHover", L"#f2ebcd");
        }
        plb->SetCurSel(0);

        _pMemberSelWnd->AdjustWindowSize();
    }

    _pMemberSelWnd->SetWindowPos(HWND_TOP,
        rcCursor.left, rcCursor.bottom, // left,top
        0, 0,                           // width,height
        SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

bool GroupChatFrame::OnMemberSelected(SOUI::EventArgs *pEvt)
{
    EventItemSelected * pev = (EventItemSelected*)pEvt;

    SStringW strKeyWord;
    CHARRANGE cr = { 0, 0 };
    _pInputRichEdit->SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

    strKeyWord = _pInputRichEdit->GetWindowText();
    strKeyWord = strKeyWord.Left(cr.cpMin);
    int nIndex = strKeyWord.ReverseFind(TCHAR('@'));
    if (nIndex >= 0)
    {
        _pInputRichEdit->SetSel(nIndex, cr.cpMin);
    }

    SStringW name = L"@";
    name += _users[pev->nCurrSelect].Name;

    SStringW str = RichEditReminderOle::MakeFormattedText(name, 13, RGB(0x00, 0x6e, 0xfe));

    SStringW content;
    content.Format(L"<RichEditContent>%s</RichEditContent>", str);

    _pInputRichEdit->InsertContent(content, RECONTENT_CARET);

    return false;
}

bool GroupChatFrame::OnInputEditorChange(SOUI::EventArgs *pEvt)
{
    EventRENotify *pReNotify = (EventRENotify*)pEvt;
    if (pReNotify->iNotify == EN_CHANGE)
    {
        CRect rcCursor;
        CHARRANGE cr = { 0, 0 };
        _pInputRichEdit->SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

        if (cr.cpMin == cr.cpMax)
        {
            SStringW strKeyWord = _pInputRichEdit->GetWindowText();
            strKeyWord = strKeyWord.Left(cr.cpMin);

            int nIndex = strKeyWord.ReverseFind(TCHAR('@'));
            if (nIndex >= 0)
            {
                _pInputRichEdit->GetCaretRect(rcCursor);
                ShowMemberSelectionWnd(rcCursor, strKeyWord);
            }
        }
    }

    return false;
}

void GroupChatFrame::OnSendPicture()
{
    SStringW strFile;
    CFileDialogEx dlg(TRUE, NULL, 0, 0,
        _T("图片文件\0*.gif;*.bmp;*.jpg;*.png\0\0"));

    if (dlg.DoModal() != IDOK)
    {
        return;
    }

    strFile = dlg.m_szFileName;
    strFile = XMLEscape(strFile);

    SStringW str;
    str.Format(L"<RichEditContent>"
        L"<para break=\"0\" disable-layout=\"1\">"
        L"<img type=\"normal_img\" path=\"%s\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
        L"</para>"
        L"</RichEditContent>", strFile);

    if (_pInputRichEdit)
    {
        _pInputRichEdit->InsertContent(str, RECONTENT_CARET);
    }
}

GroupChatFrame::UserInfo& GroupChatFrame::GetRandomUser()
{
    return _users[rand() % _users.size()];
}

void GroupChatFrame::InsertMessages()
{
    CRect caretRect;
    _pInputRichEdit->GetCaretRect2(caretRect);
    int oleHeight = caretRect.Height() > 0 ? caretRect.Height() - 2 : 0;

    SStringW name = L"@";
    name += GetRandomUser().Name;

    SStringW str = RichEditReminderOle::MakeFormattedText(name, 12, RGB(00, 0x6e, 0xfe));
    SStringW str2 = RichEditFileOle::MakeFormattedText(L"C:\\Windows\\notepad.exe", L"已发送", 218746211, 0x58);
    SStringW str3 =
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\3FFDCD6E3469FD77BDB7E8E98C511AAE.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\4B96AE958A8D53684E3A5C979D6C10A3.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\4E30D65A3F311D2E4DB6DC1AED8C84F9.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\8C86395E59DCC57C9A65E4DC87E2331F.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\8F0B16374501413B3009422C6C205E29.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\8F7266FD0372EC1AF358A6041E3F416D.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\8FD97F60A050B5EE737EB1B77430FDCB.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\8FE11CC2FDDE631AC7ADA2EF576AD4DE.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\8FE11CC2FDDE631AC7ADA2EF576AD4DE.gif\" id=\"zzz\" show-magnifier=\"0\"/>"
        L"<img type=\"smiley_img\" path=\"uires\\image\\emotion\\13E1887418EF4B26BF568158F582ECE5.gif\" id=\"zzz\" show-magnifier=\"0\" />"
        L"<text>这里有很多gif表情，是没有放大镜按钮的</text>";

    SStringW str4 =
        L"<img type=\"normal_img\" path=\"uires\\image\\bk.bmp\" id=\"201511242337090001_img\" size=\"100,50\" max-size=\"150,150\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
        L"<img type=\"normal_img\" path=\"uires\\image\\image2.png\" id=\"201511242337090002_img\" size=\"100,50\" max-size=\"150,150\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
        L"<text>这里还有一些图片，可以双击打开，也可以撤回消息</text>";

    SStringW str5 = L"<text font-size=\"10\">我是一个链接，可以</text><text underline=\"1\" link=\"1\" font-size=\"14\" color=\"#0000ff\">点我</text><text font-size=\"10\">试试！！</text>";

    SStringW str6 = L"<text font-size=\"12\" color=\"#ff0000\" >版本更新信息:\n1、加入拷贝、粘贴，可以粘贴从浏览器拷贝下来的内容，但不支持https图片\n2、加入拖拽\n3、加入菜单\n4、加入撤回\n5、完善滚动条体验\n……</text>";
    AddFetchMoreBlock();

    AddLeftMessage(GetRandomUser(), str);

    AddRightMessage(GetRandomUser(), str);

    AddCustomCenterMessage();

    AddCenterMessageWhitoutBuggle(GetRandomUser(), L"<text font-size=\"10\" color=\"#808080\">13:03:05</text>");

    AddCenterMessage(GetRandomUser(), L"<text font-size=\"10\">蓝先生在火星签到</text>");

    AddLeftMessage(GetRandomUser(), str2);

    AddLeftMessage(GetRandomUser(), str3);

    AddRightMessage(GetRandomUser(), str4);

    AddLeftMessage(GetRandomUser(), str5);

    AddSeparatorBar();

    AddRightMessage(GetRandomUser(), str6);

}

void GroupChatFrame::AddFetchMoreBlock()
{
    //
    // 获取更多消息OLE
    // 

    SStringW content;
    content.Format(
        L"<RichEditContent type=\"ContentFetchMore\" align=\"center\">"
        L"<para name=\"para\" margin=\"0,5,0,0\" break=\"1\">"
        L"<fetchmore name=\"%s\" selectable=\"0\" />"
        L"</para>"
        L"</RichEditContent>",
        OLENAME_FETCHMORE);

    _pMsgRichEdit->InsertContent(content);
}

void GroupChatFrame::AddSeparatorBar()
{
    SStringW content;
    content.Format(
        L"<RichEditContent  type=\"ContentSeperator\" align=\"center\">"
        L"<para margin=\"0,10,0,0\" break=\"1\">"
        L"<split selectable=\"0\" />"
        L"</para>"
        L"</RichEditContent>");

    _pMsgRichEdit->InsertContent(content);
}

UINT  GroupChatFrame::AddCenterMessage(UserInfo& info, const SStringW& body, UINT insertTo/*=RECONTENT_LAST*/)
{
    SStringW content;
    content.Format(
        L"<RichEditContent type=\"ContentCenter\" >"
        L"<para break=\"1\" align=\"left\" />"
        L"<para margin=\"100,0,100,0\" align=\"center\" break=\"1\" >"
        L"%s"
        L"</para>"
        L"<bkele skin=\"skin.rich_sysmsg_bk\" hittestable=\"0\" pos=\"{-10,{0,[10,[0\" />"
        L"</RichEditContent>", body);

    return _pMsgRichEdit->InsertContent(content, insertTo);
}

UINT  GroupChatFrame::AddCenterMessageWhitoutBuggle(UserInfo& info, const SStringW& body, UINT insertTo/*=RECONTENT_LAST*/)
{
    SStringW content;
    content.Format(
        L"<RichEditContent type=\"ContentCenter\" >"
        L"<para margin=\"100,5,100,5\" align=\"center\" break=\"1\" >"
        L"%s"
        L"</para>"
        L"</RichEditContent>", body);

    return _pMsgRichEdit->InsertContent(content, insertTo);
}

UINT  GroupChatFrame::AddLeftMessage(UserInfo& info, const SStringW& body, UINT insertTo/*=RECONTENT_LAST*/)
{
    static int i = 0;
    SStringW name;
    name.Format(L"【%s】%s:%d", info.Title, info.Name, i++);

    if (!ImageProvider::IsExist(info.AvatarId))
    {
        SAntialiasSkin* pSkin = new SAntialiasSkin();
        pSkin->SetRoundCorner(5, 5, 5, 5); // 添加圆角处理
        if (pSkin->LoadFromFile(info.AvatarPath))
        {
            ImageProvider::Insert(info.AvatarId, pSkin);
        }
        else
        {
            delete pSkin;
        }
    }

    /* 靠左显示的消息 */
    SStringW content;
    content.Format(
        L"<RichEditContent  type=\"ContentLeft\" align=\"left\">"
        L"<para margin=\"36,10,100,4\">"
        L"<text font-size=\"10\" color=\"#808080\">%s</text>"
        L"</para>"
        L"<bkele data=\"avatar\" skin=\"%s\" pos=\"0,]-18,@30,@30\" cursor=\"hand\" interactive=\"1\"/>"
        L"<para id=\"msgbody\" margin=\"45,0,30,0\" break=\"1\" simulate-align=\"1\">"
        L"%s"
        L"</para>"
        L"<bkele data=\"bubble\" left-skin=\"skin.rich_left_bubble\"  left-pos=\"35,{-9,[10,[10\" />"
        L"</RichEditContent>",
        name,
        info.AvatarId,
        body);

    return _pMsgRichEdit->InsertContent(content, insertTo);
}

UINT  GroupChatFrame::AddRightMessage(UserInfo& info, const SStringW& body, UINT insertTo/*=RECONTENT_LAST*/)
{
    if (!ImageProvider::IsExist(info.AvatarId))
    {
        SAntialiasSkin* pSkin = new SAntialiasSkin();
        pSkin->SetRoundCorner(5, 5, 5, 5);
        if (pSkin->LoadFromFile(info.AvatarPath))
        {
            ImageProvider::Insert(info.AvatarId, pSkin);
        }
        else
        {
            delete pSkin;
        }
    }

    LPCWSTR pResendBtn = L"<bkele name=\"BkEleSendFail\" data=\"resend\" right-skin=\"skin.rich_warning\" left-skin=\"skin.rich_warning\" left-pos=\"[0,[-25,@16,@16\" right-pos=\"{-20,[-25,@16,@16\" cursor=\"hand\" interactive=\"1\"/>";
    /* 靠右显示的消息 */
    SStringW content;
    content.Format(
        L"<RichEditContent  type=\"ContentRight\" align=\"right\" auto-layout=\"1\">"
        L"<para break=\"1\" align=\"left\" />"
        L"<bkele data=\"avatar\" skin=\"%s\" left-pos=\"0,]-4,@30,@30\" right-pos=\"-30,]-4,@30,@30\" cursor=\"hand\" interactive=\"1\"/>"
        L"<para id=\"msgbody\" margin=\"45,0,30,0\" break=\"1\" simulate-align=\"1\">"
        L"%s"
        L"</para>"
        L"<bkele data=\"bubble\" left-skin=\"skin.rich_left_bubble\" right-skin=\"skin.rich_right_bubble\" left-pos=\"35,{-9,[10,[10\" right-pos=\"{-10,{-9,-35,[10\" />"
        L"%s"
        L"</RichEditContent>",
        info.AvatarId,
        body,
        pResendBtn);

    return _pMsgRichEdit->InsertContent(content, insertTo);
}

void GroupChatFrame::AddCustomCenterMessage()
{
    const TCHAR * pFormatedCenterText =
        L"<RichEditContent id=\"201601015337090001\" type=\"msg\" >"
        L"<para break=\"1\" />"
        L"<para id=\"righttext\" margin=\"100,0,100,0\" break=\"1\" align=\"center\" simulate-align=\"1\">"
        L"<text font-size=\"10\" >您取消了“Adobe_Photoshop_CS5_green_7xdown.com.rar”(118.91MB)的发送，文件传输失败。</text>"
        L"</para>"
        L"<bkele skin=\"skin.rich_sysmsg_bk\" pos=\"{-30,{0,[10,[0\" hittestable=\"0\" />"
        L"<bkele skin=\"skin.rich_warning\" pos=\"{10,{3,@16,@16\" hittestable=\"0\"/>"
        L"</RichEditContent>";

    _pMsgRichEdit->InsertContent(pFormatedCenterText);
}

void GroupChatFrame::OnSend()
{
    CHARRANGE chr = { 0, -1 };
    SStringW content = _pInputRichEdit->GetSelectedContent(&chr);
    pugi::xml_document  doc;

    if (!doc.load_buffer(content, content.GetLength() * sizeof(WCHAR)))
    {
        return;
    }

    content.Empty();

    pugi::xml_node node = doc.child(L"RichEditContent").first_child();
    for (; node; node = node.next_sibling())
    {
        const wchar_t* pNodeName = node.name();

        if (wcscmp(RichEditText::GetClassName(), pNodeName) == 0)
        {
            content += RichEditText::MakeFormatedText(node.text().get(), node.attribute(L"file-size").as_int(10));
        }
        else if (wcscmp(RichEditReminderOle::GetClassName(), pNodeName) == 0)
        {
            content += RichEditReminderOle::MakeFormattedText(node.text().get(), 13, RGB(00, 0x6e, 0xfe));
        }
        else if (wcscmp(RichEditImageOle::GetClassName(), pNodeName) == 0)
        {
            content += RichEditImageOle::MakeFormattedText(
                L"",
                L"",
                IMGTYPE_NORMAL,
                L"",
                node.attribute(L"path").as_string(),
                L"",
                TRUE);
        }
        else if (wcscmp(RichEditMetaFileOle::GetClassName(), pNodeName) == 0)
        {
            content += RichEditFileOle::MakeFormattedText(node.attribute(L"file").as_string(),
                L"等待发送", 440227874, 0x04);
        }
    }

    if (_pMsgRichEdit->GetContentCount() % 2 == 0)
    {
        AddLeftMessage(GetRandomUser(), content);
    }
    else
    {
        AddRightMessage(GetRandomUser(), content);
    }

    _pInputRichEdit->Clear();
    _pMsgRichEdit->ScrollToBottom();
}

void GroupChatFrame::OnShowSendTypeMenu()
{
    CPoint pt;
    GetCursorPos(&pt);

    MenuWrapper menu(L"menu_template", L"SMENU");
    menu.AddMenu(_T("按Enter键发送消息"), MENUID_PRESS_ENTER_TO_SEND, TRUE, _sendType == PRESS_ENTER_TO_SEND);
    menu.AddMenu(_T("按Ctrl+Enter键发送消息"), MENUID_PRESS_ENTER_AND_CTRL_TO_SEND, TRUE, _sendType == PRESS_ENTER_AND_CTRL_TO_SEND);
    int ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);

    if (ret == MENUID_PRESS_ENTER_TO_SEND)
    {
        _sendType = PRESS_ENTER_TO_SEND;
    }
    else if (ret == MENUID_PRESS_ENTER_AND_CTRL_TO_SEND)
    {
        _sendType = PRESS_ENTER_AND_CTRL_TO_SEND;
    }
}

bool GroupChatFrame::OnShowInputRichEditMenu(SOUI::EventArgs *pEvt)
{
    CHARRANGE chrSel;
    _pInputRichEdit->GetSel(&chrSel.cpMin, &chrSel.cpMax);
    RichEditOleBase* pOle = _pInputRichEdit->GetSelectedOle();

    /*
     * 构造菜单项
     */

    MenuWrapper menu(L"menu_template", L"SMENU");

    BOOL enableCopy = (chrSel.cpMax - chrSel.cpMin) >= 1;
    menu.AddMenu(_T("复制(&S)"), MENUID_COPY, enableCopy, FALSE);
    menu.AddMenu(_T("剪切(&X)"), MENUID_CUT, enableCopy, FALSE);
    menu.AddMenu(_T("粘贴(&C)"), MENUID_PASTE, TRUE, FALSE);
    menu.AddMenu(_T("全选(&A)"), MENUID_SEL_ALL, TRUE, FALSE);

    if (pOle && pOle->IsClass(RichEditImageOle::GetClassName()))
    {
        menu.AddMenu(_T("另存为"), MENUID_SAVE_AS, TRUE, FALSE);
    }
    else if (pOle && pOle->IsClass(RichEditMetaFileOle::GetClassName()))
    {
        menu.AddMenu(_T("打开"), MENUID_OPEN_FILE, TRUE, FALSE);
        menu.AddMenu(_T("打开文件夹"), MENUID_OPEN_FILE_DIR, TRUE, FALSE);
    }

    /*
     * 弹出菜单
     */
    int ret = 0;
    POINT pt;
    ::GetCursorPos(&pt);

    ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);

    /*
     * 处理菜单消息
     */
    switch (ret)
    {
    case MENUID_COPY:
        _pInputRichEdit->Copy();
        break;

    case MENUID_SEL_ALL:
        _pInputRichEdit->SelectAll();
        break;

    case MENUID_CUT:
        _pInputRichEdit->Cut();
        break;

    case MENUID_PASTE:
        _pInputRichEdit->Paste();
        break;

    case MENUID_SAVE_AS:
        if (pOle)
        {
            RichEditImageOle* pImageOle = static_cast<RichEditImageOle*>(pOle);
            //SaveImage(pImageOle->GetImagePath());
        }
        break;

    case MENUID_OPEN_FILE:
        if (pOle)
        {
            RichEditMetaFileOle* pMetaFileOle = static_cast<RichEditMetaFileOle*>(pOle);
            ShellExecute(NULL, _T("open"), pMetaFileOle->GetFilePath(), NULL, NULL, SW_SHOW);
        }
        break;

    case MENUID_OPEN_FILE_DIR:
        if (pOle)
        {
            RichEditMetaFileOle* pMetaFileOle = static_cast<RichEditMetaFileOle*>(pOle);
            SStringW param;
            param.Format(_T("/select,\"%s\""), pMetaFileOle->GetFilePath());
            ShellExecute(NULL, _T("open"), _T("explorer.exe"), param, NULL, SW_SHOW);
        }
        break;
    }

    return true;
}

void GroupChatFrame::FillRClickAvatarMenu(MenuWrapper& menu, RichEditContent* pContent)
{
    menu.AddMenu(_T("发送消息"), MENUID_SEND_MSG, TRUE, FALSE);
    menu.AddMenu(_T("网络通话"), MENUID_MAKE_CALL, TRUE, FALSE);
    menu.AddMenu(_T("查看资料"), MENUID_SHOW_INFO, TRUE, FALSE);
    if (pContent && pContent->GetAlign() == RichEditObj::ALIGN_LEFT)
    {
        menu.AddMenu(_T("@TA"), MENUID_AT, TRUE, FALSE);
    }
}

void GroupChatFrame::FillRClickImageMenu(MenuWrapper& menu, RichEditContent* pContent)
{
    if (pContent && pContent->GetAlign() == RichEditObj::ALIGN_RIGHT)
    {
        menu.AddMenu(_T("撤回"), MENUID_RECALL_MSG, TRUE, FALSE);
    }

    menu.AddMenu(_T("复制"), MENUID_COPY, TRUE, FALSE);
    menu.AddMenu(_T("另存为"), MENUID_SAVE_AS, TRUE, FALSE);
    menu.AddMenu(_T("全部选择"), MENUID_SEL_ALL, TRUE, FALSE);
    menu.AddMenu(_T("清屏"), MENUID_CLEAR, TRUE, FALSE);
}

void GroupChatFrame::FillRClickFileMenu(MenuWrapper& menu, RichEditContent* pContent)
{
    if (pContent && pContent->GetAlign() == RichEditObj::ALIGN_RIGHT)
    {
        menu.AddMenu(_T("撤回"), MENUID_RECALL_MSG, TRUE, FALSE);
    }

    menu.AddMenu(_T("打开"), MENUID_OPEN_FILE, TRUE, FALSE);
    menu.AddMenu(_T("打开文件夹"), MENUID_OPEN_FILE_DIR, TRUE, FALSE);
    menu.AddMenu(_T("全部选择"), MENUID_SEL_ALL, TRUE, FALSE);
    menu.AddMenu(_T("清屏"), MENUID_CLEAR, TRUE, FALSE);
}

void GroupChatFrame::FillRClickSelRegionMenu(MenuWrapper& menu, RichEditContent* pContent)
{
    if (pContent && pContent->GetAlign() == RichEditObj::ALIGN_RIGHT)
    {
        menu.AddMenu(_T("撤回"), MENUID_RECALL_MSG, TRUE, FALSE);
    }

    menu.AddMenu(_T("复制"), MENUID_COPY, TRUE, FALSE);
    menu.AddMenu(_T("全部选择"), MENUID_SEL_ALL, TRUE, FALSE);
    menu.AddMenu(_T("清屏"), MENUID_CLEAR, TRUE, FALSE);
}

void GroupChatFrame::FillRClickBubbleMenu(MenuWrapper& menu, RichEditContent* pContent)
{
    if (pContent && pContent->GetAlign() == RichEditObj::ALIGN_RIGHT)
    {
        menu.AddMenu(_T("撤回"), MENUID_RECALL_MSG, TRUE, FALSE);
    }
    menu.AddMenu(_T("复制"), MENUID_COPY_BUBBLE, TRUE, FALSE);
    menu.AddMenu(_T("全部选择"), MENUID_SEL_ALL, TRUE, FALSE);
    menu.AddMenu(_T("清屏"), MENUID_CLEAR, TRUE, FALSE);
}

void GroupChatFrame::FillRClickNothingMenu(MenuWrapper& menu)
{
    menu.AddMenu(_T("全部选择"), MENUID_SEL_ALL, TRUE, FALSE);
    menu.AddMenu(_T("清屏"), MENUID_CLEAR, TRUE, FALSE);
}

bool GroupChatFrame::OnShowMessageRichEditMenu(SOUI::EventArgs *pEvt)
{
    EventCtxMenu * pev = static_cast<EventCtxMenu*>(pEvt);

    CHARRANGE selRange;
    _pMsgRichEdit->GetSel(&selRange.cpMin, &selRange.cpMax);
    int selectedCount = selRange.cpMax - selRange.cpMin;

    //
    // 如果鼠标没有落在选中区域上，需要取消选中
    //
    if (selectedCount > 0)
    {
        int cp = _pMsgRichEdit->CharFromPos(pev->pt);
        if (cp < selRange.cpMin || cp > selRange.cpMax)
        {
            _pMsgRichEdit->SetSel(cp, cp);
            selectedCount = 0;
        }
    }

    //
    // 对于右键在选择区域上的各种判断
    //

    RichEditObj * pHitTestObj = _pMsgRichEdit->HitTest(pev->pt);
    RichEditObj * pObj = pHitTestObj;

    for (; pObj != NULL && pObj->GetParent(); pObj = pObj->GetParent());

    RichEditContent * pContent = sobj_cast<RichEditContent>(pObj);
    RichEditImageOle* pImageOle = NULL;
    RichEditFileOle* pFileOle = NULL;

    MenuWrapper menu(L"menu_template", L"SMENU");

    if (pContent == NULL || pHitTestObj == NULL)
    {
        if (selectedCount > 0)
            FillRClickSelRegionMenu(menu, pContent);
        else
            FillRClickNothingMenu(menu);
    }
    else if (pHitTestObj->IsClass(RichEditBkElement::GetClassName()) && pHitTestObj->GetData() == _T("avatar"))
    {
        FillRClickAvatarMenu(menu, pContent);
    }
    else if (pHitTestObj->IsClass(RichEditImageOle::GetClassName()))
    {
        pImageOle = static_cast<RichEditImageOle*>(pHitTestObj);
        if (pImageOle->GetImageType() == IMGTYPE_NORMAL)
            FillRClickImageMenu(menu, pContent);
        else
            FillRClickSelRegionMenu(menu, pContent);
    }
    else if (pHitTestObj->IsClass(RichEditFileOle::GetClassName()))
    {
        pFileOle = static_cast<RichEditFileOle*>(pHitTestObj);
        FillRClickFileMenu(menu, pContent);
    }
    else if (pHitTestObj->IsClass(RichEditReminderOle::GetClassName()))
    {
        FillRClickSelRegionMenu(menu, pContent);
    }
    else if (pHitTestObj->IsClass(RichEditBkElement::GetClassName()) && pHitTestObj->GetData() == _T("bubble"))
    {
        if (selectedCount > 0)
            FillRClickSelRegionMenu(menu, pContent);
        else
            FillRClickBubbleMenu(menu, pContent);
    }
    else
    {
        FillRClickNothingMenu(menu);
    }

    /*
     * 弹出菜单
     */
    int ret = 0;
    POINT pt;
    ::GetCursorPos(&pt);

    ret = menu.ShowMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd);

    /*
     * 处理菜单消息
     */

    CHARRANGE          chr = { 0, -1 };
    IDataObject      * pobj;
    RichFormatConv     conv;
    SStringW           str;

    switch (ret)
    {
    case MENUID_COPY:
        _pMsgRichEdit->Copy();
        break;

    case MENUID_SEL_ALL:
        _pMsgRichEdit->SelectAll();
        break;

    case MENUID_SAVE_AS:
        if (pImageOle)
        {
            //SaveImage(pImageOle->GetImagePath());
        }
        break;

    case MENUID_OPEN_FILE:
        if (pFileOle)
        {
            //ShellExecute(NULL, _T("open"), pFileOle->GetFilePath(), NULL, NULL, SW_SHOW);
        }
        break;

    case MENUID_OPEN_FILE_DIR:
        if (pFileOle)
        {
            //SStringW param;
            //param.Format(_T("/select,\"%s\""), pMetaFileOle->GetFilePath());
            //ShellExecute(NULL, _T("open"), _T("explorer.exe"), param, NULL, SW_SHOW);
        }
        break;

    case MENUID_COPY_BUBBLE:
        if (pContent)
        {
            pObj = pContent->GetById(_T("msgbody"));
            chr = pObj->GetCharRange();
            str = _pMsgRichEdit->GetSelectedContent(&chr);
            conv.InitFromRichContent(str);
            conv.ToDataObject(&pobj);
            OleSetClipboard(pobj);
            OleFlushClipboard();
            pobj->Release();
        }
        break;

    case MENUID_CLEAR:
        _pMsgRichEdit->Clear();
        break;

    case MENUID_SEND_MSG:
        ::MessageBox(m_hWnd, _T("点击了发送消息菜单"), _T("提示"), MB_OK);
        break;

    case MENUID_MAKE_CALL:
        ::MessageBox(m_hWnd, _T("点击了网络通话菜单"), _T("提示"), MB_OK);
        break;

    case MENUID_SHOW_INFO:
        ::MessageBox(m_hWnd, _T("点击了查看资料菜单"), _T("提示"), MB_OK);
        break;

    case MENUID_AT:
        {
            str = RichEditReminderOle::MakeFormattedText(
                _T("点击菜单@人"),
                13,
                RGB(00, 0x6e, 0xfe));

            SStringW content;
            content.Format(L"<RichEditContent>%s</RichEditContent>", str);
            _pInputRichEdit->InsertContent(content, RECONTENT_CARET);
        }
        break;

    case MENUID_RECALL_MSG:
        if (pContent)
        {
            UINT index = _pMsgRichEdit->DeleteContent(pContent);
            AddCenterMessage(GetRandomUser(), L"<text font-size=\"10\">你已撤回一条消息</text>", index);
        }
        break;
    }

    return true;
}