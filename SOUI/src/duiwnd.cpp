#include "duistd.h"
#include "duiwnd.h"
#include "color.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
// SWindow Implement
//////////////////////////////////////////////////////////////////////////

SWindow::SWindow()
    : m_hSWnd(DuiWindowMgr::NewWindow(this))
    , m_pContainer(NULL)
    , m_pParent(NULL),m_pFirstChild(NULL),m_pLastChild(NULL),m_pNextSibling(NULL),m_pPrevSibling(NULL)
    , m_nChildrenCount(0)
    , m_bMsgHandled(FALSE)
    , m_uCmdID(NULL)
    , m_dwState(DuiWndState_Normal)
    , m_bMsgTransparent(FALSE)
    , m_bVisible(TRUE)
    , m_bDisplay(TRUE)
    , m_bDisable(FALSE)
    , m_nSepSpace(2)
    , m_nMaxWidth(-1)
    , m_bUpdateLocked(FALSE)
    , m_bClipClient(FALSE)
    , m_bTabStop(FALSE)
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
    addEvent(NM_COMMAND);
}

SWindow::~SWindow()
{
    DuiWindowMgr::DestroyWindow(m_hSWnd);
}


//////////////////////////////////////////////////////////////////////////
// Method Define

// Get align
UINT SWindow::GetTextAlign()
{
    return m_style.GetTextAlign() ;
}

// Get position type
DWORD SWindow::GetPositionType()
{
    return m_dlgpos.uPositionType;
}

// Set position type
void SWindow::SetPositionType(DWORD dwPosType, DWORD dwMask /*= 0xFFFFFFFF*/)
{
    m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~dwMask) | (dwPosType & dwMask);
}

void SWindow::SetFixSize(int nWid,int nHei)
{
    m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeX_Mask) | SizeX_Specify|SizeY_Specify;
    m_dlgpos.uSpecifyWidth = nWid;
    m_dlgpos.uSpecifyHeight = nHei;
}

void SWindow::SetBkColor(COLORREF cr)
{
    m_style.m_crBg=cr;
}

// Get DuiWindow rect(position in container)
void SWindow::GetRect(LPRECT prect)
{
    DUIASSERT(prect);
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

void SWindow::GetClient(LPRECT pRect)
{
    DUIASSERT(pRect);
    *pRect=m_rcWindow;
    pRect->left+=m_style.m_nMarginX;
    pRect->right-=m_style.m_nMarginX;
    pRect->top+=m_style.m_nMarginY;
    pRect->bottom-=m_style.m_nMarginY;
}

void SWindow::GetDlgPosition(DUIWND_POSITION *pPos)
{
    if (pPos)
        memcpy(pPos, &m_dlgpos, sizeof(DUIWND_POSITION));
}

// Get inner text
LPCTSTR SWindow::GetInnerText()
{
    return m_strWndText;
}

// Get tooltip text
BOOL SWindow::OnUpdateToolTip(HSWND hCurTipHost,HSWND &hNewTipHost,CRect &rcTip,CDuiStringT &strTip)
{
    if(m_hSWnd==hCurTipHost) return FALSE;
    hNewTipHost=m_hSWnd;
    GetRect(&rcTip);
    strTip=m_strToolTipText;
    return TRUE;
}

// Set inner text
HRESULT SWindow::SetInnerText(LPCTSTR lpszText)
{
    m_strWndText = lpszText;
    if(IsVisible(TRUE)) NotifyInvalidate();
    if ((m_dlgpos.uPositionType & (SizeX_FitContent | SizeY_FitContent)) && (4 != m_dlgpos.nCount))
    {
        OnWindowPosChanged(NULL);
        if(IsVisible(TRUE)) NotifyInvalidate();
    }
    return S_OK;
}

VOID SWindow::TestMainThread()
{
#ifdef DEBUG
    // 当你看到这个东西的时候，我不幸的告诉你，你的其他线程在刷界面
    // 这是一件很危险的事情
    DWORD dwCurThreadID = GetCurrentThreadId();

    BOOL bOK = (m_nMainThreadId == dwCurThreadID); // 当前线程和构造对象时的线程一致

    DUIASSERT(bOK);
#endif
}


// Send a message to DuiWindow
LRESULT SWindow::DuiSendMessage(UINT Msg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/,BOOL *pbMsgHandled/*=NULL*/)
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
    DUIMSG msgCur= {Msg,wParam,lParam};
    DUIMSG *pOldMsg=m_pCurMsg;
    m_pCurMsg=&msgCur;

    BOOL bOldMsgHandle=IsMsgHandled();//备分上一个消息的处理状态

    SetMsgHandled(FALSE);

    ProcessDuiWndMessage(Msg, wParam, lParam, lResult);

    if(pbMsgHandled) *pbMsgHandled=IsMsgHandled();

    SetMsgHandled(bOldMsgHandle);//恢复上一个消息的处理状态

    m_pCurMsg=pOldMsg;
    Release();
    return lResult;
}

// Move DuiWindow to new place
//
void SWindow::Move(LPRECT prect)
{
    DUIASSERT(prect);
    TestMainThread();

    if(m_rcWindow.EqualRect(prect)) return;

    m_rcWindow = prect;
    m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~Position_Mask)|Pos_Float;//使用Move后，程序不再自动计算窗口坐标
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
BOOL SWindow::OnDuiSetCursor(const CPoint &pt)
{
    HCURSOR hCur = ::LoadCursor(NULL, m_style.m_lpCursorName);
    ::SetCursor(hCur);
    return TRUE;
}

// Get DuiWindow state
DWORD SWindow::GetState(void)
{
    return m_dwState;
}

// Modify DuiWindow state
DWORD SWindow::ModifyState(DWORD dwStateAdd, DWORD dwStateRemove,BOOL bUpdate/*=FALSE*/)
{
    DWORD dwOldState = m_dwState;

    TestMainThread();

    m_dwState &= ~dwStateRemove;
    m_dwState |= dwStateAdd;

    OnStateChanged(dwOldState,m_dwState);
    if(bUpdate && NeedRedrawWhenStateChange()) NotifyInvalidateRect(m_rcWindow);
    return dwOldState;
}

// Get Command ID
UINT SWindow::GetCmdID()
{
    return m_uCmdID;
}

void SWindow::SetCmdID(UINT uNewID)
{
    m_uCmdID=uNewID;
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

BOOL SWindow::SetDuiTimer(char id,UINT uElapse)
{
    STimerID timerID(m_hSWnd,id);
    return ::SetTimer(GetContainer()->GetHostHwnd(),DWORD(timerID),uElapse,NULL);
}

void SWindow::KillDuiTimer(char id)
{
    STimerID timerID(m_hSWnd,id);
    ::KillTimer(GetContainer()->GetHostHwnd(),DWORD(timerID));
}


BOOL SWindow::SetDuiTimerEx( UINT_PTR id,UINT uElapse )
{
    return CDuiTimerEx::SetTimer(m_hSWnd,id,uElapse);
}

void SWindow::KillDuiTimerEx( UINT_PTR id )
{
    CDuiTimerEx::KillTimer(m_hSWnd,id);
}

HSWND SWindow::GetDuiHwnd()
{
    return m_hSWnd;
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
        ModifyState(0,DuiWndState_Invisible);
    else
        ModifyState(DuiWndState_Invisible,0);
}

BOOL SWindow::DestroyChild(SWindow *pChild)
{
    if(this != pChild->GetParent()) return FALSE;
    pChild->NotifyInvalidate();
    pChild->DuiSendMessage(WM_DESTROY);
    RemoveChild(pChild);
    pChild->Release();
    return TRUE;
}

UINT SWindow::GetChildrenCount()
{
    return m_nChildrenCount;
}

SWindow * SWindow::GetChild(UINT uCmdID)
{
    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        if(pChild->GetCmdID()==uCmdID) return pChild;
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
        DUIASSERT(pInsertAfter->m_pParent == this);
        DUIASSERT(m_pFirstChild && m_pLastChild);
        SWindow *pNext=pInsertAfter->m_pNextSibling;
        DUIASSERT(pNext);
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
    return DuiWndState_Check == (m_dwState & DuiWndState_Check);
}

BOOL SWindow::IsDisabled(BOOL bCheckParent /*= FALSE*/)
{
    if(bCheckParent) return m_dwState & DuiWndState_Disable;
    else return m_bDisable;
}

BOOL SWindow::IsVisible(BOOL bCheckParent /*= FALSE*/)
{
    if(bCheckParent) return (0 == (m_dwState & DuiWndState_Invisible));
    else return m_bVisible;
}

//因为NotifyInvalidateRect只有窗口可见时再通知刷新，这里在窗口可见状态改变前后都执行一次通知。
void SWindow::SetVisible(BOOL bVisible,BOOL bUpdate/*=FALSE*/)
{
    if(bUpdate) NotifyInvalidateRect(m_rcWindow);
    DuiSendMessage(WM_SHOWWINDOW,bVisible);
    if(bUpdate) NotifyInvalidateRect(m_rcWindow);
}

void SWindow::EnableWindow( BOOL bEnable,BOOL bUpdate)
{
    DuiSendMessage(WM_ENABLE,bEnable);
    if(bUpdate) NotifyInvalidateRect(m_rcWindow);
}

void SWindow::SetCheck(BOOL bCheck)
{
    if (bCheck)
        ModifyState(DuiWndState_Check, 0,TRUE);
    else
        ModifyState(0, DuiWndState_Check,TRUE);
}

BOOL SWindow::NeedRedrawParent()
{
    return (GetContainer()->IsTranslucent() || !m_pBgSkin || (m_style.m_crBg == CR_INVALID));
}


LPCTSTR SWindow::GetLinkUrl()
{
    return m_strLinkUrl;
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
DuiStyle& SWindow::GetStyle()
{
    return m_style;
}


SWindow* SWindow::FindChildByCmdID(UINT uCmdID)
{
    SWindow *pChild = m_pFirstChild;
    while(pChild)
    {
        if (pChild->GetCmdID() == uCmdID)
            return pChild;
        SWindow *pChildFind=pChild->FindChildByCmdID(uCmdID);
        if(pChildFind) return pChildFind;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

SWindow* SWindow::FindChildByName( LPCSTR pszName )
{
    if(!pszName) return NULL;

    SWindow *pChild = m_pFirstChild;
    while(pChild)
    {
        if (!pChild->m_strName.IsEmpty() && strcmp(pChild->m_strName, pszName)==0)
            return pChild;
        SWindow *pChildFind=pChild->FindChildByName(pszName);
        if(pChildFind) return pChildFind;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

BOOL SWindow::LoadChildren(pugi::xml_node xmlNode)
{
    for (pugi::xml_node xmlChild=xmlNode; xmlChild; xmlChild=xmlChild.next_sibling())
    {
        if(xmlChild.type() != pugi::node_element) continue;
        SWindow *pChild = DuiSystem::getSingleton().CreateWindowByName(xmlChild.name());
        if(!pChild)
        {//在窗口布局中支持include标签
            if(_stricmp(xmlChild.name(),"include")==0)
            {
                pugi::xml_document xmlDoc;
                CDuiStringT strName=DUI_CA2T(xmlChild.attribute("src").value(),CP_UTF8);
                if(LOADXML(xmlDoc,strName,RT_LAYOUT))
                {
                    LoadChildren(xmlDoc.first_child());
                }else
                {
                    DUIASSERT(FALSE);
                }

            }
            continue;
        }

        InsertChild(pChild);
        pChild->Load(xmlChild);
    }
    return TRUE;
}

// Create DuiWindow from xml element
BOOL SWindow::Load(pugi::xml_node xmlNode)
{
    DUIASSERT(m_pContainer);
    if (!xmlNode)
    {
        return FALSE;
    }

    m_strWndText = DUI_CA2T(xmlNode.text().get(), CP_UTF8);
    if (!m_strWndText.IsEmpty())
    {
        m_strWndText.TrimRight(0x0a).TrimLeft(0x0a);
        m_strWndText.TrimRight(0x0d).TrimLeft(0x0d);
        m_strWndText.TrimRight(0x20).TrimLeft(0x20);
        if (!m_strWndText.IsEmpty()) BUILDSTRING(m_strWndText);
    }

    m_dlgpos.nCount = 0;
    m_dlgpos.uPositionType = 0;
    SObject::Load(xmlNode);

    //加载style中指定的皮肤属性，由于皮肤有owner属性，而style没有owner属性，因此需要在属性加载完成后查询皮肤名称并加载 hjx:2012.1.15
    if(m_pBgSkin==NULL && !m_style.m_strSkinName.IsEmpty())
        m_pBgSkin=GETSKIN(m_style.m_strSkinName);
    if(m_pNcSkin==NULL && !m_style.m_strNcSkinName.IsEmpty())
        m_pNcSkin=GETSKIN(m_style.m_strNcSkinName);

    if(!m_bVisible || (m_pParent && !m_pParent->IsVisible(TRUE)))
        ModifyState(DuiWndState_Invisible, 0);

    if (4 != m_dlgpos.nCount)
    {
        CDuiStringA strValue = xmlNode.attribute("width").value();
        int nValue =atoi(strValue);

        if (0 == nValue && "full" == strValue && 0 == m_dlgpos.nCount)
        {
            m_rcWindow.right = 0;
            m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeX_Mask) | SizeX_FitParent;
        }
        else
        {
            if (nValue > 0)
            {
                m_rcWindow.right = nValue;
                m_dlgpos.uSpecifyWidth = nValue;
                m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeX_Mask) | SizeX_Specify;
            }
            else
            {
                m_rcWindow.right = 0;
                m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeX_Mask) | SizeX_FitContent;
            }
        }

        strValue = xmlNode.attribute("height").value();
        nValue =atoi(strValue);
        if (0 == nValue && "full" == strValue)
        {
            m_rcWindow.bottom = 0;
            m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeY_Mask) | SizeY_FitParent;
        }
        else
        {
            if (nValue > 0)
            {
                m_rcWindow.bottom = nValue;
                m_dlgpos.uSpecifyHeight = nValue;
                m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeY_Mask) | SizeY_Specify;
            }
            else
            {
                m_rcWindow.bottom = 0;
                m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeY_Mask) | SizeY_FitContent;
            }
        }

    }

    if(0!=DuiSendMessage(WM_CREATE))
    {
        if(m_pParent)    m_pParent->DestroyChild(this);
        return FALSE;
    }
    LoadChildren(xmlNode.first_child());
    return TRUE;
}

SWindow * SWindow::LoadXmlChildren(LPCSTR utf8Xml)
{
    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_buffer(utf8Xml,strlen(utf8Xml),pugi::parse_default,pugi::encoding_utf8)) return NULL;
    SWindow * pLastChild=m_pLastChild;//保存当前的最后一个子窗口
    BOOL bLoaded=LoadChildren(xmlDoc.first_child());
    if(!bLoaded) return NULL;

      CRect rcContainer=GetChildrenLayoutRect();
  
    SWindow *pNewChild=NULL;
    if(pLastChild) pNewChild=pLastChild->GetDuiWindow(GDUI_NEXTSIBLING);
    else pNewChild=m_pFirstChild;

      while(pNewChild)
      {
          pNewChild->DuiSendMessage(WM_WINDOWPOSCHANGED,0,(LPARAM)&rcContainer);
          pNewChild->DuiSendMessage(WM_SHOWWINDOW,IsVisible(TRUE),ParentShow);
        pNewChild->NotifyInvalidate();
          pNewChild=pNewChild->GetDuiWindow(GDUI_NEXTSIBLING);
      }
    return m_pLastChild;
}

// Hittest children
HSWND SWindow::DuiGetHWNDFromPoint(CPoint ptHitTest, BOOL bOnlyText)
{
    if (!m_rcWindow.PtInRect(ptHitTest)) return NULL;

    CRect rcClient;
    GetClient(&rcClient);

    if(!rcClient.PtInRect(ptHitTest))
        return m_hSWnd;    //只在鼠标位于客户区时，才继续搜索子窗口
    
    HSWND hDuiWndChild = NULL;

    SWindow *pChild=m_pLastChild;
    while(pChild)
    {
        if (pChild->IsVisible(TRUE) && !pChild->IsMsgTransparent())
        {
            hDuiWndChild = pChild->DuiGetHWNDFromPoint(ptHitTest, bOnlyText);

            if (hDuiWndChild) return hDuiWndChild;
        }

        pChild=pChild->m_pPrevSibling;
    }

    return m_hSWnd;
}

BOOL SWindow::NeedRedrawWhenStateChange()
{
    if (m_pBgSkin && !m_pBgSkin->IgnoreState())
    {
        return TRUE;
    }
    return m_style.GetStates()>1;
}

BOOL SWindow::RedrawRegion(IRenderTarget *pRT, IRegion *pRgn)
{
    PRSTATE prState=PRS_LOOKSTART;
    return _PaintRegion(pRT,pRgn,this,this,NULL,prState);
}

void SWindow::OnAttributeChanged( const CDuiStringA & strAttrName,BOOL bLoading,HRESULT hRet )
{
    if(!bLoading && hRet==S_OK)
    {
        if(strAttrName=="pos")
        {
            //位置改变时需要重新计算位置，并更新
            NotifyInvalidate();
            if(GetParent())
            {
                OnWindowPosChanged(NULL);
                NotifyInvalidate();
            }
        }
        else
        {
            NotifyInvalidate();
        }
    }
}

void SWindow::NotifyInvalidate()
{
    CRect rcClient;
    GetClient(&rcClient);
    NotifyInvalidateRect(rcClient);
}

void SWindow::NotifyInvalidateRect(LPRECT lprect)
{
    if (lprect)
    {
        CRect rect = *lprect;
        NotifyInvalidateRect(rect);
    }
}

void SWindow::NotifyInvalidateRect(const CRect& rect)
{
    if(!IsVisible(TRUE)) return ;
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

LRESULT SWindow::DuiNotify(LPSNMHDR pnms)
{
    EventArgs args(pnms,this);
    FireEvent(pnms->code,args);
    if(args.handled != 0) return 0;

    if(GetOwner()) return GetOwner()->DuiNotify(pnms);
    return GetContainer()->OnDuiNotify(pnms);
}

LRESULT SWindow::OnWindowPosChanged(LPRECT lpRcContainer)
{
    LRESULT lRet=0;
    if(!(m_dlgpos.uPositionType & Pos_Float))    
    {//窗口不是使用Move直接指定的坐标,计算出窗口位置
        lRet=CDuiLayout::CalcPosition(this,lpRcContainer,m_dlgpos,m_rcWindow);
    }
    if(lRet==0)
    {
        DuiSendMessage(WM_NCCALCSIZE);//计算非客户区大小

        CRect rcClient;
        GetClient(&rcClient);
        DuiSendMessage(WM_SIZE,0,MAKELPARAM(rcClient.Width(),rcClient.Height()));

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
    //destroy children dui windows
    SWindow *pChild=m_pFirstChild;
    while (pChild)
    {
        SWindow *pNextChild=pChild->m_pNextSibling;
        pChild->DuiSendMessage(WM_DESTROY);
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
    GetClient(&rcClient);
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
        
        if(GetState()&DuiWndState_Disable)
        {
            nState=3;
        }
        else if(GetState()&DuiWndState_Check || GetState()&DuiWndState_PushDown)
        {
            nState=2;
        }else if(GetState()&DuiWndState_Hover)
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
    IFontPtr pFont = m_style.m_ftText[IIF_STATE4(m_dwState,0,1,2,3)];
    if(pFont) 
        pRT->SelectObject(pFont,(IRenderObj**)&painter.pOldPen);
    
    COLORREF crTxt = m_style.m_crText[IIF_STATE4(m_dwState,0,1,2,3)];
    if(crTxt != CR_INVALID)
        painter.crOld = pRT->SetTextColor(crTxt);
}

void SWindow::BeforePaintEx(IRenderTarget *pRT)
{
    SWindow *pParent=GetParent();
    if(pParent) pParent->BeforePaintEx(pRT);
    SPainter painter;
    BeforePaint(pRT,painter);
    //todo:hjx
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
    DuiDrawText(pRT,m_strWndText, m_strWndText.GetLength(), rcText, GetTextAlign());

    //draw focus rect
    if(GetContainer()->GetDuiFocus()==m_hSWnd)
    {
        DuiDrawFocus(pRT);
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
        SWindow::GetClient(&rcClient);
        pRT->PushClipRect(&rcClient,RGN_DIFF);

        if(bGetRT) PaintBackground(pRT,&m_rcWindow);

        int nState=0;
        if(DuiWndState_Hover & m_dwState) nState=1;
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
        pRT->PopClipRect();
        if(bGetRT) ReleaseRenderTarget(pRT);
    }
}

CSize SWindow::CalcSize(LPRECT pRcContainer)
{
    CSize sz;
    if(m_dlgpos.uPositionType & SizeX_Specify)
        sz.cx=m_dlgpos.uSpecifyWidth;
    else if(m_dlgpos.uPositionType & SizeX_FitParent)
        sz.cx=pRcContainer->right-pRcContainer->left;
    if(m_dlgpos.uPositionType & SizeY_Specify)
        sz.cy=m_dlgpos.uSpecifyHeight;
    else if(m_dlgpos.uPositionType & SizeY_FitParent)
        sz.cy=pRcContainer->bottom-pRcContainer->top;
    if((m_dlgpos.uPositionType & SizeX_FitContent) || (m_dlgpos.uPositionType & SizeY_FitContent) && m_dlgpos.nCount!=4)
    {
        CSize szDesire=GetDesiredSize(pRcContainer);    
        if(m_dlgpos.uPositionType & SizeX_FitContent)
            sz.cx=szDesire.cx;
        if(m_dlgpos.uPositionType & SizeY_FitContent)
            sz.cy=szDesire.cy;
    }
    return sz;
}


CSize SWindow::GetDesiredSize(LPRECT pRcContainer)
{
    DUIASSERT((m_dlgpos.uPositionType & SizeX_FitContent) || (m_dlgpos.uPositionType & SizeY_FitContent));


    int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

    CRect rcTest (0,0,0x7FFF,0x7FFF);
    if(m_nMaxWidth!=-1)
    {
        rcTest.right=m_nMaxWidth;
        nTestDrawMode|=DT_WORDBREAK;
    }
    
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,1,1);
    BeforePaintEx(pRT);
    DuiDrawText(pRT,m_strWndText, m_strWndText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);

    return rcTest.Size();
}

void SWindow::GetTextRect( LPRECT pRect )
{
    GetClient(pRect);
}

void SWindow::DuiDrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    pRT->DrawText(pszBuf,cchText,pRect,uFormat,m_byAlpha);
}

void SWindow::DuiDrawFocus(IRenderTarget *pRT)
{
    CRect rcFocus;
    GetTextRect(&rcFocus);
    if(IsTabStop())    DuiDrawDefFocusRect(pRT,rcFocus);
}


void SWindow::DuiDrawDefFocusRect(IRenderTarget *pRT,CRect rcFocus )
{
    rcFocus.DeflateRect(2,2);
    CAutoRefPtr<IPen> pPen,oldPen;
    pRT->CreatePen(PS_DOT,RGBA(88,88,88,m_byAlpha),1,&pPen);
    pRT->SelectObject(pPen,(IRenderObj**)&oldPen);
    pRT->DrawRectangle(&rcFocus);    
    pRT->SelectObject(oldPen);
}

UINT SWindow::OnGetDuiCode()
{
    return 0;
}

BOOL SWindow::IsTabStop()
{
    return m_bTabStop;
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
        ModifyState(0, DuiWndState_Invisible);
    else
        ModifyState(DuiWndState_Invisible, 0);

    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->DuiSendMessage(WM_SHOWWINDOW,bShow,ParentShow);
        pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
    }
    if(!IsVisible(TRUE) && m_hSWnd == GetContainer()->GetDuiFocus())
    {
        GetContainer()->OnSetDuiFocus(NULL);
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
        ModifyState(0, DuiWndState_Disable);
    else
        ModifyState(DuiWndState_Disable, DuiWndState_Hover);

    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->DuiSendMessage(WM_ENABLE,bEnable,ParentEnable);
        pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
    }
    if(IsDisabled(TRUE) && m_hSWnd == GetContainer()->GetDuiFocus())
    {
        GetContainer()->OnSetDuiFocus(NULL);
    }
}

void SWindow::OnLButtonDown(UINT nFlags,CPoint pt)
{
    if(m_bTabStop) SetDuiFocus();
    SetDuiCapture();
    ModifyState(DuiWndState_PushDown, 0,TRUE);
}

void SWindow::OnLButtonUp(UINT nFlags,CPoint pt)
{
    ReleaseDuiCapture();
    if(!m_rcWindow.PtInRect(pt)) return;

    ModifyState(0, DuiWndState_PushDown,TRUE);

    LPCTSTR lpszUrl = GetLinkUrl();
    if (lpszUrl && lpszUrl[0])
    {
        ::ShellExecute(NULL, _T("open"), lpszUrl, NULL, NULL, SW_SHOWNORMAL);
    }
    else if (GetCmdID() || GetName())
    {
        NotifyCommand();
    }
}

void SWindow::OnRButtonDown( UINT nFlags, CPoint point )
{
    NotifyContextMenu(point);
}

void SWindow::OnMouseHover(WPARAM wParam, CPoint ptPos)
{
    if(GetDuiCapture()==m_hSWnd)
        ModifyState(DuiWndState_PushDown,0,FALSE);
    ModifyState(DuiWndState_Hover, 0,TRUE);
    OnNcPaint(0);
}

void SWindow::OnMouseLeave()
{
    if(GetDuiCapture()==m_hSWnd)
        ModifyState(0,DuiWndState_PushDown,FALSE);
    ModifyState(0,DuiWndState_Hover,TRUE);
    OnNcPaint(0);
}

BOOL SWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL bRet=FALSE;
    if(m_pParent) bRet=(BOOL)m_pParent->DuiSendMessage(WM_MOUSEWHEEL,MAKEWPARAM(nFlags,zDelta),MAKELPARAM(pt.x,pt.y));
    return bRet;
}

HRESULT SWindow::OnAttributeState( const CDuiStringA& strValue, BOOL bLoading )
{
    int nState=0;
    ::StrToIntExA(strValue,STIF_SUPPORT_HEX,&nState);
    m_dwState=nState;
    if(m_dwState & DuiWndState_Invisible) m_bVisible=FALSE;
    if(m_dwState & DuiWndState_Disable) m_bDisable=TRUE;
    return S_FALSE;
}


HRESULT SWindow::OnAttributePosition(const CDuiStringA& strValue, BOOL bLoading)
{
    if (strValue.IsEmpty()) return E_FAIL;

    ClearLayoutState();
    CDuiLayout::StrPos2DuiWndPos(strValue,m_dlgpos);
    return bLoading?S_FALSE:S_OK;
}


CRect SWindow::GetChildrenLayoutRect()
{
    CRect rcRet;
    GetClient(rcRet);//通常是非客户区，但是tab这样的控件不一样。
    return rcRet;
}


void SWindow::ClearLayoutState()
{
    if(m_dlgpos.uPositionType & Pos_Float) return;

    m_rcWindow.left=m_rcWindow.top=m_rcWindow.right=m_rcWindow.bottom=POS_INIT;
}

void SWindow::UpdateChildrenPosition()
{
    CDuiList<SWindow*> lstWnd;
    SWindow *pChild=GetDuiWindow(GDUI_FIRSTCHILD);
    while(pChild)
    {
        pChild->ClearLayoutState();
        lstWnd.AddTail(pChild);
        pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
    }
    CDuiLayout::CalcChildrenPosition(this,&lstWnd);
    NotifyInvalidate();
}

void SWindow::OnSetDuiFocus()
{
    NotifyInvalidateRect(m_rcWindow);
}

void SWindow::OnKillDuiFocus()
{
    NotifyInvalidateRect(m_rcWindow);
}

IRenderTarget * SWindow::GetRenderTarget(const LPRECT pRc/*=NULL*/,DWORD gdcFlags/*=0*/,BOOL bClientDC/*=TRUE*/)
{
    DUIASSERT(m_gdcFlags==-1);
    if(bClientDC)
        GetClient(&m_rcGetRT);
    else
        m_rcGetRT=m_rcWindow;

    if(gdcFlags!=OLEDC_NODRAW)
    {//将DC限制在父窗口的可见区域
        SWindow *pParent=GetParent();
        while(pParent)
        {
            CRect rcParent;
            pParent->GetClient(&rcParent);
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
    IRenderTarget *pRT=GetContainer()->OnGetRenderTarget(m_rcGetRT,gdcFlags);
    if(m_bClipRT)
    {
        pRT->PushClipRect(&m_rcGetRT,RGN_COPY);
    }
    if(gdcFlags&OLEDC_PAINTBKGND)
        PaintBackground(pRT,&m_rcGetRT);

    return pRT;
}

void SWindow::ReleaseRenderTarget(IRenderTarget *pRT)
{
    if(m_gdcFlags & OLEDC_PAINTBKGND) //画了背景，自动画前景
        PaintForeground(pRT,&m_rcGetRT);
    if(m_bClipRT)
        pRT->PopClipRect();
    GetContainer()->OnReleaseRenderTarget(pRT,m_rcGetRT,m_gdcFlags);
    m_bClipRT=FALSE;
    m_gdcFlags=-1;
}

HSWND SWindow::GetDuiCapture()
{
    return GetContainer()->OnGetDuiCapture();
}

HSWND SWindow::SetDuiCapture()
{
    return GetContainer()->OnSetDuiCapture(m_hSWnd);
}

BOOL SWindow::ReleaseDuiCapture()
{
    return GetContainer()->OnReleaseDuiCapture();
}

void SWindow::SetDuiFocus()
{
    GetContainer()->OnSetDuiFocus(m_hSWnd);
}

void SWindow::KillDuiFocus()
{
    if(GetContainer()->GetDuiFocus()==m_hSWnd)
    {
        GetContainer()->OnSetDuiFocus(NULL);
    }
}

SWindow *SWindow::GetCheckedRadioButton()
{
    SWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        if(pChild->IsClass("radio") && pChild->IsChecked())
        {
            return pChild;
        }
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

void SWindow::CheckRadioButton(SWindow * pRadioBox)
{
    SWindow *pChecked=GetCheckedRadioButton();
    if(pChecked == pRadioBox) return;
    if(pChecked)
    {
        pChecked->ModifyState(0,DuiWndState_Check,TRUE);
    }
    pRadioBox->ModifyState(DuiWndState_Check,0,TRUE);
}


BOOL SWindow::SetItemVisible(UINT uItemID, BOOL bVisible)
{
    SWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
    {
        pWnd->SetVisible(bVisible,TRUE);
        return TRUE;
    }

    return FALSE;
}

BOOL SWindow::IsItemVisible(UINT uItemID, BOOL bCheckParent /*= FALSE*/)
{
    SWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
        return pWnd->IsVisible(bCheckParent);

    return FALSE;
}

BOOL SWindow::GetItemCheck(UINT uItemID)
{
    SWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
        return pWnd->IsChecked();

    return FALSE;
}

BOOL SWindow::SetItemCheck(UINT uItemID, BOOL bCheck)
{
    SWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
    {
        if (bCheck)
            pWnd->ModifyState(DuiWndState_Check, 0);
        else
            pWnd->ModifyState(0, DuiWndState_Check);

        pWnd->NotifyInvalidateRect(pWnd->m_rcWindow);

        return TRUE;
    }

    return FALSE;
}

BOOL SWindow::EnableItem(UINT uItemID, BOOL bEnable)
{
    SWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
    {
        if (bEnable)
            pWnd->ModifyState(0, DuiWndState_Disable);
        else
            pWnd->ModifyState(DuiWndState_Disable, DuiWndState_Hover);

        pWnd->NotifyInvalidateRect(pWnd->m_rcWindow);
        return TRUE;
    }

    return FALSE;
}

BOOL SWindow::IsItemEnable(UINT uItemID, BOOL bCheckParent /*= FALSE*/)
{
    SWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
        return !pWnd->IsDisabled(bCheckParent);

    return FALSE;
}

BOOL SWindow::OnDuiNcHitTest(CPoint pt)
{
    return FALSE;
}

BOOL SWindow::IsMsgHandled() const
{
    return m_bMsgHandled;
}

void SWindow::SetMsgHandled(BOOL bHandled)
{
    m_bMsgHandled = bHandled;
}


SWindow * SWindow::GetDuiWindow(int uCode)
{
    SWindow *pRet=NULL;
    switch(uCode)
    {
    case GDUI_FIRSTCHILD:
        pRet=m_pFirstChild;
        break;
    case GDUI_LASTCHILD:
        pRet=m_pLastChild;
        break;
    case GDUI_PREVSIBLING:
        pRet=m_pPrevSibling;
        break;
    case GDUI_NEXTSIBLING:
        pRet=m_pNextSibling;
        break;
    case GDUI_OWNER:
        pRet=m_pOwner;
        break;
    case GDUI_PARENT:
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
    pRT->PopClipRect();
}

BOOL SWindow::_PaintRegion( IRenderTarget *pRT, IRegion *pRgn,SWindow *pWndCur,SWindow *pStart,SWindow *pEnd,SWindow::PRSTATE & prState )
{
    if (!pWndCur->IsVisible(TRUE) || (!pRgn->IsEmpty() && !pRgn->RectInRegion(&pWndCur->m_rcWindow)))
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
    pWndCur->GetClient(&rcClient);
    if(pRgn->IsEmpty() || pRgn->RectInRegion(rcClient))
    {//重绘客户区
        CRgn rgnOldClip;
        if(prsBack == PRS_DRAWING)
        {
            if(pWndCur->IsClipClient())
            {
                pRT->PushClipRect(&rcClient);
            }
            pWndCur->DuiSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
            pWndCur->DuiSendMessage(WM_PAINT, (WPARAM)pRT);
        }

        SPainter DuiDC;

//        pWndCur->BeforePaint(dc, DuiDC);    //让子窗口继承父窗口的属性

        SWindow *pChild=pWndCur->GetDuiWindow(GDUI_FIRSTCHILD);
        while(pChild)
        {
            if(pChild==pEnd) break;
            _PaintRegion(pRT,pRgn,pChild,pStart,pEnd,prState);
            if(prState == PRS_MEETEND)
                break;
            pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
        }

//        pWndCur->AfterPaint(dc, DuiDC);
        if(prsBack == PRS_DRAWING && pWndCur->IsClipClient())
        {
            pRT->PopClipRect();
        }
    }
    if(prsBack == PRS_DRAWING) 
        pWndCur->DuiSendMessage(WM_NCPAINT, (WPARAM)pRT);//ncpaint should be placed in tail of paint link

    return prState==PRS_DRAWING || prState == PRS_MEETEND;
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
        pRT->PopClipRect();
    }
}


SWindow * SWindow::GetNextVisibleWindow( SWindow *pWnd ,const CRect &rcDraw)
{
    if(!pWnd) return NULL;
    SWindow *pNextSibling=pWnd->GetDuiWindow(GDUI_NEXTSIBLING);
    if(pNextSibling && pNextSibling->IsVisible(TRUE) && !(pNextSibling->m_rcWindow & rcDraw).IsRectEmpty())
        return pNextSibling;
    else if (pNextSibling)    return GetNextVisibleWindow(pNextSibling,rcDraw);
    else return GetNextVisibleWindow(pWnd->GetParent(),rcDraw);
}

void SWindow::DrawAniStep( CRect rcFore,CRect rcBack,IRenderTarget * pRTFore,IRenderTarget * pRTBack,CPoint ptAnchor)
{
    IRenderTarget * pRT=GetRenderTarget(rcBack,OLEDC_OFFSCREEN,FALSE);
    pRT->BitBlt(&rcBack,pRTBack,rcBack.left,rcBack.top,SRCCOPY);
    pRT->BitBlt(&rcFore,pRTFore,ptAnchor.x,ptAnchor.y,SRCCOPY);
    PaintForeground(pRT,rcBack);//画前景
    ReleaseRenderTarget(pRT);
}

void SWindow::DrawAniStep( CRect rcWnd,IRenderTarget * pRTFore,IRenderTarget * pRTBack,BYTE byAlpha)
{
    IRenderTarget * pRT=GetRenderTarget(rcWnd,OLEDC_OFFSCREEN,FALSE);
    if(byAlpha>0 && byAlpha<255)
    {
        pRT->BitBlt(&rcWnd,pRTBack,rcWnd.left,rcWnd.top,SRCCOPY);
        IBitmap *pBmp = (IBitmap*)pRTFore->GetCurrentObject(OT_BITMAP);
        pRT->DrawBitmap(&rcWnd,pBmp,rcWnd.left,rcWnd.top,byAlpha);
    }else if(byAlpha==0)
    {
        pRT->BitBlt(&rcWnd,pRTBack,rcWnd.left,rcWnd.top,SRCCOPY);
    }else if(byAlpha==255)
    {
        pRT->BitBlt(&rcWnd,pRTFore,rcWnd.left,rcWnd.top,SRCCOPY);
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
    GetRect(&rcWnd);

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
    pRTBefore->BitBlt(&rcWnd,pRT,rcWnd.left,rcWnd.top,SRCCOPY);
    
    //更新窗口可见性
    SetVisible(!(dwFlags&AW_HIDE),FALSE);
    //窗口变化后
    CAutoRefPtr<IRenderTarget> pRTAfter;
    GETRENDERFACTORY->CreateRenderTarget(&pRTAfter,rcWnd.Width(),rcWnd.Height());
    pRTAfter->OffsetViewportOrg(-rcWnd.left,-rcWnd.top);

    PaintBackground(pRT,rcWnd);
    RedrawRegion(pRT,rgn);
    pRTAfter->BitBlt(&rcWnd,pRT,rcWnd.left,rcWnd.top,SRCCOPY);

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
                DrawAniStep(rcNewState,rcWnd,pRTBefore,pRTAfter,ptAnchor);
                Sleep(10);
            }
            DrawAniStep(rcWnd,rcWnd,pRTBefore,pRTAfter,rcWnd.TopLeft());
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
                DrawAniStep(rcNewState,rcWnd,pRTBefore,pRTAfter,rcNewState.TopLeft());
                Sleep(10);
            }
            DrawAniStep(rcWnd,rcWnd,pRTBefore,pRTAfter,rcWnd.TopLeft());
            return TRUE;
        }else if(dwFlags&AW_BLEND)
        {
            BYTE byAlpha=0;
            BYTE byStepLen=255/nSteps;
            for(int i=0;i<nSteps;i++)
            {
                DrawAniStep(rcWnd,pRTBefore,pRTAfter,byAlpha);
                Sleep(10);
                byAlpha+=byStepLen;
            }
            DrawAniStep(rcWnd,pRTBefore,pRTAfter,255);
            return TRUE;
        }
        return FALSE;
    }
}

LRESULT SWindow::NotifyCommand()
{
    DUINMCOMMAND nms;
    nms.hdr.hDuiWnd=m_hSWnd;
    nms.hdr.code = NM_COMMAND;
    nms.hdr.idFrom = GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.uItemData = GetUserData();
    return DuiNotify((LPSNMHDR)&nms);
}

LRESULT SWindow::NotifyContextMenu( CPoint pt )
{
    DUINMCONTEXTMENU nms;
    nms.hdr.hDuiWnd=m_hSWnd;
    nms.hdr.code = NM_CONTEXTMENU;
    nms.hdr.idFrom = GetCmdID();
    nms.hdr.pszNameFrom=GetName();
    nms.uItemData = GetUserData();
    nms.pt=pt;
    return DuiNotify((LPSNMHDR)&nms);
}

}//namespace SOUI
