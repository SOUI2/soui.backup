#include "stdafx.h"
#include "SMoveWnd.h"
#include "CNewGuid.h"
#include "control/SMessageBox.h"

#define  POINT_SIZE      4     //元素拖动点大小

#define HORZ_LT 0
#define VERT_LT 1

#define HORZ 2
#define VERT 3

#define UP     0
#define DOWN   1
#define LEFT   2
#define RIGHT  3


namespace SOUI
{
	SMoveWnd::SMoveWnd()
	{
		m_hLUpRDown = GETRESPROVIDER->LoadCursor(L"sizenwse");
		m_hAll = GETRESPROVIDER->LoadCursor(L"sizeall");
		m_hNormal = GETRESPROVIDER->LoadCursor(m_style.m_strCursor);
		m_downWindow = 0;
		m_bFocusable = TRUE;
		m_pRealWnd = NULL;
		m_bDrawFocusRect = FALSE;
		m_StateMove = 0;
	}

	SMoveWnd::~SMoveWnd()
	{
	}

	void SOUI::SMoveWnd::OnPaint(IRenderTarget *pRT)
	{
		__super::OnPaint(pRT);

		//这里将实际控件和覆盖在实际控件上面的布局控件的位置更新为一样的
		if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
		{
			CRect rectR;
			CRect rectRP;
			m_pRealWnd->GetWindowRect(rectR);
			m_pRealWnd->GetParent()->GetWindowRect(rectRP);

			SouiLayoutParam *pMoveWndLayout = GetLayoutParamT<SouiLayoutParam>();
			SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pMoveWndLayout->GetRawData();
			//pSouiLayoutParamStruct->pos[0].nPos = 20;
			//pSouiLayoutParamStruct->pos[1].nPos = 20;
			//pSouiLayoutParamStruct->pos[2].nPos = rectR.right - rectR.left;
			//pSouiLayoutParamStruct->pos[3].nPos = rectR.bottom - rectR.top;

			//pMoveWndLayout->SetSpecifiedSize(Horz, rectR.right - rectR.left);
			//pMoveWndLayout->SetSpecifiedSize(Vert, rectR.bottom - rectR.top);
			pSouiLayoutParamStruct->posLeft.nPos.fSize = 20;
			pSouiLayoutParamStruct->posTop.nPos.fSize = 20;
			pSouiLayoutParamStruct->posRight.nPos.fSize = rectR.right - rectR.left;
			pSouiLayoutParamStruct->posBottom.nPos.fSize = rectR.bottom - rectR.top;

			SLayoutSize LayoutSize = pMoveWndLayout->GetSpecifiedSize(Horz);
			LayoutSize.fSize = rectR.right - rectR.left;
			pMoveWndLayout->SetSpecifiedSize(Horz, LayoutSize);

			LayoutSize = pMoveWndLayout->GetSpecifiedSize(Vert);
			LayoutSize.fSize = rectR.bottom - rectR.top;
			pMoveWndLayout->SetSpecifiedSize(Vert, LayoutSize);
			//pMoveWndLayout->SetSpecifiedSize(Horz, rectR.right - rectR.left);
			//pMoveWndLayout->SetSpecifiedSize(Vert, rectR.bottom - rectR.top);

			CRect rect;
			GetWindowRect(rect);

			if (!rect.EqualRect(rectR))
			{
				GetParent()->RequestRelayout();
				GetParent()->UpdateLayout();
			}
		}
		else
		{
			//更新MoveWnd的位置和RealWnd一样
			CRect rectR;
			CRect rectRP;
			m_pRealWnd->GetWindowRect(rectR);
			m_pRealWnd->GetParent()->GetWindowRect(rectRP);

			SouiLayoutParam *pMoveWndLayout = GetLayoutParamT<SouiLayoutParam>();
			SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pMoveWndLayout->GetRawData();
			//pSouiLayoutParamStruct->pos[0].nPos = rectR.left - rectRP.left;
			//pSouiLayoutParamStruct->pos[1].nPos = rectR.top - rectRP.top;
			//pSouiLayoutParamStruct->pos[2].nPos = rectR.right - rectR.left;
			//pSouiLayoutParamStruct->pos[3].nPos = rectR.bottom - rectR.top;

			//pMoveWndLayout->SetSpecifiedSize(Horz, rectR.right - rectR.left);
			//pMoveWndLayout->SetSpecifiedSize(Vert, rectR.bottom - rectR.top);

			pSouiLayoutParamStruct->posLeft.nPos.fSize = rectR.left - rectRP.left;
			pSouiLayoutParamStruct->posTop.nPos.fSize = rectR.top - rectRP.top;
			pSouiLayoutParamStruct->posRight.nPos.fSize = rectR.right - rectR.left;
			pSouiLayoutParamStruct->posBottom.nPos.fSize = rectR.bottom - rectR.top;

			SLayoutSize LayoutSize = pMoveWndLayout->GetSpecifiedSize(Horz);
			LayoutSize.fSize = rectR.right - rectR.left;
			pMoveWndLayout->SetSpecifiedSize(Horz, LayoutSize);

			LayoutSize = pMoveWndLayout->GetSpecifiedSize(Vert);
			LayoutSize.fSize = rectR.bottom - rectR.top;
			pMoveWndLayout->SetSpecifiedSize(Vert, LayoutSize);

			CRect rect;
			GetWindowRect(rect);

			if (!rect.EqualRect(rectR))
			{
				GetParent()->RequestRelayout();
				GetParent()->UpdateLayout();
			}
		}

		CRect rect;
		GetWindowRect(rect);

		SPainter painter;
		BeforePaint(pRT, painter);

		AdjustRect();

		int n = POINT_SIZE / 2;
		//rect.DeflateRect(n,n,n,n);

		CAutoRefPtr<IPen> pen, oldpen;

		if (IsSelect())
		{
			pRT->CreatePen(PS_SOLID, RGBA(255, 0, 0, 255), 2, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldpen);
			pRT->DrawRectangle(m_rcPos1);
			pRT->DrawRectangle(m_rcPos2);
			pRT->DrawRectangle(m_rcPos3);
			pRT->DrawRectangle(m_rcPos4);
			pRT->DrawRectangle(m_rcPos5);
			pRT->DrawRectangle(m_rcPos6);
			pRT->DrawRectangle(m_rcPos7);
			pRT->DrawRectangle(m_rcPos8);

			pRT->DrawRectangle(rect);
			pRT->SelectObject(oldpen);
		}
		else
		{
			pRT->CreatePen(PS_SOLID, RGBA(172, 172, 172, 255), 1, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldpen);

			pRT->DrawRectangle(rect);
			pRT->SelectObject(oldpen);
		}

		AfterPaint(pRT, painter);
	}

	void SMoveWnd::OnLButtonDown(UINT nFlags, CPoint pt)
	{
		if (m_Desiner->m_nState == 1)//如果是创建控件状态
		{
			//鼠标按下时创建控件
			NewWnd(pt);
			m_Desiner->CreatePropGrid(m_Desiner->m_xmlNode.name());
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			m_Desiner->AddCodeToEditor(NULL);
			OnLButtonUp(nFlags, pt);

			return;
		}

		if (m_pRealWnd != m_Desiner->m_pRealWndRoot)
		{
			SStringT s;
			s.Format(_T("%d"), m_pRealWnd->GetUserData());
			m_Desiner->SetCurrentCtrl(m_Desiner->FindNodeByAttr(m_Desiner->m_CurrentLayoutNode, L"data", s), this);
			m_Desiner->CreatePropGrid(m_Desiner->m_xmlNode.name());
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		}
		else
		{
			m_Desiner->SetCurrentCtrl(m_Desiner->m_CurrentLayoutNode, this);
			m_Desiner->CreatePropGrid(_T("hostwnd"));
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		}
		m_Desiner->AddCodeToEditor(NULL);

		SWindow::OnLButtonDown(nFlags, pt);

		Oldx = pt.x;
		Oldy = pt.y;

		if (m_rcPos8.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			m_downWindow = 8;
		}
		else
		{
			if (m_rcPos1.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
				m_downWindow = 1;
			}
			else
				if (m_rcPos2.PtInRect(pt))
				{
					SetCursor(LoadCursor(NULL, IDC_SIZENS));
					m_downWindow = 2;
				}
				else
					if (m_rcPos3.PtInRect(pt))
					{
						//SetCursor(LoadCursor(NULL,IDC_SIZEWE));
						m_downWindow = 3;
					}
					else
						if (m_rcPos4.PtInRect(pt))
						{
							SetCursor(LoadCursor(NULL, IDC_SIZEWE));
							m_downWindow = 4;
						}
						else if (m_rcPos5.PtInRect(pt))
						{
							SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
							m_downWindow = 5;
						}
						else if (m_rcPos6.PtInRect(pt))
						{
							SetCursor(LoadCursor(NULL, IDC_SIZENS));
							m_downWindow = 6;
						}
						else if (m_rcCenter.PtInRect(pt))
						{
							//SetCursor(LoadCursor(NULL,IDC_SIZEALL));
							m_downWindow = 9;
						}
		}
	}

	void SMoveWnd::OnLButtonUp(UINT nFlags, CPoint pt)
	{
		SWindow::OnLButtonUp(nFlags, pt);

		//将控件的位置更新到xml节点;
		if (m_StateMove)
		{
			//if (m_pRealWnd != m_Desiner->m_pRealWndRoot)
			//{
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			m_Desiner->AddCodeToEditor(NULL);
			m_StateMove = 1;
			//}
		}

		m_downWindow = 0;
	}

	void SMoveWnd::OnMouseMove(UINT nFlags, CPoint pt)
	{
		if (!IsSelect())
		{
			return;
		}

		if (0 == m_downWindow) //当前控件没有被按下
		{
			//如果当前只选择一个控件，且为当前控件
			//if( mainWnd.m_designerView.m_CurSelCtrlList.size() == 1 && mainWnd.m_designerView.m_CurSelCtrlList[0]==this)
			//{
			//改变光标
			if (m_rcPos1.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			}
			else if (m_rcPos2.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENS));
			}
			else if (m_rcPos3.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			}
			else  if (m_rcPos4.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			}
			else if (m_rcPos5.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			}
			else if (m_rcPos6.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENS));
			}
			else if (m_rcPos7.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			}
			else  if (m_rcPos8.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			}
			//else  if(m_rcCenter.PtInRect(pt) && (m_Desiner->m_nState == 0)) //中间位置，可拖动控件
			//{
			//	SetCursor(LoadCursor(NULL,IDC_SIZEALL));
			//}

			//}
		}
		else//控件被按下拖动大小和位置的情况
		{
			int x = pt.x - Oldx;
			int y = pt.y - Oldy;
			int x1;
			int y1;


			BOOL bx = FALSE;
			BOOL by = FALSE;

			if (abs(pt.x - Oldx) >= 8 || abs(pt.y - Oldy) >= 8)
			{
				x1 = abs(pt.x - Oldx);
				x1 = x1 / 8;

				y1 = abs(pt.y - Oldy);
				y1 = y1 / 8;

				if (abs(pt.x - Oldx) >= 8)
				{
					if (x > 0)
					{
						x = 8;
					}
					else
					{
						x = -8;
					}

					bx = TRUE;
				}
				else
				{
					x = 0;
				}

				if (abs(pt.y - Oldy) >= 8)
				{
					if (y > 0)
					{
						y = 8;
					}
					else
					{
						y = -8;
					}

					by = TRUE;
				}
				else
				{
					y = 0;
				}
			}
			else
			{
				return;
			}

			switch (m_downWindow)
			{
				/*左边框*/
			case 8:
			{
				if (bx)
				{
					if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
					{
						return; //线性布局不能拖动左边框
					}
					MoveWndSizeLT(x*x1, HORZ_LT);
				}
			}
			break;;

			/*左上角*/
			case 1:
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					return; //线性布局不能拖动左上角
				}

				if (bx)
				{
					MoveWndSizeLT(x*x1, HORZ_LT);
				}
				if (by)
				{
					MoveWndSizeLT(y*y1, VERT_LT);
				}
			}
			break;

			/*上边框*/
			case 2:
			{
				if (by)
				{
					if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
					{
						return; //线性布局不能拖动上边框
					}
					MoveWndSizeLT(y*y1, VERT_LT);
				}
			}
			break;

			/*右上角*/
			case 3:
				return;

				/*右边框*/
			case 4:
			{
				if (bx)
				{
					if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
					{
						MoveWndSize_Linear(x*x1, Horz);
					}
					else
					{
						MoveWndSize(x*x1, HORZ);
					}
				}
			}
			break;

			/*右下角*/
			case 5:
			{
				if (bx)
				{
					if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
					{
						MoveWndSize_Linear(x*x1, Horz);
					}
					else
					{
						MoveWndSize(x*x1, HORZ);
					}
				}
				if (by)
				{
					if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
					{
						MoveWndSize_Linear(y*y1, Vert);
					}
					else
					{
						MoveWndSize(y*y1, VERT);
					}
				}
			}
			break;

			/*下边框*/
			case 6:
			{
				if (by)
				{
					if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
					{
						MoveWndSize_Linear(y*y1, Vert);
					}
					else
					{
						MoveWndSize(y*y1, VERT);
					}
				}
			}
			break;

			/*左下角*/
			case 7:
				return;

				/*中间拖动*/
			case 9:
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					return; //线性布局不能拖动
				}
				if (bx)
				{
					MoveWndHorz(x*x1);
				}
				if (by)
				{
					MoveWndVert(y*y1);
				}
			}
			break;

			default:
				return;
			}


			Oldx = Oldx + x*x1;
			Oldy = Oldy + y*y1;
			m_pRealWnd->GetParent()->RequestRelayout();
			m_pRealWnd->GetParent()->UpdateLayout();

			GetParent()->RequestRelayout();
			GetParent()->UpdateLayout();

			m_StateMove = 1;
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);

			GetParent()->Invalidate(); //刷新父窗口
		}
	}


	void SMoveWnd::AdjustRect()
	{
		CRect rect;
		GetWindowRect(rect);





		m_rcPos1.left = rect.left;
		m_rcPos1.top = rect.top;
		m_rcPos1.right = rect.left + POINT_SIZE;
		m_rcPos1.bottom = rect.top + POINT_SIZE;

		m_rcPos2.left = rect.left + (rect.right - rect.left) / 2 - POINT_SIZE / 2;
		m_rcPos2.top = rect.top;
		m_rcPos2.right = m_rcPos2.left + POINT_SIZE;
		m_rcPos2.bottom = rect.top + POINT_SIZE;

		m_rcPos3.left = rect.right - POINT_SIZE;
		m_rcPos3.top = rect.top;
		m_rcPos3.right = rect.right;
		m_rcPos3.bottom = rect.top + POINT_SIZE;

		m_rcPos4.left = rect.right - POINT_SIZE;
		m_rcPos4.top = rect.top + (rect.bottom - rect.top) / 2 - POINT_SIZE / 2;
		m_rcPos4.right = rect.right;
		m_rcPos4.bottom = m_rcPos4.top + POINT_SIZE;

		m_rcPos5.left = rect.right - POINT_SIZE;
		m_rcPos5.top = rect.bottom - POINT_SIZE;
		m_rcPos5.right = rect.right;
		m_rcPos5.bottom = rect.bottom;

		m_rcPos6.left = m_rcPos2.left;
		m_rcPos6.top = rect.bottom - POINT_SIZE;
		m_rcPos6.right = m_rcPos2.right;
		m_rcPos6.bottom = rect.bottom;

		m_rcPos7.left = rect.left;
		m_rcPos7.top = rect.bottom - POINT_SIZE;
		m_rcPos7.right = rect.left + POINT_SIZE;
		m_rcPos7.bottom = rect.bottom;

		m_rcPos8.left = rect.left;
		m_rcPos8.top = m_rcPos4.top;
		m_rcPos8.right = rect.left + POINT_SIZE;
		m_rcPos8.bottom = m_rcPos4.bottom;

		m_rcCenter.left = rect.left + POINT_SIZE;
		m_rcCenter.top = rect.top + POINT_SIZE;
		m_rcCenter.right = rect.right - POINT_SIZE;
		m_rcCenter.bottom = rect.bottom - POINT_SIZE;

	}

	BOOL SMoveWnd::IsSelect()
	{
		if (m_Desiner == NULL)
			return FALSE;

		if (m_Desiner->m_CurSelCtrl == this)
		{
			return TRUE;
		}

		return FALSE;
	}


	void SMoveWnd::MoveWndHorz(int x)
	{
		//水平移动
		if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
		{
			return;
		}

		ILayoutParam *pSouiLayoutParam = GetLayoutParam();
		SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pSouiLayoutParam->GetRawData();

		ILayoutParam *pSouiLayoutParam1 = m_pRealWnd->GetLayoutParam();
		SouiLayoutParamStruct *pSouiLayoutParamStruct1 = (SouiLayoutParamStruct*)pSouiLayoutParam1->GetRawData();


		//有margin的情况
		SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
		int nMargin = 0;

		CRect rcMargin = style.GetMargin();

		nMargin = rcMargin.left;

		//往左拖动，left不能小于0
		if (pSouiLayoutParamStruct->posLeft.nPos.fSize + x - nMargin < 0 && x < 0)
		{
			return;
		}


		//往右拖动，right不能大于父控件的right
		if (x > 0)
		{
			CRect r;
			GetParent()->GetWindowRect(r);

			if (pSouiLayoutParamStruct->posLeft.nPos.fSize + pSouiLayoutParam->GetSpecifiedSize(Horz).fSize + x + rcMargin.right > r.right - r.left)
			{
				return;
			}
		}



		if (pSouiLayoutParamStruct1->nCount == 2) //两个坐标的情况
		{
			if (pSouiLayoutParamStruct1->posLeft.cMinus == -1)// 坐标1为负数的情况
			{
				if (pSouiLayoutParamStruct1->posLeft.nPos.fSize - x < 0)
				{
					return;
				}

				pSouiLayoutParamStruct->posLeft.nPos.fSize = pSouiLayoutParamStruct->posLeft.nPos.fSize + x;
				pSouiLayoutParamStruct1->posLeft.nPos.fSize = pSouiLayoutParamStruct1->posLeft.nPos.fSize - x;

			}
			else//坐标1为正数
			{
				if (pSouiLayoutParamStruct1->posRight.cMinus == -1)
				{
					if (pSouiLayoutParamStruct1->posRight.nPos.fSize - x < 0)
					{
						return;
					}
				}

				pSouiLayoutParamStruct->posLeft.nPos.fSize = pSouiLayoutParamStruct->posLeft.nPos.fSize + x;
				pSouiLayoutParamStruct1->posLeft.nPos.fSize = pSouiLayoutParamStruct1->posLeft.nPos.fSize + x;
			}
		}
		else if (pSouiLayoutParamStruct1->nCount == 4)
		{


			int nPosTop, nPosTop1;
			int nPosButtom, nPosButtom1;



			/************************* 移动top点 **************************************/

			if (pSouiLayoutParamStruct1->posLeft.cMinus == -1)// 坐标1为负数的情况
			{
				if (pSouiLayoutParamStruct1->posLeft.nPos.fSize - x < 0)
				{
					return;
				}

				nPosTop = pSouiLayoutParamStruct->posLeft.nPos.fSize + x; //layout->pos[1].nPos = layout->pos[1].nPos + x;   
				nPosTop1 = pSouiLayoutParamStruct1->posLeft.nPos.fSize - x; //layout1->pos[1].nPos = layout1->pos[1].nPos - x;

			}
			else//坐标1为正数
			{
				if (pSouiLayoutParamStruct1->posRight.cMinus == -1)
				{
					if (pSouiLayoutParamStruct1->posRight.nPos.fSize - x < 0)
					{
						return;
					}
				}

				nPosTop = pSouiLayoutParamStruct->posLeft.nPos.fSize + x;    //layout->pos[1].nPos = layout->pos[1].nPos + x;
				nPosTop1 = pSouiLayoutParamStruct1->posLeft.nPos.fSize + x;  //layout1->pos[1].nPos = layout1->pos[1].nPos + x;
			}

			/************************* 移动 top **************************************/




			/************************* 移动 buttom  **************************************/
			if (pSouiLayoutParamStruct1->posRight.pit == PIT_SIZE)  //100, 100 ,@5, @5这种情况
			{
				//

				pSouiLayoutParamStruct->posLeft.nPos.fSize = nPosTop;
				pSouiLayoutParamStruct1->posLeft.nPos.fSize = nPosTop1;

			}
			else
			{
				if (pSouiLayoutParamStruct1->posRight.cMinus == -1)// 坐标1为负数的情况
				{
					if (pSouiLayoutParamStruct1->posRight.nPos.fSize - x < 0)
					{
						return;
					}

					nPosButtom = pSouiLayoutParamStruct->posRight.nPos.fSize + x;  //layout->pos[3].nPos = layout->pos[3].nPos + x;
					nPosButtom1 = pSouiLayoutParamStruct1->posRight.nPos.fSize - x; //layout1->pos[3].nPos = layout1->pos[3].nPos - x;

				}
				else//坐标1为正数
				{
					nPosButtom = pSouiLayoutParamStruct->posRight.nPos.fSize + x; //layout->pos[3].nPos = layout->pos[3].nPos + x;
					nPosButtom1 = pSouiLayoutParamStruct1->posRight.nPos.fSize + x; //layout1->pos[3].nPos = layout1->pos[3].nPos + x;
				}


				pSouiLayoutParamStruct->posLeft.nPos.fSize = nPosTop;
				pSouiLayoutParamStruct1->posLeft.nPos.fSize = nPosTop1;
				pSouiLayoutParamStruct->posRight.nPos.fSize = nPosButtom;
				pSouiLayoutParamStruct1->posRight.nPos.fSize = nPosButtom1;

			}
			/************************* 移动 buttom **************************************/



		}


	}
	void SMoveWnd::MoveWndVert(int x)
	{
		//垂直移动
		if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
		{
			return;
		}

		SouiLayoutParam *pSouiLayoutParam = GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pSouiLayoutParam->GetRawData();

		SouiLayoutParam *pSouiLayoutParam1 = m_pRealWnd->GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct1 = (SouiLayoutParamStruct*)pSouiLayoutParam1->GetRawData();

		//有margin的情况
		SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
		int nMargin = 0;

		CRect rcMargin = style.GetMargin();
		nMargin = rcMargin.top;

		//往上拖动，top不能小于0
		if (pSouiLayoutParamStruct->posTop.nPos.fSize + x - nMargin < 0 && x < 0)
		{
			return;
		}

		//往下拖动，bottom不能大于父控件的bottom
		if (x > 0)
		{
			CRect r;
			GetParent()->GetWindowRect(r);

			if (pSouiLayoutParamStruct->posTop.nPos.fSize + pSouiLayoutParam->GetSpecifiedSize(Vert).fSize + x + rcMargin.bottom > r.bottom - r.top)
			{
				return;
			}
		}

		if (pSouiLayoutParamStruct1->nCount == 2) //两个坐标的情况
		{
			if (pSouiLayoutParamStruct1->posTop.cMinus == -1)// 坐标1为负数的情况
			{
				if (pSouiLayoutParamStruct1->posTop.nPos.fSize - x < 0)
				{
					return;
				}

				pSouiLayoutParamStruct->posTop.nPos.fSize = pSouiLayoutParamStruct->posTop.nPos.fSize + x;
				pSouiLayoutParamStruct1->posTop.nPos.fSize = pSouiLayoutParamStruct1->posTop.nPos.fSize - x;

			}
			else//坐标1为正数
			{
				if (pSouiLayoutParamStruct1->posBottom.cMinus == -1)
				{
					if (pSouiLayoutParamStruct1->posBottom.nPos.fSize - x < 0)
					{
						return;
					}
				}

				pSouiLayoutParamStruct->posTop.nPos.fSize = pSouiLayoutParamStruct->posTop.nPos.fSize + x;
				pSouiLayoutParamStruct1->posTop.nPos.fSize = pSouiLayoutParamStruct1->posTop.nPos.fSize + x;
			}
		}
		else if (pSouiLayoutParamStruct1->nCount == 4)
		{
			int nPosTop, nPosTop1;
			int nPosButtom, nPosButtom1;
			/************************* 移动top点 **************************************/

			if (pSouiLayoutParamStruct1->posTop.cMinus == -1)// 坐标1为负数的情况
			{
				if (pSouiLayoutParamStruct1->posTop.nPos.fSize - x < 0)
				{
					return;
				}

				nPosTop = pSouiLayoutParamStruct->posTop.nPos.fSize + x; //layout->pos[1].nPos = layout->pos[1].nPos + x;   
				nPosTop1 = pSouiLayoutParamStruct1->posTop.nPos.fSize - x; //layout1->pos[1].nPos = layout1->pos[1].nPos - x;
			}
			else//坐标1为正数
			{
				if (pSouiLayoutParamStruct1->posBottom.cMinus == -1)
				{
					if (pSouiLayoutParamStruct1->posBottom.nPos.fSize - x < 0)
					{
						return;
					}
				}

				nPosTop = pSouiLayoutParamStruct->posTop.nPos.fSize + x;    //layout->pos[1].nPos = layout->pos[1].nPos + x;
				nPosTop1 = pSouiLayoutParamStruct1->posTop.nPos.fSize + x;  //layout1->pos[1].nPos = layout1->pos[1].nPos + x;
			}

			/************************* 移动 top **************************************/

			/************************* 移动 buttom  **************************************/
			if (pSouiLayoutParamStruct1->posBottom.pit == PIT_SIZE)  //100, 100 ,@5, @5这种情况
			{
				//
				pSouiLayoutParamStruct->posTop.nPos.fSize = nPosTop;
				pSouiLayoutParamStruct1->posTop.nPos.fSize = nPosTop1;
			}
			else
			{
				if (pSouiLayoutParamStruct1->posBottom.cMinus == -1)// 坐标1为负数的情况
				{
					if (pSouiLayoutParamStruct1->posBottom.nPos.fSize - x < 0)
					{
						return;
					}

					nPosButtom = pSouiLayoutParamStruct->posBottom.nPos.fSize + x;  //layout->pos[3].nPos = layout->pos[3].nPos + x;
					nPosButtom1 = pSouiLayoutParamStruct1->posBottom.nPos.fSize - x; //layout1->pos[3].nPos = layout1->pos[3].nPos - x;
				}
				else//坐标1为正数
				{
					nPosButtom = pSouiLayoutParamStruct->posBottom.nPos.fSize + x; //layout->pos[3].nPos = layout->pos[3].nPos + x;
					nPosButtom1 = pSouiLayoutParamStruct1->posBottom.nPos.fSize + x; //layout1->pos[3].nPos = layout1->pos[3].nPos + x;
				}

				pSouiLayoutParamStruct->posTop.nPos.fSize = nPosTop;
				pSouiLayoutParamStruct1->posTop.nPos.fSize = nPosTop1;
				pSouiLayoutParamStruct->posBottom.nPos.fSize = nPosButtom;
				pSouiLayoutParamStruct1->posBottom.nPos.fSize = nPosButtom1;
			}
			/************************* 移动 buttom **************************************/
		}
	}

	void SMoveWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_bCtrlShift)
		{
			m_Desiner->ShowMovWndChild(TRUE, (SMoveWnd*)this->GetWindow(GSW_FIRSTCHILD));
			m_bCtrlShift = FALSE;
		}
	}

	void SMoveWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
		{
			m_bMsgHandled = FALSE;
			return;
		}

		//左上右下方向键        选中前、后一个控件
		//esc              选中父控件
		//ctrl  + 方向键   上下左右移动1个点的位置
		//Shift + 方向键   增减控件的尺寸
		//delete           删除当前控件
		//CTRL + SHIFT 当控件被子控件挡住了，这时候是无法移动控件的，因为无论怎么选都只能选中子控件，这时按住ctrl+ shift可以隐藏子控件，使控件可以选中从而移动

		m_bMsgHandled = FALSE;

		BOOL bShift = (::GetKeyState(VK_SHIFT) < 0);
		BOOL bCtrl = (::GetKeyState(VK_CONTROL) < 0);

		if (bShift&&bCtrl)
		{
			m_Desiner->ShowMovWndChild(FALSE, (SMoveWnd*)this->GetWindow(GSW_FIRSTCHILD));
			m_bCtrlShift = TRUE;
			return;
		}
		else
		{
			if (m_bCtrlShift)
			{
				m_Desiner->ShowMovWndChild(TRUE, (SMoveWnd*)this->GetWindow(GSW_FIRSTCHILD));
				m_bCtrlShift = FALSE;
			}
		}

		switch (nChar)
		{
		case VK_UP:
			if (bCtrl)
			{

				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					return; //线性布局不能移动
				}
				MoveWndVert(-1);
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口

			}
			else if (bShift)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					MoveWndSize_Linear(-1, Vert);
				}
				else
				{
					MoveWndSize(-1, VERT);
				}
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口

			}
			else
			{
				SMap<SWindow*, SMoveWnd*>::CPair *p = m_Desiner->m_mapMoveRealWnd.Lookup(m_pRealWnd->GetWindow(GSW_PREVSIBLING));
				if (p)
				{
					SMoveWnd *pMovWnd = p->m_value;
					pMovWnd->Click(0, CPoint(0, 0));
				}

			}
			break;

		case VK_LEFT:
			if (bCtrl)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					return;
				}
				else
				{
					MoveWndHorz(-1);
				}
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口
			}
			else if (bShift)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					MoveWndSize_Linear(-1, Horz);
				}
				else
				{
					MoveWndSize(-1, HORZ);
				}
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口
			}
			else
			{
				SMap<SWindow*, SMoveWnd*>::CPair *p = m_Desiner->m_mapMoveRealWnd.Lookup(m_pRealWnd->GetWindow(GSW_PREVSIBLING));
				if (p)
				{
					SMoveWnd *pMovWnd = p->m_value;
					pMovWnd->Click(0, CPoint(0, 0));
				}
			}
			break;

		case VK_DOWN:
			if (bCtrl)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					return;
				}
				else
				{
					MoveWndVert(1);
				}
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口
			}
			else if (bShift)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					MoveWndSize_Linear(1, Vert);
				}
				else
				{
					MoveWndSize(1, VERT);
				}
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口
			}
			else
			{
				SMap<SWindow*, SMoveWnd*>::CPair *p = m_Desiner->m_mapMoveRealWnd.Lookup(m_pRealWnd->GetWindow(GSW_NEXTSIBLING));
				if (p)
				{
					SMoveWnd *pMovWnd = p->m_value;
					pMovWnd->Click(0, CPoint(0, 0));
				}
			}
			break;

		case VK_RIGHT:
			if (bCtrl)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					return;
				}
				MoveWndHorz(1);
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口
			}
			else if (bShift)
			{
				if (m_pRealWnd->GetLayoutParam()->IsClass(SLinearLayoutParam::GetClassName()))
				{
					MoveWndSize_Linear(1, Horz);
				}
				else
				{
					MoveWndSize(1, HORZ);
				}
				m_bMsgHandled = TRUE;

				m_pRealWnd->GetParent()->RequestRelayout();
				m_pRealWnd->GetParent()->UpdateChildrenPosition();

				GetParent()->RequestRelayout();
				GetParent()->UpdateChildrenPosition();
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				GetParent()->Invalidate(); //刷新父窗口
			}
			else
			{
				SMap<SWindow*, SMoveWnd*>::CPair *p = m_Desiner->m_mapMoveRealWnd.Lookup(m_pRealWnd->GetWindow(GSW_NEXTSIBLING));
				if (p)
				{
					SMoveWnd *pMovWnd = p->m_value;
					pMovWnd->Click(0, CPoint(0, 0));
				}
			}
			break;

		case VK_ESCAPE:
		{
			SMap<SWindow*, SMoveWnd*>::CPair *p = m_Desiner->m_mapMoveRealWnd.Lookup(m_pRealWnd->GetWindow(GSW_PARENT));
			if (p)
			{
				SMoveWnd *pMovWnd = p->m_value;
				pMovWnd->Click(0, CPoint(0, 0));
			}
			m_bMsgHandled = TRUE;
		}
		break;

		case VK_DELETE:
		{
			int n = SMessageBox(NULL, _T("确定要删除吗？"), _T("提示"), MB_OKCANCEL);
			if (n == IDOK)
			{
				m_Desiner->DeleteCtrl();
			}
			m_bMsgHandled = TRUE;
		}
		break;

		default:
			m_bMsgHandled = FALSE;
		}


	}


	void SMoveWnd::MoveWndSizeLT(int x, int PosN)
	{
		if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
		{
			return;
		}

		SouiLayoutParam *pSouiLayoutParam = GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pSouiLayoutParam->GetRawData();

		SouiLayoutParam *pSouiLayoutParam1 = m_pRealWnd->GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct1 = (SouiLayoutParamStruct*)pSouiLayoutParam1->GetRawData();


		CRect rcReal, rcRealParent;
		m_pRealWnd->GetWindowRect(rcReal);
		m_pRealWnd->GetParent()->GetWindowRect(rcRealParent);

		//有margin的情况
		SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
		int nMargin = 0;

		CRect rcMargin = style.GetMargin();
		if (PosN == HORZ_LT)
		{
			nMargin = rcMargin.left;
		}
		else
		{
			nMargin = rcMargin.top;
		}

		//往左、上拖动大小，left、top不能小于0
		/*if (pSouiLayoutParamStruct->pos[PosN].nPos + x - nMargin < 0 && x < 0)*/
		if (GetLayoutSize(pSouiLayoutParamStruct, PosN) + x - nMargin < 0 && x < 0)
		{
			return;
		}

		//往右拖动大小,left 不能大于 right
		if (pSouiLayoutParam->GetSpecifiedSize(Horz).fSize - x < 1 && PosN == 0 && x > 0)
		{
			return;
		}

		//往下拖动大小,top 不能大于 buttom
		if (pSouiLayoutParam->GetSpecifiedSize(Vert).fSize - x < 1 && PosN == 1 && x > 0)
		{
			return;
		}

		/*pSouiLayoutParamStruct->pos[PosN].nPos = pSouiLayoutParamStruct->pos[PosN].nPos + x;*/
		SetLayoutSize(pSouiLayoutParamStruct, PosN, GetLayoutSize(pSouiLayoutParamStruct, PosN) + x);
		if (PosN == 0)
		{
			SLayoutSize LayoutSize = pSouiLayoutParam->GetSpecifiedSize(Horz);
			LayoutSize.fSize = LayoutSize.fSize - x;
			pSouiLayoutParam->SetSpecifiedSize(Horz, LayoutSize);
			/*pSouiLayoutParam->SetSpecifiedSize(Horz, pSouiLayoutParam->GetSpecifiedSize(Horz) - x);*/
		}
		else
		{
			/*pSouiLayoutParam->SetSpecifiedSize(Vert, pSouiLayoutParam->GetSpecifiedSize(Vert) - x);*/
			SLayoutSize LayoutSize = pSouiLayoutParam->GetSpecifiedSize(Vert);
			LayoutSize.fSize = LayoutSize.fSize - x;
			pSouiLayoutParam->SetSpecifiedSize(Vert, LayoutSize);
		}

		if (pSouiLayoutParamStruct1->nCount == 2) //只有两个坐标点，自动计算大小
		{
			if (pSouiLayoutParam1->IsSpecifiedSize(Both))//用size指定大小的时候
			{

				/*pSouiLayoutParamStruct1->pos[PosN].nPos = pSouiLayoutParamStruct1->pos[PosN].nPos + x;*/
				SetLayoutSize(pSouiLayoutParamStruct1, PosN, GetLayoutSize(pSouiLayoutParamStruct1, PosN) + x);
				if (PosN == 0)
				{   //宽
					/*pSouiLayoutParam1->SetSpecifiedSize(Horz, pSouiLayoutParam1->GetSpecifiedSize(Horz) - x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Horz);
					LayoutSize.fSize = LayoutSize.fSize - x;
					pSouiLayoutParam1->SetSpecifiedSize(Horz, LayoutSize);
				}
				else
				{   //高
					/*pSouiLayoutParam1->SetSpecifiedSize(Vert, pSouiLayoutParam1->GetSpecifiedSize(Vert) - x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Vert);
					LayoutSize.fSize = LayoutSize.fSize - x;
					pSouiLayoutParam1->SetSpecifiedSize(Vert, LayoutSize);
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			//-100, -100, @20,@20
			if (GetPosInfo(pSouiLayoutParamStruct1, PosN).cMinus == -1)// 坐标为负数的情况
			{

				//pSouiLayoutParamStruct1->pos[PosN].nPos = pSouiLayoutParamStruct1->pos[PosN].nPos - x;
				SetLayoutSize(pSouiLayoutParamStruct1, PosN, GetLayoutSize(pSouiLayoutParamStruct1, PosN) - x);
			}
			else
			{
				/*pSouiLayoutParamStruct1->pos[PosN].nPos = pSouiLayoutParamStruct1->pos[PosN].nPos + x;*/
				SetLayoutSize(pSouiLayoutParamStruct1, PosN, GetLayoutSize(pSouiLayoutParamStruct1, PosN) + x);
			}

			if (GetPosInfo(pSouiLayoutParamStruct1, PosN + 2).pit == PIT_SIZE)  //@5这种情况
			{

				/*pSouiLayoutParamStruct1->pos[PosN + 2].nPos = pSouiLayoutParamStruct1->pos[PosN + 2].nPos - x;*/
				SetLayoutSize(pSouiLayoutParamStruct1, PosN + 2, GetLayoutSize(pSouiLayoutParamStruct1, PosN + 2) - x);
				if (PosN == 0)
				{
					/*pSouiLayoutParam1->SetSpecifiedSize(Horz, pSouiLayoutParam1->GetSpecifiedSize(Horz) - x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Horz);
					LayoutSize.fSize = LayoutSize.fSize - x;
					pSouiLayoutParam1->SetSpecifiedSize(Horz, LayoutSize);
				}
				else
				{
					/*pSouiLayoutParam1->SetSpecifiedSize(Vert, pSouiLayoutParam1->GetSpecifiedSize(Vert) - x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Vert);
					LayoutSize.fSize = LayoutSize.fSize - x;
					pSouiLayoutParam1->SetSpecifiedSize(Vert, LayoutSize);
				}


			}
			else  //描点坐标
			{
				//80,100,50,80 这种情况，top < buttom  right < reft的暂时不考虑 
				//
				//layout1->pos[PosN].nPos = layout1->pos[PosN].nPos + x;
			}
		}
	}

	void SMoveWnd::MoveWndSize(int x, int PosN)
	{
		//SwndLayout *layout = GetLayout();
		//SwndLayout *layout1 = m_pRealWnd->GetLayout();

		SouiLayoutParam *pSouiLayoutParam = GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pSouiLayoutParam->GetRawData();

		SouiLayoutParam *pSouiLayoutParam1 = m_pRealWnd->GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct1 = (SouiLayoutParamStruct*)pSouiLayoutParam1->GetRawData();

		CRect rcReal, rcRealParent, rcMovParent;
		m_pRealWnd->GetWindowRect(rcReal);
		m_pRealWnd->GetParent()->GetWindowRect(rcRealParent);
		GetParent()->GetWindowRect(rcMovParent);

		//有margin的情况
		SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
		int nMargin = 0;

		CRect rcMargin = style.GetMargin();
		if (PosN == HORZ)
		{
			nMargin = rcMargin.right;
		}
		else
		{
			nMargin = rcMargin.bottom;
		}

		//右拖动不能超过父控件的右边距
		if (PosN == 2 && x > 0)
		{
			if (GetLayoutSize(pSouiLayoutParamStruct, PosN - 2) + pSouiLayoutParam->GetSpecifiedSize(Horz).fSize + x + nMargin > rcMovParent.right - rcMovParent.left)
			{
				return;
			}
		}

		//下拖动不能超过父控件的下边距
		if (PosN == 3 && x > 0)
		{
			if (GetLayoutSize(pSouiLayoutParamStruct, PosN - 2) + pSouiLayoutParam->GetSpecifiedSize(Vert).fSize + x + nMargin > rcMovParent.bottom - rcMovParent.top)
			{
				return;
			}
		}

		//有margin的情况  
		SwndStyle &style1 = m_pRealWnd->GetStyle();
		int nMarginLeft = 0, nMarginTop = 0, nMarginBottom = 0, nMarginRight = 0;

		rcMargin = style1.GetMargin();

		if (PosN == HORZ)
		{
			nMarginRight = rcMargin.right;
			nMarginLeft = rcMargin.left;
		}
		else
		{
			nMarginBottom = rcMargin.bottom;
			nMarginTop = rcMargin.top;
		}

		//往左拖动大小,left 不能大于 right
		if (pSouiLayoutParam->GetSpecifiedSize(Horz).fSize + x - (nMarginRight + nMarginLeft) < 1 && PosN == 2 && x < 0)
		{
			return;
		}

		//往上拖动大小,top 不能大于 buttom
		if (pSouiLayoutParam->GetSpecifiedSize(Vert).fSize + x - ((nMarginBottom + nMarginTop)) < 1 && PosN == 3 && x < 0)
		{
			return;
		}

		/*pSouiLayoutParamStruct->pos[PosN].nPos = pSouiLayoutParamStruct->pos[PosN].nPos + x;*/
		SetLayoutSize(pSouiLayoutParamStruct, PosN, GetLayoutSize(pSouiLayoutParamStruct, PosN) + x);

		if (PosN == 2)
		{
			/*pSouiLayoutParam->SetSpecifiedSize(Horz, pSouiLayoutParam->GetSpecifiedSize(Horz) + x);*/
			SLayoutSize LayoutSize = pSouiLayoutParam->GetSpecifiedSize(Horz);
			LayoutSize.fSize = LayoutSize.fSize + x;
			pSouiLayoutParam->SetSpecifiedSize(Horz, LayoutSize);
		}
		else
		{
			/*pSouiLayoutParam->SetSpecifiedSize(Vert, pSouiLayoutParam->GetSpecifiedSize(Vert) + x);*/
			SLayoutSize LayoutSize = pSouiLayoutParam->GetSpecifiedSize(Vert);
			LayoutSize.fSize = LayoutSize.fSize + x;
			pSouiLayoutParam->SetSpecifiedSize(Vert, LayoutSize);
		}

		if (pSouiLayoutParamStruct1->nCount == 0)
		{
			if (pSouiLayoutParam1->IsSpecifiedSize(Both))//用size指定大小的时候
			{
				if (PosN == 2)
				{
					/*pSouiLayoutParam1->SetSpecifiedSize(Horz, pSouiLayoutParam1->GetSpecifiedSize(Horz) + x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Horz);
					LayoutSize.fSize = LayoutSize.fSize + x;
					pSouiLayoutParam1->SetSpecifiedSize(Horz, LayoutSize);
				}
				else
				{
					/*pSouiLayoutParam1->SetSpecifiedSize(Vert, pSouiLayoutParam1->GetSpecifiedSize(Vert) + x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Vert);
					LayoutSize.fSize = LayoutSize.fSize + x;
					pSouiLayoutParam1->SetSpecifiedSize(Vert, LayoutSize);
				}
			}
		}
		else if (pSouiLayoutParamStruct1->nCount == 2) //只有两个坐标点，自动计算大小
		{
			if (pSouiLayoutParam1->IsSpecifiedSize(Both))//用size指定大小的时候
			{

				/*pSouiLayoutParamStruct->pos[PosN].nPos = pSouiLayoutParamStruct->pos[PosN].nPos + x;*/
				SetLayoutSize(pSouiLayoutParamStruct, PosN, GetLayoutSize(pSouiLayoutParamStruct, PosN) + x);
				if (PosN == 2)
				{   //宽
					/*pSouiLayoutParam1->SetSpecifiedSize(Horz, pSouiLayoutParam1->GetSpecifiedSize(Horz) + x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Horz);
					LayoutSize.fSize = LayoutSize.fSize + x;
					pSouiLayoutParam1->SetSpecifiedSize(Horz, LayoutSize);
				}
				else
				{   //高
					/*pSouiLayoutParam1->SetSpecifiedSize(Vert, pSouiLayoutParam1->GetSpecifiedSize(Vert) + x);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Vert);
					LayoutSize.fSize = LayoutSize.fSize + x;
					pSouiLayoutParam1->SetSpecifiedSize(Vert, LayoutSize);
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			//5,3,-5,-7
			//5,3,@5,@7

			if (GetPosInfo(pSouiLayoutParamStruct1, PosN).cMinus == -1)// 坐标3为负数的情况
			{
				/*pSouiLayoutParamStruct1->pos[PosN].nPos = pSouiLayoutParamStruct1->pos[PosN].nPos - x;*/
				SetLayoutSize(pSouiLayoutParamStruct1, PosN, GetLayoutSize(pSouiLayoutParamStruct1, PosN) - x);
			}
			else if (GetPosInfo(pSouiLayoutParamStruct1, PosN).pit == PIT_SIZE)  //@5这种情况
			{
				//layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
				/*pSouiLayoutParamStruct1->pos[PosN].nPos = pSouiLayoutParamStruct1->pos[PosN].nPos + x;*/
				SetLayoutSize(pSouiLayoutParamStruct1, PosN, GetLayoutSize(pSouiLayoutParamStruct1, PosN) + x);
				if (PosN == 2)
				{
					/*pSouiLayoutParam1->SetSpecifiedSize(Horz, pSouiLayoutParamStruct1->pos[PosN].nPos);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Horz);
					LayoutSize.fSize = GetLayoutSize(pSouiLayoutParamStruct1, PosN);
					pSouiLayoutParam1->SetSpecifiedSize(Horz, LayoutSize);
				}
				else
				{
					/*pSouiLayoutParam1->SetSpecifiedSize(Vert, pSouiLayoutParamStruct1->pos[PosN].nPos);*/
					SLayoutSize LayoutSize = pSouiLayoutParam1->GetSpecifiedSize(Vert);
					LayoutSize.fSize = GetLayoutSize(pSouiLayoutParamStruct1, PosN);
					pSouiLayoutParam1->SetSpecifiedSize(Vert, LayoutSize);
				}
			}
			else  //描点坐标
			{
				//80,100,50,80 这种情况，top < buttom  right < reft的暂时不考虑 

				//layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
				/*pSouiLayoutParamStruct1->pos[PosN].nPos = pSouiLayoutParamStruct1->pos[PosN].nPos + x;*/
				SetLayoutSize(pSouiLayoutParamStruct1, PosN, GetLayoutSize(pSouiLayoutParamStruct1, PosN) + x);
			}

		}
	}

	void SMoveWnd::MoveWndSize_Linear(int x, ORIENTATION orientation)
	{
		SouiLayoutParam *pSouiLayoutParam = GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pSouiLayoutParam->GetRawData();

		SLinearLayoutParam *pLinearLayoutParam = m_pRealWnd->GetLayoutParamT<SLinearLayoutParam>();
		SLinearLayoutParamStruct *pSLinearLayoutParamStruct = (SLinearLayoutParamStruct*)pLinearLayoutParam->GetRawData();

		CRect rcReal, rcRealParent, rcMovParent;
		m_pRealWnd->GetWindowRect(rcReal);
		m_pRealWnd->GetParent()->GetWindowRect(rcRealParent);
		GetParent()->GetWindowRect(rcMovParent);


		//不能拖出父控件边界
		if (orientation == Horz)  //向左右拉动右边的边框
		{
			if (pLinearLayoutParam->IsMatchParent(Horz))  //匹配父窗口时不能改变大小
			{
				return;
			}

			if (pSLinearLayoutParamStruct->width.fSize + x > 0)
			{
				pSLinearLayoutParamStruct->width.fSize = pSLinearLayoutParamStruct->width.fSize + x;
				pSouiLayoutParamStruct->posRight.nPos.fSize = pSouiLayoutParamStruct->posRight.nPos.fSize + x;
			}
		}
		else  ////向上下拉动下边的边框
		{
			if (pLinearLayoutParam->IsMatchParent(Vert))  //匹配父窗口时不能改变大小
			{
				return;
			}

			if (pSLinearLayoutParamStruct->height.fSize + x > 0)
			{
				pSLinearLayoutParamStruct->height.fSize = pSLinearLayoutParamStruct->height.fSize + x;
				pSouiLayoutParamStruct->posBottom.nPos.fSize = pSouiLayoutParamStruct->posBottom.nPos.fSize + x;
			}
		}
	}


	void SMoveWnd::NewWnd(CPoint pt)
	{
		m_Desiner->NewWnd(pt, this);
	}

	void SMoveWnd::Click(UINT nFlags, CPoint pt)
	{
		OnLButtonDown(nFlags, pt);
		OnLButtonUp(nFlags, pt);
	}

	float SMoveWnd::GetLayoutSize(SouiLayoutParamStruct *pSouiLayoutParam, int PosN)
	{
		switch (PosN)
		{
		case 0:
			return pSouiLayoutParam->posLeft.nPos.fSize;
		case 1:
			return pSouiLayoutParam->posTop.nPos.fSize;
		case 2:
			return pSouiLayoutParam->posRight.nPos.fSize;
		case 3:
			return pSouiLayoutParam->posBottom.nPos.fSize;
		default:
			return 0.f;
		}
	}

	void SMoveWnd::SetLayoutSize(SouiLayoutParamStruct *pSouiLayoutParam, int PosN, float value)
	{
		switch (PosN)
		{
		case 0:
			pSouiLayoutParam->posLeft.nPos.fSize = value;
			break;
		case 1:
			pSouiLayoutParam->posTop.nPos.fSize = value;
			break;
		case 2:
			pSouiLayoutParam->posRight.nPos.fSize = value;
			break;
		case 3:
		default:
			pSouiLayoutParam->posBottom.nPos.fSize = value;
			break;
		}
	}


	POS_INFO SMoveWnd::GetPosInfo(SouiLayoutParamStruct *pSouiLayoutParam, int PosN)
	{
		switch (PosN)
		{
		case 0:
			return pSouiLayoutParam->posLeft;
		case 1:
			return pSouiLayoutParam->posTop;
		case 2:
			return pSouiLayoutParam->posRight;
		case 3:
		default:
			return pSouiLayoutParam->posBottom;
		}
	}

}//namesplace soui