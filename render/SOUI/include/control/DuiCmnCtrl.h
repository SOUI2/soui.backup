/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       DuiCmnCtrl.h
 * @brief      通用控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-05-28
 * 
 * Describe    此文件中定义了很多通用控件:静态文本，超链接，按钮，单选按钮等
 */

#pragma once
#include "duiwnd.h"
#include "DuiPanel.h"
#include "duiwndnotify.h"
#include "Accelerator.h"

namespace SOUI
{
/**
 * @class      CDuiSpacing
 * @brief      空格控件类
 * 
 * Describe    空格控件类
 * Usage       <spacing width=xx />
 */
class SOUI_EXP CDuiSpacing : public SWindow
{
    SOUI_CLASS_NAME(CDuiSpacing, "spacing")
public:
    // Do nothing
    void OnPaint(CDCHandle dc){}

protected:
    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    WND_MSG_MAP_END()
};

/**
 * @class      CDuiStatic
 * @brief      静态文本控件类
 * 
 * Describe    静态文本控件可支持多行，有多行属性时，\n可以强制换行
 * Usage       <text>inner text example</text>
 */
class SOUI_EXP CDuiStatic : public SWindow
{
    SOUI_CLASS_NAME(CDuiStatic, "text")
public:
    /**
     * CDuiStatic::CDuiStatic
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiStatic():m_bMultiLines(0),m_nLineInter(5)
    {
        m_bMsgTransparent=TRUE;
        m_style.SetAttribute("align","left");
    }
    /**
     * CDuiStatic::DuiDrawText
     * @brief    绘制文本
     * @param    HDC hdc -- 绘制设备句柄         
     * @param    LPCTSTR pszBuf -- 文本内容字符串         
     * @param    int cchText -- 字符串长度         
     * @param    LPRECT pRect -- 指向矩形结构RECT的指针         
     * @param    UINT uFormat --  正文的绘制选项         
     *
     * Describe  对DrawText封装
     */    
    virtual void DuiDrawText(HDC hdc,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat);

protected:
    int m_bMultiLines;  /**< 是否开启多行显示 */  
    int m_nLineInter;   /**< 不详 有待完善 */

    SOUI_ATTRS_BEGIN()
    ATTR_INT("multilines", m_bMultiLines, FALSE)
    ATTR_INT("interhei", m_nLineInter, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class      CDuiLink
 * @brief      超链接控件类
 * 
 * Describe    Only For Header Drag Test
 * Usage       <link>inner text example</link>
 */
class SOUI_EXP CDuiLink : public SWindow
{
    SOUI_CLASS_NAME(CDuiLink, "link")

public:
    /**
     * CDuiLink::CDuiLink
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiLink()
    {
        m_style.SetAttribute("align","left");
    }

protected:
    /**
     * CDuiLink::OnAttributeFinish
     * @brief    解析xml设置属性
     *
     * Describe  根据xml文件设置相关属性
     */
    virtual void OnAttributeFinish(pugi::xml_node xmlNode);
    /**
     * CDuiLink::DuiDrawText
     * @brief    绘制文本
     * @param    HDC hdc -- 绘制设备句柄         
     * @param    LPCTSTR pszBuf -- 文本内容字符串         
     * @param    int cchText -- 字符串长度         
     * @param    LPRECT pRect -- 指向矩形结构RECT的指针         
     * @param    UINT uFormat --  正文的绘制选项         
     *
     * Describe  对DrawText封装
     */
    virtual void DuiDrawText(HDC hdc,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat);

    /**
     * CDuiLink::OnDuiSetCursor
     * @brief    设置光标样式和位置
     * @param    CPoint &pt -- 设置光标位置
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  函数内部会加载光标样式
     */
    virtual BOOL OnDuiSetCursor(const CPoint &pt);

    void OnLButtonDown(UINT nFlags,CPoint pt);
    void OnLButtonUp(UINT nFlags,CPoint pt);
    void OnMouseMove(UINT nFlags,CPoint pt);
    void OnMouseHover(WPARAM wParam, CPoint ptPos);

    WND_MSG_MAP_BEGIN()
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEHOVER(OnMouseHover)
    WND_MSG_MAP_END()

    CRect m_rcText;  /**< 文本显示所在位置 */
};

/**
 * @class      CDuiButton
 * @brief      按钮控件类
 * 
 * Describe    通过属性ID绑定click事件 Use id attribute to process click event
 * Usage       <button id=xx>inner text example</button>
 */
class SOUI_EXP CDuiButton : public SWindow
    , public IAcceleratorTarget
    , public ITimelineHandler
{
    SOUI_CLASS_NAME(CDuiButton, "button")
public:
    /**
     * CDuiButton::CDuiButton
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiButton();
protected:
    /**
     * CDuiLink::NeedRedrawWhenStateChange
     * @brief    状态变化需要重画
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  当按钮状态发生变化时候需要重新绘制 默认返回TRUE
     */
    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }
    /**
     * CDuiLink::OnGetDuiCode
     * @brief    获得编码
     *
     * Describe  返回宏定义DUIC_WANTCHARS代表需要WM_CHAR消息
     */
    virtual UINT OnGetDuiCode()
    {
        return DUIC_WANTCHARS;
    }

    /**
     * CDuiLink::OnAcceleratorPressed
     * @brief    加速键按下
     * @param    CAccelerator& accelerator -- 加速键相关结构体
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  处理加速键响应消息
     */
    virtual bool OnAcceleratorPressed(const CAccelerator& accelerator);
protected:
    /**
     * CDuiLink::GetDesiredSize
     * @brief    获得期望的大小值
     * @param    LPRECT pRcContainer -- 内容窗体矩形
     *
     * Describe  根据内容窗体矩形大小，计算出适合的大小
     */
    virtual CSize GetDesiredSize(LPRECT pRcContainer);
    
    /**
     * CDuiLink::OnStateChanged
     * @brief    状态改变处理函数
     * @param    DWORD dwOldState -- 旧状态
     * @param    DWORD dwNewState -- 新状态
     *
     * Describe  状态改变处理函数
     */
    virtual void OnStateChanged(DWORD dwOldState,DWORD dwNewState);
    
    void OnPaint(CDCHandle dc);

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnDestroy();

    void OnSize(UINT nType, CSize size);

    BOOL OnEraseBkgnd(CDCHandle dc){return TRUE;}

    HRESULT OnAttrAccel(CDuiStringA strAccel,BOOL bLoading);

protected:
    virtual void OnNextFrame();
    
    /**
     * CDuiLink::StopCurAnimate
     * @brief    停止动画
     *
     * Describe  停止动画
     */
    void StopCurAnimate();

    DWORD  m_accel;
    BOOL   m_bAnimate;    /**< 动画标志 */
    BYTE   m_byAlphaAni;  /**< 动画状态 */
public:
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM("accel",OnAttrAccel)
        ATTR_INT("animate", m_bAnimate, FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KEYUP(OnKeyUp)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
    WND_MSG_MAP_END()
};

/**
 * @class      CDuiImageBtnWnd
 * @brief      图片按钮类
 * 
 * Describe    图片按钮类，继承CDuiButton
 */
class SOUI_EXP CDuiImageBtnWnd : public CDuiButton
{
    SOUI_CLASS_NAME(CDuiImageBtnWnd, "imgbtn")
public:
    CDuiImageBtnWnd()
    {
        m_bTabStop=FALSE;
    }
};

/**
 * @class      CDuiImageWnd
 * @brief      图片控件类
 * 
 * Describe    Image Control 图片控件类
 * Usage       Usage: <img skin="skin" sub="0"/>
 */
class SOUI_EXP CDuiImageWnd : public SWindow
{
    SOUI_CLASS_NAME(CDuiImageWnd, "img")
public:
    /**
     * CDuiImageWnd::CDuiImageWnd
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiImageWnd();
    /**
     * CDuiImageWnd::~CDuiImageWnd
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~CDuiImageWnd();

    void OnPaint(CDCHandle dc);
    /**
     * CDuiImageWnd::SetSkin
     * @param    CDuiSkinBase *pSkin -- 资源
     * @param    int nSubID -- 资源ID
     * @brief    设置皮肤
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  设置皮肤
     */
    BOOL SetSkin(ISkinObj *pSkin,int nSubID=0);
    /**
     * CDuiImageWnd::SetIcon
     * @param    int nSubID -- 资源ID
     * @brief    设置图标
     * @return   返回值BOOL 成功--TRUE 失败--FALSE
     *
     * Describe  设置图标
     */
    BOOL SetIcon(int nSubID);

    /**
     * CDuiImageWnd::GetSkin
     * @brief    获取资源
     * @return   返回值CDuiSkinBase指针
     *
     * Describe  获取资源
     */
    ISkinObj * GetSkin(){return m_pSkin;}
protected:
    /**
     * CDuiImageWnd::GetDesiredSize
     * @brief    获取预期大小
     * @param    LPRECT pRcContainer  --  内容矩形框 
     * @return   返回值 CSize对象 
     *
     * Describe  根据矩形的大小，获取预期大小(解释有点不对)
     */
    virtual CSize GetDesiredSize(LPRECT pRcContainer);

    BOOL m_bManaged;  /**< 暂时不详 */
    int m_nSubImageID;  /**< 资源图片ID */
    ISkinObj *m_pSkin;  /**< 资源对象 */
    //BOOL m_bCalc;

    SOUI_ATTRS_BEGIN()
    ATTR_SKIN("skin", m_pSkin, TRUE)
    ATTR_INT("sub", m_nSubImageID, FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    WND_MSG_MAP_END()
};

/**
 * @class      CDuiAnimateImgWnd
 * @brief      动画图片窗口
 * 
 * Describe    此窗口支持动画效果
 */
class SOUI_EXP CDuiAnimateImgWnd : public SWindow, public ITimelineHandler
{
    SOUI_CLASS_NAME(CDuiAnimateImgWnd, "animateimg")
public:    
    /**
     * CDuiAnimateImgWnd::CDuiImageWnd
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiAnimateImgWnd();

    /**
     * CDuiAnimateImgWnd::~CDuiAnimateImgWnd
     * @brief    析构函数
     *
     * Describe  析构函数
     */    
    virtual ~CDuiAnimateImgWnd() {}

    /**
     * CDuiAnimateImgWnd::Start
     * @brief    启动动画
     *
     * Describe  启动动画
     */  
    void Start();
    /**
     * CDuiAnimateImgWnd::Stop
     * @brief    停止动画
     *
     * Describe  停止动画
     */  
    void Stop();

    /**
     * CDuiAnimateImgWnd::IsPlaying
     * @brief    判断动画运行状态
     * @return   返回值是动画状态 TRUE -- 运行中
     *
     * Describe  判断动画运行状态
     */  
    BOOL IsPlaying(){return m_bPlaying;}
protected:
    /**
     * CDuiAnimateImgWnd::GetDesiredSize
     * @brief    获取预期大小
     * @param    LPRECT pRcContainer  --  内容矩形框 
     * @return   返回值 CSize对象 
     *
     * Describe  根据矩形的大小，获取预期大小(解释有点不对)
     */
    virtual CSize GetDesiredSize(LPRECT pRcContainer);
    virtual void OnNextFrame();

    void OnPaint(CDCHandle dc);

    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnDestroy();

    WND_MSG_MAP_BEGIN()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SHOWWINDOW(OnShowWindow)
    WND_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
    ATTR_SKIN("skin", m_pSkin, TRUE)
    ATTR_UINT("speed", m_nSpeed, FALSE)
    ATTR_UINT("autostart", m_bAutoStart, FALSE)
    SOUI_ATTRS_END()

protected:
    ISkinObj *m_pSkin;        /**< 暂时不祥 */
    int           m_nSpeed;       /**< 速度 */
    int           m_iCurFrame;    /**< 当前帧 */
    BOOL          m_bAutoStart;   /**< 是否自动启动 */
    BOOL          m_bPlaying;     /**< 是否运行中 */
};

/**
 * @class      CDuiProgress
 * @brief      进度条类
 * 
 * Describe    进度条类
 * Usage: <progress bgskin=xx posskin=xx min=0 max=100 value=10,showpercent=0/>
 */
class SOUI_EXP CDuiProgress : public SWindow
{
    SOUI_CLASS_NAME(CDuiProgress, "progress")
public:
    /**
     * CDuiProgress::CDuiProgress
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiProgress();

    /**
     * CDuiProgress::SetValue
     * @brief    设置进度条进度值
     * @param    int nValue  --  进度值 
     * @return   返回值是 TRUE -- 设置成功
     *
     * Describe  设置进度条进度值
     */  
    BOOL SetValue(int nValue);
    /**
     * CDuiProgress::GetValue
     * @brief    获取进度值
     * @return   返回值是int 
     *
     * Describe  获取进度值
     */  
    int GetValue(){return m_nValue;}

    /**
     * CDuiProgress::SetRange
     * @param    int nMin  --  进度最小值 
     * @param    int nMax  --  进度最大值      
     * @brief    设置进度值最小大值
     *
     * Describe  设置进度值
     */  
    void SetRange(int nMin,int nMax);
    /**
     * CDuiProgress::GetRange
     * @param    int nMin  --  进度最小值 
     * @param    int nMax  --  进度最大值      
     * @brief    获取进度值最小大值
     *
     * Describe  获取进度值
     */  
    void GetRange(int *pMin,int *pMax);
    /**
     * CDuiProgress::IsVertical
     * @brief    判断进度条是否为竖直状态
     * @return   返回值是 TRUE -- 竖直状态
     *
     * Describe  获取进度值
     */  
    BOOL IsVertical(){return m_bVertical;}
protected:
    /**
     * CDuiProgress::GetDesiredSize
     * @brief    获取预期大小
     * @param    LPRECT pRcContainer  --  内容矩形框 
     * @return   返回值 CSize对象 
     *
     * Describe  根据矩形的大小，获取预期大小(解释有点不对)
     */
    virtual CSize GetDesiredSize(LPRECT pRcContainer);

    void OnPaint(CDCHandle dc);

    int m_nMinValue; /**< 进度最小值 */
    int m_nMaxValue; /**< 进度最大值 */
    int m_nValue;    /**< 进度值 */

    BOOL m_bShowPercent; /**< 是否显示百分比 */
    BOOL m_bVertical;    /**< 是否竖直状态 */

    ISkinObj *m_pSkinBg;   /**< 暂时不详 */
    ISkinObj *m_pSkinPos;  /**< 暂时不详 */

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    WND_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
    ATTR_SKIN("bgskin", m_pSkinBg, TRUE)
    ATTR_SKIN("posskin", m_pSkinPos, TRUE)
    ATTR_INT("min", m_nMinValue, FALSE)
    ATTR_INT("max", m_nMaxValue, FALSE)
    ATTR_INT("value", m_nValue, FALSE)
    ATTR_UINT("vertical", m_bVertical, FALSE)
    ATTR_UINT("showpercent", m_bShowPercent, FALSE)
    SOUI_ATTRS_END()
};

/**
 * @class      CDuiLine
 * @brief      线条控件
 * 
 * Describe    线条控件
 * Usage: <hr style=solid size=1 crbg=.../>
 */
class SOUI_EXP CDuiLine : public SWindow
{
    SOUI_CLASS_NAME(CDuiLine, "hr")

public:
    /**
     * CDuiLine::CDuiLine
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiLine();

    // Do nothing
    void OnPaint(CDCHandle dc);
protected:
    int m_nPenStyle;
    int m_nLineSize;
    BOOL m_bLineShadow;
    COLORREF m_crShadow;
    enum HRMODE{
        HR_HORZ=0,
        HR_VERT,
        HR_TILT,
    }m_mode;

    SOUI_ATTRS_BEGIN()
    ATTR_INT("size", m_nLineSize, FALSE)
    ATTR_UINT("shadow", m_bLineShadow, FALSE)
    ATTR_COLOR("crshadow", m_crShadow, FALSE)
    ATTR_ENUM_BEGIN("mode", HRMODE, FALSE)
        ATTR_ENUM_VALUE("vertical", HR_VERT)
        ATTR_ENUM_VALUE("horizon", HR_VERT)
        ATTR_ENUM_VALUE("tilt", HR_VERT)
    ATTR_ENUM_END(m_mode)
    ATTR_ENUM_BEGIN("style", int, FALSE)
    ATTR_ENUM_VALUE("solid", PS_SOLID)             // default
    ATTR_ENUM_VALUE("dash", PS_DASH)               /* -------  */
    ATTR_ENUM_VALUE("dot", PS_DOT)                 /* .......  */
    ATTR_ENUM_VALUE("dashdot", PS_DASHDOT)         /* _._._._  */
    ATTR_ENUM_VALUE("dashdotdot", PS_DASHDOTDOT)   /* _.._.._  */
    ATTR_ENUM_END(m_nPenStyle)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    WND_MSG_MAP_END()
};

/**
 * @class      CDuiCheckBox
 * @brief      复选框控件
 * 
 * Describe    复选框控件
 * Usage: <check state=4>This is a check-box</check>
 */
class SOUI_EXP CDuiCheckBox : public SWindow
{
    SOUI_CLASS_NAME(CDuiCheckBox, "check")

    enum
    {
        CheckBoxSpacing = 4,
    };

public:
    /**
     * CDuiCheckBox::CDuiCheckBox
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiCheckBox();

    void OnPaint(CDCHandle dc);
protected:

    ISkinObj *m_pSkin;   /**< 暂时不详 */
    ISkinObj *m_pFocusSkin; /**< 暂时不详 */
    /**
     * CDuiCheckBox::_GetDrawState
     * @brief    获得复选框状态
     * @return   返回状态值
     *
     * Describe  获取复选框状态
     */
    UINT _GetDrawState();
    /**
     * CDuiCheckBox::GetCheckRect
     * @brief    获得复选框矩形
     * @return   返回值CRect矩形框
     *
     * Describe  获取复选框矩形
     */
    CRect GetCheckRect();
    /**
     * CDuiCheckBox::GetDesiredSize
     * @brief    获取预期大小
     * @param    LPRECT pRcContainer  --  内容矩形框 
     * @return   返回值 CSize对象 
     *
     * Describe  根据矩形的大小，获取预期大小(解释有点不对)
     */
    virtual CSize GetDesiredSize(LPRECT pRcContainer);
    /**
     * CDuiCheckBox::GetTextRect
     * @brief    获取文本大小
     * @param    LPRECT pRect  --  内容矩形框 
     *
     * Describe  设置矩形的大小
     */
    virtual void GetTextRect(LPRECT pRect);
    /**
     * CDuiCheckBox::NeedRedrawWhenStateChange
     * @brief    判断状态改变是否需要重画
     * @return   返回值 BOOL 
     *
     * Describe  状态改变是否需要重画
     */
    virtual BOOL NeedRedrawWhenStateChange()
    {
        return TRUE;
    }

    /**
     * CDuiCheckBox::OnGetDuiCode
     * @brief    返回对应消息码
     * @return   返回值 UINT 
     *
     * Describe  返回对应消息码
     */
    virtual UINT OnGetDuiCode()
    {
        return DUIC_WANTCHARS;
    }
    /**
     * CDuiCheckBox::DuiDrawFocus
     * @brief    绘制获取焦点
     * @param    HDC dc  --  设备句柄
     *
     * Describe  返回对应消息码
     */
    virtual void DuiDrawFocus(HDC dc);

    void OnLButtonDown(UINT nFlags, CPoint point);

    void OnLButtonUp(UINT nFlags, CPoint point);

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    SOUI_ATTRS_BEGIN()
        ATTR_SKIN("skin", m_pSkin, FALSE)
        ATTR_SKIN("focusskin", m_pFocusSkin, FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
        MSG_WM_PAINT(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_KEYDOWN(OnKeyDown)
    WND_MSG_MAP_END()
};

/**
 * @class      CDuiIconWnd
 * @brief      图标控件
 * 
 * Describe    图标控件 Use src attribute specify a resource id
 * Usage: <icon src=xx size="16"/>
 */
class SOUI_EXP CDuiIconWnd : public SWindow
{
    SOUI_CLASS_NAME(CDuiIconWnd, "icon")
public:    
    /**
     * CDuiIconWnd::CDuiIconWnd
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiIconWnd();
    /**
     * CDuiIconWnd::Load
     * @brief    加载资源
     * @param    pugi::xml_node xmlNode  --  资源配置文件
     * @return   返回值BOOL 
     *
     * Describe  加载图标资源
     */
    virtual BOOL Load(pugi::xml_node xmlNode);

    void OnPaint(CDCHandle dc);
    /**
     * CDuiIconWnd::AttachIcon
     * @brief    附加图标资源
     * @param    HICON hIcon -- 图标资源句柄
     * @return   返回值 HICON 
     *
     * Describe  附加图标资源
     */
    HICON AttachIcon(HICON hIcon);
    /**
     * CDuiIconWnd::LoadIconFile
     * @brief    加载资源
     * @param    LPCTSTR lpFIleName -- 资源文件
     *
     * Describe  通过文件加载图标
     */
    void LoadIconFile( LPCTSTR lpFIleName );
protected:
    virtual CSize GetDesiredSize(LPRECT pRcContainer);
    /**
     * CDuiIconWnd::_ReloadIcon
     * @brief    重新加载图标资源
     *
     * Describe  重新加载图标资源
     */
    void _ReloadIcon();

    HICON m_theIcon; /**< 图标资源句柄 */
    CDuiStringT m_strIconName; /**< 图标资源名称 */
    CDuiStringT m_strCurIconName; /**< 当前图标资源名称 */
    int m_nSize; /**< 暂时不祥 */

    SOUI_ATTRS_BEGIN()
    ATTR_STRINGT("src", m_strIconName, FALSE)
    ATTR_INT("size", m_nSize, FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    WND_MSG_MAP_END()
};


/**
 * @class      CDuiRadioBox
 * @brief      单选框控件
 * 
 * Describe    单选框控件
 * Usage: <radio state=1>This is a check-box</radio>
 */
class SOUI_EXP CDuiRadioBox : public SWindow
{
    SOUI_CLASS_NAME(CDuiRadioBox, "radio")

    enum
    {
        RadioBoxSpacing = 4,
    };

public:
    /**
     * CDuiRadioBox::CDuiRadioBox
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiRadioBox();


    void OnPaint(CDCHandle dc);

protected:

    // CDuiRadioBoxTheme m_theme;
    ISkinObj *m_pSkin;  /**< 皮肤资源 */
    ISkinObj *m_pFocusSkin; /**< 焦点皮肤资源 */
    /**
     * CDuiRadioBox::CDuiRadioBox
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    UINT _GetDrawState(); 
    /**
     * CDuiRadioBox::CDuiRadioBox
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CRect GetRadioRect();
    /**
     * CDuiRadioBox::GetTextRect
     * @brief    获得文本大小
     * @param    LPRECT pRect -- 文本大小Rect
     *
     * Describe  构造函数
     */
    virtual void GetTextRect(LPRECT pRect);
    /**
     * CDuiRadioBox::GetDesiredSize
     * @brief    获取预期大小值
     * @param    LPRECT pRcContainer -- 内容窗口Rect
     *
     * Describe  获取预期大小值
     */
    virtual CSize GetDesiredSize(LPRECT pRcContainer);
    /**
     * CDuiRadioBox::NeedRedrawWhenStateChange
     * @brief    当状态改变时候是否需要重绘
     * @return   返回BOOL
     *
     * Describe  当状态改变时候是否需要重绘
     */
    virtual BOOL NeedRedrawWhenStateChange();
    /**
     * CDuiRadioBox::DuiDrawFocus
     * @brief    绘制焦点样式
     * @param    HDC dc -- 绘制设备
     *
     * Describe  当获得焦点时候需要绘制
     */
    virtual void DuiDrawFocus(HDC dc);
    /**
     * CDuiRadioBox::OnGetDuiCode
     * @brief    获取消息编码
     * @return   返回编码值
     *
     * Describe  获取消息编码
     */
    virtual UINT OnGetDuiCode()
    {
        return 0;
    }
    /**
     * CDuiRadioBox::IsSiblingsAutoGroupped
     * @brief    是否自动添加到同一组
     * @return   返回BOOL 
     *
     * Describe  相同名称的单选按钮是否自动添加到同一组中
     */
    virtual BOOL IsSiblingsAutoGroupped() {return TRUE;}

    void OnLButtonDown(UINT nFlags, CPoint point);

    void OnSetDuiFocus();


    SOUI_ATTRS_BEGIN()
    ATTR_SKIN("skin", m_pSkin, FALSE)
    ATTR_SKIN("focusskin", m_pFocusSkin, FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
    WND_MSG_MAP_END()
};

/**
 * @class      CDuiToggle
 * @brief      Toggle控件
 * 
 * Describe    Toggle控件
 */
class SOUI_EXP CDuiToggle : public SWindow
{
    SOUI_CLASS_NAME(CDuiToggle, "togglectrl")
public:
    
    /**
     * CDuiToggle::CDuiToggle
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    CDuiToggle();
    /**
     * CDuiToggle::SetToggle
     * @brief    设置Toggle属性
     * @param    BOOL bToggle -- 是否启用Toggle特效         
     * @param    BOOL bUpdate -- 是否更新 默认值TRUE
     *
     * Describe  设置Toggle属性
     */
    void SetToggle(BOOL bToggle,BOOL bUpdate=TRUE);
    /**
     * CDuiToggle::GetToggle
     * @brief    获取Toggle属性
     * @return   返回值BOOL        
     *
     * Describe  获取Toggle属性 主要是获取是否Toggle
     */
    BOOL GetToggle();
protected:
    void OnPaint(CDCHandle dc);
    void OnLButtonUp(UINT nFlags,CPoint pt);
    virtual CSize GetDesiredSize(LPRECT pRcContainer);
    virtual BOOL NeedRedrawWhenStateChange(){return TRUE;}

    SOUI_ATTRS_BEGIN()
    ATTR_INT("toggled", m_bToggled, TRUE)
    ATTR_SKIN("skin", m_pSkin, TRUE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    WND_MSG_MAP_END()
protected:
    BOOL m_bToggled;
    ISkinObj *m_pSkin;
};

/**
 * @class      CDuiGroup
 * @brief      组控件
 * 
 * Describe    组控件
 * Usage       <group crline1="b8d5e2" crline2="999999">group text</>
 */
class SOUI_EXP CDuiGroup : public SWindow
{
    SOUI_CLASS_NAME(CDuiGroup, "group")
public:
    CDuiGroup();

protected:
    void OnPaint(CDCHandle dc);
    COLORREF m_crLine1,m_crLine2; /**< 颜色 */
    int         m_nRound; /**< 暂时不详 */
public:
    SOUI_ATTRS_BEGIN()
    ATTR_COLOR("crline1", m_crLine1, FALSE)
    ATTR_COLOR("crline2", m_crLine2, FALSE)
    ATTR_INT("round",m_nRound,FALSE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_PAINT(OnPaint)
    WND_MSG_MAP_END()
};

}//namespace SOUI
