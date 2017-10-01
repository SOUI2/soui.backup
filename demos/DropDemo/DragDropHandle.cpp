#include "stdafx.h"
#include "DragDropHandle.h"
#include "ShlGuid.h"

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

DropTargetEx::DropTargetEx(IDropTargetIF* iIF)
	: m_hWnd(NULL)
	, m_pIDropTargetHandle(iIF)
{  
	
	// Create an instance of the shell DnD helper object.  
	HRESULT s = CoCreateInstance ( CLSID_DragDropHelper,
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_IDropTargetHelper, 
		(void**) &m_piDropHelper );
	if (!SUCCEEDED(s) )  
	{  
		m_piDropHelper = NULL;  
	}  
}  


DropTargetEx::~DropTargetEx(void)  
{  
	if (NULL != m_piDropHelper)  
	{  
		m_piDropHelper->Release();
		m_piDropHelper = NULL;
	}  
}  

bool DropTargetEx::DragDropRegister(HWND hWnd, DWORD dwAcceptKeyState)  
{
	if(!IsWindow(hWnd))return false;
	
	HRESULT s = ::RegisterDragDrop (hWnd, this);  //这里报错87  那是因为没有加::OleInitialize(NULL);
	if(SUCCEEDED(s))  
	{		
		m_hWnd = hWnd;
		m_dwAcceptKeyState = dwAcceptKeyState;
		return true;  
	}  

	return false;
}  


bool DropTargetEx::DragDropRevoke(HWND hWnd)  
{  
	if(!IsWindow(hWnd))return false;  

	HRESULT s = ::RevokeDragDrop(hWnd);  

	return SUCCEEDED(s);  
}  

HRESULT STDMETHODCALLTYPE DropTargetEx::QueryInterface(REFIID riid, __RPC__deref_out void **ppvObject)  
{  
	*ppvObject = NULL;

	if (IID_IUnknown == riid || IID_IDropTarget == riid)
		*ppvObject = static_cast<IDropTarget*>(this);

	if( *ppvObject != NULL )
		((LPUNKNOWN)*ppvObject)->AddRef();
	return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
}

ULONG STDMETHODCALLTYPE DropTargetEx::AddRef()  
{
	return 1;
}  

ULONG STDMETHODCALLTYPE DropTargetEx::Release()  
{
	return 1;
}  

//进入  
HRESULT STDMETHODCALLTYPE DropTargetEx::DragEnter(__RPC__in_opt IDataObject *pDataObj, 
												   DWORD grfKeyState,
												   POINTL pt, 
												   __RPC__inout DWORD *pdwEffect)  
{
	//  判断 模式  默认设置 鼠标左键 拖放  右键的话就没有
	if(m_dwAcceptKeyState != (grfKeyState&m_dwAcceptKeyState))
	{
		return S_FALSE;
	}

	HRESULT ret = S_OK;
	*pdwEffect = DROPEFFECT_NONE;
	m_pDataObj = pDataObj;
	
	POINT pt1;
	pt1.x = pt.x;
	pt1.y = pt.y;
		
	// 响应 窗口自己的处理。返回S_FALSE 表示 不支持拖放
	if(NULL != m_pIDropTargetHandle)
	{
		ret = m_pIDropTargetHandle->OnDragEnter(m_pDataObj, grfKeyState, pt1);
	}

	if(NULL != m_piDropHelper)  
	{  
		ret = m_piDropHelper->DragEnter(m_hWnd, pDataObj, &pt1, *pdwEffect );  
	}

	return ret;
}  

//移动  
HRESULT STDMETHODCALLTYPE DropTargetEx::DragOver(DWORD dwKeyState, 
												  POINTL pt, 
												  __RPC__inout DWORD *pdwEffect)  
{
	POINT pt1;
	pt1.x = pt.x;
	pt1.y = pt.y;
	
	std::wstring szMessage;
	std::wstring szInsert;
	if(NULL != m_pIDropTargetHandle)
		*pdwEffect = m_pIDropTargetHandle->OnDragOver(m_pDataObj, dwKeyState, pt1, szMessage, szInsert);
	
	if(DROPEFFECT_NONE == *pdwEffect )
	{

	}
	
	DROPIMAGETYPE dwImage = (DROPIMAGETYPE)*pdwEffect;
	// szMessage 可以带格式的 %1 表示 szInsert的值
	if(!szMessage.empty() && !szInsert.empty())
	{
		szMessage.append(L" %1");
		
	}
	else
	{
		dwImage = DROPIMAGE_INVALID;
	}

	SetDropDescription(dwImage, szMessage.c_str(), szInsert.c_str());
	
	if(NULL != m_piDropHelper)  
	{  
		m_piDropHelper->DragOver(&pt1, *pdwEffect);  
	}
	return S_OK;
}  

//离开  
HRESULT STDMETHODCALLTYPE DropTargetEx::DragLeave()  
{  
	SetDropDescription(DROPIMAGE_INVALID, NULL);
			
	if(NULL != m_pIDropTargetHandle)
		m_pIDropTargetHandle->OnDragLeave();

	if (NULL != m_piDropHelper)  
	{  
		m_piDropHelper->DragLeave();
	}  

	return S_OK;
}  

//释放  
HRESULT STDMETHODCALLTYPE DropTargetEx::Drop(__RPC__in_opt IDataObject *pDataObj,
											  DWORD grfKeyState, 
											  POINTL pt, 
											  __RPC__inout DWORD *pdwEffect)  
{
	HRESULT ret = S_OK;
	POINT pt1;
	pt1.x = pt.x;
	pt1.y = pt.y;

	if(NULL != m_piDropHelper)
		ret = m_piDropHelper->Drop (pDataObj, (LPPOINT)&pt, *pdwEffect );
	
	if(NULL != m_pIDropTargetHandle)
		m_pIDropTargetHandle->OnDrop(m_pDataObj, grfKeyState, pt1);

	
	return ret;
}

bool DropTargetEx::SetDropDescription(DROPIMAGETYPE nImageType, LPCWSTR lpszMessage, LPCWSTR lpszInsert)
{
	STGMEDIUM stgMedium;
	FORMATETC formatEtc;
	if (DragDropHelper::GetGlobalData(m_pDataObj, CFSTR_DROPDESCRIPTION, formatEtc, stgMedium))
	{		
		bool bUpdate = false;
		DROPDESCRIPTION* pDropDescription = static_cast<DROPDESCRIPTION*>(::GlobalLock(stgMedium.hGlobal));

		// Clear description
		if (DROPIMAGE_INVALID == nImageType)
		{
			bUpdate = DragDropHelper::ClearDescription(pDropDescription);
		}	
		else // There is no need to update the description text when the source has not enabled display of text.
		{
			// If no text has been passed, use the stored text for the image type if present.
			if (NULL != lpszMessage && 0 != wcscmp(lpszMessage, pDropDescription->szMessage))
			{
				wcsncpy_s(pDropDescription->szMessage, MAX_PATH, lpszMessage, _TRUNCATE);
				bUpdate = true;
			}

			if (NULL != lpszInsert && 0 != wcscmp(lpszInsert, pDropDescription->szInsert))
			{
				wcsncpy_s(pDropDescription->szInsert, MAX_PATH, lpszInsert, _TRUNCATE);
				bUpdate = true;
			}
		}

		if (pDropDescription->type != nImageType)
		{
			bUpdate = true;
			pDropDescription->type = nImageType;
		}

		::GlobalUnlock(stgMedium.hGlobal);
		if (bUpdate)
		{
			bUpdate = SUCCEEDED(m_pDataObj->SetData(&formatEtc, &stgMedium, TRUE));
		}

		if (!bUpdate)							// Nothing changed or setting data failed
			::ReleaseStgMedium(&stgMedium);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
EnumFormatEtcEx::EnumFormatEtcEx(const std::vector<FORMATETC>& arrFE)
	: m_ulRefCount(0)
	, m_nCurrent(0)
{
	int nCount = arrFE.size();
	for (int i=0; i<nCount; ++i)
	{
		m_vctFmtEtc.push_back(arrFE.at(i));
	}
}

EnumFormatEtcEx::EnumFormatEtcEx(const std::vector<FORMATETC*>& arrFE)
	: m_ulRefCount(0)
	, m_nCurrent(0)
{
	int nCount = arrFE.size();
	for (int i=0; i<nCount; ++i)
	{
		m_vctFmtEtc.push_back(*arrFE.at(i));
	}
}

STDMETHODIMP EnumFormatEtcEx::QueryInterface(REFIID refiid, void FAR* FAR* ppv)
{
	*ppv = NULL;
	if (IID_IUnknown == refiid || IID_IEnumFORMATETC == refiid)
		*ppv=this;

	if (NULL != *ppv)
	{
		((LPUNKNOWN)*ppv)->AddRef();
		return S_OK;
	}
	return S_OK;
}

STDMETHODIMP_(ULONG) EnumFormatEtcEx::AddRef(void)
{
	return InterlockedIncrement(&m_ulRefCount);
}

STDMETHODIMP_(ULONG) EnumFormatEtcEx::Release(void)
{
	ULONG ul = InterlockedDecrement(&m_ulRefCount);
	if(0 == ul)
		delete this;

	return ul;
}

STDMETHODIMP EnumFormatEtcEx::Next(ULONG ulCelt, LPFORMATETC lpFormatEtc, ULONG FAR* pCeltFetched)
{
	if(NULL != pCeltFetched)
		*pCeltFetched=0;

	ULONG cReturn = ulCelt;

	if(ulCelt <= 0 || lpFormatEtc == NULL || m_nCurrent >= m_vctFmtEtc.size())
		return S_FALSE;

	if(NULL == pCeltFetched && 1 != ulCelt) // pceltFetched can be NULL only for 1 item request
		return S_FALSE;

	while (m_nCurrent < m_vctFmtEtc.size() && cReturn > 0)
	{
		*lpFormatEtc++ = m_vctFmtEtc.at(m_nCurrent++);
		--cReturn;
	}

	if (NULL != pCeltFetched)
		*pCeltFetched = ulCelt - cReturn;

	return (cReturn == 0) ? S_OK : S_FALSE;
}

STDMETHODIMP EnumFormatEtcEx::Skip(ULONG ulCelt)
{
	if((m_nCurrent + int(ulCelt)) >= m_vctFmtEtc.size())
		return S_FALSE;

	m_nCurrent += ulCelt;
	return S_OK;
}

STDMETHODIMP EnumFormatEtcEx::Reset(void)
{
	m_nCurrent = 0;
	return S_OK;
}

STDMETHODIMP EnumFormatEtcEx::Clone(IEnumFORMATETC FAR* FAR* ppCloneEnumFormatEtc)
{
	if(ppCloneEnumFormatEtc == NULL)
		return E_POINTER;

	EnumFormatEtcEx* pNewEnum = new EnumFormatEtcEx(m_vctFmtEtc);
	if(NULL == pNewEnum)
		return E_OUTOFMEMORY;  	

	pNewEnum->AddRef();
	pNewEnum->m_nCurrent = m_nCurrent;
	*ppCloneEnumFormatEtc = pNewEnum;
	return S_OK;
}


///////////////////////////////////////////////////////////////////////////
DataObjectEx::DataObjectEx()
	: m_pDragSourceHelper(NULL)
	, m_pDragSourceHelper2(NULL)
{
	
//#if defined(IID_PPV_ARGS) // The IID_PPV_ARGS macro has been introduced with Visual Studio 2005
	//::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDragSourceHelper));
//#else
	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDragSourceHelper, reinterpret_cast<LPVOID*>(&m_pDragSourceHelper));
//#endif
	if (m_pDragSourceHelper)
	{
		// With Vista, IDragSourceHelper2 has been inherited from by IDragSourceHelper.
//#if defined(IID_PPV_ARGS) && defined(__IDragSourceHelper2_INTERFACE_DEFINED__)
		// Can't use the IID_PPV_ARGS macro when building for pre Vista because the GUID for IDragSourceHelper2 is not defined.
//		m_pDragSourceHelper->QueryInterface(IID_PPV_ARGS(&m_pDragSourceHelper2));
//#else
		m_pDragSourceHelper->QueryInterface(IID_IDragSourceHelper2, reinterpret_cast<LPVOID*>(&m_pDragSourceHelper2));
//#endif
		if (NULL != m_pDragSourceHelper2) 
		{
			// No need to have two instances of the source helper in memory.
			m_pDragSourceHelper->Release();
			m_pDragSourceHelper = static_cast<IDragSourceHelper*>(m_pDragSourceHelper2);
		}
	}
}

DataObjectEx::~DataObjectEx()
{
	if(NULL != m_pDragSourceHelper)
	{
		m_pDragSourceHelper->Release();
		m_pDragSourceHelper = NULL;
	}
	if(NULL != m_pDragSourceHelper2)
	{
		m_pDragSourceHelper2->Release();
		m_pDragSourceHelper2 = NULL;
	}
	int nCount = m_vctFormatEtc.size();
	for (int i=0; i<nCount; ++i)
	{
		delete m_vctFormatEtc.at(i);
	}

	nCount = m_vctStgMedium.size();
	for (int i=0; i<nCount; ++i)
	{
		ReleaseStgMedium(m_vctStgMedium.at(i));
		delete m_vctStgMedium.at(i);
	}
}

HRESULT STDMETHODCALLTYPE DataObjectEx::QueryInterface(REFIID riid, void ** ppvObject)
{
	*ppvObject = NULL;

	if (IID_IUnknown == riid || IID_IDataObject == riid)
		*ppvObject = static_cast<IDataObject*>(this);

	if( NULL != *ppvObject)
	{
		((LPUNKNOWN)*ppvObject)->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE DataObjectEx::AddRef(void)
{
	//InterlockedIncrement(&m_ulRefCount); 
	//return m_ulRefCount;
	return 1;
}

ULONG STDMETHODCALLTYPE DataObjectEx::Release(void)
{
	//ULONG ul = InterlockedDecrement(&m_ulRefCount);
	//if(0 == ul)
	//	delete this;

	//return ul;
	return 1;
}

void DataObjectEx::CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc)
{
	switch(pMedSrc->tymed)
	{
	case TYMED_HGLOBAL:
		pMedDest->hGlobal = (HGLOBAL)OleDuplicateData(pMedSrc->hGlobal, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_GDI:
		pMedDest->hBitmap = (HBITMAP)OleDuplicateData(pMedSrc->hBitmap, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_MFPICT:
		pMedDest->hMetaFilePict = (HMETAFILEPICT)OleDuplicateData(pMedSrc->hMetaFilePict, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_ENHMF:
		pMedDest->hEnhMetaFile = (HENHMETAFILE)OleDuplicateData(pMedSrc->hEnhMetaFile, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_FILE:
		pMedSrc->lpszFileName = (LPOLESTR)OleDuplicateData(pMedSrc->lpszFileName, pFmtSrc->cfFormat, NULL);
		break;
	case TYMED_ISTREAM:
		pMedDest->pstm = pMedSrc->pstm;
		pMedSrc->pstm->AddRef();
		break;
	case TYMED_ISTORAGE:
		pMedDest->pstg = pMedSrc->pstg;
		pMedSrc->pstg->AddRef();
		break;
	case TYMED_NULL:
	default:
		break;
	}
	pMedDest->tymed = pMedSrc->tymed;
	pMedDest->pUnkForRelease = NULL;
	if(pMedSrc->pUnkForRelease != NULL)
	{
		pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
		pMedSrc->pUnkForRelease->AddRef();
	}
}

HRESULT STDMETHODCALLTYPE DataObjectEx::GetData(FORMATETC* pFormatEtc, STGMEDIUM *pMedium)
{
	if(NULL == pFormatEtc || NULL == pMedium)
		return E_INVALIDARG;

	pMedium->hGlobal = NULL;

	int nIndex = FindFmtEtc(pFormatEtc);
	if(-1 != nIndex)
	{
		CopyMedium(pMedium, m_vctStgMedium.at(nIndex), m_vctFormatEtc.at(nIndex));
		return S_OK;
	}
	
	return DV_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE DataObjectEx::GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
	return DATA_E_FORMATETC;
	//return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DataObjectEx::QueryGetData(FORMATETC* pFormatEtc)
{
	if(NULL == pFormatEtc)
		return E_INVALIDARG;

	//support others if needed DVASPECT_THUMBNAIL  //DVASPECT_ICON   //DVASPECT_DOCPRINT
	if (!(DVASPECT_CONTENT & pFormatEtc->dwAspect))
		return (DV_E_DVASPECT);

	HRESULT hr = DV_E_TYMED;
	int nCount = m_vctFormatEtc.size();
	for(int i = 0; i < nCount; ++i)
	{
		if(pFormatEtc->tymed & m_vctFormatEtc.at(i)->tymed)
		{
			if(pFormatEtc->cfFormat == m_vctFormatEtc.at(i)->cfFormat)
				return S_OK;
			else
				//hr = DATA_E_FORMATETC;
				hr = DV_E_CLIPFORMAT;
		}
		else
			//hr = DATA_E_FORMATETC;
			hr = DV_E_TYMED;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE DataObjectEx::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
	if (NULL == pFormatEtcOut)
		return E_INVALIDARG;

	return DATA_S_SAMEFORMATETC;
}

HRESULT __stdcall DataObjectEx::SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease)
{
	if(NULL == pFormatEtc || NULL == pMedium)
		return E_INVALIDARG;

	
	FORMATETC* fEtc = new FORMATETC;
	STGMEDIUM* pStgMed = new STGMEDIUM;

	if(NULL == fEtc || NULL == pStgMed)
		return E_OUTOFMEMORY;

	ZeroMemory(fEtc, sizeof(FORMATETC));
	ZeroMemory(pStgMed, sizeof(STGMEDIUM));

	*fEtc = *pFormatEtc;
	int nIndex = FindFmtEtc(pFormatEtc);
	if(-1 != nIndex)
	{
		FORMATETC*& fCacheEtc = m_vctFormatEtc.at(nIndex);
		delete fCacheEtc;
		fCacheEtc = fEtc;
	}
	else
		m_vctFormatEtc.push_back(fEtc);

	if(fRelease)
		*pStgMed = *pMedium;
	else
	{
		CopyMedium(pStgMed, pMedium, pFormatEtc);
	}
	if(-1 != nIndex)
	{
		STGMEDIUM*& fCacheStg = m_vctStgMedium.at(nIndex);
		delete fCacheStg;
		fCacheStg = pStgMed;
	}
	else 
		m_vctStgMedium.push_back(pStgMed);

	return S_OK;
}

HRESULT __stdcall DataObjectEx::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
{
	if(NULL == ppEnumFormatEtc)
		return E_POINTER;

	*ppEnumFormatEtc = NULL;
	switch (dwDirection)
	{
	case DATADIR_GET:
		*ppEnumFormatEtc = new EnumFormatEtcEx(m_vctFormatEtc);
		if(NULL == *ppEnumFormatEtc)
			return E_OUTOFMEMORY;

		(*ppEnumFormatEtc)->AddRef(); 
		break;

	case DATADIR_SET:
	default:
		return E_NOTIMPL;
		break;
	}

	return S_OK;
}

HRESULT __stdcall DataObjectEx::DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink *, DWORD* pdwConnection)
{
	*pdwConnection = 0;
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT __stdcall DataObjectEx::DUnadvise(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
	//return E_NOTIMPL;
}

HRESULT __stdcall DataObjectEx::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
	*ppEnumAdvise = NULL;
	return OLE_E_ADVISENOTSUPPORTED;
}


int DataObjectEx::FindFmtEtc(const FORMATETC* pFormatEtc)
{
	int nCount = m_vctFormatEtc.size();
	for(int i=0; i < nCount; ++i)
	{
		LPFORMATETC& pEtc = m_vctFormatEtc.at(i);
		if(pFormatEtc->tymed & pEtc->tymed &&
			pFormatEtc->dwAspect == pEtc->dwAspect &&
			pFormatEtc->cfFormat == pEtc->cfFormat)
		{
			return i;
		}
	}
	return -1;
}


bool DataObjectEx::CacheSingleFileAsHdrop(LPCTSTR lpszFilePath)
{
	LRESULT lRe = S_OK;


	FORMATETC fmtEtc = {0};
	fmtEtc.cfFormat = CF_HDROP;
	fmtEtc.dwAspect = DVASPECT_CONTENT;
	fmtEtc.lindex = -1;
	fmtEtc.tymed = TYMED_HGLOBAL;

	STGMEDIUM medium = {0};
	medium.tymed = TYMED_HGLOBAL;

	/*
	CHAR* lpText = ("hello world!");
	medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, strlen(lpText)+1);
	CHAR* pMem = (CHAR*)GlobalLock(medium.hGlobal);
	strcpy(pMem, lpText);
	GlobalUnlock(medium.hGlobal);
	*/
	size_t nNameSize = (_tcslen(lpszFilePath) + 1) * sizeof(TCHAR);
	// DROPFILES followed by NULL terminated file name and an additional NULL char.
	medium.hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE, sizeof(DROPFILES) + sizeof(TCHAR) + nNameSize);
	if (NULL != medium.hGlobal)
	{
		LPVOID lpData = ::GlobalLock(medium.hGlobal);
		LPDROPFILES pDropFiles = static_cast<LPDROPFILES>(lpData);
		pDropFiles->pFiles = sizeof(DROPFILES);
#ifdef _UNICODE
		pDropFiles->fWide = 1;
#endif
		LPBYTE lpFiles = static_cast<LPBYTE>(lpData) + sizeof(DROPFILES);
		::CopyMemory(lpFiles, lpszFilePath, nNameSize);
		::GlobalUnlock(medium.hGlobal);

		lRe = SetData(&fmtEtc, &medium, TRUE);
	}

	return S_OK == lRe;
}

bool DataObjectEx::SetDragImageWindow(HWND hWnd, POINT* pPt)
{
	LRESULT lRe = S_OK;
	
	if(NULL != m_pDragSourceHelper)
	{
		POINT pt = {0,0};
		if(NULL == pPt)
			pPt = &pt;

		lRe = m_pDragSourceHelper->InitializeFromWindow(hWnd, pPt, this);

		return S_OK == lRe;
	}
	
	return false;
}

bool DataObjectEx::SetDragImage(HBITMAP hBitmap, POINT* pPoint/*=NULL*/, COLORREF clr/*=GetSysColor(COLOR_WINDOW)*/)
{
	HRESULT hRes = E_NOINTERFACE;
	if (hBitmap && NULL != m_pDragSourceHelper)
	{
		BITMAP bm;
		SHDRAGIMAGE di;
		VERIFY(::GetObject(hBitmap, sizeof(bm), &bm));
		di.sizeDragImage.cx = bm.bmWidth;
		di.sizeDragImage.cy = bm.bmHeight;
		// If a position is negative, it is set to the center.
		// If a position is outside the image, it is set to the right / bottom.
		if (NULL != pPoint)
		{
			di.ptOffset = *pPoint;
			if (di.ptOffset.x < 0)
				di.ptOffset.x = di.sizeDragImage.cx / 2;
			else if (di.ptOffset.x > di.sizeDragImage.cx)
				di.ptOffset.x = di.sizeDragImage.cx;
			if (di.ptOffset.y < 0)
				di.ptOffset.y = di.sizeDragImage.cy / 2;
			else if (di.ptOffset.y > di.sizeDragImage.cy)
				di.ptOffset.y = di.sizeDragImage.cy;
		}
		// Center the cursor if pPoint is NULL. 
		else
		{
			di.ptOffset.x = di.sizeDragImage.cx / 2;
			di.ptOffset.y = di.sizeDragImage.cy / 2;
		}
		di.hbmpDragImage = hBitmap;
		di.crColorKey = clr;
		hRes = m_pDragSourceHelper->InitializeFromBitmap(&di, this);
	}

	if (FAILED(hRes) && hBitmap)
		::DeleteObject(hBitmap);					// delete image to avoid a memory leak

	return SUCCEEDED(hRes);
}

DROPEFFECT DataObjectEx::DoDragDrop(DROPEFFECT dwEffect)
{
	//设置 可以显示  描述信息 
	if(NULL != m_pDragSourceHelper2)
		m_pDragSourceHelper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);

	bool bUseDescription = (NULL != m_pDragSourceHelper2);// && ::IsAppThemed();
	// When drop descriptions can be used and description text strings has been defined,
	//  create a DropDescription data object with image type DROPIMAGE_INVALID.
	if (bUseDescription)
	{	
		//LRESULT lRe = m_pDragSourceHelper2->SetFlags(DSH_ALLOWDROPDESCRIPTIONTEXT);

		FORMATETC fmtEtc = {0};
		fmtEtc.cfFormat = ::RegisterClipboardFormat(CFSTR_DROPDESCRIPTION); 
		fmtEtc.dwAspect = DVASPECT_CONTENT;
		fmtEtc.lindex = -1;
		fmtEtc.tymed = TYMED_HGLOBAL;

		STGMEDIUM medium = {0};
		medium.tymed = TYMED_HGLOBAL;
		
		//CLIPFORMAT cfDS = 
		if (fmtEtc.cfFormat)
		{
			medium.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DROPDESCRIPTION));
			if (medium.hGlobal)
			{
				DROPDESCRIPTION* pDropDescription = static_cast<DROPDESCRIPTION*>(::GlobalLock(medium.hGlobal));
				pDropDescription->type = DROPIMAGE_INVALID;
				::GlobalUnlock(medium.hGlobal);
				//CacheGlobalData(cfDS, hGlobal);
							
				SetData(&fmtEtc, &medium, TRUE);
			}
		}
	}


	DropSourceEx dropSource;
	dropSource.m_pIDataObj = this;
	DWORD dw;
	HRESULT hr = ::DoDragDrop(this, &dropSource, dwEffect, &dw);

	return dwEffect;
}







//////////////////////////////////////////////////////////////////////////
DropSourceEx::DropSourceEx()
	: m_pIDataObj(NULL)
	, m_bSetCursor(true)
{
}

HRESULT STDMETHODCALLTYPE DropSourceEx::QueryInterface(REFIID riid, void ** ppvObject)
{
	*ppvObject = NULL;

	if (riid == IID_IDropSource)
		*ppvObject = static_cast<IDropSource*>(this);

	if( *ppvObject != NULL )
		((LPUNKNOWN)*ppvObject)->AddRef();

	return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
}

ULONG STDMETHODCALLTYPE DropSourceEx::AddRef(void)
{
	return 1;
}

ULONG STDMETHODCALLTYPE DropSourceEx::Release(void)
{
	return 1;
}

HRESULT STDMETHODCALLTYPE DropSourceEx::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	if(fEscapePressed == TRUE)
		return DRAGDROP_S_CANCEL;       
	// if the LeftMouse button has been released, then do the drop!   这里 是 判断 结束 拖放的条件 如果没有左键 就 onDrop
	if((grfKeyState & MK_LBUTTON) == 0)
		return DRAGDROP_S_DROP;
	// continue with the drag-drop
	return S_OK;
}

bool DropSourceEx::SetDragImageCursor(DROPEFFECT dwEffect)
{
	HWND hWnd = (HWND)(DragDropHelper::GetGlobalDataDWord(m_pIDataObj, _T("DragWindow")));
	if (hWnd)
	{
		WPARAM wParam = 0;								// Use DropDescription to get type and optional text
		switch (dwEffect & ~DROPEFFECT_SCROLL)
		{
		case DROPEFFECT_NONE : wParam = 1; break;
		case DROPEFFECT_COPY : wParam = 3; break;		// The order is not as for DROPEEFECT values!
		case DROPEFFECT_MOVE : wParam = 2; break;
		case DROPEFFECT_LINK : wParam = 4; break;
		}
		::SendMessage(hWnd, WM_USER + 2, wParam, 0);
	}
	return NULL != hWnd;
}

HRESULT STDMETHODCALLTYPE DropSourceEx::GiveFeedback(DWORD dwEffect)
{
	SCODE sc = DRAGDROP_S_USEDEFAULTCURSORS;
	// Drag must be started, Windows version must be Vista or later
	//  and visual styles must be enabled.
	if (NULL != m_pIDataObj)
	{
		// IsShowingLayered is true when the target window shows the drag image
		// (target window calls the IDropTargetHelper handler functions).
		bool bOldStyle = (0 == DragDropHelper::GetGlobalDataDWord(m_pIDataObj, _T("IsShowingLayered")));
		SStringT ste;
		ste.Format(_T("GiveFeedback Effect %d"), dwEffect);
		OutputDebugString(ste);
		// Check if the drop description data object must be updated:
		// - When entering a window that does not show drag images while the previous
		//   target has shown the image, the drop description should be set to invalid.
		//   Condition: bOldStyle && !m_bSetCursor
		// - When setting drop description text here is enabled and the target
		//   shows the drag image, the description should be updated if not done by the target.
		//   Condition: m_pDropDescription && !bOldStyle
		if ((bOldStyle && !m_bSetCursor) || (!bOldStyle))
		{
			// Get DropDescription data if present.
			// If not present, cursor and optional text are shown like within the Windows Explorer.
			FORMATETC FormatEtc;
			STGMEDIUM StgMedium;
			if (DragDropHelper::GetGlobalData(m_pIDataObj, CFSTR_DROPDESCRIPTION, FormatEtc, StgMedium))
			{
				bool bChangeDescription = false;		// Set when DropDescription will be changed here
				DROPDESCRIPTION *pDropDescription = static_cast<DROPDESCRIPTION*>(::GlobalLock(StgMedium.hGlobal));
				SStringT stee;
				stee.Format(_T("GetGlobalData Descr %s %s %d"), pDropDescription->szInsert, pDropDescription->szMessage, pDropDescription->type);
				OutputDebugString(stee);
				if (bOldStyle)
				{
					bChangeDescription = DragDropHelper::ClearDescription(pDropDescription);
				}
				// The drop target is showing the drag image and new cursors and may have changed the description.
				// Drop targets that change the description should clear it when leaving the window
				// (set the image type to DROPIMAGE_INVALID from within OnDragLeave()).
				// The target may have set a special image type (LABEL, WARNING, NOIMAGE).
				// Then use it and do not change anything here.
				else if (pDropDescription->type <= DROPIMAGE_LINK)
				{
					DROPIMAGETYPE nImageType = DragDropHelper::DropEffectToDropImage(dwEffect);
					// When the target has changed the description, it usually has set the correct type.
					// If the image type does not match the drop effect, set the text here.
					if (DROPIMAGE_INVALID != nImageType &&
						pDropDescription->type != nImageType)
					{						
						// When text is available, set drop description image type and text.
						//if (m_pDropDescription->HasText(nImageType))
						//{
						//	bChangeDescription = true;
						//	pDropDescription->type = nImageType;
						//	m_pDropDescription->SetDescription(pDropDescription, nImageType);
						//}
						//// Set image type to invalid. Cursor is shown according to drop effect
						////  using system default text.
						//else
						//	bChangeDescription = DragDropHelper::ClearDescription(pDropDescription);
					}

				}	

				::GlobalUnlock(StgMedium.hGlobal);

				if (bChangeDescription)				// Update the DropDescription data object when 
				{									//  image type or description text has been changed here.
					if (FAILED(m_pIDataObj->SetData(&FormatEtc, &StgMedium, TRUE)))
						bChangeDescription = false;
				}
				if (!bChangeDescription)			// Description not changed or setting it failed
					::ReleaseStgMedium(&StgMedium);
			}
		}
		if (!bOldStyle)								// Show new style drag cursors.
		{
			if (m_bSetCursor)
			{
				// Set the default cursor.
				// Otherwise, the old style drag cursor remains when leaving a window
				//  that does not support drag images.
				// NOTE: 
				//  Add '#define OEMRESOURCE' on top of stdafx.h.
				//  This is necessary to include the OCR_ definitions from winuser.h.
				HCURSOR hCursor = (HCURSOR)LoadImage(
					NULL,							// hInst must be NULL for OEM images
					MAKEINTRESOURCE(32512),	// default cursor
					IMAGE_CURSOR,					// image type is cursor
					0, 0,							// use system metrics values with LR_DEFAULTSIZE
					LR_DEFAULTSIZE | LR_SHARED);
				::SetCursor(hCursor);
			}
			// Update of the cursor may be suppressed if "IsShowingText" is false.
			// But this results in invisible cursors for very short moments.
			//			if (0 != CDragDropHelper::GetGlobalDataDWord(m_pIDataObj, _T("IsShowingText")))
			// If a drop description object exists and the image type is not invalid,
			//  it is used. Otherwise use the default cursor and text according to the drop effect. 
			SetDragImageCursor(dwEffect);
			sc = S_OK;								// Don't show default (old style) drag cursor
		}
		// When the old style drag cursor is actually used, the Windows cursor must be set
		//  to the default arrow cursor when showing new style drag cursors the next time.
		// If a new style drag cursor is actually shown, the cursor has been set above.
		// By using this flag, the cursor must be set only once when entering a window that
		//  supports new style drag cursors and not with each call when moving over it.
		m_bSetCursor = bOldStyle;
	}
	return sc;
}

//////////////////////////////////////////////////////////////////////////
bool DragDropHelper::GetGlobalData(LPDATAOBJECT pIDataObj, LPCTSTR lpszFormat, FORMATETC& formatEtc, STGMEDIUM& stgMedium)
{
	ASSERT(pIDataObj);
	ASSERT(lpszFormat && *lpszFormat);

	bool bRet = false;
	formatEtc.cfFormat = ::RegisterClipboardFormat(lpszFormat);
	formatEtc.ptd = NULL;
	formatEtc.dwAspect = DVASPECT_CONTENT;
	formatEtc.lindex = -1;
	formatEtc.tymed = TYMED_HGLOBAL;

	if (SUCCEEDED(pIDataObj->QueryGetData(&formatEtc)))
	{
		if (SUCCEEDED(pIDataObj->GetData(&formatEtc, &stgMedium)))
		{
			bRet = (TYMED_HGLOBAL == stgMedium.tymed);
			if (!bRet)
				::ReleaseStgMedium(&stgMedium);
		}
	}
	return bRet;
}

HGLOBAL DragDropHelper::GetGlobalData(LPDATAOBJECT pIDataObj, CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc/*=NULL*/)
{
	ASSERT(pIDataObj);
	
	FORMATETC formatEtc;
	
	formatEtc.cfFormat = cfFormat;
	formatEtc.ptd = NULL;
	formatEtc.dwAspect = DVASPECT_CONTENT;
	formatEtc.lindex = -1;

	formatEtc.tymed = TYMED_HGLOBAL | TYMED_MFPICT;

	if (SUCCEEDED(pIDataObj->QueryGetData(&formatEtc)))
	{
		STGMEDIUM stgMedium;
		if (SUCCEEDED(pIDataObj->GetData(&formatEtc, &stgMedium)))
		{

			if(TYMED_HGLOBAL != stgMedium.tymed)
				::ReleaseStgMedium(&stgMedium);

			return stgMedium.hGlobal;
		}
	}
	return NULL;
}

DWORD DragDropHelper::GetGlobalDataDWord(LPDATAOBJECT pIDataObj, LPCTSTR lpszFormat)
{
	DWORD dwData = 0;
	FORMATETC FormatEtc;
	STGMEDIUM StgMedium;
	if (GetGlobalData(pIDataObj, lpszFormat, FormatEtc, StgMedium))
	{
		ASSERT(::GlobalSize(StgMedium.hGlobal) >= sizeof(DWORD));
		dwData = *(static_cast<LPDWORD>(::GlobalLock(StgMedium.hGlobal)));
		::GlobalUnlock(StgMedium.hGlobal);
		::ReleaseStgMedium(&StgMedium);
	}
	return dwData;
}

bool DragDropHelper::ClearDescription(DROPDESCRIPTION* pDropDescription)
{
	bool bChanged = 
		pDropDescription->type != DROPIMAGE_INVALID ||
		pDropDescription->szMessage[0] != L'\0' || 
		pDropDescription->szInsert[0] != L'\0'; 

	pDropDescription->type = DROPIMAGE_INVALID;
	pDropDescription->szMessage[0] = pDropDescription->szInsert[0] = L'\0';

	return bChanged;
}

DROPIMAGETYPE DragDropHelper::DropEffectToDropImage(DROPEFFECT dwEffect)
{
	DROPIMAGETYPE nImageType = DROPIMAGE_INVALID;
	dwEffect &= ~DROPEFFECT_SCROLL;
	if (DROPEFFECT_NONE == dwEffect)
		nImageType = DROPIMAGE_NONE;
	else if (dwEffect & DROPEFFECT_MOVE)
		nImageType = DROPIMAGE_MOVE;
	else if (dwEffect & DROPEFFECT_COPY)
		nImageType = DROPIMAGE_COPY;
	else if (dwEffect & DROPEFFECT_LINK)
		nImageType = DROPIMAGE_LINK;

	return nImageType;
}

