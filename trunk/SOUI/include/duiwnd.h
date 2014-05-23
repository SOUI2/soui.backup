//////////////////////////////////////////////////////////////////////////
//   File Name: DuiWnd.h
// Description: DuiWindow Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//                2011.09.01 - 2.0 huang jianxiong
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "DuiWindowMgr.h"
#include "DuiTimerEx.h"
#include "DuiContainer-i.h"
#include "duimsgcracker.h"

#include "duiwndnotify.h"
#include "gdialpha.h"
#include "duiref.h"
#include "DuiEventSet.h"
#include <OCIdl.h>
#include "DuiLayout.h"

namespace SOUI
{

/////////////////////////////////////////////////////////////////////////
enum {NormalShow=0,ParentShow=1};    //提供WM_SHOWWINDOW消息识别是父窗口显示还是要显示本窗口
enum {NormalEnable=0,ParentEnable=1};    //提供WM_ENABLE消息识别是父窗口可用还是直接操作当前窗口

#define DUIC_WANTARROWS     0x0001      /* Control wants arrow keys         */
#define DUIC_WANTTAB        0x0002      /* Control wants tab keys           */
#define DUIC_WANTRETURN        0x0004        /* Control wants return keys        */
#define DUIC_WANTCHARS      0x0008      /* Want WM_CHAR messages            */
#define DUIC_WANTALLKEYS    0xFFFF      /* Control wants all keys           */
#define DUIC_WANTSYSKEY        0x80000000    /* System Key */
class SOUI_EXP CDuiTimerID
{
public:
    DWORD    hDuiWnd:24;        //窗口句柄,如果窗口句柄超过24位范围，则不能使用这种方式设置定时器
    DWORD    uTimerID:7;        //定时器ID，一个窗口最多支持128个定时器。
    DWORD    bDuiTimer:1;    //区别通用定时器的标志，标志为1时，表示该定时器为DUI定时器

    CDuiTimerID(HDUIWND hWnd,char id)
    {
        DUIASSERT(hWnd<0x00FFFFFF && id>=0);
        bDuiTimer=1;
        hDuiWnd=hWnd;
        uTimerID=id;
    }
    CDuiTimerID(DWORD dwID)
    {
        memcpy(this,&dwID,sizeof(DWORD));
    }
    operator DWORD &() const
    {
        return *(DWORD*)this;
    }
};

#define ICWND_FIRST    ((CDuiWindow*)-1)
#define ICWND_LAST    NULL

class SOUI_EXP DuiDCPaint
{
public:
    DuiDCPaint()
        : bDuiModeChanged(FALSE)
        , nOldDuiMode(OPAQUE)
        , bFontChanged(FALSE)
        , hftOld(NULL)
        , bTextColorChanged(FALSE)
        , crOld(CLR_INVALID)
        , bBgColorChanged(FALSE)
        , crOldBg(CLR_INVALID)
    {
    }
    BOOL     bDuiModeChanged;
    int      nOldDuiMode;
    BOOL     bFontChanged;
    HFONT    hftOld;
    BOOL     bTextColorChanged;
    COLORREF crOld;
    BOOL     bBgColorChanged;
    COLORREF crOldBg;
    CRect    rcClient;
};



//////////////////////////////////////////////////////////////////////////
// CDuiWindow
//////////////////////////////////////////////////////////////////////////

typedef enum tagGDUI_CODE
{
    GDUI_FIRSTCHILD=0,
    GDUI_LASTCHILD,
    GDUI_PREVSIBLING,
    GDUI_NEXTSIBLING,
    GDUI_PARENT,
    GDUI_OWNER,
} GDUI_CODE;

class SOUI_EXP CDuiWindow : public CDuiObject
    , public CDuiEventSet
    , public CDuiRef
{
    SOUI_CLASS_NAME(CDuiWindow, "window")
    friend class CDuiLayout;
public:
    CDuiWindow();

    virtual ~CDuiWindow();
    virtual void OnFinalRelease();

    typedef struct tagDUIMSG
    {
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
    } DUIMSG,*PDUIMSG;
protected:
    HDUIWND m_hDuiWnd;
    IDuiContainer *m_pContainer;
    CDuiWindow *m_pOwner;
    CDuiWindow *m_pParent,*m_pFirstChild, *m_pLastChild, *m_pNextSibling,*m_pPrevSibling;    //窗口树结构
    UINT    m_nChildrenCount;
    DUIMSG        *m_pCurMsg;

    UINT m_uCmdID;
    CDuiStringA    m_strName;

    CRect m_rcWindow;

    DuiStyle m_style;
    CDuiStringT m_strInnerText;
    DWORD m_dwState;
    CDuiStringT m_strLinkUrl;
    BOOL m_bMsgTransparent;        //不处理用户操作标志
    BOOL m_bVisible;            //可见状态
    BOOL m_bDisplay;            //隐藏时是否占位，不占位时启动重新布局。
    BOOL m_bDisable;            //禁用状态
    CDuiStringT m_strToolTipText;
    int     m_nSepSpace;    //自动排版的水平空格
    BOOL m_bClipClient;
    BOOL m_bTabStop;
    BYTE m_byAlpha;        //窗口透明度,只进行配置，支持依赖于控件。

    CDuiSkinBase * m_pBgSkin;
    CDuiSkinBase * m_pNcSkin;

    DUIWND_POSITION m_dlgpos;

    int                m_nMaxWidth;    //自动计算大小时使用
    BOOL m_bUpdateLocked;//暂时锁定更新
#ifdef _DEBUG
    DWORD m_nMainThreadId;
#endif
    ULONG_PTR m_uData;
public:

    //////////////////////////////////////////////////////////////////////////
    // Method Define

    // Get align
    UINT GetTextAlign();    
    // Get position type
    DWORD GetPositionType();

    // Set position type
    void SetPositionType(DWORD dwPosType, DWORD dwMask = 0xFFFFFFFF);

    void SetFixSize(int nWid,int nHei);
    void SetBkColor(COLORREF cr);

    // Get DuiWindow rect(position in container)
    void GetRect(LPRECT prect);

    virtual void GetClient(LPRECT pRect);

    void GetDlgPosition(DUIWND_POSITION *pPos);
    // Get inner text
    LPCTSTR GetInnerText();


    // Set inner text
    HRESULT SetInnerText(LPCTSTR lpszText);

    VOID TestMainThread();

    // Send a message to DuiWindow
    LRESULT DuiSendMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0,BOOL *pbMsgHandled=NULL);

    PDUIMSG GetCurDuiMsg()
    {
        return m_pCurMsg;
    }

    // Move DuiWindow to new place
    void Move(LPRECT prect);

    void Move(int x,int y, int cx=-1,int cy=-1);

    // Set current cursor, when hover
    virtual BOOL OnDuiSetCursor(const CPoint &pt);

    // Get tooltip Info
    virtual BOOL OnUpdateToolTip(HDUIWND hCurTipHost,HDUIWND &hNewTipHost,CRect &rcTip,CDuiStringT &strTip);

    // Get DuiWindow state
    DWORD GetState(void);

    // Modify DuiWindow state
    DWORD ModifyState(DWORD dwStateAdd, DWORD dwStateRemove,BOOL bUpdate=FALSE);

    // Get Command ID
    UINT GetCmdID();
    void SetCmdID(UINT uNewID);

    ULONG_PTR GetUserData();
    ULONG_PTR SetUserData(ULONG_PTR uData);

    LPCSTR GetName(){ return m_strName.IsEmpty()?NULL:(LPCSTR)m_strName;}
    //************************************
    // Method:    SetDuiTimer
    // Function:  利用窗口定时器来设置一个ID为0-127的DUI定时器
    // Access:    public
    // Returns:   BOOL
    // Parameter: char id
    // Parameter: UINT uElapse
    // remark:
    //************************************
    BOOL SetDuiTimer(char id,UINT uElapse);

    //************************************
    // Method:    KillDuiTimer
    // Function:  删除一个DUI定时器
    // Access:    public
    // Returns:   void
    // Parameter: char id
    // remark:
    //************************************
    void KillDuiTimer(char id);

    //************************************
    // Method:    SetDuiTimerEx
    // Function:  利用函数定时器来模拟一个兼容窗口定时器
    // Access:    public
    // Returns:   BOOL
    // Parameter: UINT_PTR id
    // Parameter: UINT uElapse
    // remark: 能够使用SetDuiTimer时尽量不用SetDuiTimerEx，在Kill时效率会比较低
    //************************************
    BOOL SetDuiTimerEx(UINT_PTR id,UINT uElapse);

    //************************************
    // Method:    KillDuiTimerEx
    // Function:  删除一个SetDuiTimerEx设置的定时器
    // Access:    public
    // Returns:   void
    // Parameter: UINT_PTR id
    // remark: 需要枚举定时器列表
    //************************************
    void KillDuiTimerEx(UINT_PTR id);

    HDUIWND GetDuiHwnd();


    CDuiWindow *GetParent();

    void SetParent(CDuiWindow *pParent);

    CDuiWindow *GetTopLevelParent();

    BOOL DestroyChild(CDuiWindow *pChild);

    UINT GetChildrenCount();

    CDuiWindow * GetChild(UINT uCmdID);

    virtual void SetChildContainer(CDuiWindow *pChild);

    void InsertChild(CDuiWindow *pNewChild,CDuiWindow *pInsertAfter=ICWND_LAST);

    BOOL RemoveChild(CDuiWindow *pChild);

    BOOL IsChecked();

    BOOL IsDisabled(BOOL bCheckParent = FALSE);

    BOOL IsVisible(BOOL bCheckParent = FALSE);
    void SetVisible(BOOL bVisible,BOOL bUpdate=FALSE);

    void EnableWindow( BOOL bEnable,BOOL bUpdate=FALSE);

    void SetCheck(BOOL bCheck);

    BOOL NeedRedrawParent();

    LPCTSTR GetLinkUrl();

    IDuiContainer *GetContainer();

    void SetContainer(IDuiContainer *pContainer);

    void SetOwner(CDuiWindow *pOwner);

    CDuiWindow *GetOwner();

    BOOL IsMsgTransparent();

    DuiStyle& GetStyle();

    
    //************************************
    // Method:    FindChildByCmdID, 通过ID查找对应的子窗口
    // Access:    public 
    // Returns:   CDuiWindow*
    // Qualifier:
    // Parameter: UINT uCmdID
    //************************************
    CDuiWindow* FindChildByCmdID(UINT uCmdID);

    template<class T>
    T FindChildByCmdID2(UINT uCmdID)
    {
        return dynamic_cast<T>(FindChildByCmdID(uCmdID));
    }

    //************************************
    // Method:    FindChildByName，通过名字查找子窗口
    // Access:    public 
    // Returns:   CDuiWindow*
    // Qualifier:
    // Parameter: LPCSTR pszName
    //************************************
    CDuiWindow* FindChildByName(LPCSTR pszName);

    template<class T>
    T FindChildByName2(LPCSTR pszName)
    {
        return dynamic_cast<T>(FindChildByName(pszName));
    }

    // 从XML创建子窗口
    // LPCSTR utf8Xml: utf8 编码的XML串
    // return : 顶层的最后一个窗口
    CDuiWindow *LoadXmlChildren(LPCSTR utf8Xml);

    void NotifyInvalidate();
    void NotifyInvalidateRect(LPRECT lprect);
    void NotifyInvalidateRect(const CRect& rect);
    void LockUpdate();
    void UnlockUpdate();
    BOOL IsUpdateLocked();
    void BringWindowToTop();

    //同类控件自动成组标志,主要是给RadioButton用的。
    virtual BOOL IsSiblingsAutoGroupped(){return FALSE;}
public:
    //////////////////////////////////////////////////////////////////////////
    // Virtual functions
    virtual void OnSetCaretValidateRect(LPCRECT lpRect)
    {
        CRect rcClient;
        GetClient(&rcClient);
        CRect rcIntersect;
        rcIntersect.IntersectRect(&rcClient,lpRect);
        if(GetParent()) GetParent()->OnSetCaretValidateRect(&rcIntersect);
    }

    virtual void OnStateChanged(DWORD dwOldState,DWORD dwNewState) {}

    virtual BOOL LoadChildren(pugi::xml_node xmlNode);
    // Create DuiWindow from xml element
    virtual BOOL Load(pugi::xml_node xmlNode);

    virtual HDUIWND DuiGetHWNDFromPoint(CPoint ptHitTest, BOOL bOnlyText);

    virtual LRESULT DuiNotify(LPDUINMHDR pnms);

    virtual UINT OnGetDuiCode();

    virtual BOOL IsTabStop();

    virtual BOOL OnDuiNcHitTest(CPoint pt);


    virtual BOOL IsClipClient()
    {
        return m_bClipClient;
    }

    virtual void OnAttributeChanged(const CDuiStringA & strAttrName,BOOL bLoading,HRESULT hRet);

    //************************************
    // Method:    UpdateChildrenPosition :更新子窗口位置
    // FullName:  SOUI::CDuiWindow::UpdateChildrenPosition
    // Access:    virtual protected 
    // Returns:   void
    // Qualifier:
    //************************************
    virtual void UpdateChildrenPosition();

public:
    //************************************
    // Method:    RedrawRegion
    // Function:  将窗口及子窗口内容绘制到DC
    // Access:    public 
    // Returns:   BOOL
    // Qualifier:
    // Parameter: CDCHandle & dc
    // Parameter: CRgn & rgn
    //************************************
    BOOL RedrawRegion(CDCHandle& dc, CRgn& rgn);

    //************************************
    // Method:    GetDuiDC
    // Function:  获取一个与DUI窗口相适应的内存DC
    // Access:    public
    // Returns:   HDC
    // Parameter: LPRECT pRc - DC范围
    // Parameter: DWORD gdcFlags 同OLEDCFLAGS
    // Parameter: BOOL bClientDC 限制在client区域
    // remark: 使用ReleaseDuiDC释放
    //************************************
    HDC GetDuiDC(const LPRECT pRc=NULL,DWORD gdcFlags=0,BOOL bClientDC=TRUE);


    //************************************
    // Method:    ReleaseDuiDC
    // Function:  释放由GetDuiDC获取的DC
    // Access:    public
    // Returns:   void
    // Parameter: HDC hdc
    // remark:
    //************************************
    void ReleaseDuiDC(HDC hdc);

    //************************************
    // Method:    PaintBackground
    // Function:  画窗口的背景内容
    // Access:    public
    // Returns:   void
    // Parameter: HDC hdc 目标DC
    // Parameter: LPRECT pRc 目标位置
    // remark:    目标位置必须在窗口位置内
    //************************************
    void PaintBackground(HDC hdc,LPRECT pRc);

    //************************************
    // Method:    PaintForeground
    // Function:  画窗口的前景内容,不包括当前窗口的子窗口
    // Access:    public
    // Returns:   void
    // Parameter: HDC hdc 目标DC
    // Parameter: LPRECT pRc 目标位置
    // remark:    目标位置必须在窗口位置内
    //************************************
    void PaintForeground(HDC hdc,LPRECT pRc);


    //************************************
    // Method:    AnimateWindow
    // Function:  窗口动画效果
    // Access:    public
    // Returns:   BOOL
    // Parameter: DWORD dwTime,执行时间
    // Parameter: DWORD dwFlags,执行模式
    // remark:
    //************************************
    BOOL AnimateWindow(DWORD dwTime,DWORD dwFlags);
protected:
    typedef enum _PRSTATE{
        PRS_LOOKSTART=0,    //查找开始窗口
        PRS_DRAWING,        //窗口渲染中
        PRS_MEETEND            //碰到指定的结束窗口
    } PRSTATE;
    static BOOL _PaintRegion(CDCHandle& dc, CRgn& rgn,CDuiWindow *pWndCur,CDuiWindow *pStart,CDuiWindow *pEnd,CDuiWindow::PRSTATE & prState);


    CRect        m_rcGetDC;
    DWORD        m_gdcFlags;
    int            m_nSaveDC;
public:
    HDUIWND GetDuiCapture();
    HDUIWND SetDuiCapture();

    BOOL ReleaseDuiCapture();
    void SetDuiFocus();
    void KillDuiFocus();

    CDuiWindow *GetCheckedRadioButton();

    void CheckRadioButton(CDuiWindow * pRadioBox);

    BOOL SetItemVisible(UINT uItemID, BOOL bVisible);

    BOOL IsItemVisible(UINT uItemID, BOOL bCheckParent = FALSE);
    BOOL GetItemCheck(UINT uItemID);

    BOOL SetItemCheck(UINT uItemID, BOOL bCheck);
    BOOL EnableItem(UINT uItemID, BOOL bEnable);
    BOOL IsItemEnable(UINT uItemID, BOOL bCheckParent = FALSE);

    CDuiWindow *GetDuiWindow(int uCode);    

    //************************************
    // Method:    BeforePaint
    // Function:  为DC准备好当前窗口的绘图环境
    // Access:    public
    // Returns:   void
    // Parameter: CDCHandle & dc
    // Parameter: DuiDCPaint & DuiDC
    // remark:
    //************************************
    void BeforePaint(CDCHandle &dc, DuiDCPaint &DuiDC);

    //************************************
    // Method:    AfterPaint
    // Function:  恢复由BeforePaint设置的DC状态
    // Access:    public
    // Returns:   void
    // Parameter: CDCHandle & dc
    // Parameter: DuiDCPaint & DuiDC
    // remark:
    //************************************
    void AfterPaint(CDCHandle &dc, DuiDCPaint &DuiDC);

    //************************************
    // Method:    BeforePaintEx
    // Function:  为DC准备好当前窗口的绘图环境,从顶层窗口开始设置
    // Access:    public
    // Returns:   int 当前的DC环境
    // Parameter: CDCHandle & dc
    // remark: 使用前使用SaveDC来保存状态，使用后调用RestoreDC来恢复状态
    //************************************
    void BeforePaintEx(CDCHandle &dc);

    BOOL IsMsgHandled() const;

    void SetMsgHandled(BOOL bHandled);

protected:
    BOOL m_bMsgHandled;

    SOUO_ATTRIBUTES_BEGIN()
        DUIWIN_SKIN_ATTRIBUTE("skin", m_pBgSkin, TRUE)//直接获得皮肤对象
        DUIWIN_SKIN_ATTRIBUTE("ncskin", m_pNcSkin, TRUE)//直接获得皮肤对象
        DUIWIN_STRING_ATTRIBUTE("name",m_strName,FALSE)
        DUIWIN_STYLE_ATTRIBUTE("class", m_style, TRUE)    //获得style
        DUIWIN_CHAIN_ATTRIBUTE(m_style)                    //支持对style中的属性定制
        DUIWIN_INT_ATTRIBUTE("id", m_uCmdID, FALSE)
        DUIWIN_INT_ATTRIBUTE("data", m_uData, 0 )
        DUIWIN_CUSTOM_ATTRIBUTE("state", OnAttributeState)
        DUIWIN_TSTRING_ATTRIBUTE("href", m_strLinkUrl, FALSE)
        DUIWIN_TSTRING_ATTRIBUTE("tip", m_strToolTipText, FALSE)
        DUIWIN_CUSTOM_ATTRIBUTE("pos", OnAttributePosition)
        DUIWIN_INT_ATTRIBUTE("show", m_bVisible,FALSE)
        DUIWIN_INT_ATTRIBUTE("display", m_bDisplay,FALSE)
        DUIWIN_INT_ATTRIBUTE("msgtransparent", m_bMsgTransparent, FALSE)
        DUIWIN_INT_ATTRIBUTE("sep", m_nSepSpace, FALSE)
        DUIWIN_INT_ATTRIBUTE("maxwidth",m_nMaxWidth,FALSE)
        DUIWIN_INT_ATTRIBUTE("clipclient",m_bClipClient,FALSE)
        DUIWIN_INT_ATTRIBUTE("tabstop",m_bTabStop,FALSE)
        DUIWIN_ENUM_ATTRIBUTE("pos2type",POS2TYPE,FALSE)
            DUIWIN_ENUM_VALUE("lefttop",POS2_LEFTTOP)
            DUIWIN_ENUM_VALUE("center",POS2_CENTER)
            DUIWIN_ENUM_VALUE("righttop",POS2_RIGHTTOP)
            DUIWIN_ENUM_VALUE("leftbottom",POS2_LEFTBOTTOM)
            DUIWIN_ENUM_VALUE("rightbottom",POS2_RIGHTBOTTOM)
        DUIWIN_ENUM_END(m_dlgpos.pos2Type)
        DUIWIN_INT_ATTRIBUTE("alpha",m_byAlpha,TRUE)
    SOUI_ATTRIBUTES_END()

protected:
    LRESULT NotifyCommand();
    LRESULT NotifyContextMenu(CPoint pt);

    //************************************
    // Method:    GetChildrenLayoutRect :返回子窗口的排版空间
    // FullName:  SOUI::CDuiWindow::GetChildrenLayoutRect
    // Access:    virtual protected 
    // Returns:   CRect
    // Qualifier:
    //************************************
    virtual CRect GetChildrenLayoutRect();

    void ClearLayoutState();

    //************************************
    // Method:    GetDesiredSize: 当没有指定窗口大小时，通过如皮肤计算窗口的期望大小
    // FullName:  SOUI::CDuiWindow::GetDesiredSize
    // Access:    virtual protected 
    // Returns:   CSize
    // Qualifier:
    // Parameter: LPRECT pRcContainer
    //************************************
    virtual CSize GetDesiredSize(LPRECT pRcContainer);

    //************************************
    // Method:    CalcSize ：计算窗口大小
    // FullName:  SOUI::CDuiWindow::CalcSize
    // Access:    protected 
    // Returns:   CSize
    // Qualifier:
    // Parameter: LPRECT pRcContainer
    //************************************
    CSize CalcSize(LPRECT pRcContainer);

    //************************************
    // Method:    GetNextVisibleWindow 获得指定窗口的下一个可见窗口
    // FullName:  SOUI::CDuiWindow::GetNextVisibleWindow
    // Access:    protected static 
    // Returns:   CDuiWindow *    :下一个可见窗口
    // Qualifier:
    // Parameter: CDuiWindow * pWnd    :参考窗口
    // Parameter: const CRect &rcDraw:目标矩形
    //************************************
    static CDuiWindow *GetNextVisibleWindow(CDuiWindow *pWnd,const CRect &rcDraw);

    virtual BOOL NeedRedrawWhenStateChange();
    virtual void GetTextRect(LPRECT pRect);
    virtual void DuiDrawText(HDC hdc,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat);
    virtual void DuiDrawFocus(HDC dc);

    void DuiDrawDefFocusRect(CDCHandle dc,CRect rc);
    void DrawAniStep(CRect rcFore,CRect rcBack,HDC dcFore,HDC dcBack,CPoint ptAnchor);
    void DrawAniStep( CRect rcWnd,HDC dcFore,HDC dcBack,BYTE byAlpha);
    //////////////////////////////////////////////////////////////////////////
    // Message Handler

    //************************************
    // Method:    DuiWndProc
    // Function:  默认的消息处理函数
    // Access:    virtual public
    // Returns:   BOOL
    // Parameter: UINT uMsg
    // Parameter: WPARAM wParam
    // Parameter: LPARAM lParam
    // Parameter: LRESULT & lResult
    // remark: 在消息映射表中没有处理的消息进入该函数处理
    //************************************
    virtual BOOL DuiWndProc(UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT & lResult)
    {
        return FALSE;
    }

    LRESULT OnWindowPosChanged(LPRECT lpRcContainer);

    int OnCreate(LPVOID);

    void OnDestroy();

    // Draw background default
    BOOL OnEraseBkgnd(CDCHandle dc);

    // Draw inner text default
    void OnPaint(CDCHandle dc);


    //************************************
    // Method:    OnNcPaint
    // Function:  draw non-client area
    // Access:    protected
    // Returns:   void
    // Parameter: CDCHandle dc
    // remark:
    //************************************
    void OnNcPaint(CDCHandle dc);

    BOOL OnDefKeyDown(UINT nChar, UINT nFlags);

    void OnShowWindow(BOOL bShow, UINT nStatus);

    void OnEnable(BOOL bEnable,UINT nStatus);

    void OnLButtonDown(UINT nFlags,CPoint pt);

    void OnLButtonUp(UINT nFlags,CPoint pt);
    
    void OnRButtonDown(UINT nFlags, CPoint point);

    void OnMouseMove(UINT nFlags,CPoint pt) {}

    void OnMouseHover(WPARAM wParam, CPoint ptPos);

    void OnMouseLeave();

    BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    void OnSetDuiFocus();
    void OnKillDuiFocus();

    HRESULT OnAttributePosition(const CDuiStringA& strValue, BOOL bLoading);
    HRESULT OnAttributeState(const CDuiStringA& strValue, BOOL bLoading);

    WND_MSG_MAP_BEGIN()
        MSG_WM_ERASEBKGND(OnEraseBkgnd)
        MSG_WM_PAINT(OnPaint)
        MSG_WM_NCPAINT_EX(OnNcPaint)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_DUIWINPOSCHANGED(OnWindowPosChanged)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_ENABLE_EX(OnEnable)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
        MSG_WM_KILLFOCUS_EX(OnKillDuiFocus)
    WND_MSG_MAP_END_BASE()
};
}//namespace SOUI