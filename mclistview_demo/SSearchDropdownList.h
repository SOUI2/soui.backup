#pragma once

namespace SOUI
{
    struct IDropdownList
    {
        virtual BOOL Create(pugi::xml_node popupStyle) PURE;
        virtual SIZE GetDesiredSize() PURE;
    };

    
    class EventFillSearchDropdownList : public TplEventArgs<EventFillSearchDropdownList>
    {
        SOUI_CLASS_NAME(EventFillSearchDropdownList,L"on_fill_search_dropdown_list")
    public:
        EventFillSearchDropdownList(SObject *pSender):TplEventArgs<EventFillSearchDropdownList>(pSender),bPopup(false),pDropdownWnd(NULL){}
        enum{EventID=EVT_EXTERNAL_BEGIN + 30000};
        
        SStringT    strKey;
        SDropDownWnd * pDropdownWnd;
        bool        bPopup;
    };
        
        
    class EventDropdownListSelected : public TplEventArgs<EventDropdownListSelected>
    {
        SOUI_CLASS_NAME(EventDropdownListSelected,L"on_dropdown_list_selected")
    public:
        EventDropdownListSelected(SObject *pSender):TplEventArgs<EventDropdownListSelected>(pSender),nValue(-1),pDropdownWnd(NULL){}
        enum{EventID=EVT_EXTERNAL_BEGIN + 30001};
        
        SDropDownWnd * pDropdownWnd;
        int nValue;
    };

    class SDropdownList : public SDropDownWnd
                        , public IDropdownList
    {
    public:
        SDropdownList(ISDropDownOwner * pOwner);

        virtual BOOL Create(pugi::xml_node popupStyle) ;
        virtual SIZE GetDesiredSize();

        virtual int GetValue() const;
    protected:
        BOOL SDropdownList::PreTranslateMessage( MSG* pMsg )
        {
            if(SDropDownWnd::PreTranslateMessage(pMsg))
                return TRUE;

            if(pMsg->message==WM_MOUSEWHEEL 
                || ((pMsg->message == WM_KEYDOWN || pMsg->message==WM_KEYUP) && (pMsg->wParam == VK_UP || pMsg->wParam==VK_DOWN || pMsg->wParam==VK_RETURN || pMsg->wParam==VK_ESCAPE)))
            {//截获滚轮及上下键消息
                CSimpleWnd::SendMessage(pMsg->message,pMsg->wParam,pMsg->lParam);
                return TRUE;    
            }
            return FALSE;
        }
        
        SListView * m_pListView;
    };
        
    class SSearchDropdownList : public SWindow , public ISDropDownOwner
    {
    enum{
    DROPALIGN_LEFT,
    DROPALIGN_RIGHT,
    };
    protected:
    SOUI_CLASS_NAME(SSearchDropdownList,L"searchDropdownList")
    
        SDropdownList * m_pDropDownWnd;  /**< DropDown指针 */
        int             m_nDropAlign;
        int             m_nMaxDropHeight;
        
        pugi::xml_document  m_xmlDropdown;
        
    public:
        SSearchDropdownList(void);
        ~SSearchDropdownList(void);
        
        void SetDropdownList(IDropdownList *p);
        
    public:
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"dropAlign", m_nDropAlign, FALSE)
            ATTR_INT(L"maxDropHeight",m_nMaxDropHeight,FALSE)
        SOUI_ATTRS_END()

    protected:
        bool OnEditNotify(EventArgs *e);

    protected:
        virtual BOOL CreateChildren(pugi::xml_node xmlNode);
        
        virtual BOOL FireEvent(EventArgs &evt);
        
        virtual SWindow * GetDropDownOwner();

        virtual void OnCreateDropDown(SDropDownWnd *pDropDown);

        virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);
        
    protected:
        void AdjustDropdownList();
        
        void CloseUp(UINT uCode);
    };

}
