#include "stdafx.h"
#include "SPathBar.h"

namespace SOUI
{

SPathBar::SPathBar(void)
	: m_nHoverItem(-1)
{
	m_pSkin = GETBUILTINSKIN(SKIN_SYS_BTN_NORMAL);
	m_evtSet.addEvent(EVENTID(EventPathCmd));
	
}


SPathBar::~SPathBar(void)
{
}

void SPathBar::OnPaint(IRenderTarget *pRT)
{
	SPainter painter;
	BeforePaint(pRT,painter);

	CRect rcClient;
	GetClientRect(&rcClient);
	
	pRT->PushClipRect(&rcClient, RGN_AND);

	//pRT->FillSolidRect(&rcClient, StringToColor(L"#FF3254"));
	//m_style.m_crBg = 0xFF3254;
	CRect inRect = rcClient;
	inRect.left += 1;
	inRect.right = inRect.left;
	inRect.top = rcClient.top;
	inRect.bottom = rcClient.bottom;
	
	
	int nCount = m_arrItems.GetCount();
	for(int i=0; i<nCount; i++)
	{
		BarItemInfo& info = m_arrItems.GetAt(i);
				
		inRect.left = inRect.right;
		inRect.right = inRect.left + info.nWidth;

		COLORREF crItemBg = 0x0;//
		if(i == m_nHoverItem)
		{					
			m_pSkin->Draw(pRT, inRect, IIF_STATE4(GetState(), 0, 1, 2, 3));
		}
		else if (CR_INVALID != crItemBg)//先画背景
			pRT->FillSolidRect(&inRect, crItemBg);
		
		SStringT sText = info.sText + _T(" >");
		pRT->DrawText(sText, sText.GetLength(), inRect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		
	}

	pRT->PopClip();

	AfterPaint(pRT,painter);
}

void SPathBar::OnLButtonDown(UINT nFlags, CPoint pt)
{
	m_nHoverItem = HitTest(pt);
	__super::OnLButtonDown(nFlags, pt);
}

void SPathBar::OnLButtonUp(UINT nFlags, CPoint pt)
{
	__super::ReleaseCapture();

	if(!(GetState()&WndState_PushDown)) return;

	ModifyState(0, WndState_PushDown, TRUE);

	int nHoverItem = HitTest(pt);
	if(nHoverItem < 0 || nHoverItem != m_nHoverItem)
	{
		return ;
	}

	//删除 后面的 
	//m_arrItems.RemoveAt(nHoverItem+1, m_arrItems.GetCount()-nHoverItem-1);
	
	//分发 事件 
	EventPathCmd evt(this);
	evt.iItem = nHoverItem;
	FireEvent(evt);
}

void SPathBar::OnMouseMove(UINT nFlags,CPoint pt)
{
	 if(GetCapture() == m_swnd)
	 {
		 return ;
	 }

	int nHoverItem = HitTest(pt);
	if(nHoverItem != m_nHoverItem)
	{
		m_nHoverItem = nHoverItem;
		Invalidate();
	}
	__super::OnMouseMove(nFlags, pt);
}

void SPathBar::OnMouseLeave()
{
	m_nHoverItem = -1;
	Invalidate();
	__super::OnMouseLeave();
}

int SPathBar::HitTest(CPoint pt)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int nOffX = pt.x - rcClient.left;
	int nCount = m_arrItems.GetCount();
	for(int i=0; i<nCount; i++)
	{
		BarItemInfo& info = m_arrItems.GetAt(i);
		if(info.nWidth > nOffX)
		{
			return i;
		}
		nOffX -= info.nWidth;
	}
	return -1;
}

int SPathBar::InsertItem(int nItem, LPCTSTR pszText)
{
	//if( m_listItems)
	BarItemInfo info;
	info.sText = pszText;

	CAutoRefPtr<IRenderTarget> pRT;
	GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
	BeforePaintEx(pRT);
	CRect rcText;
	DrawText(pRT, info.sText, info.sText.GetLength(), rcText, DT_VCENTER | DT_CALCRECT);
	
	rcText.InflateRect(m_style.GetPadding());

	//pRT->MeasureText(p1, p2-p1, &szChar);

	info.nWidth = rcText.Width() + 24;		//添加 
	m_arrItems.Add(info);

	Invalidate();
	return 0;
}

void SPathBar::DeleteItem(int nItem, int nCount)
{
	if(0 == nCount)
		return ;

	if(nItem >= 0 && nItem < (int)m_arrItems.GetCount())
	{
		if(nCount < 0)
			nCount = m_arrItems.GetCount() - nItem;

		m_arrItems.RemoveAt(nItem, nCount);
		Invalidate();
	}
}

void SPathBar::DeleteAllItems()
{	
	if(m_arrItems.GetCount() > 0)
	{
		m_arrItems.RemoveAll();
		Invalidate();
	}
}

SStringT SPathBar::GetItemText(int nItem) const
{
	if(nItem >=0 && nItem < (int)m_arrItems.GetCount())
	{
		return m_arrItems.GetAt(nItem).sText;
	}

	return _T("");
}

BOOL SPathBar::SetItemData(int nItem, DWORD dwData)
{
	if(nItem < 0 || nItem >= (int)m_arrItems.GetCount())
	{
		return FALSE;
	}
	m_arrItems.GetAt(nItem).dwData = dwData;
	return TRUE;
}

DWORD SPathBar::GetItemData(int nItem)
{
	if(nItem >= 0 && nItem < (int)m_arrItems.GetCount())
	{
		return m_arrItems.GetAt(nItem).dwData;
	}

	return 0;
}

DWORD SPathBar::GetItemCount() const
{
	return m_arrItems.GetCount();
}




}	//end namespace