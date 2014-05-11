#include "duistd.h"
#include "DuiFrameDropTarget.h"
#include "duiframe.h"

namespace SOUI{
	CDuiFrameDropTarget::CDuiFrameDropTarget(CDuiWindow * pFrame)
		:m_pDuiFrame(pFrame)
		,m_pDataObj(NULL)
		,m_hDuiHover(0)
	{
	}

	CDuiFrameDropTarget::~CDuiFrameDropTarget(void)
	{
		DragLeave();
		POSITION pos=m_mapDropTarget.GetStartPosition();
		while(pos)
		{
			DTMAP::CPair *pPair=m_mapDropTarget.GetNext(pos);
			pPair->m_value->Release();
		}
	}

	BOOL CDuiFrameDropTarget::RegisterDragDrop( HDUIWND hDuiWnd,IDropTarget *pDropTarget )
	{
		if(m_mapDropTarget.Lookup(hDuiWnd)) return FALSE;
		m_mapDropTarget[hDuiWnd]=pDropTarget;
		pDropTarget->AddRef();
		return TRUE;
	}

	BOOL CDuiFrameDropTarget::RevokeDragDrop( HDUIWND hDuiWnd )
	{
		DTMAP::CPair *pPair=m_mapDropTarget.Lookup(hDuiWnd);
		if(!pPair) return FALSE;
		pPair->m_value->Release();
		m_mapDropTarget.RemoveKey(hDuiWnd);
		return TRUE;
	}


	HRESULT STDMETHODCALLTYPE CDuiFrameDropTarget::QueryInterface( /* [in] */ REFIID riid, /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject )
	{
		HRESULT hr=S_FALSE;
		if(riid==__uuidof(IUnknown))
			*ppvObject=(IUnknown*) this,hr=S_OK;
		else if(riid==__uuidof(IDropTarget))
			*ppvObject=(IDropTarget*)this,hr=S_OK;
		return hr;
	}

	HRESULT STDMETHODCALLTYPE CDuiFrameDropTarget::DragEnter( /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj, /* [in] */ DWORD grfKeyState, /* [in] */ POINTL pt, /* [out][in] */ __RPC__inout DWORD *pdwEffect )
	{
		m_pDataObj=pDataObj;
		m_pDataObj->AddRef();
		return DragOver(grfKeyState,pt,pdwEffect);
	}

	HRESULT STDMETHODCALLTYPE CDuiFrameDropTarget::DragOver( /* [in] */ DWORD grfKeyState, /* [in] */ POINTL pt, /* [out][in] */ __RPC__inout DWORD *pdwEffect )
	{
		HDUIWND hDuiHover=m_pDuiFrame->DuiGetHWNDFromPoint(PointL2FrameClient(pt),FALSE);
		DUIASSERT(hDuiHover);
		*pdwEffect=DROPEFFECT_NONE;
		if(hDuiHover != m_hDuiHover)
		{
			DTMAP::CPair *pPair=m_mapDropTarget.Lookup(m_hDuiHover);
			if(m_hDuiHover && pPair)
				pPair->m_value->DragLeave();
			m_hDuiHover=hDuiHover;
			pPair=m_mapDropTarget.Lookup(m_hDuiHover);
			if(pPair && m_hDuiHover)
				pPair->m_value->DragEnter(m_pDataObj,grfKeyState,pt,pdwEffect);
		}else
		{
			DTMAP::CPair *pPair=m_mapDropTarget.Lookup(m_hDuiHover);
			if(m_hDuiHover && pPair)
				pPair->m_value->DragOver(grfKeyState,pt,pdwEffect);
		}

		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE CDuiFrameDropTarget::DragLeave( void )
	{
		if(m_pDataObj)
		{
			m_pDataObj->Release();
			m_pDataObj=NULL;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE CDuiFrameDropTarget::Drop( /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj, /* [in] */ DWORD grfKeyState, /* [in] */ POINTL pt, /* [out][in] */ __RPC__inout DWORD *pdwEffect )
	{
		DTMAP::CPair *pPair=m_mapDropTarget.Lookup(m_hDuiHover);
		if(m_hDuiHover && pPair)
			pPair->m_value->Drop(pDataObj,grfKeyState,pt,pdwEffect);
		m_hDuiHover=NULL;
		m_pDataObj->Release();
		m_pDataObj=NULL;
		return S_OK;
	}

	POINT CDuiFrameDropTarget::PointL2FrameClient( const POINTL & pt )
	{
		CPoint pt2(pt.x,pt.y);
		ScreenToClient(m_pDuiFrame->GetContainer()->GetHostHwnd(),&pt2);
		return pt2;
	}

}

