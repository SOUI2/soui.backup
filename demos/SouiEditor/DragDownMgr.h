#pragma once

extern CMainDlg* g_pMainDlg;

class CTestDropTarget :public IDropTarget
{
public:
	CTestDropTarget()
	{
		nRef = 0;
	}

	virtual ~CTestDropTarget() {}

	//////////////////////////////////////////////////////////////////////////
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		HRESULT hr = S_FALSE;
		if (riid == __uuidof(IUnknown))
			*ppvObject = (IUnknown*) this, hr = S_OK;
		else if (riid == __uuidof(IDropTarget))
			*ppvObject = (IDropTarget*)this, hr = S_OK;
		if (SUCCEEDED(hr)) AddRef();
		return hr;

	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) { return ++nRef; }

	virtual ULONG STDMETHODCALLTYPE Release(void) {
		ULONG uRet = --nRef;
		if (uRet == 0) delete this;
		return uRet;
	}

	//////////////////////////////////////////////////////////////////////////
	// IDropTarget

	virtual HRESULT STDMETHODCALLTYPE DragEnter(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragOver(
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		return S_OK;
	}


protected:
	int nRef;
};

class CFileDropTarget : public CTestDropTarget
{
protected:
	SWindow *m_pSWnd;
public:
	CFileDropTarget(SWindow *pEdit) :m_pSWnd(pEdit)
	{
		if (m_pSWnd) m_pSWnd->AddRef();
	}
	~CFileDropTarget()
	{
		if (m_pSWnd) m_pSWnd->Release();
	}
public:
	virtual HRESULT STDMETHODCALLTYPE Drop(
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect)
	{
		FORMATETC format =
		{
			CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
		};
		STGMEDIUM medium;
		if (FAILED(pDataObj->GetData(&format, &medium)))
		{
			return S_FALSE;
		}

		HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

		if (!hdrop)
		{
			return S_FALSE;
		}

		bool success = false;
		TCHAR filename[MAX_PATH];
		success = !!DragQueryFile(hdrop, 0, filename, MAX_PATH);
		DragFinish(hdrop);
		GlobalUnlock(medium.hGlobal);

		if (success && m_pSWnd)
		{
			SStringT fileN = filename;
			if (fileN.Right(4).CompareNoCase(_T(".idx")) == 0)
				g_pMainDlg->OutOpenProject(filename);
		}

		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
};