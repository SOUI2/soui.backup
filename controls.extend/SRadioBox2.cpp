#include "stdafx.h"
#include "SRadioBox2.h"

namespace SOUI
{
    SRadioBox2::SRadioBox2(void)
    {
    }

    SRadioBox2::~SRadioBox2(void)
    {
    }

    void SRadioBox2::OnPaint(IRenderTarget *pRT)
    {
        if(m_pSkin)
        {
            CRect rc;
            GetClientRect(&rc);
            int nState = 0;
            if(GetState() & WndState_Check) nState = 2;
            else if(GetState() & WndState_PushDown) nState = 2;
            else if(GetState() & WndState_Hover) nState = 1;
            
            m_pSkin->Draw(pRT,rc,nState);
        }
    }

}
