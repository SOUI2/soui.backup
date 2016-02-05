#pragma once

#include "interface/Adapter-i.h"
#include <souicoll.h>
#include "helper/copylist.hpp"
#include "control/stree.hpp"

namespace SOUI
{
    class SLvObserverMgr
    {
    public:
        SLvObserverMgr(){}
        ~SLvObserverMgr(){
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->Release();
            }
            m_lstObserver.RemoveAll();
        }

        void registerObserver(ILvDataSetObserver * observer) {
            SASSERT(observer);
            if(m_lstObserver.Find(observer)) return;
            m_lstObserver.AddTail(observer);
            observer->AddRef();
        }

        /**
        * Removes a previously registered observer. The observer must not be null and it
        * must already have been registered.
        * @param observer the observer to unregister
        * @throws IllegalArgumentException the observer is null
        * @throws IllegalStateException the observer is not yet registered
        */
        void unregisterObserver(ILvDataSetObserver * observer) {
            SASSERT(observer);
            SPOSITION pos = m_lstObserver.Find(observer);
            if(!pos) return;
            m_lstObserver.RemoveAt(pos);
            observer->Release();
        }
        
        void notifyChanged()
        {
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->onChanged();
            }
        }
        
        void notifyInvalidated()
        {
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                ILvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->onInvalidated();
            }
        }
    protected:
        SList<ILvDataSetObserver *> m_lstObserver;
    };
    
    template<class BaseClass>
    class LvAdatperImpl : public BaseClass
    {
    public:
        void notifyDataSetChanged() {
            m_obzMgr.notifyChanged();
        }

        /**
        * Notifies the attached observers that the underlying data is no longer valid
        * or available. Once invoked this adapter is no longer valid and should
        * not report further data set changes.
        */
        void notifyDataSetInvalidated() {
            m_obzMgr.notifyInvalidated();
        }

        virtual void registerDataSetObserver(ILvDataSetObserver * observer)
        {
            m_obzMgr.registerObserver(observer);
        }

        virtual void unregisterDataSetObserver(ILvDataSetObserver * observer)
        {
            m_obzMgr.unregisterObserver(observer);
        }

        virtual int getCount() PURE;   

        virtual long getItemId(int position)
        {
            return position;
        }

        virtual int getItemViewType(int position)
        {
            return 0;
        }

        virtual int getViewTypeCount()
        {
            return 1;
        }

        virtual bool isEmpty()
        {
            return getCount()>0;
        }

        virtual ULONG_PTR getItemData(int position){
            return 0;
        }
        
        virtual SStringT getItemDesc(int position){
            return SStringT();
        }
    protected:
        SLvObserverMgr    m_obzMgr;
    };
    
    class SAdapterBase : public TObjRefImpl<LvAdatperImpl<ILvAdapter>>
    {
    public:
        SAdapterBase()
        {
        }

        ~SAdapterBase()
        {
        }
    };
    
    class SMcAdapterBase : public TObjRefImpl<LvAdatperImpl<IMcAdapter>>
    {
    public:
        SMcAdapterBase()
        {
        }

        ~SMcAdapterBase()
        {
        }
        
        virtual bool OnSort(int iCol,SHDSORTFLAG * stFlags,int nCols){return false;}
    };


    class STvObserverMgr
    {
    public:
        STvObserverMgr(){}
        ~STvObserverMgr(){
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->Release();
            }
            m_lstObserver.RemoveAll();
        }

        void registerObserver(ITvDataSetObserver * observer) {
            SASSERT(observer);
            if(m_lstObserver.Find(observer)) return;
            m_lstObserver.AddTail(observer);
            observer->AddRef();
        }

        /**
        * Removes a previously registered observer. The observer must not be null and it
        * must already have been registered.
        * @param observer the observer to unregister
        * @throws IllegalArgumentException the observer is null
        * @throws IllegalStateException the observer is not yet registered
        */
        void unregisterObserver(ITvDataSetObserver * observer) {
            SASSERT(observer);
            SPOSITION pos = m_lstObserver.Find(observer);
            if(!pos) return;
            m_lstObserver.RemoveAt(pos);
            observer->Release();
        }
        
        void notifyChanged(HTREEITEM hBranch)
        {
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->onChanged(hBranch);
            }
        }
        
        void notifyInvalidated(HTREEITEM hBranch)
        {
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                ITvDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->onInvalidated(hBranch);
            }
        }
    protected:
        SList<ITvDataSetObserver *> m_lstObserver;
    };


    template<class BaseClass>
    class TvAdatperImpl : public BaseClass
    {
    public:
        void notifyDataSetChanged(HTREEITEM hBranch) {
            m_obzMgr.notifyChanged(hBranch);
        }

        /**
        * Notifies the attached observers that the underlying data is no longer valid
        * or available. Once invoked this adapter is no longer valid and should
        * not report further data set changes.
        */
        void notifyDataSetInvalidated(HTREEITEM hBranch) {
            m_obzMgr.notifyInvalidated(hBranch);
        }

        virtual void registerDataSetObserver(ITvDataSetObserver * observer)
        {
            m_obzMgr.registerObserver(observer);
        }

        virtual void unregisterDataSetObserver(ITvDataSetObserver * observer)
        {
            m_obzMgr.unregisterObserver(observer);
        }


    protected:
        STvObserverMgr    m_obzMgr;
    };
    
    template<typename T>
	class STreeAdapterBase: public TObjRefImpl<TvAdatperImpl<ITvAdapter>>
	{
	public:
		STreeAdapterBase() {}
		~STreeAdapterBase(){}
		
		struct ItemInfo
		{
		ULONG_PTR userData[DATA_INDEX_NUMBER];
		T         data;
		};
		
		//获取hItem中的指定索引的数据
        virtual ULONG_PTR GetItemDataByIndex(HTREEITEM hItem,DATA_INDEX idx) const
        {
            if(hItem == ITvAdapter::ITEM_ROOT)
                return m_rootUserData[idx];
            ItemInfo & ii = m_tree.GetItemRef((HSTREEITEM)hItem);
            return ii.userData[idx];
        }
        
        //保存hItem指定索引的数据
        virtual void SetItemDataByIndex(HTREEITEM hItem,DATA_INDEX idx,ULONG_PTR data)
        {
            if(hItem == ITvAdapter::ITEM_ROOT)
                m_rootUserData[idx] = data;
            else
            {
                ItemInfo & ii = m_tree.GetItemRef((HSTREEITEM)hItem);
                ii.userData[idx] = data;
            }
        }
        
        virtual HTREEITEM GetParentItem(HTREEITEM hItem) const
        {
            if(hItem == ITEM_ROOT) return ITvAdapter::ITEM_NULL;
            HSTREEITEM hParent = m_tree.GetParentItem((HSTREEITEM)hItem);
            if(hParent == NULL) hParent = ITvAdapter::ITEM_ROOT;
            return (HTREEITEM)hParent;
        }
        
        virtual HTREEITEM GetFirstChildItem(HTREEITEM hItem) const
        {
            SASSERT(hItem != ITvAdapter::ITEM_NULL);
            return (HTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem,TRUE);    
        }
        
        virtual HTREEITEM GetLastChildItem(HTREEITEM hItem) const 
        {
            SASSERT(hItem != ITvAdapter::ITEM_NULL);
            return (HTREEITEM)m_tree.GetChildItem((HSTREEITEM)hItem,FALSE);    
        }
        
        virtual HTREEITEM GetPrevSiblingItem(HTREEITEM hItem) const 
        {
            SASSERT(hItem != ITvAdapter::ITEM_NULL && hItem != ITvAdapter::ITEM_ROOT);
            return (HTREEITEM)m_tree.GetPrevSiblingItem((HSTREEITEM)hItem);
        }
        
        virtual HTREEITEM GetNextSiblingItem(HTREEITEM hItem) const
        {
            SASSERT(hItem != ITvAdapter::ITEM_NULL && hItem != ITvAdapter::ITEM_ROOT);
            return (HTREEITEM)m_tree.GetNextSiblingItem((HSTREEITEM)hItem);
        }
        
        virtual int GetChildrenCount(HTREEITEM hItem) const
        {
            return -1;
        }
        
        virtual HTREEITEM GetChildAt(HTREEITEM hItem,int iChild) const
        {
            return ITvAdapter::ITEM_NULL;
        }
        
        
        virtual int getViewType(HTREEITEM hItem) const
        {
            return 0;
        }
        
		virtual int getViewTypeCount() const
		{
		    return 1;
		}
		
    public:
        HSTREEITEM InsertItem(const T & data,HSTREEITEM hParent = STVI_ROOT,HSTREEITEM hInsertAfter = STVI_LAST)
        {
            ItemInfo ii={0};
            ii.data = data;
            return m_tree.InsertItem(ii,hParent,hInsertAfter);
        }
        
    protected:
		CSTree<ItemInfo> m_tree;
		ULONG_PTR        m_rootUserData[DATA_INDEX_NUMBER];
	};
}
