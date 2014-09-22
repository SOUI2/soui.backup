#include "souistd.h"
#include "control/SHotKeyCtrl.h"
#include <CommCtrl.h>

namespace SOUI
{

#define EDIT_INSET 2

SHotKeyCtrl::SHotKeyCtrl(void)
{
    m_wInvalidModifier=0;
    m_wInvalidComb=HKCOMB_NONE;
    m_bInSetting=FALSE;
    m_bFocusable=TRUE;
}

SHotKeyCtrl::~SHotKeyCtrl(void)
{
}

int SHotKeyCtrl::OnCreate( LPVOID )
{
    int nRet=__super::OnCreate(NULL);
    if(nRet!=0) return nRet;
    
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
    BeforePaintEx(pRT);
    m_curFont=(IFont*)pRT->GetCurrentObject(OT_FONT);
    return 0;
}

void SHotKeyCtrl::OnLButtonDown( UINT nFlags,CPoint pt )
{
    __super::OnLButtonDown(nFlags,pt);
    GetContainer()->OnSetSwndFocus(m_swnd);
}

void SHotKeyCtrl::OnPaint( IRenderTarget * pRT )
{
    SPainter painter;
    BeforePaint(pRT,painter);
    CRect rcClient;
    GetClientRect(&rcClient);
    rcClient.OffsetRect(EDIT_INSET,0);
    SStringT str=FormatHotkey();
    pRT->DrawText(str,str.GetLength(),&rcClient,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    AfterPaint(pRT,painter);
}

void SHotKeyCtrl::UpdateCaret()
{
    SStringT str=FormatHotkey();
    IRenderTarget *pRT=GetRenderTarget(NULL,OLEDC_NODRAW);
    CAutoRefPtr<IFont> oldFont;
    pRT->SelectObject(m_curFont,(IRenderObj**)&oldFont);
    SIZE szTxt;
    pRT->MeasureText(str,str.GetLength(),&szTxt);
    pRT->SelectObject(oldFont);
    ReleaseRenderTarget(pRT);
    
    CRect rcClient;
    GetClientRect(&rcClient);
    GetContainer()->SwndSetCaretPos(rcClient.left+EDIT_INSET+szTxt.cx,rcClient.top+(rcClient.Height()-szTxt.cy)/2);
}

void SHotKeyCtrl::OnSetFocus()
{
    IRenderTarget *pRT=GetRenderTarget(NULL,OLEDC_NODRAW);
    CAutoRefPtr<IFont> oldFont;
    pRT->SelectObject(m_curFont,(IRenderObj**)&oldFont);
    SIZE szTxt;
    pRT->MeasureText(_T("A"),1,&szTxt);
    pRT->SelectObject(oldFont);
    ReleaseRenderTarget(pRT);
    GetContainer()->SwndCreateCaret(NULL,1,szTxt.cy);

    CRect rcClient;
    GetClientRect(&rcClient);
    OnSetCaretValidateRect(&rcClient);

    UpdateCaret();
    GetContainer()->SwndShowCaret(TRUE);
}

void SHotKeyCtrl::OnKillFocus()
{
    GetContainer()->SwndShowCaret(FALSE);

}

void SHotKeyCtrl::UpdateModifier()
{
    BOOL bAlt=GetKeyState(VK_MENU)&0x8000;
    BOOL bCtrl=GetKeyState(VK_CONTROL)&0x8000;
    BOOL bShift=GetKeyState(VK_SHIFT)&0x8000;

    WORD wCombKey=0;
    if(!bAlt && !bCtrl && !bShift) wCombKey=HKCOMB_NONE,m_wModifier=0;
    else if(bAlt && !bCtrl && !bShift) wCombKey=HKCOMB_A,m_wModifier=HOTKEYF_ALT;
    else if(!bAlt && bCtrl && !bShift) wCombKey=HKCOMB_C,m_wModifier=HOTKEYF_CONTROL;
    else if(!bAlt && !bCtrl && bShift) wCombKey=HKCOMB_S,m_wModifier=HOTKEYF_SHIFT;
    else if(bAlt && bCtrl && !bShift) wCombKey=HKCOMB_CA,m_wModifier=HOTKEYF_ALT|HOTKEYF_CONTROL;
    else if(bAlt && !bCtrl && bShift) wCombKey=HKCOMB_SA,m_wModifier=HOTKEYF_SHIFT|HOTKEYF_ALT;
    else if(!bAlt && bCtrl && bShift) wCombKey=HKCOMB_SC,m_wModifier=HOTKEYF_SHIFT|HOTKEYF_CONTROL;
    else wCombKey=HKCOMB_SCA,m_wModifier=HOTKEYF_ALT|HOTKEYF_SHIFT|HOTKEYF_CONTROL;
    if(wCombKey&m_wInvalidComb)
        m_wModifier=m_wInvalidModifier;
}

void SHotKeyCtrl::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if(!m_bInSetting)
    {
        m_bInSetting=TRUE;
        m_wVK=0;
        m_wModifier=m_wInvalidModifier;
    }
    SStringT strKey=GetKeyName(nChar);
    if(!strKey.IsEmpty())
    {
        //
        m_wVK=nChar;
    }
    UpdateModifier();
    UpdateCaret();
    Invalidate();
}

void SHotKeyCtrl::OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if(!m_bInSetting) return;

    if(nChar == m_wVK)
    {
        m_bInSetting=FALSE;
    }
    else if(m_wVK==0 && (GetKeyState(VK_SHIFT)&0x8000)==0 && (GetKeyState(VK_MENU)&0x8000)==0 && (GetKeyState(VK_CONTROL)&0x8000)==0)
    {
        m_bInSetting=FALSE;
        UpdateModifier();
        UpdateCaret();
        Invalidate();
    }
    else if(nChar==VK_SHIFT || nChar==VK_MENU || nChar== VK_CONTROL)
    {
        UpdateModifier();
        UpdateCaret();
        Invalidate();
    }
}

void SHotKeyCtrl::OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    if(GetKeyState(VK_MENU)&0x8000) OnKeyDown(nChar,nRepCnt,nFlags);
    else SetMsgHandled(FALSE);
}

void SHotKeyCtrl::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(nChar==VK_MENU || GetKeyState(VK_MENU)&0x8000) OnKeyUp(nChar,nRepCnt,nFlags);
    else SetMsgHandled(FALSE);
}

void SHotKeyCtrl::SetRule( WORD wInvalidComp,WORD wModifier )
{
    m_wInvalidComb=wInvalidComp;
    m_wInvalidModifier=wModifier;
}

void SHotKeyCtrl::SetHotKey( WORD vKey,WORD wModifiers )
{
    m_wVK=vKey;
    m_wModifier=wModifiers;
    UpdateModifier();
    UpdateCaret();
    Invalidate();
}

void SHotKeyCtrl::GetHotKey( WORD & vKey,WORD &wModifers )
{
    vKey=m_wVK;
    wModifers=m_wModifier;
}

LRESULT SHotKeyCtrl::OnWindowPosChanged( LPRECT lpRcContainer )
{
    KillFocus();
    return __super::OnWindowPosChanged(lpRcContainer);
}
}//namespace SOUI
