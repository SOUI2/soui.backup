#include "StdAfx.h"
#include "SColorPicker.h"

namespace SOUI
{

SColorPicker::SColorPicker(void):m_crDef(0),m_crCur(0)
{
    GetEventSet()->addEvent(EventColorChange::EventID);
}

SColorPicker::~SColorPicker(void)
{
}

void SColorPicker::OnPaint( IRenderTarget *pRT)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	pRT->FillSolidRect(&rcClient,m_crCur);
}

void SColorPicker::OnLButtonUp( UINT nFlags,CPoint pt )
{
	__super::OnLButtonUp(nFlags,pt);
	CColourPopup *pCrPopup = new CColourPopup(GetContainer()->GetHostHwnd(),this);
	CRect rcWnd;
	GetWindowRect(rcWnd);
	pt.x=rcWnd.left,pt.y=rcWnd.bottom;
	::ClientToScreen(GetContainer()->GetHostHwnd(),&pt);
	pCrPopup->SetDefColor(m_crDef);
	pCrPopup->Create(pt,m_crCur,_T("Ä¬ÈÏ"),_T("¸ü¶à"));
}

void SColorPicker::OnColorChanged( COLORREF cr )
{
	m_crCur=cr;
	Invalidate();
}

void SColorPicker::OnColorEnd( BOOL bCancel,COLORREF cr )
{
	if(bCancel) m_crCur=m_crDef;
	else m_crCur=cr;
	ModifyState(0,WndState_PushDown,TRUE);
    EventColorChange evt(this,m_crCur);
    FireEvent(evt);
}

}//end of namespace