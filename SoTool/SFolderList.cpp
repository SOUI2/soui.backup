#include "stdafx.h"
#include "SFolderList.h"

namespace SOUI
{
    SFolderTreeCtrl::SFolderTreeCtrl()
    {
    }

    SFolderTreeCtrl::~SFolderTreeCtrl()
    {

    }

    void SFolderTreeCtrl::OnFinalRelease()
    {
        delete this;
    }

    void SFolderTreeCtrl::OnNodeFree(LPTVITEM & pItemData)
    {
        MCITEM *pMcItem = (MCITEM*)pItemData->lParam;
        delete (FILEITEMINFO*)pMcItem->lParam;
        delete pMcItem;
        STreeCtrl::OnNodeFree(pItemData);
    }

    void SFolderTreeCtrl::OnInsertItem( LPTVITEM & pItemData )
    {
        HSTREEITEM hItem = pItemData->hItem;
        FILEITEMINFO * pFi = GetFileInfo(hItem);
        pFi->percent = 50;//todo: update percent of ascendant items
    }

    void SFolderTreeCtrl::DrawItem(IRenderTarget *pRT, CRect & rc, HSTREEITEM hItem)
    {
        CRect rcTreeItem = rc;
        rcTreeItem.right = rcTreeItem.left + m_nTreeWidth;
        STreeCtrl::DrawItem(pRT,rcTreeItem,hItem);
        
        CRect rcListItem = rc;
        rcListItem.left = rcTreeItem.right;
        DrawListItem(pRT,rcListItem,hItem);
    }

    void SFolderTreeCtrl::DrawListItem(IRenderTarget *pRT, CRect & rc,HSTREEITEM hItem)
    {
        FILEITEMINFO *pFi = GetFileInfo(hItem);
        CRect rcItem = rc;
        rcItem.right = rcItem.left + m_arrColWidth[0];
        SStringT strTxt = SStringT().Format(_T("%d"),pFi->nSize);        
        pRT->DrawText(strTxt,strTxt.GetLength(),&rcItem,DT_SINGLELINE|DT_VCENTER);

        rcItem.left =rcItem.right;
        rcItem.right = rcItem.left + m_arrColWidth[1];
        
        pRT->FillSolidRect(rcItem,RGBA(0,0,128,255));
        CRect rcPercent = rcItem;
        rcPercent.right = rcItem.left + rcItem.Width()*pFi->percent/100;
        pRT->FillSolidRect(rcPercent,RGBA(128,0,0,255));
        strTxt.Format(_T("%d%%"),pFi->percent);
        pRT->DrawText(strTxt,strTxt.GetLength(),&rcItem,DT_SINGLELINE|DT_VCENTER);
    }

    HSTREEITEM SFolderTreeCtrl::InsertItem( LPCTSTR pszFileName,BOOL bFolder,__int64 fileSize,HSTREEITEM hParent )
    {
        FILEITEMINFO *pFi = new FILEITEMINFO;
        pFi->nSize = fileSize;
        pFi->percent=-1;
        return SMCTreeCtrl::InsertItem(pszFileName,0,0,(LPARAM)pFi,hParent);
    }

    SFolderTreeCtrl::FILEITEMINFO * SFolderTreeCtrl::GetFileInfo( HSTREEITEM hItem )
    {
        return (FILEITEMINFO*)SMCTreeCtrl::GetItemData(hItem);
    }
    //////////////////////////////////////////////////////////////////////////
    SFolderTreeList::SFolderTreeList(void):m_pFolderTreectrl(NULL)
    {
    }

    SFolderTreeList::~SFolderTreeList(void)
    {
    }

    BOOL SFolderTreeList::CreateChildren(pugi::xml_node xmlNode)
    {
        BOOL bRet = STreeList::CreateChildren(xmlNode);
        if(!bRet)
            return FALSE;
        
        m_pFolderTreectrl = sobj_cast<SFolderTreeCtrl>(GetMCTreeCtrl());
        SASSERT(m_pFolderTreectrl);
        SASSERT(m_pHeader->GetItemCount() == 3);

        return TRUE;
    }

    bool SFolderTreeList::OnHeaderClick(EventArgs *pEvt)
    {
        return true;
    }

    SMCTreeCtrl * SFolderTreeList::CreateMcTreeCtrl()
    {
        return new SFolderTreeCtrl;
    }
}
