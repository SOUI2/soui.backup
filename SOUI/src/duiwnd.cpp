//////////////////////////////////////////////////////////////////////////
//   File Name: DuiWnd.h
// Description: DuiWindow Definition
//     Creator: Zhang Xiaoxuan
//     Version: 2009.04.28 - 1.0 - Create
//				2011.09.01 - 2.0 huang jianxiong
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "duiwnd.h"
#include "MemDC.h"

namespace SOUI
{

//////////////////////////////////////////////////////////////////////////
// CDuiWindow Implement
//////////////////////////////////////////////////////////////////////////

CDuiWindow::CDuiWindow()
    : m_hDuiWnd(DuiWindowMgr::NewWindow(this))
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
    , m_nSaveDC(0)
    , m_gdcFlags(-1)
	, m_byAlpha(0xFF)
#ifdef _DEBUG
    , m_nMainThreadId( ::GetCurrentThreadId() ) // 初始化对象的线程不一定是主线程
#endif
{
	ClearLayoutState();
	addEvent(NM_COMMAND);
}

CDuiWindow::~CDuiWindow()
{
    DuiWindowMgr::DestroyWindow(m_hDuiWnd);
}

void CDuiWindow::OnFinalRelease()
{
    delete this;
}


//////////////////////////////////////////////////////////////////////////
// Method Define

// Get align
UINT CDuiWindow::GetTextAlign()
{
    return m_style.GetTextAlign() ;
}

// Get position type
DWORD CDuiWindow::GetPositionType()
{
    return m_dlgpos.uPositionType;
}

// Set position type
void CDuiWindow::SetPositionType(DWORD dwPosType, DWORD dwMask /*= 0xFFFFFFFF*/)
{
    m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~dwMask) | (dwPosType & dwMask);
}

void CDuiWindow::SetFixSize(int nWid,int nHei)
{
    m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~SizeX_Mask) | SizeX_Specify|SizeY_Specify;
    m_dlgpos.uSpecifyWidth = nWid;
    m_dlgpos.uSpecifyHeight = nHei;
}

void CDuiWindow::SetBkColor(COLORREF cr)
{
    m_style.m_crBg=cr;
}

// Get DuiWindow rect(position in container)
void CDuiWindow::GetRect(LPRECT prect)
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

void CDuiWindow::GetClient(LPRECT pRect)
{
    DUIASSERT(pRect);
    *pRect=m_rcWindow;
    pRect->left+=m_style.m_nMarginX;
    pRect->right-=m_style.m_nMarginX;
    pRect->top+=m_style.m_nMarginY;
    pRect->bottom-=m_style.m_nMarginY;
}

void CDuiWindow::GetDlgPosition(DUIWND_POSITION *pPos)
{
    if (pPos)
        memcpy(pPos, &m_dlgpos, sizeof(DUIWND_POSITION));
}

// Get inner text
LPCTSTR CDuiWindow::GetInnerText()
{
    return m_strInnerText;
}

// Get tooltip text
BOOL CDuiWindow::OnUpdateToolTip(HDUIWND hCurTipHost,HDUIWND &hNewTipHost,CRect &rcTip,CDuiStringT &strTip)
{
    if(m_hDuiWnd==hCurTipHost) return FALSE;
    hNewTipHost=m_hDuiWnd;
    GetRect(&rcTip);
    strTip=m_strToolTipText;
    return TRUE;
}

// Set inner text
HRESULT CDuiWindow::SetInnerText(LPCTSTR lpszText)
{
    m_strInnerText = lpszText;
	if(IsVisible(TRUE)) NotifyInvalidate();
	if ((m_dlgpos.uPositionType & (SizeX_FitContent | SizeY_FitContent)) && (4 != m_dlgpos.nCount))
	{
		OnWindowPosChanged(NULL);
		if(IsVisible(TRUE)) NotifyInvalidate();
	}
	return S_OK;
}

VOID CDuiWindow::TestMainThread()
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
LRESULT CDuiWindow::DuiSendMessage(UINT Msg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/,BOOL *pbMsgHandled/*=NULL*/)
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
void CDuiWindow::Move(LPRECT prect)
{
    DUIASSERT(prect);
    TestMainThread();

    if(m_rcWindow.EqualRect(prect)) return;

    m_rcWindow = prect;
	m_dlgpos.uPositionType = (m_dlgpos.uPositionType & ~Position_Mask)|Pos_Float;//使用Move后，程序不再自动计算窗口坐标
	OnWindowPosChanged(NULL);
}

void CDuiWindow::Move(int x,int y, int cx/*=-1*/,int cy/*=-1*/)
{
    if(cx==-1) cx=m_rcWindow.Width();
    if(cy==-1) cy=m_rcWindow.Height();
    CRect rcNew(x,y,x+cx,y+cy);
    Move(&rcNew);
}

// Set current cursor, when hover
BOOL CDuiWindow::OnDuiSetCursor(const CPoint &pt)
{
    HCURSOR hCur = ::LoadCursor(NULL, m_style.m_lpCursorName);
    ::SetCursor(hCur);
    return TRUE;
}

// Get DuiWindow state
DWORD CDuiWindow::GetState(void)
{
    return m_dwState;
}

// Modify DuiWindow state
DWORD CDuiWindow::ModifyState(DWORD dwStateAdd, DWORD dwStateRemove,BOOL bUpdate/*=FALSE*/)
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
UINT CDuiWindow::GetCmdID()
{
    return m_uCmdID;
}

void CDuiWindow::SetCmdID(UINT uNewID)
{
    m_uCmdID=uNewID;
}

ULONG_PTR CDuiWindow::GetUserData()
{
    return m_uData;
}

ULONG_PTR CDuiWindow::SetUserData(ULONG_PTR uData)
{
    ULONG_PTR uOld=m_uData;
    m_uData=uData;
    return uOld;
}

BOOL CDuiWindow::SetDuiTimer(char id,UINT uElapse)
{
    CDuiTimerID timerID(m_hDuiWnd,id);
    return ::SetTimer(GetContainer()->GetHostHwnd(),DWORD(timerID),uElapse,NULL);
}

void CDuiWindow::KillDuiTimer(char id)
{
    CDuiTimerID timerID(m_hDuiWnd,id);
    ::KillTimer(GetContainer()->GetHostHwnd(),DWORD(timerID));
}


BOOL CDuiWindow::SetDuiTimerEx( UINT_PTR id,UINT uElapse )
{
    return CDuiTimerEx::SetTimer(m_hDuiWnd,id,uElapse);
}

void CDuiWindow::KillDuiTimerEx( UINT_PTR id )
{
    CDuiTimerEx::KillTimer(m_hDuiWnd,id);
}

HDUIWND CDuiWindow::GetDuiHwnd()
{
    return m_hDuiWnd;
}


CDuiWindow *CDuiWindow::GetParent()
{
    return m_pParent;
}


CDuiWindow * CDuiWindow::GetTopLevelParent()
{
    CDuiWindow *pParent=this;
    while(pParent->GetParent()) pParent=pParent->GetParent();
    return pParent;
}

void CDuiWindow::SetParent(CDuiWindow *pParent)
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

BOOL CDuiWindow::DestroyChild(CDuiWindow *pChild)
{
    if(this != pChild->GetParent()) return FALSE;
	pChild->NotifyInvalidate();
    pChild->DuiSendMessage(WM_DESTROY);
    RemoveChild(pChild);
    pChild->Release();
    return TRUE;
}

UINT CDuiWindow::GetChildrenCount()
{
    return m_nChildrenCount;
}

CDuiWindow * CDuiWindow::GetChild(UINT uCmdID)
{
    CDuiWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        if(pChild->GetCmdID()==uCmdID) return pChild;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

void CDuiWindow::SetChildContainer(CDuiWindow *pChild)
{
    pChild->SetContainer(GetContainer());
}

void CDuiWindow::InsertChild(CDuiWindow *pNewChild,CDuiWindow *pInsertAfter/*=ICWND_LAST*/)
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
        CDuiWindow *pNext=pInsertAfter->m_pNextSibling;
        DUIASSERT(pNext);
        pInsertAfter->m_pNextSibling=pNewChild;
        pNewChild->m_pPrevSibling=pInsertAfter;
        pNewChild->m_pNextSibling=pNext;
        pNext->m_pPrevSibling=pNewChild;
    }
    m_nChildrenCount++;
}

BOOL CDuiWindow::RemoveChild(CDuiWindow *pChild)
{
    if(this != pChild->GetParent()) return FALSE;
    CDuiWindow *pPrevSib=pChild->m_pPrevSibling;
    CDuiWindow *pNextSib=pChild->m_pNextSibling;
    if(pPrevSib) pPrevSib->m_pNextSibling=pNextSib;
    else m_pFirstChild=pNextSib;
    if(pNextSib) pNextSib->m_pPrevSibling=pPrevSib;
    else m_pLastChild=pPrevSib;
	pChild->m_pParent=NULL;
    m_nChildrenCount--;
    return TRUE;
}

BOOL CDuiWindow::IsChecked()
{
    return DuiWndState_Check == (m_dwState & DuiWndState_Check);
}

BOOL CDuiWindow::IsDisabled(BOOL bCheckParent /*= FALSE*/)
{
	if(bCheckParent) return m_dwState & DuiWndState_Disable;
	else return m_bDisable;
}

BOOL CDuiWindow::IsVisible(BOOL bCheckParent /*= FALSE*/)
{
    if(bCheckParent) return (0 == (m_dwState & DuiWndState_Invisible));
    else return m_bVisible;
}

//因为NotifyInvalidateRect只有窗口可见时再通知刷新，这里在窗口可见状态改变前后都执行一次通知。
void CDuiWindow::SetVisible(BOOL bVisible,BOOL bUpdate/*=FALSE*/)
{
	if(bUpdate) NotifyInvalidateRect(m_rcWindow);
    DuiSendMessage(WM_SHOWWINDOW,bVisible);
    if(bUpdate) NotifyInvalidateRect(m_rcWindow);
}

void CDuiWindow::EnableWindow( BOOL bEnable,BOOL bUpdate)
{
	DuiSendMessage(WM_ENABLE,bEnable);
	if(bUpdate) NotifyInvalidateRect(m_rcWindow);
}

void CDuiWindow::SetCheck(BOOL bCheck)
{
    if (bCheck)
        ModifyState(DuiWndState_Check, 0,TRUE);
    else
        ModifyState(0, DuiWndState_Check,TRUE);
}

BOOL CDuiWindow::NeedRedrawParent()
{
    return (GetContainer()->IsTranslucent() || !m_pBgSkin || (m_style.m_crBg == CLR_INVALID));
}


LPCTSTR CDuiWindow::GetLinkUrl()
{
    return m_strLinkUrl;
}

IDuiContainer *CDuiWindow::GetContainer()
{
    return m_pContainer;
}

void CDuiWindow::SetContainer(IDuiContainer *pContainer)
{
    m_pContainer=pContainer;
    CDuiWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->SetContainer(pContainer);
        pChild=pChild->m_pNextSibling;
    }
}

void CDuiWindow::SetOwner(CDuiWindow *pOwner)
{
    m_pOwner=pOwner;
}

CDuiWindow *CDuiWindow::GetOwner()
{
    return m_pOwner;
}

BOOL CDuiWindow::IsMsgTransparent()
{
    return m_bMsgTransparent;
}

// add by dummyz@126.com
DuiStyle& CDuiWindow::GetStyle()
{
    return m_style;
}


CDuiWindow* CDuiWindow::FindChildByCmdID(UINT uCmdID)
{
    CDuiWindow *pChild = m_pFirstChild;
    while(pChild)
    {
        if (pChild->GetCmdID() == uCmdID)
            return pChild;
        CDuiWindow *pChildFind=pChild->FindChildByCmdID(uCmdID);
        if(pChildFind) return pChildFind;
        pChild=pChild->m_pNextSibling;
    }
    return NULL;
}

CDuiWindow* CDuiWindow::FindChildByName( LPCSTR pszName )
{
	if(!pszName) return NULL;

	CDuiWindow *pChild = m_pFirstChild;
	while(pChild)
	{
		if (!pChild->m_strName.IsEmpty() && strcmp(pChild->m_strName, pszName)==0)
			return pChild;
		CDuiWindow *pChildFind=pChild->FindChildByName(pszName);
		if(pChildFind) return pChildFind;
		pChild=pChild->m_pNextSibling;
	}
	return NULL;
}

BOOL CDuiWindow::LoadChildren(pugi::xml_node xmlNode)
{
	for (pugi::xml_node xmlChild=xmlNode; xmlChild; xmlChild=xmlChild.next_sibling())
    {
		if(xmlChild.type() != pugi::node_element) continue;
        CDuiWindow *pChild = DuiSystem::getSingleton().CreateWindowByName(xmlChild.name());
		if(!pChild)
		{//在窗口布局中支持include标签
			if(stricmp(xmlChild.name(),"include")==0)
			{
				pugi::xml_document xmlDoc;
				CDuiStringT strName=DUI_CA2T(xmlChild.attribute("src").value(),CP_UTF8);
				if(LOADXML(xmlDoc,strName,DUIRES_XML_TYPE))
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
BOOL CDuiWindow::Load(pugi::xml_node xmlNode)
{
    DUIASSERT(m_pContainer);
    if (!xmlNode)
    {
        return FALSE;
    }

    {
        m_strInnerText = DUI_CA2T(xmlNode.text().get(), CP_UTF8);
        if (!m_strInnerText.IsEmpty())
		{
			m_strInnerText.TrimRight(0x20).TrimLeft(0x20);
			if (!m_strInnerText.IsEmpty()) BUILDSTRING(m_strInnerText);
		}
    }

	m_dlgpos.nCount = 0;
    m_dlgpos.uPositionType = 0;
    CDuiObject::Load(xmlNode);

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
        if(m_pParent)	m_pParent->DestroyChild(this);
        return FALSE;
    }
    LoadChildren(xmlNode.first_child());
    return TRUE;
}

CDuiWindow * CDuiWindow::LoadXmlChildren(LPCSTR utf8Xml)
{
	pugi::xml_document xmlDoc;
	if(!xmlDoc.load_buffer(utf8Xml,strlen(utf8Xml),pugi::parse_default,pugi::encoding_utf8)) return NULL;
	CDuiWindow * pLastChild=m_pLastChild;//保存当前的最后一个子窗口
    BOOL bLoaded=LoadChildren(xmlDoc.first_child());
    if(!bLoaded) return NULL;

  	CRect rcContainer=GetChildrenLayoutRect();
  
	CDuiWindow *pNewChild=NULL;
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
HDUIWND CDuiWindow::DuiGetHWNDFromPoint(CPoint ptHitTest, BOOL bOnlyText)
{
    if (!m_rcWindow.PtInRect(ptHitTest)) return NULL;

	CRect rcClient;
	GetClient(&rcClient);

	if(!rcClient.PtInRect(ptHitTest))
		return m_hDuiWnd;	//只在鼠标位于客户区时，才继续搜索子窗口
	
    HDUIWND hDuiWndChild = NULL;

    CDuiWindow *pChild=m_pLastChild;
    while(pChild)
    {
        if (pChild->IsVisible(TRUE) && !pChild->IsMsgTransparent())
        {
            hDuiWndChild = pChild->DuiGetHWNDFromPoint(ptHitTest, bOnlyText);

            if (hDuiWndChild) return hDuiWndChild;
        }

        pChild=pChild->m_pPrevSibling;
    }

    return m_hDuiWnd;
}

BOOL CDuiWindow::NeedRedrawWhenStateChange()
{
    if (m_pBgSkin && !m_pBgSkin->IgnoreState())
    {
        return TRUE;
    }

    return (CLR_INVALID != m_style.m_crHoverText) || (NULL != m_style.m_ftHover) || (CLR_INVALID != m_style.m_crBgHover);
}

BOOL CDuiWindow::RedrawRegion(CDCHandle& dc, CRgn& rgn)
{
	PRSTATE prState=PRS_LOOKSTART;
	return _PaintRegion(dc,rgn,this,this,NULL,prState);
}

void CDuiWindow::OnAttributeChanged( const CDuiStringA & strAttrName,BOOL bLoading,HRESULT hRet )
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

void CDuiWindow::NotifyInvalidate()
{
	CRect rcClient;
	GetClient(&rcClient);
    NotifyInvalidateRect(rcClient);
}

void CDuiWindow::NotifyInvalidateRect(LPRECT lprect)
{
    if (lprect)
    {
        CRect rect = *lprect;
        NotifyInvalidateRect(rect);
    }
}

void CDuiWindow::NotifyInvalidateRect(const CRect& rect)
{
	if(!IsVisible(TRUE)) return ;
	BOOL bUpdateLocked=FALSE;
	CDuiWindow *pWnd=this;
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

void CDuiWindow::LockUpdate()
{
    m_bUpdateLocked=TRUE;
}

void CDuiWindow::UnlockUpdate()
{
    m_bUpdateLocked=FALSE;
}

BOOL CDuiWindow::IsUpdateLocked()
{
    return m_bUpdateLocked;
}

void CDuiWindow::BringWindowToTop()
{
	CDuiWindow *pParent=GetParent();
    if(!pParent) return;
    pParent->RemoveChild(this);
    pParent->InsertChild(this);
}

LRESULT CDuiWindow::DuiNotify(LPDUINMHDR pnms)
{
	EventArgs args(pnms,this);
	FireEvent(pnms->code,args);
	if(args.handled != 0) return 0;

	if(GetOwner()) return GetOwner()->DuiNotify(pnms);
    return GetContainer()->OnDuiNotify(pnms);
}

LRESULT CDuiWindow::OnWindowPosChanged(LPRECT lpRcContainer)
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

int CDuiWindow::OnCreate( LPVOID )
{
    return 0;
}

void CDuiWindow::OnDestroy()
{
    //destroy children dui windows
    CDuiWindow *pChild=m_pFirstChild;
    while (pChild)
    {
        CDuiWindow *pNextChild=pChild->m_pNextSibling;
        pChild->DuiSendMessage(WM_DESTROY);
        pChild->Release();

        pChild=pNextChild;
    }
    m_pFirstChild=m_pLastChild=NULL;
    m_nChildrenCount=0;
}

// Draw background default
BOOL CDuiWindow::OnEraseBkgnd(CDCHandle dc)
{
    CRect rcClient;
    GetClient(&rcClient);
    if (!m_pBgSkin)
    {
        COLORREF crBg = m_style.m_crBg;

        if (DuiWndState_Hover == (GetState() & DuiWndState_Hover) && CLR_INVALID != m_style.m_crBgHover)
        {
            crBg = m_style.m_crBgHover;
        }

        if (CLR_INVALID != crBg)
            CGdiAlpha::FillSolidRect(dc,&rcClient, crBg);
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
		m_pBgSkin->Draw(dc, rcClient, nState,m_byAlpha); 
    }
    return TRUE;
}

void CDuiWindow::BeforePaint(CDCHandle &dc, DuiDCPaint &DuiDC)
{
    HFONT hftDraw = NULL;
    COLORREF crDraw = CLR_INVALID;

    GetClient(DuiDC.rcClient);

    if (m_pBgSkin)
    {
        DuiDC.bDuiModeChanged = TRUE;
        DuiDC.nOldDuiMode = dc.SetBkMode(TRANSPARENT);
    }
    if (CLR_INVALID != m_style.m_crBg)
    {
        DuiDC.bBgColorChanged = TRUE;
        DuiDC.crOldBg = dc.SetBkColor(m_style.m_crBg);
    }

    if (m_style.m_ftText)
        hftDraw = m_style.m_ftText;

    if (m_style.m_crText != CLR_INVALID)
        crDraw = m_style.m_crText;

    if (IsDisabled(TRUE))
    {
        if (m_style.m_crDisabledText != CLR_INVALID)
            crDraw = m_style.m_crDisabledText;
    }
    else if (DuiWndState_Hover == (GetState() & DuiWndState_Hover))
    {
        if (m_style.m_ftHover)
            hftDraw = m_style.m_ftHover;

        if (m_style.m_crHoverText != CLR_INVALID)
            crDraw = m_style.m_crHoverText;
    }

    if (hftDraw)
    {
        DuiDC.bFontChanged = TRUE;
        DuiDC.hftOld = dc.SelectFont(hftDraw);
    }

    if (crDraw != CLR_INVALID)
    {
        DuiDC.bTextColorChanged = TRUE;
        DuiDC.crOld = dc.SetTextColor(crDraw);
    }
}

void CDuiWindow::BeforePaintEx( CDCHandle &dc)
{
    CDuiWindow *pParent=GetParent();
    if(pParent) pParent->BeforePaintEx(dc);
    DuiDCPaint DuiDC;
    BeforePaint(dc,DuiDC);
}

void CDuiWindow::AfterPaint(CDCHandle &dc, DuiDCPaint &DuiDC)
{
    if (DuiDC.bFontChanged)
        dc.SelectFont(DuiDC.hftOld);

    if (DuiDC.bTextColorChanged)
        dc.SetTextColor(DuiDC.crOld);

    if (DuiDC.bDuiModeChanged)
        dc.SetBkMode(DuiDC.nOldDuiMode);

    if (DuiDC.bBgColorChanged)
        dc.SetBkColor(DuiDC.crOldBg);
}

// Draw inner text default and focus rect
void CDuiWindow::OnPaint(CDCHandle dc)
{
    DuiDCPaint DuiDC;

    BeforePaint(dc, DuiDC);

	CRect rcText;
	GetTextRect(rcText);
    DuiDrawText(dc,m_strInnerText, m_strInnerText.GetLength(), rcText, GetTextAlign());

    //draw focus rect
    if(GetContainer()->GetDuiFocus()==m_hDuiWnd)
    {
        DuiDrawFocus(dc);
    }

    AfterPaint(dc, DuiDC);

}

void CDuiWindow::OnNcPaint(CDCHandle dc)
{
    if(m_style.m_nMarginX!=0 || m_style.m_nMarginY!=0)
    {
        BOOL bGetDC = dc==0;
        if(bGetDC) dc=GetDuiDC(&m_rcWindow,OLEDC_OFFSCREEN,FALSE);//不自动画背景
        int nSavedDC=dc.SaveDC();

        CRect rcClient;
        CDuiWindow::GetClient(&rcClient);
        dc.ExcludeClipRect(&rcClient);
        if(bGetDC) PaintBackground(dc,&m_rcWindow);

        int nState=0;
        if(DuiWndState_Hover & m_dwState) nState=1;
        if(m_pNcSkin)
		{
			if(nState>=m_pNcSkin->GetStates()) nState=0;
			m_pNcSkin->Draw(dc,m_rcWindow,nState,m_byAlpha);
		}
        else
        {
            COLORREF crBg = m_style.m_crBorder;
            if (DuiWndState_Hover == (GetState() & DuiWndState_Hover) && CLR_INVALID != m_style.m_crBgHover)
            {
                crBg = m_style.m_crBorderHover;
            }
            if (CLR_INVALID != crBg) CGdiAlpha::FillSolidRect(dc,&m_rcWindow, crBg);
        }
        if(bGetDC) PaintForeground(dc,&m_rcWindow);
        dc.RestoreDC(nSavedDC);
        if(bGetDC) ReleaseDuiDC(dc);
    }
}

CSize CDuiWindow::CalcSize(LPRECT pRcContainer)
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


CSize CDuiWindow::GetDesiredSize(LPRECT pRcContainer)
{
	DUIASSERT((m_dlgpos.uPositionType & SizeX_FitContent) || (m_dlgpos.uPositionType & SizeY_FitContent));


	int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

	CRect rcTest (0,0,0x7FFF,0x7FFF);
	if(m_nMaxWidth!=-1)
	{
		rcTest.right=m_nMaxWidth;
		nTestDrawMode|=DT_WORDBREAK;
	}

	CDCHandle dcDesktop = ::GetDC(::GetDesktopWindow());
	int nSavedDC=dcDesktop.SaveDC();

	BeforePaintEx(dcDesktop);
	DuiDrawText(dcDesktop,m_strInnerText, m_strInnerText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);

	dcDesktop.RestoreDC(nSavedDC);
	::ReleaseDC(::GetDesktopWindow(), dcDesktop);	
	return rcTest.Size();
}

void CDuiWindow::GetTextRect( LPRECT pRect )
{
	GetClient(pRect);
}

void CDuiWindow::DuiDrawText(HDC hdc,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
{
    CGdiAlpha::DrawText(hdc,pszBuf,cchText,pRect,uFormat);
}

void CDuiWindow::DuiDrawFocus(HDC dc)
{
    CRect rcFocus;
    GetTextRect(&rcFocus);
	if(IsTabStop())	DuiDrawDefFocusRect(dc,rcFocus);
}


void CDuiWindow::DuiDrawDefFocusRect( CDCHandle dc,CRect rcFocus )
{
	rcFocus.DeflateRect(2,2);
	HBRUSH hbr=(HBRUSH)::GetStockObject(NULL_BRUSH);
	CPen pen;
	pen.CreatePen(PS_DOT,1,RGB(88,88,88));
	HBRUSH hOldBr=dc.SelectBrush(hbr);
	HPEN hOldPen=dc.SelectPen(pen);
	ALPHAINFO ai;
	if(GetContainer()->IsTranslucent()) CGdiAlpha::AlphaBackup(dc,&rcFocus,ai);
	dc.Rectangle(&rcFocus);
	if(GetContainer()->IsTranslucent()) CGdiAlpha::AlphaRestore(dc,ai);
	dc.SelectPen(hOldPen);
	dc.SelectBrush(hOldBr);

}

UINT CDuiWindow::OnGetDuiCode()
{
    return 0;
}

BOOL CDuiWindow::IsTabStop()
{
    return m_bTabStop;
}

BOOL CDuiWindow::OnDefKeyDown(UINT nChar, UINT nFlags)
{
    CDuiWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        if(pChild->OnDefKeyDown(nChar,nFlags)) return TRUE;
        pChild=pChild->m_pNextSibling;
    }
    return FALSE;
}

void CDuiWindow::OnShowWindow(BOOL bShow, UINT nStatus)
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

    CDuiWindow *pChild=m_pFirstChild;
    while(pChild)
    {
        pChild->DuiSendMessage(WM_SHOWWINDOW,bShow,ParentShow);
		pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
    }
    if(!IsVisible(TRUE) && m_hDuiWnd == GetContainer()->GetDuiFocus())
    {
        GetContainer()->OnSetDuiFocus(NULL);
    }

	if(!m_bDisplay)
	{
		CDuiWindow *pParent=GetParent();
		if(pParent) pParent->UpdateChildrenPosition();
	}
}


void CDuiWindow::OnEnable( BOOL bEnable,UINT nStatus )
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

	CDuiWindow *pChild=m_pFirstChild;
	while(pChild)
	{
		pChild->DuiSendMessage(WM_ENABLE,bEnable,ParentEnable);
		pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
	}
	if(IsDisabled(TRUE) && m_hDuiWnd == GetContainer()->GetDuiFocus())
	{
		GetContainer()->OnSetDuiFocus(NULL);
	}
}

void CDuiWindow::OnLButtonDown(UINT nFlags,CPoint pt)
{
	if(m_bTabStop) SetDuiFocus();
    SetDuiCapture();
    ModifyState(DuiWndState_PushDown, 0,TRUE);
}

void CDuiWindow::OnLButtonUp(UINT nFlags,CPoint pt)
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

void CDuiWindow::OnRButtonDown( UINT nFlags, CPoint point )
{
	NotifyContextMenu(point);
}

void CDuiWindow::OnMouseHover(WPARAM wParam, CPoint ptPos)
{
    if(GetDuiCapture()==m_hDuiWnd)
        ModifyState(DuiWndState_PushDown,0,FALSE);
    ModifyState(DuiWndState_Hover, 0,TRUE);
    OnNcPaint(0);
}

void CDuiWindow::OnMouseLeave()
{
    if(GetDuiCapture()==m_hDuiWnd)
        ModifyState(0,DuiWndState_PushDown,FALSE);
    ModifyState(0,DuiWndState_Hover,TRUE);
    OnNcPaint(0);
}

BOOL CDuiWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    BOOL bRet=FALSE;
    if(m_pParent) bRet=(BOOL)m_pParent->DuiSendMessage(WM_MOUSEWHEEL,MAKEWPARAM(nFlags,zDelta),MAKELPARAM(pt.x,pt.y));
    return bRet;
}

HRESULT CDuiWindow::OnAttributeState( const CDuiStringA& strValue, BOOL bLoading )
{
	int nState=0;
	::StrToIntExA(strValue,STIF_SUPPORT_HEX,&nState);
	m_dwState=nState;
	if(m_dwState & DuiWndState_Invisible) m_bVisible=FALSE;
	if(m_dwState & DuiWndState_Disable) m_bDisable=TRUE;
	return S_FALSE;
}


HRESULT CDuiWindow::OnAttributePosition(const CDuiStringA& strValue, BOOL bLoading)
{
    if (strValue.IsEmpty()) return E_FAIL;

	ClearLayoutState();
	CDuiLayout::StrPos2DuiWndPos(strValue,m_dlgpos);
    return bLoading?S_FALSE:S_OK;
}


CRect CDuiWindow::GetChildrenLayoutRect()
{
	CRect rcRet;
	GetClient(rcRet);//通常是非客户区，但是tab这样的控件不一样。
	return rcRet;
}


void CDuiWindow::ClearLayoutState()
{
	if(m_dlgpos.uPositionType & Pos_Float) return;

	m_rcWindow.left=m_rcWindow.top=m_rcWindow.right=m_rcWindow.bottom=POS_INIT;
}

void CDuiWindow::UpdateChildrenPosition()
{
	CDuiList<CDuiWindow*> lstWnd;
	CDuiWindow *pChild=GetDuiWindow(GDUI_FIRSTCHILD);
	while(pChild)
	{
		pChild->ClearLayoutState();
		lstWnd.AddTail(pChild);
		pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
	}
	CDuiLayout::CalcChildrenPosition(this,&lstWnd);
	NotifyInvalidate();
}

void CDuiWindow::OnSetDuiFocus()
{
	NotifyInvalidateRect(m_rcWindow);
}

void CDuiWindow::OnKillDuiFocus()
{
	NotifyInvalidateRect(m_rcWindow);
}

HDC CDuiWindow::GetDuiDC(const LPRECT pRc/*=NULL*/,DWORD gdcFlags/*=0*/,BOOL bClientDC/*=TRUE*/)
{
    DUIASSERT(m_gdcFlags==-1);
	if(bClientDC)
		GetClient(&m_rcGetDC);
    else
		m_rcGetDC=m_rcWindow;

	if(gdcFlags!=OLEDC_NODRAW)
	{//将DC限制在父窗口的可见区域
		CDuiWindow *pParent=GetParent();
		while(pParent)
		{
			CRect rcParent;
			pParent->GetClient(&rcParent);
			m_rcGetDC.IntersectRect(m_rcGetDC,rcParent);
			pParent=pParent->GetParent();
		}
	}

    m_gdcFlags=gdcFlags;
    BOOL bClip=FALSE;
    if(pRc)
    {
        m_rcGetDC.IntersectRect(pRc,&m_rcGetDC);
        bClip=!m_rcGetDC.EqualRect(pRc);
    }
    HDC hdc=GetContainer()->OnGetDuiDC(m_rcGetDC,gdcFlags);
    if(bClip)
    {
        m_nSaveDC=SaveDC(hdc);
        CRgn rgn;
        rgn.CreateRectRgnIndirect(&m_rcGetDC);
        SelectClipRgn(hdc,rgn);
    }
    else
    {
        m_nSaveDC=0;
    }
    if(gdcFlags&OLEDC_PAINTBKGND)
        PaintBackground(hdc,&m_rcGetDC);

    return hdc;
}

void CDuiWindow::ReleaseDuiDC(HDC hdc)
{
    if(m_gdcFlags & OLEDC_PAINTBKGND) //画了背景，自动画前景
        PaintForeground(hdc,&m_rcGetDC);
    if(m_nSaveDC!=0) RestoreDC(hdc,m_nSaveDC);
    GetContainer()->OnReleaseDuiDC(hdc,m_rcGetDC,m_gdcFlags);
    m_nSaveDC=0;
    m_gdcFlags=-1;
}

HDUIWND CDuiWindow::GetDuiCapture()
{
    return GetContainer()->OnGetDuiCapture();
}

HDUIWND CDuiWindow::SetDuiCapture()
{
    return GetContainer()->OnSetDuiCapture(m_hDuiWnd);
}

BOOL CDuiWindow::ReleaseDuiCapture()
{
    return GetContainer()->OnReleaseDuiCapture();
}

void CDuiWindow::SetDuiFocus()
{
    GetContainer()->OnSetDuiFocus(m_hDuiWnd);
}

void CDuiWindow::KillDuiFocus()
{
	if(GetContainer()->GetDuiFocus()==m_hDuiWnd)
	{
		GetContainer()->OnSetDuiFocus(NULL);
	}
}

CDuiWindow *CDuiWindow::GetCheckedRadioButton()
{
    CDuiWindow *pChild=m_pFirstChild;
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

void CDuiWindow::CheckRadioButton(CDuiWindow * pRadioBox)
{
    CDuiWindow *pChecked=GetCheckedRadioButton();
    if(pChecked == pRadioBox) return;
    if(pChecked)
    {
        pChecked->ModifyState(0,DuiWndState_Check,TRUE);
    }
    pRadioBox->ModifyState(DuiWndState_Check,0,TRUE);
}


BOOL CDuiWindow::SetItemVisible(UINT uItemID, BOOL bVisible)
{
    CDuiWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
    {
		pWnd->SetVisible(bVisible,TRUE);
        return TRUE;
    }

    return FALSE;
}

BOOL CDuiWindow::IsItemVisible(UINT uItemID, BOOL bCheckParent /*= FALSE*/)
{
    CDuiWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
        return pWnd->IsVisible(bCheckParent);

    return FALSE;
}

BOOL CDuiWindow::GetItemCheck(UINT uItemID)
{
    CDuiWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
        return pWnd->IsChecked();

    return FALSE;
}

BOOL CDuiWindow::SetItemCheck(UINT uItemID, BOOL bCheck)
{
    CDuiWindow *pWnd = FindChildByCmdID(uItemID);

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

BOOL CDuiWindow::EnableItem(UINT uItemID, BOOL bEnable)
{
    CDuiWindow *pWnd = FindChildByCmdID(uItemID);

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

BOOL CDuiWindow::IsItemEnable(UINT uItemID, BOOL bCheckParent /*= FALSE*/)
{
    CDuiWindow *pWnd = FindChildByCmdID(uItemID);

    if (pWnd)
        return !pWnd->IsDisabled(bCheckParent);

    return FALSE;
}

BOOL CDuiWindow::OnDuiNcHitTest(CPoint pt)
{
    return FALSE;
}

BOOL CDuiWindow::IsMsgHandled() const
{
    return m_bMsgHandled;
}

void CDuiWindow::SetMsgHandled(BOOL bHandled)
{
    m_bMsgHandled = bHandled;
}


CDuiWindow * CDuiWindow::GetDuiWindow(int uCode)
{
    CDuiWindow *pRet=NULL;
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


void CDuiWindow::PaintBackground( HDC hdc,LPRECT pRc )
{
    CRect rcDraw=m_rcWindow;
    if(pRc) rcDraw.IntersectRect(rcDraw,pRc);

	CDuiWindow *pTopWnd=GetTopLevelParent();
	CDCHandle dc(hdc); 
	int nSavedDC=dc.SaveDC();
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcDraw);
	dc.SelectClipRgn(rgn,RGN_AND);
	dc.FillSolidRect(rcDraw,0);//清除残留的alpha值
	PRSTATE prState=PRS_LOOKSTART;
	_PaintRegion(dc,rgn,pTopWnd,pTopWnd,this,prState);
	dc.RestoreDC(nSavedDC);
}

BOOL CDuiWindow::_PaintRegion( CDCHandle& dc, CRgn& rgn,CDuiWindow *pWndCur,CDuiWindow *pStart,CDuiWindow *pEnd,CDuiWindow::PRSTATE & prState )
{
	if (!pWndCur->IsVisible(TRUE) || (!rgn.IsNull() && !rgn.RectInRegion(pWndCur->m_rcWindow)))
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

	PRSTATE prsBack=prState;	//保存当前的绘制状态,在递归结束后根据这个状态来判断是否需要绘制非客户区

	CRect rcClient;
	pWndCur->GetClient(&rcClient);
	if(rgn.IsNull() || rgn.RectInRegion(rcClient))
	{//重绘客户区
		CRgn rgnOldClip;
		if(prsBack == PRS_DRAWING)
		{
			if(pWndCur->IsClipClient())
			{
				dc.GetClipRgn(rgnOldClip);
				CRgn rgn;
				rgn.CreateRectRgnIndirect(&rcClient);
				dc.SelectClipRgn(rgn,RGN_AND);
			}
			pWndCur->DuiSendMessage(WM_ERASEBKGND, (WPARAM)(HDC)dc);
			pWndCur->DuiSendMessage(WM_PAINT, (WPARAM)(HDC)dc);
		}

		DuiDCPaint DuiDC;

		pWndCur->BeforePaint(dc, DuiDC);	//让子窗口继承父窗口的属性

		CDuiWindow *pChild=pWndCur->GetDuiWindow(GDUI_FIRSTCHILD);
		while(pChild)
		{
			if(pChild==pEnd) break;
			_PaintRegion(dc, rgn,pChild,pStart,pEnd,prState);
			if(prState == PRS_MEETEND)
				break;
			pChild=pChild->GetDuiWindow(GDUI_NEXTSIBLING);
		}

		pWndCur->AfterPaint(dc, DuiDC);
		if(prsBack == PRS_DRAWING && pWndCur->IsClipClient())
		{
			dc.SelectClipRgn(rgnOldClip);
		}
	}
	if(prsBack == PRS_DRAWING) 
		pWndCur->DuiSendMessage(WM_NCPAINT, (WPARAM)(HDC)dc);//ncpaint should be placed in tail of paint link

	return prState==PRS_DRAWING || prState == PRS_MEETEND;
}

void CDuiWindow::PaintForeground( HDC hdc,LPRECT pRc )
{
    CRect rcDraw=m_rcWindow;
    if(pRc) rcDraw.IntersectRect(rcDraw,pRc);

    CDuiWindow *pStart=GetNextVisibleWindow(this,rcDraw);

    if(pStart)
    {
		PRSTATE prState=PRS_LOOKSTART;
		CRgn rgn;
		rgn.CreateRectRgnIndirect(rcDraw);
		CDCHandle dc(hdc);
		int nSavedDC=dc.SaveDC();
		dc.SelectClipRgn(rgn,RGN_AND);
		_PaintRegion(CDCHandle(hdc),rgn,this,pStart,NULL,prState);
		dc.RestoreDC(nSavedDC);
	}
}


CDuiWindow * CDuiWindow::GetNextVisibleWindow( CDuiWindow *pWnd ,const CRect &rcDraw)
{
	if(!pWnd) return NULL;
	CDuiWindow *pNextSibling=pWnd->GetDuiWindow(GDUI_NEXTSIBLING);
	if(pNextSibling && pNextSibling->IsVisible(TRUE) && !(pNextSibling->m_rcWindow & rcDraw).IsRectEmpty())
		return pNextSibling;
	else if (pNextSibling)	return GetNextVisibleWindow(pNextSibling,rcDraw);
	else return GetNextVisibleWindow(pWnd->GetParent(),rcDraw);
}

void CDuiWindow::DrawAniStep( CRect rcFore,CRect rcBack,HDC dcFore,HDC dcBack,CPoint ptAnchor)
{
	CDCHandle dc=GetDuiDC(rcBack,OLEDC_OFFSCREEN,FALSE);
	BitBlt(dc,rcBack.left,rcBack.top,rcBack.Width(),rcBack.Height(),dcBack,rcBack.left,rcBack.top,SRCCOPY);
	BitBlt(dc,rcFore.left,rcFore.top,rcFore.Width(),rcFore.Height(),dcFore,ptAnchor.x,ptAnchor.y,SRCCOPY);
	PaintForeground(dc,rcBack);//画前景
	ReleaseDuiDC(dc);
}

void CDuiWindow::DrawAniStep( CRect rcWnd,HDC dcFore,HDC dcBack,BYTE byAlpha)
{
	CDCHandle dc=GetDuiDC(rcWnd,OLEDC_OFFSCREEN,FALSE);
	if(byAlpha>0 && byAlpha<255)
	{
		BitBlt(dc,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),dcBack,rcWnd.left,rcWnd.top,SRCCOPY);
		//do alphablend
		BLENDFUNCTION bf={0};
		bf.BlendOp=AC_SRC_OVER;
		bf.AlphaFormat=0;//AC_SRC_ALPHA
		bf.SourceConstantAlpha=byAlpha;
		AlphaBlend(dc,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),dcFore,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),bf);
	}else if(byAlpha==0)
	{
		BitBlt(dc,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),dcBack,rcWnd.left,rcWnd.top,SRCCOPY);
	}else if(byAlpha==255)
	{
		BitBlt(dc,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),dcFore,rcWnd.left,rcWnd.top,SRCCOPY);
	}
	PaintForeground(dc,rcWnd);//画前景
	ReleaseDuiDC(dc);
}

BOOL CDuiWindow::AnimateWindow(DWORD dwTime,DWORD dwFlags )
{
	if(dwFlags & AW_HIDE)
	{
		if(!IsVisible(TRUE))
			return FALSE;
	}else
	{//动画显示窗口时，不能是最顶层窗口，同时至少上一层窗口应该可见
		if(IsVisible(TRUE))
			return FALSE;
		CDuiWindow *pParent=GetParent();
		if(!pParent) return FALSE;
		if(!pParent->IsVisible(TRUE)) return FALSE;
	}
	CRect rcWnd;
	GetRect(&rcWnd);

	CRgn rgn;
	rgn.CreateRectRgnIndirect(rcWnd);

	CDCHandle dc=GetDuiDC(rcWnd,OLEDC_NODRAW,FALSE);
	CMemDC dcBefore(dc,CGdiAlpha::CreateBitmap32(dc,rcWnd.Width(),rcWnd.Height(),NULL,255));
	dcBefore.SetBitmapOwner(TRUE); 
	dcBefore.OffsetViewportOrg(-rcWnd.left,-rcWnd.top);

	//渲染窗口变化前状态
	PaintBackground(dc,rcWnd);
	RedrawRegion(CDCHandle(dc),rgn);
	BitBlt(dcBefore,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),dc,rcWnd.left,rcWnd.top,SRCCOPY);
	//更新窗口可见性
	SetVisible(!(dwFlags&AW_HIDE),FALSE);
	//窗口变化后
	CMemDC dcAfter(dc,CGdiAlpha::CreateBitmap32(dc,rcWnd.Width(),rcWnd.Height(),NULL,255));
	dcAfter.SetBitmapOwner(TRUE); 
	dcAfter.OffsetViewportOrg(-rcWnd.left,-rcWnd.top);

	PaintBackground(dc,rcWnd);
	RedrawRegion(CDCHandle(dc),rgn);
	BitBlt(dcAfter,rcWnd.left,rcWnd.top,rcWnd.Width(),rcWnd.Height(),dc,rcWnd.left,rcWnd.top,SRCCOPY);

	ReleaseDuiDC(dc);

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
				DrawAniStep(rcNewState,rcWnd,dcBefore,dcAfter,ptAnchor);
				Sleep(10);
			}
			DrawAniStep(CRect(),rcWnd,dcBefore,dcAfter,rcWnd.TopLeft());
			return TRUE;
		}else if(dwFlags&AW_CENTER)
		{
			CRect rcNewState(rcWnd);
			int xStep=rcNewState.Width()/(2*nSteps);
			int yStep=rcNewState.Height()/(2*nSteps);
			for(int i=0;i<nSteps;i++)
			{
				rcNewState.DeflateRect(xStep,yStep);
				DrawAniStep(rcNewState,rcWnd,dcBefore,dcAfter,rcNewState.TopLeft());
				Sleep(10);
			}
			DrawAniStep(CRect(),rcWnd,dcBefore,dcAfter,rcWnd.TopLeft());
			return TRUE;
		}else if(dwFlags&AW_BLEND)
		{
			BYTE byAlpha=255;
			BYTE byStepLen=255/nSteps;
			for(int i=0;i<nSteps;i++)
			{
				DrawAniStep(rcWnd,dcBefore,dcAfter,byAlpha);
				Sleep(10);
				byAlpha-=byStepLen;
			}
			DrawAniStep(rcWnd,dcBefore,dcAfter,0);
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
				DrawAniStep(rcNewState,rcWnd,dcAfter,dcBefore,ptAnchor);
				Sleep(10);
			}
			DrawAniStep(rcWnd,rcWnd,dcAfter,dcBefore,rcWnd.TopLeft());
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
				DrawAniStep(rcNewState,rcWnd,dcAfter,dcBefore,rcNewState.TopLeft());
				Sleep(10);
			}
			DrawAniStep(rcWnd,rcWnd,dcAfter,dcBefore,rcWnd.TopLeft());
			return TRUE;
		}else if(dwFlags&AW_BLEND)
		{
			BYTE byAlpha=0;
			BYTE byStepLen=255/nSteps;
			for(int i=0;i<nSteps;i++)
			{
				DrawAniStep(rcWnd,dcAfter,dcBefore,byAlpha);
				Sleep(10);
				byAlpha+=byStepLen;
			}
			DrawAniStep(rcWnd,dcAfter,dcBefore,255);
			return TRUE;
		}
		return FALSE;
	}
}

LRESULT CDuiWindow::NotifyCommand()
{
	DUINMCOMMAND nms;
	nms.hdr.hDuiWnd=m_hDuiWnd;
	nms.hdr.code = NM_COMMAND;
	nms.hdr.idFrom = GetCmdID();
	nms.hdr.pszNameFrom=GetName();
	nms.uItemData = GetUserData();
	return DuiNotify((LPDUINMHDR)&nms);
}

LRESULT CDuiWindow::NotifyContextMenu( CPoint pt )
{
	DUINMCONTEXTMENU nms;
	nms.hdr.hDuiWnd=m_hDuiWnd;
	nms.hdr.code = NM_CONTEXTMENU;
	nms.hdr.idFrom = GetCmdID();
	nms.hdr.pszNameFrom=GetName();
	nms.uItemData = GetUserData();
	nms.pt=pt;
	return DuiNotify((LPDUINMHDR)&nms);
}

}//namespace SOUI
