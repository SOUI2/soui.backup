#include "StdAfx.h"
#include "SPropertyGrid.h"

const int KPropItemIndent   = 10;

namespace SOUI
{
    int SPropertyItemBase::GetIndent() const
    {
        int iLevel = 0;
        IPropertyItem *pParent=GetParent();
        while(pParent)
        {
            iLevel ++;
        }
        return iLevel * GetOwner()->GetIndent();
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
        return TRUE;
    }

    BOOL SPropertyItemBase::RemoveChild( IPropertyItem * pChild )
    {
        POSITION pos = m_childs.Find(pChild);
        if(!pos) return FALSE;
        m_childs.RemoveAt(pos);
        return TRUE;
    }

    SOUI::SStringT SPropertyItemBase::GetCaption() const
    {
        return m_strCaption;
    }

    SOUI::SStringT SPropertyItemBase::GetDescription() const
    {
        return m_strDescription;
    }

    BOOL SPropertyItemBase::HasEdit() const
    {
        return FALSE;
    }

    CSize SPropertyItemBase::GetButtonSize() const
    {
        return CSize();
    }

    void SPropertyItemBase::DrawButton( IRenderTarget *pRT,CRect rc,int nState )
    {

    }

    void SPropertyItemBase::DrawClient( IRenderTarget *pRT,CRect rc )
    {
        SStringT strValue = GetValue();
        pRT->DrawText(strValue,strValue.GetLength(),rc,DT_SINGLELINE|DT_VCENTER);
    }

    //////////////////////////////////////////////////////////////////////////
    SPropertyGrid::SPropertyGrid(void):m_nIndent(KPropItemIndent)
    {
    }

    SPropertyGrid::~SPropertyGrid(void)
    {
    }

    int SPropertyGrid::GetIndent()
    {
        return m_nIndent;
    }

    void SPropertyGrid::OnItemExpanded( IPropertyItem *pItem)
    {
    
    }


}
