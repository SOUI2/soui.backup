#include "souistd.h"
#include "core/SWnd.h"
#include "helper/color.h"
#include "helper/SplitString.h"

namespace SOUI
{


//////////////////////////////////////////////////////////////////////////
// SWindow Implement
//////////////////////////////////////////////////////////////////////////

SWindow::SWindow()
    : m_swnd(SWindowMgr::NewWindow(this))
    , m_layout(this)
    , m_nID(0)
    , m_pContainer(NULL)
    , m_pParent(NULL),m_pFirstChild(NULL),m_pLastChild(NULL),m_pNextSibling(NULL),m_pPrevSibling(NULL)
    , m_nChildrenCount(0)
    , m_dwState(WndState_Normal)
    , m_bMsgTransparent(FALSE)
    , m_bVisible(TRUE)
    , m_bDisplay(TRUE)
    , m_bDisable(FALSE)
    , m_nMaxWidth(-1)
    , m_bUpdateLocked(FALSE)
    , m_bClipClient(FALSE)
    , m_bFocusable(FALSE)
    , m_bCacheDraw(FALSE)
    , m_bDirty(TRUE)
    , m_uData(0)
    , m_pOwner(NULL)
    , m_pCurMsg(NULL)
    , m_pBgSkin(NULL)
    , m_pNcSkin(NULL)
    , m_bClipRT(FALSE)
    , m_gdcFlags(-1)
    , m_byAlpha(0xFF)
#ifdef _DEBUG
    , m_nMainThreadId( ::GetCurrentThreadId() ) // 初始化对象的线程不一定是主线程
#endif
{
    ClearLayoutState();
    m_evtSet.addEvent(EventCmd::EventID);
    m_evtSet.addEvent(EventCtxMenu::EventID);
    m_evtSet.addEvent(EventSetFocus::EventID);
    m_evtSet.addEvent(EventKillFocus::EventID);
}

SWindow::~SWindow()
{
    SWindowMgr::DestroyWindow(m_swnd);
}


// Get align
UINT SWindow::GetTextAlign()
{
    return m_style.GetTextAlign() ;
}

DWORD SWindow::GetPositionType()
{
    return m_layout.uPositionType;
}

void SWindow::SetPositionType(DWORD dwPosType, DWORD dwMask /*= 0xFFFFFFFF*/)
{
    m_layout.uPositionType = (m_layout.uPositionType & ~dwMask) | (dwPosType & dwMask);
}

void SWindow::SetFixSize( int nWid,int nHei )
{
    m_layout.uPositionType = (m_layout.uPositionType & ~SizeX_Mask) | SizeX_Specify|SizeY_Specify;
    m_layout.uSpecifyWidth = nWid;
    m_layout.uSpecifyHeight = nHei;
}

void SWindow::GetWindowRect(LPRECT prect)
{
    SASSERT(prect);
    prect->left     = m_rcWindow.left;
    prect->top      = m_rcWindow.top;
    if(m_bDisplay || m_bVisible)
    {
        prect->right    = m_rcWindow.right;
        prect->bottom   = m_rcWindow.bottom;
    }else
    {
        prect->right=prect->left;
        prect->bottom=prect->top;
    }
}

void SWindow::GetClientRect(LPRECT pRect)
{
    SASSERT(pRect);
    *pRect=m_rcWindow;
    pRect->left+=m_style.m_nMarginX;
    pRect->right-=m_style.m_nMarginX;
    pRect->top+=m_style.m_nMarginY;
    pRect->bottom-=m_style.m_nMarginY;
}

// Get inner text
SStringT SWindow::GetWindowText()
{
    return m_strText;
}


void SWindow::OnSetCaretValidateRect( LPCRECT lpRect )
{
    CRect rcClient;
    GetClientRect(&rcClient);
    CRect rcIntersect;
    rcIntersect.IntersectRect(&rcClient,lpRect);
    if(GetParent()) GetParent()->OnSetCaretValidateRect(&rcIntersect);
}

BOOL SWindow::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    if(m_strToolTipText.IsEmpty()) return FALSE;
    tipInfo.swnd = m_swnd;
    tipInfo.dwCookie =0;
    tipInfo.rcTarget = m_rcWindow;
    tipInfo.strTip = m_strToolTipText;
    return TRUE;
}

void SWindow::SetWindowText(LPCTSTR lpszText)
{
    m_strText = lpszText;
    if(IsVisible(TRUE)) Invalidate();
    if ((m_layout.uPositionType & (SizeX_FitContent | SizeY_FitContent)) && (4 != m_layout.nCount))
    {
        OnWindowPosChanged(NULL);
        if(IsVisible(TRUE)) Invalidate();
    }
}

void SWindow::TestMainThread()
{
#ifdef DEBUG
    // 当你看到这个东西的时候，我不幸的告诉你，你的其他线程在刷界面
    // 这是一件很危险的事情
    DWORD dwCurThreadID = GetCurrentThreadId();

    BOOL bOK = (m_nMainThreadId == dwCurThreadID); // 当前线程和构造对象时的线程一致

    SASSERT(bOK);
#endif
}


// Send a message to SWindow
LRESULT SWindow::SSendMessage(UINT Msg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/,BOOL *pbMsgHandled/*=NULL*/)
{
    LRESULT lResult = 0;

    if ( Msg < WM_USER
            && Msg != WM_DESTROY
            && Msg != WM_CLOSE
       )
    {
        TestMainThread();
    }
    AddRef();
    SWNDMSG msgCur= {Msg,wParam,lParam};
    SWNDMSG *pOldMsg=m_pCurMsg;
    m_pCurMsg=&msgCur;

    BOOL bOldMsgHandle=IsMsgHandled();//备分上一个消息的处理状态

    SetMsgHandled(FALSE);

    ProcessSwndMessage(Msg, wParam, lParam, lResult);
    
    if(pbMsgHandled) *pbMsgHandled=IsMsgHandled();

    SetMsgHandled(bOldMsgHandle);//恢复上一个消息的处理状态

    m_pCurMsg=pOldMsg;
    Release();
    
    if(m_style.m_bMouseRelay && GetParent())
    {//将鼠标消息交给父窗口处理
        switch(Msg)
        {
        case WM_MOUSEMOVE:
        case WM_MOUSEHOVER:
            lResult = GetParent()->SSendMessage(Msg,wParam,lParam,pbMsgHandled);
            break;
        case WM_MOUSELEAVE:
            {
                lResult = GetParent()->SSendMessage(Msg,0,0,pbMsgHandled);
                HWND hHost=GetContainer()->GetHostHwnd();
                CPoint pt;
                ::GetCursorPos(&pt);
                ::ScreenToClient(hHost,&pt);
                ::PostMessage(hHost,WM_MOUSEMOVE,0,MAKELPARAM(pt.x,pt.y));
            }
            break;    
        }    
    }
    return lResult;
}

LRESULT SWindow::SDispatchMessage( MSG * pMsg,BOOL *pbMsgHandled/*=NULL*/ )
{
    LRESULT lRet = SSendMessage(pMsg->message,pMsg->wParam,pMsg->lParam,pbMsgHandled);
    if(pbMsgHandled && *pbMsgHandled) return lRet;
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    while(pChild)
    {
        pChild->SDispatchMessage(pMsg,pbMsgHandled);
        pChild = pChild->GetWindow(GSW_NEXTSIBLING);
    }
    return lRet;
}

// Move SWindow to new place
//
void SWindow::Move(LPRECT prect)
{
    SASSERT(prect);
    TestMainThread();

    if(m_rcWindow.EqualRect(prect)) return;

    m_rcWindow = prect;
    m_layout.uPositionType = (m_layout.uPositionType & ~Position_Mask)|Pos_Float;//使用Move后，程序不再自动计算窗口坐标
    OnWindowPosChanged(NULL);
}

void SWindow::Move(int x,int y, int cx/*=-1*/,int cy/*=-1*/)
{
    if(cx==-1) cx=m_rcWindow.Width();
    if(cy==-1) cy=m_rcWindow.Height();
    CRect rcNew(x,y,x+cx,y+cy);
    Move(&rcNew);
}

// Set current cursor, when hover
BOOL SWindow::OnSetCursor(const CPoint &pt)
{
    HCURSOR hCursor=GETRESPROVIDER->LoadCursor(m_style.m_strCursor);
    ::SetCursor(hCursor);
    return TRUE;
}

// Get SWindow state
DWORD SWindow::GetState(void)
{
    return m_dwState;
}

// Modify SWindow state
DWORD SWindow::ModifyState(DWORD dwStateAdd, DWORD dwStateRemove,BOOL bUpdate/*=FALSE*/)
{
    DWORD dwOldState = m_dwState;

    TestMainThread();

    m_dwState &= ~dwStateRemove;
    m_dwState |= dwStateAdd;

    OnStateChanged(dwOldState,m_dwState);
    if(bUpdate && NeedRedrawWhenStateChange()) InvalidateRect(m_rcWindow);
    return dwOldState;
}

ULONG_PTR SWindow::GetUserData()
{
    return m_uData;
}

ULONG_PTR SWindow::SetUserData(ULONG_PTR uData)
{
    ULONG_PTR uOld=m_uData;
    m_uData=uData;
    return uOld;
}

BOOL SWindow::SetTimer(char id,UINT uElapse)
{
    STimerID timerID(m_swnd,id);
    return ::SetTimer(GetContainer()->GetHostHwnd(),DWORD(timerID),uElapse,NULL);
}

void SWindow::KillTimer(char id)
{
    STimerID timerID(m_swnd,id);
    ::KillTimer(GetContainer()->GetHostHwnd(),DWORD(timerID));
}


BOOL SWindow::SetTimer2( UINT_PTR id,UINT uElapse )
{
    return STimer2::SetTimer(m_swnd,id,uElapse);
}

void SWindow::KillTimer2( UINT_PTR id )
{
    STimer2::KillTimer(m_swnd,id);
}

SWND SWindow::GetSwnd()
{
    return m_swnd;
}


SWindow *SWindow::GetParent()
{
    return m_pParent;
}


SWindow * SWindow::GetTopLevelParent()
{
    SWindow *pParent=this;
    while(pParent->GetParent()) pParent=pParent->GetParent();
    return pParent;
}

void SWindow::SetParent(SWindow *pParent)
{
    if(m_pParent)
    {
        m_pParent->RemoveChild(this);
        m_pParent=NULL;
    }
    m_pParent=pParent;
    //根据Parent的状态来初始化自己的状态
    if(!m_pParent) return;
    m_pParent->InsertChild(this);
    if(m_pParent->IsVisible(TRUE) && m_bVisible)
        ModifyState(0,WndState_Invisible);
    else
        ModifyState(WndState_Invisible,0);
}

BOOL SWindow::DestroyChild(SWindow *pChild)
{
    if(this != pChild->GetParent()) return FALSE;
    pChild->Invalidate();
    pChild->SSendMessage(WM_DESTROY);
    RemoveChild(pChild);
    pChild->Release();
    return TRUE;
}

UINT SWindow::GetChildrenCount()
{
    return m_nChildrenCount;
}

SWindow * SWindow::GetChild(int nID)
{
    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        if(pChild->GetID()==nID) return pChild;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

void SWindow::SetChildContainer(SWindow *pChild)
{
    pChild->SetContainer(GetContainer());
}

void SWindow::InsertChild(SWindow *pNewChild,SWindow *pInsertAfter/*=ICWND_LAST*/)
{
    pNewChild->m_pParent=this;
    pNewChild->m_pPrevSibling=pNewChild->m_pNextSibling=NULL;
    SetChildContainer(pNewChild);

    if(pInsertAfter==m_pLastChild) pInsertAfter=ICWND_LAST;

    if(pInsertAfter==ICWND_LAST)
    {
        //insert window at head
        pNewChild->m_pPrevSibling=m_pLastChild;
        if(m_pLastChild) m_pLastChild->m_pNextSibling=pNewChild;
        else m_pFirstChild=pNewChild;
        m_pLastChild=pNewChild;
    }
    else if(pInsertAfter==ICWND_FIRST)
    {
        //insert window at tail
        pNewChild->m_pNextSibling=m_pFirstChild;
        if(m_pFirstChild) m_pFirstChild->m_pPrevSibling=pNewChild;
        else m_pLastChild=pNewChild;
        m_pFirstChild=pNewChild;
    }
    else
    {
        //insert window at middle
        SASSERT(pInsertAfter->m_pParent == this);
        SASSERT(m_pFirstChild && m_pLastChild);
        SWindow *pNext=pInsertAfter->m_pNextSibling;
        SASSERT(pNext);
        pInsertAfter->m_pNextSibling=pNewChild;
        pNewChild->m_pPrevSibling=pInsertAfter;
        pNewChild->m_pNextSibling=pNext;
        pNext->m_pPrevSibling=pNewChild;
    }
    m_nChildrenCount++;
}

BOOL SWindow::RemoveChild(SWindow *pChild)
{
    if(this != pChild->GetParent()) return FALSE;
    SWindow *pPrevSib=pChild->m_pPrevSibling;
    SWindow *pNextSib=pChild->m_pNextSibling;
    if(pPrevSib) pPrevSib->m_pNextSibling=pNextSib;
    else m_pFirstChild=pNextSib;
    if(pNextSib) pNextSib->m_pPrevSibling=pPrevSib;
    else m_pLastChild=pPrevSib;
    pChild->m_pParent=NULL;
    m_nChildrenCount--;
    return TRUE;
}

BOOL SWindow::IsChecked()
{
    return WndState_Check == (m_dwState & WndState_Check);
}

BOOL SWindow::IsDisabled(BOOL bCheckParent /*= FALSE*/)
{
    if(bCheckParent) return m_dwState & WndState_Disable;
    else return m_bDisable;
}

BOOL SWindow::IsVisible(BOOL bCheckParent /*= FALSE*/)
{
    if(bCheckParent) return (0 == (m_dwState & WndState_Invisible));
    else return m_bVisible;
}

//因为NotifyInvalidateRect只有窗口可见时再通知刷新，这里在窗口可见状态改变前后都执行一次通知。
void SWindow::SetVisible(BOOL bVisible,BOOL bUpdate/*=FALSE*/)
{
    if(bUpdate) InvalidateRect(m_rcWindow);
    SSendMessage(WM_SHOWWINDOW,bVisible);
    if(bUpdate) InvalidateRect(m_rcWindow);
}

void SWindow::EnableWindow( BOOL bEnable,BOOL bUpdate)
{
    SSendMessage(WM_ENABLE,bEnable);
    if(bUpdate) InvalidateRect(m_rcWindow);
}

void SWindow::SetCheck(BOOL bCheck)
{
    if (bCheck)
        ModifyState(WndState_Check, 0,TRUE);
    else
        ModifyState(0, WndState_Check,TRUE);
}

BOOL SWindow::NeedRedrawParent()
{
    return (GetContainer()->IsTranslucent() || !m_pBgSkin || (m_style.m_crBg == CR_INVALID));
}

ISwndContainer *SWindow::GetContainer()
{
    return m_pContainer;
}

void SWindow::SetContainer(ISwndContainer *pContainer)
{
    m_pContainer=pContainer;
    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->SetContainer(pContainer);
        pChild=pChild->m_pNextSibling;
    }
}

void SWindow::SetOwner(SWindow *pOwner)
{
    m_pOwner=pOwner;
}

SWindow *SWindow::GetOwner()
{
    return m_pOwner;
}

BOOL SWindow::IsMsgTransparent()
{
    return m_bMsgTransparent;
}

// add by dummyz@126.com
SwndStyle& SWindow::GetStyle()
{
    return m_style;
}


SWindow* SWindow::FindChildByID(int id)
{
    SWindow *pChild = m_pFirstChild;
    while(pChild)
    {
        if (pChild->GetID() == id)
            return pChild;
        SWindow *pChildFind=pChild->FindChildByID(id);
        if(pChildFind) return pChildFind;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

SWindow* SWindow::FindChildByName( LPCWSTR pszName )
{
    if(!pszName) return NULL;

    SWindow *pChild = m_pFirstChild;
    while(pChild)
    {
        if (!pChild->m_strName.IsEmpty() && wcscmp(pChild->m_strName, pszName)==0)
            return pChild;
        SWindow *pChildFind=pChild->FindChildByName(pszName);
        if(pChildFind) return pChildFind;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

BOOL SWindow::CreateChildren(pugi::xml_node xmlNode)
{
    for (pugi::xml_node xmlChild=xmlNode.first_child(); xmlChild; xmlChild=xmlChild.next_sibling())
    {
        if(xmlChild.type() != pugi::node_element) continue;

        if(_wcsicmp(xmlChild.name(),L"include")==0)
        {//在窗口布局中支持include标签
            pugi::xml_document xmlDoc;
            SStringTList strLst;

            if(2 == SplitString(S_CW2T(xmlChild.attribute(L"src").value()),_T(':'),strLst))
            {
                LOADXML(xmlDoc,strLst[1],strLst[0]);
            }else
            {
                LOADXML(xmlDoc,strLst[0],RT_LAYOUT);
            }
            if(xmlDoc)
            {
                CreateChildren(xmlDoc.child(L"include"));
            }else
            {
                SASSERT(FALSE);
            }

        }else
        {
            SWindow *pChild = SApplication::getSingleton().CreateWindowByName(xmlChild.name());
            if(pChild)
            {
                InsertChild(pChild);
                pChild->InitFromXml(xmlChild);
            }
        }
    }
    return TRUE;
}


SStringW SWindow::tr( const SStringW &strSrc )
{
    return TR(strSrc,GetContainer()->GetTranslatorContext());
}

// Create SWindow from xml element
BOOL SWindow::InitFromXml(pugi::xml_node xmlNode)
{
    SASSERT(m_pContainer);
    if (!xmlNode)
    {
        return FALSE;
    }

    m_strText = S_CW2T(tr(xmlNode.text().get()));   //使用语言包翻译。

    if (!m_strText.IsEmpty())
    {
        m_strText.TrimBlank();
        if (!m_strText.IsEmpty()) BUILDSTRING(m_strText);
    }

    m_layout.nCount = 0;
    m_layout.uPositionType = 0;
    
    //标记不处理width and height属性
    xmlNode.attribute(L"width").set_userdata(1);
    xmlNode.attribute(L"height").set_userdata(1);
    
    SObject::InitFromXml(xmlNode);

    if(!m_bVisible || (m_pParent && !m_pParent->IsVisible(TRUE)))
        ModifyState(WndState_Invisible, 0);

    if (4 != m_layout.nCount)
    {
        SStringW strValue = xmlNode.attribute(L"width").value();
        int nValue =_wtoi(strValue);
        
        if (0 == nValue && L"full" == strValue && 0 == m_layout.nCount)
        {
            m_rcWindow.right = 0;
            m_layout.uPositionType = (m_layout.uPositionType & ~SizeX_Mask) | SizeX_FitParent;
        }
        else
        {
            if (nValue > 0)
            {
                m_rcWindow.right = nValue;
                m_layout.uSpecifyWidth = nValue;
                m_layout.uPositionType = (m_layout.uPositionType & ~SizeX_Mask) | SizeX_Specify;
            }
            else
            {
                m_rcWindow.right = 0;
                m_layout.uPositionType = (m_layout.uPositionType & ~SizeX_Mask) | SizeX_FitContent;
            }
        }

        strValue = xmlNode.attribute(L"height").value();
        
        nValue =_wtoi(strValue);
        if (0 == nValue && L"full" == strValue)
        {
            m_rcWindow.bottom = 0;
            m_layout.uPositionType = (m_layout.uPositionType & ~SizeY_Mask) | SizeY_FitParent;
        }
        else
        {
            if (nValue > 0)
            {
                m_rcWindow.bottom = nValue;
                m_layout.uSpecifyHeight = nValue;
                m_layout.uPositionType = (m_layout.uPositionType & ~SizeY_Mask) | SizeY_Specify;
            }
            else
            {
                m_rcWindow.bottom = 0;
                m_layout.uPositionType = (m_layout.uPositionType & ~SizeY_Mask) | SizeY_FitContent;
            }
        }
    }

    if(0!=SSendMessage(WM_CREATE))
    {
        if(m_pParent)    m_pParent->DestroyChild(this);
        return FALSE;
    }
    CreateChildren(xmlNode);
    return TRUE;
}

SWindow * SWindow::CreateChildren(LPCWSTR pszXml)
{
    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_buffer(pszXml,wcslen(pszXml)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16)) return NULL;
    BOOL bLoaded=CreateChildren(xmlDoc);
    if(!bLoaded) return NULL;
    OnWindowPosChanged(NULL);
    return m_pLastChild;
}

// Hittest children
SWND SWindow::SwndFromPoint(CPoint ptHitTest, BOOL bOnlyText)
{
    if (!m_rcWindow.PtInRect(ptHitTest)) return NULL;

    CRect rcClient;
    GetClientRect(&rcClient);

    if(!rcClient.PtInRect(ptHitTest))
        return m_swnd;    //只在鼠标位于客户区时，才继续搜索子窗口
    
    SWND swndChild = NULL;

    SWindow *pChild=m_pLastChild;
    while(pChild)
    {
        if (pChild->IsVisible(TRUE) && !pChild->IsMsgTransparent())
        {
            swndChild = pChild->SwndFromPoint(ptHitTest, bOnlyText);

            if (swndChild) return swndChild;
        }

        pChild=pChild->m_pPrevSibling;
    }

    return m_swnd;
}

BOOL SWindow::NeedRedrawWhenStateChange()
{
    if (m_pBgSkin && !m_pBgSkin->IgnoreState())
    {
        return TRUE;
    }
    return m_style.GetStates()>1;
}

BOOL SWindow::_PaintRegion( IRenderTarget *pRT, IRegion *pRgn,SWindow *pWndCur,SWindow *pStart,SWindow *pEnd,PRSTATE & prState )
{
    RECT rcWnd;
    pWndCur->GetWindowRect(&rcWnd);
    if (!pWndCur->IsVisible(TRUE) || (pRgn && !pRgn->IsEmpty() && !pRgn->RectInRegion(&rcWnd)))
    {
        return FALSE;
    }
    if(prState == PRS_DRAWING && pWndCur == pEnd)
    {
        prState=PRS_MEETEND;
    }

    if(prState == PRS_MEETEND)
    {
        return FALSE;
    }

    if(prState == PRS_LOOKSTART && pWndCur==pStart)
    {
        prState=PRS_DRAWING;//开始进行绘制
    }

    PRSTATE prsBack=prState;    //保存当前的绘制状态,在递归结束后根据这个状态来判断是否需要绘制非客户区

    CRect rcClient;
    pWndCur->GetClientRect(&rcClient);
    if(!pRgn || pRgn->IsEmpty() || pRgn->RectInRegion(rcClient))
    {//重绘客户区
        if(prsBack == PRS_DRAWING)
        {
            if(pWndCur->IsClipClient())
            {
                pRT->PushClipRect(&rcClient);
            }
            if(pWndCur->IsDrawToCache())
            {
                CRect rcWnd=pWndCur->m_rcWindow;
                IRenderTarget *pRTCache=pWndCur->GetCachedRenderTarget();
                if(pWndCur->IsCacheDirty())
                {
                    pRTCache->SetViewportOrg(-rcWnd.TopLeft());
                    pRTCache->BitBlt(&rcWnd,pRT,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);//把父窗口的内容复制过来。
                    CAutoRefPtr<IFont> oldFont;
                    COLORREF crOld=pRT->GetTextColor();
                    pRTCache->SelectObject(pRT->GetCurrentObject(OT_FONT),(IRenderObj**)&oldFont);
                    pRTCache->SetTextColor(crOld);
                    
                    pWndCur->SSendMessage(WM_ERASEBKGND, (WPARAM)pRTCache);
                    pWndCur->SSendMessage(WM_PAINT, (WPARAM)pRTCache);
                    
                    pRTCache->SelectObject(oldFont);
                    pRTCache->SetTextColor(crOld);
                    
                    pWndCur->MarkCacheDirty(false);
                }
                pRT->BitBlt(&rcWnd,pRTCache,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);
            }else
            {
                pWndCur->SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
                pWndCur->SSendMessage(WM_PAINT, (WPARAM)pRT);
            }
        }

        SPainter painter;

        pWndCur->BeforePaint(pRT, painter);    //让子窗口继承父窗口的属性

        SWindow *pChild=pWndCur->GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(pChild==pEnd) break;
            _PaintRegion(pRT,pRgn,pChild,pStart,pEnd,prState);
            if(prState == PRS_MEETEND)
                break;
            pChild=pChild->GetWindow(GSW_NEXTSIBLING);
        }

        pWndCur->AfterPaint(pRT, painter);
        if(prsBack == PRS_DRAWING && pWndCur->IsClipClient())
        {
            pRT->PopClip();
        }
    }
    if(prsBack == PRS_DRAWING) 
        pWndCur->SSendMessage(WM_NCPAINT, (WPARAM)pRT);//ncpaint should be placed in tail of paint link

    return prState==PRS_DRAWING || prState == PRS_MEETEND;
}

void SWindow::RedrawRegion(IRenderTarget *pRT, IRegion *pRgn)
{
    PRSTATE prState=PRS_LOOKSTART;
    _PaintRegion(pRT,pRgn,this,this,NULL,prState);
}

void SWindow::Invalidate()
{
    CRect rcClient;
    GetClientRect(&rcClient);
    InvalidateRect(rcClient);
}

void SWindow::InvalidateRect(LPRECT lprect)
{
    if (lprect)
    {
        CRect rect = *lprect;
        InvalidateRect(rect);
    }
}

void SWindow::InvalidateRect(const CRect& rect)
{
    if(!IsVisible(TRUE)) return ;
    MarkCacheDirty(true);
    BOOL bUpdateLocked=FALSE;
    SWindow *pWnd=this;
    while(pWnd && !bUpdateLocked)
    {
        bUpdateLocked=pWnd->IsUpdateLocked();
        pWnd=pWnd->GetParent();
    }
    if (!bUpdateLocked)
    {
        GetContainer()->OnRedraw(rect);
    }
}

void SWindow::LockUpdate()
{
    m_bUpdateLocked=TRUE;
}

void SWindow::UnlockUpdate()
{
    m_bUpdateLocked=FALSE;
}

BOOL SWindow::IsUpdateLocked()
{
    return m_bUpdateLocked;
}

void SWindow::BringWindowToTop()
{
    SWindow *pParent=GetParent();
    if(!pParent) return;
    pParent->RemoveChild(this);
    pParent->InsertChild(this);
}

BOOL SWindow::FireEvent(EventArgs &evt)
{
    m_evtSet.FireEvent(evt);
    if(evt.handled != 0) return TRUE;

    if(GetOwner()) return GetOwner()->FireEvent(evt);
    return GetContainer()->OnFireEvent(evt);
}

LRESULT SWindow::OnWindowPosChanged(LPRECT lpRcContainer)
{
    LRESULT lRet=0;
    if(!(m_layout.uPositionType & Pos_Float))    
    {//窗口不是使用Move直接指定的坐标,计算出窗口位置
        m_rcWindow.left = m_rcWindow.top = m_rcWindow.right = m_rcWindow.bottom = POS_INIT;//注意先使原窗口坐标无效
        lRet=m_layout.CalcPosition(lpRcContainer,m_rcWindow);
    }
    if(lRet==0)
    {
        SSendMessage(WM_NCCALCSIZE);//计算非客户区大小

        CRect rcClient;
        GetClientRect(&rcClient);
        SSendMessage(WM_SIZE,0,MAKELPARAM(rcClient.Width(),rcClient.Height()));

        UpdateChildrenPosition();
    }
    return lRet;
}

int SWindow::OnCreate( LPVOID )
{
    return 0;
}

void SWindow::OnDestroy()
{
    //destroy children windows
    SWindow *pChild=m_pFirstChild;
    while (pChild)
    {
        SWindow *pNextChild=pChild->m_pNextSibling;
        pChild->SSendMessage(WM_DESTROY);
        pChild->Release();

        pChild=pNextChild;
    }
    m_pFirstChild=m_pLastChild=NULL;
    m_nChildrenCount=0;
}

// Draw background default
BOOL SWindow::OnEraseBkgnd(IRenderTarget *pRT)
{
    CRect rcClient;
    GetClientRect(&rcClient);
    if (!m_pBgSkin)
    {
        COLORREF crBg = m_style.m_crBg;

        if (CR_INVALID != crBg)
        {
            pRT->FillSolidRect(&rcClient,crBg);
        }
    }
    else
    {
        int nState=0;
        
        if(GetState()&WndState_Disable)
        {
            nState=3;
        }
        else if(GetState()&WndState_Check || GetState()&WndState_PushDown)
        {
            nState=2;
        }else if(GetState()&WndState_Hover)
        {
            nState=1;
        }
        if(nState>=m_pBgSkin->GetStates()) nState=0;
        m_pBgSkin->Draw(pRT, rcClient, nState,m_byAlpha); 
    }
    return TRUE;
}

void SWindow::BeforePaint(IRenderTarget *pRT, SPainter &painter)
{
    IFontPtr pFont = m_style.GetTextFont(IIF_STATE4(m_dwState,0,1,2,3));
    if(pFont) 
        pRT->SelectObject(pFont,(IRenderObj**)&painter.pOldPen);
    
    COLORREF crTxt = m_style.GetTextColor(IIF_STATE4(m_dwState,0,1,2,3));
    if(crTxt != CR_INVALID)
        painter.crOld = pRT->SetTextColor(crTxt);
}


void SWindow::_BeforePaintEx( SWindow *pWnd,IRenderTarget *pRT )
{
    SWindow *pParent=pWnd->GetParent();
    if(pParent) _BeforePaintEx(pParent,pRT);
    SPainter painter;
    pWnd->BeforePaint(pRT,painter);
}

void SWindow::BeforePaintEx(IRenderTarget *pRT)
{
    IFont *pDefFont = SFontPool::getSingleton().GetFont(FF_DEFAULTFONT);
    pRT->SelectObject(pDefFont);
    _BeforePaintEx(this,pRT);
}

void SWindow::AfterPaint(IRenderTarget *pRT, SPainter &painter)
{
    if(painter.pOldPen) pRT->SelectObject(painter.pOldPen);
    if(painter.crOld!=CR_INVALID) pRT->SetTextColor(painter.crOld);
}

// Draw inner text default and focus rect
void SWindow::OnPaint(IRenderTarget *pRT)
{
    SPainter painter;

    BeforePaint(pRT, painter);

    CRect rcText;
    GetTextRect(rcText);
    DrawText(pRT,m_strText, m_strText.GetLength(), rcText, GetTextAlign());

    //draw focus rect
    if(GetContainer()->SwndGetFocus()==m_swnd)
    {
        DrawFocus(pRT);
    }

    AfterPaint(pRT, painter);
}

void SWindow::OnNcPaint(IRenderTarget *pRT)
{
    if(m_style.m_nMarginX!=0 || m_style.m_nMarginY!=0)
    {
        BOOL bGetRT = pRT==0;
        if(bGetRT) pRT=GetRenderTarget(&m_rcWindow,OLEDC_OFFSCREEN,FALSE);//不自动画背景

        CRect rcClient;
        SWindow::GetClientRect(&rcClient);
        pRT->SaveClip(NULL);
        pRT->ExcludeClipRect(&rcClient);

        if(bGetRT) PaintBackground(pRT,&m_rcWindow);

        int nState=0;
        if(WndState_Hover & m_dwState) nState=1;
        if(m_pNcSkin)
        {
            if(nState>=m_pNcSkin->GetStates()) nState=0;
            m_pNcSkin->Draw(pRT,m_rcWindow,nState,m_byAlpha);
        }
        else
        {
            COLORREF crBg = m_style.m_crBorder;
            if (CR_INVALID != crBg)
            {
                pRT->FillSolidRect(&m_rcWindow,crBg);
            }
        }
        if(bGetRT) PaintForeground(pRT,&m_rcWindow);
        pRT->PopClip();
        if(bGetRT) ReleaseRenderTarget(pRT);
    }
}

CSize SWindow::GetDesiredSize(LPRECT pRcContainer)
{
    SASSERT(m_layout.IsFitContent());


    int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

    CRect rcTest (0,0,0x7FFF,0x7FFF);
    if(m_nMaxWidth!=-1)
    {
        rcTest.right=m_nMaxWidth;
        nTestDrawMode|=DT_WORDBREAK;
    }
    
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
    BeforePaintEx(pRT);
    DrawText(pRT,m_strText, m_strText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);
    rcTest.right += m_style.m_nMarginX * 2;
    rcTest.bottom += m_style.m_nMarginY * 2;
    return rcTest.Size();
}

void SWindow::GetTextRect( LPRECT pRect )
{
    GetClientRect(pRect);
}

void SWindow::DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    pRT->DrawText(pszBuf,cchText,pRect,uFormat,m_byAlpha);
}

void SWindow::DrawFocus(IRenderTarget *pRT)
{
    CRect rcFocus;
    GetTextRect(&rcFocus);
    if(IsFocusable())    DrawDefFocusRect(pRT,rcFocus);
}


void SWindow::DrawDefFocusRect(IRenderTarget *pRT,CRect rcFocus )
{
    rcFocus.DeflateRect(2,2);
    CAutoRefPtr<IPen> pPen,oldPen;
    pRT->CreatePen(PS_DOT,RGBA(88,88,88,m_byAlpha),1,&pPen);
    pRT->SelectObject(pPen,(IRenderObj**)&oldPen);
    pRT->DrawRectangle(&rcFocus);    
    pRT->SelectObject(oldPen);
}

UINT SWindow::OnGetDlgCode()
{
    return 0;
}

BOOL SWindow::IsFocusable()
{
    return m_bFocusable;
}

BOOL SWindow::OnDefKeyDown(UINT nChar, UINT nFlags)
{
    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        if(pChild->OnDefKeyDown(nChar,nFlags)) return TRUE;
        pChild=pChild->m_pNextSibling;
    }
    return FALSE;
}

void SWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
    if(nStatus == ParentShow)
    {
        if(bShow && !IsVisible(FALSE)) bShow=FALSE;
    }
    else
    {
        m_bVisible=bShow;
    }
    if(bShow && m_pParent)
    {
        bShow=m_pParent->IsVisible(TRUE);
    }

    if (bShow)
        ModifyState(0, WndState_Invisible);
    else
        ModifyState(WndState_Invisible, 0);

    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->SSendMessage(WM_SHOWWINDOW,bShow,ParentShow);
        pChild=pChild->GetWindow(GSW_NEXTSIBLING);
    }
    if(!IsVisible(TRUE) && m_swnd == GetContainer()->SwndGetFocus())
    {
        GetContainer()->OnSetSwndFocus(NULL);
    }

    if(!m_bDisplay)
    {
        SWindow *pParent=GetParent();
        if(pParent) pParent->UpdateChildrenPosition();
    }
}


void SWindow::OnEnable( BOOL bEnable,UINT nStatus )
{
    if(nStatus == ParentEnable)
    {
        if(bEnable && IsDisabled(FALSE)) bEnable=FALSE;
    }
    else
    {
        m_bDisable=!bEnable;
    }
    if(bEnable && m_pParent)
    {
        bEnable=!m_pParent->IsDisabled(TRUE);
    }

    if (bEnable)
        ModifyState(0, WndState_Disable);
    else
        ModifyState(WndState_Disable, WndState_Hover);

    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->SSendMessage(WM_ENABLE,bEnable,ParentEnable);
        pChild=pChild->GetWindow(GSW_NEXTSIBLING);
    }
    if(IsDisabled(TRUE) && m_swnd == GetContainer()->SwndGetFocus())
    {
        GetContainer()->OnSetSwndFocus(NULL);
    }
}

void SWindow::OnLButtonDown(UINT nFlags,CPoint pt)
{
    if(m_bFocusable) SetFocus();
    SetCapture();
    ModifyState(WndState_PushDown, 0,TRUE);
}

void SWindow::OnLButtonUp(UINT nFlags,CPoint pt)
{
    ReleaseCapture();
    ModifyState(0, WndState_PushDown,TRUE);
    if(!m_rcWindow.PtInRect(pt)) return;


    if (GetID() || GetName())
    {
        FireCommand();
    }
}

void SWindow::OnRButtonDown( UINT nFlags, CPoint point )
{
    FireCtxMenu(point);
}

void SWindow::OnMouseHover(WPARAM wParam, CPoint ptPos)
{
    if(GetCapture()==m_swnd)
        ModifyState(WndState_PushDown,0,FALSE);
    ModifyState(WndState_Hover, 0,TRUE);
    OnNcPaint(0);
}

void SWindow::OnMouseLeave()
{
    if(GetCapture()==m_swnd)
        ModifyState(0,WndState_PushDown,FALSE);
    ModifyState(0,WndState_Hover,TRUE);
    OnNcPaint(0);
}

BOOL SWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL bRet=FALSE;
    if(m_pParent) bRet=(BOOL)m_pParent->SSendMessage(WM_MOUSEWHEEL,MAKEWPARAM(nFlags,zDelta),MAKELPARAM(pt.x,pt.y));
    return bRet;
}

CRect SWindow::GetChildrenLayoutRect()
{
    CRect rcRet;
    GetClientRect(rcRet);//通常是非客户区，但是tab这样的控件不一样。
    return rcRet;
}


void SWindow::ClearLayoutState()
{
    if(m_layout.IsFloat()) return;

    m_rcWindow.left=m_rcWindow.top=m_rcWindow.right=m_rcWindow.bottom=POS_INIT;
}

void SWindow::UpdateChildrenPosition()
{
    SList<SWindow*> lstWnd;
    SWindow *pChild=GetWindow(GSW_FIRSTCHILD);
    while(pChild)
    {
        pChild->ClearLayoutState();
        lstWnd.AddTail(pChild);
        pChild=pChild->GetWindow(GSW_NEXTSIBLING);
    }
    m_layout.CalcChildrenPosition(&lstWnd);
    Invalidate();
}

void SWindow::OnSetFocus()
{
    EventSetFocus evt(this);
    FireEvent(evt);
    InvalidateRect(m_rcWindow);
}

void SWindow::OnKillFocus()
{
    EventKillFocus evt(this);
    FireEvent(evt);
    InvalidateRect(m_rcWindow);
}

IRenderTarget * SWindow::GetRenderTarget(const LPRECT pRc/*=NULL*/,DWORD gdcFlags/*=0*/,BOOL bClientDC/*=TRUE*/)
{
    SASSERT(m_gdcFlags==-1);
    if(bClientDC)
        GetClientRect(&m_rcGetRT);
    else
        m_rcGetRT=m_rcWindow;
    
    
    if(gdcFlags!=OLEDC_NODRAW)
    {//将DC限制在父窗口的可见区域
        SWindow *pParent=GetParent();
        while(pParent)
        {
            CRect rcParent;
            pParent->GetClientRect(&rcParent);
            m_rcGetRT.IntersectRect(m_rcGetRT,rcParent);
            pParent=pParent->GetParent();
        }
    }

    m_gdcFlags=gdcFlags;
    m_bClipRT=FALSE;
    if(pRc)
    {
        m_rcGetRT.IntersectRect(pRc,&m_rcGetRT);
        m_bClipRT=!m_rcGetRT.EqualRect(pRc);
    }
    IRenderTarget *pRT=GetContainer()->OnGetRenderTarget(m_rcGetRT,gdcFlags);   //不管是不是cache都直接从container获取RT
    SASSERT(pRT);
    
    if(m_bClipRT)
    {
        pRT->PushClipRect(&m_rcGetRT,RGN_COPY);
    }
    if(gdcFlags&OLEDC_PAINTBKGND)
    {
        PaintBackground(pRT,&m_rcGetRT);
    }
        
    BeforePaintEx(pRT);
    return pRT;
}

void SWindow::ReleaseRenderTarget(IRenderTarget *pRT)
{
    if(m_gdcFlags & OLEDC_PAINTBKGND) //画了背景，自动画前景
        PaintForeground(pRT,&m_rcGetRT);
    if(m_bClipRT)
        pRT->PopClip();
        
    GetContainer()->OnReleaseRenderTarget(pRT,m_rcGetRT,m_gdcFlags);
    //调用GetRenderTarget后通常是要在上面绘制即时图象，但是并不会更新到缓存上，因此要标记缓存脏
    if(m_bCacheDraw) MarkCacheDirty(true);

    m_bClipRT=FALSE;
    m_gdcFlags=-1;
}

SWND SWindow::GetCapture()
{
    return GetContainer()->OnGetSwndCapture();
}

SWND SWindow::SetCapture()
{
    return GetContainer()->OnSetSwndCapture(m_swnd);
}

BOOL SWindow::ReleaseCapture()
{
    return GetContainer()->OnReleaseSwndCapture();
}

void SWindow::SetFocus()
{
    GetContainer()->OnSetSwndFocus(m_swnd);
}

void SWindow::KillFocus()
{
    if(GetContainer()->SwndGetFocus()==m_swnd)
    {
        GetContainer()->OnSetSwndFocus(NULL);
    }
}

BOOL SWindow::OnNcHitTest(CPoint pt)
{
    return FALSE;
}

SWindow * SWindow::GetWindow(int uCode)
{
    SWindow *pRet=NULL;
    switch(uCode)
    {
    case GSW_FIRSTCHILD:
        pRet=m_pFirstChild;
        break;
    case GSW_LASTCHILD:
        pRet=m_pLastChild;
        break;
    case GSW_PREVSIBLING:
        pRet=m_pPrevSibling;
        break;
    case GSW_NEXTSIBLING:
        pRet=m_pNextSibling;
        break;
    case GSW_OWNER:
        pRet=m_pOwner;
        break;
    case GSW_PARENT:
        pRet=m_pParent;
        break;
    }
    return pRet;
}


void SWindow::PaintBackground(IRenderTarget *pRT,LPRECT pRc )
{
    CRect rcDraw=m_rcWindow;
    if(pRc) rcDraw.IntersectRect(rcDraw,pRc);
    pRT->PushClipRect(&rcDraw);
    
    SWindow *pTopWnd=GetTopLevelParent();
    CAutoRefPtr<IRegion> pRgn;
    GETRENDERFACTORY->CreateRegion(&pRgn);
    pRgn->CombineRect(&rcDraw,RGN_COPY);
    
    pRT->FillSolidRect(&rcDraw,0);//清除残留的alpha值
    PRSTATE prState=PRS_LOOKSTART;
    _PaintRegion(pRT,pRgn,pTopWnd,pTopWnd,this,prState);
    pRT->PopClip();
}

void SWindow::PaintForeground( IRenderTarget *pRT,LPRECT pRc )
{
    CRect rcDraw=m_rcWindow;
    if(pRc) rcDraw.IntersectRect(rcDraw,pRc);

    SWindow *pStart=GetNextVisibleWindow(this,rcDraw);

    if(pStart)
    {
        PRSTATE prState=PRS_LOOKSTART;
        CAutoRefPtr<IRegion> pRgn;
        GETRENDERFACTORY->CreateRegion(&pRgn);
        pRgn->CombineRect(&rcDraw,RGN_COPY);
        pRT->PushClipRect(&rcDraw);
        _PaintRegion(pRT,pRgn,this,pStart,NULL,prState);
        pRT->PopClip();
    }
}


SWindow * SWindow::GetNextVisibleWindow( SWindow *pWnd ,const CRect &rcDraw)
{
    if(!pWnd) return NULL;
    SWindow *pNextSibling=pWnd->GetWindow(GSW_NEXTSIBLING);
    if(pNextSibling && pNextSibling->IsVisible(TRUE) && !(pNextSibling->m_rcWindow & rcDraw).IsRectEmpty())
        return pNextSibling;
    else if (pNextSibling)    return GetNextVisibleWindow(pNextSibling,rcDraw);
    else return GetNextVisibleWindow(pWnd->GetParent(),rcDraw);
}

void SWindow::DrawAniStep( CRect rcFore,CRect rcBack,IRenderTarget * pRTFore,IRenderTarget * pRTBack,CPoint ptAnchor)
{
    IRenderTarget * pRT=GetRenderTarget(rcBack,OLEDC_OFFSCREEN,FALSE);
    pRT->BitBlt(&rcBack,pRTBack,rcBack.left,rcBack.top,SRCCOPY|0x80000000);
    pRT->BitBlt(&rcFore,pRTFore,ptAnchor.x,ptAnchor.y,SRCCOPY|0x80000000);
    PaintForeground(pRT,rcBack);//画前景
    ReleaseRenderTarget(pRT);
}

void SWindow::DrawAniStep( CRect rcWnd,IRenderTarget * pRTFore,IRenderTarget * pRTBack,BYTE byAlpha)
{
    IRenderTarget * pRT=GetRenderTarget(rcWnd,OLEDC_OFFSCREEN,FALSE);
    if(byAlpha>0 && byAlpha<255)
    {
        pRT->BitBlt(&rcWnd,pRTBack,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);
        pRT->AlphaBlend(&rcWnd,pRTFore,&rcWnd,byAlpha);
    }else if(byAlpha==0)
    {
        pRT->BitBlt(&rcWnd,pRTBack,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);
    }else if(byAlpha==255)
    {
        pRT->BitBlt(&rcWnd,pRTFore,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);
    }
    PaintForeground(pRT,rcWnd);//画前景
    ReleaseRenderTarget(pRT);
}

BOOL SWindow::AnimateWindow(DWORD dwTime,DWORD dwFlags )
{
    if(dwFlags & AW_HIDE)
    {
        if(!IsVisible(TRUE))
            return FALSE;
    }else
    {//动画显示窗口时，不能是最顶层窗口，同时至少上一层窗口应该可见
        if(IsVisible(TRUE))
            return FALSE;
        SWindow *pParent=GetParent();
        if(!pParent) return FALSE;
        if(!pParent->IsVisible(TRUE)) return FALSE;
    }
    CRect rcWnd;
    GetWindowRect(&rcWnd);

    CAutoRefPtr<IRegion> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);
    rgn->CombineRect(&rcWnd,RGN_COPY);

    IRenderTarget *pRT=GetRenderTarget(rcWnd,OLEDC_NODRAW,FALSE);
    CAutoRefPtr<IRenderTarget> pRTBefore;
    GETRENDERFACTORY->CreateRenderTarget(&pRTBefore,rcWnd.Width(),rcWnd.Height());
    pRTBefore->OffsetViewportOrg(-rcWnd.left,-rcWnd.top);

    //渲染窗口变化前状态
    PaintBackground(pRT,rcWnd);
    RedrawRegion(pRT,rgn);
    pRTBefore->BitBlt(&rcWnd,pRT,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);
    
    //更新窗口可见性
    SetVisible(!(dwFlags&AW_HIDE),FALSE);
    //窗口变化后
    CAutoRefPtr<IRenderTarget> pRTAfter;
    GETRENDERFACTORY->CreateRenderTarget(&pRTAfter,rcWnd.Width(),rcWnd.Height());
    pRTAfter->OffsetViewportOrg(-rcWnd.left,-rcWnd.top);

    PaintBackground(pRT,rcWnd);
    RedrawRegion(pRT,rgn);
    pRTAfter->BitBlt(&rcWnd,pRT,rcWnd.left,rcWnd.top,SRCCOPY|0x80000000);

    ReleaseRenderTarget(pRT);

    int nSteps=dwTime/10;
    if(dwFlags & AW_HIDE)
    {//hide
        if(dwFlags& AW_SLIDE)
        {
            CRect rcNewState(rcWnd);
            LONG  x1 = rcNewState.left;
            LONG  x2 = rcNewState.left;
            LONG  y1 = rcNewState.top;
            LONG  y2 = rcNewState.top;
            LONG * x =&rcNewState.left;
            LONG * y =&rcNewState.top;

            if(dwFlags & AW_HOR_POSITIVE)
            {//left->right:move left
                x1=rcNewState.left,x2=rcNewState.right;
                x=&rcNewState.left;
            }else if(dwFlags & AW_HOR_NEGATIVE)
            {//right->left:move right
                x1=rcNewState.right,x2=rcNewState.left;
                x=&rcNewState.right;
            }
            if(dwFlags & AW_VER_POSITIVE)
            {//top->bottom
                y1=rcNewState.top,y2=rcNewState.bottom;
                y=&rcNewState.top;
            }else if(dwFlags & AW_VER_NEGATIVE)
            {//bottom->top
                y1=rcNewState.bottom,y2=rcNewState.top;
                y=&rcNewState.bottom;
            }
            LONG xStepLen=(x2-x1)/nSteps;
            LONG yStepLen=(y2-y1)/nSteps;

            CPoint ptAnchor;
            for(int i=0;i<nSteps;i++)
            {
                *x+=xStepLen;
                *y+=yStepLen;
                ptAnchor=rcWnd.TopLeft();
                if(dwFlags & AW_HOR_NEGATIVE)
                {//right->left:move right
                    ptAnchor.x=rcWnd.right-rcNewState.Width();
                }
                if(dwFlags & AW_VER_NEGATIVE)
                {
                    ptAnchor.y=rcWnd.bottom-rcNewState.Height();
                }
                DrawAniStep(rcNewState,rcWnd,pRTBefore,pRTAfter,ptAnchor);
                Sleep(10);
            }
            DrawAniStep(CRect(),rcWnd,pRTBefore,pRTAfter,rcWnd.TopLeft());
            return TRUE;
        }else if(dwFlags&AW_CENTER)
        {
            CRect rcNewState(rcWnd);
            int xStep=rcNewState.Width()/(2*nSteps);
            int yStep=rcNewState.Height()/(2*nSteps);
            for(int i=0;i<nSteps;i++)
            {
                rcNewState.DeflateRect(xStep,yStep);
                DrawAniStep(rcNewState,rcWnd,pRTBefore,pRTAfter,rcNewState.TopLeft());
                Sleep(10);
            }
            DrawAniStep(CRect(),rcWnd,pRTBefore,pRTAfter,rcWnd.TopLeft());
            return TRUE;
        }else if(dwFlags&AW_BLEND)
        {
            BYTE byAlpha=255;
            BYTE byStepLen=255/nSteps;
            for(int i=0;i<nSteps;i++)
            {
                DrawAniStep(rcWnd,pRTBefore,pRTAfter,byAlpha);
                Sleep(10);
                byAlpha-=byStepLen;
            }
            DrawAniStep(rcWnd,pRTBefore,pRTAfter,0);
            return TRUE;
        }
        return FALSE;
    }else
    {//show
        if(dwFlags& AW_SLIDE)
        {
            CRect rcNewState(rcWnd);
            LONG  x1 = rcNewState.left;
            LONG  x2 = rcNewState.left;
            LONG  y1 = rcNewState.top;
            LONG  y2 = rcNewState.top;
            LONG * x =&rcNewState.left;
            LONG * y =&rcNewState.top;

            if(dwFlags & AW_HOR_POSITIVE)
            {//left->right:move right
                x1=rcNewState.left,x2=rcNewState.right;
                rcNewState.right=rcNewState.left,x=&rcNewState.right;
            }else if(dwFlags & AW_HOR_NEGATIVE)
            {//right->left:move left
                x1=rcNewState.right,x2=rcNewState.left;
                rcNewState.left=rcNewState.right,x=&rcNewState.left;
            }
            if(dwFlags & AW_VER_POSITIVE)
            {//top->bottom
                y1=rcNewState.top,y2=rcNewState.bottom;
                rcNewState.bottom=rcNewState.top,y=&rcNewState.bottom;
            }else if(dwFlags & AW_VER_NEGATIVE)
            {//bottom->top
                y1=rcNewState.bottom,y2=rcNewState.top;
                rcNewState.top=rcNewState.bottom,y=&rcNewState.top;
            }
            LONG xStepLen=(x2-x1)/nSteps;
            LONG yStepLen=(y2-y1)/nSteps;

            CPoint ptAnchor;
            for(int i=0;i<nSteps;i++)
            {
                *x+=xStepLen;
                *y+=yStepLen;
                ptAnchor=rcWnd.TopLeft();
                if(dwFlags & AW_HOR_POSITIVE)
                {//left->right:move left
                    ptAnchor.x=rcWnd.right-rcNewState.Width();
                }
                if(dwFlags & AW_VER_POSITIVE)
                {
                    ptAnchor.y=rcWnd.bottom-rcNewState.Height();
                }
                DrawAniStep(rcNewState,rcWnd,pRTAfter,pRTBefore,ptAnchor);
                Sleep(10);
            }
            DrawAniStep(rcWnd,rcWnd,pRTAfter,pRTBefore,rcWnd.TopLeft());
            return TRUE;
        }else if(dwFlags&AW_CENTER)
        {
            CRect rcNewState(rcWnd);
            int xStep=rcNewState.Width()/(2*nSteps);
            int yStep=rcNewState.Height()/(2*nSteps);
            rcNewState.left=rcNewState.right=(rcNewState.left+rcNewState.right)/2;
            rcNewState.top=rcNewState.bottom=(rcNewState.top+rcNewState.bottom)/2;
            for(int i=0;i<nSteps;i++)
            {
                rcNewState.InflateRect(xStep,yStep);
                DrawAniStep(rcNewState,rcWnd,pRTAfter,pRTBefore,rcNewState.TopLeft());
                Sleep(10);
            }
            DrawAniStep(rcWnd,rcWnd,pRTAfter,pRTBefore,rcWnd.TopLeft());
            return TRUE;
        }else if(dwFlags&AW_BLEND)
        {
            BYTE byAlpha=0;
            BYTE byStepLen=255/nSteps;
            for(int i=0;i<nSteps;i++)
            {
                DrawAniStep(rcWnd,pRTAfter,pRTBefore,byAlpha);
                Sleep(10);
                byAlpha+=byStepLen;
            }
            DrawAniStep(rcWnd,pRTAfter,pRTBefore,255);
            return TRUE;
        }
        return FALSE;
    }
}

BOOL SWindow::FireCommand()
{
    EventCmd evt(this);
    return FireEvent(evt);
}

BOOL SWindow::FireCtxMenu( CPoint pt )
{
    EventCtxMenu evt(this);
    evt.pt=pt;
    FireEvent(evt);
    return evt.bCancel;
}

//////////////////////////////////////////////////////////////////////////
HRESULT SWindow::OnAttrPos(const SStringW& strValue, BOOL bLoading)
{
    if (strValue.IsEmpty()) return E_FAIL;
    m_layout.ParseStrPostion(strValue);
    if(!bLoading)
    {
        SWindow *pParent=GetParent();
        SASSERT(pParent);
        pParent->UpdateChildrenPosition();
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrVisible( const SStringW& strValue, BOOL bLoading )
{
    BOOL bVisible = strValue != L"0";
    if(!bLoading)   SetVisible(bVisible,TRUE);
    else m_bVisible=bVisible;
    return S_FALSE;
}

HRESULT SWindow::OnAttrEnable( const SStringW& strValue, BOOL bLoading )
{
    BOOL bEnable = strValue != L"0";
    if(bLoading)
    {
        if (bEnable)
            ModifyState(0, WndState_Disable);
        else
            ModifyState(WndState_Disable, WndState_Hover);
    }
    else
    {
        EnableWindow(bEnable,TRUE);
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrDisplay( const SStringW& strValue, BOOL bLoading )
{
    m_bDisplay = strValue != L"0";
    if(!bLoading && !IsVisible(TRUE))
    {
        SWindow *pParent=GetParent();
        SASSERT(pParent);
        pParent->UpdateChildrenPosition();
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrSkin( const SStringW& strValue, BOOL bLoading )
{
    m_pBgSkin = GETSKIN(strValue);
    if(!bLoading && m_layout.IsFitContent())
    {
        SWindow *pParent=GetParent();
        SASSERT(pParent);
        pParent->UpdateChildrenPosition();
    }
    return S_FALSE;
}

HRESULT SWindow::OnAttrClass( const SStringW& strValue, BOOL bLoading )
{
    BOOL bGet=GETSTYLE(strValue,m_style);
    if(!bGet) return E_FAIL;
    if(!m_style.m_strSkinName.IsEmpty())
    {
        m_pBgSkin = GETSKIN(m_style.m_strSkinName);
    }
    if(!m_style.m_strNcSkinName.IsEmpty())
    {
        m_pNcSkin = GETSKIN(m_style.m_strNcSkinName);
    }
    if(!bLoading)
    {
        if(m_layout.IsFitContent())
        {
            SWindow *pParent=GetParent();
            SASSERT(pParent);
            pParent->UpdateChildrenPosition();
        }else
        {
            InvalidateRect(m_rcWindow);
        }
    }
    return S_FALSE;
}

void SWindow::OnSize( UINT nType, CSize size )
{
    if(m_bCacheDraw)
    {
        if(!m_cachedRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT,m_rcWindow.Width(),m_rcWindow.Height());
        }else
        {
            m_cachedRT->Resize(m_rcWindow.Size());
        }
        MarkCacheDirty(true);
    }
}

HRESULT SWindow::OnAttrCache( const SStringW& strValue, BOOL bLoading )
{
    m_bCacheDraw = strValue != L"0";
    
    if(!bLoading)
    {
        if(m_bCacheDraw && !m_cachedRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT,m_rcWindow.Width(),m_rcWindow.Height());
            BeforePaintEx(m_cachedRT);  //从父窗口中继承字体等属性
        }
        if(!m_bCacheDraw && m_cachedRT)
        {
            m_cachedRT=NULL;
        }
        m_bDirty=TRUE;
        InvalidateRect(m_rcWindow);
    }
    return S_FALSE;
}

SWindow * SWindow::GetSelectedChildInGroup()
{
    SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
    if(!pChild || !pChild->IsSiblingsAutoGroupped()) return NULL;
    return pChild->GetSelectedSiblingInGroup();
}

}//namespace SOUI
