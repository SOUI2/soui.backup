// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "wtlhelper/whwindow.h"

struct SkinInfo
{
	SStringT strType;
	SStringT strName;
	SStringT strSrc;

	union{
		struct{
		int			nState;
		BOOL		bTile;
		BOOL		bVertical;
		CRect		rcMargin;
		};
		struct{
			COLORREF	crUp[4];
			COLORREF	crDown[4];
			COLORREF	crBorder;
		};
		struct{
			COLORREF cr1;
			COLORREF cr2;
			BOOL dir;
		};
	};
};

class CMainDlg : public SHostDialog
               , public CWHRoundRectFrameHelper<CMainDlg>
{
	friend class CNewSkinDlg;
	friend class CNewFileDlg;
public:
	CMainDlg();
	~CMainDlg();


	SStringT GetImageSrcFile(const SStringT & strSrcName);

	void OnMaximize()
	{
		SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
	}
	void OnRestore()
	{
		SendMessage(WM_SYSCOMMAND,SC_RESTORE);
	}
	void OnMinimize()
	{
		SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
	}

	void OnSize(UINT nType, CSize size)
	{
		SetMsgHandled(FALSE);
		if(!FindChildByName(L"btn_sys_restore")) return;
		if(nType==SIZE_MAXIMIZED)
		{
			FindChildByName(L"btn_sys_restore")->SetVisible(TRUE);
			FindChildByName(L"btn_sys_maximize")->SetVisible(FALSE);
		}else if(nType==SIZE_RESTORED)
		{
			FindChildByName(L"btn_sys_restore")->SetVisible(FALSE);
			FindChildByName(L"btn_sys_maximize")->SetVisible(TRUE);
		}
	}

	void OnDestroy();

	BOOL OpenProject(LPCTSTR pszInitXml);

	BOOL InitIndexList(LPCTSTR pszIndexFile);

	void InitSkinList();

	void OnBtnClick_ProjectOpen();
	void OnBtnClick_Project_File_Add();
	void OnBtnClick_Project_Skin_Add();

    BOOL OnListCtrl_File_SelChanged(EventArgs *pEvt);
    BOOL OnListCtrl_Skin_SelChanged(EventArgs *pEvt);

protected:
	virtual void EndDialog(UINT uRetCode)
	{
		AnimateHostWindow(200,AW_CENTER|AW_HIDE);
		__super::EndDialog(uRetCode);
	}

	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_sys_maximize", OnMaximize)
		EVENT_NAME_COMMAND(L"btn_sys_restore", OnRestore)
		EVENT_NAME_COMMAND(L"btn_sys_minimize", OnMinimize)
		EVENT_NAME_HANDLER(L"prj_list_file",EventLCSelChanged::EventID,OnListCtrl_File_SelChanged);
		EVENT_NAME_HANDLER(L"prj_list_skin",EventLCSelChanged::EventID,OnListCtrl_Skin_SelChanged);
		EVENT_NAME_COMMAND(L"prg_btn_open", OnBtnClick_ProjectOpen)
		EVENT_NAME_COMMAND(L"prg_btn_add_file", OnBtnClick_Project_File_Add)
		EVENT_NAME_COMMAND(L"prg_btn_add_skin", OnBtnClick_Project_Skin_Add)
	EVENT_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_SIZE(OnSize)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(CWHRoundRectFrameHelper<CMainDlg>)
		CHAIN_MSG_MAP(SHostDialog)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	void ClearSkinList();

	SStringT	m_strPrjPath;	//皮肤路径
	SStringT m_strPrjIndex;	//皮肤index.xml
	SStringT m_strInitFile;	//皮肤的初始化文件名
};
