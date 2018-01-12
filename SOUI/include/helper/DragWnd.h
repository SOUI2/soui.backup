#pragma once

#include "core/SimpleWnd.h"
#include "helper/MemDC.h"

namespace SOUI
{
	/**
	* @class      CDragWnd
	* @brief      实现一个拖动窗口HOST	
	*/
class CDragWnd : public CSimpleWnd
{
public:
    CDragWnd(void);
    ~CDragWnd(void);
	/**
	* BeginDrag
	* @brief    开始拖动窗口
	* @param    HBITMAP hBmp --  拖动时在拖动窗口上显示的位图
	* @param    POINT ptHot --  拖动开始的位置
	* @param    COLORREF crKey --  透明色（给定位图为32位时此参数无效）
	* @param    BYTE byAlpha --  窗口透明度【0-255】
	* @param    DWORD dwFlags -- 当不是位图不是32位时给SetLayeredWindowAttributes的参数即LWA_ALPHA时：crKey参数无效，bAlpha参数有效；LWA_COLORKEY窗体中的所有颜色为crKey的地方将变为透明，bAlpha参数无效。其常量值为1。
	*/
    static BOOL BeginDrag(HBITMAP hBmp,POINT ptHot ,COLORREF crKey, BYTE byAlpha,DWORD dwFlags);
	/**
	* DragMove
	* @brief    拖动窗口到指定位置
	* @param    POINT pt --  拖动的目标位置
	* @note		拖动位置为屏幕坐标，注意转换
	*/
    static void DragMove(POINT pt);
	/**
	* EndDrag
	* @brief    结束拖动
	*/
    static void EndDrag();
protected:

    void OnPaint(HDC dc);

    BEGIN_MSG_MAP_EX(CDragWnd)
        MSG_WM_PAINT(OnPaint)
    END_MSG_MAP()

    CPoint m_ptHot;
    HBITMAP m_bmp;

    static CDragWnd * s_pCurDragWnd;
};

}//end of namespace
