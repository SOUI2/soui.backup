#pragma once
#include "stdafx.h"

class SharkWinHandle
{
public:
	SharkWinHandle(void)
		: m_nIndex(0)
	{

	}

	virtual ~SharkWinHandle(void)
	{

	}
	
	// 初始化 
	void Init(HWND hWnd)
	{
		m_hWnd = hWnd;
		
	}
	 void UpdateSize(const CRect& rc)
	 {
		 m_rcWnd = rc;
	 }
	//  开始 
	void OnTimeSharkWin()
	{
		int ty = 2;
		
		int nLeft = m_rcWnd.left;
		int nTop = m_rcWnd.top;

		int nType = m_nIndex % 7;
		switch (nType)
		{
		case 0:					// 左 下
			nLeft -= ty;
			nTop += ty;
			break;
		case 1:					// 左
			nLeft -= ty;
			//nTop += ty;
			break;
		case 2:					// 左 上上
			nLeft -= ty;
			nTop -= 2 * ty;
			break;
		case 3:					// 上 上
			//nLeft -= ty;
			nTop -= 2 * ty;
			break;
		case 4:					// 右 右 上 上
			nLeft += 2 * ty;
			nTop -= 2 * ty;
			break;
		case 5:					// 右 右 上
			nLeft += 2 * ty;
			nTop -= ty;
			break;
		case 6:					// 右 右 下 
			nLeft += 2 * ty;
			nTop += ty;
			break;
		default:
			break;
		}
		
		::SetWindowPos(m_hWnd, NULL, nLeft, nTop, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW );

		++m_nIndex;
	}
	bool IsSharkDone()
	{
		if(35 == m_nIndex)
		{
			m_nIndex = 0;
			return true;
		}

		return false;
	}
protected:
	HWND				m_hWnd;
	CRect					m_rcWnd;
	UINT					m_nIndex;
};
