#pragma once

#include <control/SListbox.h>
#include <souicoll.h>
#include <unknown/obj-ref-i.h>
#include <unknown/obj-ref-impl.hpp>

namespace SOUI
{
    
    #define IC_FIRST    (IPropertyItem *)0
    #define IC_LAST     (IPropertyItem *)1

    class SPropertyGrid;
    struct IPropertyItem;
    
    typedef IPropertyItem * (__cdecl *FunCreatePropItem)(SPropertyGrid *);
    
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
        virtual void DrawItem(IRenderTarget *pRT,CRect rc) =0;
        
        virtual LRESULT OnKeyEvent(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL & bHandled) =0;
        virtual LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL & bHandled) =0;
        virtual void OnSetFocus() =0;
        virtual void OnKillFocus() =0;
    };

    class SPropertyItemBase : public TObjRefImpl<IPropertyItem>
                            , public SObject
    {
    public:
        virtual ~SPropertyItemBase();
        
        virtual BOOL IsGroup() const {return FALSE;}
        virtual int  GetLevel() const ;
        virtual BOOL IsExpand() const ;
        virtual void Expand(BOOL bExpend) ;

        virtual IPropertyItem * GetParent() const ;
        virtual void SetParent(IPropertyItem * pParent);
        virtual IPropertyItem * GetItem(PROPITEMTYPE type) const ;
        virtual SPropertyGrid * GetOwner() const ;
        virtual BOOL InsertChild(IPropertyItem * pChild,IPropertyItem * pInsertAfter=IC_LAST);
        virtual BOOL RemoveChild(IPropertyItem * pChild);
        virtual int ChildrenCount() const;
        

        virtual SStringT GetName() const{return m_strName;}
        virtual void SetName(const SStringT & strName){m_strName=strName;}
        virtual SStringT GetDescription() const {return m_strDescription;}
        virtual void SetDescription(const SStringT & strDescription){m_strDescription =strDescription;}
        virtual SStringT GetValue() const {return _T("");}

        virtual void DrawItem(IRenderTarget *pRT,CRect rc){}

        virtual LRESULT OnKeyEvent(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL & bHandled)
        {
            return 0;
        }
        virtual LRESULT OnMouseEvent(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL & bHandled)
        {
            return 0;
        }

        virtual void OnSetFocus() {}
        virtual void OnKillFocus() {}

        SOUI_ATTRS_BEGIN()
            ATTR_STRINGT(L"Name",m_strName,TRUE)
            ATTR_STRINGT(L"descption",m_strDescription,TRUE)
        SOUI_ATTRS_END()

    protected:
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);

        SStringT        m_strName;
        SStringT        m_strDescription;
        
        SPropertyGrid * m_pOwner;
        IPropertyItem * m_pParent;
        
        typedef IPropertyItem* IPropertyItemPtr;
        typedef SList<IPropertyItemPtr> PropItemList;
        PropItemList    m_childs;
        
        BOOL            m_bExpanded;
        
    protected:
        SPropertyItemBase(SPropertyGrid * pOwner):m_pOwner(pOwner),m_pParent(NULL),m_bExpanded(FALSE){
            SASSERT(pOwner);
        }
    };
    
    class SPropertyItemText : public SPropertyItemBase
    {
        SOUI_CLASS_NAME(SPropertyGroup,L"proptext")
    public:
        virtual void DrawItem(IRenderTarget *pRT,CRect rc);
        
        void SetValue(const SStringT & strValue){m_strValue = strValue;}
        virtual SStringT GetValue() const {return m_strValue;}
       
        SOUI_ATTRS_BEGIN()
            ATTR_STRINGT(L"value",m_strValue,TRUE)
        SOUI_ATTRS_END()

    protected:
        SStringT m_strValue;
    public:
        static IPropertyItem * CreatePropItem(SPropertyGrid *pOwner)
        {
            return new SPropertyItemText(pOwner);
        }
    protected:
        SPropertyItemText(SPropertyGrid *pOwner):SPropertyItemBase(pOwner)
        {
        }
    };
     
    class SPropertyGroup : public SPropertyItemBase
    {
        SOUI_CLASS_NAME(SPropertyGroup,L"propgroup")
    public:
        virtual BOOL IsGroup() const {return TRUE;}
        virtual void DrawItem(IRenderTarget *pRT,CRect rc);
                
        static IPropertyItem * CreatePropItem(SPropertyGrid *pOwner)
        {
            return new SPropertyGroup(pOwner);
        }
        
    protected:
        SPropertyGroup(SPropertyGrid *pOwner):SPropertyItemBase(pOwner)
        {
        }
    };
    
    template<class T>
    T * CreatePropItem(SPropertyGrid *pOwner)
    {
        return static_cast<T*>(T::CreatePropItem(pOwner));
    }
    
    class SPropItemMap : public SMap<SStringW,FunCreatePropItem>
    {
    public:
        static void RegPropItem(const SStringW & strName, FunCreatePropItem funCreate);
        static IPropertyItem * CreatePropItem(const SStringW & strName,SPropertyGrid *pOwner );
    protected:
        SPropItemMap();
        
        static SPropItemMap s_mapPropItem;
    };
    
    #define IG_FIRST (SPropertyGroup*)0
    #define IG_LAST  (SPropertyGroup*)1
    
    class SPropertyGrid : protected SListBox
    {
        SOUI_CLASS_NAME(SPropertyGrid, L"propgrid")
    public:
        enum ORDERTYPE
        {
            OT_NULL,
            OT_GROUP,
            OT_NAME,
        };
        SPropertyGrid(void);
        ~SPropertyGrid(void);
        
        int GetIndent();
        
        void OnItemExpanded(IPropertyItem *pItem);
        CRect GetItemRect(IPropertyItem *pItem) const;
        
        void SortInsert(IPropertyItem *pItem);
        
        BOOL InsertGroup(SPropertyGroup * pGroup,SPropertyGroup* pInertAfter=IG_LAST);
        
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"indent",m_nIndent,TRUE)
            ATTR_INT(L"NameWidth",m_nNameWidth,TRUE)
            ATTR_ENUM_BEGIN(L"orderType",ORDERTYPE,TRUE)
                ATTR_ENUM_VALUE(L"null",OT_NULL)
                ATTR_ENUM_VALUE(L"group",OT_GROUP)
                ATTR_ENUM_VALUE(L"name",OT_NAME)
            ATTR_ENUM_END(m_orderType)
            ATTR_SKIN(L"switchSkin",m_switchSkin,TRUE)
        SOUI_ATTRS_END()
        
    protected:
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);
        
        virtual void DrawItem(IRenderTarget *pRT, CRect &rc, int iItem);
        virtual UINT OnGetDlgCode(){return SC_WANTALLKEYS;}
        
        void OnLButtonDblClk(UINT nFlags, CPoint point);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
        SOUI_MSG_MAP_END()
    protected:
        int ExpandChildren(const IPropertyItem *pItem,int iInsert);
        void CollapseChildren(const IPropertyItem *pItem,int idx);
        
        int IndexOfPropertyItem(const IPropertyItem *pItem) const;
        
        int m_nIndent;          //缩进大小
        int m_nNameWidth;    //属性名占用空间
        ORDERTYPE   m_orderType;
        SList<SPropertyGroup *> m_lstGroup; //根分类列表
        ISkinObj  *  m_switchSkin;
        
        static SMap<SStringW, FunCreatePropItem> s_mapProps;
    };
    
}
