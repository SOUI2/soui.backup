#include "StdAfx.h"
#include "UIHelper.h"

CDuiStringT CUIHelper::GetEditText( CDuiRichEdit *pEdit )
{
	CDuiStringT str;
	int nLen=pEdit->GetWindowTextLength()+1;
	LPTSTR pBuf=str.GetBufferSetLength(nLen);
	pEdit->GetWindowText(pBuf,nLen);
	str.ReleaseBuffer();
	return str;
}

CDuiStringT CUIHelper::GetComboboxText( CDuiComboBox *pCtrl )
{
	CDuiStringT str;
	int nLen=pCtrl->GetWindowTextLength()+1;
	LPTSTR pBuf=str.GetBufferSetLength(nLen);
	pCtrl->GetWindowText(pBuf,nLen);
	str.ReleaseBuffer();
	return str;
}