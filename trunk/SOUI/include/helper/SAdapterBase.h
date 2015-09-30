#pragma once

#include "interface/Adapter-i.h"
#include <souicoll.h>
#include "helper/copylist.hpp"

namespace SOUI
{
    class SObserverMgr
    {
    public:
        SObserverMgr(){}
        ~SObserverMgr(){
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                IDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->Release();
            }
            m_lstObserver.RemoveAll();
        }

        void registerObserver(IDataSetObserver * observer) {
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
        void unregisterObserver(IDataSetObserver * observer) {
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
                IDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->onChanged();
            }
        }
        
        void notifyInvalidated()
        {
            SPOSITION pos = m_lstObserver.GetHeadPosition();
            while(pos)
            {
                IDataSetObserver *pObserver = m_lstObserver.GetNext(pos);
                pObserver->onInvalidated();
            }
        }
    protected:
        SList<IDataSetObserver *> m_lstObserver;
    };
    
    template<class BaseClass>
    class AdatperImpl : public BaseClass
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

        virtual void registerDataSetObserver(IDataSetObserver * observer)
        {
            m_obzMgr.registerObserver(observer);
        }

        virtual void unregisterDataSetObserver(IDataSetObserver * observer)
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
        SObserverMgr    m_obzMgr;
    };
    
    class SAdapterBase : public TObjRefImpl<AdatperImpl<IAdapter>>
    {
    public:
        SAdapterBase()
        {
        }

        ~SAdapterBase()
        {
        }
    };
    
    class SMcAdapterBase : public TObjRefImpl<AdatperImpl<IMcAdapter>>
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
}
