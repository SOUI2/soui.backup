#pragma once
#include "DesignerView.h"
#include "Adapter.h"
class SPrintWindow : public SWindow
{
	SOUI_CLASS_NAME(SPrintWindow, L"SPrintWindow")
public:
	void Attach(SWindow *pHostCtrl)
	{
		SASSERT(pHostCtrl);
		CRect rcHost = pHostCtrl->GetWindowRect();
		pHostCtrl->InsertChild(this);
		Move(rcHost);
		GETRENDERFACTORY->CreateRenderTarget(&m_memRt, rcHost.Width(), rcHost.Height());
		pHostCtrl->SetVisible(TRUE,TRUE);
		CPoint pt;
		pt-= rcHost.TopLeft();
		m_memRt->SetViewportOrg(pt);
		PaintBackground(m_memRt, &rcHost);
		m_memRt->SetViewportOrg(CPoint());
		pHostCtrl->SetVisible(FALSE);
		m_Size.SetSize(rcHost.Width(), rcHost.Height());
	}
	CSize GetSize()
	{
		return m_Size;
	}
	
	void Print(IRenderTarget *pRT)
	{
		CRect rcWindow(0, 0, m_Size.cx, m_Size.cy);
		pRT->BitBlt(rcWindow, m_memRt, 0, 0, SRCCOPY);
	}
protected:
	CAutoRefPtr<IRenderTarget> m_memRt;
	SWindow *                 m_pHostCtrl;
	CSize	m_Size;
};
class CDropWnd : public SWindow
{
	SOUI_CLASS_NAME(CDropWnd, L"dropwindow")
private:
	CAutoRefPtr<IRenderTarget> m_memRt;	
	HWND m_hHostWnd;
protected:
	void OnPaint(IRenderTarget *pRT)
	{
		if (m_memRt == NULL)
			return;
		CRect rcWindow;
		GetWindowRect(&rcWindow);
		pRT->BitBlt(rcWindow, m_memRt, 0, 0, SRCCOPY);
	}
public:
	CDropWnd():m_memRt(NULL)
	{
		m_hHostWnd = SApplication::getSingleton().GetMainWnd();
	}
	void CopyDropWindBkgndFormWindow(SPrintWindow *FormWindow)
	{
		if (m_memRt)
			m_memRt = NULL;
		CSize sizeWindow = FormWindow->GetSize();		
		Move(0,0, sizeWindow.cx,sizeWindow.cy);
		GETRENDERFACTORY->CreateRenderTarget(&m_memRt, sizeWindow.cx, sizeWindow.cy);
		FormWindow->Print(m_memRt);
		CRect rc(0, 0, sizeWindow.cx-1, sizeWindow.cy-1);
		m_memRt->DrawRectangle(rc);
	}
	void MoveWindow(CPoint &pt)
	{
		//if (GetParent()->IsVisible()) //已经保证显示才拖动
		{
			CRect parentRc=GetParent()->GetClientRect();			
			if (parentRc.PtInRect(pt))
			{
				if (!IsVisible())
				{
					SetVisible(TRUE, TRUE);
					BringWindowToTop();
				}				
				Move(pt.x, pt.y);
			}
			else
			{
				if (IsVisible())
				{
					SetVisible(FALSE, TRUE);
				}
			}
		}
	}
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
	SOUI_MSG_MAP_END()
};
class SListBoxDrop :public SListBox
{
	SOUI_CLASS_NAME(SListBoxDrop, L"listboxdrop")
private:
	SMap<SStringT, pugi::xml_node> * m_controlDb;
	SDesignerView *m_pDesignerView;	
	CDropWnd *m_pCDropWnd;
	bool bFirstDrop, bLdown;
	pugi::xml_node ctrlNode;
public:
	SListBoxDrop() :m_controlDb(NULL), m_pDesignerView(NULL),bFirstDrop(false),bLdown(false)
	{		
	}
	void init(SMap<SStringT, pugi::xml_node> *_controlDB, SDesignerView *_pDesignerView)
	{
		m_controlDb = _controlDB;
		m_pDesignerView = _pDesignerView;		
		m_pCDropWnd=GetTopLevelParent()->FindChildByName2<CDropWnd>(L"dropwindow");
		SASSERT(m_pCDropWnd);
	}
	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
		__super::OnLButtonDown(nFlags, pt);	
		if (!m_pDesignerView->m_pContainer->IsVisible(TRUE))
			return;
		if (m_pDesignerView->m_pContainer->GetChildrenCount()==0)
			return;
		int iHoverItem = HitTest(pt);
		if (iHoverItem != -1)
		{
			if (m_controlDb != NULL)
			{
				SStringT strText;
				GetText(iHoverItem, strText);
				SMap<SStringT, pugi::xml_node>::CPair *p = m_controlDb->Lookup(strText);  //查找
				if (p)
				{
					ctrlNode = p->m_value;
					SWindow *pChild = SApplication::getSingleton().CreateWindowByName(p->m_value.first_child().name());
					if (pChild)
					{
						SPrintWindow *pPrintWindow = new SPrintWindow();						
						m_pDesignerView->m_pContainer->GetParent()->InsertChild(pChild);

						m_pDesignerView->UseEditorUIDef(false);

						pChild->InitFromXml(p->m_value.first_child());
						//view系列加上适配器
						if (pChild->IsClass(SMCListView::GetClassNameW()))
						{
							CBaseMcAdapterFix *mcAdapter = new CBaseMcAdapterFix();
							((SMCListView*)pChild)->SetAdapter(mcAdapter);
							mcAdapter->Release();
						}
						//listview(flex)需要重新处理，有空再来
						if (pChild->IsClass(SListView::GetClassNameW()))
						{
							CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
							((SListView*)pChild)->SetAdapter(listAdapter);
							listAdapter->Release();
						}
						if (pChild->IsClass(STileView::GetClassNameW()))
						{
							CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
							((STileView*)pChild)->SetAdapter(listAdapter);
							listAdapter->Release();
						}

						m_pDesignerView->UseEditorUIDef(true);

						m_pDesignerView->m_pContainer->BringWindowToTop();
						m_pDesignerView->m_pContainer->GetParent()->UpdateChildrenPosition();
						pPrintWindow->Attach(pChild);
						m_pCDropWnd->CopyDropWindBkgndFormWindow(pPrintWindow);	
						m_pCDropWnd->SetVisible(FALSE);
						m_pDesignerView->m_pContainer->GetParent()->DestroyChild(pChild);
						::SetCursor(GETRESPROVIDER->LoadCursor(_T("hand")));
						bLdown = true;	
					}
				}
			}
		}
	}
	void OnMouseMove(UINT nFlags, CPoint pt)
	{		 
		if (m_pDesignerView&&bLdown)
		{
			m_pCDropWnd->MoveWindow(pt);
		}
	}
	void OnLButtonUp(UINT nFlags, CPoint pt)
	{
		bLdown = false;
		__super::OnLButtonUp(nFlags, pt);
		if (m_pCDropWnd->IsVisible())
		{
			m_pCDropWnd->SetVisible(FALSE, TRUE);
			m_pDesignerView->SetSelCtrlNode(ctrlNode);
			SWND pChild = m_pDesignerView->m_pMoveWndRoot->SwndFromPoint(pt, FALSE);			
			SWindow *pSChild = SWindowMgr::GetWindow(pChild);
			if (pSChild == 0)
			{
				m_pDesignerView->m_nState = 0;
				return;
			}

			m_pDesignerView->NewWnd(pt,(SMoveWnd*)pSChild);
			((SMoveWnd*)pSChild)->m_pRealWnd->RequestRelayout();
			((SMoveWnd*)pSChild)->m_pRealWnd->UpdateLayout();

			m_pDesignerView->CreatePropGrid(m_pDesignerView->m_xmlNode.name());
			m_pDesignerView->UpdatePropGrid(m_pDesignerView->m_xmlNode);
		}
	}
	SOUI_MSG_MAP_BEGIN()
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
	SOUI_MSG_MAP_END()
};