#include "stdafx.h"
#include "STabCtrlHeaderBinder.h"

STabCtrlHeaderBinder::STabCtrlHeaderBinder(STabCtrl * _host) :m_pTabHost(_host)
{
	
}

STabCtrlHeaderBinder::~STabCtrlHeaderBinder()
{
}

void STabCtrlHeaderBinder::Bind(SWindow * _pWnd, int _idx, DWORD _evtcode)
{
	if (_pWnd->GetEventSet()->subscribeEvent(_evtcode, Subscriber(&STabCtrlHeaderBinder::EvtHander, this)))
		m_lstPages[_pWnd] = _idx;
}

bool STabCtrlHeaderBinder::EvtHander(EventArgs *e)
{
	if (SMap<SWindow*,int>::CPair *pair=m_lstPages.Lookup((SWindow*)e->sender))
	{
		m_pTabHost->SetCurSel(pair->m_value);
	}	
	return false;
}