#pragma once
#include <vector>

namespace SOUI
{

    //----------------------------------------------------------------------------------
    //
    // 通用扩展通知, 起始偏移300
    //
    //----------------------------------------------------------------------------------
#define EVT_STDEXT_BEGIN           (EVT_EXTERNAL_BEGIN + 300)
#define EVT_STD_WINDOWS            (EVT_STDEXT_BEGIN+0)

    //
    // 控件用于通知自己收到的WINDOWS消息
    //

    class EventStdWindows : public TplEventArgs < EventStdWindows >
    {
        SOUI_CLASS_NAME(EventStdWindows, L"on_mouse_event")
    public:
        EventStdWindows(SWindow *pSender) :TplEventArgs<EventStdWindows>(pSender)
        {
        }

        enum { EventID = EVT_STD_WINDOWS };

        SWNDMSG msg;
    };

    //----------------------------------------------------------------------------------
    //
    // TabView 相关通知，起始偏移400
    //
    //----------------------------------------------------------------------------------

#define EVT_TABVIEW_BEGIN           (EVT_EXTERNAL_BEGIN + 400)
#define EVT_TABVIEW_NEW             (EVT_TABVIEW_BEGIN + 0)
#define EVT_TABVIEW_CLOSE           (EVT_TABVIEW_BEGIN + 1)
#define EVT_TABVIEW_SELCHANGED      (EVT_TABVIEW_BEGIN + 2)

    class EventTabViewNew : public TplEventArgs < EventTabViewNew >
    {
        SOUI_CLASS_NAME(EventTabViewNew, L"on_tabview_new")
    public:
        EventTabViewNew(SWindow *pSender) :TplEventArgs<EventTabViewNew>(pSender)
        {

        }
        enum { EventID = EVT_TABVIEW_NEW };
        SWindow * pNewTab;

        int       iNewTab;
    };

    class EventTabViewClose : public TplEventArgs < EventTabViewClose >
    {
        SOUI_CLASS_NAME(EventTabViewClose, L"on_tabview_close")
    public:
        EventTabViewClose(SWindow *pSender) :TplEventArgs<EventTabViewClose>(pSender)
        {

        }
        enum { EventID = EVT_TABVIEW_CLOSE };

        SWindow * pCloseTab;

        int       iCloseTab;
    };

    class EventTabviewSelChanged : public TplEventArgs < EventTabviewSelChanged >
    {
        SOUI_CLASS_NAME(EventTabviewSelChanged, L"on_tabview_sel_changed")
    public:
        EventTabviewSelChanged(SWindow *pSender) :TplEventArgs<EventTabviewSelChanged>(pSender)
        {

        }
        enum { EventID = EVT_TABVIEW_SELCHANGED };

        int         iOldSel;
        int         iNewSel;
    };

    //----------------------------------------------------------------------------------
    //
    // RichEdit 相关通知，起始偏移600
    //
    //----------------------------------------------------------------------------------

#define EVT_RICHEDIT_BEGIN              (EVT_EXTERNAL_BEGIN + 600)
#define EVT_RE_QUERY_ACCEPT             (EVT_RICHEDIT_BEGIN+0)
#define EVT_RE_OBJ                      (EVT_RICHEDIT_BEGIN+1)
#define EVT_RE_SCROLLBAR                (EVT_RICHEDIT_BEGIN+2)

    class RichFormatConv;
    class EventQueryAccept : public TplEventArgs < EventQueryAccept >
    {
        SOUI_CLASS_NAME(EventQueryAccept, L"on_re_query_accept_data")

    public:

        EventQueryAccept(SWindow *pSender) :TplEventArgs<EventQueryAccept>(pSender)
        {
        }

        enum { EventID = EVT_RE_QUERY_ACCEPT };

        RichFormatConv * Conv;
    };

    class RichEditObj;
    class EventRichEditObj : public TplEventArgs < EventRichEditObj >
    {
        SOUI_CLASS_NAME(EventRichEditObj, L"on_re_ole")

    public:

        EventRichEditObj(SWindow *pSender) :TplEventArgs<EventRichEditObj>(pSender)
        {
        }

        enum { EventID = EVT_RE_OBJ };

        RichEditObj * RichObj;
        int           SubEventId;
        WPARAM        wParam;
        LPARAM        lParam;
    };

    class EventRichEditScroll : public TplEventArgs < EventRichEditScroll >
    {
        SOUI_CLASS_NAME(EventRichEditScroll, L"on_re_scroll")

    public:

        EventRichEditScroll(SWindow *pSender) :TplEventArgs<EventRichEditScroll>(pSender)
        {
            WheelDelta = 0;
            ScrollAtTop = FALSE;
            ScrollAtBottom = FALSE;
        }

        enum { EventID = EVT_RE_SCROLLBAR };

        int WheelDelta;
        BOOL ScrollAtTop;
        BOOL ScrollAtBottom;

        //SCROLLBARINFO ScrollInfo;
    };

    //----------------------------------------------------------------------------------
    //
    // List 相关通知，起始偏移700
    //
    //----------------------------------------------------------------------------------

#define EVT_LIST_BEGIN              (EVT_EXTERNAL_BEGIN + 700)
#define EVT_LIST_HOVER_CHANGED      (EVT_LIST_BEGIN+0)

    class EventListHoverChanged : public TplEventArgs < EventListHoverChanged >
    {
        SOUI_CLASS_NAME(EventListHoverChanged, L"on_list_hover_changed")
    public:
        EventListHoverChanged(SWindow *pSender) :TplEventArgs<EventListHoverChanged>(pSender)
        {
        }
        enum { EventID = EVT_LIST_HOVER_CHANGED };

        int nHoverNew;
        int nHoverOld;
    };

    //----------------------------------------------------------------------------------
    //
    // DropDown 窗口相关通知，起始偏移800
    //
    //----------------------------------------------------------------------------------

#define EVT_DROPDOWN_BEGIN          (EVT_EXTERNAL_BEGIN + 800)
#define EVT_DD_ITEM_SELECTED        (EVT_DROPDOWN_BEGIN+0)

    class EventDropDownItemSelected : public TplEventArgs < EventDropDownItemSelected >
    {
        SOUI_CLASS_NAME(EventDropDownItemSelected, L"on_dropdown_item_selected")
    public:
        EventDropDownItemSelected(SObject *pSender) :TplEventArgs<EventDropDownItemSelected>(pSender) {}
        enum { EventID = EVT_DD_ITEM_SELECTED };

        int CurrentSelected;
    };

    //----------------------------------------------------------------------------------
    //
    // CefWebView 窗口相关通知，起始偏移900
    //
    //----------------------------------------------------------------------------------

#define EVT_CEFWEBVIEW_BEGIN        (EVT_EXTERNAL_BEGIN + 900)
#define EVT_WEBVIEW_NOTIFY          (EVT_CEFWEBVIEW_BEGIN+0)

    class EventWebViewNotify : public TplEventArgs < EventWebViewNotify >
    {
        SOUI_CLASS_NAME(EventWebViewNotify, L"on_webview_notify")
    public:
        EventWebViewNotify(SObject *pSender) :TplEventArgs<EventWebViewNotify>(pSender) {}
        enum { EventID = EVT_WEBVIEW_NOTIFY };

        SStringW         MessageName;
        SArray<SStringW> Arguments;
    };

    //----------------------------------------------------------------------------------
    //
    // ImgCache 相关通知，起始偏移1000
    //
    //----------------------------------------------------------------------------------

#define EVT_IMGCACHE_BEGIN          (EVT_EXTERNAL_BEGIN + 1000)
#define EVT_DONE_UPDATE             (EVT_IMGCACHE_BEGIN+0)

    class ImageAttr;

    class EventImgCacheNotify : public TplEventArgs < EventImgCacheNotify >
    {
        SOUI_CLASS_NAME(EventImgCacheNotify, L"on_imgcache_notify")
    public:
        EventImgCacheNotify(SObject *pSender) : TplEventArgs<EventImgCacheNotify>(pSender)
            , Attrs(NULL)
            , Context(0)
        {
        }

        enum { EventID = EVT_DONE_UPDATE };

        std::vector<ImageAttr*>* Attrs;
        int         Context;
    };

}// namespace SOUI
