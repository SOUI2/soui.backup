#include "duistd.h"
#include "DuiMessageBox.h"
#include "DuiSystem.h"
#include "DuiCmnCtrl.h"

namespace SOUI
{


	int CDuiMessageBox::MessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
	{
		pugi::xml_node xmlTempl=DuiSystem::getSingleton().GetMsgBoxTemplate().child("SOUI");
		if(!xmlTempl) return ::MessageBox(hWnd,lpText,lpCaption,uType);

		Create(NULL,NULL,WS_POPUPWINDOW,0,0,0,10,10,NULL);
		SetXml(xmlTempl);


		switch(uType&0x0F)
		{
		case MB_ABORTRETRYIGNORE:
			{
				FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
				FindChildByName(NAME_MSGBOX_BTN2PANEL)->SetVisible(FALSE);
				CDuiWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN3PANEL);
				pBtnPanel->SetVisible(TRUE);
				CDuiWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
				pBtn->SetInnerText(_T("中止"));pBtn->SetCmdID(IDABORT);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
				pBtn->SetInnerText(_T("重试"));pBtn->SetCmdID(IDRETRY);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN3);
				pBtn->SetInnerText(_T("忽略"));pBtn->SetCmdID(IDIGNORE);
			}
			break;
		case MB_YESNOCANCEL:
			{
				FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
				FindChildByName(NAME_MSGBOX_BTN2PANEL)->SetVisible(FALSE);
				CDuiWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN3PANEL);
				pBtnPanel->SetVisible(TRUE);
				CDuiWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
				pBtn->SetInnerText(_T("是"));pBtn->SetCmdID(IDYES);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
				pBtn->SetInnerText(_T("否"));pBtn->SetCmdID(IDNO);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN3);
				pBtn->SetInnerText(_T("取消"));pBtn->SetCmdID(IDCANCEL);
			}
			break;
		case MB_OKCANCEL:
			{
				FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
				FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

				CDuiWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN2PANEL);
				pBtnPanel->SetVisible(TRUE);

				CDuiWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
				pBtn->SetInnerText(_T("确定"));	pBtn->SetCmdID(IDOK);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
				pBtn->SetInnerText(_T("取消"));	pBtn->SetCmdID(IDCANCEL);
			}
			break;
		case MB_YESNO:
			{
				FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
				FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

				CDuiWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN2PANEL);
				pBtnPanel->SetVisible(TRUE);

				CDuiWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
				pBtn->SetInnerText(_T("是"));	pBtn->SetCmdID(IDYES);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
				pBtn->SetInnerText(_T("否"));	pBtn->SetCmdID(IDNO);
			}
			break;
		case MB_RETRYCANCEL:
			{
				FindChildByName(NAME_MSGBOX_BTN1PANEL)->SetVisible(FALSE);
				FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

				CDuiWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN2PANEL);
				pBtnPanel->SetVisible(TRUE);

				CDuiWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
				pBtn->SetInnerText(_T("重试"));	pBtn->SetCmdID(IDRETRY);
				pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN2);
				pBtn->SetInnerText(_T("取消"));	pBtn->SetCmdID(IDCANCEL);
			}
			break;
		case MB_OK:
			{
				FindChildByName(NAME_MSGBOX_BTN2PANEL)->SetVisible(FALSE);
				FindChildByName(NAME_MSGBOX_BTN3PANEL)->SetVisible(FALSE);

				CDuiWindow *pBtnPanel=FindChildByName(NAME_MSGBOX_BTN1PANEL);
				pBtnPanel->SetVisible(TRUE);
				CDuiWindow *pBtn=pBtnPanel->FindChildByName(NAME_MSGBOX_BTN1);
				pBtn->SetInnerText(_T("确定"));	pBtn->SetCmdID(IDOK);
			}
			break;
		default:
			DUIASSERT(FALSE);
			break;
		}
		const char *pszFrameAttr=xmlTempl.attribute("frame_size").value();
		CRect rcFrame;
		sscanf(pszFrameAttr,"%d,%d,%d,%d",&rcFrame.left,&rcFrame.top,&rcFrame.right,&rcFrame.bottom);
		CSize szMin;
		const char *pszMinAttr=xmlTempl.attribute("minsize").value();
		sscanf(pszMinAttr,"%d,%d",&szMin.cx,&szMin.cy);

		CDuiWindow * pTitle= FindChildByName(NAME_MSGBOX_TITLE);
		DUIASSERT(pTitle);
		pTitle->SetInnerText(lpCaption?lpCaption:_T("提示"));

		CDuiWindow * pMsg= FindChildByName(NAME_MSGBOX_TEXT);
		DUIASSERT(pMsg);
		pMsg->SetInnerText(lpText);

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

	BOOL CDuiMessageBox::SetIcon( UINT uType )
	{
		CDuiIconWnd *pIcon=(CDuiIconWnd *)FindChildByName(NAME_MSGBOX_ICON);
		if(!pIcon) return FALSE;
		switch(uType&0xF0)
		{
		case MB_ICONEXCLAMATION:
			pIcon->AttachIcon(LoadIcon(NULL,IDI_EXCLAMATION));
			break;
		case MB_ICONINFORMATION:
			pIcon->AttachIcon(LoadIcon(NULL,IDI_INFORMATION));
			break;
		case MB_ICONQUESTION:
			pIcon->AttachIcon(LoadIcon(NULL,IDI_QUESTION));
			break;
		case MB_ICONHAND:
			pIcon->AttachIcon(LoadIcon(NULL,IDI_HAND));
			break;
		}
		return TRUE;
	}

	int DuiMessageBox( HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType )
	{
		CDuiMessageBox duiMsgBox;
		return duiMsgBox.MessageBox(hWnd,lpText,lpCaption,uType);
	}



}//end of namespace 
