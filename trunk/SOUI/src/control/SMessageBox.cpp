#include "souistd.h"
#include "SApp.h"
#include "control/SMessageBox.h"
#include "control/SCmnCtrl.h"

namespace SOUI
{

    pugi::xml_document SMessageBoxImpl::s_xmlMsgTemplate;
    
    const TCHAR kTrCtx_Msgbox[] = _T("messagebox"); //MSGBOX的翻译上下文

    BOOL SMessageBoxImpl::SetMsgTemplate( pugi::xml_node uiRoot )
    {
        if(wcscmp(uiRoot.name(),L"SOUI")!=0 ) return FALSE;
        if(!uiRoot.attribute(L"frameSize").value()[0]) return FALSE;
        if(!uiRoot.attribute(L"minSize").value()[0]) return FALSE;

        s_xmlMsgTemplate.reset();
        s_xmlMsgTemplate.append_copy(uiRoot);
        return TRUE;
    }

    SMessageBoxImpl::SMessageBoxImpl() :SHostDialog(NULL)
    {

    }
    
    static struct MsgBoxInfo
    {
        LPCTSTR pszText;
        LPCTSTR pszCaption;
        UINT    uType;
    }s_MsgBoxInfo;
    
    INT_PTR SMessageBoxImpl::MessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
    {
        if(!s_xmlMsgTemplate) return ::MessageBox(hWnd,lpText,lpCaption,uType);
        s_MsgBoxInfo.pszText=lpText;
        s_MsgBoxInfo.pszCaption=lpCaption;
        s_MsgBoxInfo.uType=uType;
        
        return DoModal(hWnd);
    }

    BOOL SMessageBoxImpl::OnSetIcon( UINT uType )
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
        default:
            pIcon->SetVisible(FALSE,TRUE);
            break;
        }
        return TRUE;
    }

    struct MSGBTN_TEXT
    {
        int   nBtns;
        struct
        {
            UINT uBtnID;
            TCHAR szText[20];       
        }btnInfo[3];
    }g_msgBtnText[]=
    {
        //MB_OK
        {
            1,
            {
                {IDOK,    _T("ok")},
                {0,    _T("")},
                {0,   _T("")}
            }
        },
        //MB_OKCANCEL
        {
            2,
            {
                {IDOK,    _T("ok")},
                {IDCANCEL,    _T("cancel")},
                {0,    _T("")}
            }
        },
        //MB_ABORTRETRYIGNORE
        {
            3,
            {
                {IDABORT,_T("abort")},
                {IDRETRY,_T("retry")},
                {IDIGNORE,_T("ignore")}
            }
        },
        //MB_YESNOCANCEL
        {
            3,
            {
                {IDYES,_T("yes")},
                {IDNO,_T("no")},
                {IDCANCEL,_T("cancel")}
            }
        },
        //MB_YESNO
        {
            2,
            {
                {IDYES,_T("yes")},
                {IDNO,_T("no")},
                {0,_T("")}
            }
        },
        //MB_RETRYCANCEL
        {
            2,
            {
                {IDRETRY,_T("retry")},
                {IDCANCEL,_T("cancel")},
                {0,_T("")}
            }
        }
    };
    
    const WCHAR * g_wcsNameOfBtns[] =
    {
        NAME_MSGBOX_BTN1,
        NAME_MSGBOX_BTN2,
        NAME_MSGBOX_BTN3
    };
    
    BOOL SMessageBoxImpl::OnInitDialog( HWND wnd, LPARAM lInitParam )
    {
        pugi::xml_node uiRoot=s_xmlMsgTemplate.child(L"SOUI");
        
        InitFromXml(uiRoot);
        UINT uType = s_MsgBoxInfo.uType&0x0F;

        STabCtrl *pBtnSwitch= FindChildByName2<STabCtrl>(NAME_MSGBOX_BTNSWITCH);
        SASSERT(pBtnSwitch);
        pBtnSwitch->SetCurSel(g_msgBtnText[uType].nBtns-1);
        SWindow *pBtnPanel=pBtnSwitch->GetItem(g_msgBtnText[uType].nBtns-1);
        SASSERT(pBtnPanel);
        
        for(int i=0; i<g_msgBtnText[uType].nBtns; i++)
        {
            SWindow *pBtn=pBtnPanel->FindChildByName(g_wcsNameOfBtns[i]);
            pBtn->SetWindowText(TR(g_msgBtnText[uType].btnInfo[i].szText,kTrCtx_Msgbox));    
            pBtn->SetID(g_msgBtnText[uType].btnInfo[i].uBtnID);
        }
        
        const wchar_t *pszFrameAttr=uiRoot.attribute(L"frameSize").value();
        CRect rcFrame;
        swscanf(pszFrameAttr,L"%d,%d,%d,%d",&rcFrame.left,&rcFrame.top,&rcFrame.right,&rcFrame.bottom);
        CSize szMin;
        const wchar_t *pszMinAttr=uiRoot.attribute(L"minSize").value();
        swscanf(pszMinAttr,L"%d,%d",&szMin.cx,&szMin.cy);

        SWindow * pTitle= FindChildByName(NAME_MSGBOX_TITLE);
        SASSERT(pTitle);
        pTitle->SetWindowText(TR(s_MsgBoxInfo.pszCaption?s_MsgBoxInfo.pszCaption:_T("prompt"),kTrCtx_Msgbox));

        SWindow * pMsg= FindChildByName(NAME_MSGBOX_TEXT);
        SASSERT(pMsg);
        pMsg->SetWindowText(TR(s_MsgBoxInfo.pszText,kTrCtx_Msgbox));

        OnSetIcon(s_MsgBoxInfo.uType);

        CRect rcText;
        pMsg->GetWindowRect(&rcText);

        CSize szWnd;
        szWnd.cx=max(szMin.cx,rcText.Width()+rcFrame.left+rcFrame.right);
        szWnd.cy=max(szMin.cy,rcText.Height()+rcFrame.top+rcFrame.bottom);

        SetWindowPos(0,0,0,szWnd.cx,szWnd.cy,SWP_NOMOVE);     
        CenterWindow(wnd);  
        return 0;
    }

    int SMessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
    {
        SMessageBoxImpl duiMsgBox;
        return duiMsgBox.MessageBox(hWnd,lpText,lpCaption,uType);
    }

}//end of namespace 
