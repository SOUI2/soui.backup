//////////////////////////////////////////////////////////////////////////
//  Class Name: CDuiItemBox
// Description: Items Container
//     Creator: huangjianxiong
//     Version: 2011.7.8 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////


#pragma once
#include "DuiPanel.h"

namespace SOUI
{

class SOUI_EXP CDuiItemBox
    : public CDuiScrollView
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiItemBox, "itembox")
public:
    CDuiItemBox();
    virtual ~CDuiItemBox() {}

    CDuiWindow* InsertItem(LPCWSTR pszXml,int iItem=-1,BOOL bEnsureVisible=FALSE);

	CDuiWindow* InsertItem(pugi::xml_node xmlNode, int iItem=-1,BOOL bEnsureVisible=FALSE);

    BOOL RemoveItem(UINT iItem);

    BOOL RemoveItem(CDuiWindow * pChild);

    BOOL SetNewPosition(CDuiWindow * pChild, DWORD nPos, BOOL bEnsureVisible = TRUE);

    void RemoveAllItems();

    UINT GetItemCount();

    void PageUp();

    void PageDown();

    void EnsureVisible(CDuiWindow *pItem);

    int GetItemPos(CDuiWindow * lpCurItem);

protected:
    int m_nItemWid,m_nItemHei;
    int m_nSepWid,m_nSepHei;

    void UpdateScroll();
    CRect GetItemRect(int iItem);

    void BringWindowAfter(CDuiWindow * pChild, CDuiWindow * pInsertAfter);

    void OnSize(UINT nType, CSize size);

	virtual void UpdateChildrenPosition(){}//leave it empty

    void ReLayout();
    virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

    virtual int GetScrollLineSize(BOOL bVertical);

    virtual BOOL LoadChildren(pugi::xml_node xmlNode);

    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_INT_ATTRIBUTE("itemwid", m_nItemWid, TRUE)
    DUIWIN_INT_ATTRIBUTE("itemhei", m_nItemHei, TRUE)
    DUIWIN_INT_ATTRIBUTE("sepwid", m_nSepWid, TRUE)
    DUIWIN_INT_ATTRIBUTE("sephei", m_nSepHei, TRUE)
    DUIWIN_DECLARE_ATTRIBUTES_END()

    DUIWIN_BEGIN_MSG_MAP()
    MSG_WM_SIZE(OnSize)
    DUIWIN_END_MSG_MAP()

};

}//namespace SOUI