/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       DuiCmnCtrl.h
 * @brief      通用控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-06-26
 * 
 * Describe    ComboBox控件
 */
#pragma once
#include "duiwnd.h"
#include "DuiRichEdit.h"
#include "DuiDropDown.h"
#include "duilistbox.h"
#include "duilistboxex.h"
#include "DuiCmnCtrl.h"

namespace SOUI
{

#define IDC_CB_EDIT          -100
#define IDC_DROPDOWN_LIST    -200

class CDuiComboBoxBase;


/**
 * @class      CComboEdit
 * @brief      可输入CommboBox
 * 
 * Describe    可输入下拉列表
 */
class CComboEdit:public SEdit
{
public:
    /**
     * CComboEdit::CComboEdit
     * @param    CDuiComboBoxBase *pOwner  -- 暂无       
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CComboEdit(CDuiComboBoxBase *pOwner);
    
    /**
     * CComboEdit::~CComboEdit
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~CComboEdit(){}
protected:
    /**
     * CComboEdit::OnMouseHover
     * @brief    键盘鼠标悬停事件
     * @param    WPARAM wParam 
     * @param    CPoint ptPos -- 鼠标所在位置
     * 
     * Describe  此函数是消息响应函数
     */
    void OnMouseHover(WPARAM wParam, CPoint ptPos);
    /**
     * CComboEdit::OnMouseLeave
     * @brief    键盘鼠标离开事件
     * 
     * Describe  此函数是消息响应函数
     */    
    void OnMouseLeave();
    /**
     * CComboEdit::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值 
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     * 
     * Describe  此函数是消息响应函数
     */   
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * CComboEdit::FireEvent
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

/**
 * @class      CComboEdit
 * @brief      可输入CommboBox
 * 
 * Describe    可输入下拉列表
 */
class SOUI_EXP CDuiComboBoxBase 
    : public SWindow
    , public ISDropDownOwner
{
    friend class CComboListBox;
public:
    
    /**
     * CDuiComboBoxBase::CDuiComboBoxBase
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiComboBoxBase(void);
    
    /**
     * CDuiComboBoxBase::~CDuiComboBoxBase
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~CDuiComboBoxBase(void);
    /**
     * CDuiComboBoxBase::GetCurSel
     * @brief    获取选中值索引
     * @return   返回int  
     *
     * Describe  获取当前选中索引
     */
    virtual int GetCurSel() const =0;

    /**
     * CDuiComboBoxBase::GetWindowText
     * @brief    获取窗口标题
     * @return   返回SStringT
     *
     * Describe  获取当前选中索引
     */
    virtual SStringT GetWindowText() =0;

    /**
     * CDuiComboBoxBase::DropDown
     * @brief    下拉事件
     *
     * Describe  下拉事件
     */
    void DropDown();
    
    /**
     * CDuiComboBoxBase::CloseUp
     * @brief    下拉关闭
     *
     * Describe  下拉关闭
     */
    void CloseUp();
protected:
    /**
     * CDuiComboBoxBase::GetDropDownOwner
     * @brief    获取owner
     * @return   SWindow
     *
     * Describe  获取owner
     */
    virtual SWindow* GetDropDownOwner();

    /**
     * CDuiComboBoxBase::OnDropDown
     * @brief    下拉事件
     * @param     SDropDownWnd *pDropDown -- 下拉窗口指针
     *
     * Describe  下拉事件
     */
    virtual void OnDropDown(SDropDownWnd *pDropDown);

    /**
     * CDuiComboBoxBase::OnCloseUp
     * @brief    下拉事件
     * @param     SDropDownWnd *pDropDown -- 下拉窗口指针
     * @param     UINT uCode -- 消息码
     *
     * Describe  下拉关闭
     */
    virtual void OnCloseUp(SDropDownWnd *pDropDown,UINT uCode);
protected:
    /**
     * CDuiComboBoxBase::OnSelChanged
     * @brief    下拉窗口改变事件
     *
     * Describe  下拉关闭
     */
    virtual void OnSelChanged();

    /**
     * CDuiComboBoxBase::FireEvent
     * @brief    通知消息
     * @param    EventArgs &evt -- 事件对象 
     * 
     * Describe  此函数是消息响应函数
     */ 
    virtual BOOL FireEvent(EventArgs &evt);
    
protected:

    /**
     * CDuiComboBoxBase::CalcPopupRect
     * @brief    计算弹出窗口位置
     * @param    int nHeight -- 下拉窗口高度
     * @param    CRect & rcPopup -- 保存弹出窗口Rect
     * @return   BOOL  TRUE -- 成功  FALSE -- 失败
     *
     * Describe  计算弹出窗口位置,保存在rcPopup中
     */    
    BOOL CalcPopupRect(int nHeight,CRect & rcPopup);
    
    /**
     * CDuiComboBoxBase::CreateListBox
     * @brief    创建下拉列表
     * @param    pugi::xml_node xmlNode  -- xml对象
     * @return   BOOL  TRUE -- 成功  FALSE -- 失败
     *
     * Describe  创建下拉列表
     */    
    virtual BOOL CreateListBox(pugi::xml_node xmlNode)=0;
    
    /**
     * CDuiComboBoxBase::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @return   返回int 高度
     *
     * Describe  获取下拉列表高度
     */        
    virtual int  GetListBoxHeight()=0;

    /**
     * CDuiComboBoxBase::GetDropBtnRect
     * @brief    获取下拉列表按钮位置
     * @param    LPRECT prc -- 按钮Rect
     *
     * Describe  获取下拉列表右侧按钮位置
     */        
    void GetDropBtnRect(LPRECT prc);
    /**
     * CDuiComboBoxBase::LoadChildren
     * @brief    加载子项
     * @param    pugi::xml_node xmlNode  -- xml文件
     * @return   返回BOOL  TRUE -- 成功 FALSE -- 失败
     *
     * Describe  加载子项
     */
    virtual BOOL CreateChildren(pugi::xml_node xmlNode);    
    /**
     * CDuiComboBoxBase::GetTextRect
     * @brief    获取文本位置
     * @param    LPRECT pRect -- 文本位置
     *
     * Describe  获取文本位置
     */
    virtual void GetTextRect(LPRECT pRect);
    /**
     * CDuiComboBoxBase::OnPaint
     * @brief    绘制消息
     * @param    IRenderTarget * pRT -- 暂无
     * 
     * Describe  此函数是消息响应函数
     */
    void OnPaint(IRenderTarget * pRT);
    
    /**
     * CDuiComboBoxBase::OnLButtonDown
     * @brief    左键按下事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     *
     * Describe  此函数是消息响应函数
     */
    void OnLButtonDown(UINT nFlags,CPoint pt);

    /**
     * CDuiComboBoxBase::OnMouseMove
     * @brief    键盘鼠标移动事件
     * @param    UINT nFlags -- 标志
     * @param    CPoint point -- 鼠标坐标
     * 
     * Describe  此函数是消息响应函数
     */
    void OnMouseMove(UINT nFlags,CPoint pt);

    /**
     * CDuiComboBoxBase::OnMouseLeave
     * @brief    键盘鼠标移动事件
     * 
     * Describe  此函数是消息响应函数
     */
    void OnMouseLeave();

    /**
     * CDuiComboBoxBase::OnKeyDown
     * @brief    键盘按下事件
     * @param    UINT nChar -- 按键对应的码值 
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     * 
     * Describe  此函数是消息响应函数
     */
    void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );

    /**
     * CDuiComboBoxBase::OnChar
     * @brief    字符消息
     * @param    UINT nChar -- 按键对应的码值 
     * @param    UINT nRepCnt -- 重复次数
     * @param    UINT nFlags -- 标志
     * 
     * Describe  此函数是消息响应函数
     */ 
    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    /**
     * CDuiComboBoxBase::OnDestroy
     * @brief    下拉窗口销毁
     * 
     * Describe  此函数是用于销毁下拉窗口
     */  
    void OnDestroy();
    /**
     * CDuiComboBoxBase::OnGetDuiCode
     * @brief    获取消息码
     * 
     * Describe  获取消息码
     */  
    UINT OnGetDlgCode();
    
    /**
     * CDuiComboBoxBase::IsTabStop
     * @brief    是否禁止TAB键
     * 
     * Describe  是否禁止TAB键
     */  
    BOOL IsTabStop();

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"dropdown", m_bDropdown, FALSE)
        ATTR_INT(L"dropheight", m_nDropHeight, FALSE)
         ATTR_INT(L"cursel", m_iInitSel, FALSE)
        ATTR_SKIN(L"btnskin", m_pSkinBtn, FALSE)
        ATTR_INT(L"animtime", m_iAnimTime, FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)        
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KEYDOWN(OnKeyDown) 
        MSG_WM_CHAR(OnChar)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

protected:
    /**
     * CDuiComboBoxBase::GetEditText
     * @brief    获取编辑框内容
     * 
     * Describe  获取编辑框内容
     */  
    SStringT GetEditText()
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
class SOUI_EXP SComboBox : public CDuiComboBoxBase
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
    int  GetCount()
    {
        return m_pListBox->GetCount();
    }
    /**
     * SComboBox::GetTextRect
     * @brief    获取文本位置
     * @param    LPRECT pRect -- 文本位置
     *
     * Describe  获取文本位置
     */
    SStringT GetWindowText()
    {
        if(!m_bDropdown)
        {
            return GetEditText();
        }
        if(m_pListBox->GetCurSel()==-1) return _T("");
        return GetLBText(m_pListBox->GetCurSel());
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

class SOUI_EXP SComboBoxEx : public CDuiComboBoxBase
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
    int  GetCount()
    {
        return m_pListBox->GetItemCount();
    }
    
    /**
     * SComboBoxEx::GetTextRect
     * @brief    获取文本位置
     * @param    LPRECT pRect -- 文本位置
     *
     * Describe  获取文本位置
     */
    SStringT GetWindowText()
    {
        if(!m_bDropdown) return GetEditText();
        return GetLBText(m_pListBox->GetCurSel());
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
    SStringT GetLBText(int iItem)
    {
        if(m_uTxtID == 0 || iItem<0 || iItem>= GetCount()) return _T("");
        SWindow *pItem=m_pListBox->GetItemPanel(iItem);
        SWindow *pText=pItem->FindChildByID(m_uTxtID);
        if(!pText) return _T("");
        return pText->GetWindowText();
    }

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
