#include "stdafx.h"
#include "SPropertyGridEx.h"

SPropertyGridEx::SPropertyGridEx(void)
	:m_crGroup(CR_INVALID)
	,m_crItem(CR_INVALID)
	,m_crItemText(CR_INVALID)
	,m_crItemSel(CR_INVALID)
	,m_strEditBkgndColor(_T("#FFFFFF00"))
	,m_strEditTextColor(_T("#FFFFFF"))
	,m_crBorder(CR_INVALID)
	,m_strEnableAutoWordSel(_T("1"))

{

}
SPropertyGridEx::~SPropertyGridEx(void)
{

}


void SPropertyGridEx::DrawItem( IRenderTarget *pRT, CRect &rc, int iItem )
{
	IPropertyItem *pItem = (IPropertyItem*)GetItemData(iItem);

	CRect rcSwitch = rc;
	CRect rcNameBack = rc;
	rcSwitch.right = rcSwitch.left +rcSwitch.Height();
	rcNameBack.left = rcSwitch.right;
	rcNameBack.right = rcNameBack.left + m_nNameWidth;
	pRT->FillSolidRect(rcSwitch,m_crGroup);
	pRT->FillSolidRect(rcNameBack,iItem == SListBox::GetCurSel()? m_crItemSel:(pItem->IsGroup()?m_crGroup:m_crItem));
	int iLevel = pItem->GetLevel();
	if(iLevel>1) rcSwitch.OffsetRect(rcSwitch.Width()*(iLevel-1),0);
	if(pItem->ChildrenCount() && m_switchSkin)
	{
		int iState = pItem->IsExpand()?GROUP_EXPANDED:GROUP_COLLAPSED;
		if(!pItem->IsGroup()) iState += 2;
		CRect rcDraw = rcSwitch;
		rcDraw.DeflateRect((rcSwitch.Size()-m_switchSkin->GetSkinSize())/2);
		m_switchSkin->Draw(pRT,rcDraw,iState);
	}

	CRect rcName = rcNameBack;
	rcName.left = rcSwitch.right;

	SStringT strName = S_CW2T(pItem->GetName2());
	pRT->DrawText(strName,strName.GetLength(),rcName,DT_SINGLELINE|DT_VCENTER);
	CRect rcItem = rc;
	rcItem.left= rcNameBack.right;
	if(pItem->HasButton()) rcItem.right -= rcItem.Height();

	COLORREF oldCR = pRT->GetTextColor();

	//if(pItem->IsInplaceActive())
	//{
	   //oldCR = pRT->SetTextColor(m_crItemSelText);
	//}
	pItem->DrawItem(pRT,rcItem); //»æÖÆItem

	//pRT->SetTextColor(oldCR);

	CAutoRefPtr<IPen> pen,oldPen;
	pRT->CreatePen(PS_SOLID,m_crBorder,1,&pen);
	pRT->SelectObject(pen,(IRenderObj**)&oldPen);
	CPoint pts[2]={CPoint(rc.left+rc.Height(),rc.bottom-1),CPoint(rc.right,rc.bottom-1)};
	pRT->DrawLines(pts,2);
	CPoint pts2[2]={CPoint(rcNameBack.right,rcNameBack.top),rcNameBack.BottomRight()};
	pRT->DrawLines(pts2,2);
	pRT->SelectObject(oldPen);

}


void SPropertyGridEx::OnInplaceActiveWndCreate( IPropertyItem *pItem,SWindow *pWnd ,pugi::xml_node xmlInit)
{

	//pugi::xml_document xmlDoc;
	//pugi::xml_node xmlNode=xmlDoc.append_child(L"root");
	//xmlNode.append_attribute(L"colorBkgnd").set_value(L"#ffffff");
	xmlInit.attribute(L"colorBkgnd").set_value(m_strEditBkgndColor);
	xmlInit.append_attribute(L"autoWordSel").set_value(m_strEnableAutoWordSel);
	//xmlInit.append_attribute(L"cueColor").set_value(L"#0000FF");
	//xmlInit.attribute(L"colorBkgnd").set_value(L"")

	SASSERT(m_pInplaceActiveWnd == NULL);
	InsertChild(pWnd);
	pWnd->InitFromXml(xmlInit);

	CRect rcItem = GetItemRect(pItem);
	CRect rcValue= rcItem;
	rcValue.left += rcItem.Height()+m_nNameWidth;
	if(pItem->HasButton()) rcValue.right -= rcValue.Height();
	pItem->AdjustInplaceActiveWndRect(rcValue);
	pWnd->Move(rcValue);
	pWnd->SetFocus();
	m_pInplaceActiveWnd = pWnd;

	//SASSERT(m_pInplaceActiveWnd == NULL);
	//InsertChild(pWnd);
	//pWnd->InitFromXml(xmlInit);

	//CRect rcItem = GetItemRect(pItem);
	//CRect rcValue= rcItem;
	//rcValue.left += rcItem.Height()+m_nNameWidth;
	//if(pItem->HasButton()) rcValue.right -= rcValue.Height();
	//pItem->AdjustInplaceActiveWndRect(rcValue);
	//pWnd->Move(rcValue);
	//pWnd->SetFocus();
	//m_pInplaceActiveWnd = pWnd;
}