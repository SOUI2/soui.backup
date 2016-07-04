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

        /**
        * FireEventSync
        * @brief    触发一个同步通知事件
        * @param    EventArgs *e -- 事件对象
        * @return    
        *
        * Describe  只能在UI线程中调用
        */
		void FireEventSync(EventArgs *e);

        /**
        * FireEventAsync
        * @brief    触发一个异步通知事件
        * @param    EventArgs *e -- 事件对象
        * @return    
        *
        * Describe  可以在非UI线程中调用，EventArgs *e必须是从堆上分配的内存，调用后使用Release释放引用计数
        */
		void FireEventAsync(EventArgs *e);


        /**
        * RegisterEventMap
        * @brief    注册一个处理通知的对象
        * @param    const ISlotFunctor &slot -- 事件处理对象
        * @return    
        *
        * Describe 
        */
		bool RegisterEventMap(const ISlotFunctor &slot);

        /**
        * RegisterEventMap
        * @brief    注销一个处理通知的对象
        * @param    const ISlotFunctor &slot -- 事件处理对象
        * @return    
        *
        * Describe 
        */
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
		SList<EventArgs*>	*m_evtPending;//挂起的等待执行的事件
		DWORD				m_dwMainTrdID;//主线程ID
		
		UINT_PTR			m_timerID;	//定时器ID，用来执行异步事件

		SList<ISlotFunctor*>	m_evtHandlerMap;
	};
}
