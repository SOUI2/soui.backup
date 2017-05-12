/* $Copyright (c) 2006-2017 Green Net World
 * 
 * Author	:	cherish  version 1.0
 * DateTime	:	2017-4-13 14:13:46
 *
 *
 * GSTabCtrl  功能
 */

#ifndef __GSTABCTRL_BFFA82AF_9970_4395_8F9C_CF25F8700172__
#define __GSTABCTRL_BFFA82AF_9970_4395_8F9C_CF25F8700172__

/** CGSTabCtrl */
class GSTabCtrl : public STabCtrl , protected ITimelineHandler
{
	SOUI_CLASS_NAME(GSTabCtrl, L"gstabctrl")

public:
	GSTabCtrl();     /** 默认构造函数 */
	~GSTabCtrl();    /** 默认析构函数 */


protected:

	/**
	* STabCtrl::OnPaint
	* @brief    绘画消息
	* @param    IRenderTarget *pRT -- 绘制设备句柄
	*
	* Describe  此函数是消息响应函数
	*/
	void OnPaint(IRenderTarget *pRT);

	/**
	* STabCtrl::DrawItem
	* @brief    绘制item
	* @param    IRenderTarget *pRT -- 绘制设备
	* @param    const CRect &rcItem -- 绘制区域
	* @param    int iItem  -- 索引
	* @param    DWORD dwState  -- 状态
	*
	* Describe  绘制item
	*/
	virtual void DrawItem(IRenderTarget *pRT, const CRect &rcItem, int iItem, DWORD dwState);

	/**
	* STabCtrl::OnMouseMove
	* @brief    鼠标移动事件
	* @param    UINT nFlags -- 标志
	* @param    CPoint point -- 鼠标坐标
	*
	* Describe  此函数是消息响应函数
	*/
	void OnMouseMove(UINT nFlags, CPoint point);

	/**
	* STabCtrl::OnMouseLeave
	* @brief    鼠标离开事件
	*
	* Describe  此函数是消息响应函数
	*/
	void OnMouseLeave()
	{
		OnMouseMove(0, CPoint(-1, -1));
	}


	virtual void OnNextFrame();
protected:

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_PAINT_EX(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
	SOUI_MSG_MAP_END()


	SOUI_ATTRS_BEGIN()
		ATTR_SKIN(L"anihover", m_pSkinAniHover, FALSE)
		ATTR_SKIN(L"anidown", m_pSkinAniDown, FALSE)
		ATTR_INT(L"tab_show_name", m_nTabShowName, FALSE)
	SOUI_ATTRS_END()

private:
	ISkinObj *					m_pSkinAniHover; /**< ISkibObj对象 */
	ISkinObj *					m_pSkinAniDown; /**< ISkibObj对象 */

	SArray<int>					m_arrHoverNumber;
	int							m_nElapseTime;
	int							m_nTabShowName;
};


#endif //__GSTABCTRL_BFFA82AF_9970_4395_8F9C_CF25F8700172__

