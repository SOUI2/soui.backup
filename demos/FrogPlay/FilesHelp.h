#pragma once
#include "stdafx.h"
#include <vector>
using namespace std;

class CFileHelp
{
public:
	CFileHelp(void);
	~CFileHelp(void);
public:
	
	
	//打开对话框 lpstrFilter：过滤字符串   hwndOwner：父窗口  fileNames：完整文件路径
	static BOOL OpenFile(LPCWSTR lpstrFilter, HWND hwndOwner, vector<SStringT> &fileNames, bool IsMulti = true);
	//把一个绝对路径拆分成，文件名、扩展名
	static void SplitPathFileName(SStringT fileName, SStringT &szName, SStringT &szExt);
	// 浏览文件夹 path：路径  hwndOwner : 父窗口  tile : 窗口标题
	static BOOL BrowseDir(SStringT &path, HWND hwndOwner, SStringT title);
	// 检查文件后缀名 pstrPath：文件路径 pstrExtFilter：过滤列表
	static bool FindFileExt(LPCTSTR pstrPath, LPCTSTR pstrExtFilter);
	//递归遍历当前目录内文件文件
	static void EnumerateFiles(vector<SStringT> &vctString, LPCTSTR p_strExtFilter);
	//得到文件的大小
	static DWORD GetFileSize(LPCTSTR fileName);

	//文件大小转换为字符换： xx.xxM
	static LPCTSTR FileSizeToString(DWORD dwSize);

	//文件总时间04:00
	static SStringT TimeToToleString(int time);
	//检查文件是否存在
	static BOOL CheckFileExist(SStringT pathFileName);
};

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


