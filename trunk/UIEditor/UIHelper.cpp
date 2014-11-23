#include "StdAfx.h"
#include "UIHelper.h"

SStringT CUIHelper::GetEditText( SRichEdit *pEdit )
{
    return pEdit->GetWindowText(); 
}

SStringT CUIHelper::GetComboboxText( SComboBox *pCtrl )
{
    return pCtrl->GetWindowText(); 
}