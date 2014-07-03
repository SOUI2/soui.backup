#include "duistd.h"
#include <event/EventSet.h>

namespace SOUI
{
    //////////////////////////////////////////////////////////////////////////
    // CDuiEvent
    bool SEvent::subscribe( const SlotFunctorBase& slot )
    {
        if(findSlotFunctor(slot) != -1) return false;
        m_evtSlots.Add(slot.Clone());
        return true;
    }

    bool SEvent::unsubscribe( const SlotFunctorBase& slot )
    {
        int idx=findSlotFunctor(slot);
        if(idx==-1) return false;

        delete m_evtSlots[idx];
        m_evtSlots.RemoveAt(idx);
        return true;
    }

    int SEvent::findSlotFunctor( const SlotFunctorBase& slot )
    {
        for(UINT i=0;i<m_evtSlots.GetCount();i++)
        {
            if(m_evtSlots[i]->Equal(slot))
            {
                return i;
            }
        }
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////
    // CDuiEventSet
    SEventSet::SEventSet(void):m_bMuted(FALSE)
    {
    }

    SEventSet::~SEventSet(void)
    {
        removeAllEvents();
    }

    SEvent * SEventSet::GetEventObject(const DWORD dwEventID )
    {
        for(UINT i=0;i<m_evtArr.GetCount();i++)
        {
            if(m_evtArr[i]->GetEventID()==dwEventID) return m_evtArr[i];
        }
        return NULL;
    }

    void SEventSet::FireEvent( const DWORD dwEventID, EventArgs& args )
    {
        // find event object
        SEvent* ev = GetEventObject(dwEventID);

        // fire the event if present and set is not muted
        if ((ev != 0) && !m_bMuted)
        {
            (*ev)(args);
        }
    }

    void SEventSet::addEvent( const DWORD dwEventID )
    {
        if(!isEventPresent(dwEventID))
        {
            m_evtArr.Add(new SEvent(dwEventID));
        }
    }

    void SEventSet::removeEvent( const DWORD dwEventID )
    {
        for(UINT i=0;i<m_evtArr.GetCount();i++)
        {
            if(m_evtArr[i]->GetEventID()==dwEventID)
            {
                delete m_evtArr[i];
                m_evtArr.RemoveAt(i);
                return;
            }
        }
    }

    bool SEventSet::isEventPresent( const DWORD dwEventID )
    {
        return GetEventObject(dwEventID)!=NULL;
    }

    void SEventSet::removeAllEvents( void )
    {
        for(UINT i=0;i<m_evtArr.GetCount();i++)
        {
            delete m_evtArr[i];
        }
        m_evtArr.RemoveAll();
    }

    bool SEventSet::subscribeEvent( const DWORD dwEventID, const SlotFunctorBase & subscriber )
    {
        if(!isEventPresent(dwEventID)) return false;
        return GetEventObject(dwEventID)->subscribe(subscriber);
    }

    bool SEventSet::unsubscribeEvent( const DWORD dwEventID, const SlotFunctorBase & subscriber )
    {
        if(!isEventPresent(dwEventID)) return false;
        return GetEventObject(dwEventID)->unsubscribe(subscriber);
    }


}//end of namespace