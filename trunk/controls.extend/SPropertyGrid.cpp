#include "StdAfx.h"
#include "SPropertyGrid.h"

const int KPropItemIndent   = 10;

namespace SOUI
{
    int SPropertyItemBase::GetLevel() const
    {
        int iLevel = 0;
        IPropertyItem *pParent=GetParent();
        while(pParent)
        {
            iLevel ++;
        }
        return iLevel;
    }

    BOOL SPropertyItemBase::IsExpand() const
    {
        return m_bExpanded;
    }

    void SPropertyItemBase::Expand( BOOL bExpend )
    {
        m_bExpanded = bExpend;
        GetOwner()->OnItemExpanded(this);
    }

    IPropertyItem * SPropertyItemBase::GetParent() const
    {
        return m_pParent;
    }

    void SPropertyItemBase::SetParent( IPropertyItem * pParent )
    {
        m_pParent = pParent;
    }


    IPropertyItem * SPropertyItemBase::GetItem( PROPITEMTYPE type) const
    {
        switch (type)
        {
        case GPI_PARENT:
            return GetParent();
            break;
        case GPI_FIRSTCHILD:
            if(m_childs.IsEmpty()) return NULL;
            else return m_childs.GetHead();
            break;
        case GPI_LASTCHILD:
            if(m_childs.IsEmpty()) return NULL;
            else return m_childs.GetTail();
            break;
        case GPI_NEXTSIBLING:
            {
                SPropertyItemBase * pParent = (SPropertyItemBase*)GetParent();
                if(!pParent) return NULL;
                
                POSITION pos = pParent->m_childs.Find((IPropertyItemPtr)this);
                SASSERT(pos);
                pParent->m_childs.GetNext(pos);
                if(!pos) return NULL;
                return pParent->m_childs.GetAt(pos);
            }
            break;
        case GPI_PREVSIBLINIG:
            {
                SPropertyItemBase * pParent = (SPropertyItemBase*)GetParent();
                if(!pParent) return NULL;
                POSITION pos = pParent->m_childs.Find((IPropertyItemPtr)this);
                SASSERT(pos);
                pParent->m_childs.GetPrev(pos);
                if(!pos) return NULL;
                return pParent->m_childs.GetAt(pos);
            }
            break;
        }
        return NULL;
    }

    SPropertyGrid * SPropertyItemBase::GetOwner() const
    {
        return m_pOwner;
    }

    BOOL SPropertyItemBase::InsertChild( IPropertyItem * pChild,IPropertyItem * pInsertAfter/*=IC_LAST*/ )
    {
        if(pInsertAfter == IC_LAST) m_childs.InsertAfter(NULL,pChild);
        else if(pInsertAfter == IC_FIRST) m_childs.InsertBefore(NULL,pChild);
        else
        {
            POSITION pos = m_childs.Find(pInsertAfter);
            if(!pos) return FALSE;
            m_childs.InsertAfter(pos,pChild);            
        }
        pChild->SetParent(this);
        pChild->AddRef();
        return TRUE;
    }

    int SPropertyItemBase::ChildrenCount() const
    {
        return m_childs.GetCount();
    }

    BOOL SPropertyItemBase::RemoveChild( IPropertyItem * pChild )
    {
        POSITION pos = m_childs.Find(pChild);
        if(!pos) return FALSE;
        m_childs.RemoveAt(pos);
        pChild->Release();
        return TRUE;
    }

    SPropertyItemBase::~SPropertyItemBase()
    {
        POSITION pos = m_childs.GetHeadPosition();
        while(pos)
        {
            IPropertyItemPtr pChild = m_childs.GetNext(pos);
            pChild->Release();
        }
        m_childs.RemoveAll();
    }

    BOOL SPropertyItemBase::InitFromXml( pugi::xml_node xmlNode )
    {
        pugi::xml_node xmlProp=xmlNode.first_child();
        while(xmlProp)
        {
            IPropertyItem * pItem = SPropItemMap::CreatePropItem(xmlProp.name(),GetOwner());
            if(pItem)
            {
                SPropertyItemBase *pItem2 = static_cast<SPropertyItemBase*>(pItem);
                if(pItem2)
                {
                    InsertChild(pItem);
                    pItem2->InitFromXml(xmlProp);
                }
            }
            xmlProp = xmlProp.next_sibling();
        }
        return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
    void SPropertyItemText::DrawItem( IRenderTarget *pRT,CRect rc )
    {
        pRT->DrawText(m_strValue,m_strValue.GetLength(),rc,DT_SINGLELINE|DT_VCENTER);
    }

    //////////////////////////////////////////////////////////////////////////
    void SPropertyGroup::DrawItem( IRenderTarget *pRT,CRect rc )
    {
        pRT->FillSolidRect(rc,0xFF888888);
    }


    //////////////////////////////////////////////////////////////////////////
    SPropItemMap SPropItemMap::s_mapPropItem;
    
    SPropItemMap::SPropItemMap()
    {
        SetAt(SPropertyItemText::GetClassName(),SPropertyItemText::CreatePropItem);
        SetAt(SPropertyGroup::GetClassName(),SPropertyGroup::CreatePropItem);
    }

    void SPropItemMap::RegPropItem( const SStringW & strName, FunCreatePropItem funCreate )
    {
        s_mapPropItem.SetAt(strName,funCreate);
    }

    IPropertyItem * SPropItemMap::CreatePropItem( const SStringW & strName ,SPropertyGrid *pOwner )
    {
        if(!s_mapPropItem.Lookup(strName)) return NULL;
        return s_mapPropItem[strName](pOwner);
    }

    //////////////////////////////////////////////////////////////////////////
    SPropertyGrid::SPropertyGrid(void):m_nIndent(KPropItemIndent),m_nNameWidth(100),m_switchSkin(NULL)
    {
    }

    SPropertyGrid::~SPropertyGrid(void)
    {
        POSITION pos = m_lstGroup.GetHeadPosition();
        while(pos)
        {
            SPropertyGroup * pGroup = m_lstGroup.GetNext(pos);
            pGroup->Release();        
        }
        m_lstGroup.RemoveAll();
    }

    int SPropertyGrid::GetIndent()
    {
        return m_nIndent;
    }

    void SPropertyGrid::OnItemExpanded( IPropertyItem *pItem)
    {
        SASSERT(m_orderType == OT_GROUP);
        int iInsert = IndexOfPropertyItem(pItem);
        if(pItem->IsExpand())
        {
            ExpandChildren(pItem,iInsert);
        }else
        {
            CollapseChildren(pItem,iInsert);
        }
    }

    int SPropertyGrid::IndexOfPropertyItem( const IPropertyItem *pItem ) const
    {
        for(int i=0;i<GetCount();i++)
        {
            if(pItem == (IPropertyItem *)GetItemData(i)) return i;
        }
        return -1;
    }
    
    CRect SPropertyGrid::GetItemRect( IPropertyItem *pItem ) const
    {
        int idx = IndexOfPropertyItem(pItem);
        SASSERT(idx != -1);
        int iTopIdx = GetTopIndex();
        
        int nPageItems = (m_rcClient.Height()+m_nItemHei-1)/m_nItemHei+1;
        if(iTopIdx + nPageItems > GetCount()) nPageItems = GetCount() - iTopIdx;
        CRect rcItem;
        if(idx >= iTopIdx && idx <= iTopIdx+nPageItems)
        {
            rcItem = CRect(0,0,m_rcClient.Width(),m_nItemHei);
            rcItem.OffsetRect(0,m_nItemHei*idx-m_ptOrigin.y);
            rcItem.OffsetRect(m_rcClient.TopLeft());
        }
        return rcItem;
    }

    void SPropertyGrid::SortInsert( IPropertyItem *pItem )
    {
        int iInsert = -1;
        for(int i=0;i<GetCount();i++)
        {
            IPropertyItem *p = (IPropertyItem *)GetItemData(i);
            if(pItem->GetName()>p->GetName()) 
            {
                iInsert = i;
                break;
            }
        }
        InsertString(iInsert,NULL,-1,(LPARAM)pItem);
    }

    BOOL SPropertyGrid::InsertGroup( SPropertyGroup * pGroup,SPropertyGroup* pInertAfter/*=IG_LAST*/ )
    {
        POSITION pos = m_lstGroup.Find(pGroup);
        if(pos) return FALSE;
        if(pInertAfter == IG_FIRST)
        {
            m_lstGroup.InsertBefore(0,pGroup);
        }else if(pInertAfter == IG_LAST)
        {
            m_lstGroup.InsertAfter(0,pGroup);
        }else
        {
            pos = m_lstGroup.Find(pInertAfter);
            if(!pos) return FALSE;
            m_lstGroup.InsertAfter(pos,pGroup);
        }
        pGroup->AddRef();

        switch(m_orderType)
        {
        case OT_NULL:
            {
                IPropertyItem *pChild=pGroup->GetItem(IPropertyItem::GPI_FIRSTCHILD);
                while(pChild)
                {
                    InsertString(-1,NULL,-1,(LPARAM)pChild);
                    pChild = pChild->GetItem(IPropertyItem::GPI_NEXTSIBLING);
                }
            }
            break;
        case OT_GROUP:
            {
                int iInserted = InsertString(-1,NULL,-1,(LPARAM)pGroup);
                if(pGroup->IsExpand())
                {
                    ExpandChildren(pGroup,iInserted);
                }
            }
            break;
        case OT_NAME:
            {
                IPropertyItem *pChild=pGroup->GetItem(IPropertyItem::GPI_FIRSTCHILD);
                while(pChild)
                {
                    SortInsert(pChild);
                    pChild = pChild->GetItem(IPropertyItem::GPI_NEXTSIBLING);
                }
            }
            break;
        }
        return TRUE;
    }

    int SPropertyGrid::ExpandChildren( const IPropertyItem *pItem,int iInsert )
    {
        SASSERT(m_orderType == OT_GROUP);
        SASSERT(pItem->IsExpand());
        SASSERT(iInsert != -1);
        int nRet =0;
        IPropertyItem *pChild = pItem->GetItem(IPropertyItem::GPI_FIRSTCHILD);
        while(pChild)
        {
            InsertString(iInsert++,NULL,-1,(LPARAM)pChild);
            nRet ++;
            pChild = pItem->GetItem(IPropertyItem::GPI_NEXTSIBLING);
            if(pChild->ChildrenCount() && pChild->IsExpand())
            {
                int nAdded = ExpandChildren(pChild,iInsert);
                nRet += nAdded;
                iInsert += nAdded;
            }
            pChild = pItem->GetItem(IPropertyItem::GPI_NEXTSIBLING);
        }
        return nRet;
    }

    void SPropertyGrid::CollapseChildren( const IPropertyItem *pItem,int idx)
    {
        SASSERT(m_orderType == OT_GROUP);
        int nChilds = pItem->ChildrenCount();
        for(int i=0;i<nChilds;i++)
        {
            IPropertyItem *pChild = (IPropertyItem *)GetItemData(idx+1);
            if(pChild->ChildrenCount() && pChild->IsExpand())
            {
                CollapseChildren(pChild,idx+1);
            }
            DeleteString(idx+1);
        }
    }

    void SPropertyGrid::DrawItem( IRenderTarget *pRT, CRect &rc, int iItem )
    {
        IPropertyItem *pItem = (IPropertyItem*)GetItemData(iItem);

        CRect rcSwitch = rc;
        CRect rcNameBack = rc;
        rcSwitch.right = rcSwitch.left +rcSwitch.Height();
        rcNameBack.left = rcSwitch.right;
        rcNameBack.right = rcNameBack.left + m_nNameWidth;
        pRT->FillSolidRect(rcSwitch,0xFF888888);
        pRT->FillSolidRect(rcNameBack,iItem == SListBox::GetCurSel()? 0xFF880000:0xFFFFFFFF);
        
        if(pItem->ChildrenCount() && m_switchSkin)
        {
            int iLevel = pItem->GetLevel();
            if(iLevel>1) rcSwitch.OffsetRect(rcSwitch.Width()*(iLevel-1),0);
            int iState = pItem->IsExpand()?0:1;
            if(pItem->IsGroup()) iState += 2;
            m_switchSkin->Draw(pRT,rcSwitch,iState);
        }
        
        CRect rcName = rcNameBack;
        rcName.left = rcSwitch.right;
        
        pRT->DrawText(pItem->GetName(),pItem->GetName().GetLength(),rcName,DT_SINGLELINE|DT_VCENTER);
        CRect rcItem = rc;
        rc.left = rcName.right;
        pItem->DrawItem(pRT,rcItem);
    }

    void SPropertyGrid::OnLButtonDblClk( UINT nFlags, CPoint point )
    {
        SListBox::OnLButtonDbClick(nFlags,point);
        int iItem = SListBox::HitTest(point);
        if(iItem != -1)
        {
            IPropertyItem *pItem = (IPropertyItem*)GetItemData(iItem);
            if(pItem->ChildrenCount())
            {
                pItem->Expand(!pItem->IsExpand());
                OnItemExpanded(pItem);
            }
        }
    }

    BOOL SPropertyGrid::InitFromXml( pugi::xml_node xmlNode )
    {
        
        return TRUE;
    }



}
