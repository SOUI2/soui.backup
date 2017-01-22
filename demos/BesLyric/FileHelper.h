/*
	BesLyric  一款 操作简单、功能实用的 专门用于制作网易云音乐滚动歌词的 歌词制作软件。
    Copyright (C) 2017  BensonLaur

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
* @file       FileHelper.h
* @version    v1.0      
* @author     BensonLaur   
* @date       2017/01/08
* 
* Describe    File类（管理打开的文件） 和CFileDialogEx 类（用对话框选择文件和文件夹）的定义
*/

#pragma once
#include "stdafx.h"
#include <windows.h>

//定义歌词文件每一行的最多字符数
#define MAX_CHAR_COUNT_OF_LINE 200
#define MAX_WCHAR_COUNT_OF_LINE MAX_CHAR_COUNT_OF_LINE/2

//改写GetOpenFileName时用到的系统使用控件的资源ID
#define  ID_COMBO_ADDR 0x47c
#define  ID_LEFT_TOOBAR 0x4A0

//使用GetOpenFileName打开文件夹时 用来替换 m_ofn.lpfnHook 的回调函数 
UINT_PTR __stdcall  MyFolderProc(  HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam );
LRESULT __stdcall  _WndProc ( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam  );

/*
*   @brief 应用RAII思想，管理文件文件的资源
*/
class File{

public:
	File():m_pf(NULL),m_lpszPathFile(NULL),m_lpszMode(NULL){}
	
	File(LPCTSTR pathFile,LPCTSTR mode)
	{
		m_pf=_tfopen(pathFile,mode);

		m_lpszPathFile = pathFile;
		m_lpszMode = mode;
	}

	BOOL openFile(LPCTSTR pathFile,LPCTSTR mode)
	{
		//如果已存在，则释放资源
		if(m_pf)
			fclose(m_pf);
		File(pathFile,mode);
	}

	BOOL isValidFile()
	{
		return m_pf!=NULL;
	}

	~File()
	{
		//如果已存在，则释放资源
		if(m_pf)
			fclose(m_pf);
	}
public:
	LPCTSTR m_lpszPathFile;		/* 文件路径和名字串*/
	LPCTSTR m_lpszMode;			/* 打开文件的模式 */

	FILE *m_pf;					/* 存放当前打开文件的指针*/

};

/*
*	@用于打开文件 和 文件夹； 以及 保存文件
*/
class CFileDialogEx
{
public:

    OPENFILENAME m_ofn;
    BOOL m_bOpenFileDialog;            // TRUE for file open, FALSE for file save
    TCHAR m_szFileTitle[_MAX_FNAME];   // contains file title after return
    TCHAR m_szFileName[_MAX_PATH];     // contains full path name after return

    CFileDialogEx(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
        LPCTSTR lpszDefExt = NULL,
        LPCTSTR lpszFileName = NULL,
        DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT ,
        LPCTSTR lpszFilter = NULL,
        HWND hWndParent = NULL,
		BOOL bFloder = FALSE)
    {
        memset(&m_ofn, 0, sizeof(m_ofn)); // initialize structure to 0/NULL
        m_szFileName[0] = _T('\0');
        m_szFileTitle[0] = _T('\0');

        m_bOpenFileDialog = bOpenFileDialog;
        m_ofn.lStructSize = sizeof(m_ofn);
        m_ofn.lpstrFile = m_szFileName;
        m_ofn.nMaxFile = _MAX_PATH;
        m_ofn.lpstrDefExt = lpszDefExt;
        m_ofn.lpstrFileTitle = (LPTSTR)m_szFileTitle;
        m_ofn.nMaxFileTitle = _MAX_FNAME;
        m_ofn.Flags = dwFlags | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING| OFN_NOCHANGEDIR;
        m_ofn.lpstrFilter = lpszFilter;
        m_ofn.hwndOwner = hWndParent;

        // setup initial file name
        if(lpszFileName != NULL)
            _tcscpy_s(m_szFileName, _countof(m_szFileName), lpszFileName);

		//文件夹设置
		if(bFloder)
		{
			m_ofn.hInstance = (HMODULE)GetCurrentProcess();//不要使用NULL,可能造成无法定制的问题
			m_ofn.lpfnHook = (LPOFNHOOKPROC)MyFolderProc;
		}
    }

    INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
    {
        if(m_ofn.hwndOwner == NULL)   // set only if not specified before
            m_ofn.hwndOwner = hWndParent;

        if(m_bOpenFileDialog)
            return ::GetOpenFileName(&m_ofn);
        else
            return ::GetSaveFileName(&m_ofn);
    }

public:
	/**
	*   @brief 检查文件名是否符合要求格式（仅仅检查名字上的格式）
	*	@param  format  支持的检查格式		普通文件格式：如 *.txt、 *.mp3  ("*." 是必须的；且后缀必须至少有一个字符)
	*										文件夹格式：..
	*			toChecked 被检查的路径字符串
	*	@return TRUE 符合要求
	*	@note	
	*/

	static BOOL checkPathName(LPCTSTR format,LPCTSTR toChecked)
	{
		int i;
		bool isFloder = false;
		//TODO：异常抛出处理
		int len = _tcslen(format);
		if(_tcscmp(format,_T(".."))==0)
		{
			isFloder = true;
		}
		else if(len < 3 || format[0]!=_T('*') || format[1]!=_T('.'))
			return FALSE;  //TODO：异常
		

		//获取并检查 被检查的路径字符串 toChecked 的信息
		TCHAR pathName[_MAX_PATH];
		TCHAR ext[_MAX_EXT];

		int lenPathName = 0, pos =-1;

		_tcscpy(pathName,toChecked);
		lenPathName = _tcslen(pathName);	//得到路径总长
		if(!lenPathName)
			return FALSE;

		//得到路径中最后一个“.”的位置置于pos中
		for( i=0; i< lenPathName; i++)
		{
			if(_T('.')==pathName[i])
				pos = i;
		}

		if(isFloder) //检查文件夹类型
		{
			if(pos == -1)//这里默认文件夹的路径不包含任何点'.'
				return TRUE;
			else
				return FALSE;
		}
		else //检查普通后缀名类型
		{
			_tcscpy(ext,&pathName[pos+1]);  //得到路径的后缀（不包含“.”）
			if(_tcscmp(&format[2],ext)==0)	//和 参数提供的后缀对比
				return TRUE;
			else
				return FALSE;
		}
	}

};

