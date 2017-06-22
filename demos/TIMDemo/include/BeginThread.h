#ifndef __BEGINTHREAD_H_INCLUDE_20160727__
#define __BEGINTHREAD_H_INCLUDE_20160727__
#pragma once
#include <winnt.h>
#include <process.h>
#include <functional>			//C++11  不支持 vs2008 
/*//////////////////////////////////////////////////////////////////////////
time 2016 07 27 
线程 封装类  可直接使用类的成员函数 做 线程 函数。
 可以使用 模版的方式调用  
	BeginThread thread;
	thread.Start(&Class::Fun, this);

也可以用 C++11的function 调用
	BeginThread thread;
	thread.Start(bind(&Class::Fun, this));


后续 的 功能  慢慢加吧  先够用
//////////////////////////////////////////////////////////////////////////*/
class BeginThread
{
public:
	BeginThread(bool bAutoRelease=true)
		: m_hThread(NULL)
		, m_dwThreadId(0)
		, m_threadFun(nullptr)
		, m_bAutoRelease(bAutoRelease)
	{
	}
	~BeginThread()
	{
		Release();
	}
public:
	template<class Fun, class T>
	bool Start(Fun fun, T t)
	{
		if(IsWorking())
		{
			m_sErrorText = L"线程已开启！";
			return false;
		}

		m_threadFun = std::bind(fun, t);
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, this, NULL, &m_dwThreadId);
		return NULL != m_hThread;
	}

	bool Start(std::function<UINT()> threadFun)
	{
		if(IsWorking())
		{
			m_sErrorText = L"线程已开启！";
			return false;
		}

		m_threadFun = threadFun;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, this, NULL, &m_dwThreadId);
		return NULL != m_hThread;
	}
	
	/*bool IsValid() const
	{
	return NULL != m_hThread;
	}*/

	bool IsWorking()
	{
		if(NULL == m_hThread)
			return false;

		DWORD dwWait = ::WaitForSingleObject(m_hThread, 0);
		if(WAIT_TIMEOUT == dwWait)
			return true;

		return false;
	}

	UINT GetThreadId()
	{
		return m_dwThreadId;
	}

	//关闭线程   
	bool Terminate(DWORD dwWaitTime=200)
	{
		if (NULL != m_hThread)
		{
			DWORD dwEixtCode = 0;
			if (GetExitCodeThread(m_hThread, &dwEixtCode))
			{
				if(WAIT_OBJECT_0 != ::WaitForSingleObject(m_hThread, dwWaitTime))
				{
					::TerminateThread(m_hThread, 0);
				}
			}

			Release();
		}

		return true;
	}

	void Release()
	{
		if(NULL != m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}

	std::wstring GetErrorText()
	{
		std::wstring s = m_sErrorText;
		m_sErrorText.clear();
		return s;
	}
private:
	//静态 线程 函数 
	static unsigned int __stdcall ThreadFun(void* lp)
	{
		BeginThread* pThread = (BeginThread*)lp;
		if(NULL == pThread || nullptr == pThread->m_threadFun)
			return 0;
		
		unsigned int nRet = pThread->m_threadFun();
		if(pThread->m_bAutoRelease)
			pThread->Release();

		return nRet;
	}

	
private:
	HANDLE				m_hThread;
	UINT					m_dwThreadId;
	bool					m_bAutoRelease;					//自动释放
	std::function<UINT()> m_threadFun;

	std::wstring		m_sErrorText;
};



class BeginThreadEx
{
public:
	BeginThreadEx()
		: m_hThread(NULL)
		, m_dwThreadId(0)
		//, m_threadFun(nullptr)
		, m_funInfo(NULL)
	{
	}
	~BeginThreadEx()
	{
		Release();
		if (NULL  != m_funInfo)
		{
			delete m_funInfo;
			m_funInfo = NULL;
		}
	}
public:
	bool Start(std::function<void(LPARAM)> threadFun, LPARAM lParam)
	{
		if(IsWorking())
		{
			m_sErrorText = L"线程已开启！";
			return false;
		}

		m_funInfo = new FunInfo;
		m_funInfo->threadFun = threadFun;
		m_funInfo->lParam = lParam;

		//m_threadFun = threadFun;
		//m_lParam = lParam;
		m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, this, NULL, &m_dwThreadId);
		return NULL != m_hThread;
	}

	bool IsWorking()
	{
		if(NULL == m_hThread)
			return false;

		DWORD dwWait = ::WaitForSingleObject(m_hThread, 0);
		if(WAIT_TIMEOUT == dwWait)
			return true;

		return false;
	}

	UINT GetThreadId()
	{
		return m_dwThreadId;
	}

	//关闭线程   
	bool Terminate(DWORD dwWaitTime=200)
	{
		if (NULL != m_hThread)
		{
			DWORD dwEixtCode = 0;
			if (GetExitCodeThread(m_hThread, &dwEixtCode))
			{
				if(WAIT_OBJECT_0 != ::WaitForSingleObject(m_hThread, dwWaitTime))
				{
					::TerminateThread(m_hThread, 0);
				}
			}

			Release();
		}

		return true;
	}

	void Release()
	{
		if(NULL != m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}

	std::wstring GetErrorText()
	{
		std::wstring s = m_sErrorText;
		m_sErrorText.clear();
		return s;
	}
	unsigned int Do()
	{
		m_funInfo->threadFun(m_funInfo->lParam);
		
		Release();

		return 0;
	}
public:
	//静态 线程 函数 
	static unsigned int __stdcall ThreadFun(void* lp)
	{
		BeginThreadEx* pThread = (BeginThreadEx*)lp;
		if(NULL == pThread)
			return 0;
		
		return pThread->Do();
	}

	struct FunInfo
	{
		std::function<void(LPARAM)>			threadFun;
		LPARAM lParam;
	};
private:
	HANDLE				m_hThread;
	UINT					m_dwThreadId;
	std::wstring		m_sErrorText;

	FunInfo*			m_funInfo;
};

//////////////////////////////////////////////////////////////////////////
#endif // __BEGINTHREAD_H_INCLUDE_20160727__




//  vs2008 写法  
#if 0

//采用了模板的写法   加上 类名 的模版  
//	例子：
//	CBeginThread<CMainDlg> beginThread;
//	beginThead.Start(this, &CMainDlg::Fun); 

template<class Type>
class CBeginThread
{
public:
	CBeginThread()
		: m_hThread(NULL)
		, m_pT(NULL)
		, m_pFun(NULL)
	{
	}

	virtual ~CBeginThread()
	{
		Release();
	}
	
	typedef unsigned int (Type::*pFun)();

	//开启启动线程    类的指针 加 函数指针
	bool Start(Type* pT, pFun p)
	{
		if(NULL != m_hThread)
			return false;

		if(NULL == pT || NULL == p)
			return false;

		m_pT = pT;
		m_pFun = p;

		m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFun, this, NULL, 0);
		if(NULL == m_hThread)
			return false;

		return true;
	}

	//关闭线程   
	bool Terminate(DWORD dwWaitTime=200)
	{
		if (NULL != m_hThread)
		{
			DWORD dwEixtCode = 0;
			if (GetExitCodeThread(m_hThread, &dwEixtCode))
			{
				if(WAIT_OBJECT_0 != ::WaitForSingleObject(m_hThread, dwWaitTime))
				{
					::TerminateThread(m_hThread, 0);
				}
			}

			Release();
		}

		return true;
	}

	//判断线程 
	bool IsWorking()
	{
		if(NULL == m_hThread)
			return false;

		DWORD dwWait = ::WaitForSingleObject(m_hThread, 0);
		if(WAIT_OBJECT_0 == dwWait)
			return false;
		else if(WAIT_TIMEOUT == dwWait)
			return true;

		return false;
	}

	//关闭 句柄 
	void Release()
	{
		if(NULL != m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}


	//线程 函数
	unsigned int OnFun()
	{
		if(NULL != m_pT)
		{
			return (m_pT->*m_pFun)();
		}
		return 0;
	}
public:
	//静态 线程 函数 
	static unsigned int __stdcall ThreadFun(void* lp)
	{
		CBeginThread* pThread = (CBeginThread*)lp;
		if(NULL == pThread)
			return 0;

		unsigned int nRet = pThread->OnFun();
		pThread->Release();
		return nRet;
	}
private:
	HANDLE m_hThread;
	Type*				m_pT;
	pFun					m_pFun;
};
#endif