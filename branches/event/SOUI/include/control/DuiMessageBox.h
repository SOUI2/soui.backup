/********************************************************************
    created:    2013/02/19
    created:    19:2:2013   10:11
    filename:     DuiMessageBox.h
    author:        Huang Jianxiong
    
    purpose:    模拟一个MessageBox
*********************************************************************/
#pragma once

#include "duihostwnd.h"

namespace SOUI
{
    //下面是几个在msgbox模板中必须指定的ID。
    #define NAME_MSGBOX_BTN1PANEL    L"btn1panel"    //包含单个按钮的panel
    #define NAME_MSGBOX_BTN2PANEL    L"btn2panel"    //包含2个按钮的panel
    #define NAME_MSGBOX_BTN3PANEL    L"btn3panel"    //包含3个按钮的panel
    #define NAME_MSGBOX_TEXT            L"msgtext"    //文本控件，只需要指定两个坐标
    #define NAME_MSGBOX_TITLE        L"msgtitle"    //标题ID
    #define NAME_MSGBOX_ICON            L"msgicon"    //图标显示控件
    #define NAME_MSGBOX_BTN1            L"button1st"    //第1个按钮ID，按钮ID在显示时会自动修改为如IDOK,IDCANCEL这样的ID。
    #define NAME_MSGBOX_BTN2            L"button2nd"    //第2个按钮ID
    #define NAME_MSGBOX_BTN3            L"button3rd"    //第3个按钮ID

    //msgbox的消息处理对象，如果需要更加个性化的msgbox，可以派生该类。
    class SOUI_EXP SMessageBoxImpl:public CDuiHostWnd
    {
    public:
        int MessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType );

        static BOOL SetMsgTemplate(pugi::xml_node xmlNode);
    protected:
        //可以重载该方法来显示自定义的图标
        virtual BOOL SetIcon(UINT uType);

        void OnBtnClick(UINT uID)
        {
            EndDialog(uID);
        }

        static pugi::xml_document s_xmlMsgTemplate;

//         SOUI_NOTIFY_MAP_BEGIN()
//             SOUI_NOTIFY_ID_COMMAND_RANGE(IDOK,IDNO, OnBtnClick)
//         SOUI_NOTIFY_MAP_END()    

        BEGIN_MSG_MAP_EX(SMessageBoxImpl)
//             MSG_SOUI_NOTIFY()
            CHAIN_MSG_MAP(CDuiHostWnd)
            REFLECT_NOTIFICATIONS_EX()
        END_MSG_MAP()
    };

    int SOUI_EXP SMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);

}//end of namespace 

