/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       SSplitWnd.h
 * @brief      
 * @version    v1.0      
 * @author     soui      
 * @date       2014-07-08
 * 
 * Describe     
 */
#pragma once
#include "core/SPanel.h"

namespace SOUI
{

enum SPLITMODE {SM_COL=0,SM_ROW};


/** 
 * @class     SSplitPane
 * @brief     分割窗口
 *
 * Describe   分割窗口
 */
class SOUI_EXP SSplitPane : public SWindow
{
    friend class SSplitWnd;
    SOUI_CLASS_NAME(SSplitPane, L"splitpane")
public:
    
    /**
     * SSplitPane::SSplitPane
     * @brief    构造函数
     *
     * Describe  构造函数  
     */
    SSplitPane();

    /**
     * SSplitPane::~SSplitPane
     * @brief    析构函数
     *
     * Describe  析构函数  
     */

    virtual ~SSplitPane() {}

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"idealsize", m_nSizeIdeal, TRUE)
        ATTR_INT(L"minsize", m_nSizeMin, TRUE)
        ATTR_INT(L"priority", m_nPriority, TRUE)
    SOUI_ATTRS_END()
protected:
    int m_nSizeIdeal;  /**< 理想大小 */
    int m_nSizeMin;    /**< 最小大小 */
    int m_nPriority;   /**< 优先级   */
};

/** 
 * @class     SSplitWnd
 * @brief     分割窗口
 *
 * Describe   分割窗口
 */
class SOUI_EXP SSplitWnd :
    public SWindow
{
    SOUI_CLASS_NAME(SSplitWnd, L"splitwnd")

    enum {
        layout_vert=1,        //纵向布局改变
        layout_horz=2,        //横向布局改变
        layout_pos=4,        //窗口位置发生改变
    };
    struct PANEORDER
    {
        int idx;
        SSplitPane *pPane;
    };
public:
    
    /**
     * SSplitWnd::SSplitWnd
     * @brief    构造函数
     *
     * Describe  构造函数  
     */
    SSplitWnd(void);

    /**
     * SSplitWnd::~SSplitWnd
     * @brief    析构函数
     *
     * Describe  析构函数  
     */
    virtual ~SSplitWnd(void);

    /**
     * SSplitWnd::SetPaneInfo
     * @brief    设置panel信息
     * @param    UINT iPane  --  panel id
     * @param    int nIdealSize  -- 理想大小
     * @param    int nMinSize    -- 最小大小
     * @param    int nPriority   -- 优先级
     * @return   返回BOOL     
     *
     * Describe  设置panel信息  
     */
    BOOL SetPaneInfo(UINT iPane,int nIdealSize,int nMinSize,int nPriority);

    /**
     * SSplitWnd::GetPaneInfo
     * @brief    获取panel信息
     * @param    UINT iPane  --  panel id
     * @param    int *pnIdealSize  -- 理想大小
     * @param    int *pnMinSize    -- 最小大小
     * @param    int *pnPriority   -- 优先级
     * @return   返回BOOL     
     *
     * Describe  获取panel信息 
     */
    BOOL GetPaneInfo(UINT iPane,int *pnIdealSize,int *pnMinSize,int *pnPriority);

    /**
     * SSplitWnd::ShowPanel
     * @brief    显示panel
     * @param    UINT iPane -- panel id     
     * @return   返回BOOL     
     *
     * Describe  显示panel  
     */
    BOOL ShowPanel(UINT iPane);

    /**
     * SSplitWnd::HidePanel
     * @brief    隐藏panel
     * @param    UINT iPane -- panel id
     * @return   返回BOOL
     *
     * Describe  隐藏panel  
     */
    BOOL HidePanel(UINT iPane);

protected:
    
    /**
     * SSplitWnd::UpdateChildrenPosition
     * @brief    更新子窗口位置
     *
     * Describe  更新子窗口位置
     */
    virtual void UpdateChildrenPosition(){}//empty
    
    /**
     * SSplitWnd::GetVisiblePanelCount
     * @brief    获取显示可见panel个数
     * @return   返回 int
     *
     * Describe  获取显示可见panel个数
     */
    int GetVisiblePanelCount();
    
    /**
     * SSplitWnd::GetNextVisiblePanel
     * @brief    获取下一个 panel-id
     * @param    UINT iPanel -- panel id
     * @return   返回int
     *
     * Describe  获取下一个 panel-id 
     */
    int GetNextVisiblePanel(UINT iPanel);
    
    /**
     * SSplitWnd::CreateChildren
     * @brief    创建panel
     * @param    pugi::xml_node xmlNode -- xml文件
     * @return   返回BOOL
     *
     * Describe  创建panel 
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);
    
    /**
     * SSplitWnd::OnSetCursor
     * @brief    设置坐标
     * @param    const CPoint &pt -- 坐标
     * @return   返回BOOL
     *
     * Describe  设置坐标
     */
    virtual BOOL OnSetCursor(const CPoint &pt);
    
    /**
     * SSplitWnd::OnDestroy
     * @brief    销毁
     *
     * Describe  销毁  
     */
    void OnDestroy();
    
    /**
     * SSplitWnd::OnPaint
     * @brief    绘制
     * @param    IRenderTarget * pRT -- 绘制设备
     *
     * Describe  绘制 
     */
    void OnPaint(IRenderTarget * pRT);
    
    /**
     * SSplitWnd::OnWindowPosChanged
     * @brief    修改窗口位置
     * @param    LPRECT lpWndPos -- 窗口位置
     *
     * Describe  修改窗口位置  消息响应函数
     */
    LRESULT OnWindowPosChanged(LPRECT lpWndPos);
    
    /**
     * SSplitWnd::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags,CPoint pt);

    /**
     * SSplitWnd::OnLButtonUp
     * @brief    左键抬起事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonUp(UINT nFlags,CPoint pt);

    /**
     * SSplitWnd::OnMouseMove
     * @brief    鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags,CPoint pt);

    /**
     * SSplitWnd::FunComp
     * @brief    比较函数
     * @param    const void * p1 -- 参数1
     * @param    const void * p2 -- 参数2
     *
     * Describe  此函数是消息响应函数
     */
    static int FunComp(const void * p1,const void * p2);

    /**
     * SSplitWnd::Relayout
     * @brief    重新布局
     * @param    UINT uMode -- 
     *
     * Describe  重新布局
     */
    void Relayout(UINT uMode);

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"sepsize", m_nSepSize, TRUE)
        ATTR_INT(L"adjustable", m_bAdjustable, TRUE)
        ATTR_INT(L"colmode", m_bColMode, TRUE)
        ATTR_SKIN(L"skinsep",m_pSkinSep,TRUE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_WINPOSCHANGED_EX(OnWindowPosChanged)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    SOUI_MSG_MAP_END()

protected:
    SArray<SSplitPane *> m_arrPane; /**< 保存panel */
    ISkinObj *m_pSkinSep;   /**< ISkinObj对象 */
    int m_nSepSize;         /**<  */
    BOOL m_bAdjustable;     /**<  */
    BOOL m_bColMode;        /**<  */

    int m_iDragBeam;        /**<  */
    CPoint m_ptClick;       /**<  */
};

/** 
 * @class     SSplitWnd_Col
 * @brief     垂直分割窗口
 *
 * Describe   垂直分割窗口
 */
class SOUI_EXP SSplitWnd_Col : public SSplitWnd
{
    SOUI_CLASS_NAME(SSplitWnd_Col, L"splitcol")
public:
    SSplitWnd_Col()
    {
        m_bColMode=TRUE;
    }
};

/** 
 * @class     SSplitWnd_Row
 * @brief     横向分割窗口
 *
 * Describe   横向分割窗口
 */
class SOUI_EXP SSplitWnd_Row : public SSplitWnd
{
    SOUI_CLASS_NAME(SSplitWnd_Row, L"splitrow")
public:
    SSplitWnd_Row()
    {
        m_bColMode=FALSE;
    }
};

}//namespace SOUI
