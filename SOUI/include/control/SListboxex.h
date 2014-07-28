/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       SListboxex.h
 * @brief      
 * @version    v1.0      
 * @author     soui      
 * @date       2014-07-06
 * 
 * Describe    扩展列表框 
 */
#pragma  once
#include "core/SPanel.h"
#include "core/Sitempanel.h"

namespace SOUI
{
/** 
 * @class     SListBox
 * @brief     扩展列表框
 *
 * Describe   扩展列表框
 */
class SOUI_EXP SListBoxEx :public SScrollView
    ,public IItemContainer
{
public:

    SOUI_CLASS_NAME(SListBoxEx, L"listboxex")
    
    /**
     * SListBoxEx::SListBox
     * @brief    构造函数
     *
     * Describe  构造函数  
     */
    SListBoxEx();

    /**
     * SListBox::~SListBox
     * @brief    析构函数
     *
     * Describe  析构函数  
     */
    virtual ~SListBoxEx();

    /**
     * SListBoxEx::DeleteAllItems
     * @brief    删除所有
     * @param    BOOL bUpdate -- 是否更新
     *
     * Describe  删除所有
     */
    void DeleteAllItems(BOOL bUpdate=TRUE);

    /**
     * SListBoxEx::DeleteItem
     * @brief    删除指定项
     * @param    int iItem -- 索引
     *
     * Describe  删除指定项
     */
    void DeleteItem(int iItem);

    /**
     * SListBoxEx::InsertItem
     * @brief    插入新项
     * @param    int iItem -- 索引
     * @param    pugi::xml_node xmlNode -- xml文件
     * @param    DWORD dwData  --  附加数据
     *
     * Describe  在某项之后插入
     */
    int InsertItem(int iItem,pugi::xml_node xmlNode,LPARAM dwData=0);

    /**
     * SListBoxEx::InsertItem
     * @brief    插入新项
     * @param    int iItem -- 索引
     * @param    LPCWSTR pszXml -- xml文件
     * @param    DWORD dwData  --  附加数据
     *
     * Describe  在某项之后插入
     */
    int InsertItem(int iItem,LPCWSTR pszXml,LPARAM dwData=0);
    
    /**
     * SListBoxEx::SetCurSel
     * @brief    设置选中项
     * @param    int nIndex -- 待选中索引
     *
     * Describe  设置选中项
     */
    BOOL SetCurSel(int iItem);

    /**
     * SListBoxEx::EnsureVisible
     * @brief    设置某项显示
     * @param    int nIndex  -- 索引
     *
     * Describe  设置某项显示
     */
    void EnsureVisible(int iItem);

    /**
     * SListBoxEx::GetCurSel
     * @brief    获取当前选中项索引
     * @return   返回int
     *
     * Describe  获取当前选中项索引
     */
    int GetCurSel();


    /**
     * SListBoxEx::GetItemPanel
     * @brief    获取指定项结构
     * @param    int iItem -- 索引
     * @return   返回SWindow
     *
     * Describe  获取指定项索引
     */
    SWindow * GetItemPanel(int iItem);

    /**
     * SListBoxEx::GetItemData
     * @brief    获取附加数据
     * @param    int nIndex -- 选项索引
     * @return   返回LPARAM
     *
     * Describe  获取附加数据
     */
    LPARAM GetItemData(int iItem);

    /**
     * SListBoxEx::SetItemData
     * @brief    设置选项附加数据
     * @param    int nIndex -- 选项索引
     * @param    LPARAM lParam  -- 附加数据
     * @return   返回BOOL
     *
     * Describe  设置选项附加数据
     */
    void SetItemData(int iItem,LPARAM lParam);

    /**
     * SListBoxEx::SetItemCount
     * @brief    设置条目数量
     * @param    int nItems 条目数量
     * @param    LPCTSTR pszXmlTemplate 显示时使用的XML模板，
     *           为空时使用XML脚本中指定的template子节点数据
     * @return   设置条目数量
     *
     * Describe  设置选项附加数据
     */
    BOOL SetItemCount(int nItems,LPCTSTR pszXmlTemplate=NULL);

    /**
     * SListBoxEx::GetItemCount
     * @brief    获取条目数量
     * @return   返回 int 
     *
     * Describe  获取条目数量
     */
    int GetItemCount() ;

    /**
     * SListBoxEx::GetItemHeight
     * @brief    获取高度
     * @return   返回int 
     *
     * Describe  获取高度
     */
    int GetItemHeight()
    {
        return m_nItemHei;
    }

    /**
     * SListBoxEx::RedrawItem
     * @brief    重绘选项
     * @param    int iItem  -- 索引
     *
     * Describe  重绘选项
     */
    void RedrawItem(int iItem);

    /**
     * SListBoxEx::HitTest
     * @brief    获取鼠标相关信息
     * @param    CPoint &pt -- 坐标
     *
     * Describe  自动修改pt的位置为相对当前项的偏移量
     */
    int HitTest(CPoint &pt);

protected:
    /**
     * SListBoxEx::OnItemSetCapture
     * @brief     
     * @param     SItemPanel *pItem
     * @param     BOOL bCapture
     *
     * Describe  
     */    
    virtual void OnItemSetCapture(SItemPanel *pItem,BOOL bCapture);

    /**
     * SListBoxEx::OnItemGetRect
     * @brief     
     * @param     SItemPanel *pItem
     * @param     CRect &rcItem
     *
     * Describe   
     */
    virtual BOOL OnItemGetRect(SItemPanel *pItem,CRect &rcItem);

    /**
     * SListBoxEx::IsItemRedrawDelay
     * @brief    
     *
     * Describe  
     */
    virtual BOOL IsItemRedrawDelay(){return m_bItemRedrawDelay;}

protected:

    /**
     * SListBoxEx::UpdatePanelsIndex
     * @brief    更新索引
     * @param    UINT nFirst
     * @param    UINT nLast
     *
     * Describe  更新索引
     */   
    void UpdatePanelsIndex(UINT nFirst=0,UINT nLast=-1);

    /**
     * SListBoxEx::GetItemRect
     * @brief    获取指定位置
     * @param    int iItem -- 索引
     * @return   返回CRect 
     *
     * Describe  更新索引
     */
    CRect    GetItemRect(int iItem);

    /**
     * SListBoxEx::GetScrollLineSize
     * @brief    获取滚动大小
     * @param    BOOL bVertical -- 是否是垂直
     *
     * Describe  获取滚动大小
     */
    virtual int GetScrollLineSize(BOOL bVertical);

    /**
     * SListBoxEx::OnPaint
     * @brief    绘制
     * @param    IRenderTarget *pRT -- 绘制设备
     *
     * Describe  消息响应函数 
     */
    void OnPaint(IRenderTarget * pRT);

    /**
     * SListBoxEx::OnSize
     * @brief    消息响应函数
     * @param    UINT nType --
     * @param    CSize size -- 
     *
     * Describe  获取某项得索引  
     */
    void OnSize(UINT nType, CSize size);

    /**
     * SListBoxEx::OnDrawItem
     * @brief    绘制
     * @param    IRenderTarget *pRT  -- 绘制设备
     * @param    CRect & rc -- 位置
     * @param    int iItem -- 索引
     *
     * Describe  绘制
     */
    virtual void OnDrawItem(IRenderTarget *pRT, CRect & rc, int iItem);

    /**
     * SListBoxEx::CreateChildren
     * @brief    创建新项
     * @param    pugi::xml_node xmlNode -- xml文件
     *
     * Describe  通过解析xml文件创建
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);

    /**
     * SListBoxEx::OnUpdateToolTip
     * @brief    更新提示框
     * @param    SWND hCurTipHost -- 
     * @param    SWND &hNewTipHost -- 
     * @param    CRect &rcTip --
     * @param    SStringT &strTip -- 
     *
     * Describe  更新提示框
     */
    virtual BOOL OnUpdateToolTip(SWND hCurTipHost,SWND &hNewTipHost,CRect &rcTip,SStringT &strTip);

    /**
     * SListBoxEx::NotifySelChange
     * @brief    修改选中项
     * @param    int nOldSel -- 旧选中项
     * @param    int nNewSel -- 新选中项
     *
     * Describe  修改选中项
     */
    void NotifySelChange(int nOldSel,int nNewSel);

    /**
     * SListBoxEx::OnMouseEvent
     * @brief    自定义消息响应函数
     * @param    UINT uMsg --
     * @param    WPARAM wParam -- 
     * @param    LPARAM lParam --
     * @return   返回LRESULT
     *
     * Describe  自定义消息响应函数
     */
    LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam);

    /**
     * SListBoxEx::OnKeyEvent
     * @brief    自定义消息响应函数
     * @param    UINT uMsg --
     * @param    WPARAM wParam -- 
     * @param    LPARAM lParam --
     * @return   返回LRESULT
     * 
     * Describe  自定义消息响应函数
     */
    LRESULT OnKeyEvent( UINT uMsg,WPARAM wParam,LPARAM lParam );

    /**
     * SListBoxEx::OnMouseLeave
     * @brief    自定义消息响应函数
     * 
     * Describe  自定义消息响应函数
     */
    void OnMouseLeave();

    /**
     * SListBoxEx::OnMouseWheel
     * @brief    鼠标滚轮事件
     * @param    UINT nFlags -- 标志
     * @param    short zDelta -- 滚轮滚动的距离
     * @param    CPoint pt -- 坐标
     * @return   返回BOOL
     * 
     * Describe  鼠标滚轮事件
     */
    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    /**
     * SListBoxEx::OnKeyDown
     * @brief    按键按下
     * @param    TCHAR nChar -- 键码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  消息响应函数 
     */
    void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

    /**
     * SListBoxEx::OnChar
     * @brief    按键按下
     * @param    TCHAR nChar -- 键码值
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     *
     * Describe  消息响应函数 
     */
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * SListBoxEx::OnGetDlgCode
     * @brief    获取窗口消息码
     * @param    返回UINT
     *
     * Describe  获取窗口消息码
     */
    UINT OnGetDlgCode();

    /**
     * SListBoxEx::OnSetCursor
     * @brief    设置鼠标位置
     * @param    返回BOOL
     *
     * Describe  设置鼠标位置
     */
    virtual BOOL OnSetCursor(const CPoint &pt);

    /**
     * SListBoxEx::OnViewOriginChanged
     * @brief    
     * @param    CPoint ptOld  --
     * @param    CPoint ptNew  -- 
     *
     * Describe  
     */
    virtual void OnViewOriginChanged(CPoint ptOld,CPoint ptNew);

    /**
     * SListBoxEx::OnDestroy
     * @brief    销毁
     *
     * Describe  销毁
     */
    void OnDestroy();

    /**
     * SListBoxEx::OnSetFocus
     * @brief    获得焦点
     *
     * Describe  获得焦点
     */
    void OnSetFocus();

    /**
     * SListBoxEx::OnKillFocus
     * @brief    失去焦点
     *
     * Describe  失去焦点
     */
    void OnKillFocus();

    /**
     * SListBoxEx::OnNcCalcSize
     * @brief    计算非客户区大小
     * @param    BOOL bCalcValidRects -- 大小
     * @param    LPARAM lParam -- 附加参数
     * Describe  计算非客户区大小
     */
    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    /**
     * SListBoxEx::Relayout
     * @brief    重新布局
     *
     * Describe  重新布局
     */
    void Relayout();
protected:
    SArray<SItemPanel *> m_arrItems;  /**< 保存条目的数组 */
    int        m_nItemHei;     /**< 高度          */
    int        m_iSelItem;     /**< 选中条目      */
    int        m_iHoverItem;   /**< Hover状态条目 */
    int        m_iScrollSpeed; /**< 滚动速度      */

    pugi::xml_document m_xmlTempl;     /**< 列表模板XML */
    SItemPanel   *m_pCapturedFrame;    /**<  */
    ISkinObj     *m_pItemSkin;         /**<  */
    COLORREF     m_crItemBg;           /**< 背景色 */
    COLORREF     m_crItemSelBg;        /**< 选中背景色 */
    BOOL       m_bVirtual;             /**< 是否垂直 */
    BOOL       m_bItemRedrawDelay;     /**< 表项重绘时缓冲 */       
public:
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"scrollspeed", m_iScrollSpeed, FALSE)
        ATTR_INT(L"itemheight", m_nItemHei, FALSE)
        ATTR_INT(L"virtual", m_bVirtual, TRUE)
        ATTR_SKIN(L"itemskin", m_pItemSkin, TRUE)
        ATTR_COLOR(L"critembg",m_crItemBg,FALSE)
        ATTR_COLOR(L"critemselbg",m_crItemSelBg,FALSE)
        ATTR_INT(L"itemredrawdelay", m_bItemRedrawDelay, TRUE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST,WM_MOUSELAST,OnMouseEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_KEYFIRST,WM_KEYLAST,OnKeyEvent)
        MESSAGE_RANGE_HANDLER_EX(WM_IME_STARTCOMPOSITION,WM_IME_KEYLAST,OnKeyEvent)
        MESSAGE_HANDLER_EX(WM_IME_CHAR,OnKeyEvent)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_CHAR(OnChar)
        MSG_WM_SIZE(OnSize)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_NCCALCSIZE(OnNcCalcSize)
    SOUI_MSG_MAP_END()
};

}//namespace SOUI