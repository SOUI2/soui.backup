// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"

CMainDlg::CMainDlg() : SHostDialog(_T("XML:MAIN_DIALOG"))
{
} 

CMainDlg::~CMainDlg()
{
}

void CMainDlg::OnBtnAnimate()
{
    SWindow * pImg=FindChildByName(L"img_tst");
    if(pImg)
    {
        DWORD dwFlag=0;
        SWindow * pRadio=FindChildByName(L"grp_animode")->GetSelectedChildInGroup();
        if(wcscmp(pRadio->GetName(),L"ani_slide")==0)
            dwFlag|=AW_SLIDE;
        if(wcscmp(pRadio->GetName(),L"ani_center")==0)
            dwFlag|=AW_CENTER;
        if(wcscmp(pRadio->GetName(),L"ani_blend")==0)
            dwFlag|=AW_BLEND;
        if(dwFlag & AW_SLIDE)
        {
            SWindow * pRadioHor=FindChildByName(L"grp_hor_mode")->GetSelectedChildInGroup();
            if(pRadioHor)
            {
                if(wcscmp(pRadioHor->GetName(),L"slide_hor_nagetive")==0) dwFlag|=AW_HOR_NEGATIVE;
                else dwFlag |= AW_HOR_POSITIVE;
            }
            SWindow * pRadioVer=FindChildByName(L"grp_ver_mode")->GetSelectedChildInGroup();
            if(pRadioVer)
            {
                if(wcscmp(pRadioVer->GetName(),L"slide_ver_nagetive")==0) dwFlag|=AW_VER_NEGATIVE;
                else dwFlag |= AW_VER_POSITIVE;
            }
        }

        if(pImg->IsVisible(TRUE)) dwFlag |= AW_HIDE;

        pImg->AnimateWindow(200,dwFlag);
    }
}
