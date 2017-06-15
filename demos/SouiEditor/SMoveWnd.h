#pragma once
#include "DesignerView.h"

namespace SOUI
{

	class SMoveWnd : public SUIWindow
	{
		SOUI_CLASS_NAME(SMoveWnd, L"movewnd")
	public:
		SMoveWnd(void);
		~SMoveWnd(void);
	protected:
		void OnLButtonDown(UINT nFlags,CPoint pt);

		void OnLButtonUp(UINT nFlags,CPoint pt);

		void OnMouseMove(UINT nFlags,CPoint pt);
		void OnPaint(IRenderTarget *pRT);

		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MSG_WM_LBUTTONUP(OnLButtonUp)
			MSG_WM_MOUSEMOVE(OnMouseMove)
		    MSG_WM_PAINT_EX(OnPaint)   
			MSG_WM_KEYDOWN(OnKeyDown)
			MSG_WM_KEYUP(OnKeyUp)
		SOUI_MSG_MAP_END()

	public:
		void AdjustRect(); //调整8个点的位置;
		BOOL IsSelect();   //是否被选中
		void NewWnd(CPoint pt);
		void Click(UINT nFlags,CPoint pt);

        void MoveWndSize(int x, int PosN);   //拉动右边框或下边框
		void MoveWndSizeLT(int x, int PosN); //拉动左边框或上边框

		void MoveWndSize_Linear(int x , ORIENTATION orientation); //线性布局 拉动右边框或下边框

		void MoveWndHorz(int x);
		void MoveWndVert(int x);

		float GetLayoutSize(SouiLayoutParamStruct *pSouiLayoutParam, int PosN);
		void SetLayoutSize(SouiLayoutParamStruct *pSouiLayoutParam, int PosN, float value);

		POS_INFO GetPosInfo(SouiLayoutParamStruct *pSouiLayoutParam, int PosN);

	protected:

		//SOUI_ATTRS_BEGIN()
		//	SOUI_ATTRS_END()

		INT Oldx;
		INT Oldy;

		HCURSOR m_hLUpRDown;
		HCURSOR m_hAll;
		HCURSOR m_hNormal;

		//移动元素
		int      m_downIndex; //拖动选择元素 -1空 0在空处 1左上角 2上 3右上角 4右 5右下角 6下 7左下角 8左 9中间
		POINT    m_downPt;   //按下相对位置
		CRect     m_rcPos1,m_rcPos2,m_rcPos3,m_rcPos4,m_rcPos5,m_rcPos6,m_rcPos7,m_rcPos8; //八个角点,从左上角顺时针旋转

		CRect m_rcCenter; //拖动位置

		//拖动窗口
		int m_downWindow;  // 0:未按下,4:按在右边框,5.按在右下角, 6:按在下边框

		INT m_StateMove;	   //控件大小位置是否被改变;

	public:
		SDesignerView *m_Desiner;
		SUIWindow *m_pRealWnd;  //与这个窗口相关的实际窗口
		BOOL m_bCtrlShift;
	};

}
