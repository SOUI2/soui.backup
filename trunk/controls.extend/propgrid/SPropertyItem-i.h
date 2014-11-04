#pragma once

namespace SOUI
{
    #define IC_FIRST    (IPropertyItem *)0
    #define IC_LAST     (IPropertyItem *)1

    class SPropertyGrid;
    struct IPropertyItem : public IObjRef
    {
        enum PROPITEMTYPE{
            GPI_PARENT,
            GPI_FIRSTCHILD,
            GPI_LASTCHILD,
            GPI_NEXTSIBLING,
            GPI_PREVSIBLINIG,
        };
        virtual ~IPropertyItem(){}
        virtual BOOL IsGroup() const =0;
        virtual int  GetLevel() const =0;
        virtual BOOL IsExpand() const =0;
        virtual void Expand(BOOL bExpend) =0;

        virtual IPropertyItem * GetParent() const =0;
        virtual void SetParent(IPropertyItem * pParent) =0;
        virtual IPropertyItem * GetItem(PROPITEMTYPE  type) const =0;
        virtual SPropertyGrid * GetOwner() const =0;
        virtual BOOL InsertChild(IPropertyItem * pChild,IPropertyItem * pInsertAfter=IC_LAST)=0;
        virtual BOOL RemoveChild(IPropertyItem * pChild)=0;
        virtual int ChildrenCount() const =0;

        virtual SStringT GetName() const =0;
        virtual void SetName(const SStringT & strName) =0;
        virtual SStringT GetDescription() const =0;
        virtual void SetDescription(const SStringT & strDescription) =0;
        virtual SStringT GetValue() const =0;
        virtual void SetValue(void *pValue,UINT uType=0)=0;
        virtual void DrawItem(IRenderTarget *pRT,CRect rc) =0;

        virtual void OnInplaceActive(bool bActive)=0;
    };

    struct IPropInplaceWnd{
        virtual IPropertyItem * GetOwner() =0;
        virtual void UpdateData() =0;
    };

}