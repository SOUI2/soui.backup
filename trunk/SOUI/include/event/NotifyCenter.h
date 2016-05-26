#pragma once

#include <core/SSingleton.h>

namespace SOUI
{
	template<class T>
	class TAutoEventMapReg
	{
		typedef TAutoEventMapReg<T> _thisClass;
	public:
		TAutoEventMapReg()
		{
			SNotifyCenter::getSingleton().RegisterEventMap(Subscriber(&_thisClass::OnEvent,this));
		}

		~TAutoEventMapReg()
		{
			SNotifyCenter::getSingleton().UnregisterEventMap(Subscriber(&_thisClass::OnEvent,this));
		}

	protected:
		bool OnEvent(EventArgs *e){
			T * pThis = static_cast<T*>(this);
			return !!pThis->_HandleEvent(e);
		}
	};

	class SOUI_EXP SNotifyCenter : public SSingleton<SNotifyCenter>
						, public SEventSet
	{
	public:
		SNotifyCenter(void);
		~SNotifyCenter(void);

		void FireEventSync(EventArgs *e);
		void FireEventAsync(EventArgs *e);

		bool RegisterEventMap(const ISlotFunctor &slot);
		bool UnregisterEventMap(const ISlotFunctor & slot);
	protected:
		void OnFireEvent(EventArgs *e);

		void ExecutePendingEvents();

		static VOID CALLBACK OnTimer( HWND hwnd,
			UINT uMsg,
			UINT_PTR idEvent,
			DWORD dwTime
			);

		SCriticalSection	m_cs;			//线程同步对象
		SList<EventArgs*>	m_evtPending;//挂起的等待执行的事件
		DWORD				m_dwMainTrdID;//主线程ID
		
		UINT_PTR			m_timerID;	//定时器ID，用来执行异步事件

		SList<ISlotFunctor*>	m_evtHandlerMap;
	};
}
