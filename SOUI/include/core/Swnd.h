/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       Swnd.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    SOUI基础DUI窗口模块
*/

#pragma once
#include "SWindowMgr.h"
#include "SwndContainer-i.h"

#include "helper/STimerEx.h"
#include "helper/SwndMsgCracker.h"

#include "event/EventSubscriber.h"
#include "event/events.h"
#include "event/EventSet.h"
#include <OCIdl.h>
#include "SwndLayout.h"
#include "res.mgr/SStylePool.h"
#include "res.mgr/SSkinPool.h"

namespace SOUI
{

    /////////////////////////////////////////////////////////////////////////
    enum {NormalShow=0,ParentShow=1};    //提供WM_SHOWWINDOW消息识别是父窗口显示还是要显示本窗口
    enum {NormalEnable=0,ParentEnable=1};    //提供WM_ENABLE消息识别是父窗口可用还是直接操作当前窗口

#define SC_WANTARROWS     0x0001      /* Control wants arrow keys         */
#define SC_WANTTAB        0x0002      /* Control wants tab keys           */
#define SC_WANTRETURN     0x0004      /* Control wants return keys        */
#define SC_WANTCHARS      0x0008      /* Want WM_CHAR messages            */
#define SC_WANTALLKEYS    0xFFFF      /* Control wants all keys           */
#define SC_WANTSYSKEY     0x80000000    /* System Key */

    class SOUI_EXP STimerID
    {
    public:
        DWORD    Swnd:24;        //窗口句柄,如果窗口句柄超过24位范围，则不能使用这种方式设置定时器
        DWORD    uTimerID:7;        //定时器ID，一个窗口最多支持128个定时器。
        DWORD    bSwndTimer:1;    //区别通用定时器的标志，标志为1时，表示该定时器为SWND定时器

        STimerID(SWND hWnd,char id)
        {
            SASSERT(hWnd<0x00FFFFFF && id>=0);
            bSwndTimer=1;
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

    typedef enum tagGW_CODE
    {
        GSW_FIRSTCHILD=0,
        GSW_LASTCHILD,
        GSW_PREVSIBLING,
        GSW_NEXTSIBLING,
        GSW_PARENT,
        GSW_OWNER,
    } GW_CODE;

    typedef struct tagSWNDMSG
    {
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
    } SWNDMSG,*PSWNDMSG;

    /**
    * @class     SWindow
    * @brief     SOUI窗口基类 
    * 
    * Describe   SOUI窗口基类,实现窗口的基本接口
    */
    class SOUI_EXP SWindow : public SObject
        , public SMsgHandleState
        , public TObjRefImpl2<IObjRef,SWindow>
    {
        SOUI_CLASS_NAME(SWindow, L"window")
            friend class SwndLayout;
    public:
        SWindow();

        virtual ~SWindow();

    protected:
        SWND m_hSWnd;       /**< 窗口句柄 */

        ISwndContainer *m_pContainer;/**< 容器对象 */
        SEventSet   m_evtSet;/**< 窗口事件集合 */

        SWindow *m_pOwner;  /**< 容器Owner，事件分发时，会把事件交给Owner处理 */
        SWindow *m_pParent; /**< 父窗口 */
        SWindow *m_pFirstChild;/**< 第一子窗口 */
        SWindow *m_pLastChild;/**< 最后窗口 */
        SWindow *m_pNextSibling;/**< 前一兄弟窗口 */
        SWindow *m_pPrevSibling; /**< 后一兄弟窗口 */
        UINT    m_nChildrenCount;  /**< 子窗口数量 */

        SWNDMSG        *m_pCurMsg;  /**< 当前正在处理的窗口消息 */

        CRect m_rcWindow;       /**< 窗口在容器中的位置 */

        SStringW m_strName;     /**< 窗口名称 */
        int     m_nID;          /**< 窗口ID */

        SwndStyle m_style;      /**< 窗口Style，是一组窗口属性 */
        SStringT m_strText;     /**< 窗口文字 */
        SStringT m_strToolTipText;/**< 窗口ToolTip */

        DWORD m_dwState;        /**< 窗口状态 */
        DWORD m_bVisible:1;        /**< 窗口可见状态 */
        DWORD m_bDisplay:1;        /**< 窗口隐藏时是否占位，不占位时启动重新布局 */
        DWORD m_bDisable:1;        /**< 窗口禁用状状态 */
        DWORD m_bClipClient:1;     /**< 窗口绘制时做clip客户区处理的标志,由于clip可能增加计算量，只在绘制可能走出客户区时才设置*/
        DWORD m_bMsgTransparent:1; /**< 接收消息标志 TRUE-不处理消息 */
        DWORD m_bFocusable:1;      /**< 窗口可获得焦点标志 */
        DWORD m_bUpdateLocked:1;   /**< 暂时锁定更新，锁定后，不向宿主发送Invalidate */
        DWORD m_bCacheDraw:1;      /**< 支持窗口内容的Cache标志 */
        DWORD m_bDirty:1;          /**< 缓存窗口脏标志 */

        BYTE m_byAlpha;         /**< 窗口透明度,只进行配置，支持依赖于控件 */

        ISkinObj * m_pBgSkin;   /**< 背景skin */
        ISkinObj * m_pNcSkin;   /**< 非客户区skin */
        ULONG_PTR m_uData;      /**< 窗口的数据位,可以通过GetUserData获得 */

        SwndLayout    m_layout; /**< 布局对象 */

        int           m_nMaxWidth;    /**< 自动计算大小时，窗口的最大宽度 */

        CAutoRefPtr<IRenderTarget> m_cachedRT;/**< 缓存窗口绘制的RT */
#ifdef _DEBUG
        DWORD m_nMainThreadId;  /**< 窗口宿线程ID */
#endif
    public:

        //////////////////////////////////////////////////////////////////////////
        // Method Define

        /**
        * GetTextAlign
        * @brief    获得文本的对齐标志
        * @return   UINT 
        *
        * Describe  获得文本的对齐标志
        */
        UINT GetTextAlign();    

        /**
        * GetPositionType
        * @brief    获得窗口布局类型
        * @return   DWORD 
        *
        * Describe  获得窗口布局类型
        */
        DWORD GetPositionType();

        /**
        * SetPositionType
        * @brief    设置布局类型
        * @param    DWORD dwPosType --  布局类型
        * @param    DWORD dwMask --  布局类型mask
        * @return   void 
        *
        * Describe  
        */    
        void SetPositionType(DWORD dwPosType, DWORD dwMask = 0xFFFFFFFF);


        /**
         * SetFixSize
         * @brief    设置窗口大小
         * @param    int nWid --  窗口宽度
         * @param    int nHei --  窗口高度
         * @return   void 
         *
         * Describe  
         */
        void SetFixSize(int nWid,int nHei);

        /**
        * GetWindowRect
        * @brief    获得窗口在宿主中的位置
        * @param    [out] LPRECT prect --  窗口矩形
        * @return   void 
        *
        * Describe  
        */    
        void GetWindowRect(LPRECT prect);

        /**
        * GetClientRect
        * @brief    获得窗口的客户区
        * @param    [out] LPRECT pRect --  窗口矩形
        * @return   void 
        *
        * Describe  
        */
        virtual void GetClientRect(LPRECT pRect);

        /**
        * GetWindowText
        * @brief    获得窗口文本
        * @return   SStringT 
        *
        * Describe  
        */
        SStringT GetWindowText();

        /**
        * SetWindowText
        * @brief    设置窗口文本
        * @param    LPCTSTR lpszText --  窗口文本
        * @return   void 
        *
        * Describe  
        */
        void SetWindowText(LPCTSTR lpszText);


        /**
        * SSendMessage
        * @brief    向SWND发送条窗口消息
        * @param    UINT Msg --  消息类型
        * @param    WPARAM wParam --  参数1
        * @param    LPARAM lParam --  参数2
        * @param [out] BOOL * pbMsgHandled -- 消息处理标志 
        * @return   LRESULT 消息处理状态，依赖于消息类型
        *
        * Describe  
        */
        LRESULT SSendMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0,BOOL *pbMsgHandled=NULL);

        /**
        * GetCurMsg
        * @brief    获得当前正在处理的消息
        * @return   PSWNDMSG 
        *
        * Describe  
        */
        PSWNDMSG GetCurMsg()
        {
            return m_pCurMsg;
        }

        /**
        * Move
        * @brief    将窗口移动到指定位置
        * @param    LPRECT prect --  
        * @return   void 
        *
        * Describe  移动后，窗口的布局标志自动变为Pos_Float
        */
        void Move(LPRECT prect);

        /**
        * Move
        * @brief    将窗口移动到指定位置
        * @param    int x --  left
        * @param    int y --  top
        * @param    int cx --  width
        * @param    int cy --  height
        * @return   void 
        *
        * Describe 
        * @see     Move(LPRECT prect)
        */
        void Move(int x,int y, int cx=-1,int cy=-1);


        // Get SWindow state
        DWORD GetState(void);

        // Modify SWindow state
        DWORD ModifyState(DWORD dwStateAdd, DWORD dwStateRemove,BOOL bUpdate=FALSE);

        /**
        * GetUserData
        * @brief    读userdata
        * @return   ULONG_PTR 
        *
        * Describe  
        */
        ULONG_PTR GetUserData();
        /**
        * SetUserData
        * @brief    设置userdata
        * @param    ULONG_PTR uData --  原来的userdata
        * @return   ULONG_PTR 
        *
        * Describe  
        */
        ULONG_PTR SetUserData(ULONG_PTR uData);

        /**
        * SetTimer
        * @brief    利用窗口定时器来设置一个ID为0-127的SWND定时器
        * @param    char id --  定时器ID
        * @param    UINT uElapse --  延时(MS)
        * @return   BOOL 
        *
        * Describe  参考::SetTimer
        */
        BOOL SetTimer(char id,UINT uElapse);

        /**
        * KillTimer
        * @brief    删除一个SWND定时器
        * @param    char id --  定时器ID
        * @return   void 
        *
        * Describe  
        */
        void KillTimer(char id);

        /**
        * SetTimer2
        * @brief    利用函数定时器来模拟一个兼容窗口定时器
        * @param    UINT_PTR id --  定时器ID
        * @param    UINT uElapse --  延时(MS)
        * @return   BOOL 
        *
        * Describe  由于SetTimer只支持0-127的定时器ID，SetTimer2提供设置其它timerid
        *           能够使用SetTimer时尽量不用SetTimer2，在Kill时效率会比较低
        */
        BOOL SetTimer2(UINT_PTR id,UINT uElapse);

        /**
        * KillTimer2
        * @brief    删除一个SetTimer2设置的定时器
        * @param    UINT_PTR id --  
        * @return   void 
        *
        * Describe  需要枚举定时器列表
        */
        void KillTimer2(UINT_PTR id);

        /**
        * GetSwnd
        * @brief    获得窗口句柄
        * @return   SWND 
        *
        * Describe  
        */
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

        ISwndContainer *GetContainer();

        void SetContainer(ISwndContainer *pContainer);

        void SetOwner(SWindow *pOwner);

        SWindow *GetOwner();

        BOOL IsMsgTransparent();

        SwndStyle& GetStyle();

        LPCWSTR GetName(){return m_strName;}
        void SetName(LPCWSTR pszName){m_strName=pszName;}

        int GetID(){return m_nID;}
        void SetID(int nID){m_nID=nID;}

        /**
        * FindChildByID
        * @brief    通过ID查找对应的子窗口
        * @param    int nID --  窗口ID
        * @return   SWindow* 
        *
        * Describe  
        */
        SWindow* FindChildByID(int nID);

        /**
        * FindChildByID2
        * @brief    FindChildByID的模板类，支持类型转换
        * @param    int nID --  窗口ID
        * @return   T* 
        *
        * Describe  
        */
        template<class T>
        T* FindChildByID2(int nID)
        {
            SWindow *pTarget = FindChildByID(nID);
            if(!pTarget || !pTarget->IsClass(T::GetClassName()))
            {
                SASSERT(pTarget);
                return NULL;
            }
            return (T*)pTarget;
        }

        /**
        * FindChildByName
        * @brief    通过名字查找子窗口
        * @param    LPCWSTR pszName --  窗口name属性
        * @return   SWindow* 
        *
        * Describe  
        */
        SWindow* FindChildByName(LPCWSTR pszName);

        template<class T>
        T* FindChildByName2(LPCWSTR pszName)
        {
            SWindow *pTarget = FindChildByName(pszName);
            if(!pTarget || !pTarget->IsClass(T::GetClassName()))
            {
                SASSERT(pTarget);
                return NULL;
            }
            return (T*)pTarget;
        }

        /**
        * CreateChildren
        * @brief    从XML创建子窗口
        * @param    LPCWSTR pszXml --  合法的utf16编码XML字符串
        * @return   SWindow * 创建成功的的最后一个窗口
        *
        * Describe  
        */
        SWindow *CreateChildren(LPCWSTR pszXml);

        void Invalidate();
        void InvalidateRect(LPRECT lprect);
        void InvalidateRect(const CRect& rect);
        void LockUpdate();
        void UnlockUpdate();
        BOOL IsUpdateLocked();
        void BringWindowToTop();

    public:
        //////////////////////////////////////////////////////////////////////////
        // Virtual functions

        /**
        * IsSiblingsAutoGroupped
        * @brief    同类窗口自动成组标志
        * @return   BOOL 
        *
        * Describe  主要是给RadioButton用的
        */
        virtual BOOL IsSiblingsAutoGroupped(){return FALSE;}

        /**
        * GetSelectedSiblingInGroup
        * @brief    获得在一个group中选中状态的窗口
        * @return   SWindow * 
        *
        * Describe  不是group中的窗口时返回NULL
        */
        virtual SWindow * GetSelectedSiblingInGroup(){return NULL;}

        virtual void OnSetCaretValidateRect(LPCRECT lpRect)
        {
            CRect rcClient;
            GetClientRect(&rcClient);
            CRect rcIntersect;
            rcIntersect.IntersectRect(&rcClient,lpRect);
            if(GetParent()) GetParent()->OnSetCaretValidateRect(&rcIntersect);
        }
        // Set current cursor, when hover
        virtual BOOL OnSetCursor(const CPoint &pt);

        // Get tooltip Info
        virtual BOOL OnUpdateToolTip(SWND hCurTipHost,SWND &hNewTipHost,CRect &rcTip,SStringT &strTip);

        virtual void OnStateChanged(DWORD dwOldState,DWORD dwNewState) {}

        virtual BOOL CreateChildren(pugi::xml_node xmlNode);
        // Create SWindow from xml element
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);

        virtual SStringW tr(const SStringW &strSrc);

        virtual SWND SwndFromPoint(CPoint ptHitTest, BOOL bOnlyText);

        virtual BOOL FireEvent(EventArgs &evt);

        SEventSet * GetEventSet(){return &m_evtSet;}

        virtual UINT OnGetDlgCode();

        virtual BOOL IsFocusable();

        virtual BOOL OnNcHitTest(CPoint pt);

        virtual BOOL IsClipClient()
        {
            return m_bClipClient;
        }


        /**
        * UpdateChildrenPosition
        * @brief    更新子窗口位置
        * @return   void 
        *
        * Describe  
        */
        virtual void UpdateChildrenPosition();

    public:
        /**
        * RedrawRegion
        * @brief    将窗口及子窗口内容绘制到RenderTarget
        * @param    IRenderTarget * pRT --  渲染目标RT
        * @param    IRegion * pRgn --  渲染区域，为NULL时渲染整个窗口
        * @return   void 
        *
        * Describe  
        */
        void RedrawRegion(IRenderTarget *pRT, IRegion *pRgn);

        /**
        * GetRenderTarget
        * @brief    获取一个与SWND窗口相适应的内存DC
        * @param    const LPRECT pRc --  RT范围
        * @param    DWORD gdcFlags --  同OLEDCFLAGS
        * @param    BOOL bClientDC --  限制在client区域
        * @return   IRenderTarget * 
        *
        * Describe  使用ReleaseRenderTarget释放
        */
        IRenderTarget * GetRenderTarget(const LPRECT pRc=NULL,DWORD gdcFlags=0,BOOL bClientDC=TRUE);


        /**
        * ReleaseRenderTarget
        * @brief    
        * @param    IRenderTarget * pRT --  释放由GetRenderTarget获取的RT
        * @return   void 
        *
        * Describe  
        */
        void ReleaseRenderTarget(IRenderTarget *pRT);

        /**
        * PaintBackground
        * @brief    画窗口的背景内容
        * @param    IRenderTarget * pRT --  目标RT
        * @param    LPRECT pRc --  目标位置
        * @return   void 
        *
        * Describe  目标位置必须在窗口位置内
        */
        void PaintBackground(IRenderTarget *pRT,LPRECT pRc);

        /**
        * PaintForeground
        * @brief    画窗口的前景内容
        * @param    IRenderTarget * pRT --  目标RT
        * @param    LPRECT pRc --  目标位置
        * @return   void 
        *
        * Describe  目标位置必须在窗口位置内,不包括当前窗口的子窗口
        */
        void PaintForeground(IRenderTarget *pRT,LPRECT pRc);


        /**
        * AnimateWindow
        * @brief    窗口动画效果
        * @param    DWORD dwTime --  执行时间
        * @param    DWORD dwFlags --  执行模式
        * @return   BOOL 
        *
        * Describe  
        */
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

        SWindow *GetWindow(int uCode);    

        /**
        * BeforePaint
        * @brief    为RT准备好当前窗口的绘图环境
        * @param    IRenderTarget * pRT --  
        * @param    SPainter & painter --  
        * @return   void 
        *
        * Describe  
        */
        void BeforePaint(IRenderTarget *pRT, SPainter &painter);

        /**
        * AfterPaint
        * @brief    恢复由BeforePaint设置的RT状态
        * @param    IRenderTarget * pRT --  
        * @param    SPainter & painter --  
        * @return   void 
        *
        * Describe  
        */
        void AfterPaint(IRenderTarget *pRT, SPainter &painter);

        /**
        * BeforePaintEx
        * @brief    为DC准备好当前窗口的绘图环境,从顶层窗口开始设置
        * @param    IRenderTarget * pRT --  渲染RT
        * @return   void 
        *
        * Describe  一般应该和CreateRanderTarget配合使用
        */
        void BeforePaintEx(IRenderTarget *pRT);

        /**
        * FireCommand
        * @brief    激活窗口的EVT_CMD事件
        * @return   BOOL-- true:EVT_CMD事件被处理
        *
        * Describe  
        */
        BOOL FireCommand();

        /**
        * FireCtxMenu
        * @brief    激活快捷菜单事件
        * @param    CPoint pt --  鼠标点击位置
        * @return   BOOL -- true:外部处理了快捷菜单事件
        *
        * Describe  
        */
        BOOL FireCtxMenu(CPoint pt);
    protected:
        /**
         * IsCacheDirty
         * @brief    查询Cache的Dirty标志
         * @return   bool -- true表示Cache已经Dirty
         * Describe  
         */    
        bool IsCacheDirty() const  {return m_bCacheDraw&&m_bDirty;}

        /**
         * MarkCacheDirty
         * @brief    标记Cache的Dirty标志
         * @param    bool bDirty --  Dirty标志
         * @return   void
         * Describe  
         */    
        void MarkCacheDirty(bool bDirty) {m_bDirty = bDirty;}

        /**
         * IsDrawToCache
         * @brief    查看当前是否是把窗口内容绘制到cache上
         * @return   bool -- true绘制到cache上。
         * Describe  
         */    
        bool IsDrawToCache() const {return m_bCacheDraw;}

        /**
         * GetCachedRenderTarget
         * @brief    获得Cache窗口内容的RenderTarget
         * @return   IRenderTarget * -- Cache窗口内容的RenderTarget
         * Describe  
         */    
        IRenderTarget * GetCachedRenderTarget(){return m_cachedRT;}

        void TestMainThread();

        /**
        * GetChildrenLayoutRect
        * @brief    获得子窗口的布局空间
        * @return   CRect 
        *
        * Describe  
        */
        virtual CRect GetChildrenLayoutRect();

        /**
        * ClearLayoutState
        * @brief    清除子窗口的布局状态标志
        * @return   void 
        *
        * Describe  
        */
        void ClearLayoutState();

        /**
        * GetDesiredSize
        * @brief    当没有指定窗口大小时，通过如皮肤计算窗口的期望大小
        * @param    LPRECT pRcContainer --  容器位置
        * @return   CSize 
        *
        * Describe  
        */
        virtual CSize GetDesiredSize(LPRECT pRcContainer);

        /**
        * GetNextVisibleWindow
        * @brief    获得指定窗口的下一个可见窗口
        * @param    SWindow * pWnd --  参考窗口
        * @param    const CRect & rcDraw --  目标矩形
        * @return   SWindow * 下一个可见窗口
        *
        * Describe  
        */
        static SWindow *GetNextVisibleWindow(SWindow *pWnd,const CRect &rcDraw);

        typedef enum _PRSTATE{
            PRS_LOOKSTART=0,    //查找开始窗口
            PRS_DRAWING,        //窗口渲染中
            PRS_MEETEND         //碰到指定的结束窗口
        } PRSTATE;

        static  BOOL _PaintRegion( IRenderTarget *pRT, IRegion *pRgn,SWindow *pWndCur,SWindow *pStart,SWindow *pEnd,PRSTATE & prState );

        virtual BOOL NeedRedrawWhenStateChange();
        virtual void GetTextRect(LPRECT pRect);
        virtual void DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat);
        virtual void DrawFocus(IRenderTarget *pRT);

        void DrawDefFocusRect(IRenderTarget *pRT,CRect rc);
        void DrawAniStep(CRect rcFore,CRect rcBack,IRenderTarget *pRTFore,IRenderTarget * pRTBack,CPoint ptAnchor);
        void DrawAniStep( CRect rcWnd,IRenderTarget *pRTFore,IRenderTarget * pRTBack,BYTE byAlpha);

    protected:
        //////////////////////////////////////////////////////////////////////////
        // Message Handler

        /**
        * SwndProc
        * @brief    默认的消息处理函数
        * @param    UINT uMsg --  消息类型
        * @param    WPARAM wParam --  参数1
        * @param    LPARAM lParam --  参数2
        * @param    LRESULT & lResult --  消息返回值
        * @return   BOOL 是否被处理
        *
        * Describe  在消息映射表中没有处理的消息进入该函数处理
        */
        virtual BOOL SwndProc(UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT & lResult)
        {
            return FALSE;
        }

        LRESULT OnWindowPosChanged(LPRECT lpRcContainer);

        int OnCreate(LPVOID);

        void OnSize(UINT nType, CSize size);

        void OnDestroy();

        BOOL OnEraseBkgnd(IRenderTarget *pRT);

        void OnPaint(IRenderTarget *pRT);

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

        //////////////////////////////////////////////////////////////////////////
        // 属性处理函数
        HRESULT OnAttrPos(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrVisible(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrEnable(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrDisplay(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrCache(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrSkin(const SStringW& strValue, BOOL bLoading);
        HRESULT OnAttrClass(const SStringW& strValue, BOOL bLoading);

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)
            MSG_WM_NCPAINT_EX(OnNcPaint)
            MSG_WM_CREATE(OnCreate)
            MSG_WM_SIZE(OnSize)
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
        WND_MSG_MAP_END_BASE()  //消息不再往基类传递，此外使用WND_MSG_MAP_END_BASE而不是WND_MSG_MAP_END

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"id",m_nID,FALSE)
            ATTR_STRINGW(L"name",m_strName,FALSE)
            ATTR_CUSTOM(L"skin", OnAttrSkin)        //直接获得皮肤对象
            ATTR_SKIN(L"ncskin", m_pNcSkin, TRUE)   //直接获得皮肤对象
            ATTR_CUSTOM(L"class", OnAttrClass)      //获得style
            ATTR_CHAIN(m_style)                     //支持对style中的属性定制
            ATTR_INT(L"data", m_uData, 0 )
            ATTR_CUSTOM(L"enable", OnAttrEnable)
            ATTR_CUSTOM(L"visible", OnAttrVisible)
            ATTR_CUSTOM(L"show", OnAttrVisible)
            ATTR_CUSTOM(L"pos", OnAttrPos)
            ATTR_CUSTOM(L"cache", OnAttrCache)
            ATTR_CUSTOM(L"display", OnAttrDisplay)
            ATTR_I18NSTRT(L"tip", m_strToolTipText, FALSE)  //使用语言包翻译
            ATTR_INT(L"msgTransparent", m_bMsgTransparent, FALSE)
            ATTR_INT(L"maxWidth",m_nMaxWidth,FALSE)
            ATTR_INT(L"clipClient",m_bClipClient,FALSE)
            ATTR_INT(L"focusable",m_bFocusable,FALSE)
            ATTR_ENUM_BEGIN(L"pos2type",POS2TYPE,FALSE)
                ATTR_ENUM_VALUE(L"leftTop",POS2_LEFTTOP)
                ATTR_ENUM_VALUE(L"center",POS2_CENTER)
                ATTR_ENUM_VALUE(L"righTtop",POS2_RIGHTTOP)
                ATTR_ENUM_VALUE(L"leftBottom",POS2_LEFTBOTTOM)
                ATTR_ENUM_VALUE(L"rightBottom",POS2_RIGHTBOTTOM)
            ATTR_ENUM_END(m_layout.pos2Type)
            ATTR_INT(L"sep", m_layout.nSepSpace, FALSE)
            ATTR_INT(L"alpha",m_byAlpha,TRUE)
        SOUI_ATTRS_END()

    };
}//namespace SOUI