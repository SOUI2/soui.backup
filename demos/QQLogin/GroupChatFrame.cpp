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
#include "GroupChatFrame.h"
#include "helper\SAdapterBase.h"
#include "extend.ctrls\imre\RichEditOleCtrls.h"
#include "helper\SMenuEx.h"
#include "../controls.extend\FileHelper.h"
#include "utils.h"
#include <GdiPlus.h>
using namespace Gdiplus;

class MemberListAdapter : public SAdapterBase
{
public:

    virtual int getCount()
    {
        return 1000;
    }

    virtual void getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate)
    {
        if(pItem->GetChildrenCount()==0)
        {
            pItem->InitFromXml(xmlTemplate);
        }
    }
};

//////////////////////////////////////////////////////////////////////////
GroupChatFrame::GroupChatFrame() : SHostWnd(_T("LAYOUT:group-chatframe"))
    ,m_preRecord(NULL)
    ,m_pMemberSelWnd(NULL)
    ,m_preInput(NULL)
{
    m_lstMembers.push_back(L"蓝先生好好学习天天向上哈哈哈嘿嘿");
    m_lstMembers.push_back(L"黄先生好好学习天天向上哈哈哈嘿嘿");
    m_lstMembers.push_back(L"白先生好好学习");

    m_bLayoutInited=FALSE;
} 

GroupChatFrame::~GroupChatFrame()
{
}

BOOL GroupChatFrame::OnInitDialog( HWND hWnd, LPARAM lParam )
{    
    // 		MARGINS mar = {5,5,30,5};
    // 		DwmExtendFrameIntoClientArea ( m_hWnd, &mar );

    m_bLayoutInited=TRUE;

    //行高固定的列表
    SListView *pLstViewFix = FindChildByName2<SListView>("group_member_list");
    if(pLstViewFix)
    {
        ILvAdapter *pAdapter = new MemberListAdapter;
        pLstViewFix->SetAdapter(pAdapter);
        pAdapter->Release();
    }

    m_preRecord   = FindChildByName2<SImRichEdit>("re-message");
    m_preInput       = FindChildByName2<SImRichEdit>("re-input");

    ImageProvider::Insert(_T("avatar"), _T("uires\\image\\icon1.png"), CRect(0,0,0,0));

    if (m_preInput)
    {
        m_preInput->SetFocus();
        DWORD dwEvtMask = m_preInput->SSendMessage(EM_GETEVENTMASK);
        m_preInput->SSendMessage(EM_SETEVENTMASK, 0, dwEvtMask | ENM_CHANGE);
        m_preInput->GetEventSet()->subscribeEvent(EventRENotify::EventID,Subscriber(&GroupChatFrame::OnInputEditorChange,this));
    }

    return 0;
}

BOOL GroupChatFrame::GetEditorCursorRect(SImRichEdit * pRichEdit, LONG cpStart, LONG cpEnd, CRect &rcCursor)
{
    if (cpStart < 0 || cpEnd < 0)
    {
        return FALSE;
    }
    ITextDocument     * pdoc = pRichEdit->GetTextDoc();
    SComPtr<ITextRange>  prange;

    rcCursor.SetRectEmpty();

    pdoc->Range(cpStart, cpEnd, &prange);
    if (!prange)
    {
        return FALSE;
    }

    // http://technet.microsoft.com/zh-cn/hh768766(v=vs.90) 新类型定义
#define _tomClientCoord     256  // 默认获取到的是屏幕坐标， Use client coordinates instead of screen coordinates.
#define _tomAllowOffClient  512  // Allow points outside of the client area.

    POINT ptStart   = {0,0};
    POINT ptEnd     = {0,0};

    long lTypeTopLeft       = _tomAllowOffClient|_tomClientCoord|tomStart|TA_TOP|TA_LEFT;
    long lTypeRightBottom   = _tomAllowOffClient|_tomClientCoord|tomEnd|TA_BOTTOM|TA_RIGHT;
    if (prange->GetPoint(lTypeTopLeft,     &ptStart.x, &ptStart.y) != S_OK ||
        prange->GetPoint(lTypeRightBottom, &ptEnd.x,   &ptEnd.y) != S_OK)
    {
        return FALSE;
    }

    rcCursor.SetRect(ptStart, ptEnd);
    if (rcCursor.Width() == 0)
    {
        rcCursor.right += 1;
    }

    ClientToScreen(rcCursor);
    return TRUE;
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
    m_pMemberSelWnd=NULL;
}

void GroupChatFrame::ShowMemberSelectionWnd(const CRect& rcCursor, const SStringW& strKeyWord)
{
    if (!m_pMemberSelWnd)
    {
        m_pMemberSelWnd = new DropDownListWnd(this);
        m_pMemberSelWnd->Create(CRect(),0);
        m_pMemberSelWnd->GetEventSet()->subscribeEvent(
            EventItemSelected::EventID,
            Subscriber(&GroupChatFrame::OnMemberSelected,this));

        SListBoxEx * plb        = m_pMemberSelWnd->FindChildByName2<SListBoxEx>(L"selectionlist");
        StringList::iterator it = m_lstMembers.begin();
        for (int i = 0; it != m_lstMembers.end(); ++it, ++i)
        {
            SStringW strItem;

            strItem.Format(
                L"<item><text pos=\"5,0,-0,-0\" font=\"size:12\" valign=\"middle\" dotted=\"1\">%s</text></item>",
                *it);

            int npos            = plb->InsertItem(-1, strItem);
            SWindow * pPanel    = plb->GetItemPanel(npos);

            pPanel->SetAttribute(L"colorNormal", i%2==0? L"#f0f7fc" : L"#e4eff8");
            pPanel->SetAttribute(L"colorSelected", L"#f2ebcd");
            pPanel->SetAttribute(L"colorHover", L"#f2ebcd");
        }
        plb->SetCurSel(0);

        m_pMemberSelWnd->AdjustWindowSize();
    }

    m_pMemberSelWnd->SetWindowPos(HWND_TOP,
        rcCursor.left, rcCursor.bottom, // left,top
        0, 0,                           // width,height
        SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
}

bool GroupChatFrame::OnMemberSelected(SOUI::EventArgs *pEvt)
{
    EventItemSelected * pev = (EventItemSelected*)pEvt;

    SStringW strKeyWord;
    CHARRANGE cr ={0, 0};
    m_preInput->SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

    strKeyWord = m_preInput->GetWindowText();
    strKeyWord = strKeyWord.Left(cr.cpMin);
    int nIndex = strKeyWord.ReverseFind(TCHAR('@'));
    if (nIndex >= 0)
    {
        m_preInput->SetSel(nIndex, cr.cpMin);
    }

    SStringW str;
    str.Format(L"<RichEditContent>"
        L"<para break=\"0\">"
        L"<remainder color-text=\"#000000\" color-bkgnd=\"#d3eebc\" color-border=\"#b8e68e\" border-width=\"1\" id=\"remainder_2015122301\" >@%s</remainder>"
        L"</para>"
        L"</RichEditContent>", m_lstMembers[pev->nCurrSelect]);

    m_preInput->InsertContent(str, RECONTENT_CARET);

    return false;
}

bool GroupChatFrame::OnInputEditorChange(SOUI::EventArgs *pEvt)
{
    EventRENotify *pReNotify = (EventRENotify*)pEvt;
    if (pReNotify->iNotify == EN_CHANGE)
    {    
        CRect rcCursor;
        CHARRANGE cr ={0, 0};
        m_preInput->SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

        if (cr.cpMin == cr.cpMax)
        {
            SStringW strKeyWord = m_preInput->GetWindowText();
            strKeyWord = strKeyWord.Left(cr.cpMin);

            int nIndex = strKeyWord.ReverseFind(TCHAR('@'));
            if (nIndex >= 0)
            {
                GetEditorCursorRect(m_preInput, cr.cpMin, cr.cpMax, rcCursor);
                ShowMemberSelectionWnd(rcCursor, strKeyWord);
            }

            nIndex = strKeyWord.ReverseFind(TCHAR('/'));
            if (nIndex >= 0)
            {
                GetEditorCursorRect(m_preInput, cr.cpMin, cr.cpMax, rcCursor);
                //ShowSmileySelectionWnd(rcCursor, strKeyWord);
            }
        }
    }

    return false; 
}

void GroupChatFrame::OnSendPicture()
{
    SStringW strFile;
    CFileDialogEx dlg(TRUE,NULL,0, 0,
        _T("gif files(*.gif)\0*.gif\0)")
        _T("bmp files(*.bmp)\0*.bmp\0)")
        _T("jpg files(*.jpg)\0*.jpg\0)")
        _T("png files(*.png)\0*.png\0)\0"));

    if(dlg.DoModal()!=IDOK)
    {
        return;
    }

    strFile = dlg.m_szFileName;
    strFile = XMLEscape(strFile);

    SStringW str;
    str.Format(L"<RichEditContent>"
        L"<para break=\"0\" disable-layout=\"1\">"
        L"<img type=\"image\" src=\"%s\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
        L"</para>"
        L"</RichEditContent>", strFile);

    if (m_preInput)
    {
        m_preInput->InsertContent(str, RECONTENT_CARET);
    }
}

void GroupChatFrame::OnSend()
{

    SStringW str;

    /* 获取更多消息 */
    const TCHAR * pFetchMore = 
        L"<RichEditContent name=\"parent\" id=\"201511242337090001\" type=\"fetchmore\" align=\"center\">"
            L"<para name=\"para\" margin=\"0,5,0,0\" break=\"1\">"
                L"<fetchmore name=\"fetch\" selectable=\"0\" />"
            L"</para>"
        L"</RichEditContent>";

    /*  以上是历史消息 */
    const TCHAR * pSplitLine = 
        L"<RichEditContent id=\"201511242337090001\" type=\"split\" align=\"center\">"
        L"<para margin=\"0,10,0,0\" break=\"1\">"
        L"<split selectable=\"0\" />"
        L"</para>"
        L"</RichEditContent>";

    /* 新闻消息，参考微信 */
    const TCHAR * pNews = 
        L"<RichEditContent id=\"201512152337090001\" type=\"news\" align=\"center\">"
        L"<para margin=\"0,10,0,0\" break=\"1\">"
        L"<news id=\"news_2015121501\" selectable=\"0\"/>"
        L"</para>"
        L"</RichEditContent>";

    /* @消息，@别人*/
    const TCHAR * pRemainderLeft = 
        L"<RichEditContent id=\"201511242337090001\" type=\"msg\" align=\"left\" >"
        L"<para margin=\"36,2,0,4\" break=\"1\">"
            L"<text font=\"size:12\" color=\"#808080\">【传说】蓝先生</text>"
        L"</para>"
        L"<avatar id=\"\" src=\"avatar\" pos-left=\"0,]-18,@30,@30\" pos-right=\"-30,]-15,@30,@30\" />"

        L"<para margin=\"50,0,30,0\" break=\"1\">"
            L"<remainder color-text=\"#000000\" color-bkgnd=\"#d3eebc\" color-border=\"#b8e68e\" border-width=\"1\" id=\"remainder_2015122301\" >@使用+蓝先生</remainder>"
        L"</para>"
        L"<bubble id=\"\" left-bubble=\"skin.rich_left_bubble\" right-bubble=\"skin.rich_right_bubble\" pos-left=\"35,{-9,[15,[10\" pos-right=\"{-15,{-9,-35,[10\"/>"
        L"</RichEditContent>";

    /* @消息，@自己*/
    const TCHAR * pRemainderRight = 
        L"<RichEditContent id=\"201511242337090001\" type=\"msg\" align=\"right\" auto-layout=\"1\">"
            L"<avatar id=\"\" src=\"avatar\" pos-left=\"0,]-4,@30,@30\" pos-right=\"-30,]-4,@30,@30\" />"
            L"<para margin=\"50,0,30,0\" break=\"1\" simulate-align=\"1\">"
                L"<remainder color-text=\"#006efe\" id=\"remainder_2015122301\" >@使用+蓝先生好好学习，天天向上</remainder>"
            L"</para>"
            L"<bubble id=\"\" left-bubble=\"skin.rich_left_bubble\" right-bubble=\"skin.rich_right_bubble\" pos-left=\"35,{-9,[15,[10\" pos-right=\"{-15,{-9,-35,[10\"/>"
        L"</RichEditContent>";

    /* 靠左显示的消息 */
    const TCHAR * pFormatedLeftText =
        L"<RichEditContent id=\"201511242337090001\" type=\"msg\" align=\"left\">"
            L"<para margin=\"36,10,100,4\">"
                L"<text font-size=\"10\" color=\"#808080\">【传说】蓝先生:%d</text>"
            L"</para>"
            
            L"<avatar id=\"\" src=\"avatar\" pos-left=\"0,]-18,@30,@30\" pos-right=\"-30,]-15,@30,@30\" />"
            L"<para margin=\"50,0,30,0\" break=\"1\" simulate-align=\"1\">"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\3FFDCD6E3469FD77BDB7E8E98C511AAE.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\4B96AE958A8D53684E3A5C979D6C10A3.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\4E30D65A3F311D2E4DB6DC1AED8C84F9.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8C86395E59DCC57C9A65E4DC87E2331F.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8F0B16374501413B3009422C6C205E29.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8F7266FD0372EC1AF358A6041E3F416D.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8FD97F60A050B5EE737EB1B77430FDCB.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8FE11CC2FDDE631AC7ADA2EF576AD4DE.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8FE11CC2FDDE631AC7ADA2EF576AD4DE.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\13E1887418EF4B26BF568158F582ECE5.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" selectable=\"1\" cursor=\"hand\" />"
                //L"<text font-face=\"宋体\" underline=\"1\" color=\"#ff0000\">%s</text>"
                L"<text >%s</text>"
            L"</para>"
            L"<bubble id=\"\" left-bubble=\"skin.rich_left_bubble\" right-bubble=\"skin.rich_right_bubble\" pos-left=\"35,{-9,[15,[10\" pos-right=\"{-15,{-9,-35,[8\"/>"
        L"</RichEditContent>";
 
    /* 靠右显示的消息 */
    const TCHAR * pFormatedRightText =
        L"<RichEditContent id=\"201511242337090001\" type=\"msg\" align=\"right\" auto-layout=\"1\">"
            L"<avatar id=\"\" src=\"avatar\" pos-left=\"0,]-4,@30,@30\" pos-right=\"-30,]-4,@30,@30\" />"
            L"<para id=\"righttext\" margin=\"50,0,30,0\" break=\"1\" simulate-align=\"1\">"
                //L"<text  font-size=\"12\" italic=\"1\" bold=\"1\" color=\"#00ff00\">%s</text>"
                L"<text>%s</text>"
                L"<img type=\"image\" src=\"uires\\image\\bk.bmp\" id=\"201511242337090001_img\" size=\"100,50\" max-size=\"150,150\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\image2.png\" id=\"201511242337090002_img\" size=\"100,50\" max-size=\"150,150\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\3FFDCD6E3469FD77BDB7E8E98C511AAE.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\4B96AE958A8D53684E3A5C979D6C10A3.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\4E30D65A3F311D2E4DB6DC1AED8C84F9.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8C86395E59DCC57C9A65E4DC87E2331F.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8F0B16374501413B3009422C6C205E29.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8F7266FD0372EC1AF358A6041E3F416D.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8FD97F60A050B5EE737EB1B77430FDCB.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8FE11CC2FDDE631AC7ADA2EF576AD4DE.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\8FE11CC2FDDE631AC7ADA2EF576AD4DE.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
                L"<img type=\"image\" src=\"uires\\image\\emotion\\13E1887418EF4B26BF568158F582ECE5.gif\" id=\"zzz\" size=\"100,50\" max-size=\"\" minsize=\"\" scaring=\"1\" cursor=\"hand\" />"
            L"</para>"
            L"<bubble id=\"\" left-bubble=\"skin.rich_left_bubble\" right-bubble=\"skin.rich_right_bubble\" pos-left=\"35,{-9,[15,[10\" pos-right=\"{-10,{-9,-35,[10\"/>"
        L"</RichEditContent>";

    /* 居中显示的文件发送消息 */

    const TCHAR * pFormatedCenterText =
        L"<RichEditContent id=\"201601015337090001\" type=\"msg\" >"
            L"<para break=\"1\" />"
            L"<para id=\"righttext\" margin=\"100,0,100,0\" break=\"1\" align=\"center\" simulate-align=\"1\">"
            L"<text font-size=\"10\" >您取消了“Adobe_Photoshop_CS5_green_7xdown.com.rar”(118.91MB)的发送，文件传输失败。</text>"
            L"</para>"
            L"<bkimage id=\"\" src=\"skin.rich_sysmsg_bk\"  pos=\"{-30,{0,[10,[0\" />"
            L"<bkimage id=\"\" src=\"skin.rich_warning\"  pos=\"{10,{3,@16,@16\" />"
        L"</RichEditContent>";

    /* 居中显示的签到消息 */

    const TCHAR * pFormatedCenterText2 =
        L"<RichEditContent id=\"201601015337090001\" type=\"msg\" >"

        L"<para id=\"righttext\" margin=\"100,10,100,10\" break=\"1\" align=\"center\" >"
            L"<text font-size=\"10\"  color=\"#808080\">13:03:05</text>"
        L"</para>"

        L"<para id=\"righttext\" margin=\"100,0,100,0\" break=\"1\" align=\"center\" >"
        L"<text font-size=\"10\">蓝先生在火星签到</text>"
        L"</para>"
        L"<bkimage id=\"\" src=\"skin.rich_sysmsg_bk\"  pos=\"{-10,{0,[10,[0\" />"
        L"</RichEditContent>";

    /* 文件消息块 */

    const TCHAR * pFormatedFileText =
        L"<RichEditContent id=\"201511242337090001\" type=\"msg\" align=\"left\" >"
            L"<para margin=\"36,10,100,4\">"
                L"<text font-size=\"10\" color=\"#808080\">【传说】蓝先生</text>"
            L"</para>"
            L"<avatar id=\"\" src=\"avatar\" pos-left=\"0,]-18,@30,@30\" pos-right=\"-30,]-15,@30,@30\" />"
            L"<para margin=\"50,0,30,0\" break=\"1\">"
                L"<file id=\"file_id_xxxx\" selectable=\"0\"/>"
            L"</para>"
            L"<bubble id=\"\" left-bubble=\"skin.rich_left_bubble\" right-bubble=\"skin.rich_right_bubble\" pos-left=\"35,{-9,[15,[10\" pos-right=\"{-10,{-9,-35,[10\"/>"
        L"</RichEditContent>";

    static int i = 0;

    if (i == 0)
    {
        m_preRecord->InsertContent(pFetchMore);
        m_preRecord->InsertContent(pRemainderLeft);
        m_preRecord->InsertContent(pRemainderRight); 
        m_preRecord->InsertContent(pFormatedCenterText); 
        m_preRecord->InsertContent(pFormatedCenterText2); 
        m_preRecord->InsertContent(pFormatedFileText); 
        m_preRecord->InsertContent(pSplitLine);
    }

    i += 1;
    //int n = 0;
    //str.Format(pFormatedLeftText2, i++, L"2");
    //int n = m_pRichEditMsg->InsertContent(str, m_pRichEditMsg->GetContent(0));
    //n = m_pRichEditMsg->InsertContent(str);

    //str.Format(pFormatedLeftText, i++, L"你好啊");
    //n = m_pRichEditMsg->InsertContent(str, 0);
    //m_pRichEditMsg->InserSpaceLine(n+1);

    //return;
    //m_pRichEditMsg->InsertContent(pFormatedCenterText);

    SStringW strInput = m_preInput->GetWindowText();
    strInput = XMLEscape(strInput);
    if (strInput.GetLength() > 0)
    {
        for (int n = 0; n < 2; ++n)
        {
            if (i%2==1)
            {
                str.Format(pFormatedLeftText, i,strInput);
                //str.Format(pFormatedLeftText, strInput);
                //str.Format(pFormatedLeftText);
            }
            else
            {
                m_preRecord->InserSpaceLine();
                str.Format(pFormatedRightText, strInput);
                //str.Format(pFormatedRightText);
            }

            m_preRecord->InsertContent(str);
            i += 1;
        }
    }
    return;

    // 新闻消息，还有问题
    m_preRecord->InsertContent(pNews);
    RichEditNewsOle * pNewsOle = static_cast<RichEditNewsOle*>(m_preRecord->GetOleById(L"news_2015121501"));
    if (pNewsOle)
    {
        pNewsOle->AddNews(L"这是一条测试新闻", L"这是一条测试新闻测试新闻测试新闻", L"www.baidu.com", L"uires\\image\\image2.png");
        pNewsOle->AddNews(L"这是一条测试新闻", L"这是一条测试新闻测试新闻测试新闻", L"www.google.com", L"uires\\image\\image2.png");
        pNewsOle->AddNews(L"这是一条测试新闻", L"这是一条测试新闻测试新闻测试新闻", L"www.yahoo.com", L"uires\\image\\image2.png");
        pNewsOle->AddNews(L"这是一条测试新闻", L"这是一条测试新闻测试新闻测试新闻", L"www.bing.com", L"uires\\image\\image2.png");
        pNewsOle->ReLayout();
    }
}

void GroupChatFrame::OnShowSendTypeMenu()
{
    CPoint pt;
    GetCursorPos(&pt);

    SMenuEx menu;
    menu.LoadMenu(_T("SMENU:menu_send"));
    menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd);
}
