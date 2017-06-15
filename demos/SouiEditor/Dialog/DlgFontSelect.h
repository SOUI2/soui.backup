#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"
#include "DesignerView.h"

#ifdef _UNICODE  
#define tstring wstring  
#else  
#define tstring string  
#endif  


namespace SOUI
{
	class SDlgFontSelect: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgFontSelect,L"dlgfontselect")
	public:
		SDlgFontSelect(SStringT strFont, SDesignerView *pDesignerView);

		~SDlgFontSelect(void)
		{

		}

		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
		//virtual INT_PTR DoModal(HWND hParent=NULL);

		//virtual void EndDialog(INT_PTR nResult);


	protected:
		//void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnOK();
		void OnCancel();
		void OnCKClick();


		bool OnReNotify(EventArgs *pEvt);
		bool OnSizeReNotify(EventArgs *pEvt);

		bool OnLBSelChanged(EventArgs *pEvt);
		bool OnCBSelChanged(EventArgs *pEvt);

		//virtual SMessageLoop * GetMsgLoop(){return m_MsgLoop;}

		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"btnOK", OnOK)
			EVENT_NAME_COMMAND(L"btnCancel", OnCancel)
			EVENT_NAME_COMMAND(L"chkBold", OnCKClick)
			EVENT_NAME_COMMAND(L"chkItalic", OnCKClick)
			EVENT_NAME_COMMAND(L"chkUnderline", OnCKClick)
			EVENT_NAME_COMMAND(L"chkStrike", OnCKClick)
			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SDlgNewSkin)
			MSG_WM_INITDIALOG(OnInitDialog)
			//MSG_WM_CLOSE(OnCancel)
			//MSG_WM_KEYDOWN(OnKeyDown)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	public:
		
		 static int CALLBACK EnumFontFamProc(LPENUMLOGFONT lpelf,LPNEWTEXTMETRIC lpntm,DWORD nFontType,long lparam)
		{

			SMap<SStringT, SStringT> *map = (SMap<SStringT, SStringT>*) lparam;
			SStringT str = lpelf ->elfLogFont.lfFaceName;
			(*map)[str] = str;
			return TRUE;
		}

		static int GetLbIndexFromText(SListBox *lb, SStringT strText);


		void PreviewFont();

		void InitInfo(IFontPtr ft);
		SStringT GetFontStr();

	public:
		SListBox*     m_LbFont;
		SCheckBox*    m_chkBold;
		SCheckBox*    m_chkItalic;
		SCheckBox*    m_chkUnderline;
		SCheckBox*    m_chkStrike;

		SEdit*        m_edtSearch;
		SEdit*        m_edtSize;
		SWindow*      m_wndPreview;

		SStringT m_strFont;
		SMap<SStringT, SStringT> m_map;
		SDesignerView *m_pDesignerView;




	};

}