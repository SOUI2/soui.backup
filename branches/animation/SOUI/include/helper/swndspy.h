#pragma once

#define SPYMSG_BASE      (WM_USER+1000)
#define SPYMSG_SETSPY    (SPYMSG_BASE+0)     //设置SPY消息接收窗口句柄

#define SPYMSG_SWNDENUM  (SPYMSG_BASE+1)     //枚举窗口列表,wparam:SWND,lparam:SWindow::GetWindow
                                            //返回窗口的SWND值

#define SPYMSG_SWNDINFO  (SPYMSG_BASE+2)      //获取窗口属性，wparam:swnd

#define SPYMSG_HITTEST   (SPYMSG_BASE+3)    //lparam:pos

#define SWND_MAX_NAME   250
#define SWND_MAX_CLASS  50
#define SWND_MAX_XML    5000

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