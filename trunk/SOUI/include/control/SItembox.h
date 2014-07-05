/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       SItembox.h
 * @brief      
 * @version    v1.0      
 * @author     soui      
 * @date       2014-07-03
 * 
 * Describe     
 */
#pragma once
#include "core/SPanel.h"

namespace SOUI
{

/**
 * @class      CDuiItemBox
 * @brief      ItemBox
 * 
 * Describe    ItemBox
 */
class SOUI_EXP CDuiItemBox
    : public SScrollView
{
    SOUI_CLASS_NAME(CDuiItemBox, L"itembox")
public:
    /**
     * CDuiItemBox::CDuiItemBox
     * @brief    构造函数
     *
     * Describe  构造函数  
     */
    CDuiItemBox();

    /**
     * CDuiItemBox::~CDuiItemBox
     * @brief    析构函数
     *
     * Describe  析构函数  
     */
    virtual ~CDuiItemBox() {}
    
    /**
     * CDuiItemBox::InsertItem
     * @brief    插入新项
     * @param    LPCWSTR pszXml -- xml配置文件
     * @param    int iItem -- 索引
     * @param    BOOL bEnsureVisible -- 是否显示
     * @return   返回SWindow
     *
     * Describe  插入新项  
     */
    SWindow* InsertItem(LPCWSTR pszXml,int iItem=-1,BOOL bEnsureVisible=FALSE);

    /**
     * CDuiItemBox::InsertItem
     * @brief    插入新项
     * @param    LPCWSTR pszXml -- xml配置文件
     * @param    int iItem -- 索引
     * @param    BOOL bEnsureVisible -- 是否显示
     * @return   返回SWindow
     *
     * Describe  插入新项  
     */
    SWindow* InsertItem(pugi::xml_node xmlNode, int iItem=-1,BOOL bEnsureVisible=FALSE);

    /**
     * CDuiItemBox::RemoveItem
     * @brief    删除项
     * @param    UINT iItem -- 索引
     * @return   返回BOOL
     *     
     * Describe  删除项  
     */
    BOOL RemoveItem(UINT iItem);

    /**
     * CDuiItemBox::RemoveItem
     * @brief    删除项
     * @param    SWindow * pChild -- 窗口节点
     * @return   返回BOOL
     *
     * Describe  删除项  
     */
    BOOL RemoveItem(SWindow * pChild);

    /**
     * CDuiItemBox::SetNewPosition
     * @brief    析构函数
     * @param    SWindow * pChild -- 节点
     * @param    DWORD nPos -- 位置
     * @param    BOOL bEnsureVisible -- 是否显示
     * @return   返回BOOL
     *     
     * Describe  析构函数  
     */
    BOOL SetNewPosition(SWindow * pChild, DWORD nPos, BOOL bEnsureVisible = TRUE);

    /**
     * CDuiItemBox::RemoveAllItems
     * @brief    删除所有
     *
     * Describe  删除所有  
     */
    void RemoveAllItems();

    /**
     * CDuiItemBox::GetItemCount
     * @brief    获取项个数
     * @return   UINT
     *
     * Describe  获取项个数  
     */
    UINT GetItemCount();

    /**
     * CDuiItemBox::PageUp
     * @brief    上一页
     *
     * Describe  上一页  
     */
    void PageUp();

    /**
     * CDuiItemBox::PageDown
     * @brief    下一页
     *
     * Describe  下一页  
     */
    void PageDown();

    /**
     * CDuiItemBox::EnsureVisible
     * @brief    设置显示
     * @param    SWindow *pItem  -- 某项指针
     *
     * Describe  设置显示  
     */
    void EnsureVisible(SWindow *pItem);

    /**
     * CDuiItemBox::GetItemPos
     * @brief    获取某项得索引
     * @return   返回int
     *
     * Describe  获取某项得索引  
     */
    int GetItemPos(SWindow * lpCurItem);

protected:
    int m_nItemWid; /**< Item宽度 */
    int m_nItemHei; /**< Item高度*/
    int m_nSepWid;  /**< */
    int m_nSepHei;  /**< */

    /**
     * CDuiItemBox::UpdateScroll
     * @brief    更新滚动条
     *
     * Describe  更新滚动条  
     */
    void UpdateScroll();

    /**
     * CDuiItemBox::GetItemRect
     * @brief    获取某项位置
     * @param    int iItem -- 某项索引
     * @return   返回int
     *
     * Describe  获取某项得索引  
     */
    
    CRect GetItemRect(int iItem);

    /**
     * CDuiItemBox::BringWindowAfter
     * @brief    插入新节点
     * @param    SWindow * pChild -- 新节点
     * @param    SWindow * pInsertAfter -- 位置节点
     *
     * Describe  在某个节点后插入新节点  
     */
    void BringWindowAfter(SWindow * pChild, SWindow * pInsertAfter);

    /**
     * CDuiItemBox::OnSize
     * @brief    消息响应函数
     * @param    UINT nType --
     * @param    CSize size -- 
     *
     * Describe  获取某项得索引  
     */
    void OnSize(UINT nType, CSize size);

    /**
     * CDuiItemBox::UpdateChildrenPosition
     * @brief    更新子节点位置
     *
     * Describe  更新子节点位置  
     */
    virtual void UpdateChildrenPosition(){}//leave it empty

    /**
     * CDuiItemBox::ReLayout
     * @brief    重新布局
     *
     * Describe  重新布局  
     */
    void ReLayout();

    /**
     * CDuiItemBox::OnScroll
     * @brief    滚动事件
     * @param    BOOL bVertical -- 是否是竖直
     * @param    UINT uCode -- 消息码
     * @param    int nPos -- 位置
     * @retur    返回int
     *
     * Describe  获取某项得索引  
     */
    virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

    /**
     * CDuiItemBox::GetScrollLineSize
     * @brief    获取滚动条大小
     * @param    BOOL bVertical -- 是否是竖直方向
     * @retur    返回int
     *
     * Describe  获取滚动条大小  
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * CDuiItemBox::CreateChildren
     * @brief    创建新项
     * @param    pugi::xml_node xmlNode
     * @return   返回BOOL
     *
     * Describe  获取某项得索引  
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);

    SOUI_ATTRS_BEGIN()
    ATTR_INT(L"itemwid", m_nItemWid, TRUE)
    ATTR_INT(L"itemhei", m_nItemHei, TRUE)
    ATTR_INT(L"sepwid", m_nSepWid, TRUE)
    ATTR_INT(L"sephei", m_nSepHei, TRUE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
    MSG_WM_SIZE(OnSize)
    SOUI_MSG_MAP_END()

};

}//namespace SOUI
