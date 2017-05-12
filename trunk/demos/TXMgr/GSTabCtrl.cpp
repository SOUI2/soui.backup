#include "stdafx.h"
#include "GSTabCtrl.h"
#include <algorithm>
#include "souistd.h"

GSTabCtrl::GSTabCtrl()
{
	m_pSkinAniHover = NULL;
	m_pSkinAniDown = NULL;
	m_nElapseTime = 0;
	m_nTabShowName = 0;
}


GSTabCtrl::~GSTabCtrl()
{
}


/**
* STabCtrl::OnPaint
* @brief    绘画消息
* @param    IRenderTarget *pRT -- 绘制设备句柄
*
* Describe  此函数是消息响应函数
*/
void GSTabCtrl::OnPaint(IRenderTarget *pRT)
{
	SPainter painter;
	BeforePaint(pRT, painter);

	CRect rcItem, rcItemPrev;
	CRect rcSplit;
	DWORD dwState;
	CRect rcTitle = GetTitleRect();


	pRT->PushClipRect(&rcTitle, RGN_AND);

	for (size_t i = 0; i<GetItemCount(); i++)
	{
		dwState = WndState_Normal;
		if (i == (size_t)m_nCurrentPage) dwState = WndState_PushDown;
		else if (i == (size_t)m_nHoverTabItem) dwState = WndState_Hover;

		GetItemRect(i, rcItem);
		if (rcItem.IsRectEmpty()) continue;

		//画分隔线
		if (i>0 && m_pSkinTabInter)
		{
			rcSplit = rcItem;
			if (m_nTabAlign == AlignLeft)
			{
				rcSplit.top = rcItemPrev.bottom;
				rcSplit.bottom = rcSplit.top + m_nTabInterSize;
			}
			else
			{
				rcSplit.left = rcItemPrev.right;
				rcSplit.right = rcSplit.left + m_nTabInterSize;
			}
			m_pSkinTabInter->Draw(pRT, rcSplit, 0);
		}

		DrawItem(pRT, rcItem, i, dwState);
		rcItemPrev = rcItem;

		if (m_pSkinAniHover && m_arrHoverNumber.GetCount())
		{
			m_pSkinAniHover->Draw(pRT, rcItem, m_arrHoverNumber.GetAt(i));
		}
	}
	pRT->PopClip();

	if (m_pSkinFrame)
	{
		CRect rcPage = GetChildrenLayoutRect();
		m_pSkinFrame->Draw(pRT, rcPage, WndState_Normal);
	}

	if (IsFocused() && IsFocusable() && m_bDrawFocusRect)
	{
		CRect rc;
		GetItemRect(m_nCurrentPage, rc);
		rc.DeflateRect(2, 2);
		DrawDefFocusRect(pRT, &rc);
	}
	AfterPaint(pRT, painter);
}



void GSTabCtrl::DrawItem(IRenderTarget *pRT, const CRect &rcItem, int iItem, DWORD dwState)
{
	if (rcItem.IsRectEmpty()) return;
	int iState = IIF_STATE3(dwState, WndState_Normal, WndState_Hover, WndState_PushDown);
	if (m_pSkinTab)
	{
		m_pSkinTab->Draw(pRT, rcItem, iState);
	}
	else
	{
// 		else if (m_pSkinAniDown && dwState == WndState_PushDown)
// 		{
// 			GetContainer()->RegisterTimelineHandler(this);
// 		}
	}
		

	//根据状态从style中获得字体，颜色
	SOUI::IFontPtr font = m_style.GetTextFont(iState);
	COLORREF crTxt = m_style.GetTextColor(iState);
	CAutoRefPtr<SOUI::IFont> oldFont;
	if (font) pRT->SelectObject(font, (IRenderObj**)&oldFont);
	COLORREF crOld = 0;
	if (crTxt != CR_INVALID) crOld = pRT->SetTextColor(crTxt);

	CRect rcIcon(m_ptIcon + rcItem.TopLeft(), CSize(0, 0));
	if (m_pSkinIcon)
	{
		rcIcon.right = rcIcon.left + m_pSkinIcon->GetSkinSize().cx;
		rcIcon.bottom = rcIcon.top + m_pSkinIcon->GetSkinSize().cy;
		int iIcon = GetItem(iItem)->GetIconIndex();
		if (iIcon == -1) iIcon = iItem;
		iIcon = GetItemCount() * iState + iIcon;
		m_pSkinIcon->Draw(pRT, rcIcon, iIcon);
	}

	if (m_nTabShowName)
	{
		if (m_ptText.x != -1 && m_ptText.y != -1)
		{//从指定位置开始绘制文字
			if (m_txtDir == Text_Horz)
				pRT->TextOut(rcItem.left + m_ptText.x, rcItem.top + m_ptText.y, GetItem(iItem)->GetTitle(), -1);
			else
				TextOutV(pRT, rcItem.left + m_ptText.x, rcItem.top + m_ptText.y, GetItem(iItem)->GetTitle());
		}
		else
		{
			CRect rcText = rcItem;
			UINT alignStyle = m_style.GetTextAlign();
			UINT align = alignStyle;
			if (m_ptText.x == -1 && m_ptText.y != -1)
			{//指定了Y偏移，X居中
				rcText.top += m_ptText.y;
				align = alignStyle&(DT_CENTER | DT_RIGHT | DT_SINGLELINE | DT_END_ELLIPSIS);
			}
			else if (m_ptText.x != -1 && m_ptText.y == -1)
			{//指定了X偏移，Y居中
				rcText.left += m_ptText.x;
				align = alignStyle&(DT_VCENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);
			}

			if (m_txtDir == Text_Horz)
				pRT->DrawText(GetItem(iItem)->GetTitle(), -1, &rcText, align);
			else
				DrawTextV(pRT, rcText, GetItem(iItem)->GetTitle());
		}
	}



	//恢复字体，颜色
	if (font) pRT->SelectObject(oldFont);
	if (crTxt != CR_INVALID) pRT->SetTextColor(crOld);
}


void GSTabCtrl::OnNextFrame()
{
	if (m_arrHoverNumber.GetCount() == 0)
	{
		for (int i = 0; i < GetItemCount(); i++)
			m_arrHoverNumber.Add(0);
	}

	m_nElapseTime++;
	if (m_nElapseTime < 5)
	{
		return;
	}

	m_nElapseTime = 0;


	bool bNeedInvalidate = false;
	for (int i = 0; i < m_arrHoverNumber.GetCount(); i++)
	{
		DWORD dwState = WndState_Normal;
		if (i == (size_t)m_nCurrentPage) dwState = WndState_PushDown;
		else if (i == (size_t)m_nHoverTabItem) dwState = WndState_Hover;

		if (i == (size_t)m_nHoverTabItem)
		{
			if (m_arrHoverNumber.GetAt(i) < m_pSkinAniHover->GetStates() - 1)
			{
				m_arrHoverNumber.GetAt(i)++;
				bNeedInvalidate = true;
			}
		}
		else
		{
			if (m_arrHoverNumber.GetAt(i) > 0)
			{
				m_arrHoverNumber.GetAt(i)--;
				bNeedInvalidate = true;
			}
		}
	}

	if (bNeedInvalidate == true)
		Invalidate();
	else
		GetContainer()->UnregisterTimelineHandler(this);
}

void GSTabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rcItem;
	int nOldHover = m_nHoverTabItem;
	m_nHoverTabItem = -1;
	int nTabCount = GetItemCount();
	for (int i = 0; i < nTabCount; i++)
	{
		GetItemRect(i, rcItem);

		if (rcItem.PtInRect(point))
		{
			m_nHoverTabItem = i;
			break;
		}
	}
	if (m_nHoverTabItem != nOldHover)
	{
 		if (nOldHover != -1)
 		{
			if (nOldHover != m_nCurrentPage)
			{
				GetItemRect(nOldHover, rcItem);
				InvalidateRect(rcItem);
			}
			EventTabItemLeave evt(this);
			evt.iLeave = nOldHover;
			FireEvent(evt);
		}
		if (m_nHoverTabItem != -1)
		{
			if (m_nHoverTabItem != m_nCurrentPage)
			{
				GetItemRect(m_nHoverTabItem, rcItem);
				InvalidateRect(rcItem);
			}

			EventTabItemHover evt2(this);
			evt2.iHover = m_nHoverTabItem;
			FireEvent(evt2);
		}

		if (m_pSkinAniHover)
		{
			GetContainer()->RegisterTimelineHandler(this);
		}
	}
}



