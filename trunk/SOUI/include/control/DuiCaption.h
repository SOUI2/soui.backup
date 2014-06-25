/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       DuiCaption.h
 * @brief      标题栏控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-05-28
 * 
 * Describe    此类完成标题栏控件
 */

#pragma once
#include "duiwnd.h"

namespace SOUI
{
/**
 * @class      CDuiCaption
 * @brief      标签类
 * 
 * Describe    标签类 只需要继承此类即可
 */
class SCaption :
    public SWindow
{
    SOUI_CLASS_NAME(SCaption, "caption")
public:
    /**
     * CDuiCaption::CDuiCaption
     * @brief    构造函数
     *
     * Describe  CDuiCaption类的构造函数
     */ 
    SCaption(void);
    /**
     * CDuiCaption::~CDuiCaption
     * @brief    析构函数
     *
     * Describe  ~CDuiCaption类的构造函数
     */     
    virtual ~SCaption(void);

protected:
    /**
     * CDuiCaption::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags, CPoint point);
    /**
     * CDuiCaption::OnLButtonDblClk
     * @brief    左键双击事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */    
    void OnLButtonDblClk(UINT nFlags, CPoint point);

    WND_MSG_MAP_BEGIN()
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
    WND_MSG_MAP_END()
};
}
