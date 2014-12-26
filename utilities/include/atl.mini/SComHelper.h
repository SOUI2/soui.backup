#pragma once

#include <Unknwn.h>

namespace SOUI
{
    #define COM_INTERFACE_BEGIN() \
    public:\
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( \
        /* [in] */ REFIID riid,\
        /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)\
        {\
            HRESULT hr = E_NOINTERFACE; \
            
    #define COM_INTERFACE(IFACE)    \
            if(hr != S_OK && riid == __uuidof(IFACE)) \
            {\
                *ppvObject = static_cast<IFACE*>(this); \
                AddRef();\
                hr = S_OK;\
            }\
            
    #define COM_INTERFACE_END()  \
            if(hr != S_OK) hr = __super::QueryInterface(riid,ppvObject);\
            return hr; \
         }
    
     
    template<class T>
    class SUnknownImpl: public T
    {
    public:
        SUnknownImpl():m_cRef(0){}
        virtual ~SUnknownImpl(){}
        
        
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
            {
                if(riid == __uuidof(IUnknown))
                {
                    *ppvObject = this;
                    AddRef();
                    return  S_OK;
                }else
                {
                    return E_NOINTERFACE;
                }
            }

        virtual ULONG STDMETHODCALLTYPE AddRef( void)
        {
            return (ULONG)::InterlockedIncrement(&m_cRef);
        }

        virtual ULONG STDMETHODCALLTYPE Release( void)
        {
            LONG uRet = ::InterlockedDecrement(&m_cRef);
            if(uRet == 0)
            {
                OnFinialRelease();
            }
            return uRet;
        }

    public:
    
        virtual void OnFinialRelease(){delete this;}

    protected:
        LONG m_cRef;
    };
}
