#include "souistd.h"
#include "event/NotifyCenter.h"

namespace SOUI{

template<> SNotifyCenter * SSingleton<SNotifyCenter>::ms_Singleton = 0;


VOID SNotifyCenter::OnTimer( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	getSingleton().ExecutePendingEvents();
}

SNotifyCenter::SNotifyCenter(void):m_evtPending(NULL)
{
	m_dwMainTrdID = GetCurrentThreadId();
	m_timerID = ::SetTimer(NULL,0,20,OnTimer);
}

SNotifyCenter::~SNotifyCenter(void)
{
	::KillTimer(NULL,m_timerID);
	SAutoLock lock(m_cs);
	if(m_evtPending)
	{
		SPOSITION pos = m_evtPending->GetTailPosition();
		while(pos)
		{
			EventArgs *e = m_evtPending->GetPrev(pos);
			e->Release();
		}
		delete m_evtPending;
	}
}

void SNotifyCenter::FireEventSync( EventArgs *e )
{
	SASSERT(m_dwMainTrdID == GetCurrentThreadId());
	OnFireEvent(e);
}

void SNotifyCenter::FireEventAsync( EventArgs *e )
{
	SAutoLock lock(m_cs);
	if(!m_evtPending)
	{
		m_evtPending = new SList<EventArgs*>;
	}
	m_evtPending->AddTail(e);
    e->AddRef();
}

void SNotifyCenter::ExecutePendingEvents()
{
	SASSERT(m_dwMainTrdID == GetCurrentThreadId());
	SList<EventArgs*> *pEvtList = NULL;
	{
		SAutoLock lock(m_cs);
		if(m_evtPending)
		{
			pEvtList = m_evtPending;
			m_evtPending = NULL;
		}
	}
	
	if(!pEvtList) return;

	SPOSITION pos = pEvtList->GetHeadPosition();
	while(pos)
	{
		EventArgs *e = pEvtList->GetNext(pos);
		OnFireEvent(e);
        e->Release();
	}
	delete pEvtList;
}

void SNotifyCenter::OnFireEvent( EventArgs *e )
{
	FireEvent(*e);
	if(!e->bubbleUp) return ;
	
	SPOSITION pos = m_evtHandlerMap.GetTailPosition();
	while(pos)
	{
		ISlotFunctor * pSlot = m_evtHandlerMap.GetPrev(pos);
		(*pSlot)(e);
		if(!e->bubbleUp) break;
	}
}

bool SNotifyCenter::RegisterEventMap( const ISlotFunctor &slot )
{
	for(SPOSITION pos = m_evtHandlerMap.GetHeadPosition();pos;)
	{
		ISlotFunctor * pSlot = m_evtHandlerMap.GetNext(pos);
		if(pSlot->Equal(slot)) return false;
	}
	m_evtHandlerMap.AddTail(slot.Clone());
	return true;
}

bool SNotifyCenter::UnregisterEventMap( const ISlotFunctor &slot )
{
	for(SPOSITION pos = m_evtHandlerMap.GetHeadPosition();pos;)
	{
		SPOSITION posPrev = pos;
		ISlotFunctor * pSlot = m_evtHandlerMap.GetNext(pos);
		if(pSlot->Equal(slot))
		{
			m_evtHandlerMap.RemoveAt(posPrev);
			delete pSlot;
			return true;
		}
	}
	return false;
}

}