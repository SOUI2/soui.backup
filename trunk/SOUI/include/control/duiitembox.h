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
    : public SScrollView
{
    SOUI_CLASS_NAME(CDuiItemBox, L"itembox")
public:
    CDuiItemBox();
    virtual ~CDuiItemBox() {}

    SWindow* InsertItem(LPCWSTR pszXml,int iItem=-1,BOOL bEnsureVisible=FALSE);

    SWindow* InsertItem(pugi::xml_node xmlNode, int iItem=-1,BOOL bEnsureVisible=FALSE);

    BOOL RemoveItem(UINT iItem);

    BOOL RemoveItem(SWindow * pChild);

    BOOL SetNewPosition(SWindow * pChild, DWORD nPos, BOOL bEnsureVisible = TRUE);

    void RemoveAllItems();

    UINT GetItemCount();

    void PageUp();

    void PageDown();

    void EnsureVisible(SWindow *pItem);

    int GetItemPos(SWindow * lpCurItem);

protected:
    int m_nItemWid,m_nItemHei;
    int m_nSepWid,m_nSepHei;

    void UpdateScroll();
    CRect GetItemRect(int iItem);

    void BringWindowAfter(SWindow * pChild, SWindow * pInsertAfter);

    void OnSize(UINT nType, CSize size);

    virtual void UpdateChildrenPosition(){}//leave it empty

    void ReLayout();
    virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

    virtual int GetScrollLineSize(BOOL bVertical);

    virtual BOOL CreateChildren(pugi::xml_node xmlNode);

    SOUI_ATTRS_BEGIN()
    ATTR_INT(L"itemwid", m_nItemWid, TRUE)
    ATTR_INT(L"itemhei", m_nItemHei, TRUE)
    ATTR_INT(L"sepwid", m_nSepWid, TRUE)
    ATTR_INT(L"sephei", m_nSepHei, TRUE)
    SOUI_ATTRS_END()

    WND_MSG_MAP_BEGIN()
    MSG_WM_SIZE(OnSize)
    WND_MSG_MAP_END()

};

}//namespace SOUI