// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

struct SkinInfo
{
	CDuiStringT strType;
	CDuiStringT strName;
	CDuiStringT strSrc;

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

class CMainDlg : public CDuiHostWnd
{
	friend class CNewSkinDlg;
	friend class CNewFileDlg;
public:
	CMainDlg();
	~CMainDlg();


	CDuiStringT GetImageSrcFile(const CDuiStringT & strSrcName);

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
		if(!FindChildByName("btn_sys_close")) return;
		if(nType==SIZE_MAXIMIZED)
		{
			FindChildByName("btn_sys_restore")->SetVisible(TRUE);
			FindChildByName("btn_sys_maximize")->SetVisible(FALSE);
		}else if(nType==SIZE_RESTORED)
		{
			FindChildByName("btn_sys_restore")->SetVisible(FALSE);
			FindChildByName("btn_sys_maximize")->SetVisible(TRUE);
		}
	}

	void OnDestroy();

	BOOL OpenProject(LPCTSTR pszInitXml);

	BOOL InitIndexList(LPCTSTR pszIndexFile);

	void InitSkinList();

	void OnBtnClick_ProjectOpen();
	void OnBtnClick_Project_File_Add();
	void OnBtnClick_Project_Skin_Add();

	LRESULT OnListCtrl_File_SelChanged(LPDUINMHDR pnmh);
	LRESULT OnListCtrl_Skin_SelChanged(LPDUINMHDR pnmh);

protected:
	virtual void EndDialog(UINT uRetCode)
	{
		AnimateHostWindow(200,AW_CENTER|AW_HIDE);
		__super::EndDialog(uRetCode);
	}
	virtual HWND OnRealWndCreate(CDuiRealWnd *pRealWnd);
	virtual void OnRealWndDestroy(CDuiRealWnd *pRealWnd);

	DUI_NOTIFY_MAP_BEGIN()
		DUI_NOTIFY_NAME_COMMAND("btn_sys_maximize", OnMaximize)
		DUI_NOTIFY_NAME_COMMAND("btn_sys_restore", OnRestore)
		DUI_NOTIFY_NAME_COMMAND("btn_sys_minimize", OnMinimize)
		DUI_NOTIFY_NAME_HANDLER("prj_list_file",DUINM_LBSELCHANGED,OnListCtrl_File_SelChanged);
		DUI_NOTIFY_NAME_HANDLER("prj_list_skin",DUINM_LBSELCHANGED,OnListCtrl_Skin_SelChanged);
		DUI_NOTIFY_NAME_COMMAND("prg_btn_open", OnBtnClick_ProjectOpen)
		DUI_NOTIFY_NAME_COMMAND("prg_btn_add_file", OnBtnClick_Project_File_Add)
		DUI_NOTIFY_NAME_COMMAND("prg_btn_add_skin", OnBtnClick_Project_Skin_Add)
	DUI_NOTIFY_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_SIZE(OnSize)
		MSG_WM_DESTROY(OnDestroy)
		MSG_DUI_NOTIFY()
		CHAIN_MSG_MAP(CDuiHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

private:
	void ClearSkinList();

	CDuiStringT	m_strPrjPath;	//皮肤路径
	CDuiStringT m_strPrjIndex;	//皮肤index.xml
	CDuiStringT m_strInitFile;	//皮肤的初始化文件名
};
