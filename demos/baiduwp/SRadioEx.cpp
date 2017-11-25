#include "stdafx.h"
#include "SRadioEx.h"
namespace SOUI
{

SRadioEx::SRadioEx()
{
}


SRadioEx::~SRadioEx()
{
}

void SRadioEx::OnStateChanged(DWORD dwOldState, DWORD dwNewState)
{
    __super::OnStateChanged(dwOldState, dwNewState);
    if(m_dwState == WndState_Normal || (m_dwState & (WndState_Hover | WndState_PushDown | WndState_Check)))
    {
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            pChild->ModifyState(m_dwState & (WndState_Hover | WndState_PushDown | WndState_Check),
                                pChild->GetState() & (WndState_Hover | WndState_PushDown | WndState_Check), TRUE);
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
    }
}

}