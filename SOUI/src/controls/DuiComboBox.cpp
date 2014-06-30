#include "duistd.h"
#include "control/DuiComboBox.h"


namespace SOUI
{

// CComboEdit
CComboEdit::CComboEdit( CDuiComboBoxBase *pOwner )
{
    SetOwner(pOwner);
}

void CComboEdit::OnMouseHover( WPARAM wParam, CPoint ptPos )
{
    __super::OnMouseHover(wParam,ptPos);
    GetOwner()->DuiSendMessage(WM_MOUSEHOVER,wParam,MAKELPARAM(ptPos.x,ptPos.y));
}

void CComboEdit::OnMouseLeave()
{
    __super::OnMouseLeave();
    GetOwner()->DuiSendMessage(WM_MOUSELEAVE);
}

void CComboEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    SWindow *pOwner = GetOwner();
    if (pOwner && (nChar == VK_DOWN || nChar == VK_ESCAPE))
    {
        pOwner->DuiSendMessage(WM_KEYDOWN, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }

    SetMsgHandled(FALSE);
}

LRESULT CComboEdit::DuiNotify( LPSNMHDR pnms )
{
    //转发richedit的txNotify消息
    if(pnms->code==NM_RICHEDIT_NOTIFY)
    {
        pnms->idFrom=GetOwner()->GetCmdID();
    }
    return __super::DuiNotify(pnms);
}


//////////////////////////////////////////////////////////////////////////
// CDuiComboBox
CDuiComboBoxBase::CDuiComboBoxBase(void)
:m_pSkinBtn(GETSKIN(L"comboboxbtn"))
,m_pEdit(NULL)
,m_bDropdown(TRUE)
,m_nDropHeight(200)
,m_dwBtnState(DuiWndState_Normal)
,m_iAnimTime(200)
,m_pDropDownWnd(NULL)
,m_iInitSel(-1)
{
    m_bTabStop=TRUE;
    m_style.SetAttribute(L"align",L"left",TRUE);
    m_style.SetAttribute(L"valign",L"middle",TRUE);

    addEvent(NM_CBSELCHANGE);
    addEvent(NM_RICHEDIT_NOTIFY);
}

CDuiComboBoxBase::~CDuiComboBoxBase(void)
{
}


BOOL CDuiComboBoxBase::LoadChildren( pugi::xml_node xmlNode )
{
    pugi::xml_node xmlParent = xmlNode.parent();

    DUIASSERT(m_pSkinBtn);
    //创建edit对象
    if(!m_bDropdown)
    {
        SIZE szBtn=m_pSkinBtn->GetSkinSize();
        m_pEdit=new CComboEdit(this);
        InsertChild(m_pEdit);
        pugi::xml_node xmlEditStyle=xmlParent.child(L"editstyle");
        if(xmlEditStyle)
            m_pEdit->Load(xmlEditStyle);
        else
            m_pEdit->DuiSendMessage(WM_CREATE);
        SStringW strPos;
        strPos.Format(L"0,0,-%d,-0",szBtn.cx);
        m_pEdit->SetAttribute(L"pos",strPos,TRUE);
        m_pEdit->SetCmdID(IDC_CB_EDIT);

    }
    return CreateListBox(xmlNode);
}


void CDuiComboBoxBase::GetDropBtnRect(LPRECT prc)
{
    SIZE szBtn=m_pSkinBtn->GetSkinSize();
    GetClient(prc);
    prc->left=prc->right-szBtn.cx;
}

void CDuiComboBoxBase::GetTextRect( LPRECT pRect )
{
    GetClient(pRect);
    SIZE szBtn=m_pSkinBtn->GetSkinSize();
    pRect->right-=szBtn.cx;
}

void CDuiComboBoxBase::OnPaint(IRenderTarget * pRT )
{
    SPainter painter;

    BeforePaint(pRT, painter);
    if(GetCurSel() != -1 && m_pEdit==NULL)
    {
        CRect rcText;
        GetTextRect(rcText);
        SStringT strText=GetWindowText();
        DuiDrawText(pRT,strText, strText.GetLength(), rcText, GetTextAlign());
    }
    //draw focus rect
    if(GetContainer()->GetDuiFocus()==m_hSWnd)
    {
        DuiDrawFocus(pRT);
    }
    AfterPaint(pRT, painter);
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    m_pSkinBtn->Draw(pRT,rcBtn,IIF_STATE3(m_dwBtnState,DuiWndState_Normal,DuiWndState_Hover,DuiWndState_PushDown));
}

void CDuiComboBoxBase::OnLButtonDown( UINT nFlags,CPoint pt )
{
    SetDuiFocus();
    DropDown();
}

void CDuiComboBoxBase::OnMouseMove( UINT nFlags,CPoint pt )
{
    if(m_dwBtnState==DuiWndState_PushDown) return;

    __super::OnMouseHover(nFlags,pt);
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    if(rcBtn.PtInRect(pt))
    {
        m_dwBtnState=DuiWndState_Hover;
        NotifyInvalidateRect(rcBtn);
    }else if(m_dwBtnState==DuiWndState_Hover)
    {
        m_dwBtnState=DuiWndState_Normal;
        NotifyInvalidateRect(rcBtn);
    }
}

void CDuiComboBoxBase::OnMouseLeave()
{
    if(m_dwBtnState==DuiWndState_PushDown) return;

    if(GetState()&DuiWndState_Hover) 
        __super::OnMouseLeave();
    if(m_dwBtnState==DuiWndState_Hover)
    {
        m_dwBtnState=DuiWndState_Normal;
        CRect rcBtn;
        GetDropBtnRect(&rcBtn);
        NotifyInvalidateRect(rcBtn);
    }
}

void CDuiComboBoxBase::OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags )
{    
    if ( nChar == VK_DOWN)
        DropDown();
}

void CDuiComboBoxBase::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!m_bDropdown)
    {
        CComboEdit *pEdit = static_cast<CComboEdit *>(FindChildByCmdID(IDC_CB_EDIT));
        if (pEdit)
            pEdit->DuiSendMessage(WM_CHAR, nChar, MAKELONG(nFlags, nRepCnt));
        return;
    }
}

UINT CDuiComboBoxBase::OnGetDuiCode()
{        
    return DUIC_WANTARROWS;
}

BOOL CDuiComboBoxBase::IsTabStop()
{
    if (m_bDropdown && m_bTabStop)
        return TRUE;
    return FALSE;
}


SWindow* CDuiComboBoxBase::GetDropDownOwner()
{
    return this;
}


void CDuiComboBoxBase::OnDropDown( SDropDownWnd *pDropDown )
{
    m_dwBtnState=DuiWndState_PushDown;
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    NotifyInvalidateRect(rcBtn);
    pDropDown->SetCapture();
}

void CDuiComboBoxBase::OnCloseUp(SDropDownWnd *pDropDown,UINT uCode)
{
    ReleaseCapture();

    if (!m_bDropdown && m_pEdit)
    {
        m_pEdit->SetDuiFocus();
        m_pEdit->SetSel(MAKELONG(0,-1));
    }

    m_dwBtnState = DuiWndState_Normal;
    m_pDropDownWnd=NULL;
    CRect rcBtn;
    GetDropBtnRect(&rcBtn);
    NotifyInvalidateRect(rcBtn);
    ModifyState(0,DuiWndState_Hover,TRUE);
    CPoint pt;
    GetCursorPos(&pt);
    ScreenToClient(GetContainer()->GetHostHwnd(),&pt);
    ::PostMessage(GetContainer()->GetHostHwnd(),WM_MOUSEMOVE,0,MAKELPARAM(pt.x,pt.y));

    if(uCode==IDOK)
    {
        OnSelChanged();
    }

}

BOOL CDuiComboBoxBase::CalcPopupRect( int nHeight,CRect & rcPopup )
{
    CRect rcWnd;
    GetRect(&rcWnd);
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


void CDuiComboBoxBase::DropDown()
{
    if(m_dwBtnState==DuiWndState_PushDown) return;

    if(!m_pDropDownWnd)
    {
        m_pDropDownWnd = new SDropDownWnd(this);
        CRect rcPopup;
        BOOL bDown=CalcPopupRect(GetListBoxHeight(),rcPopup);
        m_pDropDownWnd->Create(rcPopup,0);
        m_pDropDownWnd->AnimateHostWindow(m_iAnimTime,AW_SLIDE|(bDown?AW_VER_POSITIVE:AW_VER_NEGATIVE));
    }
}

void CDuiComboBoxBase::CloseUp()
{
    if(m_pDropDownWnd)
    {
        m_pDropDownWnd->EndDropDown(IDCANCEL);
    }
}

void CDuiComboBoxBase::OnDestroy()
{
    CloseUp();
    __super::OnDestroy();
}

LRESULT CDuiComboBoxBase::DuiNotify( LPSNMHDR pnms )
{
    if(pnms->idFrom == IDC_DROPDOWN_LIST)
    {
        DUIASSERT(m_pDropDownWnd);
        const MSG *pMsg=m_pDropDownWnd->GetCurrentMessage();
        if(pnms->code==NM_LBSELCHANGED)
        {
            OnSelChanged();
            if(pMsg->message != WM_KEYDOWN)
                CloseUp();
            return 0;
        }
    }
    return __super::DuiNotify(pnms);
}

void CDuiComboBoxBase::OnSelChanged()
{
    DUINMHDR nms;
    nms.code=NM_CBSELCHANGE;
    nms.hDuiWnd=m_hSWnd;
    nms.idFrom=GetCmdID();
    nms.pszNameFrom=GetName();
    DuiNotify(&nms);
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
        m_pListBox->DuiSendMessage(WM_DESTROY);
        delete m_pListBox;
    }
}

BOOL SComboBox::CreateListBox( pugi::xml_node xmlNode )
{
    DUIASSERT(xmlNode);
    //创建列表控件
    m_pListBox=new SListBox;
    m_pListBox->SetContainer(GetContainer());

    m_pListBox->Load(xmlNode.parent().child(L"liststyle"));
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetAttribute(L"hottrack",L"1",TRUE);
    m_pListBox->SetOwner(this);    //chain notify message to combobox
    m_pListBox->SetCmdID(IDC_DROPDOWN_LIST);

    //初始化列表数据
    pugi::xml_node xmlNode_Items=xmlNode.parent().child(L"items");
    if(xmlNode_Items)
    {
        pugi::xml_node xmlNode_Item=xmlNode_Items.child(L"item");
        while(xmlNode_Item)
        {
            SStringT strText=DUI_CW2T(xmlNode_Item.attribute(L"text").value());
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
    m_pListBox->SetDuiFocus();
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
    int nRet=m_pListBox->GetCurSel();
    if(m_pEdit)
    {
        SStringT strText=GetLBText(m_pListBox->GetCurSel());
        m_pEdit->setMutedState(true);
        m_pEdit->SetWindowText(DUI_CT2W(strText));
        m_pEdit->setMutedState(false);
    }
    NotifyInvalidate();
    __super::OnSelChanged();
}

//////////////////////////////////////////////////////////////////////////

SComboBoxEx::SComboBoxEx():m_uTxtID(0),m_uIconID(0),m_pListBox(NULL)
{

}

SComboBoxEx::~SComboBoxEx()
{
    if(m_pListBox)
    {
        m_pListBox->DuiSendMessage(WM_DESTROY);
        delete m_pListBox;
    }
}

BOOL SComboBoxEx::CreateListBox( pugi::xml_node xmlNode )
{
    DUIASSERT(xmlNode);
    //创建列表控件
    m_pListBox=new SListBoxEx;
    m_pListBox->SetContainer(GetContainer());

    m_pListBox->Load(xmlNode.parent().child(L"liststyle"));
    m_pListBox->SetAttribute(L"pos", L"0,0,-0,-0", TRUE);
    m_pListBox->SetAttribute(L"hottrack",L"1",TRUE);
    m_pListBox->SetOwner(this);    //chain notify message to combobox
    m_pListBox->SetCmdID(IDC_DROPDOWN_LIST);

    //初始化列表数据
    pugi::xml_node xmlNode_Items=xmlNode.parent().child(L"items");
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
                SWindow *pText=pWnd->FindChildByCmdID(m_uTxtID);
                if(pText)
                {
                    SStringT strText=DUI_CW2T(xmlNode_Item.attribute(L"text").value());
                    pText->SetInnerText(strText);
                }
            }
            if(m_uIconID!=0)
            {
                SImageWnd * pImg = pWnd->FindChildByCmdID2<SImageWnd *>(m_uIconID);
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
    m_pListBox->SetDuiFocus();
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
    if(m_pEdit)
    {
        SStringT strText=GetLBText(iSel);
        m_pEdit->setMutedState(true);
        m_pEdit->SetWindowText(DUI_CT2W(strText));
        m_pEdit->setMutedState(false);
    }
    NotifyInvalidate();
    __super::OnSelChanged();
}


}//namespace SOUI

