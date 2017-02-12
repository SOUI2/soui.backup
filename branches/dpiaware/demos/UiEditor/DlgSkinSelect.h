#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"
#include "control/SRichEdit.h"
#include "SImageEx.h"

namespace SOUI
{
	class SDlgSkinSelect: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgSkinSelect,L"dlgskinselect")
	public:
		SDlgSkinSelect(LPCTSTR pszXmlName, SStringT strSkinName, SStringT strPath, BOOL bGetSkin = TRUE);

		~SDlgSkinSelect(void)
		{

		}


		void OnClose();
		void OnMaximize();
		void OnRestore();
		void OnMinimize();
		void OnZYLXNew();
		void OnZYLXDel();
		void OnZYNew();		
		void OnZYDel();
		void OnSkinNew();
		void OnSkinDel();

		void Save();

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
		//virtual INT_PTR DoModal(HWND hParent=NULL);

		//virtual void EndDialog(INT_PTR nResult);


		bool OnLbResTypeSelChanged(EventArgs *pEvtBase);
		bool OnLbResSelChanged(EventArgs *pEvtBase);
        bool OnLbSkinSelChanged(EventArgs *pEvtBase);

		bool OnPropGridValueChanged( EventArgs *pEvt );

	protected:
		//void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnOK();
		//void OnCancel();
		//virtual SMessageLoop * GetMsgLoop(){return m_MsgLoop;}

		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_close", OnClose)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_restore", OnRestore)

			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_ZYLX_new", OnZYLXNew)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_ZYLX_DEL", OnZYLXDel)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_ZY_NEW", OnZYNew)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_ZY_DEL", OnZYDel)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_SKIN_new", OnSkinNew)
			EVENT_NAME_COMMAND(L"NAME_UIDESIGNER_btn_SKIN_DEL", OnSkinDel)

			EVENT_NAME_COMMAND(L"btnOK", OnOK)

			//EVENT_ID_COMMAND(IDOK,OnOK)
			//EVENT_ID_COMMAND(IDCANCEL,OnCancel)
			EVENT_MAP_END()

		BEGIN_MSG_MAP_EX(SDlgSkinSelect)
			MSG_WM_INITDIALOG(OnInitDialog)
			//MSG_WM_CLOSE(OnCancel)
			//MSG_WM_KEYDOWN(OnKeyDown)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	protected:
		 void InitResType();
		 void LoadSkinFile();
		 void LoadSysSkin();
		 void LoadSkinProperty();
		 void LoadUIRes();
         void UpdatePropGrid();

		 void DestroyGrid();
         SStringT GetLBCurSelText(SListBox * lb);
		 int GetLbIndexFromText(SListBox *lb, SStringT strText);

		 void SelectLBItem(SListBox * lb, int nIndex);
         bool OnReNotify(EventArgs *pEvt);
		 void GoToSkin();
		 void ShowImage();
		 void ShowSysImage();


	public:
		SStringT m_strinput;
		SStringT m_strSkinName;  //皮肤名


		SStringT m_strUIResFile;   //uires.idx完整文件名

		SStringT m_strSkinFile;   //skin完整文件名

		pugi::xml_node m_xmlNodeUiRes;
	    SListBox *m_lbResType;  //资源类型
		SListBox *m_lbRes;  //资源
		SListBox *m_lbSkin;  //皮肤
		SPropertyGrid *m_pgGrid;  //皮肤属性

		//SButton *m_btnZYLXNew;
		//SButton *m_btnZYLXDel;

		//SButton *m_btnZYNew;
		//SButton *m_btnZYDel;

		//SButton *m_btnSkinNew;
		//SButton *m_btnSkinDel;

		SWindow *m_wndGridContainer;
		SStringT m_strProPath;

		SEdit *m_pEdit;

		pugi::xml_document m_xmlDocSkin;

		pugi::xml_document m_xmlDocSysSkin;
		pugi::xml_document m_xmlDocSkinProperty;
        pugi::xml_document m_xmlDocUiRes;

		SMap<SStringT, SStringT> m_mapSysSkin; 

		SImageWnd *m_imgView;
		BOOL m_bGetSkin; //

		pugi::xml_node m_xmlNodeCurSkin;
	};

}