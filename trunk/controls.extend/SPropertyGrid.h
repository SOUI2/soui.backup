#pragma once

#include <control/SListbox.h>
#include <souicoll.h>

namespace SOUI
{
    
    #define IC_FIRST    (IPropertyItem *)0
    #define IC_LAST     (IPropertyItem *)1

    class SPropertyGrid;
    struct IPropertyItem
    {
        enum PROPITEMTYPE{
            GPI_PARENT,
            GPI_FIRSTCHILD,
            GPI_LASTCHILD,
            GPI_NEXTSIBLING,
            GPI_PREVSIBLINIG,
        };
        virtual ~IPropertyItem(){}
        virtual int  GetIndent() const =0;
        virtual BOOL IsExpand() const =0;
        virtual void Expand(BOOL bExpend) =0;

        virtual IPropertyItem * GetParent() const =0;
        virtual IPropertyItem * GetItem(PROPITEMTYPE  type) const =0;
        virtual SPropertyGrid * GetOwner() const =0;
        virtual BOOL InsertChild(IPropertyItem * pChild,IPropertyItem * pInsertAfter=IC_LAST)=0;
        virtual BOOL RemoveChild(IPropertyItem * pChild)=0;


        virtual SStringT GetCaption() const =0;
        virtual SStringT GetDescription() const =0;
        virtual SStringT GetValue() const =0;
        virtual BOOL HasEdit() const =0;
        virtual CSize GetButtonSize() const =0;
        virtual void DrawButton(IRenderTarget *pRT,CRect rc,int nState) =0;
        virtual void DrawClient(IRenderTarget *pRT,CRect rc)=0;
    };

    class SPropertyItemBase : public IPropertyItem
    {
    public:
        SPropertyItemBase():m_pParent(NULL),m_bExpanded(FALSE){}
        
        virtual int  GetIndent() const ;
        virtual BOOL IsExpand() const ;
        virtual void Expand(BOOL bExpend) ;

        virtual IPropertyItem * GetParent() const ;
        virtual IPropertyItem * GetItem(PROPITEMTYPE type) const ;
        virtual SPropertyGrid * GetOwner() const ;
        virtual BOOL InsertChild(IPropertyItem * pChild,IPropertyItem * pInsertAfter=IC_LAST);
        virtual BOOL RemoveChild(IPropertyItem * pChild);


        virtual SStringT GetCaption() const ;
        virtual SStringT GetDescription() const;
        virtual SStringT GetValue() const ;
        virtual BOOL HasEdit() const ;
        
        virtual CSize GetButtonSize() const ;
        virtual void DrawButton(IRenderTarget *pRT,CRect rc,int nState) ;
        virtual void DrawClient(IRenderTarget *pRT,CRect rc);

    protected:
        SStringT        m_strCaption;
        SStringT        m_strDescription;
        
        SPropertyGrid * m_pOwner;
        IPropertyItem * m_pParent;
        
        typedef IPropertyItem* IPropertyItemPtr;
        typedef SList<IPropertyItemPtr> PropItemList;
        PropItemList    m_childs;
        
        BOOL            m_bExpanded;
    };
    
    class SPropertyGrid : protected SListBox
    {
        SOUI_CLASS_NAME(SPropertyGrid, L"propgrid")
    public:
        SPropertyGrid(void);
        ~SPropertyGrid(void);
        
        int GetIndent();
        
        void OnItemExpanded(IPropertyItem *pItem);
        CRect GetItemRect(IPropertyItem *pItem) const;
        
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"indent",m_nIndent,TRUE)
        SOUI_ATTRS_END()
    protected:
        int m_nIndent;
    };
}
