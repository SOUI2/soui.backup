/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SHostDialog.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    SOUIµÄDialogÄ£¿é
*/

#pragma once
#include "core/shostwnd.h"
#include "core/smsgloop.h"
#include "core/SHostDialog.h"
#include "control/SRichEdit.h"

namespace SOUI
{
	class SDlgCreatePro: public SHostDialog
	{
		SOUI_CLASS_NAME(SDlgCreatePro,L"dlgcreatepro")
	public:
		SDlgCreatePro(LPCTSTR pszXmlName):SHostDialog(pszXmlName)
		{

		}
		~SDlgCreatePro(void)
		{

		}

		//virtual INT_PTR DoModal(HWND hParent=NULL);

		//virtual void EndDialog(INT_PTR nResult);

	protected:
		//void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnOK();
		//void OnCancel();
		//virtual SMessageLoop * GetMsgLoop(){return m_MsgLoop;}

		EVENT_MAP_BEGIN()
			EVENT_ID_COMMAND(IDOK,OnOK)
			//EVENT_ID_COMMAND(IDCANCEL,OnCancel)
			EVENT_MAP_END()

			BEGIN_MSG_MAP_EX(SCreateProDlg)
			//MSG_WM_CLOSE(OnCancel)
			//MSG_WM_KEYDOWN(OnKeyDown)
			CHAIN_MSG_MAP(SHostDialog)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

	public:
		SStringT m_strinput;
	};

}