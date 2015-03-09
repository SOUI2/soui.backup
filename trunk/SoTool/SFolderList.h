#pragma once

#include "STreeList.h"

namespace SOUI
{
    class SFolderTreeCtrl : public SMCTreeCtrl
    {
        SOUI_CLASS_NAME(SFolderTreeCtrl,L"foldertreectrl")
        
        struct FILEITEMINFO
        {
            __int64 nSize;  //总文件夹大小
            int     percent;//占上级文件夹的百分比
        };
    public:
        SFolderTreeCtrl();
        ~SFolderTreeCtrl();
        
        HSTREEITEM InsertItem(LPCTSTR pszFileName,BOOL bFolder,__int64 fileSize,HSTREEITEM hParent);
    protected:
        FILEITEMINFO * GetFileInfo(HSTREEITEM hItem);

        virtual void DrawItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem);
        virtual void DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem);
        virtual void OnNodeFree(LPTVITEM & pItemData);
        virtual void OnInsertItem(LPTVITEM & pItemData);

        virtual void OnFinalRelease();
    };
    
    class SFolderTreeList : public STreeList
    {
    SOUI_CLASS_NAME(SFolderTreeList,L"foldertreelist")
    public:
        SFolderTreeList(void);
        ~SFolderTreeList(void);
                
        SFolderTreeCtrl * GetFolderTreeCtrl() { return m_pFolderTreectrl;}
    protected:
            /**
        * SListCtrl::OnHeaderClick
        * @brief    列表头单击事件 -- 
        * @param    EventArgs *pEvt
        *
        * Describe  列表头单击事件
        */
        bool            OnHeaderClick(EventArgs *pEvt);

    protected:
        virtual BOOL CreateChildren(pugi::xml_node xmlNode);
        virtual SMCTreeCtrl * CreateMcTreeCtrl();

        SFolderTreeCtrl * m_pFolderTreectrl;
    };
}

