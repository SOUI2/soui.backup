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

#include "gdialpha.h"
#include "event/EventSubscriber.h"
#include "event/events.h"
#include "event/EventSet.h"
#include <OCIdl.h>
#include "DuiLayout.h"

namespace SOUI
{

/////////////////////////////////////////////////////////////////////////
enum {NormalShow=0,ParentShow=1};    //提供WM_SHOWWINDOW消息识别是父窗口显示还是要显示本窗口
enum {NormalEnable=0,ParentEnable=1};    //提供WM_ENABLE消息识别是父窗口可用还是直接操作当前窗口

#define DUIC_WANTARROWS     0x0001      /* Control wants arrow keys         */
#define DUIC_WANTTAB        0x0002      /* Control wants tab keys           */
#define DUIC_WANTRETURN     0x0004      /* Control wants return keys        */
#define DUIC_WANTCHARS      0x0008      /* Want WM_CHAR messages            */
#define DUIC_WANTALLKEYS    0xFFFF      /* Control wants all keys           */
#define DUIC_WANTSYSKEY     0x80000000    /* System Key */
class SOUI_EXP STimerID
{
public:
    DWORD    Swnd:24;        //窗口句柄,如果窗口句柄超过24位范围，则不能使用这种方式设置定时器
    DWORD    uTimerID:7;        //定时器ID，一个窗口最多支持128个定时器。
    DWORD    bDuiTimer:1;    //区别通用定时器的标志，标志为1时，表示该定时器为DUI定时器

    STimerID(SWND hWnd,char id)
    {
        ASSERT(hWnd<0x00FFFFFF && id>=0);
        bDuiTimer=1;
        Swnd=hWnd;
        uTimerID=id;
    }
    STimerID(DWORD dwID)
    {
        memcpy(this,&dwID,sizeof(DWORD));
    }
    operator DWORD &() const
    {
        return *(DWORD*)this;
    }
};

#define ICWND_FIRST    ((SWindow*)-1)
#define ICWND_LAST    NULL

class SOUI_EXP SPainter
{
public:
    SPainter(): crOld(CR_INVALID)
    {
    }

    CAutoRefPtr<IFont> pOldPen;
    COLORREF          crOld;
};

class SOUI_EXP SMsgHandleState
{
public:
    SMsgHandleState():m_bMsgHandled(FALSE)
    {

    }

    BOOL IsMsgHandled() const
    {
        return m_bMsgHandled;
    }

    void SetMsgHandled(BOOL bHandled)
    {
        m_bMsgHandled = bHandled;
    }

    BOOL m_bMsgHandled;
};

//////////////////////////////////////////////////////////////////////////
// SWindow
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

class SOUI_EXP SWindow : public SObject
    , public SMsgHandleState
    , public TObjRefImpl2<IObjRef,SWindow>
{
    SOUI_CLASS_NAME(SWindow, L"window")
    friend class SLayout;
public:
    SWindow();

    virtual ~SWindow();

    typedef struct tagSWNDMSG
    {
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
    } SWNDMSG,*PSWNDMSG;
protected:
    SEventSet   m_evtSet;

    SWND m_hSWnd;
    ISwndContainer *m_pContainer;
    SWindow *m_pOwner;
    SWindow *m_pParent,*m_pFirstChild, *m_pLastChild, *m_pNextSibling,*m_pPrevSibling;    //窗口树结构
    UINT    m_nChildrenCount;
    SWNDMSG        *m_pCurMsg;

    CRect m_rcWindow;

    SStringW m_strName;
    int     m_nID;

    DuiStyle m_style;
    SStringT m_strText;
    DWORD m_dwState;
    SStringT m_strLinkUrl;
    BOOL m_bMsgTransparent;        //不处理用户操作标志
    BOOL m_bVisible;            //可见状态
    BOOL m_bDisplay;            //隐藏时是否占位，不占位时启动重新布局。
    BOOL m_bDisable;            //禁用状态
    SStringT m_strToolTipText;
    int     m_nSepSpace;    //自动排版的水平空格
    BOOL m_bClipClient;
    BOOL m_bTabStop;
    BYTE m_byAlpha;        //窗口透明度,只进行配置，支持依赖于控件。

    ISkinObj * m_pBgSkin;   //背景skin
    ISkinObj * m_pNcSkin;   //非客户区skin
    
    DUIWND_POSITION m_dlgpos;
    int             m_nMaxWidth;    //自动计算大小时使用

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

    // Get SWindow rect(position in container)
    void GetRect(LPRECT prect);

    virtual void GetClient(LPRECT pRect);

    void GetDlgPosition(DUIWND_POSITION *pPos);
    // Get inner text
    SStringT GetWindowText();
    // Set inner text
    BOOL SetWindowText(LPCTSTR lpszText);

    void TestMainThread();

    // Send a message to DuiWindow
    LRESULT SendMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0,BOOL *pbMsgHandled=NULL);

    PSWNDMSG GetCurMsg()
    {
        return m_pCurMsg;
    }

    // Move DuiWindow to new place
    void Move(LPRECT prect);

    void Move(int x,int y, int cx=-1,int cy=-1);

    // Set current cursor, when hover
    virtual BOOL OnSetCursor(const CPoint &pt);

    // Get tooltip Info
    virtual BOOL OnUpdateToolTip(SWND hCurTipHost,SWND &hNewTipHost,CRect &rcTip,SStringT &strTip);

    // Get DuiWindow state
    DWORD GetState(void);

    // Modify SWindow state
    DWORD ModifyState(DWORD dwStateAdd, DWORD dwStateRemove,BOOL bUpdate=FALSE);

    ULONG_PTR GetUserData();
    ULONG_PTR SetUserData(ULONG_PTR uData);

    //************************************
    // Method:    SetTimer
    // Function:  利用窗口定时器来设置一个ID为0-127的DUI定时器
    // Access:    public
    // Returns:   BOOL
    // Parameter: char id
    // Parameter: UINT uElapse
    // remark:
    //************************************
    BOOL SetTimer(char id,UINT uElapse);

    //************************************
    // Method:    KillTimer
    // Function:  删除一个SWND定时器
    // Access:    public
    // Returns:   void
    // Parameter: char id
    // remark:
    //************************************
    void KillTimer(char id);

    //************************************
    // Method:    SetTimerEx
    // Function:  利用函数定时器来模拟一个兼容窗口定时器
    // Access:    public
    // Returns:   BOOL
    // Parameter: UINT_PTR id
    // Parameter: UINT uElapse
    // remark: 能够使用SetDuiTimer时尽量不用SetDuiTimerEx，在Kill时效率会比较低
    //************************************
    BOOL SetTimerEx(UINT_PTR id,UINT uElapse);

    //************************************
    // Method:    KillTimerEx
    // Function:  删除一个SetDuiTimerEx设置的定时器
    // Access:    public
    // Returns:   void
    // Parameter: UINT_PTR id
    // remark: 需要枚举定时器列表
    //************************************
    void KillTimerEx(UINT_PTR id);

    SWND GetSwnd();


    SWindow *GetParent();

    void SetParent(SWindow *pParent);

    SWindow *GetTopLevelParent();

    BOOL DestroyChild(SWindow *pChild);

    UINT GetChildrenCount();

    SWindow * GetChild(int nID);

    virtual void SetChildContainer(SWindow *pChild);

    void InsertChild(SWindow *pNewChild,SWindow *pInsertAfter=ICWND_LAST);

    BOOL RemoveChild(SWindow *pChild);

    BOOL IsChecked();

    BOOL IsDisabled(BOOL bCheckParent = FALSE);

    BOOL IsVisible(BOOL bCheckParent = FALSE);
    void SetVisible(BOOL bVisible,BOOL bUpdate=FALSE);

    void EnableWindow( BOOL bEnable,BOOL bUpdate=FALSE);

    void SetCheck(BOOL bCheck);

    BOOL NeedRedrawParent();

    LPCTSTR GetLinkUrl();

    ISwndContainer *GetContainer();

    void SetContainer(ISwndContainer *pContainer);

    void SetOwner(SWindow *pOwner);

    SWindow *GetOwner();

    BOOL IsMsgTransparent();

    DuiStyle& GetStyle();

    LPCWSTR GetName(){return m_strName;}
    void SetName(LPCWSTR pszName){m_strName=pszName;}

    int GetID(){return m_nID;}
    void SetID(int nID){m_nID=nID;}
    
    //************************************
    // Method:    FindChildByCmdID, 通过ID查找对应的子窗口
    // Access:    public 
    // Returns:   SWindow*
    // Qualifier:
    // Parameter: UINT uCmdID
    //************************************
    SWindow* FindChildByID(int nID);

    template<class T>
    T* FindChildByID2(int nID)
    {
        T* pRet= dynamic_cast<T *>(FindChildByID(nID));
        ASSERT(pRet);
        return pRet;
    }

    //************************************
    // Method:    FindChildByName，通过名字查找子窗口
    // Access:    public 
    // Returns:   SWindow*
    // Qualifier:
    // Parameter: LPCSTR pszName
    //************************************
    SWindow* FindChildByName(LPCWSTR pszName);

    template<class T>
    T* FindChildByName2(LPCWSTR pszName)
    {
        T* pRet= dynamic_cast<T*>(FindChildByName(pszName));
        ASSERT(pRet);
        return pRet;
    }

    // 从XML创建子窗口
    // LPCWSTR pszXml: utf16编码的XML串
    // return : 顶层的最后一个窗口
    SWindow *CreateChildren(LPCWSTR pszXml);

    void Invalidate();
    void InvalidateRect(LPRECT lprect);
    void InvalidateRect(const CRect& rect);
    void LockUpdate();
    void UnlockUpdate();
    BOOL IsUpdateLocked();
    void BringWindowToTop();

public:
    //同类控件自动成组标志,主要是给RadioButton用的。
    virtual BOOL IsSiblingsAutoGroupped(){return FALSE;}
    
    //获得在一个group中选中状态的窗口，不是group中的窗口时即为当前窗口
    virtual SWindow * GetSelectedSiblingInGroup(){return this;}
    
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

    virtual BOOL CreateChildren(pugi::xml_node xmlNode);
    // Create SWindow from xml element
    virtual BOOL InitFromXml(pugi::xml_node xmlNode);

    virtual SWND SwndFromPoint(CPoint ptHitTest, BOOL bOnlyText);

    virtual BOOL FireEvent(EventArgs &evt);
    
    bool subscribeEvent(const DWORD dwEventID, const SlotFunctorBase & subscriber)
    {
        return m_evtSet.subscribeEvent(dwEventID,subscriber);
    }

    bool unsubscribeEvent( const DWORD dwEventID, const SlotFunctorBase & subscriber )
    {
        return m_evtSet.unsubscribeEvent(dwEventID,subscriber);
    }
    
    bool isEventMuted(void) const
    {
        return m_evtSet.isMuted();
    }

    void    setEventMute(bool bMute)
    {
        return m_evtSet.setMutedState(bMute);
    }
    
    virtual UINT OnGetDlgCode();

    virtual BOOL IsTabStop();

    virtual BOOL OnNcHitTest(CPoint pt);

    virtual BOOL IsClipClient()
    {
        return m_bClipClient;
    }


    //************************************
    // Method:    UpdateChildrenPosition :更新子窗口位置
    // FullName:  SOUI::SWindow::UpdateChildrenPosition
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
    BOOL RedrawRegion(IRenderTarget *pRT, IRegion *pRgn);

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
    IRenderTarget * GetRenderTarget(const LPRECT pRc=NULL,DWORD gdcFlags=0,BOOL bClientDC=TRUE);


    //************************************
    // Method:    ReleaseDuiDC
    // Function:  释放由GetDuiDC获取的DC
    // Access:    public
    // Returns:   void
    // Parameter: HDC hdc
    // remark:
    //************************************
    void ReleaseRenderTarget(IRenderTarget *pRT);

    //************************************
    // Method:    PaintBackground
    // Function:  画窗口的背景内容
    // Access:    public
    // Returns:   void
    // Parameter: HDC hdc 目标DC
    // Parameter: LPRECT pRc 目标位置
    // remark:    目标位置必须在窗口位置内
    //************************************
    void PaintBackground(IRenderTarget *pRT,LPRECT pRc);

    //************************************
    // Method:    PaintForeground
    // Function:  画窗口的前景内容,不包括当前窗口的子窗口
    // Access:    public
    // Returns:   void
    // Parameter: HDC hdc 目标DC
    // Parameter: LPRECT pRc 目标位置
    // remark:    目标位置必须在窗口位置内
    //************************************
    void PaintForeground(IRenderTarget *pRT,LPRECT pRc);


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
    CRect        m_rcGetRT;
    DWORD        m_gdcFlags;
    BOOL         m_bClipRT;
    //备分GetRenderTarget时RT中的字体及颜色
    CAutoRefPtr<IRenderObj> m_oldFont;
    COLORREF     m_oldColor;
public:
    SWND GetCapture();
    SWND SetCapture();
    BOOL ReleaseCapture();
    
    void SetFocus();
    void KillFocus();

    SWindow *GetCheckedRadioButton();

    void CheckRadioButton(SWindow * pRadioBox);

    BOOL SetItemVisible(UINT uItemID, BOOL bVisible);

    BOOL IsItemVisible(UINT uItemID, BOOL bCheckParent = FALSE);
    BOOL GetItemCheck(UINT uItemID);

    BOOL SetItemCheck(UINT uItemID, BOOL bCheck);
    BOOL EnableItem(UINT uItemID, BOOL bEnable);
    BOOL IsItemEnable(UINT uItemID, BOOL bCheckParent = FALSE);

    SWindow *GetWindow(int uCode);    

    //************************************
    // Method:    BeforePaint
    // Function:  为DC准备好当前窗口的绘图环境
    // Access:    public
    // Returns:   void
    // Parameter: CDCHandle & dc
    // Parameter: SPainter & DuiDC
    // remark:
    //************************************
    void BeforePaint(IRenderTarget *pRT, SPainter &painter);

    //************************************
    // Method:    AfterPaint
    // Function:  恢复由BeforePaint设置的DC状态
    // Access:    public
    // Returns:   void
    // Parameter: CDCHandle & dc
    // Parameter: SPainter & DuiDC
    // remark:
    //************************************
    void AfterPaint(IRenderTarget *pRT, SPainter &painter);

    //************************************
    // Method:    BeforePaintEx
    // Function:  为DC准备好当前窗口的绘图环境,从顶层窗口开始设置
    // Access:    public
    // Returns:   int 当前的DC环境
    // Parameter: CDCHandle & dc
    // remark: 使用前使用SaveDC来保存状态，使用后调用RestoreDC来恢复状态
    //************************************
    void BeforePaintEx(IRenderTarget *pRT);

protected:
    LRESULT FireCommand();
    LRESULT FireCtxMenu(CPoint pt);

    //************************************
    // Method:    GetChildrenLayoutRect :返回子窗口的排版空间
    // FullName:  SOUI::SWindow::GetChildrenLayoutRect
    // Access:    virtual protected 
    // Returns:   CRect
    // Qualifier:
    //************************************
    virtual CRect GetChildrenLayoutRect();

    void ClearLayoutState();

    //************************************
    // Method:    GetDesiredSize: 当没有指定窗口大小时，通过如皮肤计算窗口的期望大小
    // FullName:  SOUI::SWindow::GetDesiredSize
    // Access:    virtual protected 
    // Returns:   CSize
    // Qualifier:
    // Parameter: LPRECT pRcContainer
    //************************************
    virtual CSize GetDesiredSize(LPRECT pRcContainer);

    //************************************
    // Method:    CalcSize ：计算窗口大小
    // FullName:  SOUI::SWindow::CalcSize
    // Access:    protected 
    // Returns:   CSize
    // Qualifier:
    // Parameter: LPRECT pRcContainer
    //************************************
    CSize CalcSize(LPRECT pRcContainer);

    //************************************
    // Method:    GetNextVisibleWindow 获得指定窗口的下一个可见窗口
    // FullName:  SOUI::SWindow::GetNextVisibleWindow
    // Access:    protected static 
    // Returns:   SWindow *    :下一个可见窗口
    // Qualifier:
    // Parameter: SWindow * pWnd    :参考窗口
    // Parameter: const CRect &rcDraw:目标矩形
    //************************************
    static SWindow *GetNextVisibleWindow(SWindow *pWnd,const CRect &rcDraw);

    virtual BOOL NeedRedrawWhenStateChange();
    virtual void GetTextRect(LPRECT pRect);
    virtual void DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat);
    virtual void DrawFocus(IRenderTarget *pRT);

    void DrawDefFocusRect(IRenderTarget *pRT,CRect rc);
    void DrawAniStep(CRect rcFore,CRect rcBack,IRenderTarget *pRTFore,IRenderTarget * pRTBack,CPoint ptAnchor);
    void DrawAniStep( CRect rcWnd,IRenderTarget *pRTFore,IRenderTarget * pRTBack,BYTE byAlpha);
    //////////////////////////////////////////////////////////////////////////
    // Message Handler

    //************************************
    // Method:    SwndProc
    // Function:  默认的消息处理函数
    // Access:    virtual public
    // Returns:   BOOL
    // Parameter: UINT uMsg
    // Parameter: WPARAM wParam
    // Parameter: LPARAM lParam
    // Parameter: LRESULT & lResult
    // remark: 在消息映射表中没有处理的消息进入该函数处理
    //************************************
    virtual BOOL SwndProc(UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT & lResult)
    {
        return FALSE;
    }

    LRESULT OnWindowPosChanged(LPRECT lpRcContainer);

    int OnCreate(LPVOID);

    void OnDestroy();

    // Draw background default
    BOOL OnEraseBkgnd(IRenderTarget *pRT);

    // Draw inner text default
    void OnPaint(IRenderTarget *pRT);


    //************************************
    // Method:    OnNcPaint
    // Function:  draw non-client area
    // Access:    protected
    // Returns:   void
    // Parameter: CDCHandle dc
    // remark:
    //************************************
    void OnNcPaint(IRenderTarget *pRT);

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

    void OnSetFocus();
    void OnKillFocus();

    HRESULT OnAttrPos(const SStringW& strValue, BOOL bLoading);
    HRESULT OnAttrState(const SStringW& strValue, BOOL bLoading);

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
        MSG_WM_NCPAINT_EX(OnNcPaint)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_WINPOSCHANGED_EX(OnWindowPosChanged)
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_ENABLE_EX(OnEnable)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSEHOVER(OnMouseHover)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
    WND_MSG_MAP_END_BASE()
    
    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"id",m_nID,FALSE)
        ATTR_STRINGW(L"name",m_strName,FALSE)
        ATTR_SKIN(L"skin", m_pBgSkin, TRUE)//直接获得皮肤对象
        ATTR_SKIN(L"ncskin", m_pNcSkin, TRUE)//直接获得皮肤对象
        ATTR_STYLE(L"class", m_style, TRUE)    //获得style
        ATTR_CHAIN(m_style)                    //支持对style中的属性定制
        ATTR_INT(L"data", m_uData, 0 )
        ATTR_CUSTOM(L"state", OnAttrState)
        ATTR_STRINGT(L"href", m_strLinkUrl, FALSE)
        ATTR_STRINGT(L"tip", m_strToolTipText, FALSE)
        ATTR_CUSTOM(L"pos", OnAttrPos)
        ATTR_INT(L"show", m_bVisible,FALSE)
        ATTR_INT(L"display", m_bDisplay,FALSE)
        ATTR_INT(L"msgtransparent", m_bMsgTransparent, FALSE)
        ATTR_INT(L"sep", m_nSepSpace, FALSE)
        ATTR_INT(L"maxwidth",m_nMaxWidth,FALSE)
        ATTR_INT(L"clipclient",m_bClipClient,FALSE)
        ATTR_INT(L"tabstop",m_bTabStop,FALSE)
        ATTR_ENUM_BEGIN(L"pos2type",POS2TYPE,FALSE)
            ATTR_ENUM_VALUE(L"lefttop",POS2_LEFTTOP)
            ATTR_ENUM_VALUE(L"center",POS2_CENTER)
            ATTR_ENUM_VALUE(L"righttop",POS2_RIGHTTOP)
            ATTR_ENUM_VALUE(L"leftbottom",POS2_LEFTBOTTOM)
            ATTR_ENUM_VALUE(L"rightbottom",POS2_RIGHTBOTTOM)
        ATTR_ENUM_END(m_dlgpos.pos2Type)
        ATTR_INT(L"alpha",m_byAlpha,TRUE)
    SOUI_ATTRS_END()

};
}//namespace SOUI