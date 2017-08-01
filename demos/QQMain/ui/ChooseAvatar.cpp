// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChooseAvatar.h"
#include "FileHelper.h"
#include "SProfilePicture.h"



#define SUBSCRIBE(x,y,z) (x)->GetEventSet()->subscribeEvent(y,Subscriber(&z,this))


ChooseAvatarDlg::ChooseAvatarDlg() : SHostDialog(UIRES.LAYOUT.dlg_choose_avatar)
{
    m_bLayoutInited = FALSE;
}

ChooseAvatarDlg::~ChooseAvatarDlg()
{
}

int ChooseAvatarDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    SetMsgHandled(FALSE);
    return 0;
}

BOOL ChooseAvatarDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
    m_bLayoutInited = TRUE;

    m_pHeadPic = FindChildByName2<SProfilePicture>(L"picchoose");
    m_img_HeadPic_Large = FindChildByName2<SImageWnd>(L"HeadPic_Large");
    m_img_HeadPic_Middle = FindChildByName2<SImageWnd>(L"HeadPic_Middle");
    m_img_HeadPic_Small = FindChildByName2<SImageWnd>(L"HeadPic_Small");

	SUBSCRIBE(m_pHeadPic, SProfilePicture::EventSelFrameChange::EventID, ChooseAvatarDlg::OnPicFrameChange);

    return 0;
}

bool ChooseAvatarDlg::OnPicFrameChange(SOUI::EventArgs *pEvt)
{
    SProfilePicture::EventSelFrameChange* pFrameEvt = (SProfilePicture::EventSelFrameChange*)pEvt;
    if (pFrameEvt->iBmp)
    {
        m_img_HeadPic_Large->SetImage(pFrameEvt->iBmp);
        m_img_HeadPic_Middle->SetImage(pFrameEvt->iBmp);
        m_img_HeadPic_Small->SetImage(pFrameEvt->iBmp);
    }
    return true;
}

void ChooseAvatarDlg::OnChooseHeadPic()
{
    CFileDialogEx openDlg(TRUE, _T("gif"), 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("图片(*.jpg;*.jpeg;*.png)\0*.jpg;*.jpeg;*.png\0\0"));
    openDlg.m_ofn.lpstrTitle = _T("选择头像");
    if (openDlg.DoModal() == IDOK)
    {
        SStringT ttt(openDlg.m_szFileName);
        BOOL res = openDlg.m_bOpenFileDialog;
        SStringT ooo(openDlg.m_szFileTitle);
        if (!PathFileExists(ttt))
        {
            SMessageBox(m_hWnd, _T("图片不存在"), _T("提示"), MB_ICONWARNING);
            return;
        }

//         struct _stat  info;
//         _stat(S_CT2A(ttt), &info);
//         long filesize = info.st_size;
// 
//         if (filesize <= 0)
//         {
//             SMessageBox(m_hWnd, _T("图片大小不能为空"), _T("提示"), MB_ICONWARNING);
//             return;
//         }

        m_pHeadPic->SetHeadPic(ttt);
    }
}

void ChooseAvatarDlg::OnSaveHeadPic()
{
    // 保存截取的头像
}


//TODO:消息映射
void ChooseAvatarDlg::OnClose()
{
    EndDialog(IDCANCEL);
}

void ChooseAvatarDlg::OnMaximize()
{
    SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void ChooseAvatarDlg::OnRestore()
{
    SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void ChooseAvatarDlg::OnMinimize()
{
    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void ChooseAvatarDlg::OnSize(UINT nType, CSize size)
{
    SetMsgHandled(FALSE);
    if (!m_bLayoutInited) return;

    SWindow *pBtnMax = FindChildByName(L"btn_max");
    SWindow *pBtnRestore = FindChildByName(L"btn_restore");
    if (!pBtnMax || !pBtnRestore) return;

    if (nType == SIZE_MAXIMIZED)
    {
        pBtnRestore->SetVisible(TRUE);
        pBtnMax->SetVisible(FALSE);
    }
    else if (nType == SIZE_RESTORED)
    {
        pBtnRestore->SetVisible(FALSE);
        pBtnMax->SetVisible(TRUE);
    }
}

