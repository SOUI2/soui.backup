#pragma once

#define SPYMSG_SETSPY    (WM_USER+1000)     //设置SPY消息接收窗口句柄

#define SPYMSG_SWNDENUM  (WM_USER+1001)     //枚举窗口列表,wparam:SWND,lparam:SWindow::GetWindow
                                            //返回窗口的SWND值

#define SPYMSG_SWNDINFO  (WM_USER+1002)      //获取窗口属性，wparam:swnd,lparam:Cookie

#define SWND_MAX_NAME   250
#define SWND_MAX_CLASS  50
#define SWND_MAX_XML    500

#pragma pack(push,1)
struct SWNDINFO
{
    DWORD swnd;
    RECT rcWnd;
    RECT rcClient;
    BOOL bVisible;
    int  nID;
    wchar_t szName[SWND_MAX_NAME+1];
    wchar_t szClassName[SWND_MAX_CLASS+1];
    wchar_t szXmlStr[SWND_MAX_XML+1];
};
#pragma pack(pop)