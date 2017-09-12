#pragma once
#include <shellapi.h>
#include "export_table_adapter.h"
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
class CTreeViewDropTarget : public CTestDropTarget
{
protected:
	STreeView *m_pTreeView1;
	STreeView *m_pTreeView2;
public:
	CTreeViewDropTarget(STreeView *pTreeView1, STreeView *pTreeView2) :m_pTreeView1(pTreeView1),m_pTreeView2(pTreeView2)
	{
		if (m_pTreeView1) m_pTreeView1->AddRef();
		if (m_pTreeView2) m_pTreeView2->AddRef();
	}
	~CTreeViewDropTarget()
	{
		if (m_pTreeView1) m_pTreeView1->Release();
		if (m_pTreeView2) m_pTreeView2->Release();
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

		if (success)
		{
			HANDLE hFile = CreateFile(
				filename, //PEÎÄ¼þÃû
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL); 
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return S_FALSE;
			}
//			DWORD dwFileSize;
//			GetFileSize(hFile, &dwFileSize);
// 			DWORD dwMinFileSize = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS32);
// 			if (dwMinFileSize > dwFileSize)
// 			{
// 				CloseHandle(hFile);
// 				return S_FALSE;
// 			}

			HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

			if (hFileMapping == NULL || hFileMapping == INVALID_HANDLE_VALUE)
			{
				return S_FALSE;
			}

			LPBYTE lpBaseAddress = (LPBYTE)MapViewOfFile(hFileMapping,   // handle to map object
				FILE_MAP_READ, 0, 0, 0);


			if (m_pTreeView1)
			{
				CImportTableTreeViewAdapter *treead = (CImportTableTreeViewAdapter*)m_pTreeView1->GetAdapter();
				treead->Updata(lpBaseAddress);
			}
			if (m_pTreeView2)
			{
				CExportTableTreeViewAdapter *treead = (CExportTableTreeViewAdapter*)m_pTreeView2->GetAdapter();
				treead->Updata(lpBaseAddress);
			}
			UnmapViewOfFile(lpBaseAddress);
			CloseHandle(hFileMapping);
			CloseHandle(hFile);
		}

		*pdwEffect = DROPEFFECT_LINK;
		return S_OK;
	}
};
