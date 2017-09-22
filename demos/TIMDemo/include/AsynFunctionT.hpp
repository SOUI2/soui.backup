/*
异步 任务 处理

*/
#include <process.h>
#include <functional>
#include <list>

template<class ParamType=LPVOID>				// char* string void*
class AsynTaskHandle
{
	typedef struct tagTask
	{
		std::function<void(ParamType)>		fun;
		ParamType										lpParam;
	}AsynTask, *PAsynTask;

protected:
	volatile bool							m_bQuit;
	HANDLE*								m_phThread;
	unsigned int							m_nThreadCount;
	//UINT										m_dwThreadId;
	//HANDLE									m_hListEvent;
	HANDLE									m_hSemaphore;			//信号量 句柄
	CRITICAL_SECTION					m_csLock;		//维持队列同步
	std::list<PAsynTask>				m_TaskList;	
public:
	AsynTaskHandle(unsigned int nThreadCount=1)
		: m_bQuit(false)
		, m_nThreadCount(nThreadCount)
	{
		InitializeCriticalSection(&m_csLock);
		m_hSemaphore = CreateSemaphore(NULL, 0, 100, NULL);
		//m_hListEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

		assert(m_nThreadCount);
		m_phThread = new HANDLE[m_nThreadCount];
		for (unsigned int i=0; i<m_nThreadCount; ++i)
		{
			m_phThread[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, this, NULL, NULL);
		}
	}
	virtual ~AsynTaskHandle()
	{		
		if(NULL != m_hSemaphore)
		{
			CloseHandle(m_hSemaphore);
			m_hSemaphore = NULL;
		}

		if(m_TaskList.size() > 0)		// 要退出了  且 任务没执行完 要清理缓存
		{
			for each (auto var in m_TaskList)
			{
				delete var;
			}
		}
		
		for (unsigned int i=0; i<m_nThreadCount; ++i)
		{
			CloseHandle(m_phThread[i]);
		}
		delete[] m_phThread;
		m_phThread = NULL;
		DeleteCriticalSection(&m_csLock);
	}

public:
	
	template<class Fun, class This>
	void AddTask(Fun fun, This t, ParamType lpParam)
	{
		auto task = new AsynTask;
		task->fun = std::bind(fun, t, std::placeholders::_1);
		task->lpParam = lpParam;

		EnterCriticalSection(&m_csLock);
		m_TaskList.push_back(task);		//交换数据 
		LeaveCriticalSection(&m_csLock);

		//SetEvent(m_hListEvent);
		ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}

	void AddTask(std::function<void(ParamType)> fun, ParamType lpParam)
	{
		auto task = new AsynTask;
		task->fun = fun;
		task->lpParam = lpParam;

		EnterCriticalSection(&m_csLock);
		m_TaskList.push_back(task);		//交换数据 
		LeaveCriticalSection(&m_csLock);

		//SetEvent(m_hListEvent);
		ReleaseSemaphore(m_hSemaphore, 1, NULL);
	}
	void Close(DWORD dwWait=1000)
	{
		m_bQuit = true;
		//SetEvent(m_hListEvent);
		ReleaseSemaphore(m_hSemaphore, m_nThreadCount, NULL);
		//	
		for (unsigned int i=0; i<m_nThreadCount; ++i)
		{
			WaitForSingleObject(m_hThread[i], dwWait);
		}
	}
protected:
	unsigned int Handle()
	{
		while (!m_bQuit)
		{
			::WaitForSingleObject(m_hSemaphore, INFINITE);
			if(m_bQuit)
				break;
			
			OutputDebugString(_T("AsyncFun"));

			while (!m_bQuit)
			{
				PAsynTask pTask = NULL;
				EnterCriticalSection(&m_csLock);
				if(m_TaskList.size() > 0)
				{
					pTask = m_TaskList.front();
					m_TaskList.pop_front();
				}
				
				LeaveCriticalSection(&m_csLock);
				

				if(NULL == pTask)
					break;

				pTask->fun(pTask->lpParam);
				delete pTask;
			}			
		}

		return 0;
	}
private:
	//静态 线程 函数 
	static unsigned int __stdcall ThreadFun(void* lp)
	{
		AsynTaskHandle* pThis = (AsynTaskHandle*)lp;
		if(NULL == pThis )
			return 0;

		return pThis->Handle();
	}

};