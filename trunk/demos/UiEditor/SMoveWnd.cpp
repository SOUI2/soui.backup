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
		m_hLUpRDown=GETRESPROVIDER->LoadCursor(L"sizenwse");
		m_hAll=GETRESPROVIDER->LoadCursor(L"sizeall");
		m_hNormal=GETRESPROVIDER->LoadCursor(m_style.m_strCursor);
		m_downWindow = 0;
		m_bFocusable = TRUE;
		m_pRealWnd = NULL;
		StateMove = 0;

	}

	SMoveWnd::~SMoveWnd()
	{
	}



	void SOUI::SMoveWnd::OnPaint( IRenderTarget *pRT )
	{
		//__super::OnPaint(pRT);



		if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
		{
			CRect rectR;
			CRect rectRP;
			m_pRealWnd->GetWindowRect(rectR);
			m_pRealWnd->GetParent()->GetWindowRect(rectRP);
			SwndLayout *pMoveWndLayout = GetLayout();
			pMoveWndLayout->pos[0].nPos = 20;
			pMoveWndLayout->pos[1].nPos = 20;
			pMoveWndLayout->pos[2].nPos = rectR.right - rectR.left;
			pMoveWndLayout->pos[3].nPos = rectR.bottom - rectR.top;

			GetParent()->UpdateChildrenPosition();
		}
		else
		{

				//更新MoveWnd的位置和RealWnd一样
				CRect rectR;
				CRect rectRP;
				m_pRealWnd->GetWindowRect(rectR);
				m_pRealWnd->GetParent()->GetWindowRect(rectRP);
				SwndLayout *pMoveWndLayout = GetLayout();
				pMoveWndLayout->pos[0].nPos = rectR.left - rectRP.left;
				pMoveWndLayout->pos[1].nPos = rectR.top - rectRP.top;
				pMoveWndLayout->pos[2].nPos = rectR.right - rectR.left;
				pMoveWndLayout->pos[3].nPos = rectR.bottom - rectR.top;

				GetParent()->UpdateChildrenPosition();
		}
		
		CRect rect;
		GetWindowRect(rect);


		//if (!IsSelect() && m_Desiner->m_pMoveWndRoot != this)
		//if (!IsSelect() )
		//{
		//	return;
		//}

		SPainter painter;
		AdjustRect();

		BeforePaint(pRT, painter);

		
		int n = POINT_SIZE/2;
		//rect.DeflateRect(n,n,n,n);

		CAutoRefPtr<IPen> pen,oldpen;


		if (IsSelect() )
		{
			pRT->CreatePen(PS_SOLID,RGBA(255,0,0,0),1,&pen);
			pRT->SelectObject(pen,(IRenderObj**)&oldpen);
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
			//pRT->CreatePen(PS_SOLID,RGBA(234,128,16,00),1,&pen);
			pRT->CreatePen(PS_SOLID,RGB(172,172,172),1,&pen);
			pRT->SelectObject(pen,(IRenderObj**)&oldpen);

			pRT->DrawRectangle(rect);
			pRT->SelectObject(oldpen);
		}

		AfterPaint(pRT, painter);
	}

	void SMoveWnd::OnLButtonDown(UINT nFlags,CPoint pt)
	{
		if(m_Desiner->m_nState == 1)//如果是创建控件状态
		{


			//鼠标按下时创建控件
            NewWnd(pt);
			m_Desiner->CreatePropGrid(m_Desiner->m_xmlNode.name());
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			OnLButtonUp(nFlags, pt);
			return;
		}

		if (m_pRealWnd != m_Desiner->m_pRealWndRoot)
		{
			m_Desiner->SetCurrentCtrl(m_Desiner->FindNodeByAttr(m_Desiner->m_CurrentLayoutNode, L"name", m_pRealWnd->GetName()), this);
			m_Desiner->CreatePropGrid(m_Desiner->m_xmlNode.name());
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		}else
		{
			m_Desiner->SetCurrentCtrl(m_Desiner->m_CurrentLayoutNode, this);	
			m_Desiner->CreatePropGrid(_T("hostwnd"));
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		}




		SWindow::OnLButtonDown(nFlags, pt);

		Oldx = pt.x;
		Oldy = pt.y;

		if(m_rcPos8.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZEWE));
			m_downWindow=8;

		}else 

		if(m_rcPos1.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
			m_downWindow=1;

		}else 
		if(m_rcPos2.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZENS));
			m_downWindow=2;

		}else 
		if(m_rcPos3.PtInRect(pt))
		{
			//SetCursor(LoadCursor(NULL,IDC_SIZEWE));
			m_downWindow=3;

		}else 
		if(m_rcPos4.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZEWE));
			m_downWindow=4;

		}else if(m_rcPos5.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
			m_downWindow=5;

		}else if(m_rcPos6.PtInRect(pt))
		{
			SetCursor(LoadCursor(NULL,IDC_SIZENS));
			m_downWindow=6;
		}else if(m_rcCenter.PtInRect(pt))
		{
			//SetCursor(LoadCursor(NULL,IDC_SIZEALL));
			m_downWindow=9;
		}
	}

	void SMoveWnd::OnLButtonUp(UINT nFlags,CPoint pt)
	{
		SWindow::OnLButtonUp(nFlags,pt);

		//将控件的位置更新到xml节点;

		if (StateMove)
		{
			//if (m_pRealWnd != m_Desiner->m_pRealWndRoot)
			//{
				m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
				m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
				StateMove = 1;
			//}
		}


		m_downWindow = 0;

	}

	void SMoveWnd::OnMouseMove(UINT nFlags,CPoint pt)
	{
		if (!IsSelect())
		{
			return;
		}



		if(0==m_downWindow) //当前控件没有被按下
		{
			//如果当前只选择一个控件，且为当前控件
			//if( mainWnd.m_designerView.m_CurSelCtrlList.size() == 1 && mainWnd.m_designerView.m_CurSelCtrlList[0]==this)
			//{
			//改变光标
			if(m_rcPos1.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
			}else if(m_rcPos2.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZENS));
			}else if(m_rcPos3.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZENESW));
			}else  if(m_rcPos4.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZEWE));
			}else if(m_rcPos5.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
			}else if(m_rcPos6.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZENS));
			}else if(m_rcPos7.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZENESW));
			}else  if(m_rcPos8.PtInRect(pt))
			{
				SetCursor(LoadCursor(NULL,IDC_SIZEWE));
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

			if (abs(pt.x - Oldx) >= 8  || abs(pt.y - Oldy) >= 8)
			{
				SwndLayout *layout = GetLayout();
				SwndLayout *layout1 = m_pRealWnd->GetLayout();

				x1 = abs(pt.x - Oldx);
				x1 = x1 / 8;

				y1 = abs(pt.y - Oldy);
				y1 = y1 / 8;

				if (abs(pt.x - Oldx) >= 8)
				{


					if (x > 0)
					{
						x = 8;
					}else
					{
						x = -8;
					}

					bx = TRUE;

				}else
				{
					x = 0;
				}

				if (abs(pt.y - Oldy) >= 8)
				{
					if (y > 0)
					{
						y = 8;
					}else
					{
						y = -8;
					}

					by = TRUE;


				}else
				{
					y = 0;
				}



			}else
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
						MoveWndSizeLT(x*x1, HORZ_LT);
					}
				}
				break;;

            /*左上角*/
			case 1:
				{
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
						MoveWndSizeLT(y*y1, VERT_LT);
					}
				}
				break;;
            
			/*右上角*/
			case 3:
				return;

            /*右边框*/
			case 4:
				{
					if (bx)
					{
						MoveWndSize(x*x1, HORZ);
					}
				}
				break;

            /*右下角*/
			case 5:
				{
					if (bx)
					{
						MoveWndSize(x*x1, HORZ);
					}
					if (by)
					{
						MoveWndSize(y*y1, VERT);
					}
				}
				break;

			/*下边框*/
			case 6:
				{
					if (by)
					{
						MoveWndSize(y*y1, VERT);
					}
				}
				break;

            /*左下角*/
			case 7:
				return;

            /*中间拖动*/
			case 9:
				{
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

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
	
			StateMove = 1;
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);

	    	GetParent()->Invalidate(); //刷新父窗口


		}

	}


	void SMoveWnd::AdjustRect()
	{
		CRect rect;
		GetWindowRect(rect);





		m_rcPos1.left=rect.left;
		m_rcPos1.top=rect.top;
		m_rcPos1.right=rect.left + POINT_SIZE;
		m_rcPos1.bottom=rect.top + POINT_SIZE;

		m_rcPos2.left=rect.left + (rect.right - rect.left)/2 -POINT_SIZE/2;
		m_rcPos2.top=rect.top;
		m_rcPos2.right=m_rcPos2.left+POINT_SIZE;
		m_rcPos2.bottom=rect.top + POINT_SIZE;

		m_rcPos3.left=rect.right-POINT_SIZE;
		m_rcPos3.top=rect.top;
		m_rcPos3.right=rect.right;
		m_rcPos3.bottom=rect.top + POINT_SIZE;

		m_rcPos4.left=rect.right-POINT_SIZE;
		m_rcPos4.top=rect.top + (rect.bottom - rect.top)/2 -POINT_SIZE/2;
		m_rcPos4.right=rect.right;
		m_rcPos4.bottom=m_rcPos4.top+POINT_SIZE;

		m_rcPos5.left=rect.right-POINT_SIZE;
		m_rcPos5.top=rect.bottom-POINT_SIZE;
		m_rcPos5.right=rect.right;
		m_rcPos5.bottom=rect.bottom;

		m_rcPos6.left=m_rcPos2.left;
		m_rcPos6.top=rect.bottom-POINT_SIZE;
		m_rcPos6.right=m_rcPos2.right;
		m_rcPos6.bottom=rect.bottom;

		m_rcPos7.left=rect.left;
		m_rcPos7.top=rect.bottom-POINT_SIZE;
		m_rcPos7.right=rect.left + POINT_SIZE;
		m_rcPos7.bottom=rect.bottom;

		m_rcPos8.left=rect.left;
		m_rcPos8.top=m_rcPos4.top;
		m_rcPos8.right=rect.left + POINT_SIZE;
		m_rcPos8.bottom=m_rcPos4.bottom;

		m_rcCenter.left=rect.left + POINT_SIZE;
		m_rcCenter.top=rect.top + POINT_SIZE;
		m_rcCenter.right=rect.right - POINT_SIZE;
		m_rcCenter.bottom=rect.bottom - POINT_SIZE;

	}

	BOOL SMoveWnd::IsSelect()
	{
		if (m_Desiner == NULL)
			return FALSE;

		//if (m_Desiner->m_CurSelCtrlList.GetCount() == 1)
		//{
			if (m_Desiner->m_CurSelCtrl == this)  //
			{
				return TRUE;
			}
		//}

		return FALSE;
	}


void SMoveWnd::MoveWndHorz(int x)
{
	//水平移动


	if (m_pRealWnd == m_Desiner->m_pRealWndRoot)
	{
		return;
	}

	SwndLayout *layout = GetLayout();
	SwndLayout *layout1 = m_pRealWnd->GetLayout();


	//有margin的情况
	SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
	int nMargin = 0;

	nMargin = style.m_rcMargin.left;

	//往左拖动，left不能小于0
	if (layout->pos[0].nPos + x - nMargin < 0 && x < 0)
	{
		return;
	}


	//往右拖动，right不能大于父控件的right
	if (x > 0)
	{
		CRect r;
		GetParent()->GetWindowRect(r);

		if (layout->pos[0].nPos + layout->GetSpecifySize(PD_X) + x + style.m_rcMargin.right > r.right - r.left)
		{
			return;
		}
	}



	if (layout1->nCount == 2) //两个坐标的情况
	{
		if (layout1->pos[0].cMinus == -1)// 坐标1为负数的情况
		{
			if (layout1->pos[0].nPos - x<0)
			{
				return ;
			}

			layout->pos[0].nPos = layout->pos[0].nPos + x;
			layout1->pos[0].nPos = layout1->pos[0].nPos - x;

		}
		else//坐标1为正数
		{
			if (layout1->pos[2].cMinus == -1)
			{
				if (layout1->pos[2].nPos - x <0)
				{
					return;
				}
			}

			layout->pos[0].nPos = layout->pos[0].nPos + x;
			layout1->pos[0].nPos = layout1->pos[0].nPos + x;
		}
	}else if (layout1->nCount == 4)
	{


		int nPosTop, nPosTop1;
		int nPosButtom, nPosButtom1;



		/************************* 移动top点 **************************************/

		if (layout1->pos[0].cMinus == -1)// 坐标1为负数的情况
		{
			if (layout1->pos[0].nPos - x<0)
			{
				return ;
			}

			nPosTop = layout->pos[0].nPos + x; //layout->pos[1].nPos = layout->pos[1].nPos + x;   
			nPosTop1 = layout1->pos[0].nPos - x; //layout1->pos[1].nPos = layout1->pos[1].nPos - x;

		}
		else//坐标1为正数
		{
			if (layout1->pos[2].cMinus == -1)
			{
				if (layout1->pos[2].nPos - x <0)
				{
					return;
				}
			}

			nPosTop = layout->pos[0].nPos + x;    //layout->pos[1].nPos = layout->pos[1].nPos + x;
			nPosTop1 = layout1->pos[0].nPos + x;  //layout1->pos[1].nPos = layout1->pos[1].nPos + x;
		}

		/************************* 移动 top **************************************/




		/************************* 移动 buttom  **************************************/
		if (layout1->pos[2].pit == PIT_SIZE)  //100, 100 ,@5, @5这种情况
		{
			//

			layout->pos[0].nPos = nPosTop;
			layout1->pos[0].nPos = nPosTop1;

		}else
		{
			if (layout1->pos[2].cMinus == -1)// 坐标1为负数的情况
			{
				if (layout1->pos[2].nPos - x <0)
				{
					return ;
				}

				nPosButtom = layout->pos[2].nPos + x;  //layout->pos[3].nPos = layout->pos[3].nPos + x;
				nPosButtom1 = layout1->pos[2].nPos - x; //layout1->pos[3].nPos = layout1->pos[3].nPos - x;

			}
			else//坐标1为正数
			{
				nPosButtom = layout->pos[2].nPos + x; //layout->pos[3].nPos = layout->pos[3].nPos + x;
				nPosButtom1 = layout1->pos[2].nPos + x; //layout1->pos[3].nPos = layout1->pos[3].nPos + x;
			}


			layout->pos[0].nPos = nPosTop;
			layout1->pos[0].nPos = nPosTop1;
			layout->pos[2].nPos = nPosButtom;
			layout1->pos[2].nPos = nPosButtom1; 

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

	SwndLayout *layout = GetLayout();
	SwndLayout *layout1 = m_pRealWnd->GetLayout();

	////有margin的情况
	//SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
	//int nMargin = 0;

	//if (PosN == HORZ)
	//{
	//	nMargin = style.m_rcMargin.right;
	//}else
	//{
	//	nMargin = style.m_rcMargin.bottom;
	//}

	//有margin的情况
	SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
	int nMargin = 0;

	nMargin = style.m_rcMargin.top;

	//往上拖动，top不能小于0
	if (layout->pos[1].nPos + x - nMargin < 0 && x < 0)
	{
		return;
	}


	//往下拖动，bottom不能大于父控件的bottom
	if (x > 0)
	{
		CRect r;
		GetParent()->GetWindowRect(r);

		if (layout->pos[1].nPos + layout->GetSpecifySize(PD_Y) + x + style.m_rcMargin.bottom > r.bottom - r.top)
		{
			return;
		}
	}

	if (layout1->nCount == 2) //两个坐标的情况
	{
		if (layout1->pos[1].cMinus == -1)// 坐标1为负数的情况
		{
			if (layout1->pos[1].nPos - x<0)
			{
				return ;
			}

			layout->pos[1].nPos = layout->pos[1].nPos + x;
			layout1->pos[1].nPos = layout1->pos[1].nPos - x;

		}
		else//坐标1为正数
		{
			if (layout1->pos[1 + 2].cMinus == -1)
			{
				if (layout1->pos[1 + 2].nPos - x <0)
				{
					return;
				}
			}

			layout->pos[1].nPos = layout->pos[1].nPos + x;
			layout1->pos[1].nPos = layout1->pos[1].nPos + x;
		}
	}else if (layout1->nCount == 4)
	{


		int nPosTop, nPosTop1;
		int nPosButtom, nPosButtom1;


		/************************* 移动top点 **************************************/

		if (layout1->pos[1].cMinus == -1)// 坐标1为负数的情况
		{
			if (layout1->pos[1].nPos - x<0)
			{
				return ;
			}

			nPosTop = layout->pos[1].nPos + x; //layout->pos[1].nPos = layout->pos[1].nPos + x;   
			nPosTop1 = layout1->pos[1].nPos - x; //layout1->pos[1].nPos = layout1->pos[1].nPos - x;

		}
		else//坐标1为正数
		{
			if (layout1->pos[3].cMinus == -1)
			{
				if (layout1->pos[3].nPos - x <0)
				{
					return;
				}
			}

			nPosTop = layout->pos[1].nPos + x;    //layout->pos[1].nPos = layout->pos[1].nPos + x;
			nPosTop1 = layout1->pos[1].nPos + x;  //layout1->pos[1].nPos = layout1->pos[1].nPos + x;
		}

		/************************* 移动 top **************************************/




		/************************* 移动 buttom  **************************************/
		if (layout1->pos[3].pit == PIT_SIZE)  //100, 100 ,@5, @5这种情况
		{
			//

			layout->pos[1].nPos = nPosTop;
			layout1->pos[1].nPos = nPosTop1;

		}else
		{
			if (layout1->pos[3].cMinus == -1)// 坐标1为负数的情况
			{
				if (layout1->pos[3].nPos - x <0)
				{
					return ;
				}

				nPosButtom = layout->pos[3].nPos + x;  //layout->pos[3].nPos = layout->pos[3].nPos + x;
				nPosButtom1 = layout1->pos[3].nPos - x; //layout1->pos[3].nPos = layout1->pos[3].nPos - x;

			}
			else//坐标1为正数
			{
				nPosButtom = layout->pos[3].nPos + x; //layout->pos[3].nPos = layout->pos[3].nPos + x;
				nPosButtom1 = layout1->pos[3].nPos + x; //layout1->pos[3].nPos = layout1->pos[3].nPos + x;
			}


			layout->pos[1].nPos = nPosTop;
			layout1->pos[1].nPos = nPosTop1;
			layout->pos[3].nPos = nPosButtom;
			layout1->pos[3].nPos = nPosButtom1; 

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
	BOOL bCtrl  = (::GetKeyState(VK_CONTROL) < 0);


	if (bShift&&bCtrl)
	{
	    m_Desiner->ShowMovWndChild(FALSE, (SMoveWnd*)this->GetWindow(GSW_FIRSTCHILD));
		m_bCtrlShift = TRUE;
		return ;
	}else
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
			MoveWndVert(-1);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		    GetParent()->Invalidate(); //刷新父窗口

		}else if (bShift)
		{
			MoveWndSize(-1, VERT);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		    GetParent()->Invalidate(); //刷新父窗口

		}else
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
		    MoveWndHorz(-1);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			GetParent()->Invalidate(); //刷新父窗口

		}else if (bShift)
		{
			MoveWndSize(-1, HORZ);
		    m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		    GetParent()->Invalidate(); //刷新父窗口

		}else
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
			MoveWndVert(1);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			GetParent()->Invalidate(); //刷新父窗口


		}else if (bShift)
		{
			MoveWndSize(1, VERT);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
		    GetParent()->Invalidate(); //刷新父窗口


		}else
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
			MoveWndHorz(1);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			GetParent()->Invalidate(); //刷新父窗口


		}else if (bShift)
		{
			MoveWndSize(1, HORZ);
			m_bMsgHandled = TRUE;

			m_pRealWnd->GetParent()->UpdateChildrenPosition();	
			GetParent()->UpdateChildrenPosition();	
			m_Desiner->UpdatePosToXmlNode(m_pRealWnd, this);
			m_Desiner->UpdatePropGrid(m_Desiner->m_xmlNode);
			GetParent()->Invalidate(); //刷新父窗口


		}else
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

	SwndLayout *layout = GetLayout();
	SwndLayout *layout1 = m_pRealWnd->GetLayout();

	CRect rcReal, rcRealParent;
	m_pRealWnd->GetWindowRect(rcReal);
	m_pRealWnd->GetParent()->GetWindowRect(rcRealParent);


	//有margin的情况
	SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
	int nMargin = 0;
	
	if (PosN == HORZ_LT)
	{
	    nMargin = style.m_rcMargin.left;
	}else
	{
		nMargin = style.m_rcMargin.top;
	}

	//往左、上拖动大小，left、top不能小于0
	if (layout->pos[PosN].nPos + x - nMargin < 0 && x < 0)
	{
		return;
	}

	//往右拖动大小,left 不能大于 right
	if (layout->GetSpecifySize(PD_X) - x < 1 && PosN == 0 && x > 0)
	{
		return;
	}

	//往下拖动大小,top 不能大于 buttom
	if (layout->GetSpecifySize(PD_Y) - x < 1 && PosN == 1 && x > 0)
	{
		return;
	}

	layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
	if (PosN == 0)
	{
		layout->SetWidth(layout->GetSpecifySize(PD_X) - x);
	}else
	{
		layout->SetHeight(layout->GetSpecifySize(PD_Y) - x);
	}



	if(layout1->nCount == 2) //只有两个坐标点，自动计算大小
	{
		if (layout1->IsSpecifySize(PD_Y)||layout1->IsSpecifySize(PD_Y))//用size指定大小的时候
		{
		
			layout1->pos[PosN].nPos = layout1->pos[PosN].nPos + x;
			if (PosN==0)
			{   //宽
				layout1->SetWidth(layout1->GetSpecifySize(PD_X) - x);
			}else
			{   //高
				layout1->SetHeight(layout1->GetSpecifySize(PD_Y) - x);
			}
		}
		else
		{
			return;
		}
	}else
	{
		//-100, -100, @20,@20
		if (layout1->pos[PosN].cMinus == -1)// 坐标为负数的情况
		{

			layout1->pos[PosN].nPos = layout1->pos[PosN].nPos - x;
		}else
		{
			layout1->pos[PosN].nPos = layout1->pos[PosN].nPos + x;
		}
			
		if (layout1->pos[PosN + 2].pit == PIT_SIZE)  //@5这种情况
		{

			layout1->pos[PosN + 2].nPos = layout1->pos[PosN + 2].nPos - x;
			if (PosN==0)
			{
				layout1->SetWidth(layout1->GetSpecifySize(PD_X) - x);
			}else
			{
				layout1->SetHeight(layout1->GetSpecifySize(PD_Y) - x);
			}


		}else  //描点坐标
		{
			//80,100,50,80 这种情况，top < buttom  right < reft的暂时不考虑 
			//
			//layout1->pos[PosN].nPos = layout1->pos[PosN].nPos + x;
		}


	}

}


void SMoveWnd::MoveWndSize(int x, int PosN)
{
	SwndLayout *layout = GetLayout();
	SwndLayout *layout1 = m_pRealWnd->GetLayout();

	CRect rcReal, rcRealParent, rcMovParent;
	m_pRealWnd->GetWindowRect(rcReal);
	m_pRealWnd->GetParent()->GetWindowRect(rcRealParent);
	GetParent()->GetWindowRect(rcMovParent);


	//有margin的情况
	SwndStyle &style = m_pRealWnd->GetParent()->GetStyle();
	int nMargin = 0;

	if (PosN == HORZ)
	{
		nMargin = style.m_rcMargin.right;
	}else
	{
		nMargin = style.m_rcMargin.bottom;
	}

	//右拖动不能超过父控件的右边距
	if (PosN == 2 && x > 0)
	{
		if (layout->pos[PosN - 2].nPos + layout->GetSpecifySize(PD_X) + x + nMargin > rcMovParent.right - rcMovParent.left)
		{
			return;
		}
	}


	//下拖动不能超过父控件的下边距
	if (PosN == 3 && x > 0)
	{
		if (layout->pos[PosN - 2].nPos + layout->GetSpecifySize(PD_Y) + x + nMargin> rcMovParent.bottom - rcMovParent.top)
		{
			return;
		}	
	}


	//往左拖动大小,left 不能大于 right
	if (layout->GetSpecifySize(PD_X) + x < 1 && PosN == 2 && x < 0)
	{
		return;
	}

	//往上拖动大小,top 不能大于 buttom
	if (layout->GetSpecifySize(PD_Y) + x < 1 && PosN == 3 && x < 0)
	{
		return;
	}

	layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
	if (PosN == 0)
	{
		layout->SetWidth(layout->GetSpecifySize(PD_X) - x);
	}else
	{
		layout->SetHeight(layout->GetSpecifySize(PD_Y) - x);
	}



	if(layout1->nCount == 2) //只有两个坐标点，自动计算大小
	{
		if (layout1->IsSpecifySize(PD_Y)||layout1->IsSpecifySize(PD_Y))//用size指定大小的时候
		{


			layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
			if (PosN==2)
			{   //宽
				layout1->SetWidth(layout1->GetSpecifySize(PD_X) + x);
			}else
			{   //高
				layout1->SetHeight(layout1->GetSpecifySize(PD_Y) + x);
			}
		}
		else
		{
			return;
		}
	}else
	{
		//5,3,-5,-7
		//5,3,@5,@7
		if (layout1->pos[PosN].cMinus == -1)// 坐标3为负数的情况
		{

			layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
			layout1->pos[PosN].nPos = layout1->pos[PosN].nPos - x;
		}else if (layout1->pos[PosN].pit == PIT_SIZE)  //@5这种情况
		{

			layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
			layout1->pos[PosN].nPos = layout1->pos[PosN].nPos + x;
			if (PosN==2)
			{
				layout1->SetWidth(layout1->pos[PosN].nPos);
			}else
			{
				layout1->SetHeight(layout1->pos[PosN].nPos);
			}


		}else  //描点坐标
		{
			//80,100,50,80 这种情况，top < buttom  right < reft的暂时不考虑 

			layout->pos[PosN].nPos = layout->pos[PosN].nPos + x;
			layout1->pos[PosN].nPos = layout1->pos[PosN].nPos + x;
		}


	}
}


void SMoveWnd::NewWnd(CPoint pt)
{
    m_Desiner->NewWnd(pt, this);
}

void SMoveWnd::Click(UINT nFlags,CPoint pt)
{

    OnLButtonDown(nFlags, pt);
	OnLButtonUp(nFlags, pt);
}







}//namesplace soui