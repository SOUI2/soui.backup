#pragma once
#include "wtl.mini\duicoll.h"
#include "duiwnd.h"

namespace SOUI{

    class CDuiFrameDropTarget : public IDropTarget
    {
    public:
        CDuiFrameDropTarget(SWindow * pFrame);
        ~CDuiFrameDropTarget(void);

        BOOL RegisterDragDrop(HSWND hDuiWnd,IDropTarget *pDropTarget);
        BOOL RevokeDragDrop(HSWND hDuiWnd);


        //////////////////////////////////////////////////////////////////////////
        // IUnknown
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */  void __RPC_FAR *__RPC_FAR *ppvObject);

        virtual ULONG STDMETHODCALLTYPE AddRef( void){return 1;}

        virtual ULONG STDMETHODCALLTYPE Release( void) {return 1;}

        //////////////////////////////////////////////////////////////////////////
        // IDropTarget

        virtual HRESULT STDMETHODCALLTYPE DragEnter( 
            /* [unique][in] */ IDataObject *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect);

        virtual HRESULT STDMETHODCALLTYPE DragOver( 
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect);

        virtual HRESULT STDMETHODCALLTYPE DragLeave( void);

        virtual HRESULT STDMETHODCALLTYPE Drop( 
            /* [unique][in] */  IDataObject *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */  DWORD *pdwEffect);

    protected:
        POINT PointL2FrameClient(const POINTL & pt);

        SWindow *m_pDuiFrame;

        typedef CDuiMap<HSWND,IDropTarget *> DTMAP;
        DTMAP m_mapDropTarget;

        IDataObject *m_pDataObj;
        HSWND         m_hDuiHover;
    };

}
