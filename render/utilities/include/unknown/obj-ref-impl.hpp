//IObjRef的实现类
#pragma  once

namespace SOUI
{

template<class T>
class TObjRefImpl :  public T
{
public:
	TObjRefImpl():m_cRef(1)
	{
	}

	virtual ~TObjRefImpl(){
	}

	//!添加引用
	/*!
	*/
	virtual void __stdcall AddRef()
	{
		InterlockedIncrement(&m_cRef);
	}

	//!释放引用
	/*!
	*/
	virtual void __stdcall Release()
	{
		InterlockedDecrement(&m_cRef);
		if(m_cRef==0)
		{
			OnFinalRelease();
		}
	}

	//!释放对象
	/*!
	*/
    virtual void __stdcall OnFinalRelease()
    {
        delete this;
    }
protected:
	volatile LONG m_cRef;
};

template<class T,class T2>
class TObjRefImpl2 :  public TObjRefImpl<T>
{
public:
    virtual void __stdcall OnFinalRelease()
    {
        delete static_cast<T2*>(this);
    }
};

//CAutoRefPtr provides the basis for all other smart pointers
template <class T>
class CAutoRefPtr
{
public:
	CAutoRefPtr() throw()
	{
		p = NULL;
	}
	CAutoRefPtr(_In_ int nNull) throw()
	{
		(void)nNull;
		p = NULL;
	}
	CAutoRefPtr(_In_opt_ T* lp) throw()
	{
		p = lp;
		if (p != NULL)
		{
			p->AddRef();
		}
	}

	CAutoRefPtr(const CAutoRefPtr & src) throw()
	{
		p=src.p;
		if(p)
		{
			p->AddRef();
		}
	}

	~CAutoRefPtr() throw()
	{
		if (p)
		{
			p->Release();
		}
	}

	T* operator->() const throw()
	{
		return p;
	}

	operator T*() const throw()
	{
		return p;
	}
	T& operator*() const
	{
		return *p;
	}
	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&() throw()
	{
	    DUIASSERT(p==NULL);
		return &p;
	}
	bool operator!() const throw()
	{
		return (p == NULL);
	}
	bool operator<(_In_opt_ T* pT) const throw()
	{
		return p < pT;
	}
	bool operator!=(_In_opt_ T* pT) const
	{
		return !operator==(pT);
	}
	bool operator==(_In_opt_ T* pT) const throw()
	{
		return p == pT;
	}

	T* operator=(_In_opt_ T* lp) throw()
	{
		if(*this!=lp)
		{
			if(p)
			{
				p->Release();
			}
			p=lp;
			if(p)
			{
				p->AddRef();
			}
		}
		return *this;
	}

	T* operator=(_In_ const CAutoRefPtr<T>& lp) throw()
	{
		if(*this!=lp)
		{
			if(p)
			{
				p->Release();
			}
			p=lp;
			if(p)
			{
				p->AddRef();
			}
		}
		return *this;	
	}

	// Release the interface and set to NULL
	void Release() throw()
	{
		T* pTemp = p;
		if (pTemp)
		{
			p = NULL;
			pTemp->Release();
		}
	}

	// Attach to an existing interface (does not AddRef)
	void Attach(_In_opt_ T* p2) throw()
	{
		if (p)
		{
			p->Release();
		}
		p = p2;
	}
	// Detach the interface (does not Release)
	T* Detach() throw()
	{
		T* pt = p;
		p = NULL;
		return pt;
	}
	HRESULT CopyTo(_Deref_out_opt_ T** ppT) throw()
	{
		if (ppT == NULL)
			return E_POINTER;
		*ppT = p;
		if (p)
		{
			p->AddRef();
		}
		return S_OK;
	}

protected:
	T* p;
};


}//end of namespace SOUI
