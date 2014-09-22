/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 * 
 * @file       stabctrl.h
 * @brief      
 * @version    v1.0      
 * @author     soui      
 * @date       2014-07-06
 * 
 * Describe    扩展列表框 
 */
#pragma once
#include "core/SPanel.h"
#include "core/SItempanel.h"
#include "stree.hpp"

namespace SOUI
{
/** 
 * @class     STreeItem 
 * @brief     tree item
 *
 * Describe   tree item
 */
class SOUI_EXP STreeItem : public SItemPanel
{
public:
    /**
     * STreeItem::STreeItem
     * @brief    构造函数
     * @param    SWindow *pFrameHost -- 宿主
     * @param    IItemContainer *pContainer -- 表项的容器
     *
     * Describe  构造函数  
     */
    STreeItem(SWindow *pFrameHost,IItemContainer *pContainer);

    BOOL m_bCollapsed; /**< 是否折叠 */
    BOOL m_bVisible;   /**< 是否显示 */
    int  m_nLevel;     /**< tree深度 */

    HSTREEITEM m_hItem; /**< 结构 */
};

/** 
 * @class     STreeBox 
 * @brief     STreeBox
 *
 * Describe   STreeBox
 */
class SOUI_EXP STreeBox
    : public SScrollView
    , public IItemContainer
    , protected CSTree<STreeItem *>
{
    SOUI_CLASS_NAME(STreeBox, L"treebox")
public:
    /**
     * STreeBox::STreeBox
     * @brief    构造函数
     *
     * Describe  构造函数  
     */
    STreeBox();

    /**
     * STreeBox::~STreeBox
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~STreeBox();

    /**
     * STreeBox::InsertItem
     * @brief    插入新项    
     * @param    pugi::xml_node xmlNode -- xml文件
     * @param    DWORD dwData -- 附加数据
     * @param    HSTREEITEM hParent -- 父节点
     * @param    HSTREEITEM hInsertAfter -- 某点之后插入
     * @param    BOOL bEnsureVisible -- 是否显示
     * @return   返回HSTREEITEM
     * 
     * Describe  插入新项
     */
    HSTREEITEM InsertItem(pugi::xml_node xmlNode,DWORD dwData,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);

    /**
     * STreeBox::InsertItem
     * @brief    插入新项    
     * @param    LPCWSTR pszXml -- xml文件名
     * @param    DWORD dwData -- 附加数据
     * @param    HSTREEITEM hParent -- 父节点
     * @param    HSTREEITEM hInsertAfter -- 某点之后插入
     * @param    BOOL bEnsureVisible -- 是否显示
     * @return   返回STreeItem*
     * 
     * Describe  插入新项
     */
    STreeItem* InsertItem(LPCWSTR pszXml,DWORD dwData,HSTREEITEM hParent=STVI_ROOT, HSTREEITEM hInsertAfter=STVI_LAST,BOOL bEnsureVisible=FALSE);

    /**
     * STreeBox::RemoveItem
     * @brief    删除某项    
     * @param    HSTREEITEM hItem -- 待删除节点
     * @return   返回BOOL
     * 
     * Describe  插入新项
     */
    BOOL RemoveItem(HSTREEITEM hItem);

    /**
     * STreeBox::RemoveAllItems
     * @brief    删除所有某项    
     * 
     * Describe  插入新项
     */
    void RemoveAllItems();

    /**
     * STreeBox::GetRootItem
     * @brief    获取根节点   
     * @return   返回HSTREEITEM
     * 
     * Describe  获取根节点   
     */
    HSTREEITEM GetRootItem();

    /**
     * STreeBox::GetNextSiblingItem
     * @brief    获取兄弟节点--下一个  
     * @param    HSTREEITEM hItem -- 节点
     * @return   返回HSTREEITEM
     * 
     * Describe  获取兄弟节点--下一个  
     */
    HSTREEITEM GetNextSiblingItem(HSTREEITEM hItem);
    /**
     * STreeBox::GetPrevSiblingItem
     * @brief    获取兄弟节点--前一个 
     * @param    HSTREEITEM hItem -- 节点
     * @return   返回HSTREEITEM
     * 
     * Describe  获取兄弟节点--前一个
     */
    HSTREEITEM GetPrevSiblingItem(HSTREEITEM hItem);
    /**
     * STreeBox::GetChildItem
     * @brief    获取子节点    
     * @param    HSTREEITEM hItem -- 父节点
     * @param    BOOL bFirst -- 第几个子节点
     * @return   返回HSTREEITEM
     * 
     * Describe  获取子节点  
     */
    HSTREEITEM GetChildItem(HSTREEITEM hItem,BOOL bFirst =TRUE);
    /**
     * STreeBox::GetParentItem
     * @brief    获取父节点    
     * @param    HSTREEITEM hItem -- 子节点
     * @return   返回HSTREEITEM
     * 
     * Describe  获取父节点 
     */
    HSTREEITEM GetParentItem(HSTREEITEM hItem);

    /**
     * STreeBox::PageUp
     * @brief    上一页  
     * 
     * Describe  上一页  
     */
    void PageUp();

    /**
     * STreeBox::PageDown
     * @brief    下一页 
     * 
     * Describe  下一页
     */
    void PageDown();

    /**
     * STreeBox::OnDestroy
     * @brief    销毁    
     * 
     * Describe  获取父节点 
     */
    void OnDestroy();

    /**
     * STreeBox::Expand
     * @brief    展开    
     * @param    HSTREEITEM hItem -- 节点
     * @param    UINT nCode -- 
     * @return   返回BOOL
     * 
     * Describe  展开
     */
    BOOL Expand(HSTREEITEM hItem , UINT nCode);

    /**
     * STreeBox::EnsureVisible
     * @brief    显示某节点 
     * @param    HSTREEITEM hItem -- 节点
     * 
     * Describe  显示某节点 
     */
    BOOL EnsureVisible(HSTREEITEM hItem);

    /**
     * STreeBox::HitTest
     * @brief    获取节点
     * @param    HSTREEITEM hItem -- 节点
     * 
     * Describe  点击某节点 返回节点信息
     */
    HSTREEITEM HitTest(CPoint &pt);

    /**
     * STreeBox::GetItemPanel
     * @brief    获取节点
     * @param    HSTREEITEM hItem -- 节点
     * 
     * Describe  获取节点
     */
    STreeItem * GetItemPanel(HSTREEITEM hItem)
    {
        return GetItem(hItem);
    }
protected:
    /**
     * STreeBox::SetChildrenVisible
     * @brief    设置节点显示
     * @param    HSTREEITEM hItem -- 节点
     * @param    BOOL bVisible -- 是否显示
     * 
     * Describe  设置节点显示
     */
    void SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible);
    /**
     * STreeBox::OnNodeFree
     * @brief    释放节点
     * @param    STreeItem * & pItem  -- 待释放节点
     * 
     * Describe  释放节点
     */
    virtual void OnNodeFree(STreeItem * & pItem);
    /**
     * STreeBox::GetScrollLineSize
     * @brief    滚动条长度
     * @param    BOOL bVertical -- 是否是垂直
     * 
     * Describe  滚动条长度
     */
    virtual int GetScrollLineSize(BOOL bVertical);
    /**
     * STreeBox::CreateChildren
     * @brief    创建tree
     * @param    pugi::xml_node xmlNode -- xml文件
     * 
     * Describe  创建tree
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);
    /**
     * STreeBox::LoadBranch
     * @brief    加载分支节点
     * @param    HSTREEITEM hParent -- 父节点
     * @param    pugi::xml_node xmlNode -- xml文件
     * 
     * Describe  加载分支节点
     */
    void LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlNode);
    /**
     * STreeBox::OnNcCalcSize
     * @brief    计算非客户区大小
     * @param    BOOL bCalcValidRects  --  
     * @param    LPARAM lParam -- 
     * 
     * Describe  计算非客户区大小
     */
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);
    /**
     * STreeBox::GetItemShowIndex
     * @brief    获取索引
     * @param    HSTREEITEM hItemObj -- 节点
     * 
     * Describe  获取索引
     */
    int GetItemShowIndex(HSTREEITEM hItemObj);
    /**
     * STreeBox::RedrawItem
     * @brief    重新绘制
     * @param    HSTREEITEM hItem -- 节点
     * 
     * Describe  重新绘制
     */
    void RedrawItem(HSTREEITEM hItem);

    /**
     * STreeBox::DrawItem
     * @brief    绘制
     * @param    IRenderTarget *pRT -- 绘图设备
     * @param    CRect & rc -- 位置
     * @param    HSTREEITEM hItem -- 节点
     * 
     * Describe  重新绘制
     */
    void DrawItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem);

    /**
     * STreeBox::OnPaint
     * @brief    绘制
     * @param    IRenderTarget *pRT -- 绘图设备
     * 
     * Describe  绘制
     */
    void OnPaint(IRenderTarget *pRT);

    /**
     * STreeBox::OnLButtonDown
     * @brief    鼠标左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
    */
    void OnLButtonDown(UINT nFlags,CPoint pt);
    /**
     * STreeBox::OnLButtonDbClick
     * @brief    鼠标左键双击事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
    */
    void OnLButtonDbClick(UINT nFlags,CPoint pt);
    /**
     * STreeBox::OnLButtonDbClick
     * @brief    鼠标事件
     * @param    UINT uMsg  -- 消息码
     * @param    WPARAM wParam  --
     * @param    LPARAM lParam  -- 
     * @return   返回LRESULT
     *
     * Describe  此函数是消息响应函数
    */
    LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    /**
     * STreeBox::OnMouseMove
     * @brief    鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
    */
    void OnMouseMove(UINT nFlags,CPoint pt);

    /**
     * STreeBox::OnMouseLeave
     * @brief    鼠标离开事件
     *
     * Describe  此函数是消息响应函数
    */
    void OnMouseLeave();
    /**
     * STreeBox::OnSetFocus
     * @brief    获得焦点
     *
     * Describe  此函数是消息响应函数
    */
    void OnSetFocus();
    /**
     * STreeBox::OnKillFocus
     * @brief    失去焦点
     *
     * Describe  此函数是消息响应函数
    */
    void OnKillFocus();
    /**
     * STreeBox::OnKeyEvent
     * @brief    按键事件
     * @param    UINT uMsg  -- 消息码
     * @param    WPARAM wParam  --
     * @param    LPARAM lParam  -- 
     * @return   返回LRESULT
     *
     * Describe  此函数是消息响应函数
    */
    LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );
    /**
     * STreeBox::FireEvent
     * @brief     
     * @param    EventArgs &evt  --  
     *
     * Describe   
    */
    virtual BOOL FireEvent(EventArgs &evt);
    /**
     * STreeBox::OnSetCursor
     * @brief    设置鼠标
     * @param    const CPoint &pt  -- 位置
     *
     * Describe  此函数是消息响应函数
    */
    virtual BOOL OnSetCursor(const CPoint &pt);
    /**
     * STreeBox::OnViewOriginChanged
     * @brief    
     * @param    CPoint ptOld  -- 
     * @param    CPoint ptNew  --
     *
     * Describe  
    */
    virtual void OnViewOriginChanged( CPoint ptOld,CPoint ptNew );
    /**
     * STreeBox::OnGetDlgCode
     * @brief    获取窗口消息码
     * @return   返回UINT
     *
     * Describe  此函数是消息响应函数
    */
    virtual UINT OnGetDlgCode()
    {
        return SC_WANTALLKEYS;
    }

    /**
     * STreeBox::IsAncestor
     * @brief    判断是否是先祖
     * @param    HSTREEITEM hItem1  -- 节点1
     * @param    HSTREEITEM hItem2  -- 节点2
     *
     * Describe  判断是否是先祖
    */
    BOOL IsAncestor(HSTREEITEM hItem1,HSTREEITEM hItem2);
protected:
    /**
     * STreeBox::OnItemSetCapture
     * @brief    判断是否捕获
     * @param    SItemPanel *pItem  -- 节点
     * @param    BOOL bCapture  -- 是否捕获
     *
     * Describe  判断是否捕获
    */
    virtual void OnItemSetCapture(SItemPanel *pItem,BOOL bCapture);
    /**
     * STreeBox::OnItemGetRect
     * @brief    
     * @param    SItemPanel *pItem  -- 节点
     * @param    CRect &rcItem  -- 位置
     *
     * Describe  
    */
    virtual BOOL OnItemGetRect(SItemPanel *pItem,CRect &rcItem);
    /**
     * STreeBox::IsItemRedrawDelay
     * @brief    判断是否是先祖
     * @param    返回BOOL
     *
     * Describe  判断是否是先祖
    */
    virtual BOOL IsItemRedrawDelay(){return m_bItemRedrawDelay;}

    HSTREEITEM    m_hSelItem;  /**< 选中item */
    HSTREEITEM    m_hHoverItem; /**< hover状态item */

    int            m_nVisibleItems;  /**< 显示item个数 */

    SItemPanel    *m_pCapturedFrame;  /**<  */

    int  m_nItemHei;  /**< 高度 */ 
    int  m_nIndent;  /**< 缩进 */ 
    COLORREF m_crItemBg; /**< 背景色 */ 
    COLORREF m_crItemSelBg;  /**< 选中背景色 */ 
    ISkinObj * m_pItemSkin;  /**< ISkinObj对象 */ 
    BOOL m_bItemRedrawDelay;  /**< */
    pugi::xml_document m_xmlSwitch;/**< xml文件 */

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"indent", m_nIndent, TRUE)
        ATTR_INT(L"itemHeight", m_nItemHei, TRUE)
        ATTR_SKIN(L"itemSkin", m_pItemSkin, TRUE)
        ATTR_COLOR(L"colorItemBkgnd",m_crItemBg,FALSE)
        ATTR_COLOR(L"colorItemSelBkgnd",m_crItemSelBg,FALSE)
        ATTR_INT(L"itemRedrawDelay", m_bItemRedrawDelay, TRUE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR,OnKeyEvent)
   SOUI_MSG_MAP_END()
};

}//namespace SOUI