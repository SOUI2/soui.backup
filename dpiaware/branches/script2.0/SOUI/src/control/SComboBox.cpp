#include "souistd.h"
#include "control/SComboBox.h"


namespace SOUI
{

// CComboEdit
SComboEdit::SComboEdit( SComboBoxBase *pOwner )
{
    SetOwner(pOwner);
}

void SComboEdit::OnMouseHover( WPARAM wParam, CPoint ptPos )
{
    __super::OnMouseHover(wParam,ptPos);
    GetOwner()->SSendMessage(WM_MOUSEHOVER,wParam,MAKELPARAM(ptPos.x,ptPos.y));
}

void SComboEdit::OnMouseLeave()
{
    __super::OnMouseLeave();
    GetOwner()->SSendMessage(WM_MOUSELEAVE);
}

void SComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_DOWN || nChar == VK_ESCAPE))
    {
        pOwner->SSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    SetMsgHandled(FALSE);
}

BOOL SComboEdit::FireEvent(EventArgs & evt)
{
    if(evt.GetEventID()==EVT_RE_NOTIFY)
    {//转发richedit的txNotify消息
        evt.idFrom=GetOwner()->GetID();
        evt.nameFrom=GetOwner()->GetName();
    }
    return SEdit::FireEvent(evt);
}

//////////////////////////////////////////////////////////////////////////
// SDropDownWnd_ComboBox
BOOL SDropDownWnd_ComboBox::PreTranslateMessage( MSG* pMsg )
{
    SDropDownWnd::PreTranslateMessage(pMsg);

    if(pMsg->message==WM_MOUSEWHEEL 
    || ((pMsg->message == WM_KEYDOWN || pMsg->message==WM_KEYUP) && (pMsg->wParam == VK_UP || pMsg->wParam==VK_DOWN || pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)))
    {//截获滚轮及上下键消息
        CSimpleWnd::SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
        return TRUE;    
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// SComboBoxBase
SComboBoxBase::SComboBoxBase(void)
:m_pSkinBtn(GETBUILTINSKIN(SKIN_SYS_DROPBTN))
,m_pEdit(NULL)
,m_bDropdown(TRUE)
,m_nDropHeight(200)
,m_dwBtnState(WndState_Normal)
,m_iAnimTime(200)
,m_pDropDownWnd(NULL)
,m_iInitSel(-1)
{
    m_bFocusable=TRUE;
    m_style.SetAttribute(L"align",L"left",TRUE);
    m_style.SetAttribute(L"valign",L"middle",TRUE);

    m_evtSet.addEvent(EVENTID(EventCBSelChange));
    m_evtSet.addEvent(EVENTID(EventRENotify));
}

SComboBoxBase::~SComboBoxBase(void)
{
}


BOOL SComboBoxBase::CreateChildren( pugi::xml_node xmlNode )
{
    SASSERT(m_pSkinBtn);
    //创建edit对象
    if(!m_bDropdown)
    {
        SIZE szBtn=m_pSkinBtn->GetSkinSize();
        m_pEdit=new SComboEdit(this);
        SApplication::getSingleton().SetSwndDefAttr(m_pEdit);
        
        InsertChild(m_pEdit);
        pugi::xml_node xmlEditStyle=xmlNode.child(L"editstyle");
        m_pEdit->GetEventSet()->setMutedState(true);
        if(xmlEditStyle)
            m_pEdit->InitFromXml(xmlEditStyle);
        else
            m_pEdit->SSendMessage(WM_CREATE);
        m_pEdit->GetEventSet()->setMutedState(false);
        SStringW strPos;
        strPos.Format(L"0,0,-%d,-0",szBtn.cx);
        m_pEdit->SetAttribute(L"pos",strPos,TRUE);
        m_pEdit->SetID(IDC_CB_EDIT);
        m_pEdit->SSendMessage(EM_SETEVENTMASK,0 ,ENM_CHANGE );
        
    }
    return CreateListBox(xmlNode);
}


void SComboBoxBase::GetDropBtnRect(LPRECT prc)
{
    SIZE szBtn=m_pSkinBtn->GetSkinSize();
    GetClientRect(prc);
    prc->left=prc->right-szBtn.cx;
}

void SComboBoxBase::GetTextRect( LPRECT pRect )
{
    GetClientRect(pRect);
    SIZE szBtn=m_pSkinBtn->GetSkinSize();
    pRect->right-=szBtn.cx;
}

void SComboBoxBase::OnPaint(IRenderTarget * pRT )
{
    SPainter painter;

    BeforePaint(pRT, painter);
    if(GetCurSel() != -1 && m_pEdit==NULL)
    {
        CRect rcText;
        GetTextRect(rcText);
        SStringT strText=GetWindowText();
        DrawText(pRT,strText, strText.GetLength(), rcText, GetTextAlign());
    }
    //draw focus rect
    if(GetContainer()->SwndGetFocus()==m_swnd)
    {
        DrawFocus(pRT);
    }
    AfterPaint(pRT, painter);
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    m_pSkinBtn->Draw(pRT,rcBtn,IIF_STATE3(m_dwBtnState,WndState_Normal,WndState_Hover,WndState_PushDown));
}

void SComboBoxBase::OnLButtonDown( UINT nFlags,CPoint pt )
{
    SetFocus();
    DropDown();
}

void SComboBoxBase::OnMouseMove( UINT nFlags,CPoint pt )
{
    if(m_dwBtnState==WndState_PushDown) return;

    __super::OnMouseHover(nFlags,pt);
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    if(rcBtn.PtInRect(pt))
    {
        m_dwBtnState=WndState_Hover;
        InvalidateRect(rcBtn);
    }else if(m_dwBtnState==WndState_Hover)
    {
        m_dwBtnState=WndState_Normal;
        InvalidateRect(rcBtn);
    }
}

void SComboBoxBase::OnMouseLeave()
{
    if(m_dwBtnState==WndState_PushDown) return;

    if(GetState()&WndState_Hover) 
        __super::OnMouseLeave();
    if(m_dwBtnState==WndState_Hover)
    {
        m_dwBtnState=WndState_Normal;
        CRect rcBtn;
        GetDropBtnRect(&rcBtn);
        InvalidateRect(rcBtn);
    }
}

void SComboBoxBase::OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
{    
    if ( nChar == VK_DOWN)
        DropDown();
}

void SComboBoxBase::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_bDropdown)
    {
        SComboEdit *pEdit = static_cast<SComboEdit *>(FindChildByID(IDC_CB_EDIT));
        if (pEdit)
            pEdit->SSendMessage(WM_CHAR, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }
}

UINT SComboBoxBase::OnGetDlgCode()
{        
    return SC_WANTARROWS;
}

BOOL SComboBoxBase::IsFocusable()
{
    if (m_bDropdown && m_bFocusable)
        return TRUE;
    return FALSE;
}


SWindow* SComboBoxBase::GetDropDownOwner()
{
    return this;
}


void SComboBoxBase::OnDropDown( SDropDownWnd *pDropDown )
{
    m_dwBtnState=WndState_PushDown;
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    InvalidateRect(rcBtn);
    ((CSimpleWnd*)pDropDown)->SetCapture();
}

void SComboBoxBase::OnCloseUp(SDropDownWnd *pDropDown,UINT uCode)
{
    if (!m_bDropdown && m_pEdit)
    {
        m_pEdit->SetFocus();
        m_pEdit->SetSel(MAKELONG(0,-1));
    }

    m_dwBtnState = WndState_Normal;
    m_pDropDownWnd=NULL;
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    InvalidateRect(rcBtn);
    ModifyState(0,WndState_Hover,TRUE);
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(GetContainer()->GetHostHwnd(),&pt);
    ::PostMessage(GetContainer()->GetHostHwnd(),WM_MOUSEMOVE,0,MAKELPARAM(pt.x,pt.y));

    if(uCode==IDOK)
    {
        OnSelChanged();
    }

}

BOOL SComboBoxBase::CalcPopupRect( int nHeight,CRect & rcPopup )
{
    CRect rcWnd;
    GetWindowRect(&rcWnd);
    ClientToScreen(GetContainer()->GetHostHwnd(),(LPPOINT)&rcWnd);
    ClientToScreen(GetContainer()->GetHostHwnd(),((LPPOINT)&rcWnd)+1);

    HMONITOR hMonitor = ::MonitorFromWindow(GetContainer()->GetHostHwnd(), MONITOR_DEFAULTTONULL);
    CRect rcMonitor;
    if (hMonitor)
    {
        MONITORINFO mi = {sizeof(MONITORINFO)};
        ::GetMonitorInfo(hMonitor, &mi);
        rcMonitor = mi.rcMonitor;
    }
    else
    {
        rcMonitor.right   =   GetSystemMetrics(   SM_CXSCREEN   );   
        rcMonitor.bottom  =   GetSystemMetrics(   SM_CYSCREEN   );
    }
    if(rcWnd.bottom+nHeight<=rcMonitor.bottom)
    {
        rcPopup = CRect(rcWnd.left,rcWnd.bottom,rcWnd.right,rcWnd.bottom+nHeight);
        return TRUE;
    }else
    {
        rcPopup = CRect(rcWnd.left,rcWnd.top-nHeight,rcWnd.right,rcWnd.top);
        return FALSE;
    }
}


void SComboBoxBase::DropDown()
{
    if(m_dwBtnState==WndState_PushDown) return;

    if(!m_pDropDownWnd)
    {
        m_pDropDownWnd = new SDropDownWnd_ComboBox(this);
        CRect rcPopup;
        BOOL bDown=CalcPopupRect(GetListBoxHeight(),rcPopup);
        m_pDropDownWnd->Create(rcPopup,0);
        m_pDropDownWnd->AnimateHostWindow(m_iAnimTime,AW_SLIDE|(bDown?AW_VER_POSITIVE:AW_VER_NEGATIVE));
    }
}

void SComboBoxBase::CloseUp()
{
    if(m_pDropDownWnd)
    {
        m_pDropDownWnd->EndDropDown(IDCANCEL);
    }
}


void SComboBoxBase::OnSetFocus()
{
    if(m_pEdit) 
        m_pEdit->SetFocus();
    else
        __super::OnSetFocus();
}


void SComboBoxBase::OnDestroy()
{
    CloseUp();
    __super::OnDestroy();
}

void SComboBoxBase::OnSelChanged()
{
    EventCBSelChange evt(this,GetCurSel());
    FireEvent(evt);
}

BOOL SComboBoxBase::FireEvent( EventArgs &evt )
{
    if(evt.GetEventID() == EventRENotify::EventID)
    {
        EventRENotify *evtRe = (EventRENotify*)&evt;
        if(evtRe->iNotify == EN_CHANGE && !m_pEdit->GetEventSet()->isMuted())
        {
            m_pEdit->GetEventSet()->setMutedState(true);
            SetCurSel(-1);
            m_pEdit->GetEventSet()->setMutedState(false);
        }
    }
    return SWindow::FireEvent(evt);
}

int SComboBoxBase::FindString( LPCTSTR pszFind,int nAfter/*=0*/ )
{
    for(int i=nAfter;i<GetCount();i++)
    {
        SStringT strItem = GetLBText(i);
        if(strItem == pszFind) return i;
    }
    return -1;
}

SStringT SComboBoxBase::GetWindowText()
{
    if(!m_bDropdown)
    {
        return GetEditText();
    }
    if(GetCurSel()==-1) return _T("");
    return GetLBText(GetCurSel());
}

//////////////////////////////////////////////////////////////////////////
SComboBox::SComboBox()
:m_pListBox(NULL)
{

}

SComboBox::~SComboBox()
{
    if(m_pListBox)
    {
        m_pListBox->SSendMessage(WM_DESTROY);
        delete m_pListBox;
    }
}

BOOL SComboBox::CreateListBox( pugi::xml_node xmlNode )
{
    SASSERT(xmlNode);
    //创建列表控件
    m_pListBox=(SListBox*)SApplication::getSingleton().CreateWindowByName(SListBox::GetClassName());
    m_pListBox->SetContainer(GetContainer());

    m_pListBox->InitFromXml(xmlNode.child(L"liststyle"));
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetAttribute(L"hotTrack",L"1",TRUE);
    m_pListBox->SetOwner(this);    //chain notify message to combobox
    m_pListBox->SetID(IDC_DROPDOWN_LIST);

    //初始化列表数据
    pugi::xml_node xmlNode_Items=xmlNode.child(L"items");
    if(xmlNode_Items)
    {
        pugi::xml_node xmlNode_Item=xmlNode_Items.child(L"item");
        while(xmlNode_Item)
        {
            SStringT strText=S_CW2T(xmlNode_Item.attribute(L"text").value());
            int iIcon=xmlNode_Item.attribute(L"icon").as_int(0);
            LPARAM lParam=xmlNode_Item.attribute(L"data").as_int(0);
            m_pListBox->AddString(strText,iIcon,lParam);
            xmlNode_Item=xmlNode_Item.next_sibling(L"item");
        }
    }

    if(m_iInitSel!=-1)
    {
        SetCurSel(m_iInitSel);
    }
    return TRUE;
}

int SComboBox::GetListBoxHeight()
{
    int nDropHeight=m_nDropHeight;
    if(GetCount()) 
    {
        int nItemHeight=m_pListBox->GetItemHeight();
        nDropHeight = min(nDropHeight,nItemHeight*GetCount()+m_pListBox->GetStyle().m_nMarginY*2);
    }
    return nDropHeight;    
}

void SComboBox::OnDropDown( SDropDownWnd *pDropDown)
{
    __super::OnDropDown(pDropDown);
    pDropDown->InsertChild(m_pListBox);
    pDropDown->UpdateChildrenPosition();

    m_pListBox->SetVisible(TRUE);
    m_pListBox->SetFocus();
    m_pListBox->EnsureVisible(GetCurSel());
}

void SComboBox::OnCloseUp( SDropDownWnd *pDropDown ,UINT uCode)
{
    pDropDown->RemoveChild(m_pListBox);
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetContainer(GetContainer());
    __super::OnCloseUp(pDropDown,uCode);
}

void SComboBox::OnSelChanged()
{
    m_pListBox->GetCurSel();
    if(m_pEdit && !m_pEdit->GetEventSet()->isMuted())
    {
        SStringT strText=GetLBText(m_pListBox->GetCurSel());
        m_pEdit->GetEventSet()->setMutedState(true);
        m_pEdit->SetWindowText(strText);
        m_pEdit->GetEventSet()->setMutedState(false);
    }
    Invalidate();
    __super::OnSelChanged();
}

BOOL SComboBox::FireEvent( EventArgs &evt )
{
    if(evt.idFrom == IDC_DROPDOWN_LIST && m_pDropDownWnd)
    {
        if(evt.GetEventID()==EventLBSelChanged::EventID)
        {
            OnSelChanged();
            return TRUE;
        }
        if(evt.GetEventID() == EventCmd::EventID)
        {
            CloseUp();
            return TRUE;
        }
    }
    return SComboBoxBase::FireEvent(evt);
}

//////////////////////////////////////////////////////////////////////////

SComboBoxEx::SComboBoxEx():m_uTxtID(0),m_uIconID(0),m_pListBox(NULL)
{
    m_evtSet.addEvent(EVENTID(EventOfComoboxExItem));
}

SComboBoxEx::~SComboBoxEx()
{
    if(m_pListBox)
    {
        m_pListBox->SSendMessage(WM_DESTROY);
        delete m_pListBox;
    }
}

BOOL SComboBoxEx::CreateListBox( pugi::xml_node xmlNode )
{
    SASSERT(xmlNode);
    //创建列表控件
    m_pListBox=(SListBoxEx*)SApplication::getSingleton().CreateWindowByName(SListBoxEx::GetClassName());
    m_pListBox->SetContainer(GetContainer());

    m_pListBox->InitFromXml(xmlNode.child(L"liststyle"));
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetAttribute(L"hotTrack",L"1",TRUE);
    m_pListBox->SetOwner(this);         //chain notify message to combobox
    m_pListBox->SetID(IDC_DROPDOWN_LIST);

    //初始化列表数据
    pugi::xml_node xmlNode_Items=xmlNode.child(L"items");
    if(xmlNode_Items)
    {
        int nItems=0;
        pugi::xml_node xmlNode_Item=xmlNode_Items.child(L"item");
        while(xmlNode_Item)
        {
            nItems++;
            xmlNode_Item=xmlNode_Item.next_sibling(L"item");
        }

        m_pListBox->SetItemCount(nItems);
        int iItem=0;
        xmlNode_Item=xmlNode_Items.child(L"item");
        while(xmlNode_Item)
        {
            LPARAM lParam=xmlNode_Item.attribute(L"data").as_int(0);
            m_pListBox->SetItemData(iItem,lParam);
            SWindow *pWnd=m_pListBox->GetItemPanel(iItem);
            if(m_uTxtID!=0)
            {
                SWindow *pText=pWnd->FindChildByID(m_uTxtID);
                if(pText)
                {
                    SStringT strText=S_CW2T(xmlNode_Item.attribute(L"text").value());
                    pText->SetWindowText(strText);
                }
            }
            if(m_uIconID!=0)
            {
                SImageWnd * pImg = pWnd->FindChildByID2<SImageWnd>(m_uIconID);
                if(pImg) 
                {
                    int iIcon=xmlNode_Item.attribute(L"icon").as_int(0);
                    pImg->SetIcon(iIcon);
                }
            }
            iItem++;
            xmlNode_Item=xmlNode_Item.next_sibling(L"item");
        }
    }

    if(m_iInitSel!=-1)
    {
        SetCurSel(m_iInitSel);
    }
    return TRUE;
}

int SComboBoxEx::GetListBoxHeight()
{
    int nDropHeight=m_nDropHeight;
    if(GetCount()) 
    {
        int nItemHeight=m_pListBox->GetItemHeight();
        nDropHeight = min(nDropHeight,nItemHeight*GetCount()+m_pListBox->GetStyle().m_nMarginY*2);
    }
    return nDropHeight;    
}

void SComboBoxEx::OnDropDown( SDropDownWnd *pDropDown )
{
    __super::OnDropDown(pDropDown);
    pDropDown->InsertChild(m_pListBox);
    pDropDown->UpdateChildrenPosition();

    m_pListBox->SetVisible(TRUE);
    m_pListBox->SetFocus();
    m_pListBox->EnsureVisible(GetCurSel());
}

void SComboBoxEx::OnCloseUp( SDropDownWnd *pDropDown ,UINT uCode)
{
    pDropDown->RemoveChild(m_pListBox);
    m_pListBox->SetVisible(FALSE);
    m_pListBox->SetContainer(GetContainer());
    __super::OnCloseUp(pDropDown,uCode);
}

void SComboBoxEx::OnSelChanged()
{
    int iSel=m_pListBox->GetCurSel();
    if(m_pEdit && !m_pEdit->GetEventSet()->isMuted())
    {
        SStringT strText=GetLBText(iSel);
        m_pEdit->GetEventSet()->setMutedState(true);
        m_pEdit->SetWindowText(strText);
        m_pEdit->GetEventSet()->setMutedState(false);
    }
    Invalidate();
    __super::OnSelChanged();
}

BOOL SComboBoxEx::FireEvent( EventArgs &evt )
{
    if(evt.idFrom == IDC_DROPDOWN_LIST && m_pDropDownWnd)
    {
        if(evt.GetEventID()==EventLBSelChanged::EventID)
        {//列表选中项改变事件
            OnSelChanged();
            return TRUE;
        }
    }
    if(evt.GetEventID() == EventOfPanel::EventID)
    {
        EventOfPanel *pEvtOfPanel = (EventOfPanel*) &evt;
        if(pEvtOfPanel->pOrgEvt->GetEventID() == EventCmd::EventID)
        {
            EventOfComoboxExItem evt2(this,(EventCmd*)pEvtOfPanel->pOrgEvt);
            __super::FireEvent(evt2);
            if(!evt2.bCancel)
            {//可以关闭下拉列表
                CloseUp();
            }
            return TRUE;        
        }
    }
    return SComboBoxBase::FireEvent(evt);
}

SStringT SComboBoxEx::GetLBText( int iItem )
{
    if(m_uTxtID == 0 || iItem<0 || iItem>= GetCount()) return _T("");
    SWindow *pItem=m_pListBox->GetItemPanel(iItem);
    SWindow *pText=pItem->FindChildByID(m_uTxtID);
    if(!pText) return _T("");
    return pText->GetWindowText();
}

}//namespace SOUI

