#include <shellapi.h>
#include <windows.h>
#define FILEILTER _T("*.*")
#include "atlstr.h"//使用CString或其他
#include "BassMusic.h"

typedef map<int,tagMusicInfo*>	CMusicManagerMap;

class CTestDropTarget:public IDropTarget
{
public:
	CTestDropTarget()
	{
		nRef=0;
	}

	virtual ~CTestDropTarget(){}

public:

	//////////////////////////////////////////////////////////////////////////
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		HRESULT hr=S_FALSE;
		if(riid==__uuidof(IUnknown))
			*ppvObject=(IUnknown*) this,hr=S_OK;
		else if(riid==__uuidof(IDropTarget))
			*ppvObject=(IDropTarget*)this,hr=S_OK;
		if(SUCCEEDED(hr)) AddRef();
		return hr;

	}

	virtual ULONG STDMETHODCALLTYPE AddRef( void){return ++nRef;}

	virtual ULONG STDMETHODCALLTYPE Release( void) { 
		ULONG uRet= -- nRef;
		if(uRet==0) delete this;
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


protected:
	int nRef;
};

class CTestDropTarget1 : public CTestDropTarget
{
protected:
	SWindow *m_pEdit;
public:
	CTestDropTarget1(SWindow *pEdit):m_pEdit(pEdit)
	{
		if(m_pEdit) m_pEdit->AddRef();
	}
	~CTestDropTarget1()
	{
		if(m_pEdit) m_pEdit->Release();
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
		if(FAILED(pDataObj->GetData(&format, &medium)))
		{
			return S_FALSE;
		}

		//HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));
		HDROP hdrop = static_cast<HDROP>(medium.hGlobal);

		if(!hdrop)
		{
			return S_FALSE;
		}

		//bool success = false;
		//TCHAR filename[MAX_PATH];
		//success=!!DragQueryFile(hdrop, 0, filename, MAX_PATH);
		//DragFinish(hdrop);


		UINT count;          
		TCHAR filePath[MAX_PATH];
		count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);          
		if(count)           
		{
			for(UINT i=0; i<count; i++)                    
			{
				int pathLen = DragQueryFile(hdrop, i, filePath, sizeof(filePath));                             
				CString str= filePath;
				str.Format(_T("%s"), filePath);
				int filetype;
				filetype=IsWantedFile(filePath);
				if (1==filetype) //视频格式
				{
					int i=0;
					//MessageBox(_T("你要播放的是视频文件！！！")); 
				}
				if (0==filetype) //音频格式
				{
					int i=0;
					//MessageBox(_T("你要播放的是音频文件！！！")); 

				}
				if (-1==filetype)//其他格式
				{
					int i=0;
					//MessageBox(_T("程序不支持的文件格式！！！请拖拽“*.mp3;*.wma;*.wav;*.mid;*.rmi;*.aac;*.ac3;*.aiff;*.m4a;*.mka;*.mp2;*.ogg或*.rm;*.rmvb;*.flv;*.f4v;*.avi;*.3gp;*.mp4;*.wmv;*.mpeg;*.mpga;*.asf;*.dat;*.mov”的音视频文件！")); 
				}
				TraverseFolder(filePath);
				FindInAll(filePath);
				//想判断是文件还是文件夹再保存
				//if ()//单个文件
				//{
				//}else//文件夹
				//{
				//}
				//m_PicCtr.push_back(filePath);
				//BrowerFolder(filePath,0);
				if(count && m_pEdit)
				{
					m_pEdit->SetWindowText(filePath);
				}

			}
		}
		DragFinish(hdrop); 
		//GlobalUnlock(medium.hGlobal);

		*pdwEffect=DROPEFFECT_LINK;
		return S_OK;
	}

	/***********************************************************************
	*  函 数 名：IsWantedFile                                          
	*  功能描述: 判断是否是想要的文件                                         
	*  输入参数：const CString &str       [IN] :单个音频文件路径      
	*  输出参数: 无
	*  返 回 值：TRUE 执行成功
	* 		     HKL_DLL_FAILU 执行失败
	*  抛出异常：无
	***********************************************************************/
	BOOL IsWantedFile(const CString &str)
	{
		CString strLower;
		TCHAR   szExt[_MAX_EXT] = _T("");
		int videoType = 0;//判断是否符合的视频文件标记
		int musicType = 0;//判断是否符合的音频文件标记

		// 这里偷懒直接用了之前的过滤字符串，由于文件名不能含有【*】，所以可以在后缀名后面加上【;*】来判断是否完全匹配
		const   CString Video_FileFilter =
			_T("*.rm;*.rmvb;*.flv;*.f4v;*.avi;*.3gp;*.mp4;*.wmv;*.mpeg;*.mpga;*.asf;*.dat;*.mov;*");
		const   CString Music_FileFilter =
			_T("*.mp3;*.wma;*.wav;*.mid;*.rmi;*.aac;*.ac3;*.aiff;*.m4a;*.mka;*.mp2;*.ogg;*");

		_tsplitpath_s(str, NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);   // 获取后缀名
		strLower = szExt;
		strLower.MakeLower();

		if(! strLower.IsEmpty())    // 没有后缀名的不符合条件
		{
			strLower += _T(";*");   // .mo不符合条件，由于会匹配到.mov，所以在后面加上【;*】来判断是否完全匹配
			videoType = Video_FileFilter.Find(strLower);
			musicType = Music_FileFilter.Find(strLower);
			//在这里做判断，返回1是视频；返回0是音频；返回-1不符合的后缀格式
			if (videoType>0)
			{
				return 1;
			}
			if (musicType>0)
			{
				return 0;
			}
			if (videoType<0||musicType<0)
			{
				return -1;
			}
			//return -1 != STR_FileFilter.Find(strLower);
		}
		return FALSE;
	}

	/***********************************************************************
	*  函 数 名：FindInAll                                          
	*  功能描述: 遍历文件夹函数                                         
	*  输入参数：LPCTSTR lpPath      [IN] :文件夹路径      
	*  输出参数: 无
	*  返 回 值：无
	*  抛出异常：无
	***********************************************************************/
	void TraverseFolder(LPCTSTR lpPath)
	{
		TCHAR szFind[MAX_PATH] = {_T("\0")};
		WIN32_FIND_DATA findFileData;
		BOOL bRet;

		_tcscpy_s(szFind, MAX_PATH, lpPath);
		_tcscat_s(szFind, _T("\\*.*"));     //这里一定要指明通配符，不然不会读取所有文件和目录

		HANDLE hFind = ::FindFirstFile(szFind, &findFileData);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			return;
		}

		//遍历文件夹
		while (TRUE)
		{
			if (findFileData.cFileName[0] != _T('.'))
			{
				//不是当前路径或者父目录的快捷方式
				_tprintf(_T("%s\\%s\n"), lpPath, findFileData.cFileName);
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					//这是一个普通目录

					//设置下一个将要扫描的文件夹路径
					_tcscpy_s(szFind, MAX_PATH, lpPath);    
					_tcscat_s(szFind, _T("\\"));    
					_tcscat_s(szFind, findFileData.cFileName);
					///_tcscat_s(szNextDir, _T("\\*"));
					//遍历该目录
					TraverseFolder(szFind);
				}

			}

			//如果是当前路径或者父目录的快捷方式，或者是普通目录，则寻找下一个目录或者文件
			bRet = ::FindNextFile(hFind, &findFileData);
			if (!bRet)
			{
				//函数调用失败

				//cout << "FindNextFile failed, error code: " 

				//  << GetLastError() << endl;
				break;
			}
		}
		::FindClose(hFind);
	}

	/***********************************************************************
	*  函 数 名：FindInAll                                          
	*  功能描述: 遍历所有文件                                         
	*  输入参数：LPCTSTR lpszPath       [IN] :文件夹路径      
	*  输出参数: 无
	*  返 回 值：无
	*  抛出异常：无
	***********************************************************************/
	void FindInAll(LPCTSTR lpszPath) 
	{    
		TCHAR szFind[MAX_PATH];     
		lstrcpy(szFind, lpszPath);     
		if (!IsRoot(szFind))         
			lstrcat(szFind, _T("\\"));     
		lstrcat(szFind, FILEILTER); 
		// 找所有文件     
		WIN32_FIND_DATA wfd;     
		HANDLE hFind = FindFirstFile(szFind, &wfd);     
		if (hFind == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败         
			return;          
		do     
		{         
			if (wfd.cFileName[0] == '.')             
				continue; // 过滤这两个目录         
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)         
			{             
				TCHAR szFile[MAX_PATH];             
				if (IsRoot(lpszPath))                 
					wsprintf(szFile, _T("%s%s"), lpszPath, wfd.cFileName);            
				else             
				{
					wsprintf(szFile, _T("%s\\%s"), lpszPath, wfd.cFileName);                 
					FindInAll(szFile); // 如果找到的是目录，则进入此目录进行递归             
				}       
			}        
			else         
			{             
				TCHAR szFile[MAX_PATH];             
				if (IsRoot(lpszPath))             
				{                
					wsprintf(szFile, _T("%s%s"), lpszPath, wfd.cFileName);             
				}           
				else             
				{                
					wsprintf(szFile, _T("%s\\%s"), lpszPath, wfd.cFileName);               
					printf("%s\n",szFile);  
					InsertMusicMap(szFile);
				}// 对文件进行操作         
			}     
		}
		while (FindNextFile(hFind, &wfd));     
		FindClose(hFind); // 关闭查找句柄      
	} 

	BOOL IsRoot(LPCTSTR lpszPath) 
	{     
		TCHAR szRoot[4];     
		wsprintf(szRoot, _T("%c:\\"), lpszPath[0]);     
		return (lstrcmp(szRoot, lpszPath) == 0); 
	} 
	//int main(int argc, char* argv[]) {     char findFile[64]="d:";//要查找的目录    FindInAll(findFile);    getchar();    return 0; } 

	void InsertMusicMap( LPCTSTR lpFilePath )
	{
		//保存拖拽的文件给音频操作类操作
		int i=0;
		////加载文件
		//HSTREAM hStream = m_pBassMusic->LoadFile(lpFilePath);
		//if ( hStream == -1 ) return;

		////往ListBox中添加新数据
		////m_ListMusic.AddString(lpFileName);

		////获取媒体标签
		//tagMusicInfo *pInfo = m_pBassMusic->GetInfo(hStream);

		////通过map和ListBox结合，一起管理播放列表
		//tagMusicInfo *pMusicInfo = new tagMusicInfo;

		//pMusicInfo->dwTime = pInfo->dwTime;
		//pMusicInfo->hStream = pInfo->hStream;
		//lstrcpyn(pMusicInfo->szArtist,pInfo->szArtist,CountArray(pMusicInfo->szArtist));
		//lstrcpyn(pMusicInfo->szTitle,pInfo->szTitle,CountArray(pMusicInfo->szTitle));

		//m_MusicManager.insert(pair<int,tagMusicInfo*>(1,pMusicInfo));
	}

};

