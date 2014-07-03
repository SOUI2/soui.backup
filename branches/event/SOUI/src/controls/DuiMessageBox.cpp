#include "duistd.h"
#include "DuiSystem.h"
#include "control/DuiMessageBox.h"
#include "control/DuiCmnCtrl.h"

namespace SOUI
{

    pugi::xml_document SMessageBoxImpl::s_xmlMsgTemplate;


    BOOL SMessageBoxImpl::SetMsgTemplate( pugi::xml_node uiRoot )
    {
        if(wcscmp(uiRoot.name(),L"UIFRAME")!=0 ) return FALSE;
        if(!uiRoot.attribute(L"frame_size").value()[0]) return FALSE;
        if(!uiRoot.attribute(L"minsize").value()[0]) return FALSE;

        s_xmlMsgTemplate.reset();
        s_xmlMsgTemplate.append_copy(uiRoot);
        return TRUE;
    }

    int SMessageBoxImpl::MessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
    {
        if(!s_xmlMsgTemplate) return ::MessageBox(hWnd,lpText,lpCaption,uType);

        Create(NULL,NULL,WS_POPUPWINDOW,0,0,0,10,10,NULL);
        pugi::xml_node uiRoot=s_xmlMsgTemplate.child(L"UIFRAME");
        InitFromXml(uiRoot);


        switch(uType&0x0F)
        {
        case MB_ABORTRETRYIGNORE:
            {
                FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
                FindChildByName(NAME_MSGBOX_BTN2PANEL)->SetVisible(FALSE);
                SWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN3PANEL);
                pBtnPanel->SetVisible(TRUE);
                SWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
                pBtn->SetWindowText(_T("中止"));pBtn->SetID(IDABORT);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
                pBtn->SetWindowText(_T("重试"));pBtn->SetID(IDRETRY);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN3);
                pBtn->SetWindowText(_T("忽略"));pBtn->SetID(IDIGNORE);
            }
            break;
        case MB_YESNOCANCEL:
            {
                FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
                FindChildByName(NAME_MSGBOX_BTN2PANEL)->SetVisible(FALSE);
                SWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN3PANEL);
                pBtnPanel->SetVisible(TRUE);
                SWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
                pBtn->SetWindowText(_T("是"));pBtn->SetID(IDYES);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
                pBtn->SetWindowText(_T("否"));pBtn->SetID(IDNO);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN3);
                pBtn->SetWindowText(_T("取消"));pBtn->SetID(IDCANCEL);
            }
            break;
        case MB_OKCANCEL:
            {
                FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
                FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

                SWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN2PANEL);
                pBtnPanel->SetVisible(TRUE);

                SWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
                pBtn->SetWindowText(_T("确定"));    pBtn->SetID(IDOK);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
                pBtn->SetWindowText(_T("取消"));    pBtn->SetID(IDCANCEL);
            }
            break;
        case MB_YESNO:
            {
                FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
                FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

                SWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN2PANEL);
                pBtnPanel->SetVisible(TRUE);

                SWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
                pBtn->SetWindowText(_T("是"));    pBtn->SetID(IDYES);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
                pBtn->SetWindowText(_T("否"));    pBtn->SetID(IDNO);
            }
            break;
        case MB_RETRYCANCEL:
            {
                FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
                FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

                SWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN2PANEL);
                pBtnPanel->SetVisible(TRUE);

                SWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
                pBtn->SetWindowText(_T("重试"));    pBtn->SetID(IDRETRY);
                pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
                pBtn->SetWindowText(_T("取消"));    pBtn->SetID(IDCANCEL);
            }
            break;
        case MB_OK:
            {
                FindChildByName(NAME_MSGBOX_BTN2PANEL)->SetVisible(FALSE);
                FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

                SWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN1PANEL);
                pBtnPanel->SetVisible(TRUE);
                SWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
                pBtn->SetWindowText(_T("确定"));    pBtn->SetID(IDOK);
            }
            break;
        default:
            ASSERT(FALSE);
            break;
        }
        const wchar_t *pszFrameAttr=uiRoot.attribute(L"frame_size").value();
        CRect rcFrame;
        swscanf(pszFrameAttr,L"%d,%d,%d,%d",&rcFrame.left,&rcFrame.top,&rcFrame.right,&rcFrame.bottom);
        CSize szMin;
        const wchar_t *pszMinAttr=uiRoot.attribute(L"minsize").value();
        swscanf(pszMinAttr,L"%d,%d",&szMin.cx,&szMin.cy);

        SWindow * pTitle= FindChildByName(NAME_MSGBOX_TITLE);
        ASSERT(pTitle);
        pTitle->SetWindowText(lpCaption?lpCaption:_T("提示"));

        SWindow * pMsg= FindChildByName(NAME_MSGBOX_TEXT);
        ASSERT(pMsg);
        pMsg->SetWindowText(lpText);

        SetIcon(uType);

        CRect rcText;
        pMsg->GetRect(&rcText);

        CSize szWnd;
        szWnd.cx=max(szMin.cx,rcText.Width()+rcFrame.left+rcFrame.right);
        szWnd.cy=max(szMin.cy,rcText.Height()+rcFrame.top+rcFrame.bottom);

        SetWindowPos(HWND_TOPMOST,0,0,szWnd.cx,szWnd.cy,SWP_NOMOVE);
        CenterWindow(m_hWnd);
        
        return DoModal(hWnd);
    }

    BOOL SMessageBoxImpl::SetIcon( UINT uType )
    {
        SIconWnd *pIcon=(SIconWnd *)FindChildByName(NAME_MSGBOX_ICON);
        if(!pIcon) return FALSE;
        switch(uType&0xF0)
        {
        case MB_ICONEXCLAMATION:
            pIcon->SetIcon(LoadIcon(NULL,IDI_EXCLAMATION));
            break;
        case MB_ICONINFORMATION:
            pIcon->SetIcon(LoadIcon(NULL,IDI_INFORMATION));
            break;
        case MB_ICONQUESTION:
            pIcon->SetIcon(LoadIcon(NULL,IDI_QUESTION));
            break;
        case MB_ICONHAND:
            pIcon->SetIcon(LoadIcon(NULL,IDI_HAND));
            break;
        }
        return TRUE;
    }


    int SMessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
    {
        SMessageBoxImpl duiMsgBox;
        return duiMsgBox.MessageBox(hWnd,lpText,lpCaption,uType);
    }



}//end of namespace 
