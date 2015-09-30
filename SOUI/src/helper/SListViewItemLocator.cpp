#include "souistd.h"
#include "helper/SListViewItemLocator.h"
#include <math.h>

namespace SOUI
{
    //////////////////////////////////////////////////////////////////////////
    // SListViewItemLocatorFix
    SListViewItemLocatorFix::SListViewItemLocatorFix(int nItemHei,int nDividerSize) 
        :m_nItemHeight(nItemHei)
        ,m_nDividerSize(nDividerSize)
    {

    }

    int SListViewItemLocatorFix::GetScrollLineSize() const
    {
        return GetFixItemHeight();
    }

    int SListViewItemLocatorFix::Position2Item(int position)
    {
        if(!m_adapter) return -1;
        int nRet = position/GetFixItemHeight();

        if(nRet<0) nRet =0;
        if(nRet>m_adapter->getCount()) nRet = m_adapter->getCount();
        return nRet;
    }

    int SListViewItemLocatorFix::Item2Position(int iItem)
    {
        return iItem * GetFixItemHeight();
    }

    int SListViewItemLocatorFix::GetTotalHeight()
    {
        if(!m_adapter || m_adapter->getCount() == 0) return 0;
        return m_nItemHeight * m_adapter->getCount() + (m_adapter->getCount()-1)*m_nDividerSize;
    }

    void SListViewItemLocatorFix::SetItemHeight(int iItem,int nHeight)
    {
    }

    int SListViewItemLocatorFix::GetItemHeight(int iItem) const
    {
        return m_nItemHeight;
    }

    bool SListViewItemLocatorFix::IsFixHeight() const
    {
        return true;
    }

    void SListViewItemLocatorFix::SetAdapter(IAdapter *pAdapter)
    {
        m_adapter = pAdapter;
    }


    //////////////////////////////////////////////////////////////////////////
    //  SListViewItemLocatorFlex
    double logbase(double a, double base)
    {
        return log(a) / log(base);
    }

#define SEGMENT_SIZE    50  //数据分组最大长度
#define INDEX_WIDTH     10  //索引表一级最大节点数

    SListViewItemLocatorFlex::SListViewItemLocatorFlex(int nItemHei,int nDividerSize) 
        :m_nItemHeight(nItemHei)
        ,m_nDividerSize(nDividerSize)
    {

    }

    SListViewItemLocatorFlex::~SListViewItemLocatorFlex()
    {
        Clear();
    }

    int SListViewItemLocatorFlex::GetScrollLineSize() const
    {
        return GetFixItemHeight();
    }

    int SListViewItemLocatorFlex::Position2Item(int position)
    {
        if(!m_adapter) return -1;
        if(position<0 || position>=GetTotalHeight()) 
            return -1;
        HSTREEITEM hItem = Offset2Branch(STVI_ROOT,position);
        SASSERT(hItem);
        int idx = Branch2Index(hItem);
        int offset = Branch2Offset(hItem);
        BranchInfo &bi = m_itemPosIndex.GetItemRef(hItem);
        SASSERT(bi.nBranchHei>=position-offset);

        int iSeg = idx/SEGMENT_SIZE;
        int nRemain = position - offset;

        SegmentInfo *psi = m_segments[iSeg];

        for(int i=0;i<psi->nItems;i++)
        {
            int nItemHei = psi->pItemHeight[i]==-1?GetFixItemHeight():psi->pItemHeight[i];
            if(nRemain<=nItemHei) return idx + i;
            nRemain -= nItemHei;
        }

        SASSERT(FALSE);
        return -1;
    }

    int SListViewItemLocatorFlex::Item2Position(int iItem)
    {
        if(!m_adapter) return 0;
        int iSeg = iItem/SEGMENT_SIZE;
        int iSubItem = iItem%SEGMENT_SIZE;
        SegmentInfo *psi = m_segments[iSeg];

        int nPos = Branch2Offset(psi->hItem);
        for(int i=0;i<iSubItem;i++)
        {
            nPos += psi->pItemHeight[i]==-1?GetFixItemHeight():psi->pItemHeight[i];
        }
        return nPos;
    }

    int SListViewItemLocatorFlex::GetTotalHeight()
    {
        if(!m_adapter) return 0;
        HSTREEITEM hItem = m_itemPosIndex.GetRootItem();
        int nRet = m_itemPosIndex.GetItem(hItem).nBranchHei;
        if(m_adapter->getCount()>0) nRet -= m_nDividerSize;
        return nRet;
    }

    void SListViewItemLocatorFlex::SetItemHeight(int iItem,int nHeight)
    {
        if(!m_adapter) return;

        int iSeg = iItem/SEGMENT_SIZE;
        int iSubItem = iItem%SEGMENT_SIZE;
        SegmentInfo *psi = m_segments[iSeg];
        int nOldHei = psi->pItemHeight[iSubItem];
        if(nOldHei==-1) nOldHei = GetFixItemHeight();

        nHeight += m_nDividerSize;
        psi->pItemHeight[iSubItem] = nHeight;
        if(nOldHei != nHeight)
        {
            int nHeiDif = nHeight - nOldHei;
            HSTREEITEM hBranch = psi->hItem;
            while(hBranch)
            {
                BranchInfo & bi = m_itemPosIndex.GetItemRef(hBranch);
                bi.nBranchHei += nHeiDif;
                hBranch = m_itemPosIndex.GetParentItem(hBranch);
            }
        }
    }

    int SListViewItemLocatorFlex::GetItemHeight(int iItem) const
    {
        if(!m_adapter) return 0;
        int iSeg = iItem/SEGMENT_SIZE;
        int iSubItem = iItem%SEGMENT_SIZE;
        SegmentInfo *psi = m_segments[iSeg];
        int nRet = psi->pItemHeight[iSubItem];
        if(nRet == -1) nRet = GetFixItemHeight();
        nRet -= m_nDividerSize;
        return nRet;
    }

    bool SListViewItemLocatorFlex::IsFixHeight() const
    {
        return false;
    }

    void SListViewItemLocatorFlex::SetAdapter(IAdapter *pAdapter)
    {
        m_adapter = pAdapter;
        OnDataSetChanged();
    }

    void SListViewItemLocatorFlex::OnDataSetChanged()
    {
        Clear();
        if(m_adapter)
        {
            int nTreeSize =  m_adapter->getCount();
            int nBranchSize = SEGMENT_SIZE;
            int nTreeDeep = GetIndexDeep();
            for(int i=0;i<nTreeDeep;i++)
                nBranchSize *= INDEX_WIDTH;
            InitIndex(STVI_ROOT,nTreeSize,nBranchSize);
        }
    }

    void SListViewItemLocatorFlex::InitIndex(HSTREEITEM hParent,int nItems,int nBranchSize)
    {
        BranchInfo bi;
        bi.nBranchHei = nItems*GetFixItemHeight();
        bi.nBranchSize = nItems;

        HSTREEITEM hBranch = m_itemPosIndex.InsertItem(bi,hParent);
        if(nItems > SEGMENT_SIZE)
        {//插入子节点
            int nRemain = nItems;
            int nSubBranchSize = nBranchSize/INDEX_WIDTH;
            while(nRemain>0)
            {
                int nItems2 = nSubBranchSize;
                if(nItems2>nRemain) nItems2 = nRemain;
                InitIndex(hBranch,nItems2,nSubBranchSize);
                nRemain -= nItems2;
            }
        }else
        {
            m_segments.Add(new SegmentInfo(nItems,hBranch));
        }
    }

    int SListViewItemLocatorFlex::GetIndexDeep() const
    {
        if(!m_adapter) return 0;
        if(m_adapter->getCount()==0) return 0;
        return (int)ceil(logbase((m_adapter->getCount()+SEGMENT_SIZE-1)/SEGMENT_SIZE,INDEX_WIDTH));
    }

    void SListViewItemLocatorFlex::Clear()
    {
        m_itemPosIndex.DeleteAllItems();
        for(int i=0;i<(int)m_segments.GetCount();i++)
        {
            delete m_segments[i];
        }
        m_segments.RemoveAll();
    }

    int SListViewItemLocatorFlex::Branch2Offset(HSTREEITEM hBranch) const
    {
        int nOffset = 0;
        HSTREEITEM hPrev = m_itemPosIndex.GetPrevSiblingItem(hBranch);
        while(hPrev)
        {
            nOffset += m_itemPosIndex.GetItem(hPrev).nBranchHei;
            hPrev = m_itemPosIndex.GetPrevSiblingItem(hPrev);
        }
        HSTREEITEM hParent = m_itemPosIndex.GetParentItem(hBranch);
        if(hParent)
        {
            nOffset += Branch2Offset(hParent);
        }
        return nOffset;
    }

    int SListViewItemLocatorFlex::Branch2Index(HSTREEITEM hBranch) const
    {
        int iIndex = 0;
        HSTREEITEM hPrev = m_itemPosIndex.GetPrevSiblingItem(hBranch);
        while(hPrev)
        {
            iIndex += m_itemPosIndex.GetItem(hPrev).nBranchSize;
            hPrev = m_itemPosIndex.GetPrevSiblingItem(hPrev);
        }
        HSTREEITEM hParent = m_itemPosIndex.GetParentItem(hBranch);
        if(hParent)
        {
            iIndex += Branch2Index(hParent);
        }
        return iIndex;
    }

    HSTREEITEM SListViewItemLocatorFlex::Offset2Branch(HSTREEITEM hParent,int nOffset)
    {
        HSTREEITEM hItem = m_itemPosIndex.GetChildItem(hParent);
        if(!hItem) return hParent;

        while(hItem)
        {
            BranchInfo bi = m_itemPosIndex.GetItem(hItem);
            if(nOffset>bi.nBranchHei)
            {
                nOffset -= bi.nBranchHei;
                hItem = m_itemPosIndex.GetNextSiblingItem(hItem);
            }else
            {
                return Offset2Branch(hItem,nOffset);
            }
        }
        return NULL;
    }
}