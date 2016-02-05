#pragma once
#include "interface/LvItemLocator-i.h"
#include "control/stree.hpp"

namespace SOUI
{
    class SOUI_EXP SListViewItemLocatorFix : public TObjRefImpl<IListViewItemLocator>
    {
    public:
        SListViewItemLocatorFix(int nItemHei,int nDividerSize=0);

        virtual void SetAdapter(ILvAdapter *pAdapter);

        virtual void OnDataSetChanged(){}

        virtual bool IsFixHeight() const;

        virtual int GetItemHeight(int iItem) const ;

        virtual void SetItemHeight(int iItem,int nHeight);


        virtual int GetTotalHeight();
        virtual int Item2Position(int iItem);

        virtual int Position2Item(int position);

        virtual int GetScrollLineSize() const;

        virtual int GetDividerSize() const
        {
            return m_nDividerSize;
        }

    protected:
        int GetFixItemHeight() const {return m_nItemHeight+m_nDividerSize;}

        int m_nItemHeight;
        int m_nDividerSize;

        CAutoRefPtr<ILvAdapter> m_adapter;
    };

    class SOUI_EXP SListViewItemLocatorFlex : public TObjRefImpl<IListViewItemLocator>
    {

    public:

        SListViewItemLocatorFlex(int nItemHei,int nDividerSize=0);
        ~SListViewItemLocatorFlex();


        virtual void SetAdapter(ILvAdapter *pAdapter);
        virtual void OnDataSetChanged();

        virtual bool IsFixHeight() const;

        virtual int GetItemHeight(int iItem) const;

        virtual void SetItemHeight(int iItem,int nHeight);


        virtual int GetTotalHeight();
        virtual int Item2Position(int iItem);

        virtual int Position2Item(int position);

        virtual int GetScrollLineSize() const;   

        virtual int GetDividerSize() const
        {
            return m_nDividerSize;
        }

    protected:
        void InitIndex(HSTREEITEM hParent,int nItems,int nSubBranchSize);
        int GetFixItemHeight() const {return m_nItemHeight+m_nDividerSize;}
        int GetIndexDeep() const;
        void Clear();
        int Branch2Offset(HSTREEITEM hBranch) const;
        int Branch2Index(HSTREEITEM hBranch) const;
        HSTREEITEM Offset2Branch(HSTREEITEM hParent,int nOffset);

        int m_nItemHeight;  //默认表项高度
        int m_nDividerSize;

        struct BranchInfo
        {
            int nBranchHei; //分枝高度
            int nBranchSize;//分枝中包含的节点数量
        };

        CSTree<BranchInfo>    m_itemPosIndex;//记录分枝高度
        class SegmentInfo
        {
        public:
            SegmentInfo(int nItems,HSTREEITEM hBranch):hItem(hBranch){
                this->nItems = nItems;
                pItemHeight = new int[nItems];
                memset(pItemHeight,0xff,nItems*sizeof(int));
            }
            ~SegmentInfo()
            {
                if(pItemHeight) delete[] pItemHeight;
            }

            HSTREEITEM hItem;
            int        nItems;
            int*       pItemHeight;//段中每一个表项的高度
        };

        SArray<SegmentInfo*>     m_segments;
        CAutoRefPtr<ILvAdapter>   m_adapter;
    };

}