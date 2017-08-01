// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once


#include "SProfilePicture.h"

class ChooseAvatarDlg : public SHostDialog
{
public:
    ChooseAvatarDlg();
    ~ChooseAvatarDlg();

    void OnClose();
    void OnMaximize();
    void OnRestore();
    void OnMinimize();
    void OnSize(UINT nType, CSize size);

    void OnBtnMsgBox();
    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
    bool OnPicFrameChange(SOUI::EventArgs * pEvt);
    void OnChooseHeadPic();
    void OnSaveHeadPic();


    SImageWnd* m_img_HeadPic_Large;
    SImageWnd* m_img_HeadPic_Middle;
    SImageWnd* m_img_HeadPic_Small;
    SProfilePicture* m_pHeadPic;

protected:
    //soui消息

    EVENT_MAP_BEGIN()
        EVENT_NAME_COMMAND(L"btn_close", OnClose)
        EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
        EVENT_ID_COMMAND(R.id.btnChoose, OnChooseHeadPic)
        EVENT_ID_COMMAND(R.id.btnSave, OnSaveHeadPic)
    EVENT_MAP_END()

        //HostWnd真实窗口消息处理
    BEGIN_MSG_MAP_EX(ChooseAvatarDlg)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP(SHostWnd)
        REFLECT_NOTIFICATIONS_EX()
    END_MSG_MAP()
private:
    BOOL			m_bLayoutInited;
};
