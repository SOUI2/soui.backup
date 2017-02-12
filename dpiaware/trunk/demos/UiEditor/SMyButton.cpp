#include "stdafx.h"
#include "SMyButton.h"

void SMyButton::OnMouseMove(UINT nFlags,CPoint pt)
{
	CRect rcCenter;
	CRect rc;
	CRect rcClient = GetWindowRect();
	rc.top = rcClient.bottom - 10;
	rc.bottom = rcClient.bottom;
	rc.left = rcClient.right - 10;
	rc.right = rcClient.right;

	rcClient.DeflateRect(10, 10, 10, 10);
	rcCenter = rcClient;

	if (rc.PtInRect(pt))
	{
		::SetCursor(m_hLUpRDown);


		if (nFlags & MK_LBUTTON)
		{
			SwndLayout *layout = GetLayout();

			int x = pt.x - Oldx;
			int y = pt.y - Oldy;

			Oldx = pt.x;
			Oldy = pt.y;

			//layout->pos[2].pit = PIT_SIZE;
			layout->pos[2].nPos = layout->pos[2].nPos + x; 

			//layout->pos[3].pit = PIT_SIZE;
			layout->pos[3].nPos = layout->pos[3].nPos + y; 
			GetParent()->UpdateChildrenPosition();		
		}
	}
	else if (rcCenter.PtInRect(pt))
	{
		::SetCursor(m_hAll);

		if (nFlags & MK_LBUTTON)
		{
			//btn_test->Move(10, 10, 100, 100);
			SwndLayout *layout = GetLayout();

			int x = pt.x - Oldx;
			int y = pt.y - Oldy;

			Oldx = pt.x;
			Oldy = pt.y;

			//layout->pos[2].pit = PIT_SIZE;
			layout->pos[0].nPos = layout->pos[0].nPos + x; 

			//layout->pos[3].pit = PIT_SIZE;
			layout->pos[1].nPos = layout->pos[1].nPos + y; 
			GetParent()->UpdateChildrenPosition();		
		}

	}
	else
	{

		::SetCursor(m_hNormal);
	}


	wchar_t a[50];
	swprintf_s(a, L"%d,%d:%d,%d", pt.x, pt.y, GetClientRect().BottomRight().x, GetClientRect().BottomRight().y);
	SetWindowText(a);
}

void SMyButton::OnLButtonDown(UINT nFlags,CPoint pt)
{
   SWindow::OnLButtonDown(nFlags, pt);
   Oldx = pt.x;
   Oldy = pt.y;

}

 BOOL SMyButton::OnSetCursor(const CPoint &pt)
{
	//if(!m_rcText.PtInRect(pt)) return FALSE;
	//HCURSOR hCursor=GETRESPROVIDER->LoadCursor(m_style.m_strCursor);
	//::SetCursor(hCursor);
	//return TRUE;
	return TRUE;
}
//
//BOOL SMyButton::OnNcHitTest(CPoint pt)
//{
//
//
//	CRect rect;
//	GetClientRect(&rect);
//	int nFrame=10;
//	rect.DeflateRect(nFrame,nFrame);
//	if (!rect.PtInRect(pt))
//	{ 
//		//         if (pt.x<=nFrame && pt.y>=rect.bottom-nFrame)   
//		//         {
//		//             //            return HTBOTTOMLEFT;    //左下角拖动大小
//		//         }
//		//         else if (pt.x<=nFrame && pt.y<=nFrame)
//		//         {   
//		//             //            return HTTOPLEFT;        //左上角拖动大小
//		//         }
//		//         else if (pt.x>=rect.right-nFrame && pt.y<=nFrame)
//		//         {
//		//             //            return HTTOPRIGHT;        //右上角拖动大小
//		//         }
//		if (pt.x>=rect.right-nFrame && pt.y>=rect.bottom-nFrame)
//		{
//			return HTBOTTOMRIGHT;    //右下角拖动大小
//			//HitFlag=1;
//		}
//		//         else if (pt.x<=nFrame)
//		//         {
//		//             //            return HTLEFT;            //屏蔽向左拖动大小
//		//         }
//		//         else if (pt.y<=nFrame)
//		//         {
//		//            
//		//             //            return HTTOP;            //屏蔽向上拖动大小
//		//         }
//		else if (pt.y>=rect.bottom-nFrame)
//		{
//			//DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
//			//if ((style & WS_HSCROLL)==0)    //未出现垂直滚动条
//			//{
//				//HitFlag=1;
//				return HTBOTTOM;        //向右拖动大小
//			//}   
//		}
//		else if (pt.x>=rect.right-nFrame)
//		{
//			//DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
//			//if ((style & WS_VSCROLL)==0)    //未出现水平滚动条
//			//{
//				//HitFlag=1;
//				return HTRIGHT;        //向下拖动大小
//			//}   
//		}
//	}
//	//     else
//	//     {
//	//         //        return HTCAPTION;    //拖动位置
//	//     }
//
//	return SWindow::OnNcHitTest(pt);
//}





 //SAutoLock lock(m_cs);
 //if(!_tcsicmp(pszCursorName,_T("arrow")))  //普通箭头
	// return IDC_ARROW;
 //if(!_tcsicmp(pszCursorName,_T("ibeam"))) //输入光标
	// return IDC_IBEAM;
 //if(!_tcsicmp(pszCursorName,_T("wait")))  
	// return IDC_WAIT;
 //if(!_tcsicmp(pszCursorName,_T("cross")))
	// return IDC_CROSS;
 //if(!_tcsicmp(pszCursorName,_T("uparrow")))  //向上的箭头
	// return IDC_UPARROW;
 //if(!_tcsicmp(pszCursorName,_T("size")))  //不见了
	// return IDC_SIZE;
 //if(!_tcsicmp(pszCursorName,_T("sizenwse")))  //左上右下
	// return IDC_SIZENWSE;
 //if(!_tcsicmp(pszCursorName,_T("sizenesw"))) //左下右上
	// return IDC_SIZENESW;
 //if(!_tcsicmp(pszCursorName,_T("sizewe")))  //东西
	// return IDC_SIZEWE;
 //if(!_tcsicmp(pszCursorName,_T("sizens")))  //南北
	// return IDC_SIZENS;
 //if(!_tcsicmp(pszCursorName,_T("sizeall")))  //东西南北
	// return IDC_SIZEALL;
 //if(!_tcsicmp(pszCursorName,_T("no")))
	// return IDC_NO;
 //if(!_tcsicmp(pszCursorName,_T("hand")))
	// return IDC_HAND;
 //if(!_tcsicmp(pszCursorName,_T("help")))
	// return IDC_HELP;
 //return NULL;