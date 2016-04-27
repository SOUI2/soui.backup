#include "stdafx.h"
#include "SetSkinWnd.h"

CSetSkinWnd::CSetSkinWnd(ISetSkinHandler * pSetSkinHandler)
:SHostWnd(UIRES.LAYOUT.dlg_set_skin)
,m_pSetSkinHandler(pSetSkinHandler)
{
}

CSetSkinWnd::~CSetSkinWnd(void)
{
}

void CSetSkinWnd::OnSetSkin(EventArgs *e)
{
    SASSERT(m_pSetSkinHandler);
    m_pSetSkinHandler->OnSetSkin(e->idFrom-10);
    DestroyWindow();
}

void CSetSkinWnd::OnActivate(UINT nState, BOOL bMinimized, HWND wndOther)
{
    if(nState==WA_INACTIVE)
        DestroyWindow();
    else        
        SHostWnd::OnActivate(nState,bMinimized,wndOther);
}
