#pragma once

#include <atl.mini/SComHelper.h>

class CDropTarget:public SUnknownImpl<IDropTarget>
{
public:
    CDropTarget()
    {
    }

    virtual ~CDropTarget(){}

    COM_INTERFACE_BEGIN()
        COM_INTERFACE(IDropTarget)
        COM_INTERFACE_END()

        //////////////////////////////////////////////////////////////////////////
        // IDropTarget

        virtual HRESULT STDMETHODCALLTYPE DragEnter( 
        /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE DragOver( 
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE DragLeave( void)
    {
        return S_OK;
    }
};
