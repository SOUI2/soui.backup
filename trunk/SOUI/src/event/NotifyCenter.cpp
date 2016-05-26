#include "souistd.h"
#include "event/NotifyCenter.h"

namespace SOUI{

template<> SNotifyCenter * SSingleton<SNotifyCenter>::ms_Singleton = 0;


VOID SNotifyCenter::OnTimer( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
{
	getSingleton().ExecutePendingEvents();
}

SNotifyCenter::SNotifyCenter(void)
{
	m_dwMainTrdID = GetCurrentThreadId();
	m_timerID = ::SetTimer(NULL,0,20,OnTimer);
}

SNotifyCenter::~SNotifyCenter(void)
{
	::KillTimer(NULL,m_timerID);
}

void SNotifyCenter::FireEventSync( EventArgs *e )
{
	SASSERT(m_dwMainTrdID == GetCurrentThreadId());
	OnFireEvent(e);
}

void SNotifyCenter::FireEventAsync( EventArgs *e )
{
	SAutoLock lock(m_cs);
	m_evtPending.AddTail(e);
}

void SNotifyCenter::ExecutePendingEvents()
{
	SASSERT(m_dwMainTrdID == GetCurrentThreadId());
	SAutoLock lock(m_cs);
	SPOSITION pos = m_evtPending.GetHeadPosition();
	while(pos)
	{
		EventArgs *e = m_evtPending.GetNext(pos);
		OnFireEvent(e);
		delete e;
	}
	m_evtPending.RemoveAll();
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