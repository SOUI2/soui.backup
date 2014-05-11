#pragma once

#include "SimpleWnd.h"
#include "MemDC.h"

namespace SOUI
{

class CDragWnd : public CSimpleWnd
{
public:
	CDragWnd(void);
	~CDragWnd(void);

	static BOOL BeginDrag(HBITMAP hBmp,POINT ptHot ,COLORREF crKey, BYTE byAlpha,DWORD dwFlags);
	static void DragMove(POINT pt);
	static void EndDrag();
protected:

	void OnPaint(CDCHandle dc);

	BEGIN_MSG_MAP_EX(CDragWnd)
		MSG_WM_PAINT(OnPaint)
	END_MSG_MAP()

	CPoint m_ptHot;
	CMemDC	m_memdc;

	static CDragWnd * s_pCurDragWnd;
};

}//end of namespace
