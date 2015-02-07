#include "souistd.h"
#include "core/SWnd.h"
#include "core/SwndLayoutBuilder.h"
#include "helper/color.h"
#include "helper/SplitString.h"

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    // SWindow Implement
    //////////////////////////////////////////////////////////////////////////

    SWindow::SWindow()
        : m_swnd(SWindowMgr::NewWindow(this))
        , m_nID(0)
        , m_pContainer(NULL)
        , m_pParent(NULL),m_pFirstChild(NULL),m_pLastChild(NULL),m_pNextSibling(NULL),m_pPrevSibling(NULL)
        , m_nChildrenCount(0)
        , m_uZorder(0)
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
        , m_bCacheDirty(TRUE)
        , m_bLayeredWindow(FALSE)
        , m_uData(0)
        , m_pOwner(NULL)
        , m_pCurMsg(NULL)
        , m_pBgSkin(NULL)
        , m_pNcSkin(NULL)
        , m_pGetRTData(NULL)
        , m_bFloat(FALSE)
#ifdef _DEBUG
        , m_nMainThreadId( ::GetCurrentThreadId() ) // 初始化对象的线程不一定是主线程
#endif
    {
        m_evtSet.addEvent(EVT_MOUSE_HOVER);
        m_evtSet.addEvent(EVT_MOUSE_LEAVE);
        m_evtSet.addEvent(EVT_VISIBLECHANGED);
        m_evtSet.addEvent(EVT_STATECHANGED);
        m_evtSet.addEvent(EVT_DESTROY);
        
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


    void SWindow::GetWindowRect(LPRECT prect)
    {
        SASSERT(prect);
        if(m_rcWindow.left == POS_INIT || m_rcWindow.left == POS_WAIT)
        {
            memset(prect,0,sizeof(RECT));
        }else
        {
            memcpy(prect,&m_rcWindow,sizeof(RECT));
        }
    }

    void SWindow::GetClientRect(LPRECT pRect)
    {
        SASSERT(pRect);
        if(m_rcWindow.left == POS_INIT || m_rcWindow.left == POS_WAIT)
        {
            memset(pRect,0,sizeof(RECT));
        }else
        {
            *pRect=m_rcWindow;
            pRect->left+=m_style.m_nMarginX;
            pRect->right-=m_style.m_nMarginX;
            pRect->top+=m_style.m_nMarginY;
            pRect->bottom-=m_style.m_nMarginY;
        }
    }

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
        if (m_layout.IsFitContent(PD_ALL))
        {
            if(GetParent()) GetParent()->UpdateChildrenPosition();
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
    void SWindow::Move(LPCRECT prect)
    {
        SASSERT(prect);
        TestMainThread();

        m_bFloat = TRUE;//使用Move后，程序不再自动计算窗口坐标
        if(m_rcWindow.EqualRect(prect)) return;

        CRect rcOld = m_rcWindow;
        m_rcWindow = prect;

        OnRelayout(rcOld,m_rcWindow);
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
        TestMainThread();

        DWORD dwOldState = m_dwState;

        DWORD dwNewState = m_dwState;
        dwNewState &= ~dwStateRemove;
        dwNewState |= dwStateAdd;

        OnStateChanging(dwOldState,dwNewState);

        m_dwState = dwNewState;

        OnStateChanged(dwOldState,dwNewState);
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


    BOOL SWindow::DestroyChild(SWindow *pChild)
    {
        TestMainThread();
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

    void SWindow::InsertChild(SWindow *pNewChild,SWindow *pInsertAfter/*=ICWND_LAST*/)
    {
        TestMainThread();
        if(pNewChild->m_pParent == this) 
            return;

        pNewChild->SetContainer(GetContainer());
        pNewChild->m_pParent=this;
        pNewChild->m_pPrevSibling=pNewChild->m_pNextSibling=NULL;

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
        
        //只在插入新控件时需要标记zorder失效,删除控件不需要标记
        GetContainer()->MarkWndTreeZorderDirty();
    }

    BOOL SWindow::RemoveChild(SWindow *pChild)
    {
        TestMainThread();
        if(this != pChild->GetParent()) 
            return FALSE;

        SWindow *pPrevSib=pChild->m_pPrevSibling;
        SWindow *pNextSib=pChild->m_pNextSibling;

        if(pPrevSib) 
            pPrevSib->m_pNextSibling=pNextSib;
        else 
            m_pFirstChild=pNextSib;

        if(pNextSib) 
            pNextSib->m_pPrevSibling=pPrevSib;
        else 
            m_pLastChild=pPrevSib;

        pChild->m_pParent=NULL;
        pChild->m_pNextSibling = NULL;
        pChild->m_pPrevSibling = NULL;
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

    ISwndContainer *SWindow::GetContainer()
    {
        return m_pContainer;
    }

    void SWindow::SetContainer(ISwndContainer *pContainer)
    {
        TestMainThread();
        m_pContainer=pContainer;
        SWindow *pChild=GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            pChild->SetContainer(pContainer);
            pChild=pChild->GetWindow(GSW_NEXTSIBLING);
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

    //改用广度优先算法搜索控件,便于逐级查找 2014年12月8日
    SWindow* SWindow::FindChildByID(int id, int nDeep/* =-1*/)
    {
        if(id == 0 || nDeep ==0) return NULL;


        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if (pChild->GetID() == id)
                return pChild;
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        if(nDeep>0) nDeep--;
        if(nDeep==0) return NULL;

        pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            SWindow *pChildFind=pChild->FindChildByID(id,nDeep);
            if(pChildFind) return pChildFind;
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        return NULL;
    }

    SWindow* SWindow::FindChildByName( LPCWSTR pszName , int nDeep)
    {
        if(!pszName || nDeep ==0) return NULL;

        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if (pChild->m_strName == pszName)
                return pChild;
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        if(nDeep>0) nDeep--;
        if(nDeep==0) return NULL;

        pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            SWindow *pChildFind=pChild->FindChildByName(pszName,nDeep);
            if(pChildFind) return pChildFind;
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        return NULL;
    }

    BOOL SWindow::CreateChildren(pugi::xml_node xmlNode)
    {
        TestMainThread();
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

            }else if(!xmlChild.get_userdata())//通过userdata来标记一个节点是否可以忽略
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
        TestMainThread();
        SASSERT(m_pContainer);
        if (xmlNode)
        {
            m_strText = S_CW2T(tr(xmlNode.text().get()));   //使用语言包翻译。

            if (!m_strText.IsEmpty())
            {
                m_strText.TrimBlank();
                if (!m_strText.IsEmpty()) BUILDSTRING(m_strText);
            }

            m_layout.Clear();

            //标记不处理width , height and size属性
            xmlNode.attribute(L"width").set_userdata(1);
            xmlNode.attribute(L"height").set_userdata(1);
            xmlNode.attribute(L"size").set_userdata(1);

            SObject::InitFromXml(xmlNode);

            if(!m_bVisible || (m_pParent && !m_pParent->IsVisible(TRUE)))
                ModifyState(WndState_Invisible, 0);

            if (4 != m_layout.nCount)
            {
                m_layout.InitWidth(xmlNode.attribute(L"width").value());
                m_layout.InitHeight(xmlNode.attribute(L"height").value());
                m_layout.InitSizeFromString(xmlNode.attribute(L"size").value());
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
        UpdateChildrenPosition();
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

        SWindow *pChild=GetWindow(GSW_LASTCHILD);
        while(pChild)
        {
            if (pChild->IsVisible(TRUE) && !pChild->IsMsgTransparent())
            {
                swndChild = pChild->SwndFromPoint(ptHitTest, bOnlyText);

                if (swndChild) return swndChild;
            }

            pChild=pChild->GetWindow(GSW_PREVSIBLING);
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

    //如果当前窗口有绘制缓存，它可能是由cache属性定义的，也可能是由于定义了alpha
    void SWindow::_PaintClient(IRenderTarget *pRT)
    {
        if(IsDrawToCache())
        {
            IRenderTarget *pRTCache=m_cachedRT;
            if(pRTCache)
            {//在窗口正在创建的时候进来pRTCache可能为NULL
                CRect rcWnd=m_rcWindow;
                if(IsCacheDirty())
                {
                    pRTCache->ClearRect(&rcWnd,0);
                    
                    CAutoRefPtr<IFont> oldFont;
                    COLORREF crOld=pRT->GetTextColor();
                    pRTCache->SelectObject(pRT->GetCurrentObject(OT_FONT),(IRenderObj**)&oldFont);
                    pRTCache->SetTextColor(crOld);

                    SSendMessage(WM_ERASEBKGND, (WPARAM)pRTCache);
                    SSendMessage(WM_PAINT, (WPARAM)pRTCache);

                    pRTCache->SelectObject(oldFont);
                    pRTCache->SetTextColor(crOld);

                    MarkCacheDirty(false);
                }
                pRT->AlphaBlend(&rcWnd,pRTCache,&rcWnd,IsLayeredWindow()?0xFF:m_style.m_byAlpha);
            }
        }else
        {
            SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
            SSendMessage(WM_PAINT, (WPARAM)pRT);
        }
    }

    void SWindow::_PaintNonClient( IRenderTarget *pRT )
    {
        CRect rcWnd;
        GetWindowRect(&rcWnd);
        CRect rcClient;
        GetClientRect(&rcClient);
        if(rcWnd==rcClient) return;

        SSendMessage(WM_NCPAINT, (WPARAM)pRT);
        if(IsDrawToCache())
        {
            IRenderTarget *pRTCache=m_cachedRT;
            if(pRTCache)
            {
                SSendMessage(WM_NCPAINT, (WPARAM)pRTCache);
                pRT->PushClipRect(&rcClient,RGN_DIFF);
                pRT->AlphaBlend(&rcWnd,pRTCache,&rcWnd,IsLayeredWindow()?0xFF:m_style.m_byAlpha);
                pRT->PopClip();
            }
        }else
        {
            SSendMessage(WM_NCPAINT, (WPARAM)pRT);
        }
    }
    
    //paint zorder in [iZorderBegin,iZorderEnd) widnows
    void SWindow::_PaintRegion2( IRenderTarget *pRT, IRegion *pRgn,UINT iZorderBegin,UINT iZorderEnd )
    {
        if(!IsVisible(TRUE))  //只在自己完全可见的情况下才绘制
            return;
        
        CRect rcWnd,rcClient;
        GetWindowRect(&rcWnd);
        GetClientRect(&rcClient);
        
        IRenderTarget *pRTBack = NULL;//backup current RT
        
        if(IsLayeredWindow() && pRT != GetLayerRenderTarget())
        {//获得当前LayeredWindow RT来绘制内容
            pRTBack = pRT;
            pRT = GetLayerRenderTarget();
           
            //绘制到窗口的缓存上,需要继承原RT的绘图属性
            CAutoRefPtr<IFont> curFont;
            HRESULT hr = pRTBack->SelectDefaultObject(OT_FONT,(IRenderObj**)&curFont);
            COLORREF crTxt = pRTBack->GetTextColor();
            if(S_OK == hr) pRT->SelectObject(curFont);
            pRT->SetTextColor(crTxt);

            if(pRgn && !pRgn->IsEmpty()) pRT->PushClipRegion(pRgn,RGN_COPY);
            pRT->ClearRect(&rcWnd,0);
        }
        
        if(IsClipClient())
        {
            pRT->PushClipRect(rcClient);
        }
        if(m_uZorder >= iZorderBegin
            && m_uZorder < iZorderEnd 
            && (!pRgn || pRgn->IsEmpty() || pRgn->RectInRegion(&rcWnd)))
        {//paint client
            _PaintClient(pRT);
        }

        SPainter painter;
        BeforePaint(pRT,painter);

        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(pChild->m_uZorder >= iZorderEnd) break;
            if(pChild->m_uZorder< iZorderBegin)
            {//看整个分枝的zorder是不是在绘制范围内
                SWindow *pNextChild = pChild->GetWindow(GSW_NEXTSIBLING);
                if(pNextChild)
                {
                    if(pNextChild->m_uZorder<=iZorderBegin)
                    {
                        pChild = pNextChild;
                        continue;
                    }
                }else
                {//最后一个节点时查看最后子窗口的zorder
                    SWindow *pLastChild = pChild;
                    while(pLastChild->GetChildrenCount())
                    {
                        pLastChild = pLastChild->GetWindow(GSW_LASTCHILD);
                    }
                    if(pLastChild->m_uZorder < iZorderBegin)
                    {
                        break;
                    }
                }
            }
            pChild->_PaintRegion2(pRT,pRgn,iZorderBegin,iZorderEnd);
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
        AfterPaint(pRT,painter);

        if(IsClipClient())
        {
            pRT->PopClip();
        }

        if(m_uZorder >= iZorderBegin
            && m_uZorder < iZorderEnd 
            && (!pRgn ||  pRgn->IsEmpty() ||pRgn->RectInRegion(&rcWnd)) )
        {//paint nonclient
            _PaintNonClient(pRT);
        }
        
        if(pRTBack)
        {//将绘制到窗口的缓存上的图像返回到上一级RT
            if(pRgn  && !pRgn->IsEmpty()) pRT->PopClip();
            pRTBack->AlphaBlend(&m_rcWindow,pRT,&m_rcWindow,m_style.m_byAlpha);
            
            CAutoRefPtr<IFont> curFont;
            HRESULT hr = pRT->SelectDefaultObject(OT_FONT,(IRenderObj**)&curFont);

            pRT = pRTBack;
            if(S_OK == hr) pRT->SelectObject(curFont);
        }

    }

    //当前函数中的参数包含zorder,为了保证传递进来的zorder是正确的,必须在外面调用zorder重建.
    void SWindow::_PaintRegion(IRenderTarget *pRT, IRegion *pRgn,UINT iZorderBegin,UINT iZorderEnd)
    {
        TestMainThread();
        BeforePaintEx(pRT);
        _PaintRegion2(pRT,pRgn,iZorderBegin,iZorderEnd);
    }

    void SWindow::RedrawRegion(IRenderTarget *pRT, IRegion *pRgn)
    {
        TestMainThread();
        _PaintRegion2(pRT,pRgn,ZORDER_MIN,ZORDER_MAX);
    }

    void SWindow::Invalidate()
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        InvalidateRect(rcClient);
    }

    void SWindow::InvalidateRect(LPCRECT lprect)
    {
        if (lprect)
        {
            CRect rect = *lprect;
            InvalidateRect(rect);
        }else
        {
            InvalidateRect(m_rcWindow);
        }
    }

    void SWindow::InvalidateRect(const CRect& rect)
    {
        TestMainThread();
        MarkCacheDirty(true);
        if(!IsVisible(TRUE) || IsUpdateLocked()) return ;
        //只能更新窗口有效区域
        CRect rcIntersect = rect & m_rcWindow;
        if(rcIntersect.IsRectEmpty()) return;

        if(!m_style.m_bBkgndBlend)
        {//非背景混合窗口，直接发消息支宿主窗口来启动刷新
            if(!m_invalidRegion)
            {
                GETRENDERFACTORY->CreateRegion(&m_invalidRegion);
            }
            m_invalidRegion->CombineRect(rcIntersect,RGN_OR);
            ::SendMessage(GetContainer()->GetHostHwnd(),UM_UPDATESWND,(WPARAM)m_swnd,0);//请求刷新窗口
        }else
        {
            if(GetParent())
            {
                GetParent()->InvalidateRect(rcIntersect);
            }else
            {
                GetContainer()->OnRedraw(rcIntersect);
            }
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
        TestMainThread();
        SWindow *pParent=GetParent();
        if(!pParent) return;
        pParent->RemoveChild(this);
        pParent->InsertChild(this);
    }

    BOOL SWindow::FireEvent(EventArgs &evt)
    {
        TestMainThread();
        if(m_evtSet.isMuted()) return FALSE;

        m_evtSet.FireEvent(evt);
        if(evt.handled != 0) return TRUE;
        
        if(GetOwner()) return GetOwner()->FireEvent(evt);
        return GetContainer()->OnFireEvent(evt);
    }


    void SWindow::OnRelayout(const CRect &rcOld, const CRect & rcNew)
    {
        SWindow *pParent= GetParent();
        if(pParent)
        {
            pParent->InvalidateRect(rcOld);
            pParent->InvalidateRect(rcNew);
        }else
        {
            InvalidateRect(m_rcWindow);
        }

        SSendMessage(WM_NCCALCSIZE);//计算非客户区大小

        CRect rcClient;
        GetClientRect(&rcClient);
        SSendMessage(WM_SIZE,0,MAKELPARAM(rcClient.Width(),rcClient.Height()));

        UpdateChildrenPosition();
    }

    int SWindow::OnCreate( LPVOID )
    {
        if(m_style.m_bTrackMouseEvent)
            GetContainer()->RegisterTrackMouseEvent(m_swnd);
        else
            GetContainer()->UnregisterTrackMouseEvent(m_swnd);

        return 0;
    }

    void SWindow::OnDestroy()
    {
        EventCmnArgs evt(this,EVT_DESTROY);
        FireEvent(evt);
        
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
            m_pBgSkin->Draw(pRT, rcClient, nState); 
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

    void SWindow::BeforePaintEx(IRenderTarget *pRT)
    {
        SWindow *pParent=GetParent();
        if(pParent) pParent->BeforePaintEx(pRT);
        SPainter painter;
        BeforePaint(pRT,painter);
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
            CRect rcWnd,rcClient;
            GetWindowRect(&rcWnd);
            SWindow::GetClientRect(&rcClient);

            BOOL bGetRT = pRT==0;
            CAutoRefPtr<IRegion> rgn;
            if(bGetRT) 
            {

                GETRENDERFACTORY->CreateRegion(&rgn);
                rgn->CombineRect(&rcWnd,RGN_COPY);
                rgn->CombineRect(&rcClient,RGN_DIFF);
                pRT=GetRenderTarget(OLEDC_OFFSCREEN,rgn);//不自动画背景
                PaintBackground(pRT,&m_rcWindow);
            }else
            {
                pRT->PushClipRect(&rcClient,RGN_DIFF);
            }

            int nState=0;
            if(WndState_Hover & m_dwState) nState=1;
            if(m_pNcSkin)
            {
                if(nState>=m_pNcSkin->GetStates()) nState=0;
                m_pNcSkin->Draw(pRT,m_rcWindow,nState);
            }
            else
            {
                COLORREF crBg = m_style.m_crBorder;
                if (CR_INVALID != crBg)
                {
                    pRT->FillSolidRect(&m_rcWindow,crBg);
                }
            }
            if(bGetRT) ReleaseRenderTarget(pRT);
            else pRT->PopClip();
        }
    }

    CSize SWindow::GetDesiredSize(LPCRECT pRcContainer)
    {
        CSize szRet;
        if(m_layout.IsSpecifySize(PD_X))
        {
            szRet.cx = m_layout.uSpecifyWidth;
        }
        if(m_layout.IsSpecifySize(PD_Y))
        {
            szRet.cy = m_layout.uSpecifyHeight;
        }

        if(szRet.cx && szRet.cy) 
            return szRet;

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

        if(m_layout.IsFitContent(PD_X)) 
            szRet.cx = rcTest.Width();
        if(m_layout.IsFitContent(PD_Y)) 
            szRet.cy = rcTest.Height();
        return szRet;
    }

    void SWindow::GetTextRect( LPRECT pRect )
    {
        GetClientRect(pRect);
    }

    void SWindow::DrawText(IRenderTarget *pRT,LPCTSTR pszBuf,int cchText,LPRECT pRect,UINT uFormat)
    {
        pRT->DrawText(pszBuf,cchText,pRect,uFormat);
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
        pRT->CreatePen(PS_DOT,RGBA(88,88,88,0xFF),1,&pPen);
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
            pChild->AddRef();
            pChild->SSendMessage(WM_SHOWWINDOW,bShow,ParentShow);
            SWindow *pNextChild=pChild->GetWindow(GSW_NEXTSIBLING);;
            pChild->Release();
            pChild=pNextChild;
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
        
        EventCmnArgs evtShow(this,EVT_VISIBLECHANGED);
        FireEvent(evtShow);
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
        OnNcPaint(NULL);
        EventCmnArgs evtHover(this,EVT_MOUSE_HOVER);
        FireEvent(evtHover);
    }

    void SWindow::OnMouseLeave()
    {
        if(GetCapture()==m_swnd)
            ModifyState(0,WndState_PushDown,FALSE);
        ModifyState(0,WndState_Hover,TRUE);
        OnNcPaint(NULL);
        EventCmnArgs evtLeave(this,EVT_MOUSE_LEAVE);
        FireEvent(evtLeave);
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
        GetClientRect(rcRet);//通常是客户区，但是tab这样的控件不一样。
        return rcRet;
    }

    void SWindow::UpdateChildrenPosition()
    {
        SList<SWindowRepos*> lstWnd;
        SWindow *pChild=GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(!pChild->m_bFloat)
                lstWnd.AddTail(new SWindowRepos(pChild));
            pChild=pChild->GetWindow(GSW_NEXTSIBLING);
        }
        SwndLayoutBuilder::CalcChildrenPosition(&lstWnd,GetChildrenLayoutRect());
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

    //当窗口有半透明属性并且透明度要需要应用于子窗口时，子窗口的图像渲染到this的缓存RT上。
    BOOL SWindow::IsLayeredWindow() const
    {
        return m_bLayeredWindow;
    }

    //查询当前窗口内容将被渲染到哪一个渲染层上，没有渲染层时返回NULL
    SWindow * SWindow::_GetCurrentLayeredWindow()
    {
        SWindow *pWnd = this;
        while(pWnd)
        {
            if(pWnd->IsLayeredWindow())
            {
                break;
            }
            pWnd = pWnd->GetParent();
        }

        return pWnd;
    }

    IRenderTarget * SWindow::GetRenderTarget(LPCRECT pRc,DWORD gdcFlags/*=OLEDC_NODRAW*/,BOOL bClientRT/*=TRUE*/)
    {
        CRect rcRT ;        
        if(bClientRT)
        {
            GetClientRect(&rcRT);
        }else
        {
            GetWindowRect(&rcRT);
        }
        if(pRc) rcRT.IntersectRect(pRc,&rcRT);
        
        
        CAutoRefPtr<IRegion> rgn;
        GETRENDERFACTORY->CreateRegion(&rgn);
        rgn->CombineRect(rcRT,RGN_COPY);

        return GetRenderTarget(gdcFlags,rgn);
    }

    IRenderTarget * SWindow::GetRenderTarget( DWORD gdcFlags,IRegion *pRgn )
    {
        CRect rcClip;
        pRgn->GetRgnBox(&rcClip);
        SWindow *pParent = GetParent();
        while(pParent)
        {
            rcClip.IntersectRect(rcClip,pParent->m_rcWindow);
            pParent = pParent->GetParent();
        }
        
        pRgn->CombineRect(&rcClip,RGN_AND);
        pRgn->GetRgnBox(&rcClip);
        
        //获得最近的一个渲染层的RT
        IRenderTarget *pRT = _GetRenderTarget(rcClip,gdcFlags,pRgn);
        BeforePaintEx(pRT);
        return pRT;
    }

    void SWindow::ReleaseRenderTarget(IRenderTarget *pRT)
    {
        SASSERT(m_pGetRTData);
        _ReleaseRenderTarget(pRT);        
    }
    
    IRenderTarget * SWindow::_GetRenderTarget(CRect & rcGetRT,DWORD gdcFlags,IRegion *pRgn)
    {
        IRenderTarget *pRT = NULL;
        SWindow *pLayerWindow = _GetCurrentLayeredWindow();

        SASSERT(!m_pGetRTData);
        m_pGetRTData = new GETRTDATA;

        m_pGetRTData->gdcFlags = gdcFlags;
        m_pGetRTData->rcRT = rcGetRT;
        m_pGetRTData->rgn = pRgn;

        GetContainer()->BuildWndTreeZorder();

        if(pLayerWindow)
        {
            pRT = pLayerWindow->GetLayerRenderTarget();
        }else
        {
            pLayerWindow = GetRoot();
            pRT = GetContainer()->OnGetRenderTarget(rcGetRT,gdcFlags);
        }
                
        pRT->PushClipRegion(pRgn,RGN_COPY);
        
        if(gdcFlags == OLEDC_PAINTBKGND)
        {//重新绘制当前窗口的背景
            pRT->ClearRect(&rcGetRT,0);
            pLayerWindow->_PaintRegion(pRT,pRgn,ZORDER_MIN,m_uZorder);
        }
        return pRT;
    }

    
    void SWindow::_ReleaseRenderTarget(IRenderTarget *pRT)
    {
        SASSERT(m_pGetRTData);

        SWindow *pRoot = GetRoot();
        SWindow *pLayerWindow = _GetCurrentLayeredWindow();

        if(m_pGetRTData->gdcFlags == OLEDC_PAINTBKGND)
        {//从指定的窗口开始绘制前景
            SWindow * pLayer = pLayerWindow?pLayerWindow:pRoot;
            pLayer->_PaintRegion2(pRT,m_pGetRTData->rgn,m_uZorder+1,ZORDER_MAX);
        }
        pRT->PopClip();//对应_GetRenderTarget中调用的PushClipRegion

        if(pLayerWindow)
        {//存在一个渲染层
            SASSERT(m_pGetRTData);
            if(m_pGetRTData->gdcFlags != OLEDC_NODRAW)
            {
                UINT uFrgndZorderMin = ZORDER_MAX;
                SWindow *pParent = pLayerWindow->GetParent();
                if(pParent)
                {
                    //查找上一个渲染层的前景：向上层查找下一个兄弟，直到找到为止
                    SWindow *pWnd = pLayerWindow;
                    while(pWnd)
                    {
                        SWindow *pNextSibling = pWnd->GetWindow(GSW_NEXTSIBLING);
                        if(pNextSibling)
                        {
                            uFrgndZorderMin = pNextSibling->m_uZorder;
                            break;
                        }else
                        {
                            pWnd = pWnd->GetParent();
                        }
                    }
                }
                
                IRenderTarget *pRTRoot = GetContainer()->OnGetRenderTarget(m_pGetRTData->rcRT,OLEDC_OFFSCREEN);
                pRTRoot->PushClipRegion(m_pGetRTData->rgn);
                pRTRoot->ClearRect(m_pGetRTData->rcRT,0);
                //从root开始绘制当前layer前的窗口背景
                pRoot->_PaintRegion2(pRTRoot,m_pGetRTData->rgn,ZORDER_MIN,pLayerWindow->m_uZorder);
                //将layer的渲染更新到root上
                pRTRoot->AlphaBlend(m_pGetRTData->rcRT,pRT,m_pGetRTData->rcRT,pLayerWindow->m_style.m_byAlpha);
                //绘制当前layer前的窗口前景
                if(uFrgndZorderMin!=ZORDER_MAX) 
                    pRoot->_PaintRegion2(pRTRoot,m_pGetRTData->rgn,uFrgndZorderMin,ZORDER_MAX);
                pRTRoot->PopClip();
                GetContainer()->OnReleaseRenderTarget(pRTRoot,m_pGetRTData->rcRT,OLEDC_OFFSCREEN);
            }
            pRT->PopClip();//对应_GetRenderTarget中调用的PushClipRegion
        }else
        {//不在绘制层
            GetContainer()->OnReleaseRenderTarget(pRT,m_pGetRTData->rcRT,m_pGetRTData->gdcFlags);
        }
        delete m_pGetRTData;
        m_pGetRTData = NULL;
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
        pRT->PushClipRect(&rcDraw,RGN_AND);

        SWindow *pTopWnd=GetRoot();
        CAutoRefPtr<IRegion> pRgn;
        GETRENDERFACTORY->CreateRegion(&pRgn);
        pRgn->CombineRect(&rcDraw,RGN_COPY);

        pRT->ClearRect(&rcDraw,0);//清除残留的alpha值
        
        GetContainer()->BuildWndTreeZorder();
        pTopWnd->_PaintRegion(pRT,pRgn,ZORDER_MIN,m_uZorder);

        pRT->PopClip();
    }

    void SWindow::PaintForeground( IRenderTarget *pRT,LPRECT pRc )
    {
        CRect rcDraw=m_rcWindow;
        if(pRc) rcDraw.IntersectRect(rcDraw,pRc);
        CAutoRefPtr<IRegion> pRgn;
        GETRENDERFACTORY->CreateRegion(&pRgn);
        pRgn->CombineRect(&rcDraw,RGN_COPY);
        pRT->PushClipRect(&rcDraw);
        
        GetContainer()->BuildWndTreeZorder();
        GetTopLevelParent()->_PaintRegion(pRT,pRgn,m_uZorder+1,ZORDER_MAX);

        pRT->PopClip();
    }

    void SWindow::DrawAniStep( CRect rcFore,CRect rcBack,IRenderTarget * pRTFore,IRenderTarget * pRTBack,CPoint ptAnchor)
    {
        IRenderTarget * pRT=GetRenderTarget(rcBack,OLEDC_OFFSCREEN);
        pRT->BitBlt(&rcBack,pRTBack,rcBack.left,rcBack.top,SRCCOPY);
        pRT->BitBlt(&rcFore,pRTFore,ptAnchor.x,ptAnchor.y,SRCCOPY);
        PaintForeground(pRT,rcBack);//画前景
        ReleaseRenderTarget(pRT);
    }

    void SWindow::DrawAniStep( CRect rcWnd,IRenderTarget * pRTFore,IRenderTarget * pRTBack,BYTE byAlpha)
    {
        IRenderTarget * pRT=GetRenderTarget(rcWnd,OLEDC_OFFSCREEN);
        if(byAlpha>0 && byAlpha<255)
        {
            pRT->BitBlt(&rcWnd,pRTBack,rcWnd.left,rcWnd.top,SRCCOPY);
            pRT->AlphaBlend(&rcWnd,pRTFore,&rcWnd,byAlpha);
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
        GetWindowRect(&rcWnd);

        CAutoRefPtr<IRegion> rgn;
        GETRENDERFACTORY->CreateRegion(&rgn);
        rgn->CombineRect(&rcWnd,RGN_COPY);

        IRenderTarget *pRT=GetRenderTarget(rcWnd,OLEDC_NODRAW);
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
        if(!m_layout.InitPosFromString(strValue)) return E_FAIL;
        if(!bLoading && GetParent())
        {
            GetParent()->UpdateChildrenPosition();
        }
        return S_FALSE;
    }

    HRESULT SWindow::OnAttrOffset(const SStringW& strValue, BOOL bLoading)
    {
        if (strValue.IsEmpty()) return E_FAIL;
        if(!m_layout.InitOffsetFromString(strValue)) return E_FAIL;

        if(!bLoading && GetParent())
        {
            GetParent()->UpdateChildrenPosition();
        }
        return S_FALSE;
    }


    HRESULT SWindow::OnAttrPos2type(const SStringW& strValue, BOOL bLoading)
    {
        if(!m_layout.InitOffsetFromPos2Type(strValue)) return E_FAIL;

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
        if(!bLoading && m_layout.IsFitContent(PD_ALL))
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
            if(m_layout.IsFitContent(PD_ALL))
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

    HRESULT SWindow::OnAttrTrackMouseEvent( const SStringW& strValue, BOOL bLoading )
    {
        m_style.m_bTrackMouseEvent = strValue==L"0"?0:1;
        if(!bLoading)
        {
            if(m_style.m_bTrackMouseEvent)
                GetContainer()->RegisterTrackMouseEvent(m_swnd);
            else
                GetContainer()->UnregisterTrackMouseEvent(m_swnd);
        }
        return S_FALSE;
    }

    void SWindow::OnSize( UINT nType, CSize size )
    {
        if(IsDrawToCache())
        {
            if(!m_cachedRT)
            {
                GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT,m_rcWindow.Width(),m_rcWindow.Height());
            }else
            {
                m_cachedRT->Resize(m_rcWindow.Size());
            }
            m_cachedRT->SetViewportOrg(-m_rcWindow.TopLeft());

            MarkCacheDirty(true);
        }
        if(IsLayeredWindow())
        {
            if(!m_layeredRT)
            {
                GETRENDERFACTORY->CreateRenderTarget(&m_layeredRT,m_rcWindow.Width(),m_rcWindow.Height());
            }else
            {
                m_layeredRT->Resize(m_rcWindow.Size());
            }
            m_layeredRT->SetViewportOrg(-m_rcWindow.TopLeft());
        }
    }

    void SWindow::UpdateCacheMode()
    {
        if(IsDrawToCache() && !m_cachedRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT,m_rcWindow.Width(),m_rcWindow.Height());
            m_cachedRT->SetViewportOrg(-m_rcWindow.TopLeft());
            MarkCacheDirty(true);
        }
        if(!IsDrawToCache() && m_cachedRT)
        {
            m_cachedRT=NULL;
        }
    }

    HRESULT SWindow::OnAttrCache( const SStringW& strValue, BOOL bLoading )
    {
        m_bCacheDraw = strValue != L"0";

        if(!bLoading)
        {
            UpdateCacheMode();
            InvalidateRect(NULL);
        }
        return S_FALSE;
    }

    HRESULT SWindow::OnAttrAlpha( const SStringW& strValue, BOOL bLoading )
    {
        m_style.m_byAlpha = _wtoi(strValue);
        if(!bLoading)
        {
            if(!IsLayeredWindow()) UpdateCacheMode();
            InvalidateRect(NULL);
        }
        return bLoading?S_FALSE:S_OK;
    }

    void SWindow::UpdateLayeredWindowMode()
    {
        if(IsLayeredWindow() && !m_layeredRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_layeredRT,m_rcWindow.Width(),m_rcWindow.Height());
            m_layeredRT->SetViewportOrg(-m_rcWindow.TopLeft());
        }
        if(!IsLayeredWindow() && m_layeredRT)
        {
            m_layeredRT=NULL;
        }

    }

    HRESULT SWindow::OnAttrLayeredWindow( const SStringW& strValue, BOOL bLoading )
    {
        m_bLayeredWindow = strValue!=L"0";
        if(!bLoading)
        {
            UpdateLayeredWindowMode();
        }
        return bLoading?S_FALSE:S_OK;
    }

    HRESULT SWindow::OnAttrID( const SStringW& strValue, BOOL bLoading )
    {
        if(!strValue.IsEmpty())
        {
            if(strValue[0]==L'#')//#123
            {
                m_nID = STR2ID(strValue.Right(strValue.GetLength()-1));
            }else if(strValue.Left(2) == L"ID")
            {
                if(strValue == L"IDOK")
                {
                    m_nID = IDOK;
                }else if(strValue == L"IDCANCEL")
                {
                    m_nID = IDCANCEL;
                }else if(strValue == L"IDCLOSE")
                {
                    m_nID = IDCLOSE;
                }else if(strValue == L"IDHELP")
                {
                    m_nID = IDHELP;
                }else if(strValue == L"IDCLOSE")
                {
                    m_nID = IDCLOSE;
                }else if(strValue == L"IDYES")
                {
                    m_nID = IDYES;
                }else if(strValue == L"IDNO")
                {
                    m_nID = IDNO;
                }else if(strValue == L"IDRETRY")
                {
                    m_nID = IDRETRY;
                }else if(strValue == L"IDIGNORE")
                {
                    m_nID = IDIGNORE;
                }
            }else
            {
                m_nID = _wtoi(strValue);
            }
        }
        return S_FALSE;
    }

    SWindow * SWindow::GetSelectedChildInGroup()
    {
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        if(!pChild || !pChild->IsSiblingsAutoGroupped()) return NULL;
        return pChild->GetSelectedSiblingInGroup();
    }

    void SWindow::_Update()
    {
        SASSERT(!m_style.m_bBkgndBlend);

        if(!m_style.m_bBkgndBlend && m_invalidRegion && !m_invalidRegion->IsEmpty()) 
        {
            if(m_invalidRegion)
            {
                //刷新非背景混合的窗口
                CRect rcDirty;
                m_invalidRegion->GetRgnBox(&rcDirty);
                CAutoRefPtr<IRegion> tmpRegin = m_invalidRegion;
                m_invalidRegion = NULL;

                if(IsVisible(TRUE))
                {//可能已经不可见了。
                    IRenderTarget *pRT = GetRenderTarget(rcDirty,OLEDC_OFFSCREEN);

                    pRT->PushClipRegion(tmpRegin,RGN_AND);
                    SSendMessage(WM_ERASEBKGND, (WPARAM)pRT);
                    SSendMessage(WM_PAINT, (WPARAM)pRT);
                    PaintForeground(pRT,rcDirty);//画前景
                    pRT->PopClip();

                    ReleaseRenderTarget(pRT);
                }
            }
        }
    }

    const SwndLayout * SWindow::GetLayout() const
    {
        return &m_layout;
    }

    IRenderTarget * SWindow::GetCachedRenderTarget()
    {
        SASSERT(IsDrawToCache());
        if(!m_cachedRT) GETRENDERFACTORY->CreateRenderTarget(&m_cachedRT,0,0);
        return m_cachedRT;
    }
    
    bool SWindow::IsDrawToCache() const
    {
        return m_bCacheDraw || (!m_bLayeredWindow && m_style.m_byAlpha!=0xff);
    }
    
    IRenderTarget * SWindow::GetLayerRenderTarget()
    {
        SASSERT(IsLayeredWindow());
        if(!m_layeredRT)  GETRENDERFACTORY->CreateRenderTarget(&m_layeredRT,0,0);
        return m_layeredRT;
    }

    void SWindow::OnStateChanging( DWORD dwOldState,DWORD dwNewState )
    {
        
    }

    void SWindow::OnStateChanged( DWORD dwOldState,DWORD dwNewState )
    {
        EventStateChanged evt(this);
        evt.dwOldState = dwOldState;
        evt.dwNewState = dwNewState;
        FireEvent(evt);
    }
}//namespace SOUI
