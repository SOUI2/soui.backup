/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 * 
 * @file       SCmnCtrl.h
 * @brief      通用控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-06-26
 * 
 * Describe    ComboBox控件
 */
#pragma once
#include "core/SWnd.h"
#include "SRichEdit.h"
#include "SDropDown.h"
#include "Slistbox.h"
#include "Slistboxex.h"
#include "SCmnCtrl.h"

namespace SOUI
{

#define IDC_CB_EDIT          -100
#define IDC_DROPDOWN_LIST    -200

class SComboBoxBase;


/**
 * @class      SComboEdit
 * @brief      在CommboBox中嵌入的Edit控件
 * 
 * Describe    
 */
class SComboEdit:public SEdit
{
public:
    /**
     * SComboEdit::SComboEdit
     * @param    SComboBoxBase *pOwner  -- 暂无       
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboEdit(SComboBoxBase *pOwner);
    
    /**
     * SComboEdit::~SComboEdit
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~SComboEdit(){}
protected:
    /**
     * SComboEdit::OnMouseHover
     * @brief    键盘鼠标悬停事件
     * @param    WPARAM wParam 
     * @param    CPoint ptPos -- 鼠标所在位置
     * 
     * Describe  此函数是消息响应函数
     */
    void OnMouseHover(WPARAM wParam, CPoint ptPos);
    /**
     * SComboEdit::OnMouseLeave
     * @brief    键盘鼠标离开事件
     * 
     * Describe  此函数是消息响应函数
     */    
    void OnMouseLeave();
    /**
     * SComboEdit::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值 
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     * 
     * Describe  此函数是消息响应函数
     */   
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * SComboEdit::FireEvent
     * @brief    通知消息
     * @param    EventArgs & evt -- 事件对象 
     * 
     * Describe  此函数是消息响应函数
     */   
    virtual BOOL FireEvent(EventArgs & evt);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown)
    SOUI_MSG_MAP_END()
};

class SOUI_EXP SDropDownWnd_ComboBox : public SDropDownWnd
{
public:
    SDropDownWnd_ComboBox(ISDropDownOwner* pOwner):SDropDownWnd(pOwner){}
    
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

/**
 * @class      SComboBoxBase
 * @brief      可输入CommboBox
 * 
 * Describe    可输入下拉列表
 */
class SOUI_EXP SComboBoxBase 
    : public SWindow
    , public ISDropDownOwner
{
    SOUI_CLASS_NAME(SComboBoxBase,L"comboboxbase")
public:
    
    /**
     * SComboBoxBase::SComboBoxBase
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboBoxBase(void);
    
    /**
     * SComboBoxBase::~SComboBoxBase
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~SComboBoxBase(void);
    /**
     * SComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int  
     *
     * Describe  获取当前选中索引
     */
    virtual int GetCurSel() const =0;

    /**
    * SComboBoxBase::GetCount
    * @brief    获取下拉项个数
    * @return   返回int
    * 
    * Describe  获取下拉项个数
    */ 
    virtual int  GetCount() const =0;
    
    /**
     * SComboBoxBase::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     * 
     * Describe  设置当前选中
     */ 
    virtual BOOL SetCurSel(int iSel)=0;

    /**
    * SComboBoxEx::GetTextRect
    * @brief    获取文本位置
    * @param    LPRECT pRect -- 文本位置
    *
    * Describe  获取文本位置
    */
    virtual SStringT GetWindowText();

    virtual SStringT GetLBText(int iItem) =0;
    /**
     * FindString
     * @brief    查找字符串位置
     * @param    LPCTSTR pszFind --  查找目标
     * @param    int nAfter --  开始位置
     * @return   int -- 目标索引，失败返回-1。
     * Describe  
     */    
    virtual int FindString(LPCTSTR pszFind,int nAfter=0);

    /**
     * SComboBoxBase::DropDown
     * @brief    下拉事件
     *
     * Describe  下拉事件
     */
    void DropDown();
    
    /**
     * SComboBoxBase::CloseUp
     * @brief    下拉关闭
     *
     * Describe  下拉关闭
     */
    void CloseUp();

protected:
    /**
     * SComboBoxBase::GetDropDownOwner
     * @brief    获取owner
     * @return   SWindow
     *
     * Describe  获取owner
     */
    virtual SWindow* GetDropDownOwner();

    /**
     * SComboBoxBase::OnDropDown
     * @brief    下拉事件
     * @param     SDropDownWnd *pDropDown -- 下拉窗口指针
     *
     * Describe  下拉事件
     */
    virtual void OnDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBoxBase::OnCloseUp
     * @brief    下拉事件
     * @param     SDropDownWnd *pDropDown -- 下拉窗口指针
     * @param     UINT uCode -- 消息码
     *
     * Describe  下拉关闭
     */
    virtual void OnCloseUp(SDropDownWnd *pDropDown,UINT uCode);

    /**
     * SComboBoxBase::OnSelChanged
     * @brief    下拉窗口改变事件
     *
     * Describe  下拉关闭
     */
    virtual void OnSelChanged();
    
    
    virtual BOOL FireEvent(EventArgs &evt);
protected:

    /**
     * SComboBoxBase::CalcPopupRect
     * @brief    计算弹出窗口位置
     * @param    int nHeight -- 下拉窗口高度
     * @param    CRect & rcPopup -- 保存弹出窗口Rect
     * @return   BOOL  TRUE -- 成功  FALSE -- 失败
     *
     * Describe  计算弹出窗口位置,保存在rcPopup中
     */    
    BOOL CalcPopupRect(int nHeight,CRect & rcPopup);
    
    /**
     * SComboBoxBase::CreateListBox
     * @brief    创建下拉列表
     * @param    pugi::xml_node xmlNode  -- xml对象
     * @return   BOOL  TRUE -- 成功  FALSE -- 失败
     *
     * Describe  创建下拉列表
     */    
    virtual BOOL CreateListBox(pugi::xml_node xmlNode)=0;
    
    /**
     * SComboBoxBase::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @return   返回int 高度
     *
     * Describe  获取下拉列表高度
     */        
    virtual int  GetListBoxHeight()=0;

    /**
     * SComboBoxBase::GetDropBtnRect
     * @brief    获取下拉列表按钮位置
     * @param    LPRECT prc -- 按钮Rect
     *
     * Describe  获取下拉列表右侧按钮位置
     */        
    void GetDropBtnRect(LPRECT prc);
    /**
     * SComboBoxBase::LoadChildren
     * @brief    加载子项
     * @param    pugi::xml_node xmlNode  -- xml文件
     * @return   返回BOOL  TRUE -- 成功 FALSE -- 失败
     *
     * Describe  加载子项
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);    
    /**
     * SComboBoxBase::GetTextRect
     * @brief    获取文本位置
     * @param    LPRECT pRect -- 文本位置
     *
     * Describe  获取文本位置
     */
    virtual void GetTextRect(LPRECT pRect);
    /**
     * SComboBoxBase::OnPaint
     * @brief    绘制消息
     * @param    IRenderTarget * pRT -- 暂无
     * 
     * Describe  此函数是消息响应函数
     */
    void OnPaint(IRenderTarget * pRT);
    
    /**
     * SComboBoxBase::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags,CPoint pt);

    /**
     * SComboBoxBase::OnMouseMove
     * @brief    键盘鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     * 
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags,CPoint pt);

    /**
     * SComboBoxBase::OnMouseLeave
     * @brief    键盘鼠标移动事件
     * 
     * Describe  此函数是消息响应函数
     */
    void OnMouseLeave();

    /**
     * SComboBoxBase::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值 
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     * 
     * Describe  此函数是消息响应函数
     */
    void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

    /**
     * SComboBoxBase::OnChar
     * @brief    字符消息
     * @param    UINT nChar -- 按键对应的码值 
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     * 
     * Describe  此函数是消息响应函数
     */ 
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * SComboBoxBase::OnDestroy
     * @brief    下拉窗口销毁
     * 
     * Describe  此函数是用于销毁下拉窗口
     */  
    void OnDestroy();
    /**
     * SComboBoxBase::OnGetDlgCode
     * @brief    获取消息码
     * 
     * Describe  获取消息码
     */  
    UINT OnGetDlgCode();
    
    /**
     * SComboBoxBase::IsTabStop
     * @brief    是否禁止TAB键
     * 
     * Describe  是否禁止TAB键
     */  
    BOOL IsFocusable();

    void OnSetFocus();
    
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"dropDown", m_bDropdown, FALSE)
        ATTR_INT(L"dropHeight", m_nDropHeight, FALSE)
        ATTR_INT(L"curSel", m_iInitSel, FALSE)
        ATTR_SKIN(L"btnSkin", m_pSkinBtn, FALSE)
        ATTR_INT(L"animateTime", m_iAnimTime, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)        
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown) 
        MSG_WM_CHAR(OnChar)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
    SOUI_MSG_MAP_END()

protected:
    /**
     * SComboBoxBase::GetEditText
     * @brief    获取编辑框内容
     * 
     * Describe  获取编辑框内容
     */  
    SStringT GetEditText() const
    {
        if(!m_bDropdown)
        {
            return m_pEdit->GetWindowText();
        }
        else
        {
            return SStringT();
        }
    }

    SRichEdit *m_pEdit;      /**< SRichEdit指针 */
    DWORD     m_dwBtnState;  /**< 按钮状态      */
    ISkinObj *m_pSkinBtn;    /**< 按钮资源      */
    
    BOOL m_bDropdown;        /**< 是否按下   */
    int  m_nDropHeight;      /**< 下拉框高度 */
    int  m_iAnimTime;        /**< 动画时间   */
    int  m_iInitSel;         /**< 默认选中索引 */
    SDropDownWnd *m_pDropDownWnd;  /**< DropDown指针 */
};

/**
 * @class      SComboBox
 * @brief      可输入CommboBox
 * 
 * Describe    可输入下拉列表
 */
class SOUI_EXP SComboBox : public SComboBoxBase
{
    SOUI_CLASS_NAME(SComboBox, L"combobox")
public:
    /**
     * SComboBox::SComboBox
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboBox();
      
    /**
     * SComboBox::~SComboBox
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~SComboBox();

    /**
     * SComboBox::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     * 
     * Describe  设置当前选中
     */ 
    BOOL SetCurSel(int iSel)
    {
        if(m_pListBox->SetCurSel(iSel))
        {
            OnSelChanged();
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    /**
     * SComboBox::GetCurSel
     * @brief    获取选中索引
     * @return   返回int -- 选中索引
     * 
     * Describe  获取选中索引
     */ 
    int GetCurSel() const
    {
        return m_pListBox->GetCurSel();
    }

    /**
     * SComboBox::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     * 
     * Describe  获取下拉项个数
     */ 
    int  GetCount() const
    {
        return m_pListBox->GetCount();
    }

    /**
     * SComboBox::GetItemData
     * @brief    获取附加数据
     * @param    UINT iItem -- 选项值
     *
     * Describe  获取附加数据
     */
    LPARAM GetItemData(UINT iItem) const
    {
        return m_pListBox->GetItemData(iItem);
    }

    /**
     * SComboBox::SetItemData
     * @brief    设置附加数据
     * @param    UINT iItem -- 索引值
     * @param    LPARAM lParam -- 附加值
     *
     * Describe  设置附加数据
     */
    int SetItemData(UINT iItem, LPARAM lParam)
    {
        return m_pListBox->SetItemData(iItem,lParam);
    }

    /**
     * SComboBox::InsertItem
     * @brief    插入新项
     * @param    UINT iPos -- 位置
     * @param    LPCTSTR pszText -- 文本值
     * @param    int iIcon -- 图标
     * @param    LPARAM lParam -- 附加值
     *
     * Describe  插入新项
     */
    int InsertItem(UINT iPos,LPCTSTR pszText,int iIcon,LPARAM lParam)
    {
        return m_pListBox->InsertString(iPos,pszText,iIcon,lParam);
    }

    /**
     * SComboBox::DeleteString
     * @brief    删除某一项
     * @param    UINT iItem -- 索引值
     *
     * Describe  删除某一项
     */
    BOOL DeleteString(UINT iItem)
    {
        return m_pListBox->DeleteString(iItem);
    }

    /**
     * SComboBox::ResetContent
     * @brief    删除所有项
     *
     * Describe  设置附加数据
     */
    void ResetContent()
    {
        SetCurSel(-1);
        return m_pListBox->DeleteAll();
    }

    /**
     * SComboBox::GetLBText
     * @brief    获取文本
     * @param    int iItem -- 索引值
     *
     * Describe  获取文本
     */
    SStringT GetLBText(int iItem)
    {
        SStringT strText;
        m_pListBox->GetText(iItem,strText);
        return strText;
    }
    /**
     * SComboBox::GetListBox
     * @brief    获取下拉列表指针
     * @param    返回SListBox * 
     *
     * Describe  获取下拉列表指针
     */
    SListBox * GetListBox(){return m_pListBox;}
    
protected:
    /**
     * SComboBox::FireEvent
     * @brief    通知消息
     * @param    EventArgs &evt -- 事件对象 
     * 
     * Describe  此函数是消息响应函数
     */ 
    virtual BOOL FireEvent(EventArgs &evt);

    /**
     * SComboBox::CreateListBox
     * @brief    创建下拉列表
     * @param    返回BOOL TRUE -- 成功 FALSE -- 失败
     *
     * Describe  创建下拉列表
     */
    virtual BOOL CreateListBox(pugi::xml_node xmlNode);
    
    /**
     * SComboBox::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @param    返回int
     *
     * Describe  获取下拉列表高度
     */
    virtual int  GetListBoxHeight();

    /**
     * SComboBox::OnDropDown
     * @brief    下拉列表事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     *
     * Describe  下拉列表事件
     */
    virtual void OnDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBox::OnCloseUp
     * @brief    下拉列表关闭事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     * @param    UINT uCode -- 消息码
     *
     * Describe  获取下拉列表指针
     */
    virtual void OnCloseUp(SDropDownWnd *pDropDown,UINT uCode);
    
    /**
     * SComboBox::OnSelChanged
     * @brief    下拉列表selected事件
     *
     * Describe  下拉列表selected事件
     */
    virtual void OnSelChanged();

protected:

    SListBox *m_pListBox;  /**< SListBox指针 */
};

class SOUI_EXP SComboBoxEx : public SComboBoxBase
{
    SOUI_CLASS_NAME(SComboBoxEx, L"comboboxex")
public:
    /**
     * SComboBoxEx::SComboBoxEx
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboBoxEx();
    /**
     * SComboBoxEx::~SComboBoxEx
     * @brief    析构函数
     *
     * Describe  析构函数
     */    
    virtual ~SComboBoxEx();

    /**
     * SComboBoxEx::SetCurSel
     * @brief    设置当前选中
     * @param    int iSel -- 选中索引
     * 
     * Describe  设置当前选中
     */
    BOOL SetCurSel(int iSel)
    {
        if(m_pListBox->SetCurSel(iSel))
        {
            OnSelChanged();
            return TRUE;
        }else
        {
            return FALSE;
        }
    }

    /**
     * SComboBoxEx::GetCurSel
     * @brief    获取选中索引
     * @return   返回int -- 选中索引
     * 
     * Describe  获取选中索引
     */ 
    int GetCurSel() const
    {
        return m_pListBox->GetCurSel();
    }

    /**
     * SComboBox::GetCount
     * @brief    获取下拉项个数
     * @return   返回int
     * 
     * Describe  获取下拉项个数
     */ 
    int  GetCount() const
    {
        return m_pListBox->GetItemCount();
    }
    

    /**
     * SComboBoxEx::GetItemData
     * @brief    获取附加数据
     * @param    UINT iItem -- 选项值
     *
     * Describe  获取附加数据
     */
    LPARAM GetItemData(UINT iItem) const
    {
        return m_pListBox->GetItemData(iItem);
    }

    /**
     * SComboBoxEx::SetItemData
     * @brief    设置附加数据
     * @param    UINT iItem -- 索引值
     * @param    LPARAM lParam -- 附加值
     *
     * Describe  设置附加数据
     */
    void SetItemData(UINT iItem, LPARAM lParam)
    {
        m_pListBox->SetItemData(iItem,lParam);
    }
    
    /**
     * SComboBoxEx::InsertItem
     * @brief    插入新项
     * @param    UINT iPos -- 位置
     * @param    LPCTSTR pszText -- 文本值
     * @param    int iIcon -- 图标
     * @param    LPARAM lParam -- 附加值
     *
     * Describe  插入新项
     */

    int InsertItem(UINT iPos,LPCTSTR pszText,int iIcon,LPARAM lParam)
    {
        int iInserted= m_pListBox->InsertItem(iPos,NULL,lParam);
        if(iInserted!=-1)
        {
            SWindow *pWnd=m_pListBox->GetItemPanel(iInserted);
            if(m_uTxtID!=0)
            {
                SWindow *pText=pWnd->FindChildByID(m_uTxtID);
                if(pText) pText->SetWindowText(pszText);
            }
            if(m_uIconID!=0)
            {
                SImageWnd *pIcon=pWnd->FindChildByID2<SImageWnd>(m_uIconID);
                if(pIcon) pIcon->SetIcon(iIcon);
            }
        }
        return iInserted;
    }
    /**
     * SComboBoxEx::DeleteString
     * @brief    删除某一项
     * @param    UINT iItem -- 索引值
     *
     * Describe  删除某一项
     */
    void DeleteString(UINT iItem)
    {
        m_pListBox->DeleteItem(iItem);
    }

    /**
     * SComboBoxEx::ResetContent
     * @brief    删除所有项
     *
     * Describe  设置附加数据
     */
    void ResetContent()
    {
        return m_pListBox->DeleteAllItems();
    }
    
    /**
     * SComboBoxEx::GetLBText
     * @brief    获取文本
     * @param    int iItem -- 索引值
     *
     * Describe  获取文本
     */
    SStringT GetLBText(int iItem);

    SListBoxEx * GetListBox(){return m_pListBox;}

protected:
    
    /**
     * SComboBox::OnSelChanged
     * @brief    下拉列表selected事件
     *
     * Describe  下拉列表selected事件
     */
    virtual void OnSelChanged();
protected:
    /**
     * SComboBoxEx::FireEvent
     * @brief    通知消息
     * @param    EventArgs &evt -- 事件对象 
     * 
     * Describe  此函数是消息响应函数
     */ 
    virtual BOOL FireEvent(EventArgs &evt);

    /**
     * SComboBoxEx::CreateListBox
     * @brief    创建下拉列表
     * @param    返回BOOL TRUE -- 成功 FALSE -- 失败
     *
     * Describe  创建下拉列表
     */
    virtual BOOL CreateListBox(pugi::xml_node xmlNode);

    /**
     * SComboBoxEx::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @param    返回int
     *
     * Describe  获取下拉列表高度
     */    
    virtual int  GetListBoxHeight();

    /**
     * SComboBoxEx::OnDropDown
     * @brief    下拉列表事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     *
     * Describe  下拉列表事件
     */
    virtual void OnDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBox::OnCloseUp
     * @brief    下拉列表关闭事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     * @param    UINT uCode -- 消息码
     *
     * Describe  获取下拉列表指针
     */
    virtual void OnCloseUp(SDropDownWnd *pDropDown,UINT uCode);

protected:

    SOUI_ATTRS_BEGIN()
        ATTR_UINT(L"id_text", m_uTxtID, FALSE)
        ATTR_UINT(L"id_icon", m_uIconID, FALSE)
    SOUI_ATTRS_END()

    SListBoxEx *m_pListBox;  /**< SListBox指针 */
    UINT   m_uTxtID;  /**< 文本ID */
    UINT   m_uIconID; /**< 图标ID */
};

}//namespace
