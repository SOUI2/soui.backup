#include "StdAfx.h"
#include "DuiColorPicker.h"

namespace SOUI
{

CDuiColorPicker::CDuiColorPicker(void):m_crDef(0),m_crCur(0)
{
}

CDuiColorPicker::~CDuiColorPicker(void)
{
}

void CDuiColorPicker::OnPaint( CDCHandle dc )
{
	CRect rcClient;
	GetClient(&rcClient);
	ALPHAINFO ai;
	CGdiAlpha::AlphaBackup(dc,rcClient,ai);
	dc.FillSolidRect(&rcClient,m_crCur);
	dc.DrawEdge(&rcClient, BDR_RAISEDINNER, BF_RECT);
	CGdiAlpha::AlphaRestore(dc,ai);
}

void CDuiColorPicker::OnLButtonUp( UINT nFlags,CPoint pt )
{
	__super::OnLButtonUp(nFlags,pt);
	CColourPopup *pCrPopup = new CColourPopup(GetContainer()->GetHostHwnd(),this);
	CRect rcWnd;
	GetRect(rcWnd);
	pt.x=rcWnd.left,pt.y=rcWnd.bottom;
	::ClientToScreen(GetContainer()->GetHostHwnd(),&pt);
	pCrPopup->SetDefColor(m_crDef);
	pCrPopup->Create(pt,m_crCur,_T("Ä¬ÈÏ"),_T("¸ü¶à"));
}

void CDuiColorPicker::OnColorChanged( COLORREF cr )
{
	m_crCur=cr;
	NotifyInvalidate();

}

void CDuiColorPicker::OnColorEnd( BOOL bCancel,COLORREF cr )
{
	if(bCancel) m_crCur=m_crDef;
	else m_crCur=cr;
	ModifyState(0,DuiWndState_PushDown,TRUE);
	DUINMCOLORCHANGE nm;
	nm.hdr.code=DUINM_COLORCHANGE;
	nm.hdr.hDuiWnd=m_hDuiWnd;
	nm.hdr.idFrom=m_uCmdID;
	nm.hdr.pszNameFrom=GetName();
	nm.crSel=m_crCur;

	DuiNotify((LPDUINMHDR)&nm);
}

}//end of namespace