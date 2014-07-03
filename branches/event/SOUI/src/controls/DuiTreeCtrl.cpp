//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiTreeCtrl
// Description: CDuiTreeCtrl
//     Creator: JinHui
//     Version: 2012.12.16 - 1.1 - Create
//////////////////////////////////////////////////////////////////////////

#include "duistd.h"
#include "control/DuiTreeCtrl.h"

namespace SOUI{

STreeCtrl::STreeCtrl()
: m_nItemHei(20)
, m_nIndent(16)
, m_nItemMargin(4)
, m_hSelItem(NULL)
, m_hHoverItem(NULL)
, m_hCaptureItem(NULL)
, m_pItemBgSkin(NULL)
, m_pItemSelSkin(NULL)
, m_pIconSkin(NULL)
, m_pToggleSkin(NULL)
, m_pCheckSkin(NULL)
, m_crItemBg(RGB(255,255,255))
, m_crItemSelBg(RGB(0,0,136))
, m_crItemText(RGB(0,0,0))
, m_crItemSelText(RGB(255,255,255))
, m_nVisibleItems(0)
, m_nMaxItemWidth(0)
, m_bCheckBox(FALSE)
, m_bRightClickSel(FALSE)
, m_uItemMask(0)
, m_nItemOffset(0)
, m_nItemHoverBtn(DuiTVIBtn_None)
, m_nItemPushDownBtn(DuiTVIBtn_None)
{
    m_bClipClient = TRUE;
}

STreeCtrl::~STreeCtrl()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

HSTREEITEM STreeCtrl::InsertItem(LPCTSTR lpszItem, HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    return InsertItem(lpszItem, -1, -1, NULL,  hParent, hInsertAfter);
}

HSTREEITEM STreeCtrl::InsertItem(LPCTSTR lpszItem, int nImage,
        int nSelectedImage, HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    return InsertItem(lpszItem, nImage, nSelectedImage, NULL,  hParent, hInsertAfter);
}

HSTREEITEM STreeCtrl::InsertItem(LPCTSTR lpszItem, int nImage,
    int nSelectedImage, LPARAM lParam,
    HSTREEITEM hParent, HSTREEITEM hInsertAfter)
{
    LPTVITEM pItemObj = new TVITEM();

    pItemObj->strText = lpszItem;
    pItemObj->nImage = nImage;
    pItemObj->nSelectedImage  = nSelectedImage;
    pItemObj->lParam = lParam;

    return InsertItem(pItemObj, hParent, hInsertAfter, TRUE);
}

BOOL STreeCtrl::RemoveItem(HSTREEITEM hItem)
{    
    if(!hItem) return FALSE;
    HSTREEITEM hParent=GetParentItem(hItem);

    LPTVITEM pItem= CSTree<LPTVITEM>::GetItem(hItem);

    BOOL bVisible=pItem->bVisible;
    int nItemWidth =GetMaxItemWidth(hItem);
    int nCheckBoxValue = pItem->nCheckBoxValue;
    if(bVisible)
    {
        if(GetChildItem(hItem) && pItem->bCollapsed==FALSE)
        {
            SetChildrenVisible(hItem,FALSE);
        }
    }

    if(IsAncestor(hItem,m_hHoverItem)) m_hHoverItem=NULL;
    if(IsAncestor(hItem,m_hSelItem)) m_hSelItem=NULL;
    if(IsAncestor(hItem,m_hCaptureItem)) m_hCaptureItem=NULL;

    DeleteItem(hItem);
    
    //去掉父节点的展开标志
    if(hParent && !GetChildItem(hParent))
    {
        LPTVITEM pParent = GetItem(hParent);
        pParent->bHasChildren = FALSE;
        pParent->bCollapsed = FALSE;
        CalaItemWidth(pParent);            
    }

    if(m_bCheckBox && hParent && GetChildItem(hParent))
    {
        //原结点复选框选中，检查父结点是否由半选变不选    
        if (nCheckBoxValue == DuiTVICheckBox_Checked )
            CheckState(hParent, FALSE);    
        //原结点复选框未选中，检查父结点是否由半选变全选    
        else
            CheckState(hParent, TRUE);    
    }

    if(bVisible)
    {
        m_nVisibleItems--;

        //重新计算x最大尺寸
        if (nItemWidth == m_nMaxItemWidth)        
            GetMaxItemWidth();        

        CSize szView(m_nMaxItemWidth,m_nVisibleItems*m_nItemHei);
        SetViewSize(szView);
        Invalidate();
    }
    return TRUE;
}

void STreeCtrl::RemoveAllItems()
{
    DeleteAllItems();
    m_nVisibleItems=0;
    m_hSelItem=NULL;
    m_hHoverItem=NULL;
    m_hCaptureItem=NULL;
    m_nMaxItemWidth=0;    
    SetViewSize(CSize(0,0));
    SetViewOrigin(CPoint(0,0));
}

HSTREEITEM STreeCtrl::GetRootItem()
{
    return GetChildItem(STVI_ROOT);
}

HSTREEITEM STreeCtrl::GetNextSiblingItem(HSTREEITEM hItem)
{
    return CSTree<LPTVITEM>::GetNextSiblingItem(hItem);
}

HSTREEITEM STreeCtrl::GetPrevSiblingItem(HSTREEITEM hItem)
{
    return CSTree<LPTVITEM>::GetPrevSiblingItem(hItem);
}

HSTREEITEM STreeCtrl::GetChildItem(HSTREEITEM hItem,BOOL bFirst/* =TRUE*/)
{
    return CSTree<LPTVITEM>::GetChildItem(hItem,bFirst);
}

HSTREEITEM STreeCtrl::GetParentItem(HSTREEITEM hItem)
{
    return CSTree<LPTVITEM>::GetParentItem(hItem);
}

HSTREEITEM STreeCtrl::GetSelectedItem()
{
    return m_hSelItem;
}

BOOL STreeCtrl::GetItemText(HSTREEITEM hItem, SStringT& strText) const
{
    if (hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            strText = pItem->strText;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::SetItemText(HSTREEITEM hItem, LPCTSTR lpszItem)
{
    if (hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            pItem->strText = lpszItem;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::GetItemImage(HSTREEITEM hItem, int& nImage, int& nSelectedImage) const
{
    if (hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            nImage = pItem->nImage;
            nSelectedImage = pItem->nSelectedImage;
            return TRUE;
        }
    }
    return FALSE;
}

LPARAM STreeCtrl::GetItemData(HSTREEITEM hItem) const
{
    if (hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
            return pItem->lParam;
    }
    return 0;
}

BOOL STreeCtrl::SetItemData(HSTREEITEM hItem, LPARAM lParam)
{
    if (hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if (pItem)
        {
            pItem->lParam = lParam;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL STreeCtrl::ItemHasChildren(HSTREEITEM hItem) 
{
    if (!hItem) return FALSE;

    return  GetChildItem(hItem)!=NULL;
}

BOOL STreeCtrl::GetCheckState(HSTREEITEM hItem) const
{
    if (!m_bCheckBox) return FALSE;

    LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
    if (pItem->nCheckBoxValue == DuiTVICheckBox_Checked)
        return TRUE;

    return FALSE;
}

BOOL STreeCtrl::SetCheckState(HSTREEITEM hItem, BOOL bCheck)
{
    if (!m_bCheckBox) return FALSE;

    int nCheck = bCheck ? DuiTVICheckBox_Checked : DuiTVICheckBox_UnChecked;   

    LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
    pItem->nCheckBoxValue = nCheck;

    //置子孙结点
    if (CSTree<LPTVITEM>::GetChildItem(hItem))
        SetChildrenState(hItem, nCheck);

    //检查父结点状态
    CheckState(GetParentItem(hItem), bCheck);

    Invalidate();

    return TRUE;
}

BOOL STreeCtrl::Expand(HSTREEITEM hItem , UINT nCode)
{
    BOOL bRet=FALSE;
    if(CSTree<LPTVITEM>::GetChildItem(hItem))
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if(nCode==TVE_COLLAPSE && !pItem->bCollapsed)
        {
            pItem->bCollapsed=TRUE;
            SetChildrenVisible(hItem,FALSE);
            bRet=TRUE;
        }
        if(nCode==TVE_EXPAND && pItem->bCollapsed)
        {
            pItem->bCollapsed=FALSE;
            SetChildrenVisible(hItem,TRUE);
            bRet=TRUE;
        }
        if(nCode==TVE_TOGGLE)
        {
            pItem->bCollapsed=!pItem->bCollapsed;
            SetChildrenVisible(hItem,!pItem->bCollapsed);
            bRet=TRUE;
        }
        if(bRet)
        {
            GetMaxItemWidth();
            CSize szView(m_nMaxItemWidth,m_nVisibleItems*m_nItemHei);
            SetViewSize(szView);
            Invalidate();
        }
    }
    return bRet;
}

BOOL STreeCtrl::EnsureVisible(HSTREEITEM hItem)
{
    LPTVITEM pItem=GetItem(hItem);
    if(!pItem->bVisible)
    {
        HSTREEITEM hParent=GetParentItem(hItem);
        while(hParent)
        {
            LPTVITEM pParent=GetItem(hParent);
            if(pParent->bCollapsed) Expand(hParent,TVE_EXPAND);
            hParent=GetParentItem(hParent);
        }
    }
    int iVisible= GetItemShowIndex(hItem);
    int yOffset=iVisible*m_nItemHei;
    if(yOffset+m_nItemHei>m_ptOrigin.y+m_rcClient.Height())
    {
        SetScrollPos(TRUE,yOffset+m_nItemHei-m_rcClient.Height(),TRUE);
    }else if(yOffset<m_ptOrigin.y)
    {
        SetScrollPos(TRUE,yOffset,TRUE);    
    }
    return TRUE;
}

void STreeCtrl::PageUp()
{
    OnScroll(TRUE,SB_PAGEUP,0);
}

void STreeCtrl::PageDown()
{
    OnScroll(TRUE,SB_PAGEDOWN,0);
}

////////////////////////////////////////////////////////////////////////////////////////////

BOOL STreeCtrl::CreateChildren(pugi::xml_node xmlNode)
{
    if(!xmlNode) return FALSE;

    RemoveAllItems();
    ItemLayout();

    pugi::xml_node xmlItem=xmlNode.child(L"item");

    if(xmlItem) LoadBranch(STVI_ROOT,xmlItem);

    return TRUE;
}

void STreeCtrl::LoadBranch(HSTREEITEM hParent,pugi::xml_node xmlItem)
{
    while(xmlItem)
    {
        HSTREEITEM hItem=InsertItem(xmlItem,hParent);

        pugi::xml_node xmlChild=xmlItem.child(L"item");
        if(xmlChild) LoadBranch(hItem,xmlChild);

        xmlItem=xmlItem.next_sibling(L"item");
    }
}

void STreeCtrl::LoadItemAttribute(pugi::xml_node xmlItem, LPTVITEM pItem)
{
    for (pugi::xml_attribute attr=xmlItem.first_attribute(); attr; attr=attr.next_attribute())
    {
        if ( !_wcsicmp(attr.name(), L"text"))
            pItem->strText = DUI_CW2T(attr.value()); 
        else if ( !_wcsicmp(attr.name(), L"img"))
            pItem->nImage = attr.as_int(0);
        else if ( !_wcsicmp(attr.name(), L"selimg"))
            pItem->nSelectedImage = attr.as_int(0);
        else if ( !_wcsicmp(attr.name(), L"data"))
            pItem->lParam = attr.as_uint(0);
    }
}

HSTREEITEM STreeCtrl::InsertItem(LPTVITEM pItemObj,HSTREEITEM hParent,HSTREEITEM hInsertAfter,BOOL bEnsureVisible)
{
    ASSERT(pItemObj);

    int nViewWidth;
    CRect rcClient;
    GetClient(rcClient);

    pItemObj->nLevel = GetItemLevel(hParent)+1;

    if(hParent!=STVI_ROOT)
    {        
        LPTVITEM pParentItem= GetItem(hParent);
        if(pParentItem->bCollapsed || !pParentItem->bVisible) 
            pItemObj->bVisible=FALSE;

        if (!GetChildItem(hParent) && !pParentItem->bHasChildren)
        {
            pParentItem->bHasChildren = TRUE;
            CalaItemWidth(pParentItem);
        }
    }    

    CalaItemWidth(pItemObj);

    HSTREEITEM hRet= CSTree<LPTVITEM>::InsertItem(pItemObj,hParent,hInsertAfter);
    pItemObj->hItem = hRet;

    if(pItemObj->bVisible)
    {
        m_nVisibleItems++;

        nViewWidth = max(rcClient.Width(), pItemObj->nItemWidth + pItemObj->nLevel*m_nIndent);
        if ( nViewWidth > m_nMaxItemWidth ) 
            m_nMaxItemWidth = nViewWidth;

        CSize szView(m_nMaxItemWidth, m_nVisibleItems*m_nItemHei);
        SetViewSize(szView);
        Invalidate();
    }

    if(bEnsureVisible) EnsureVisible(hRet);
    return hRet;
}

HSTREEITEM STreeCtrl::InsertItem(pugi::xml_node xmlItem,HSTREEITEM hParent/*=STVI_ROOT*/, HSTREEITEM hInsertAfter/*=STVI_LAST*/,BOOL bEnsureVisible/*=FALSE*/)
{
    LPTVITEM pItemObj = new TVITEM();

    LoadItemAttribute(xmlItem, pItemObj);
    return InsertItem(pItemObj, hParent, hInsertAfter, bEnsureVisible);
}

int STreeCtrl::GetScrollLineSize(BOOL bVertical)
{
    return m_nItemHei;
}

BOOL STreeCtrl::IsAncestor(HSTREEITEM hItem1,HSTREEITEM hItem2)
{
    while(hItem2)
    {
        if(hItem2==hItem1) return TRUE;
        hItem2=GetParentItem(hItem2);
    }
    return FALSE;
}

void STreeCtrl::SetChildrenVisible(HSTREEITEM hItem,BOOL bVisible)
{
    HSTREEITEM hChild=GetChildItem(hItem);
    while(hChild)
    {
        LPTVITEM pItem=GetItem(hChild);
        pItem->bVisible=bVisible;
        m_nVisibleItems += bVisible?1:-1;
        if(!pItem->bCollapsed) SetChildrenVisible(hChild,bVisible);
        hChild=GetNextSiblingItem(hChild);
    }
}

void STreeCtrl::SetChildrenState(HSTREEITEM hItem, int nCheckValue)
{
    HSTREEITEM hChildItem = CSTree<LPTVITEM>::GetChildItem(hItem);
    while(hChildItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hChildItem);
        pItem->nCheckBoxValue = nCheckValue;    
        SetChildrenState(hChildItem, nCheckValue);
        hChildItem = CSTree<LPTVITEM>::GetNextSiblingItem(hChildItem);
    }        
}

//子孙结点状态一致返回TRUE, 否则返回FALSE
BOOL STreeCtrl::CheckChildrenState(HSTREEITEM hItem, BOOL bCheck)
{
    HSTREEITEM hChildItem = CSTree<LPTVITEM>::GetChildItem(hItem);
    while(hChildItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hChildItem);

        int nCheckValue = bCheck ? DuiTVICheckBox_Checked : DuiTVICheckBox_UnChecked;
        //当前结点不一致立即返回
        if (pItem->nCheckBoxValue != nCheckValue) 
            return FALSE;
        //检查子结点不一致立即返回
        else if (CheckChildrenState(hChildItem, bCheck) == FALSE)
            return FALSE;
        
        //检查子结点兄弟结点
        hChildItem = CSTree<LPTVITEM>::GetNextSiblingItem(hChildItem);
    }
    return TRUE;
}

void STreeCtrl::CheckState(HSTREEITEM hItem, BOOL bCheck, BOOL bCheckChild)
{        
    if(hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);

        if (bCheckChild && CheckChildrenState(hItem, bCheck))
        {
            int nCheckValue = bCheck ? DuiTVICheckBox_Checked : DuiTVICheckBox_UnChecked;
            pItem->nCheckBoxValue = nCheckValue;
            CheckState(GetParentItem(hItem), bCheck, TRUE);
        }
        else
        {
            pItem->nCheckBoxValue = DuiTVICheckBox_PartChecked;
            CheckState(GetParentItem(hItem),bCheck,  FALSE);
        }
    }        
}

void STreeCtrl::ItemLayout()
{
    int nOffset = 0;
    CSize sizeSkin;

    m_uItemMask = 0;
    m_rcToggle.SetRect(0, 0, 0, 0);
    m_rcCheckBox.SetRect(0, 0, 0, 0);
    m_rcIcon.SetRect(0, 0, 0, 0);

    //计算位置    
    if (m_pToggleSkin)
    {
        m_uItemMask |= DuiTVIMask_Toggle;
        sizeSkin = m_pToggleSkin->GetSkinSize();        
        m_rcToggle.SetRect( 
            nOffset, 
            (m_nItemHei - sizeSkin.cy) / 2,
            nOffset + sizeSkin.cx,
            m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
        nOffset += sizeSkin.cx;
    }

    if (m_bCheckBox && m_pCheckSkin)
    {
        m_uItemMask |= DuiTVIMask_CheckBox;
        sizeSkin = m_pCheckSkin->GetSkinSize();    
        m_rcCheckBox.SetRect( 
            nOffset, 
            (m_nItemHei - sizeSkin.cy) / 2,
            nOffset + sizeSkin.cx,
            m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
        nOffset += sizeSkin.cx;
    }

    if (m_pIconSkin)
    {
        m_uItemMask |= DuiTVIMask_Icon;
        sizeSkin = m_pIconSkin->GetSkinSize();
        m_rcIcon.SetRect( 
            nOffset, 
            (m_nItemHei - sizeSkin.cy) / 2,
            nOffset + sizeSkin.cx,
            m_nItemHei - (m_nItemHei - sizeSkin.cy) / 2);
        nOffset += sizeSkin.cx;
    }

    m_nItemOffset = nOffset;
}

void STreeCtrl::CalaItemWidth(LPTVITEM pItem)
{
    CAutoRefPtr<IRenderTarget> pRT;
    GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
    BeforePaintEx(pRT);
    
    int nTestDrawMode = GetTextAlign() & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);

    CRect rcTest;
    DrawText(pRT,pItem->strText, pItem->strText.GetLength(), rcTest, nTestDrawMode | DT_CALCRECT);

    pItem->nItemWidth = rcTest.Width() + m_nItemOffset + 2*m_nItemMargin;
}

int STreeCtrl::GetMaxItemWidth(HSTREEITEM hItem)
{
    int nItemWidth = 0, nChildrenWidth = 0;

    LPTVITEM pItem=GetItem(hItem);
    if (pItem->bVisible)
        nItemWidth = pItem->nItemWidth + pItem->nLevel*m_nIndent;
    else
        return 0;

    HSTREEITEM hChild=GetChildItem(hItem);
    while(hChild)
    {
        nChildrenWidth = GetMaxItemWidth(hChild);
        if (nChildrenWidth > nItemWidth)
            nItemWidth = nChildrenWidth;

        hChild=GetNextSiblingItem(hChild);
    }

    return nItemWidth;
}

int  STreeCtrl::GetMaxItemWidth()
{
    int nItemWidth = 0;
    m_nMaxItemWidth = 0;
    HSTREEITEM hItem=CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    
    while(hItem)
    {
        nItemWidth = GetMaxItemWidth(hItem);
        if (nItemWidth > m_nMaxItemWidth)
            m_nMaxItemWidth = nItemWidth;
        hItem=GetNextSiblingItem(hItem);
    }

    return m_nMaxItemWidth;
}

int STreeCtrl::GetItemShowIndex(HSTREEITEM hItemObj)
{
    int iVisible=-1;
    HSTREEITEM hItem=GetNextItem(STVI_ROOT);
    while(hItem)
    {
        LPTVITEM pItem=GetItem(hItem);
        if(pItem->bVisible) iVisible++;
        if(hItem==hItemObj)
        {
            return iVisible;
        }
        if(pItem->bCollapsed)
        {//跳过被折叠的项
            HSTREEITEM hChild= GetChildItem(hItem,FALSE);
            while(hChild)
            {
                hItem=hChild;
                hChild= GetChildItem(hItem,FALSE);
            }
        }
        hItem=GetNextItem(hItem);
    }
    return -1;
}

BOOL STreeCtrl::GetItemRect( LPTVITEM pItemObj,CRect &rcItem )
{
    if(pItemObj->bVisible==FALSE) return FALSE;

    CRect rcClient;
    GetClient(rcClient);
    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    int iVisible=-1;
    HSTREEITEM hItem=CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while(hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if(pItem->bVisible) iVisible++;
        if(iVisible > iFirstVisible+nPageItems) break;
        if(iVisible>=iFirstVisible && pItem==pItemObj)
        {
            CRect rcRet(m_nIndent*pItemObj->nLevel,0,rcClient.Width(),m_nItemHei);
            rcRet.OffsetRect(rcClient.left-m_ptOrigin.x,rcClient.top-m_ptOrigin.y+iVisible*m_nItemHei);
            rcItem=rcRet;
            return TRUE;
        }
        if(pItem->bCollapsed)
        {//跳过被折叠的项
            HSTREEITEM hChild= GetChildItem(hItem,FALSE);
            while(hChild)
            {
                hItem=hChild;
                hChild= GetChildItem(hItem,FALSE);
            }
        }
        hItem=CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    return FALSE;
}

//自动修改pt的位置为相对当前项的偏移量
HSTREEITEM STreeCtrl::HitTest(CPoint &pt)
{
    CRect rcClient;
    GetClient(&rcClient);
    CPoint pt2=pt;
    pt2.y -= rcClient.top - m_ptOrigin.y;
    int iItem=pt2.y/m_nItemHei;
    if( iItem >= m_nVisibleItems) return NULL;

    HSTREEITEM hRet=NULL;

    int iVisible=-1;
    HSTREEITEM hItem=CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while(hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if(pItem->bVisible) iVisible++;
        if(iVisible == iItem)
        {
            CRect rcItem(m_nIndent*pItem->nLevel,0,rcClient.Width(),m_nItemHei);
            rcItem.OffsetRect(rcClient.left-m_ptOrigin.x,rcClient.top-m_ptOrigin.y+iVisible*m_nItemHei);
            pt-=rcItem.TopLeft();
            hRet=hItem;
            break;
        }
        if(pItem->bCollapsed)
        {//跳过被折叠的项
            HSTREEITEM hChild= GetChildItem(hItem,FALSE);
            while(hChild)
            {
                hItem=hChild;
                hChild= GetChildItem(hItem,FALSE);
            }
        }
        hItem=CSTree<LPTVITEM>::GetNextItem(hItem);
    }        
    return hRet;
}

void STreeCtrl::RedrawItem(HSTREEITEM hItem)
{
    if(!IsVisible(TRUE)) return;
    CRect rcClient;
    GetClient(rcClient);

    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;
    int iItem=GetItemShowIndex(hItem);
    if(iItem!=-1 && iItem>=iFirstVisible && iItem<iFirstVisible+nPageItems)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        
        CRect rcItem(m_nIndent*pItem->nLevel,0,m_nIndent*pItem->nLevel+pItem->nItemWidth ,m_nItemHei);
        rcItem.OffsetRect(rcClient.left-m_ptOrigin.x,
                rcClient.top+m_nItemHei*iItem-m_ptOrigin.y);
        if (rcItem.bottom > rcClient.bottom) rcItem.bottom = rcClient.bottom;
        if (rcItem.right > rcClient.right) rcItem.right = rcClient.right;

        CAutoRefPtr<IRenderTarget> pRT=GetRenderTarget(&rcItem,OLEDC_PAINTBKGND);
        SPainter painter;
        BeforePaint(pRT,painter);

        SendMessage(WM_ERASEBKGND,(WPARAM)(void*)pRT);

        DrawItem(pRT,rcItem,hItem);

        AfterPaint(pRT,painter);
        ReleaseRenderTarget(pRT);
    }
}

void STreeCtrl::DrawItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem)
{    
    BOOL     bTextColorChanged = FALSE;;
    COLORREF crOldText;
    CRect rcItemBg;
    LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);

    pRT->OffsetViewportOrg(rc.left,rc.top);

    rcItemBg.SetRect( m_nItemOffset + m_nItemMargin, 0, pItem->nItemWidth, m_nItemHei);

    //绘制背景
    if (hItem == m_hSelItem)
    {
        if (m_pItemSelSkin != NULL)
            m_pItemSelSkin->Draw(pRT, rcItemBg, 0);
        else if (CR_INVALID != m_crItemSelBg)
            pRT->FillSolidRect(rcItemBg, m_crItemSelBg);

        if (CR_INVALID != m_crItemSelText)
        {
            bTextColorChanged = TRUE;
            crOldText = pRT->SetTextColor(m_crItemSelText);
        }
    }
    else
    {
        if (CR_INVALID != m_crItemText)
        {
            bTextColorChanged = TRUE;
            crOldText = pRT->SetTextColor(m_crItemText);
        }
    }

    if (pItem->bHasChildren &&
        DuiTVIMask_Toggle == (m_uItemMask & DuiTVIMask_Toggle))
    {
        int nImage = IIF_STATE3(pItem->dwToggleState, 0, 1, 2);
        if (!pItem->bCollapsed) nImage += 3;
        m_pToggleSkin->Draw(pRT, m_rcToggle, nImage);
    }
    
    if (DuiTVIMask_CheckBox == (m_uItemMask & DuiTVIMask_CheckBox))
    {
        int nImage = IIF_STATE3(pItem->dwCheckBoxState, 0, 1, 2);
        if (pItem->nCheckBoxValue == DuiTVICheckBox_Checked)
            nImage += 3;
        else if (pItem->nCheckBoxValue == DuiTVICheckBox_PartChecked)
            nImage += 6;
        m_pCheckSkin->Draw(pRT, m_rcCheckBox, nImage);
    }
    
    if (DuiTVIMask_Icon == (m_uItemMask & DuiTVIMask_Icon) &&
        (pItem->nSelectedImage != -1 || pItem->nImage != -1))
    {
        if (pItem->nSelectedImage != -1 && hItem == m_hSelItem)
            m_pIconSkin->Draw(pRT, m_rcIcon, pItem->nSelectedImage);
        else 
            m_pIconSkin->Draw(pRT, m_rcIcon, pItem->nImage);        
    }
        
    UINT align=DT_VCENTER|DT_SINGLELINE;
    rcItemBg.OffsetRect(m_nItemMargin, 0);
    pRT->DrawText(pItem->strText,-1,rcItemBg,align);    

    if (bTextColorChanged)
        pRT->SetTextColor(crOldText);

    pRT->OffsetViewportOrg(-rc.left,-rc.top);
}

int STreeCtrl::ItemHitTest(HSTREEITEM hItem,CPoint &pt) const
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);
    int nHitTestBtn = DuiTVIBtn_None;

    if (DuiTVIMask_Toggle == (m_uItemMask & DuiTVIMask_Toggle)
        && pItem->bHasChildren
        && m_rcToggle.PtInRect(pt))
        nHitTestBtn = DuiTVIBtn_Toggle;
    else if (DuiTVIMask_CheckBox == (m_uItemMask & DuiTVIMask_CheckBox)
        && m_rcCheckBox.PtInRect(pt))
        nHitTestBtn = DuiTVIBtn_CheckBox;
    
    return nHitTestBtn;
}


void STreeCtrl::ModifyToggleState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    pItem->dwToggleState |= dwStateAdd;
    pItem->dwToggleState &= ~dwStateRemove;
    
    CRect rcItem, rcUpdate = m_rcToggle;
    if (GetItemRect(pItem, rcItem))
    {        
        rcUpdate.OffsetRect(rcItem.left, rcItem.top);
        InvalidateRect(rcUpdate);
    }
}

void STreeCtrl::ModifyChekcBoxState(HSTREEITEM hItem, DWORD dwStateAdd, DWORD dwStateRemove)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    pItem->dwCheckBoxState |= dwStateAdd;
    pItem->dwCheckBoxState &= ~dwStateRemove;

    CRect rcItem, rcUpdate = m_rcCheckBox;
    if (GetItemRect(pItem, rcItem))
    {
        rcUpdate.OffsetRect(rcItem.left, rcItem.top);
        InvalidateRect(rcUpdate);
    }
}

void STreeCtrl::ItemLButtonDown(HSTREEITEM hItem, UINT nFlags,CPoint pt)
{
    int nHitTestBtn = ItemHitTest(hItem, pt);    
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);    

    //清除原有pushdown按钮
    if (m_nItemPushDownBtn != nHitTestBtn)
    {
        if (m_nItemPushDownBtn == DuiTVIBtn_Toggle && 
            DuiWndState_PushDown == (pItem->dwToggleState & DuiWndState_PushDown))
        {
            ModifyToggleState(hItem, 0, DuiWndState_PushDown);
        }

        if (m_nItemPushDownBtn == DuiTVIBtn_CheckBox && 
            DuiWndState_PushDown == (pItem->dwCheckBoxState & DuiWndState_PushDown))
        {
            ModifyChekcBoxState(hItem, 0, DuiWndState_PushDown);
        }

        m_nItemPushDownBtn = nHitTestBtn;
    }

    //置新pushdown按钮
    if (m_nItemPushDownBtn != DuiTVIBtn_None)
    {
        if (m_nItemPushDownBtn == DuiTVIBtn_Toggle && 
            DuiWndState_PushDown != (pItem->dwToggleState & DuiWndState_PushDown))
        {
            ModifyToggleState(hItem, DuiWndState_PushDown, 0);
            Expand(pItem->hItem, TVE_TOGGLE);            
        }

        if (m_nItemPushDownBtn == DuiTVIBtn_CheckBox && 
            DuiWndState_PushDown != (pItem->dwCheckBoxState & DuiWndState_PushDown))
        {
            BOOL bCheck = 
                pItem->nCheckBoxValue == DuiTVICheckBox_Checked ? FALSE : TRUE;
            ModifyChekcBoxState(hItem, DuiWndState_PushDown, 0);
            SetCheckState(pItem->hItem, bCheck);
        }
    }
}

void STreeCtrl::ItemLButtonUp(HSTREEITEM hItem, UINT nFlags,CPoint pt)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    if (m_nItemPushDownBtn != DuiTVIBtn_None)
    {
        if (m_nItemPushDownBtn == DuiTVIBtn_Toggle && 
            DuiWndState_PushDown == (pItem->dwToggleState & DuiWndState_PushDown))
        {
            ModifyToggleState(hItem, 0, DuiWndState_PushDown);
        }

        if (m_nItemPushDownBtn == DuiTVIBtn_CheckBox && 
            DuiWndState_PushDown == (pItem->dwCheckBoxState & DuiWndState_PushDown))
        {
            ModifyChekcBoxState(hItem, 0, DuiWndState_PushDown);

            DUINMCOMMAND nms;
            nms.hdr.code = NM_COMMAND;
            nms.hdr.hDuiWnd=m_hSWnd;
            nms.hdr.idFrom = GetID();
            nms.hdr.pszNameFrom=GetName();
            nms.uItemData = hItem; 
            FireEvent((LPSNMHDR)&nms);
        }

        m_nItemPushDownBtn = DuiTVIBtn_None;
    }
}

void STreeCtrl::ItemLButtonDbClick(HSTREEITEM hItem, UINT nFlags,CPoint pt)
{
    int nHitTestBtn = ItemHitTest(hItem, pt);
    if (nHitTestBtn == DuiTVIBtn_CheckBox)
        ItemLButtonDown(hItem, nFlags, pt);
}

void STreeCtrl::ItemMouseMove(HSTREEITEM hItem, UINT nFlags,CPoint pt)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    int nHitTestBtn = ItemHitTest(hItem, pt);
    
    if (nHitTestBtn != m_nItemHoverBtn)
    {
        if (m_nItemHoverBtn == DuiTVIBtn_Toggle && 
            DuiWndState_Hover == (pItem->dwToggleState & DuiWndState_Hover))
        {
            ModifyToggleState(hItem, 0, DuiWndState_Hover);
        }

        if (m_nItemHoverBtn == DuiTVIBtn_CheckBox && 
            DuiWndState_Hover == (pItem->dwCheckBoxState & DuiWndState_Hover))
        {
            ModifyChekcBoxState(hItem, 0, DuiWndState_Hover);
        }

        m_nItemHoverBtn = nHitTestBtn;
    }

    if (m_nItemHoverBtn != DuiTVIBtn_None)
    {
        if (m_nItemHoverBtn == DuiTVIBtn_Toggle && 
            DuiWndState_Hover != (pItem->dwToggleState & DuiWndState_Hover))
        {
            ModifyToggleState(hItem, DuiWndState_Hover, 0);
        }

        if (m_nItemHoverBtn == DuiTVIBtn_CheckBox && 
            DuiWndState_Hover != (pItem->dwCheckBoxState & DuiWndState_Hover))
        {
            ModifyChekcBoxState(hItem, DuiWndState_Hover, 0);
        }
    }
}

void STreeCtrl::ItemMouseLeave(HSTREEITEM hItem)
{
    LPTVITEM pItem = CSTree<LPTVITEM>::GetItem(hItem);

    if (m_nItemHoverBtn != DuiTVIBtn_None)
    {
        if (m_nItemHoverBtn == DuiTVIBtn_Toggle &&
            DuiWndState_Hover == (pItem->dwToggleState & DuiWndState_Hover))
        {
            ModifyToggleState(hItem, 0, DuiWndState_Hover);
        }

        if (m_nItemHoverBtn == DuiTVIBtn_CheckBox &&
            DuiWndState_Hover == (pItem->dwCheckBoxState & DuiWndState_Hover))
        {
            ModifyChekcBoxState(hItem, 0, DuiWndState_Hover);
        }

        m_nItemHoverBtn = DuiTVIBtn_None;
    }
}

void STreeCtrl::NotifyParent()
{
    DUINMTBSELCHANGED nms;
    nms.hdr.code=NM_TBSELCHANGED;
    nms.hdr.hDuiWnd=m_hSWnd;
    nms.hdr.idFrom=GetID();
    nms.hdr.pszNameFrom=GetName();
    nms.hOldSel=m_hSelItem;
    nms.hNewSel=m_hHoverItem;

    if(m_hSelItem)
    {
        HSTREEITEM hOldSelItem = m_hSelItem;
        m_hSelItem = m_hHoverItem;        
        RedrawItem(hOldSelItem);
    }
    else
        m_hSelItem = m_hHoverItem;

    if(m_hSelItem)
    RedrawItem(m_hSelItem);

    FireEvent((LPSNMHDR)&nms);
}

////////////////////////////////////////////////////////////////////////////////////////////

void STreeCtrl::OnDestroy()
{
    DeleteAllItems();
}

void STreeCtrl::OnPaint(IRenderTarget *pRT)
{
    if(IsUpdateLocked()) return;

    CRect rcClient;
    SPainter painter;
    BeforePaint(pRT,painter);

    GetClient(rcClient);
    int iFirstVisible=m_ptOrigin.y/m_nItemHei;
    int nPageItems=(m_rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;

    int iVisible=-1;
    HSTREEITEM hItem=CSTree<LPTVITEM>::GetNextItem(STVI_ROOT);
    while(hItem)
    {
        LPTVITEM pItem=CSTree<LPTVITEM>::GetItem(hItem);
        if(pItem->bVisible) iVisible++;
        if(iVisible > iFirstVisible+nPageItems) break;
        if(iVisible>=iFirstVisible)
        {
            CRect rcItem(m_nIndent*pItem->nLevel,0,m_nIndent*pItem->nLevel+pItem->nItemWidth,m_nItemHei);
            rcItem.OffsetRect(rcClient.left-m_ptOrigin.x,
                rcClient.top-m_ptOrigin.y+iVisible*m_nItemHei);
            DrawItem(pRT,rcItem,hItem);
        }
        if(pItem->bCollapsed)
        {//跳过被折叠的项
            HSTREEITEM hChild= GetChildItem(hItem,FALSE);
            while(hChild)
            {
                hItem=hChild;
                hChild= GetChildItem(hItem,FALSE);
            }
        }
        hItem=CSTree<LPTVITEM>::GetNextItem(hItem);
    }
    AfterPaint(pRT,painter);
}

void STreeCtrl::OnLButtonDown(UINT nFlags,CPoint pt)
{
    m_hHoverItem=HitTest(pt);

    if(m_hHoverItem!=m_hSelItem)
        NotifyParent();

    //pt 已经在HitTest中被修改过
    if(m_hHoverItem)
    {    
        m_hCaptureItem = m_hHoverItem;
        ItemLButtonDown(m_hHoverItem, nFlags, pt);
    }
}

void STreeCtrl::OnRButtonDown(UINT nFlags, CPoint pt)
{
    if (!m_bRightClickSel)
        return;

    m_hHoverItem=HitTest(pt);

    if(m_hHoverItem!=m_hSelItem)
        NotifyParent();
}

void STreeCtrl::OnLButtonUp(UINT nFlags,CPoint pt)
{
    m_hHoverItem=HitTest(pt);

    if (m_hCaptureItem)
    {
        ItemLButtonUp(m_hCaptureItem, nFlags, pt);
        m_hCaptureItem = NULL;
        return;
    }

    if(m_hHoverItem)
        ItemLButtonUp(m_hHoverItem, nFlags, pt);
}

void STreeCtrl::OnLButtonDbClick(UINT nFlags,CPoint pt)
{    
    m_hHoverItem=HitTest(pt);
    if(m_hHoverItem)
    {
        Expand(m_hHoverItem,TVE_TOGGLE);
        ItemLButtonDbClick(m_hHoverItem, nFlags, pt);
    }else
    {
        DUINMITEMMOUSEEVENT nms;
        nms.hdr.hDuiWnd=m_hSWnd;
        nms.hdr.code=NM_ITEMMOUSEEVENT;
        nms.hdr.idFrom=GetID();
        nms.hdr.pszNameFrom=GetName();
        nms.pItem=NULL;
        nms.uMsg=WM_LBUTTONDBLCLK;
        nms.wParam=nFlags;
        nms.lParam=MAKELPARAM(pt.x,pt.y);
        FireEvent((LPSNMHDR)&nms);
    }
}

void STreeCtrl::OnMouseMove(UINT nFlags,CPoint pt)
{
    HSTREEITEM hHitTest=HitTest(pt);
    
    if(hHitTest!=m_hHoverItem)
    {
        if(m_hHoverItem)        
            ItemMouseLeave(m_hHoverItem);
    
        m_hHoverItem=hHitTest;
    }
    if(m_hHoverItem)
        ItemMouseMove(m_hHoverItem, nFlags, pt);
}

void STreeCtrl::OnMouseLeave()
{
    if(m_hHoverItem)
    {
        ItemMouseLeave(m_hHoverItem);
        m_hHoverItem=NULL;
    }
}

}//namespace SOUI